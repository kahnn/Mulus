#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <sys/select.h>
#include <errno.h>

#include "mls_elope.h"
#include "mls_elnet.h"
#include "mls_el.h"
#include "mls_log.h"

static struct mls_elope _lelope;
static unsigned char _req[MLS_ELOPE_PACKET_LENGTH_MAX];
static unsigned char _res[MLS_ELOPE_PACKET_LENGTH_MAX];

/***************************************************************/

/* TODO: 設定で切り替え可能にする */
static int _is_packet_hex_dump = 1;

static ssize_t
SENDTO(int sockfd, const void *buf, size_t len, int flags,
    const struct sockaddr *dest_addr, socklen_t addrlen)
{
    ssize_t slen;
    if (_is_packet_hex_dump) {
        /* TODO: refine hex dump log */
        fprintf(stderr, "[SEND]: len=%d\n", (int)len);
        mls_log_hexdump((char*)buf, len, stderr);
    }

    slen = sendto(sockfd, buf, len, flags, dest_addr, addrlen);
    return slen;
}

static ssize_t
RECVFROM(int sockfd, void *buf, size_t len, int flags,
    struct sockaddr *from, socklen_t *addrlen)
{
    ssize_t rlen;
    rlen = recvfrom(sockfd, buf, len, flags, from, addrlen);
    if ((0 < rlen) && _is_packet_hex_dump) {
        /* TODO: refine hex dump log */
#if 0
        fprintf(stderr, "[RECV]: %ld=%d\n",sizeof(struct sockaddr_un),*addrlen);
        fprintf(stderr, "[RECV]: %d %s\n", from->sun_family, from->sun_path);
#endif
        fprintf(stderr, "[RECV]: len=%d,\n", (int)rlen);
        mls_log_hexdump((char*)buf, rlen, stderr);
    }

    return rlen;
}

/***************************************************************/

struct mls_elope*
mls_elope_init(void)
{
    struct mls_elope* elope = NULL;
    struct mls_net_ud_srv *csrv = NULL;

    csrv = mls_net_udgram_srv_open(MLS_ELOPE_UD_SOCK_NAME);
    if (NULL == csrv) {
        LOG_ERR(MLS_LOG_DEFAULT_MODULE, 
            "mls_net_udgram_srv_open() error.\n");
        goto out;
    }
    _lelope.srv = csrv;

    elope = &_lelope;
out:
    return elope;
}

void
mls_elope_term(struct mls_elope *elope)
{
    mls_net_udgram_srv_close(elope->srv);
}

/***************************************************************/

static int
_recv_request(struct mls_elope *elope, struct mls_elope_packet **reqp)
{
    int ret = 0;
    unsigned char* req = _req;
    int len = sizeof(_req);

    elope->from_len = sizeof(elope->from);
    len = RECVFROM(elope->srv->sock, req, len, 0,
        (struct sockaddr*)&(elope->from), &(elope->from_len));
    if (len < 0) {
        ret = -errno;
        LOG_ERR(MLS_LOG_DEFAULT_MODULE,
            "recvfrom(%d): %s.\n", errno, strerror(errno));
        goto out;
    }

    *reqp = (struct mls_elope_packet*)req;

    /* check packet (length) */
    if (len < MLS_ELOPE_PACKET_HEADER_LENGTH) {
        LOG_ERR(MLS_LOG_DEFAULT_MODULE, "invalid packet length = %d", len);
        ret = -1;
        goto out;
    }

    switch((*reqp)->svc) {
    default:
        LOG_ERR(MLS_LOG_DEFAULT_MODULE,
            "invalid packet svc = %d", (*reqp)->svc);
        ret = -1;
        goto out;

    /* Request */
    case MLS_ELOPE_SVC_SET_REQ:
    case MLS_ELOPE_SVC_GET_REQ:
        break;
    }
    
out:
    return ret;
}

static int
_send_response(struct mls_elope *elope, struct mls_elope_packet *res)
{
    int len, ret = 0;
    len = mls_elope_packet_get_length(res);

    len = SENDTO(elope->srv->sock, res, len, 0, 
        (struct sockaddr*)&(elope->from), (elope->from_len));
    if (len < 0) {
        ret = -errno;
        LOG_ERR(MLS_LOG_DEFAULT_MODULE,
            "sendto(%d): %s.\n", errno, strerror(errno));
        goto out;
    }
out:
    return ret;
}

static unsigned char
_set_properties(struct mls_el_ctx *ctx, struct mls_elope_packet *req)
{
    unsigned char rc = MLS_ELOPE_RCODE_SUCCESS;
    struct mls_node *node;
    struct mls_eoj *eoj;
    struct mls_epr* epr;
    int ret_prop;

    node = mls_el_get_node(ctx) ;
    eoj = mls_el_node_get_device(node, &(req->deojc));
    if (NULL == eoj) {
        rc = MLS_ELOPE_RCODE_NOENT_INSTANCE; /* not found deoj */
        LOG_WARN(MLS_LOG_DEFAULT_MODULE,
            "not found deoj(%d,%d,%d)\n",
            req->deojc.cgc, req->deojc.clc, req->deojc.inc);
        goto out;
    }

    /* property */
    epr = mls_eoj_get_property(eoj, req->epc);
    if (NULL == epr) {
        rc = MLS_ELOPE_RCODE_NOENT_EPC; /* not found property */
        LOG_WARN(MLS_LOG_DEFAULT_MODULE, "not found epr(%d)\n", req->epc);
        goto out;
    }
    if (NULL == epr->setf) {
        rc = MLS_ELOPE_RCODE_NOENT_METHOD; /* not found method */
        LOG_WARN(MLS_LOG_DEFAULT_MODULE,
                "cannot access(%x)\n", epr->setf);
        goto out;
    }
    ret_prop = epr->setf(eoj, req->epc, req->data, req->pdc);
    if (ret_prop <= 0) {
        rc = MLS_ELOPE_RCODE_INVALID_DATA;
        LOG_WARN(MLS_LOG_DEFAULT_MODULE, "setf(%d,%d)\n", req->epc, ret_prop);
        goto out;
    }
    else if (epr->is_anno_when_changed) {
        /* announce */
        mls_elnet_announce_property(mls_el_get_elnet(ctx),
            node, &(req->deojc), req->epc, req->pdc, req->data);
    }

out:
    return rc;
}

static unsigned char
_get_properties(struct mls_el_ctx *ctx,
    struct mls_elope_packet *req, unsigned char *data, unsigned char *pdc)
{
    unsigned char rc = MLS_ELOPE_RCODE_SUCCESS;
    struct mls_node *node;
    struct mls_eoj *eoj;
    struct mls_epr* epr;
    int ret_prop;

    node = mls_el_get_node(ctx) ;
    eoj = mls_el_node_get_device(node, &(req->deojc));
    if (NULL == eoj) {
        rc = MLS_ELOPE_RCODE_NOENT_INSTANCE; /* not found deoj */
        LOG_WARN(MLS_LOG_DEFAULT_MODULE,
            "not found deoj(%d,%d,%d)\n",
            req->deojc.cgc, req->deojc.clc, req->deojc.inc);
        goto out;
    }

    /* property */
    epr = mls_eoj_get_property(eoj, req->epc);
    if (NULL == epr) {
        rc = MLS_ELOPE_RCODE_NOENT_EPC; /* not found property */
        LOG_WARN(MLS_LOG_DEFAULT_MODULE, "not found epr(%d)\n", req->epc);
        goto out;
    }
    if (NULL == epr->getf) {
        rc = MLS_ELOPE_RCODE_NOENT_METHOD; /* not found method */
        LOG_WARN(MLS_LOG_DEFAULT_MODULE,
                "cannot access(%x)\n", epr->getf);
        goto out;
    }
    ret_prop = epr->getf(eoj, req->epc, data, pdc);
    if (ret_prop <= 0) {
        rc = MLS_ELOPE_RCODE_INVALID_DATA;
        LOG_WARN(MLS_LOG_DEFAULT_MODULE, "getf(%d,%d)\n", req->epc, ret_prop);
        goto out;
    }
    *pdc = (unsigned char)ret_prop;

out:
    return rc;
}

static int
_handle_message(struct mls_el_ctx *ctx,
    struct mls_elope_packet *req, struct mls_elope_packet **res)
{
    int ret = 0;
    unsigned char svc, rc;
    *res = (struct mls_elope_packet*)_res;
    memset((*res), 0, sizeof(_res));

    switch(req->svc) {
    default:
        LOG_ERR(MLS_LOG_DEFAULT_MODULE, "invalid svc(%d)\n", req->svc);
        assert(0);
        break;

    /* Request */
    case MLS_ELOPE_SVC_GET_REQ:
        svc = MLS_ELOPE_SVC_GET_RES;
        (*res)->pdc = UCHAR_MAX;
        rc = _get_properties(ctx, req, (*res)->data, &((*res)->pdc));
        break;
    case MLS_ELOPE_SVC_SET_REQ:
        svc = MLS_ELOPE_SVC_SET_RES;
        (*res)->pdc = 0;
        rc = _set_properties(ctx, req);
        break;
    }

    /* Response */
    (*res)->svc = svc;
    (*res)->rc = rc;
    (*res)->deojc = req->deojc;
    (*res)->epc = req->epc;
    /* pdc & data set by _*_properties(). */

    return ret;
}

void
mls_elope_event_handler(struct mls_evt *evt, void *tag)
{
    int ret = 0;
    struct mls_el_ctx *ctx = (struct mls_el_ctx *)tag;
    struct mls_elope *elope = mls_el_get_elope(ctx);
    struct mls_elope_packet *req, *res;

    if ((ret = _recv_request(elope, &req)) != 0) {
        /* Invalid message, don't need response */
        LOG_ERR(MLS_LOG_DEFAULT_MODULE,
            "mls_elope_event_handler(1,%d)\n", ret);
        goto out;
    }

    if ((ret = _handle_message(ctx, req, &res)) != 0) {
        /* don't need response */
        LOG_ERR(MLS_LOG_DEFAULT_MODULE,
            "mls_elope_event_handler(2,%d)\n", ret);
        goto out;
    }

    if ((ret = _send_response(elope, res)) != 0) {
        LOG_ERR(MLS_LOG_DEFAULT_MODULE,
            "mls_elope_event_handler(3,%d)\n", ret);
        goto out;
    }

out:
    return;
}

/***************************************************************/

int
mls_elope_rpc(struct mls_net_ud_cln *cln,
    struct mls_elope_packet *req, struct mls_elope_packet *res, int reslen)
{
    int ret = 0;
    int sock = cln->sock;
    struct sockaddr_storage from;
    socklen_t fromlen;
    ssize_t len;

    fd_set mask;
    int width;
    int i, retry = 2, wait_sec = 2;

    FD_ZERO(&mask);
    FD_SET(sock, &mask);
    width = sock + 1;

    for (i = 0; i < retry; i++) {
        fd_set ready;
        struct timeval timeout;

        /* request */
        if ((len = SENDTO(sock, req, mls_elope_packet_get_length(req),
                          0, (struct sockaddr *)&(cln->to), cln->tolen)) == -1)
        {
            ret = -errno;
            LOG_ERR(MLS_LOG_DEFAULT_MODULE,
                    "sendto(%d): %s.\n", errno, strerror(errno));
            goto out;
        }

        ready = mask;
        timeout.tv_sec = wait_sec;
        timeout.tv_usec = 0;

        switch (select(width, (fd_set*)&ready, NULL, NULL, &timeout)) {
        case -1:
            ret = -errno;
            LOG_ERR(MLS_LOG_DEFAULT_MODULE,
                "select(%d,%s)\n", errno, strerror(errno));
            goto out;
        case 0:
            /* timeout, retry */
            break;

        default:
            if (FD_ISSET(sock, &ready)) {
                ssize_t len;

                /* response */
                fromlen = sizeof(from);
                if ((len = RECVFROM(sock, (char*)res, reslen, 
                                    0, (struct sockaddr*)&from, &fromlen)) == -1)
                {
                    ret = -errno;
                    LOG_ERR(MLS_LOG_DEFAULT_MODULE,
                            "recvfrom(%d): %s.\n", errno, strerror(errno));
                    goto out;
                }

                /* OK, response packet */
                ret = len;
                LOG_INFO(MLS_LOG_DEFAULT_MODULE,
                    "message rpc, ok(%d,%d)\n", i, ret);
                goto out;
            }
            break;
        }
    }

    /* retry over */
    ret = -1;
    LOG_ERR(MLS_LOG_DEFAULT_MODULE, "message rpc, retry over(%d,%d)\n", i, ret);

out:
    return ret;
}

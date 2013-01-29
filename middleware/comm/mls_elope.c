#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>

#include "mls_elope.h"
#include "mls_elnet.h"
#include "mls_el.h"
#include "mls_log.h"

static struct mls_elope _lelope;
static unsigned char _req[MLS_ELOPE_PACKET_LENGTH_MAX];
static unsigned char _res[MLS_ELOPE_PACKET_LENGTH_MAX];

/***************************************************************/

struct mls_elope*
mls_elope_init(void)
{
    struct mls_elope* elope = NULL;
    struct mls_net_ud_srv *csrv = NULL;

    csrv = mls_net_udgram_srv_open(MLS_ELOPE_UD_SOCK_NAME);
    if (NULL == csrv) {
        /* XXX error */
        fprintf(stderr, "ERROR: mls_net_udgram_srv_open()\n");
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
    len = recvfrom(elope->srv->sock, req, len, 0,
        (struct sockaddr*)&(elope->from), &(elope->from_len));
    if (len < 0) {
        /* XXXX error log */
        perror("recvfrom");
        ret = -errno;
        goto out;
    }
#if 0
    {
        printf("RECVFROM: %ld=%d\n", sizeof(struct sockaddr_un), from_len);
        printf("RECVFROM: %d %s\n", from.sun_family, from.sun_path);
    }
#endif    
#if 1
    {
        fprintf(stdout, "[REQ]:len=%d\n", (int)len);
        mls_log_hexdump((char*)req, len, stdout);
    }
#endif

    *reqp = (struct mls_elope_packet*)req;

    /* check packet (length) */
    if (len < MLS_ELOPE_PACKET_HEADER_LENGTH) {
        /* ignore message XXXX error log */
        ret = -1;
        goto out;
    }

    switch((*reqp)->svc) {
    default:
        /* ignore message XXXX error log */
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

#if 1
    {
        fprintf(stdout, "[RES]:len=%d\n", len);
        mls_log_hexdump((char*)res, len, stdout);
    }
#endif
    len = sendto(elope->srv->sock, res, len, 0, 
        (struct sockaddr*)&(elope->from), (elope->from_len));
    if (len < 0) {
        /* XXXX error log */
        perror("sendto():");
        ret = -errno;
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
        goto out;
    }

    /* property */
    epr = mls_eoj_get_property(eoj, req->epc);
    if (NULL == epr) {
        rc = MLS_ELOPE_RCODE_NOENT_EPC; /* not found property */
        goto out;
    }
    if ((!(epr->access_attr & MLS_EPR_ACCESS_SET)) || (NULL == epr->setf)) {
        rc = MLS_ELOPE_RCODE_NOENT_METHOD; /* not found method */
        goto out;
    }
    ret_prop = epr->setf(eoj, req->epc, req->data, req->pdc);
    if (ret_prop <= 0) {
        rc = MLS_ELOPE_RCODE_INVALID_DATA;
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
        goto out;
    }

    /* property */
    epr = mls_eoj_get_property(eoj, req->epc);
    if (NULL == epr) {
        rc = MLS_ELOPE_RCODE_NOENT_EPC; /* not found property */
        goto out;
    }
    if ((!(epr->access_attr & MLS_EPR_ACCESS_GET)) || (NULL == epr->getf)) {
        rc = MLS_ELOPE_RCODE_NOENT_METHOD; /* not found method */
        goto out;
    }
    ret_prop = epr->getf(eoj, req->epc, data, pdc);
    if (ret_prop <= 0) {
        rc = MLS_ELOPE_RCODE_INVALID_DATA;
        goto out;
    }

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
        assert(0);
        break;

    /* Request */
    case MLS_ELOPE_SVC_GET_REQ:
        svc = MLS_ELOPE_SVC_GET_RES;
        rc = _get_properties(ctx, req, (*res)->data, &((*res)->pdc));
        break;
    case MLS_ELOPE_SVC_SET_REQ:
        svc = MLS_ELOPE_SVC_SET_RES;
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
        /* XXX error */
        fprintf(stderr, "ERROR: mls_elope_event_handler(1)\n");
        goto out;
    }

    if ((ret = _handle_message(ctx, req, &res)) != 0) {
        /* XXX error */
        /* don't need response */
        fprintf(stderr, "ERROR: mls_elope_event_handler(2)\n");
        goto out;
    }

    if ((ret = _send_response(elope, res)) != 0) {
        /* XXX error */
        fprintf(stderr, "ERROR: mls_elope_event_handler(3)\n");
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
    struct sockaddr_storage from;
    socklen_t fromlen;
    ssize_t len;

    /* request */
    if ((len = sendto(cln->sock, req, mls_elope_packet_get_length(req),
                0, (struct sockaddr *)&(cln->to), cln->tolen)) == -1)
    {
        /* XXX error */
        ret = -errno;
        perror("sendto");
        goto out;
    }

    /* response */
    fromlen = sizeof(from);
    if ((len = recvfrom(cln->sock, (char*)res, reslen, 
                0, (struct sockaddr*)&from, &fromlen)) == -1)
    {
        /* XXX error */
        ret = -errno;
        perror("recvfrom");
        goto out;
    }

out:
    return ret;
}

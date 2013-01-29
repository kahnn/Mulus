#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>

#include "mls_type.h"
#include "mls_elnet.h"
#include "mls_el.h"
#include "mls_log.h"

static struct mls_elnet _lelnet;
static unsigned char _req[MLS_ELNET_FRAME_LENGTH_MAX];
static unsigned char _res[MLS_ELNET_FRAME_LENGTH_MAX];

/*
  @arg ifname  ex."eth0" -or- "192.168.11.131" ....
 */
struct mls_elnet*
mls_elnet_init(char *ifname)
{
    struct mls_net_mcast_srv* srv = NULL;
    struct mls_elnet* elnet = NULL;
    char ifaddr[64];

    if (isdigit(*ifname)) {
        strncpy(ifaddr, ifname, sizeof(ifaddr));
    } else {
        int ret;
        ret =
            mls_net_getaddr_by_ifname(ifname, AF_INET, ifaddr, sizeof(ifaddr));
        if (0 != ret) {
            fprintf(stderr, "mls_net_getaddr_by_ifname(%d) error.\n", ret);
            goto out;
        }
    }
    srv = 
        mls_net_mcast_srv_open(MLS_ELNET_MCAST_ADDRESS, MLS_ELNET_MCAST_PORT,
            ifaddr);
    if (NULL == srv) {
        fprintf(stderr, "mls_net_mcast_srv_open() error.\n");
        goto out;
    }

    elnet = &_lelnet;
    elnet->srv = srv;

out:
    return elnet;
}

void
mls_elnet_term(struct mls_elnet* elnet)
{
    mls_net_mcast_srv_close(elnet->srv);
}

/* ------------------------------------------------------------- */

static unsigned short
_get_next_tid(void)
{
    static unsigned short _tid;
    return ++_tid;
}

static int
_set_property(unsigned char* buf, unsigned int* len, 
    struct mls_eoj* eoj, struct mls_epr* prop)
{
    unsigned char size = ((UCHAR_MAX <= *len) ? UCHAR_MAX : *len);
    buf[0] = prop->epc;
    prop->getf(eoj, prop->epc, &(buf[2]), &size);
    buf[1] = size;

    *len -= (int)size;

    return size;
}

static int
_check_frame_header(struct mls_elnet_frame *header, int len)
{
    int ret = 0;

    if (len < MLS_ELNET_FRAME_HEADER_LENGTH) {
        /* XXXX error log */
        ret = -1;
        goto out;
    }

    if ((MLS_ELNET_EHD1_ECHONET_LITE != header->ehd1) ||
        (MLS_ELNET_EHD2_REGULAR != header->ehd1))
    {
        /* XXXX error log */
        ret = -1;
        goto out;
    }

out:
    return ret;
}

/*
  @arg status =0: success, <0: error
 */
static unsigned char
_select_response_esv(unsigned char req_esv, int status)
{ 
    unsigned char res_esv = 0;

    switch(req_esv) {
    default:
        assert(0);
        break;

    /* Request(0x6X) */
    case MLS_ELNET_ESV_SetI:
        if (0 == status) assert(0);
        else             res_esv = MLS_ELNET_ESV_SetI_SNA;
        break;
    case MLS_ELNET_ESV_SetC:
        if (0 == status) res_esv = MLS_ELNET_ESV_Set_Res;
        else             res_esv = MLS_ELNET_ESV_SetC_SNA;
        break;
    case MLS_ELNET_ESV_Get:
        if (0 == status) res_esv = MLS_ELNET_ESV_Get_Res;
        else             res_esv = MLS_ELNET_ESV_Get_SNA;
        break;
    case MLS_ELNET_ESV_INF_REQ:
        if (0 == status) res_esv = MLS_ELNET_ESV_INF;
        else             res_esv = MLS_ELNET_ESV_INF_SNA;
        break;
    case MLS_ELNET_ESV_SetGet:
        if (0 == status) res_esv = MLS_ELNET_ESV_SetGet_Res;
        else             res_esv = MLS_ELNET_ESV_SetGet_SNA;
        break;

    /* Response(0x7X) */
    case MLS_ELNET_ESV_INFC: /* spontaneous */
        res_esv = MLS_ELNET_ESV_INFC_Res;
        break;
    }

    return res_esv;
}

/*
  @arg reslen resのサイズが指定され、コピー後の残りサイズを返す。
  @return =0 : 処理正常
          =-1: EOJが見つからない -or- 破棄message
          =-2: EPC処理異常
*/
static int
_set_properties(struct mls_el_ctx *ctx,
    struct mls_elnet_frame* reqp, unsigned int reqd_len,
    unsigned char *res, unsigned int *reslen)
{
    int ret = 0;
    struct mls_node* node;
    struct mls_eoj *eoj;
    int i;
    unsigned char *reqd = reqp->data;

    node = mls_el_get_node(ctx) ;
    eoj = mls_el_node_get_device(node, &(reqp->deoj));
    if (NULL == eoj) {
        ret = -1; /* not found deoj */
        goto out;
    }

    *reslen = 0;
    for (i = 0; i < reqp->opc; i++) {
        int ret_prop;
        unsigned char epc, pdc;
        struct mls_epr* epr;

        if (reqd_len < 2) {
            ret = -1; /* invalid packet */
            goto out;
        }
        epc = reqd[0];
        pdc = reqd[1];
        reqd += 2; reqd_len -= 2;
        if (reqd_len < pdc) {
            ret = -1; /* invalid packet */
            goto out;
        }

        /* set property */
        epr = mls_eoj_get_property(eoj, epc);
        if (NULL == epr) {
            ret_prop = -1;
            goto set_res;
        }
        if (!(epr->access_attr & MLS_EPR_ACCESS_SET) || (NULL == epr->setf)) {
            ret_prop = -1;
            goto set_res;
        }
        ret_prop = epr->setf(eoj, epc, reqd, pdc);
        if ((0 < ret_prop) && (epr->is_anno_when_changed)) {
            /* announce */
            mls_elnet_announce_property(mls_el_get_elnet(ctx),
                node, &(reqp->deoj), epc, pdc, reqd);
        }
    set_res:
        {
            res[0] = epc;
            if (ret_prop < 0)
                res[1] = pdc;
            else
                res[1] = 0; /* pdc */
            res += 2; *reslen += 2;
            if (ret_prop < 0) {
                memcpy(res, reqd, pdc);
                res += pdc; *reslen += pdc;
            }

            /* EPC error. */
            if (ret_prop < 0)
                ret = -2;
        }

        reqd += pdc; reqd_len -= pdc;
    }

out:
    return ret;
}

static int
_get_properties(struct mls_el_ctx *ctx,
    struct mls_elnet_frame* reqp, unsigned int reqd_len,
    unsigned char *res, unsigned int *reslen)
{
    int ret = 0;
    struct mls_node* node;
    struct mls_eoj *eoj;
    int i;
    unsigned char *reqd = reqp->data;

    node = mls_el_get_node(ctx) ;
    eoj = mls_el_node_get_device(node, &(reqp->deoj));
    if (NULL == eoj) {
        ret = -1; /* not found deoj */
        goto out;
    }

    *reslen = 0;
    for (i = 0; i < reqp->opc; i++) {
        int ret_prop;
        unsigned char epc, pdc;
        struct mls_epr* epr;
        unsigned char tmplen = UCHAR_MAX;
        unsigned char tmp[UCHAR_MAX];

        if (reqd_len < 2) {
            ret = -1; /* invalid packet */
            goto out;
        }
        epc = reqd[0];
        pdc = reqd[1];
        reqd += 2; reqd_len -= 2;
        if (0 != pdc) {
            ret = -1; /* invalid packet */
            goto out;
        }

        /* get property */
        epr = mls_eoj_get_property(eoj, epc);
        if (NULL == epr) {
            ret_prop = -1;
            goto set_res;
        }
        if (!(epr->access_attr & MLS_EPR_ACCESS_GET) || (NULL == epr->getf)) {
            ret_prop = -1;
            goto set_res;
        }
        ret_prop = epr->getf(eoj, epc, tmp, &tmplen);
    set_res:
        {
            res[0] = epc;
            if (0 < ret_prop)
                res[1] = (unsigned int)ret_prop;
            else
                res[1] = 0; /* pdc */
            res += 2; *reslen += 2;
            if (0 < ret_prop) {
                memcpy(res, tmp, ret_prop);
                res += ret_prop; *reslen += ret_prop;
            }

            /* EPC error. */
            if (ret_prop <= 0)
                ret = -2;
        }
    }

out:
    return ret;
}

static int
_setget_properties(struct mls_el_ctx *ctx,
    struct mls_elnet_frame* req, unsigned int reqlen,
    unsigned char *res, unsigned int *reslen)
{
    /* XXXXXX not impl */
    return -1;
}

static int
_inf_properties(struct mls_el_ctx *ctx,
    struct mls_elnet_frame* reqp, unsigned int reqd_len,
    unsigned char *res, unsigned int *reslen)
{
    int ret = 0;
    struct mls_node* node;
    struct mls_eoj *eoj;
    int i;
    unsigned char *reqd = reqp->data;

    node = mls_el_get_node(ctx) ;
    eoj = mls_el_node_get_device(node, &(reqp->deoj));
    if (NULL == eoj) {
        ret = -1; /* not found deoj XXXX */
        goto out;
    }

    *reslen = 0;
    for (i = 0; i < reqp->opc; i++) {
        unsigned char epc, pdc;

        if (reqd_len < 2) {
            ret = -1; /* invalid packet XXXX */
            goto out;
        }
        epc = reqd[0];
        pdc = reqd[1];
        reqd += 2; reqd_len -= 2;
        if (reqd_len < pdc) {
            ret = -1; /* invalid packet XXXX */
            goto out;
        }

        /* XXXXXX
           仮想ノード(proxy)機能をサポートしないと意味がない
           とりあえず、処理した事だけを返す。
         */
        {
            res[0] = epc;
            res[1] = 0; /* pdc */
            res += 2; *reslen += 2;
        }

        reqd += pdc; reqd_len -= pdc;
    }

out:
    return ret;
}

/*
  @arg reslen resのサイズが指定され、コピーしたサイズを返す。
  @return =0 : 処理正常 -or- EPC処理異常
          =-1: EOJが見つからない -or- 破棄message
 */
static int
_handle_message(struct mls_el_ctx *ctx,
    unsigned char* _req, unsigned int _reqlen, 
    unsigned char* _res, unsigned int *_reslen)
{
    int ret = 0;
    struct mls_elnet_frame *req = (struct mls_elnet_frame*)_req;
    int reqd_len = _reqlen - MLS_ELNET_FRAME_HEADER_LENGTH;
    struct mls_elnet_frame *res = (struct mls_elnet_frame*)_res;
    unsigned int resd_len;

    ret = _check_frame_header(req, _reqlen);
    if (ret < 0) {
        /* ignore message */
        goto out;
    }

    switch(req->esv) {
    default:
        /* ignore message XXXX error log */
        ret = -1;
        break;

    /* Request(0x6X) */
    case MLS_ELNET_ESV_SetI:
    case MLS_ELNET_ESV_SetC:
        ret = _set_properties(ctx, req, reqd_len, res->data, &resd_len);
        break;
    case MLS_ELNET_ESV_Get:
    case MLS_ELNET_ESV_INF_REQ:
        ret = _get_properties(ctx, req, reqd_len, res->data, &resd_len);
        break;
    case MLS_ELNET_ESV_SetGet:
        ret = _setget_properties(ctx, req, reqd_len, res->data, &resd_len);
        break;

    /* Response(0x7X) */
    case MLS_ELNET_ESV_Set_Res:
    case MLS_ELNET_ESV_Get_Res:
    case MLS_ELNET_ESV_INF:  /* don't need response */
    case MLS_ELNET_ESV_INFC_Res: /* INFC response */
    case MLS_ELNET_ESV_SetGet_Res:
        /* ignore message XXXX error log */
        ret = -1;
        break;
    case MLS_ELNET_ESV_INFC: /* spontaneous */
        ret = _inf_properties(ctx, req, reqd_len, res->data, &resd_len);
        break;

    /* Error Response(0x5X) */
    case MLS_ELNET_ESV_SetI_SNA:
    case MLS_ELNET_ESV_SetC_SNA:
    case MLS_ELNET_ESV_Get_SNA:
    case MLS_ELNET_ESV_INF_SNA:
    case MLS_ELNET_ESV_SetGet_SNA:
        /* ignore message XXXX error log */
        ret = -1;
        break;
    }

    /* *********************************************** */
    /*
      ret==0 : 処理正常
      ret==-1: EOJが見つからない -or- 破棄message
      ret==-2: EPC処理異常
     */
    /* not found eoj -or- discard message type */
    if (-1 == ret) {
        goto out;
    }
    /* don't need response message */
    if ((MLS_ELNET_ESV_SetI == req->esv) && (ret == 0)) {
        ret = -1;
        goto out;
    }
    /* set packet */
    {
        unsigned char res_esv = _select_response_esv(req->esv, ret);
        mls_elnet_setup_frame_header(res, 
            &(req->deoj), &(req->seoj), res_esv, req->opc, 0);
        res->tid = req->tid; /* set request-tid */
        *_reslen = MLS_ELNET_FRAME_HEADER_LENGTH + resd_len;

        ret = 0; /* ok, send message -- when ret is 0 -or- -2 */
    }
out:
    return ret;
}

/* ------------------------------------------------------------- */

/*
  指定されたプロパティの内容をアナウンスする。
 */
void
mls_elnet_announce_property(struct mls_elnet *elnet,
    struct mls_node* node, struct mls_eoj_code *deojc,
    unsigned char epc, unsigned char pdc, unsigned char *data)
{
    unsigned char tmp[MLS_ELNET_FRAME_HEADER_LENGTH + 2 + UCHAR_MAX];
    struct mls_elnet_frame *req = (struct mls_elnet_frame*)tmp;
    int reqlen = MLS_ELNET_FRAME_HEADER_LENGTH + 2 + pdc;

    mls_elnet_setup_frame_header(req, 
        deojc, &(node->prof->code), MLS_ELNET_ESV_INF, 1, 1);
    req->data[0] = epc;
    req->data[1] = pdc;
    memcpy(&(req->data[2]), data, pdc);

    {
        int ret;
        ret = sendto(elnet->srv->sock, tmp, reqlen, 0,
            (struct sockaddr*)&(elnet->srv->to), elnet->srv->tolen);
        if (-1 == ret) {
            /* XXXX error log */
            perror("sendto@mls_elnet_announce_profile()");
        }
    }
}

/*
  初期処理で、指定されたノードの内容をアナウンスする。
 */
void
mls_elnet_announce_profile(struct mls_elnet *elnet, struct mls_node *node)
{
    struct mls_eoj* profile = node->prof;
    struct mls_elnet_frame *req = (struct mls_elnet_frame*)_req;
    unsigned int reqlen = sizeof(_req), proplen = sizeof(_req);

    /* packet base */
    mls_elnet_setup_frame_header(req, 
        &(profile->code), &(profile->code), MLS_ELNET_ESV_INF, 1, 1);
    /* property */
    proplen = _set_property(req->data, &proplen, 
        profile, 
        mls_eoj_get_property(profile, MLS_EL_EPC_INSTANCE_LIST_NOTIFICATION));
    reqlen = MLS_ELNET_FRAME_HEADER_LENGTH + 2 + proplen;

    {
        int ret;
        ret = sendto(elnet->srv->sock, (char *)req, reqlen, 0,
            (struct sockaddr*)&(elnet->srv->to), elnet->srv->tolen);
        if (-1 == ret) {
            /* XXXX error log */
            perror("sendto@mls_elnet_announce_profile()");
        }
    }
}

/*
  通信処理の本体。ここで、RECEIVE,RESPONSE処理をおこなう。
 */
/* Event handler for mls_evt framework */
void
mls_elnet_event_handler(struct mls_evt* evt, void* tag)
{
    struct sockaddr_storage from;
    socklen_t fromlen;
    struct mls_el_ctx *ctx = (struct mls_el_ctx *)tag;
    struct mls_elnet *elnet = mls_el_get_elnet(ctx);
    int sock = elnet->srv->sock;
    unsigned char* req = _req;
    unsigned int len = sizeof(_req);
    unsigned char* res = _res;
    unsigned int reslen = sizeof(_res);

    /* Request */
    fromlen = sizeof(from);
    len = recvfrom(sock, req, len, 0, 
        (struct sockaddr*)&from, &fromlen);
    if (-1 == len) {
        /* XXXX error log */
        perror("recvfrom");
        return;
#if 0
        if (EAGAIN == errno)
            goto recv;
#endif
    }
#if 0 /* check */
    else {
        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
        getnameinfo((struct sockaddr *)&from, fromlen,
            hbuf, sizeof(hbuf),
            sbuf, sizeof(sbuf),
            NI_NUMERICHOST | NI_NUMERICSERV);
        fprintf(stdout, "recvfrom:%s:%s:len=%d\n", hbuf, sbuf, (int)len);
    }
#endif
#if 1
    {
        fprintf(stdout, "[REQ]:len=%d\n", (int)len);
        mls_log_hexdump((char*)req, len, stdout);
    }
#endif

    if (_handle_message(ctx, req, len, res, &reslen) < 0) {
        return; /* don't need response */
    }

#if 1
    {
        fprintf(stdout, "[RES]:len=%d\n", (int)reslen);
        mls_log_hexdump((char*)res, reslen, stdout);
    }
#endif

    /* Response */
    len = sendto(sock, res, reslen, 0, (struct sockaddr*)&from, fromlen);
    if (-1 == len) {
        /* XXXX error log */
        perror("sendto");
        return;
    }
}

static int
_unmarshal_frame_response(struct mls_elnet_frame *req,
    struct mls_elnet_frame *res, int reslen)
{
    int ret = 0;

    /* check header */
    ret = _check_frame_header(res, reslen);
    if (ret < 0) {
        goto out;
    }

    /* check tid */
#if 0
    res->tid = mls_type_get_short(&(res->tid), 2);
#endif
    if (req->tid != res->tid) {
        /* XXXX error log */
        ret = -1;
        goto out;
    }

    /* check seoj & deoj */
    if (!mls_eoj_equal_eojc(&(req->seoj), &(res->deoj)) ||
        !mls_eoj_equal_eojc(&(req->deoj), &(res->seoj)))
    {
        /* XXXX error log */
        ret = -1;
        goto out;
    }

    /* check esv */
    if ((req->esv & 0xF) != (res->esv & 0xF)) {
        /* XXXX error log */
        ret = -1;
        goto out;
    }
    
    /* check opc */
    if (req->opc != res->opc) {
        /* XXXX error log */
        ret = -1;
        goto out;
    }

out:
    return ret;
}

void
mls_elnet_setup_frame_header(struct mls_elnet_frame *req, 
    struct mls_eoj_code *seoj, struct mls_eoj_code *deoj,
    unsigned char esv, unsigned char opc, int set_auto_tid)
{
    req->ehd1 = MLS_ELNET_EHD1_ECHONET_LITE;
    req->ehd2 = MLS_ELNET_EHD2_REGULAR;
    req->tid = (set_auto_tid ? _get_next_tid() : 0);
    req->seoj = *seoj;
    req->deoj = *deoj;
    req->esv = esv;
    req->opc = opc;
}

int
mls_elnet_rpc(struct mls_net_mcast_cln *cln, char *addr, char *port,
    struct mls_elnet_frame *req, int reqlen,
    struct mls_elnet_frame *res, int reslen)
{
    int ret = 0;
    int sock = cln->sock;
    struct sockaddr_storage to, *top;
    socklen_t tolen;
    fd_set mask;
    int width;
    int i, retry = 3, wait_sec = 2;

    if (NULL != addr) {
        if ((ret = mls_net_get_sockaddr_info(addr, port, &to, &tolen)) < 0) {
            fprintf(stderr, "mls_net_get_sockaddr_info:%s\n", strerror(errno));
            goto out;
        }
        top = &to;
    } else {
        top = &(cln->to);
        tolen = cln->tolen;
    }

    FD_ZERO(&mask);
    FD_SET(sock, &mask);
    width = sock + 1;

    for (i = 0; i < retry; i++) {
        fd_set ready;
        struct timeval timeout;

        /* send request */
        ret = sendto(sock, (char*)req, reqlen, 0,
            (struct sockaddr*)&top, tolen);
        if (-1 == ret) {
            /* XXXX error log */
            ret = -errno;
            perror("sendto");
            goto out;
        }

        ready = mask;
        timeout.tv_sec = wait_sec;
        timeout.tv_usec = 0;

        switch (select(width, (fd_set*)&ready, NULL, NULL, &timeout)) {
        case -1:
            ret = -errno;
            perror("select");
            goto out;
        case 0:
            /* timeout, retry */
            break;

        default:
            if (FD_ISSET(sock, &ready)) {
                ssize_t len;

                /* recv response */
                cln->fromlen = sizeof(cln->from);
                if ((len = recvfrom(sock, res, reslen, 0,
                            (struct sockaddr*)&(cln->from),
                            &(cln->fromlen))) == -1)
                {
                    /* XXXX error log */
                    ret = -errno;
                    perror("recvfrom");
                    goto out;
                }

                /* un-marshal response */
                ret = _unmarshal_frame_response(req, res, len);
                if (ret < 0) {
                    /* ignore message retry, XXXX error log */
                    i--;
                    continue;
                }

                /* OK, response packet */
                ret = len;
                goto out;
            }
            break;
        }
    }

    /* retry over */
    ret = -1;

out:
    return ret;
}

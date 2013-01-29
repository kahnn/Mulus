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
static unsigned char _req[MLS_ELNET_PACKET_LENGTH];
static unsigned char _res[MLS_ELNET_PACKET_LENGTH];

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

/* XXXX struct mls_elnet_frame_base で書き換え */
unsigned char*
mls_elnet_set_packet_base(unsigned short tid, 
    struct mls_eoj_code *seoj, struct mls_eoj_code *deoj,
    unsigned char esv, unsigned char opc,
    unsigned char *buf, unsigned int *blen)
{
    unsigned int len = MLS_ELNET_PACKET_BASE_LENGTH;

    /* header */
    buf[0] = MLS_ELNET_EHD1_ECHONET_LITE;
    buf[1] = MLS_ELNET_EHD2_REGULAR;
    len -= 2;
    buf += 2;

    /* transaction id */
    buf += mls_type_set_short(buf, &len, (short)tid);

    /* seoj */
    buf += mls_eoj_set_eojcode(seoj, buf, &len);
    /* deoj */
    buf += mls_eoj_set_eojcode(deoj, buf, &len);

    /* esv */
    buf += mls_type_set_char(buf, &len, (char)esv);

    /* opc */
    buf += mls_type_set_char(buf, &len, (char)opc);

    if (NULL != blen)
        *blen -= MLS_ELNET_PACKET_BASE_LENGTH;

    return buf;
}

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
_check_header(unsigned char* buf, unsigned int* len)
{
    int header_size = 2;
    if ((MLS_ELNET_EHD1_ECHONET_LITE != buf[0]) ||
        (MLS_ELNET_EHD2_REGULAR != buf[1]))
    {
        return -1;
    }
    *len -= header_size;
    return header_size;
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
    struct mls_eoj_code *seojc, struct mls_eoj_code *deojc,
    unsigned char opc,
    unsigned char *req, unsigned int len,
    unsigned char *res, unsigned int *reslen)
{
    int ret = 0;
    struct mls_node* node;
    struct mls_eoj *eoj;
    int i;

    node = mls_el_get_node(ctx) ;
    eoj = mls_el_node_get_device(node, deojc);
    if (NULL == eoj) {
        ret = -1; /* not found deoj */
        goto out;
    }

    for (i = 0; i < opc; i++) {
        int ret_prop;
        unsigned char epc, pdc;
        struct mls_epr* epr;

        if (len < 2) {
            ret = -1; /* invalid packet */
            goto out;
        }
        epc = req[0];
        pdc = req[1];
        req += 2; len -= 2;
        if (len < pdc) {
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
        ret_prop = epr->setf(eoj, epc, req, pdc);
        if ((0 < ret_prop) && (epr->is_anno_when_changed)) {
            /* announce */
            mls_elnet_announce_property(mls_el_get_elnet(ctx),
                node, deojc, epc, pdc, req);
        }
    set_res:
        {
            res[0] = epc;
            if (ret_prop < 0)
                res[1] = pdc;
            else
                res[1] = 0; /* pdc */
            res += 2; *reslen -= 2;
            if (ret_prop < 0) {
                memcpy(res, req, pdc);
                res += pdc; *reslen -= pdc;
            }

            /* EPC error. */
            if (ret_prop < 0)
                ret = -2;
        }

        req += pdc; len -= pdc;
    }

out:
    return ret;
}

static int
_get_properties(struct mls_el_ctx *ctx,
    struct mls_eoj_code *seojc, struct mls_eoj_code *deojc,
    unsigned char opc,
    unsigned char *req, unsigned int len,
    unsigned char *res, unsigned int *reslen)
{
    int ret = 0;
    struct mls_node* node;
    struct mls_eoj *eoj;
    int i;
    unsigned char tmplen = UCHAR_MAX;
    unsigned char tmp[UCHAR_MAX];

    node = mls_el_get_node(ctx) ;
    eoj = mls_el_node_get_device(node, deojc);
    if (NULL == eoj) {
        ret = -1; /* not found deoj */
        goto out;
    }

    for (i = 0; i < opc; i++) {
        int ret_prop;
        unsigned char epc, pdc;
        struct mls_epr* epr;

        if (len < 2) {
            ret = -1; /* invalid packet */
            goto out;
        }
        epc = req[0];
        pdc = req[1];
        req += 2; len -= 2;
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
            res += 2; *reslen -= 2;
            if (0 < ret_prop) {
                memcpy(res, tmp, ret_prop);
                res += ret_prop; *reslen -= ret_prop;
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
    struct mls_eoj_code *seojc, struct mls_eoj_code *deojc,
    unsigned char opc,
    unsigned char *req, unsigned int len,
    unsigned char *res, unsigned int *reslen)
{
    /* XXXXXX not impl */
    return -1;
}

static int
_inf_properties(struct mls_el_ctx *ctx,
    struct mls_eoj_code *seojc, struct mls_eoj_code *deojc,
    unsigned char opc,
    unsigned char *req, unsigned int len,
    unsigned char *res, unsigned int *reslen)
{
    int ret = 0;
    struct mls_node* node;
    struct mls_eoj *eoj;
    int i;

    node = mls_el_get_node(ctx) ;
    eoj = mls_el_node_get_device(node, deojc);
    if (NULL == eoj) {
        ret = -1; /* not found deoj */
        goto out;
    }

    for (i = 0; i < opc; i++) {
        unsigned char epc, pdc;

        if (len < 2) {
            ret = -1; /* invalid packet */
            goto out;
        }
        epc = req[0];
        pdc = req[1];
        req += 2; len -= 2;
        if (len < pdc) {
            ret = -1; /* invalid packet */
            goto out;
        }

        /* XXXXXX
           仮想ノード(proxy)機能をサポートしないと意味がない
           とりあえず、処理した事だけを返す。
         */
        {
            res[0] = epc;
            res[1] = 0; /* pdc */
            res += 2; *reslen -= 2;
        }

        req += pdc; len -= pdc;
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
    unsigned char* req, unsigned int len, 
    unsigned char* res, unsigned int *reslen)
{
    int ret = 0;
    unsigned short tid;
    struct mls_eoj_code seoj, deoj;
    unsigned char esv, opc;
    unsigned char* org_res = res;
    unsigned int org_reslen = *reslen;

    /* check length */
    if (len < MLS_ELNET_PACKET_BASE_LENGTH) {
        /* ignore message XXXX error log */
        ret = -1;
        goto out;
    }

    /* XXXX struct mls_elnet_frame_base で書き換え */
    /* header */
    ret = _check_header(req, &len);
    if (ret < 0) {
        /* ignore message XXXX error log */
        ret = -1;
        goto out;
    }
    req += ret;

    /* transaction id */
    tid = mls_type_get_short(req, len);
    /* seoj */
    req += mls_eoj_get_eojcode(&seoj, req, &len);
    /* deoj */
    req += mls_eoj_get_eojcode(&deoj, req, &len);
    /* esv */
    esv = (unsigned char)mls_type_get_char(req, len);
    req += 1;
    /* opc */
    opc = (unsigned char)mls_type_get_char(req, len);
    req += 1;

    /* *********************************************** */
    res += MLS_ELNET_PACKET_BASE_LENGTH;
    *reslen -= MLS_ELNET_PACKET_BASE_LENGTH;
    switch(esv) {
    default:
        /* ignore message XXXX error log */
        ret = -1;
        break;

    /* Request(0x6X) */
    case MLS_ELNET_ESV_SetI:
    case MLS_ELNET_ESV_SetC:
        ret = _set_properties(ctx, &seoj, &deoj, opc, req, len, res, reslen);
        break;
    case MLS_ELNET_ESV_Get:
    case MLS_ELNET_ESV_INF_REQ:
        ret = _get_properties(ctx, &seoj, &deoj, opc, req, len, res, reslen);
        break;
    case MLS_ELNET_ESV_SetGet:
        ret = _setget_properties(ctx, &seoj, &deoj, opc, req, len, res, reslen);
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
        ret = _inf_properties(ctx, &seoj, &deoj, opc, req, len, res, reslen);
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
    if ((MLS_ELNET_ESV_SetI == esv) && (ret == 0)) {
        ret = -1;
        goto out;
    }
    /* set packet */
    esv = _select_response_esv(esv, ret);
    mls_elnet_set_packet_base(tid, &deoj, &seoj, esv, opc, org_res, NULL);
    *reslen = org_reslen - *reslen;
    ret = 0; /* ok, send message -- when ret is -2 */

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
    unsigned int tmplen = UCHAR_MAX;
    unsigned char tmp[UCHAR_MAX];
    unsigned char *buf = tmp;
    buf = mls_elnet_set_packet_base(_get_next_tid(),
        deojc, &(node->prof->code),
        MLS_ELNET_ESV_INF, 1, buf, &tmplen);
    buf[0] = epc;
    buf[1] = pdc;
    memcpy(&(buf[2]), data, pdc);
    buf += (2 + pdc);

    {
        int ret;
        ret = sendto(elnet->srv->sock, tmp, (buf - tmp), 0,
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
    unsigned char* buf = _req;
    unsigned int len = MLS_ELNET_PACKET_LENGTH;

    /* packet base */
    buf =
        mls_elnet_set_packet_base(_get_next_tid(),
            &(profile->code), &(profile->code),
            MLS_ELNET_ESV_INF, 1, buf, &len);
    /* property */
    buf += _set_property(buf, &len, 
        profile, 
        mls_eoj_get_property(profile, MLS_EL_EPC_INSTANCE_LIST_NOTIFICATION));

    {
        int ret;
        ret = sendto(elnet->srv->sock, buf, (buf - _req), 0,
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
    unsigned int len = MLS_ELNET_PACKET_LENGTH;
    unsigned char* res = _res;
    unsigned int reslen = MLS_ELNET_PACKET_LENGTH;

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

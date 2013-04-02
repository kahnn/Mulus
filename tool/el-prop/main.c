/*
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <strings.h>
#include <assert.h>
#include <errno.h>

#include "mls_type.h"
#include "mls_elnet.h"
#include "mls_el.h"

#define errlog(fmt, ...) do{                   \
        fprintf(stderr, fmt, ##__VA_ARGS__);      \
  }while(0)
#define showlog(fmt, ...) do{                   \
        fprintf(stdout, fmt, ##__VA_ARGS__);      \
  }while(0)

#define _PDUMP_DIR "/psm/logs/pktdump"
#define _USE_UCAST_CONTEXT

/* epc [data]
 */
struct _prop_arg {
    unsigned char epc;
    char *data;
};

/**********************************************************************/
static char *_remote_host;
static struct mls_eoj_code _eoj_code;
static int _is_pdump_file = 0; /* default: output to file. */
static char* _ifname = "eth0";

static struct _prop_arg _set_prop[UCHAR_MAX];
unsigned char _set_prop_len;
static struct _prop_arg _get_prop[UCHAR_MAX];
unsigned char _get_prop_len;

static unsigned char _esv_code;

struct mls_net_mcast_ctx* _cln_ctx = NULL;
static unsigned char _req[MLS_ELNET_FRAME_LENGTH_MAX];
static unsigned char _res[MLS_ELNET_FRAME_LENGTH_MAX];

/**********************************************************************/

static void
usage(char *name)
{
    errlog("Usage: %s [-i interface-name] -h remote-host -d destination-eoj \n", name);
    errlog("             [-s epc value] ... [-g epc] ...\n");
    errlog("       interface-name: specified interface-name, default eth0.\n");
    errlog("       remote-host: ip-address:port\n");
    errlog("       destination-eoj: cgc clc inc, Ex.) 0EF001 \n");
    errlog("Result: \n");
    errlog("        {S|G},epc,pdc[,edt] ....\n");
    errlog("    Ex.)\n");
    errlog("        S,80,0,\n");
    errlog("        G,80,1,31\n");
    exit(-1);
}

static unsigned char
parse_uchar(char *data)
{
    char token[2+1];
    token[0] = data[0];
    token[1] = data[1];
    token[2] = '\0';

    return (unsigned char)strtoul(token, NULL, 16);
}

static int
parse_rawdata(char *data, unsigned char *rawdata, unsigned char* rawdata_len)
{
    int i, len = strlen(data);
    char *str = data;

    *rawdata_len = 0;
    for (i = 0, str = data; i < len; i += 2, str += 2) {
        *rawdata = parse_uchar(str);
        rawdata += 1;
        *rawdata_len += 1;
    }

    return (int)*rawdata_len;
}

static unsigned char
_select_esv(void)
{
    unsigned char esv_code;
    if (0 == _set_prop_len) {
        if (0 == _get_prop_len) {
            assert(0);
        } else {
            esv_code = MLS_ELNET_ESV_Get;
        }
    } else {
        if (0 == _get_prop_len) {
            esv_code = MLS_ELNET_ESV_SetC;
        } else {
            esv_code = MLS_ELNET_ESV_SetGet;
        }
    }
     
    return esv_code;
}

static int
parse_args(int argc, char* argv[])
{
    int opt, ret = 0;
    while ((opt = getopt(argc, argv, "ni:h:d:s:g:")) != -1) {
        switch (opt) {
        case 'i':
            _ifname = optarg;
            break;
        case 'n':
            _is_pdump_file = 0; /* output to stderr */
            break;
        case 'h':
            _remote_host = optarg;
            break;
        case 'd':
            _eoj_code.cgc = parse_uchar(optarg);
            _eoj_code.clc = parse_uchar(optarg + 2);
            _eoj_code.inc = parse_uchar(optarg + 4);
            break;
        case 'g':
            _get_prop[_get_prop_len].epc = (unsigned char)strtoul(optarg, NULL, 16);
            _get_prop_len++;
            break;
        case 's':
            _set_prop[_set_prop_len].epc = (unsigned char)strtoul(optarg, NULL, 16);
            _set_prop[_set_prop_len].data = argv[optind++];
            _set_prop_len++;
            break;
        default: /* '?' */ 
            usage(argv[0]);
            break;
        }
    }
    if (!_remote_host || (0 == _get_prop_len && 0 == _set_prop_len))
    {
        usage(argv[0]);
    }

    _esv_code = _select_esv();

#if 0
    {
        int i;
        errlog("%s,%s,%x,%x,%x\n",
               _ifname, _remote_host, 
               _eoj_code.cgc, _eoj_code.clc, _eoj_code.inc);
        for (i = 0; i < _set_prop_len; i++) {
            errlog("SET:%d: %x=%s\n", i, _set_prop[i].epc, _set_prop[i].data);
        }
        for (i = 0; i < _get_prop_len; i++) {
            errlog("GET:%d: %x\n", i, _get_prop[i].epc);
        }
        exit(0);
    }
#endif
    return ret;
}

static int
open_sock(char *ifname)
{
    int ret = 0;
    char ifaddr[256];

    ret = mls_net_getaddr_by_ifname(ifname, AF_INET, 
        ifaddr, sizeof(ifaddr));
    if (0 != ret) {
        errlog("mls_net_getaddr_by_ifname(%d) error.\n", ret);
        goto out;
    }
#if 0
    errlog("ifaddr => %s\n", ifaddr);
#endif

#ifdef _USE_UCAST_CONTEXT
    _cln_ctx = (struct mls_net_mcast_ctx*)mls_net_ucast_open_ctx();
    if (NULL == _cln_ctx) {
        errlog("mls_net_ucast_cln_open() error.\n");
        ret = -1;
        goto out;
    }
#else  /* _USE_UCAST_CONTEXT */
    _cln_ctx = 
        mls_net_mcast_open_ctx(MLS_ELNET_MCAST_ADDRESS,
            MLS_ELNET_MCAST_PORT, ifaddr);
    if (NULL == _cln_ctx) {
        errlog("mls_net_mcast_cln_open() error.\n");
        ret = -1;
        goto out;
    }
#endif  /* _USE_UCAST_CONTEXT */

    mls_elnet_set_pdump(_is_pdump_file, _PDUMP_DIR);

out:
    return ret;
}

static int
parse_addr(char *host, char **addr, char **port)
{
    int ret = 0;
    char *del;

    if (NULL == (del = rindex(host, ':'))) {
        errlog("inavlid host format:%s\n", host);
        ret = -1;
        goto out;
    }

    /* get port-number */
    *port = (del + 1);
    *del = '\0';

    /* get hostname -or- ip-address */
    if ('[' == *host) {
        /* for ipv6 [] style ==> ex.) [2001:db8::1]:80 */
        *addr = (host + 1);
        *(del - 1) = '\0';
    } else {
        *addr = host;
    }

  out:
    return ret;
}

#define ENTER_ASSEMBLE_FRAME(r) int r = 0
#define FINALLY_ASSEMBLE_FRAME()  out:
#define EXIT_ASSEMBLE_FRAME(r)  return r
#define ASSEMBLE_FRAME(dp, dl, sp, sl, r)         \
do {                                              \
    if (dl < sl) {                                \
        r = -ENOMEM;                              \
        goto out;                                 \
    }                                             \
    memcpy(dp, sp, sl);                           \
    dp += sl; dl -= sl;                           \
} while(0)

/*
 * data -> OPC
 */
static int
_setup_frame_data(unsigned char *data, int dlen,
                  struct _prop_arg *setp, unsigned char slen,
                  struct _prop_arg *getp, unsigned char glen)
{
    int i, org_len = dlen;
    unsigned char *dp = data; 
    unsigned char rawdata[UCHAR_MAX], rawdata_len;
    ENTER_ASSEMBLE_FRAME(ret);

    /*
     * Set properties.
     */
    if (NULL != setp && 0 < slen) {
        /* OPC - Set */
        ASSEMBLE_FRAME(dp, dlen, &slen, sizeof(slen), ret);
        for (i = 0; i < slen; i++, setp++) {
            parse_rawdata(setp->data, rawdata, &rawdata_len);
            ASSEMBLE_FRAME(dp, dlen, &(setp->epc), sizeof(setp->epc), ret);
            ASSEMBLE_FRAME(dp, dlen, &rawdata_len, sizeof(rawdata_len), ret);
            ASSEMBLE_FRAME(dp, dlen, rawdata, rawdata_len, ret);
        }
    }

    /*
     * Get properties.
     */
    if (NULL != getp && 0 < glen) {
        /* OPC - Get */
        ASSEMBLE_FRAME(dp, dlen, &glen, sizeof(glen), ret);
        for (i = 0; i < glen; i++, getp++) {
            rawdata_len = 0;
            ASSEMBLE_FRAME(dp, dlen, &getp->epc, sizeof(getp->epc), ret);
            ASSEMBLE_FRAME(dp, dlen, &rawdata_len, sizeof(rawdata_len), ret);
        }
    }

    /* OK */
    ret = org_len - dlen;
    
    FINALLY_ASSEMBLE_FRAME();
    EXIT_ASSEMBLE_FRAME(ret);
}

static int
output_one_prop(char ope, unsigned char *edata)
{
    int i;
    unsigned char pdc;
    unsigned char *dp = edata;

    showlog("%c,", ope); /* Set -or- Get */
    showlog("%02x,", dp[0]); /* EPC */
    pdc = dp[1];
    showlog("%02x,", pdc);      /* PDC */
    dp += 2;
    for (i = 0; i < pdc; i++) {
        showlog("%02x", *dp++);
    }
    showlog("\n");

    return dp - edata;
}

static void
output_data(unsigned char *data)
{
    int i;
    unsigned char opc;
    unsigned char *edata = data;

    /* first byte is opc */
    opc = *edata++;
    for (i = 0; i < opc; i++) {
        /* Set -or- Get */
        edata += output_one_prop(((MLS_ELNET_ESV_Get == _esv_code) ? 'G' : 'S'), edata);
    }

    if (MLS_ELNET_ESV_SetGet != _esv_code) {
        return;
    }

    /* Get data (SetGet service) */
    opc = *edata++;
    for (i = 0; i < opc; i++) {
        /* Get only */
        edata += output_one_prop('G', edata);
    }
}

static int
command(void)
{
    int ret = 0;
    char *addr, *port;
    struct mls_elnet_frame *req = (struct mls_elnet_frame*)_req;
    unsigned int reqlen = sizeof(_req);
    struct mls_elnet_frame *res = (struct mls_elnet_frame*)_res;
    int reslen = sizeof(_res);

    /* target host */
    if (parse_addr(_remote_host, &addr, &port) < 0) {
        ret = -1;
        goto out;
    }

    /* create request */
    {
        int dlen;
        struct mls_eoj_code seoj, deoj;
        seoj.cgc = MLS_EL_CGC_PROFILE;
        seoj.clc = MLS_EL_CLC_NODEPROFILE;
        seoj.inc = 0x01;
        deoj = _eoj_code;

        mls_elnet_setup_frame_header(req, &seoj, &deoj, _esv_code, 1, 1);
        dlen = _setup_frame_data((req->data - 1),
                                 (reqlen - MLS_ELNET_FRAME_HEADER_LENGTH),
                                 _set_prop, _set_prop_len, _get_prop, _get_prop_len);
        if (dlen < 0) {
            errlog("Error _setup_frame_data(%d)\n", dlen);
            ret = dlen;
            goto out;
        }
        reqlen = MLS_ELNET_FRAME_HEADER_LENGTH - 1  + dlen;
    }

    /* RPC */
    reslen = mls_elnet_rpc(_cln_ctx, addr, port, req, reqlen, res, reslen);
    if (reslen < 0) {
        errlog("Error mls_elnet_rpc(%d)\n", reslen);
        ret = -10;
        goto out;
    }

    /*
     * OK! valid response packet.
     */

    /* check error code */
    if (MLS_ELNET_ESV_IS_ERR_RES_GROUP(res->esv)) {
        errlog("Error response code (%02X)\n", res->esv);
        ret = -11;
        goto out;
    }
    /* output result */
    output_data(&(res->opc));
    ret = 0;

out:
    return ret;
}

int
main(int argc, char* argv[])
{
    int ret = 0;

    (void)mls_el_ini();

    if (parse_args(argc, argv) < 0) {
        goto out;
    }

    if ((ret = open_sock(_ifname)) != 0) {
        goto out;
    }

    if ((ret = command()) != 0) {
        goto out;
    }

out:
    if (NULL != _cln_ctx) {
#ifdef _USE_UCAST_CONTEXT
        mls_net_ucast_close_ctx((struct mls_net_ucast_ctx*)_cln_ctx);
#else  /* _USE_UCAST_CONTEXT */
        mls_net_mcast_close_ctx(_cln_ctx);
#endif  /* _USE_UCAST_CONTEXT */
    }

    mls_el_fin();
    return ret;
}

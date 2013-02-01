/*
el_prop  {-s|-g} remote-host cg cc ic epc [value]
-sの場合は valueも必要
-gの場合は valueは不要
remote-host = ipaddr:port

-g の際の出力フォーマット: (epc,edata)
0x82,000042

...
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
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

/**********************************************************************/
int _run_mode; /* -s=1 -or -g=0 */
char *_remote_host;
struct mls_eoj_code _eoj_code;
unsigned char _epc;
char *_epc_val = "dummy";

unsigned char _epc_rawdata[UCHAR_MAX];
unsigned char _epc_rawdata_len;

struct mls_net_mcast_cln* _cln_ctx = NULL;

static unsigned char _req[MLS_ELNET_FRAME_LENGTH_MAX];
static unsigned char _res[MLS_ELNET_FRAME_LENGTH_MAX];

/**********************************************************************/

void
usage(char *name)
{
    errlog("Usage: %s {-s|-g} remote-host cgc clc inc epc [value]\n", name);
    exit(-1);
}

static int
parse_rawdata(char *data, unsigned char *rawdata, unsigned char* rawdata_len)
{
    int i, len = strlen(data);
    char *str = data;
    char token[2+1];

    *rawdata_len = 0;
    for (i = 0, str = data; i < len; i += 2, str += 2) {
        token[0] = str[0];
        token[1] = str[1];
        token[2] = '\0';

        *rawdata = (unsigned char)strtoul(token, NULL, 16);

        rawdata += 1;
        *rawdata_len += 1;
    }

    return (int)*rawdata_len;
}

static void
output_data(unsigned char epc, unsigned char *datap, unsigned int datalen)
{
    int i;

    showlog("%02x,", epc);
    for (i = 0; i < datalen; i++) {
        showlog("%02x", *(datap + i));
    }
    showlog("\n");
}

static void
parse_args(int argc, char* argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "sg")) != -1) {
        switch (opt) {
        case 'g':
            _run_mode = 0;
            break;
        case 's':
            _run_mode = 1;
            break;
        default: /* '?' */ 
            usage(argv[0]);
            break;
        }
    }
    if (!((_run_mode == 0 && argc == 7) || (_run_mode == 1 && argc == 8))) {
        usage(argv[0]);
    }

    _remote_host = argv[optind++];
    _eoj_code.cgc = (unsigned char)strtoul(argv[optind++], NULL, 16);
    _eoj_code.clc = (unsigned char)strtoul(argv[optind++], NULL, 16);
    _eoj_code.inc = (unsigned char)strtoul(argv[optind++], NULL, 16);
    _epc = (unsigned char)strtoul(argv[optind++], NULL, 16);
    if (_run_mode) {
        _epc_val = argv[optind++];
    }

#if 1
    errlog("%d:%s:%x,%x,%x:%x,%s\n",
        _run_mode, _remote_host, 
        _eoj_code.cgc, _eoj_code.clc, _eoj_code.inc, _epc, _epc_val);
#endif

    if (_run_mode)
        parse_rawdata(_epc_val, _epc_rawdata, &_epc_rawdata_len);
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
    errlog("ifaddr => %s\n", ifaddr);

    _cln_ctx = 
        mls_net_mcast_cln_open(MLS_ELNET_MCAST_ADDRESS,
            MLS_ELNET_MCAST_PORT, ifaddr, "0");
    if (NULL == _cln_ctx) {
        errlog("mls_net_mcast_cln_open() error.\n");
        ret = -1;
        goto out;
    }

out:
    return ret;
}

static int
parse_addr(char *host, char **addr, char **port)
{
    int ret = 0;
    char *saveptr;

    *addr = strtok_r(host, ":", &saveptr);
    *port = strtok_r(NULL, ":", &saveptr);
    if (NULL == *addr || NULL == *port) {
        errlog("inavlid host format:%s\n", host);
        ret = -1;
        goto out;
    }

out:
    return ret;
}

static int
command(void)
{
    int ret = 0;
    char *addr, *port;
    struct mls_elnet_frame *req = (struct mls_elnet_frame*)_req;
    unsigned int reqlen = sizeof(_req);
    struct mls_elnet_frame *res = (struct mls_elnet_frame*)_res;
    unsigned int reslen = sizeof(_res);

    /* target host */
    if (parse_addr(_remote_host, &addr, &port) < 0) {
        ret = -1;
        goto out;
    }

    /* create request */
    {
        struct mls_eoj_code seoj, deoj;
        seoj.cgc = MLS_EL_CGC_PROFILE;
        seoj.clc = MLS_EL_CLC_NODEPROFILE;
        seoj.inc = 0x01;
        deoj = _eoj_code;

        mls_elnet_setup_frame_header(req,
            &seoj, &deoj, 
            ((_run_mode) ? MLS_ELNET_ESV_SetC : MLS_ELNET_ESV_Get),
            1, 1);
        req->data[0] = _epc;
        if (_run_mode) {
            req->data[1] = _epc_rawdata_len;
            memcpy(&(req->data[2]), _epc_rawdata, _epc_rawdata_len);
        } else {
            req->data[1] = 0;
        }
        reqlen = MLS_ELNET_FRAME_HEADER_LENGTH + 2 + req->data[1];
    }

    /* RPC */
    reslen = mls_elnet_rpc(_cln_ctx, addr, port, req, reqlen, res, reslen);
    if (reslen < 0) {
        errlog("Error mls_elnet_rpc(%d)\n", reslen);
        ret = -1;
        goto out;
    }

    /*
     * OK! response valid packet.
     */
    /* output result */
    output_data(_epc,
        ((_run_mode) ? _epc_rawdata : &(res->data[0])),
        ((_run_mode) ? _epc_rawdata_len : (res->data[1])));
    ret = 0;

out:
    return ret;
}

int
main(int argc, char* argv[])
{
    int ret = 0;
    char* ifname = "eth0";

    (void)mls_el_ini();

    parse_args(argc, argv);

    if ((ret = open_sock(ifname)) != 0) {
        goto out;
    }

    if ((ret = command()) != 0) {
        goto out;
    }

out:
    if (NULL != _cln_ctx)
        mls_net_mcast_cln_close(_cln_ctx);

    mls_el_fin();
    return ret;
}

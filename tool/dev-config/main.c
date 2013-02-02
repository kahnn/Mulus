/*
  device_config {-s|-g} cgc clc inc epc [value]
    -sの場合は valueも必要
    -gの場合は valueは不要
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#include "mls_type.h"
#include "mls_elnet.h"
#include "mls_elope.h"
#include "mls_el.h"

#define errlog(fmt, ...) do{                   \
        fprintf(stderr, fmt, ##__VA_ARGS__);      \
  }while(0)
#define showlog(fmt, ...) do{                   \
        fprintf(stdout, fmt, ##__VA_ARGS__);      \
  }while(0)

/**********************************************************************/
int _run_mode; /* -s=1 -or -g=0 */

struct mls_eoj_code _eoj_code;

unsigned char _epc;
char *_epc_val = "dummy";

unsigned char _epc_rawdata[UCHAR_MAX];
unsigned char _epc_rawdata_len;

struct mls_net_ud_cln* _cln_ctx = NULL;

static unsigned char _req[MLS_ELOPE_PACKET_LENGTH_MAX];
static unsigned char _res[MLS_ELOPE_PACKET_LENGTH_MAX];

/**********************************************************************/

void
usage(char *name)
{
    errlog("Usage: %s {-s|-g} cgc clc inc epc [value]\n", name);
    exit(-1);
}

static int
parse_rawdata(char *data, unsigned char *rawdata, unsigned char* rawdata_len)
{
    int i, len = strlen(data);
    char *str = data;
    char token[2];

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
    if (!((_run_mode == 0 && argc == 6) || (_run_mode == 1 && argc == 7))) {
        usage(argv[0]);
    }

    _eoj_code.cgc = (unsigned char)strtoul(argv[optind++], NULL, 16);
    _eoj_code.clc = (unsigned char)strtoul(argv[optind++], NULL, 16);
    _eoj_code.inc = (unsigned char)strtoul(argv[optind++], NULL, 16);
    _epc = (unsigned char)strtoul(argv[optind++], NULL, 16);
    if (_run_mode) {
        _epc_val = argv[optind++];
    }

#if 0
    errlog("%d:%x,%x,%x:%x,%s\n",
        _run_mode, 
        _eoj_code.cgc, _eoj_code.clc, _eoj_code.inc, _epc, _epc_val);
#endif

    if (_run_mode)
        parse_rawdata(_epc_val, _epc_rawdata, &_epc_rawdata_len);
}

static int
open_sock(void)
{
    int ret = 0;

    _cln_ctx = mls_net_udgram_cln_open(MLS_ELOPE_UD_SOCK_NAME);
    if (NULL == _cln_ctx) {
        errlog("mls_net_udgram_cln_open() error.\n");
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
    struct mls_elope_packet *req = (struct mls_elope_packet*)_req;
    struct mls_elope_packet *res = (struct mls_elope_packet*)_res;

    /*
     * request packet
     */
    req->rc = 0;
    req->deojc = _eoj_code;
    req->epc = _epc;
    if (_run_mode) {
        req->svc = MLS_ELOPE_SVC_SET_REQ;
        req->pdc = _epc_rawdata_len;
        memcpy(req->data, _epc_rawdata, _epc_rawdata_len);
    } else {
        req->svc = MLS_ELOPE_SVC_GET_REQ;
        req->pdc = 0;
    }

    /*
     * rpc
     */
    ret = mls_elope_rpc(_cln_ctx, req, res, sizeof(_res));
    if (ret < 0) {
        errlog("mls_elope_rpc() error.\n");
        goto out;
    }

    /*
     * response packet check
     */
    if (!((_run_mode && res->svc == MLS_ELOPE_SVC_SET_RES) || 
          ((!_run_mode) && res->svc == MLS_ELOPE_SVC_GET_RES)))
    {
        errlog("Invalid packet (%d,%d)\n", _run_mode, (int)res->svc);
        ret = -1;
        goto out;
    }
    if (MLS_ELOPE_RCODE_SUCCESS != res->rc) {
       errlog("Error response (%d)\n", (int)res->rc);
        ret = -1;
        goto out;
    }

    /*
     * output result
     */
    output_data(res->epc,
        ((_run_mode) ? _epc_rawdata : res->data),
        ((_run_mode) ? _epc_rawdata_len : res->pdc));
    ret = 0;

out:
    return ret;
}

int
main(int argc, char* argv[])
{
    int ret = 0;

    (void)mls_el_ini();

    parse_args(argc, argv);

    if ((ret = open_sock()) != 0) {
        goto out;
    }

    if ((ret = command()) != 0) {
        goto out;
    }

out:
    if (NULL != _cln_ctx)
        mls_net_udgram_cln_close(_cln_ctx);

    mls_el_fin();
    return ret;
}

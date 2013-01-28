/*
el_prop  {-s|-g} remote-host cg cc ic epc [value]
-sの場合は valueも必要
-gの場合は valueは不要
remote-host = ipaddr:port

-g の際の出力フォーマット: ('|'でbyte単位で区切る)
0x82,0x00|0x00|0x42

...
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#include "mls_type.h"
#include "mls_elnet.h"

/**********************************************************************/
int _run_mode; /* -s=1 -or -g=0 */
char *_remote_host;
unsigned char _cgc;
unsigned char _clc;
unsigned char _inc;
unsigned char _epc;
char *_epc_val = "dummy";
unsigned char _epc_rawdata[UCHAR_MAX];
unsigned char _epc_rawdata_len;
struct mls_net_mcast_cln* _cln_ctx = NULL;
/**********************************************************************/

void
usage(char *name)
{
    fprintf(stderr, "Usage: %s {-s|-g} remote-host cgc clc inc epc [value]\n", name);
    exit(-1);
}

static int
parse_rawdata(char *data, unsigned char *rawdata, unsigned char* rawdata_len)
{
    int i;
    char *str, *token, *saveptr;

    *rawdata_len = 0;
    for (i = 0, str = data; ; i++, str = NULL) {
        token = strtok_r(str, "|", &saveptr);
        if (token == NULL)
            break;
        *rawdata = (unsigned char)strtoul(token, NULL, 0);

        rawdata += 1;
        *rawdata_len += 1;
    }

    return (int)*rawdata_len;
}

static void
output_data(unsigned char epc, unsigned char *datap, unsigned int datalen)
{
    int i;

    fprintf(stdout, "0x%x,", epc);
    for (i = 0; i < datalen; i++) {
        fprintf(stdout, "0x%x", *(datap + i));
        if (datalen != (i + 1)) {
            fprintf(stdout, "|");
        }
    }
    fprintf(stdout, "\n");
}

static void
parse_args(int argc, char* argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "sg:")) != -1) {
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
    _cgc = (unsigned char)strtoul(argv[optind++], NULL, 0);
    _clc = (unsigned char)strtoul(argv[optind++], NULL, 0);
    _inc = (unsigned char)strtoul(argv[optind++], NULL, 0);
    _epc = (unsigned char)strtoul(argv[optind++], NULL, 0);
    if (_run_mode) {
        _epc_val = argv[optind++];
    }

#if 1
    fprintf(stderr, "%d:%s:%x,%x,%x:%x,%s\n",
        _run_mode, _remote_host, _cgc, _clc, _inc, _epc, _epc_val);
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
        fprintf(stderr, "mls_net_getaddr_by_ifname(%d) error.\n", ret);
        goto out;
    }
    fprintf(stdout, "ifaddr => %s\n", ifaddr);

    _cln_ctx = 
        mls_net_mcast_cln_open(MLS_ELNET_MCAST_ADDRESS,
            MLS_ELNET_MCAST_PORT, ifaddr, "0");
    if (NULL == _cln_ctx) {
        fprintf(stderr, "mls_net_mcast_cln_open() error.\n");
        ret = -1;
        goto out;
    }

out:
    return ret;
}

static int
parse_addr(char *host, struct sockaddr_storage *to, socklen_t *tolen)
{
    int ret = 0;
    char *addr, *port, *saveptr;

    addr = strtok_r(host, ":", &saveptr);
    port = strtok_r(NULL, ":", &saveptr);
    if (NULL == addr || NULL == port) {
        fprintf(stderr, "inavlid host format:%s\n", host);
        ret = -1;
        goto out;
    }

    if ((ret = mls_net_get_sockaddr_info(addr, port, to, tolen)) < 0) {
        fprintf(stderr, "mls_net_get_sockaddr_info():%s\n", strerror(errno));
        goto out;
    }

out:
    return ret;
}

static unsigned char _req[MLS_ELNET_PACKET_LENGTH];
static unsigned char _res[MLS_ELNET_PACKET_LENGTH];

static int
command(void)
{
    int ret = 0;
    struct mls_eoj_code seoj, deoj;
    unsigned char esv;
    unsigned short tid = 5963;

    struct sockaddr_storage to;
    socklen_t tolen;
    unsigned char *req = _req;
    unsigned int reqlen = sizeof(_req);
    unsigned char *res_datap, res_datalen;
    int i, retry = 3, wait_sec = 2;

    if (parse_addr(_remote_host, &to, &tolen) < 0) {
        fprintf(stderr, "mls_net_get_sockaddr_info():%s\n", strerror(errno));
        ret = -1;
        goto out;
    }

    /* create request */
    seoj.cgc = MLS_EL_CGC_PROFILE;
    seoj.clc = MLS_EL_CLC_NODEPROFILE;
    seoj.inc = 0x01;
    deoj.cgc = _cgc;
    deoj.clc = _clc;
    deoj.inc = _inc;
    esv = (_run_mode) ? MLS_ELNET_ESV_SetC : MLS_ELNET_ESV_Get;

    req = mls_elnet_set_packet_base(tid, 
        &seoj, &deoj, esv,
        1, /* XXXX OPC: GETでどうしよう？ 最大数を指定する？ */
        req, &reqlen);
    req[0] = _epc;
    if (_run_mode) {
        req[1] = _epc_rawdata_len;
        memcpy(&(req[2]), _epc_rawdata, _epc_rawdata_len);
        req += (2 + _epc_rawdata_len);
    } else {
        req[1] = 0;
        req += 2;
    }

    for (i = 0; i < retry; i++) {
        fd_set mask, ready;
        int width;
        struct timeval timeout;

        /* send request */
        ret = sendto(_cln_ctx->sock, _req, (req - _req), 0,
            (struct sockaddr*)&to, tolen);
        if (-1 == ret) {
            /* XXXX error log */
            perror("sendto");
            goto out;
        }

        FD_ZERO(&mask);
        FD_SET(_cln_ctx->sock, &mask);
        width = _cln_ctx->sock + 1;
        ready = mask;
        timeout.tv_sec = wait_sec;
        timeout.tv_usec = 0;

        switch (select(width, (fd_set*)&ready, NULL, NULL, &timeout)) {
        case -1:
            perror("select");
            goto out;
        case 0:
            /* timeout, retry */
            break;
        default:
            if (FD_ISSET(_cln_ctx->sock, &ready)) {
                struct sockaddr_storage from;
                socklen_t fromlen;
                unsigned char *res = _res;
                unsigned short res_tid;
                unsigned char res_esv, res_opc, res_epc, res_pdc;

                /* recv response */
                fromlen = sizeof(from);
                if ((ret = recvfrom(_cln_ctx->sock, res, sizeof(_res), 0,
                            (struct sockaddr*)&from, &fromlen)) == -1)
                {
                    /* XXXX error log */
                    perror("recvfrom");
                    goto out;
                }

                /*
                 * parse response
                 */
                /* XXXX mls_elnet での受信処理と合わせて整理する */
                if (ret < MLS_ELNET_PACKET_BASE_LENGTH) {
                    /* ignore message retry, XXXX error log */
                    i--;
                    continue;
                }
                /* skip header */
                res += 2;
                /* transaction id */
                res_tid = mls_type_get_short(res, 2);
                res += 2;
                if (tid != res_tid) {
                    /* ignore message retry, XXXX error log */
                    i--;
                    continue;
                }
                /* skip eoj*2 */
                /* XXX check seoj */
                res += 6;
                /* esv */
                res_esv = (unsigned char)mls_type_get_char(res, 1);
                res += 1;
                if (!((MLS_ELNET_ESV_Get == esv
                        && MLS_ELNET_ESV_Get_Res == res_esv)
                        ||
                      (MLS_ELNET_ESV_SetC == esv
                        && MLS_ELNET_ESV_Set_Res == res_esv)))
                {
                    /* ignore message retry, XXXX error log */
                    i--;
                    continue;
                }
                /* opc */
                res_opc = (unsigned char)mls_type_get_char(res, 1);
                res += 1;
                if (1 != res_opc) {
                    /* ignore message retry, XXXX error log */
                    i--;
                    continue;
                }
                /* epc */
                res_epc = (unsigned char)mls_type_get_char(res, 1);
                res += 1;
                if (_epc != res_epc) {
                    /* ignore message retry, XXXX error log */
                    i--;
                    continue;
                }
                /* pdc */
                res_pdc = (unsigned char)mls_type_get_char(res, 1);
                res += 1;
                if (!((MLS_ELNET_ESV_Get == esv && res_pdc != 0)
                        ||
                      (MLS_ELNET_ESV_SetC == esv && res_pdc == 0)))
                {
                    /* ignore message retry, XXXX error log */
                    i--;
                    continue;
                }
                /* edt */
                res_datap = res;
                res_datalen = res_pdc;

                /*
                 * OK! response valid packet.
                 */
                /* output result */
                output_data(res_epc,
                    ((_run_mode) ? _epc_rawdata : res_datap),
                    ((_run_mode) ? _epc_rawdata_len : res_datalen));
                ret = 0;
                goto out;
            }
            break;
        }
    }
    ret = -1;

out:
    return ret;
}

int
main(int argc, char* argv[])
{
    int ret = 0;
    char* ifname = "eth0";

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
    return ret;
}

/*
  remote-host,cgc,clc,inc
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#include "mls_type.h"
#include "mls_elnet.h"

/**********************************************************************/
struct mls_eoj_code _cond_code;
struct mls_net_mcast_cln* _cln_ctx = NULL;
/**********************************************************************/

void
usage(char *name)
{
    fprintf(stderr, "Usage: %s cgc clc inc\n", name);
    exit(-1);
}

static void
parse_args(int argc, char* argv[])
{
    if (argc != 4) {
        usage(argv[0]);
    }

    _cond_code.cgc = (unsigned char)strtoul(argv[1], NULL, 0);
    _cond_code.clc = (unsigned char)strtoul(argv[2], NULL, 0);
    _cond_code.inc = (unsigned char)strtoul(argv[3], NULL, 0);

#if 1
    fprintf(stderr, "%x,%x,%x\n",
        _cond_code.cgc, _cond_code.clc, _cond_code.inc);
#endif
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
print_host(struct sockaddr_storage *from, socklen_t fromlen, 
    struct mls_eoj_code *cond)
{
    int ret = 0;
    char nbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

    if ((ret = getnameinfo((struct sockaddr *)from, fromlen,
                nbuf, sizeof(nbuf), sbuf, sizeof(sbuf),
                NI_NUMERICHOST | NI_NUMERICSERV)) != 0)
    {
        fprintf(stderr,
            "getnameinfo():%s\n", gai_strerror(ret));
        goto out;
    }
    fprintf(stdout,
        "%s:%s,0x%x,0x%x,0x%x\n", nbuf, sbuf, cond->cgc, cond->clc, cond->inc);
out:
    return ret;
}

/*
  condで指定されたインスタンスがdataの中に見つかれば、出力して 0 を返す。
  見つからなければ 0以外 を返す。
 */
static int
find_and_print_eoj(struct sockaddr_storage *from, socklen_t fromlen,
    unsigned char *datap, unsigned char datalen,
    struct mls_eoj_code *cond)
{
    int ret = -1; /* not found -or- error */
    int i, nins = (int)*datap;
    unsigned char *cp = (datap + 1);

    for (i = 0; i < nins; i++) {
        if ((cond->cgc == cp[0]) &&
            (cond->clc == cp[1]) &&
            (cond->inc == cp[2]))
        {
            /* print */
            ret = print_host(from, fromlen, cond);
            goto out;
        }
        cp += 3;
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
    struct mls_eoj_code eoj;
    unsigned char esv;
    unsigned short tid = 1192;

    unsigned char *req = _req;
    unsigned int reqlen = sizeof(_req);
    int i, retry = 3, wait_sec = 2;

    /* create request */
    eoj.cgc = MLS_EL_CGC_PROFILE;
    eoj.clc = MLS_EL_CLC_NODEPROFILE;
    eoj.inc = 0x01;
    esv = MLS_ELNET_ESV_INF_REQ;

    req = mls_elnet_set_packet_base(tid, &eoj, &eoj, esv, 1, req, &reqlen);
    req[0] = MLS_EL_EPC_INSTANCE_LIST_NOTIFICATION;
    req[1] = 0;
    req += 2;

    for (i = 0; i < retry; i++) {
        fd_set mask, ready;
        int width;
        struct timeval timeout;

        /* send request */
        ret = sendto(_cln_ctx->sock, _req, (req - _req), 0,
            (struct sockaddr*)&(_cln_ctx->to), _cln_ctx->tolen);
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
                unsigned char *res_datap, res_datalen;

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
                if (MLS_ELNET_ESV_INF != res_esv) {
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
                if (MLS_EL_EPC_INSTANCE_LIST_NOTIFICATION != res_epc) {
                    /* ignore message retry, XXXX error log */
                    i--;
                    continue;
                }
                /* pdc */
                res_pdc = (unsigned char)mls_type_get_char(res, 1);
                res += 1;
                if (res_pdc == 0) {
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
                /* check result */
                {
                    ret = find_and_print_eoj(&from, fromlen,
                        res_datap, res_datalen, &_cond_code);
                    if (ret != 0) {
                        /* ignore message retry, XXXX error log */
                        i--;
                        continue;
                    }
                    ret = 0;
                    goto out;
                }
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

    if ((ret = command()) < 0) {
        goto out;
    }

out:
    if (NULL != _cln_ctx)
        mls_net_mcast_cln_close(_cln_ctx);
    return ret;
}

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
#include "mls_el.h"

#define errlog(fmt, ...) do{                   \
        fprintf(stderr, fmt, ##__VA_ARGS__);      \
  }while(0)
#define showlog(fmt, ...) do{                   \
        fprintf(stdout, fmt, ##__VA_ARGS__);      \
  }while(0)

#define _PDUMP_DIR "/psm/logs/pktdump"
#define _RECV_PACKET_NUM_MAX 5

/**********************************************************************/
struct mls_eoj_code _cond_code;
struct mls_net_mcast_ctx* _cln_ctx = NULL;
char* _ifname = "eth0";
static unsigned char _req[MLS_ELNET_FRAME_LENGTH_MAX];
static unsigned char _res[_RECV_PACKET_NUM_MAX][MLS_ELNET_FRAME_LENGTH_MAX];
static int _is_pdump_file = 1; /* default: output to file. */
/**********************************************************************/

void
usage(char *name)
{
    errlog("Usage: %s [-i interface-name] [-n] cgc clc inc\n", name);
    errlog("       -i: specified interface-name, default eth0.\n");
    errlog("       -n: don't output packet-dump to file.\n");
    exit(-1);
}

static void
parse_args(int argc, char* argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "ni:")) != -1) {
        switch (opt) {
        case 'i':
            _ifname = optarg;
            break;
        case 'n':
            _is_pdump_file = 0; /* output to stderr */
            break;
        default: /* '?' */ 
            usage(argv[0]);
            break;
        }
    }
    if (!(3 == (argc - optind))) {
        usage(argv[0]);
    }

    _cond_code.cgc = (unsigned char)strtoul(argv[optind++], NULL, 16);
    _cond_code.clc = (unsigned char)strtoul(argv[optind++], NULL, 16);
    _cond_code.inc = (unsigned char)strtoul(argv[optind++], NULL, 16);

#if 0
    errlog("%s:%x,%x,%x\n",
        _ifname, _cond_code.cgc, _cond_code.clc, _cond_code.inc);
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
        errlog("mls_net_getaddr_by_ifname(%d) error.\n", ret);
        goto out;
    }
#if 0
    errlog("ifaddr => %s\n", ifaddr);
#endif

    _cln_ctx = 
        mls_net_mcast_open_ctx(MLS_ELNET_MCAST_ADDRESS,
            MLS_ELNET_MCAST_PORT, ifaddr);
    if (NULL == _cln_ctx) {
        errlog("mls_net_mcast_cln_open() error.\n");
        ret = -1;
        goto out;
    }

    mls_elnet_set_pdump(_is_pdump_file, _PDUMP_DIR);

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
        errlog("getnameinfo():%s\n", gai_strerror(ret));
        goto out;
    }
    showlog("%s:%s,%02X,%02X,%02X\n",
        nbuf, sbuf, cond->cgc, cond->clc, cond->inc);
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
    int i, nins = (unsigned int)*datap;
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

static int
command(void)
{
    int ret = 0, i, rpc_reslen, found_num;
    struct mls_elnet_frame *req = (struct mls_elnet_frame*)_req;
    unsigned int reqlen = sizeof(_req);
    struct mls_elnet_infres resl[_RECV_PACKET_NUM_MAX];
    int retry_max = 2, retry = 0;

    /* create request */
    {
        struct mls_eoj_code eoj;
        eoj.cgc = MLS_EL_CGC_PROFILE;
        eoj.clc = MLS_EL_CLC_NODEPROFILE;
        eoj.inc = 0x01;

        mls_elnet_setup_frame_header(req,
            &eoj, &eoj, 
            MLS_ELNET_ESV_INF_REQ,
            1, 1);
        req->data[0] = MLS_EL_EPC_INSTANCE_LIST_NOTIFICATION;
        req->data[1] = 0;
        reqlen = MLS_ELNET_FRAME_HEADER_LENGTH + 2 + req->data[1];
    }
    /* prepare response */
    for (i = 0; i < _RECV_PACKET_NUM_MAX; i++) {
        resl[i].res = (struct mls_elnet_frame*)&(_res[i][0]);
        resl[i].reslen = MLS_ELNET_FRAME_LENGTH_MAX;
    }

    /* RPC */
re_rpcexec:
    rpc_reslen = mls_elnet_infreq(_cln_ctx, req, reqlen, resl, _RECV_PACKET_NUM_MAX);
    if (rpc_reslen < 0) {
        errlog("Error mls_elnet_rpc(%d)\n", rpc_reslen);
        ret = -1;
        goto out;
    }

    found_num = 0;
    for (i = 0; i < rpc_reslen; i++) {
        struct mls_elnet_frame *res = resl[i].res;

        /* epc */
        if (MLS_EL_EPC_INSTANCE_LIST_NOTIFICATION != res->data[0]) {
            /* ignore message retry, error log */
            errlog("ignore message retry(0, %d)\n", res->data[0]);
            continue;
        }
        /* pdc */
        if (res->data[1] == 0) {
            /* ignore message retry, error log */
            errlog("ignore message retry(1, %d)\n", res->data[1]);
            continue;
        }

        /*
         * OK! response valid packet.
         */
        /* check result */
        ret = find_and_print_eoj(&(resl[i].from), resl[i].fromlen,
                                 &(res->data[2]), res->data[1], &_cond_code);
        if (ret == 0) {
            found_num++;
        }
    }
    if (0 < found_num) {
        ret = 0;
    } else {
        if (retry < retry_max) {
            retry++;
            goto re_rpcexec;
        }
        ret = -1;
    }

out:
    return ret;
}

int
main(int argc, char* argv[])
{
    int ret = 0;

    (void)mls_el_ini();

    parse_args(argc, argv);

    if ((ret = open_sock(_ifname)) != 0) {
        goto out;
    }

    if ((ret = command()) < 0) {
        goto out;
    }

out:
    if (NULL != _cln_ctx)
        mls_net_mcast_close_ctx(_cln_ctx);

    mls_el_fin();
    return ret;
}

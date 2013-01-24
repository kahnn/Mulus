#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>

#include "mls_evt.h"
#include "mls_net.h"

#define UD_SOCK_NAME "/tmp/test_ud_dgram"

/* stdin */
static void _callback_1(struct mls_evt* evt, void* tag)
{
    char buf[128];
    fgets(buf, sizeof(buf), stdin);
    if (feof(stdin)) {
        mls_evt_deactivate(evt);
        return;
    }
    fprintf(stdout, "STDIN: %u %s: %s\n",
        (unsigned int)time(NULL), (char*)tag, buf);
}

/* timeout */
static void _callback_2(struct mls_evt* evt, void* tag)
{
    fprintf(stdout, "TIMEOUT: %u %s\n",
        (unsigned int)time(NULL), (char*)tag);
}

/* unix domain datagram */
static void _callback_3(struct mls_evt* evt, void* tag)
{
    struct mls_net_ud_srv *srv = (struct mls_net_ud_srv*)tag;
    struct sockaddr_un from;
    socklen_t from_len;
    int len;
    char msg[64];

    msg[0] = '\0';
    from_len = sizeof(from);
    len = recvfrom(srv->sock, msg, sizeof(msg), 
        0, (struct sockaddr*)&from, &from_len);
    if (len < 0) {
        perror("recvfrom():");
        goto out;
    }
    msg[len] = '\0';
#if 0
    {
        printf("RECVFROM: %ld=%d\n", sizeof(struct sockaddr_un), from_len);
        printf("RECVFROM: %d %s\n", from.sun_family, from.sun_path);
    }
#endif    
    len = sendto(srv->sock, msg, len, 0, 
        (struct sockaddr*)&from, from_len);
    if (len < 0) {
        perror("sendto():");
        goto out;
    }

out:
    fprintf(stdout, "UD DGRAM: %u %s %d: %s: %d\n",
        (unsigned int)time(NULL), srv->addr, srv->sock, msg, len);
}

int
main(int argc, char* argv[])
{
    int ret = 0;
    struct mls_evt* evt;
    struct mls_net_ud_srv *srv;

    evt = mls_evt_ini(10);

    ret = mls_evt_add_handle(evt, MLS_EVT_FD, 0 /* stdin */,
        _callback_1, "stdin fd");
    ret = mls_evt_add_handle(evt, MLS_EVT_TIMEOUT, 0 /* timeout key=0 */,
        _callback_2, "timeout");
    {
        srv = mls_net_udgram_srv_open(UD_SOCK_NAME);
        if (NULL == srv) {
            fprintf(stderr, "ERROR: mls_net_udgram_srv_open()\n");
            goto out;
        }
        ret = mls_evt_add_handle(evt, MLS_EVT_FD, srv->sock,
            _callback_3, srv);
        if (ret < 0) {
            fprintf(stderr, "ERROR: mls_evt_add_handle(%d)\n", ret);
            goto out;
        }
    }

    mls_evt_dispatch(evt);

    {
        mls_net_udgram_srv_close(srv);
    }

    mls_evt_fin(evt);

out:
    return 0;
}

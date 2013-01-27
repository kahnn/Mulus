#include    <stdio.h>
#include    <string.h>
#include    <ctype.h>
#include    <errno.h>

#include "mls_elnet.h"

struct mls_elnet lelnet;

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

    elnet = &lelnet;
    elnet->srv = srv;

out:
    return elnet;
}

void
mls_elnet_term(struct mls_elnet* elnet)
{
    mls_net_mcast_srv_close(elnet->srv);
}

/*
  初期処理で、指定されたノードの内容をアナウンスする。
 */
void
mls_elnet_announce_profile(struct mls_elnet *elnet, struct mls_node *node)
{
    /* XXXX */
}

/*
  通信処理の本体。ここで、RECEIVE,RESPONSE処理をおこなう。
 */
/* Event handler for mls_evt framework */
void
mls_elnet_event_handler(struct mls_evt* evt, void* tag)
{
    /* XXXX */
}



#if 0
void
server_dispatch(struct mls_net_mcast_srv* srv)
{
    struct sockaddr_storage from;
    socklen_t fromlen;
    int sock = srv->sock;
    ssize_t len;
    char buf[128];

    while (1) {
        /* Receive */
        fromlen = sizeof(from);
        len = recvfrom(sock,
            buf, sizeof(buf),
            0,
            (struct sockaddr*)&from, &fromlen);
        if (-1 == len) {
            int back = errno;
            perror("recvfrom");
            if (EAGAIN == back)
                continue;
        }
#if 1 /* check */
        else {
            char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
            getnameinfo((struct sockaddr *) &from, fromlen,
                hbuf, sizeof(hbuf),
                sbuf, sizeof(sbuf),
                NI_NUMERICHOST | NI_NUMERICSERV);
            fprintf(stdout, "recvfrom:%s:%s:len=%d\n", hbuf, sbuf, (int)len);
        }
#endif

        /* procedure ... */
        {
            char *ptr;
            buf[len] = '\0';
            if ((ptr = strpbrk(buf, "\r\n")) != NULL) {
                *ptr = '\0';
            }
        }
        fprintf(stdout, "client> \'%s\'\n", buf);

        strncat(buf, "-> OK\r\n", sizeof(buf));
        len = strlen(buf);

        /* Response */
        len = sendto(sock,
            buf, len,
            0,
            (struct sockaddr*)&from, fromlen);
        if (-1 == len) {
            perror("sendto");
            break;
        }
    }
}
#endif

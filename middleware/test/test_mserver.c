#include    <stdio.h>
#include    <string.h>
#include    <ctype.h>
#include    <errno.h>
#include    "mls_log.h"
#include    "mls_net.h"

void
server_dispatch(struct mls_net_mcast_ctx* srv)
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

int
main(int argc, char *argv[])
{
    struct mls_net_mcast_ctx* srv = NULL;
    char ifaddr[256];

    if (argc <= 3) {
        fprintf(stderr, "%s mc-address mc-port if-address\n", argv[0]);
        fprintf(stderr, "   mc-address: 224.0.23.0\n");
        fprintf(stderr, "   mc-port:    3610\n");
        fprintf(stderr, "   if-address: 0.0.0.0 -or- eth0\n");
        goto out;
    }

#if 0
    mls_net_show_if_all();
#endif

    {
        int ret = 0;
        if (isdigit(*(argv[3]))) {
            strncpy(ifaddr, argv[3], sizeof(ifaddr));
        } else {
            ret = mls_net_getaddr_by_ifname(argv[3], AF_INET, 
                ifaddr, sizeof(ifaddr));
            if (0 != ret) {
                fprintf(stderr, "mls_net_getaddr_by_ifname(%d) error.\n", ret);
                goto out;
            }
        }
        fprintf(stdout, "ifaddr => %s\n", ifaddr);
    }

    srv = mls_net_mcast_open_ctx(argv[1], argv[2], ifaddr);
    if (NULL == srv) {
        fprintf(stderr, "mls_net_mcast_srv_open() error.\n");
        goto out;
    }

    server_dispatch(srv);

out:
    if (NULL != srv)
        mls_net_mcast_close_ctx(srv);

    return 0;
}

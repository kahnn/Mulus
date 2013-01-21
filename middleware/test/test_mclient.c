#include    <stdio.h>
#include    <string.h>
#include    <ctype.h>
#include    "mls_log.h"
#include    "mls_net.h"

void
client_dispatch(struct mls_net_mcast_cln* cln)
{
    int sock = cln->sock;
    fd_set mask;
    int width, is_end = 0;

    FD_ZERO(&mask);
    FD_SET(sock, &mask);
    FD_SET(0, &mask);
    width = sock + 1;

    do {
        fd_set ready;
        struct timeval timeout;
        ready = mask;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        switch (select(width, (fd_set*)&ready, NULL, NULL, &timeout)) {
        case -1:
            perror("select");
            break;
        case 0:
            /* timeout */
            break;
        default:
            if (FD_ISSET(sock, &ready)) {
                struct sockaddr_storage from;
                socklen_t fromlen;
                ssize_t len;
                char buf[128];
                int ret;
                char nbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                fromlen = sizeof(from);
                if ((len = recvfrom(sock,
                            buf, sizeof(buf),
                            0,
                            (struct sockaddr*)&from, &fromlen)) == -1)
                {
                    perror("recvfrom");
                    is_end = 1;
                    break;
                }
                if ((ret = getnameinfo((struct sockaddr *)&from, fromlen,
                            nbuf, sizeof(nbuf),
                            sbuf, sizeof(sbuf),
                            NI_NUMERICHOST | NI_NUMERICSERV)) != 0)
                {
                    fprintf(stderr,
                        "getnameinfo():%s\n", gai_strerror(ret));
                }
                fprintf(stdout,
                    "recvfrom:%s:%s:len(%d)\n", nbuf, sbuf, (int)len);
                buf[len] = '\0';
                fprintf(stdout, "server> %s", buf);
            }
            if (FD_ISSET(0, &ready)) {
                char buf[128];
                ssize_t len;

                fgets(buf, sizeof(buf), stdin);
                if (feof(stdin)) {
                    is_end = 1;
                    break;
                }
                if ((len = sendto(sock,
                            buf, strlen(buf),
                            0,
                            (struct sockaddr *)&(cln->tom),
                            cln->tomlen)) == -1)
                {
                    perror("sendto");
                    is_end = 1;
                    break;
                }
            }
            break;
        }
    } while (!is_end);
}

int
main(int argc, char *argv[])
{
    struct mls_net_mcast_cln* cln = NULL;
    char ifaddr[256];

    if (argc <= 4) {
        fprintf(stderr, "%s mc-address mc-port if-address l-port\n", argv[0]);
        fprintf(stderr, "   mc-address: 224.0.23.0\n");
        fprintf(stderr, "   mc-port:    3610\n");
        fprintf(stderr, "   if-address: 0.0.0.0 -or- eth0\n");
        fprintf(stderr, "   l-port:     0\n");
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

    cln = mls_net_mcast_cln_open(argv[1], argv[2], ifaddr, argv[4]);
    if (NULL == cln) {
        fprintf(stderr, "mls_net_mcast_cln_open() error.\n");
        goto out;
    }

    /* client dispatch */
    client_dispatch(cln);

out:
    if (NULL != cln)
        mls_net_mcast_cln_close(cln);

    return 0;
}

#include    <stdio.h>
#include    <string.h>
#include    <ctype.h>
#include    "mls_log.h"
#include    "mls_net.h"

void
client_dispatch(struct mls_net_ud_cln* cln)
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
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        switch (select(width, (fd_set*)&ready, NULL, NULL, &timeout)) {
        case -1:
            perror("select");
            break;
        case 0:
            /* timeout */
            break;
        default:
            if (FD_ISSET(cln->sock, &ready)) {
                struct sockaddr_storage from;
                socklen_t fromlen;
                ssize_t len;
                char buf[128];

                fromlen = sizeof(from);
                if ((len = recvfrom(cln->sock,
                            buf, sizeof(buf),
                            0,
                            (struct sockaddr*)&from, &fromlen)) == -1)
                {
                    perror("recvfrom");
                    is_end = 1;
                    break;
                }
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
                if ((len = sendto(cln->sock,
                            buf, strlen(buf),
                            0,
                            (struct sockaddr *)&(cln->to),
                            cln->tolen)) == -1)
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
    struct mls_net_ud_cln* cln = NULL;

    if (argc <= 1) {
        fprintf(stderr, "%s address\n", argv[0]);
        fprintf(stderr, "   address: /tmp/test_ud_dgram\n");
        goto out;
    }

    cln = mls_net_udgram_cln_open(argv[1]);
    if (NULL == cln) {
        fprintf(stderr, "mls_net_udgram_cln_open() error.\n");
        goto out;
    }

    /* client dispatch */
    client_dispatch(cln);

out:
    if (NULL != cln)
        mls_net_udgram_cln_close(cln);

    return 0;
}

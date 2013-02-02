/*
 * network utility.
 */
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <ifaddrs.h>

#include "mls_log.h"
#include "mls_net.h"

#if defined(CDEBUG)
#define CDBG(fmt, ...) do{                    \
        fprintf(stderr, fmt, ##__VA_ARGS__);  \
  }while(0)
#else 
#define CDBG(...) do{}while(0)
#endif

#define errlog(fmt, ...) do{                   \
        LOG_ERR(MLS_LOG_DEFAULT_MODULE, fmt, ##__VA_ARGS__);      \
  }while(0)
/* TODO: replace mls_log API */
#define showlog(fmt, ...) do{                  \
        fprintf(stdout, fmt, ##__VA_ARGS__);   \
  }while(0)

/* ######################################################################## */

int
mls_net_get_sockaddr_info(const char *hostnm, const char *portnm,
    struct sockaddr_storage *saddr, socklen_t *saddr_len)
{
    struct addrinfo hints, *res = NULL;
    int ret = 0;
    
    /* Setup hints */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    /* Get Address */
    if ((ret = getaddrinfo(hostnm, portnm, &hints, &res)) != 0) {
        errlog("getaddrinfo():%s\n", gai_strerror(ret));
        goto out;
    }
    /* Check */
    {
        char nbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
        if ((ret = getnameinfo(res->ai_addr, res->ai_addrlen,
                    nbuf, sizeof(nbuf),
                    sbuf, sizeof(sbuf),
                    NI_NUMERICHOST | NI_NUMERICSERV)) != 0)
        {
            errlog("getnameinfo():%s\n", gai_strerror(ret));
            goto out;
        }
        CDBG("addr=%s\n", nbuf);
        CDBG("port=%s\n", sbuf);
    }
    memcpy(saddr, res->ai_addr, res->ai_addrlen);
    *saddr_len = res->ai_addrlen;

out:
    if (NULL != res) {
        freeaddrinfo(res);
    }
    return ret;
}

struct mls_net_mcast_ctx*
mls_net_mcast_open_ctx(const char* maddr, const char *mport, const char *ifaddr)
{
    struct sockaddr_storage tom;
    socklen_t tomlen;
    struct addrinfo *res = NULL;
    int sock = -1;
    struct mls_net_mcast_ctx* ctx = NULL;

    /* Get address information */
    {
        int ret;
        struct addrinfo hints;
        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

        /* Setup hints */
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE;

        ret = getaddrinfo(NULL, mport, &hints, &res);
        if (0 != ret) {
            errlog("getaddrinfo():%s\n", gai_strerror(ret));
            goto out;
        }

        /* Check */
        ret = getnameinfo(res->ai_addr, res->ai_addrlen,
            hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
            NI_NUMERICHOST | NI_NUMERICSERV);
        if (0 != ret) {
            errlog("getnameinfo():%s\n", gai_strerror(ret));
            goto out;
        }
#if 0
        showlog("H=%s, S=%s\n", hbuf, sbuf);
#endif
    }

    /* Create socket */
    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (-1 == sock) {
        errlog("socket():%s\n", strerror(errno));
        goto out;
    }
    {
        int op = 1;

        /* Set option (reuse-addr) */
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op)) == -1)
        {
            errlog("setsockopt():SO_REUSEADDR:%s\n", strerror(errno));
            goto out;
        }
        /* Bind addres */
        if (bind(sock, res->ai_addr, res->ai_addrlen) == -1) {
            errlog("bind():%s\n", strerror(errno));
            goto out;
        }
    }

    /* Set multicast TTL */
    {
        unsigned char op2 = MLS_NET_MULTICAST_TTL;

        if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL,
                &op2, sizeof(op2)) == -1)
        {
            errlog("setsockopt():IP_MULTICAST_TTL:%s\n", strerror(errno));
            goto out;
        }
    }
#if 0
    /* Set multicast interface */
    /* TODO: XXXX use ifaddr */
    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF,
            &((struct sockaddr_in*)res->ai_addr)->sin_addr,
            sizeof(struct in_addr)) == -1)
    {
        errlog("setsockopt():IP_MULTICAST_IF:%s\n", strerror(errno));
        goto out;
    }
    /* Set multicast loopback */
    {
        unsigned char op2 = 1;

        if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP,
                &op2, sizeof(op2)) == -1)
        {
            errlog("setsockopt():IP_MULTICAST_LOOP:%s\n", strerror(errno));
            goto out;
        }
    }
#endif

    /* Join multi-cast group */
    {
        struct ip_mreq  mreq;

        memset(&mreq, 0, sizeof(mreq));
        inet_pton(AF_INET, maddr, &mreq.imr_multiaddr);
        inet_pton(AF_INET, ifaddr, &mreq.imr_interface);
        if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
                &mreq, sizeof(mreq)) == -1)
        {
            errlog("setsockopt():IP_ADD_MEMBERSHIP:%s\n", strerror(errno));
            goto out;
        }
    }

    /* Get multicast-address */
    if (mls_net_get_sockaddr_info(maddr, mport, &tom, &tomlen) == -1) {
        errlog("mls_net_get_sockaddr_info():%s\n", strerror(errno));
        goto out;
    }

    /* Create context */
    ctx = malloc(sizeof(*ctx));
    if (NULL == ctx) {
        errlog("malloc():%s\n", strerror(errno));
        goto out;
    }
    ctx->sock = sock;
    ctx->to = tom;
    ctx->tolen = tomlen;
    strncpy(ctx->maddr, maddr, sizeof(ctx->maddr));
    strncpy(ctx->mport, mport, sizeof(ctx->mport));
    strncpy(ctx->ifaddr, ifaddr, sizeof(ctx->ifaddr));

out:
    if (NULL == ctx) {
        if (-1 != sock) {
            close(sock);
        }
    }
    if (NULL != res) {
        freeaddrinfo(res);
    }
    return ctx;
}

void
mls_net_mcast_close_ctx(struct mls_net_mcast_ctx* ctx)
{
    struct ip_mreq  mreq;

    memset(&mreq, 0, sizeof(mreq));
    inet_pton(AF_INET, ctx->maddr, &mreq.imr_multiaddr);
    inet_pton(AF_INET, ctx->ifaddr, &mreq.imr_interface);
    if (setsockopt(ctx->sock,
            IPPROTO_IP, IP_DROP_MEMBERSHIP,
            &mreq, sizeof(mreq)) == -1)
    {
        errlog("setsockopt():IP_DROP_MEMBERSHIP:%s\n", strerror(errno));
    }
    close(ctx->sock);

    free(ctx);
}

struct mls_net_ud_srv*
mls_net_udgram_srv_open(const char* addr)
{
    struct mls_net_ud_srv *srv = NULL;
    int sock = -1;
    char srv_addr[MLS_NET_LEN_ADDR];
    struct sockaddr_un name;

    /* Create socket */
    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (-1 == sock) {
        errlog("socket():%s\n", strerror(errno));
        goto out;
    }
    /* Create name. */
    strcpy(srv_addr, addr);
    strcat(srv_addr, MLS_NET_UNIX_DOMAIN_SRV_SUFFIX);
    unlink(srv_addr); /* clean up */
    memset(&name, 0, sizeof(name));
    name.sun_family = AF_UNIX;
    strcpy(name.sun_path, srv_addr);

    if (bind(sock, (struct sockaddr*)&name, sizeof(struct sockaddr_un))) {
        errlog("bind():binding name to datagram socket:%s\n", strerror(errno));
        goto out;
    }

    /* Create server-context */
    srv = malloc(sizeof(*srv));
    if (NULL == srv) {
        errlog("malloc():%s\n", strerror(errno));
        goto out;
    }
    srv->sock = sock;
    strncpy(srv->addr, srv_addr, sizeof(srv->addr));

out:
    if (NULL == srv) {
        if (-1 != sock) {
            close(sock);
            unlink(srv_addr);
        }
    }
    return srv;
}

void
mls_net_udgram_srv_close(struct mls_net_ud_srv *srv)
{
    close(srv->sock);
    unlink(srv->addr);
    free(srv);
}

struct mls_net_ud_cln*
mls_net_udgram_cln_open(const char *addr)
{
    struct mls_net_ud_cln *cln = NULL;
    int sock = -1;
    char cln_addr[MLS_NET_LEN_ADDR];
    struct sockaddr_un name;

    /* Create socket */
    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (-1 == sock) {
        errlog("socket():%s\n", strerror(errno));
        goto out;
    }
    /* Create name. */
    strcpy(cln_addr, addr);
    strcat(cln_addr, MLS_NET_UNIX_DOMAIN_CLN_SUFFIX);
    unlink(cln_addr); /* clean up */
    memset(&name, 0, sizeof(name));
    name.sun_family = AF_UNIX;
    strcpy(name.sun_path, cln_addr);

    if (bind(sock, (struct sockaddr*)&name, sizeof(struct sockaddr_un))) {
        errlog("bind():binding name to datagram socket:%s\n", strerror(errno));
        goto out;
    }

    /* Create client-context */
    cln = malloc(sizeof(*cln));
    if (NULL == cln) {
        errlog("malloc():%s\n", strerror(errno));
        goto out;
    }
    cln->sock = sock;
    strncpy(cln->addr, cln_addr, sizeof(cln->addr));
    /* Construct name of socket to send to. */
    memset(&(cln->to), 0, sizeof(cln->to));
    cln->to.sun_family = AF_UNIX;
    strcpy(cln->to.sun_path, addr);
    strcat(cln->to.sun_path, MLS_NET_UNIX_DOMAIN_SRV_SUFFIX);
    cln->tolen = sizeof(struct sockaddr_un);

out:
    if (NULL == cln) {
        if (-1 != sock) {
            close(sock);
            unlink(cln_addr);
        }
    }
    return cln;
}

void
mls_net_udgram_cln_close(struct mls_net_ud_cln *cln)
{
    close(cln->sock);
    unlink(cln->addr);
    free(cln);
}

struct mls_net_ud_srv*
mls_net_ustream_srv_open(const char* addr)
{
    struct mls_net_ud_srv *srv = NULL;
    int sock = -1;
    char srv_addr[MLS_NET_LEN_ADDR];
    struct sockaddr_un name;

    /* Create socket */
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (-1 == sock) {
        errlog("socket():%s\n", strerror(errno));
        goto out;
    }
    /* Create name. */
    strcpy(srv_addr, addr);
    strcat(srv_addr, MLS_NET_UNIX_DOMAIN_SRV_SUFFIX);
    unlink(srv_addr); /* clean up */
    memset(&name, 0, sizeof(name));
    name.sun_family = AF_UNIX;
    strcpy(name.sun_path, srv_addr);

    if (-1 == bind(sock, (struct sockaddr*)&name, sizeof(struct sockaddr_un))) {
        errlog("bind():binding name to datagram socket:%s\n", strerror(errno));
        goto out;
    }

    if (-1 == listen(sock, 5)) {
        errlog("listen():%s\n", strerror(errno));
        goto out;
    }

#if 0
    if ((s = accept(sock, (struct sockaddr *)&from, &fromlen)) == -1) {
        errlog("accept():%s\n", strerror(errno));
        goto out;
    }
#endif

    /* Create server-context */
    srv = malloc(sizeof(*srv));
    if (NULL == srv) {
        errlog("malloc():%s\n", strerror(errno));
        goto out;
    }
    srv->sock = sock;
    strncpy(srv->addr, srv_addr, sizeof(srv->addr));

out:
    if (NULL == srv) {
        if (-1 != sock) {
            close(sock);
            unlink(srv_addr);
        }
    }
    return srv;
}

void
mls_net_ustream_srv_close(struct mls_net_ud_srv *srv)
{
    close(srv->sock);
    unlink(srv->addr);
    free(srv);
}

struct mls_net_ud_cln*
mls_net_ustream_cln_open(const char *addr)
{
    struct mls_net_ud_cln *cln = NULL;
    int sock = -1;
    char svr_addr[MLS_NET_LEN_ADDR];
    struct sockaddr_un name;

    /* Create socket */
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (-1 == sock) {
        errlog("socket():%s\n", strerror(errno));
        goto out;
    }

    /* Create name. */
    strcpy(svr_addr, addr);
    strcat(svr_addr, MLS_NET_UNIX_DOMAIN_SRV_SUFFIX);
    memset(&name, 0, sizeof(name));
    name.sun_family = AF_UNIX;
    strcpy(name.sun_path, svr_addr);

    if (connect(sock, (struct sockaddr *)&name, sizeof(name)) == -1) {
        errlog("connect():%s\n", strerror(errno));
        goto out;
    }

    /* Create client-context */
    cln = malloc(sizeof(*cln));
    if (NULL == cln) {
        errlog("malloc():%s\n", strerror(errno));
        goto out;
    }
    cln->sock = sock;
    strncpy(cln->addr, svr_addr, sizeof(cln->addr));
    /* Construct name of socket to send to. */
    memcpy(&(cln->to), &name, sizeof(cln->to));
    cln->tolen = sizeof(struct sockaddr_un);

out:
    if (NULL == cln) {
        if (-1 != sock) {
            close(sock);
        }
    }
    return cln;
}

void
mls_net_ustream_cln_close(struct mls_net_ud_cln *cln)
{
    close(cln->sock);
    free(cln);
}

int
mls_net_getaddr_by_ifname(char* ifname, int af, char* addr, int addrlen)
{
    struct ifaddrs *ifaddrs, *ifa;
    int ret = 0;
    int sock = -1;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        ret = -errno;
        errlog("socket():%s\n", strerror(errno));
        goto out;
    }

    if (getifaddrs(&ifaddrs) == -1) {
        ret = -errno;
        errlog("getifaddrs():%s\n", strerror(errno));
        goto out;
    }
    ret = -ENOENT;
    for (ifa = ifaddrs; ifa != NULL; ifa = ifa->ifa_next) {
        if (strcmp(ifa->ifa_name, ifname) != 0) {
            continue;
        }
        if (ifa->ifa_addr == NULL) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == af) {
            /* IPv4 */
            if (AF_INET == af) {
                inet_ntop(AF_INET, 
                    &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr,
                    addr, addrlen);
            /* IPv6 */
            } else if (AF_INET6 == af) {
                inet_ntop(AF_INET6, 
                    &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr,
                    addr, addrlen);
            }
            ret = 0; /* OK */
            break;
        }
    }
    freeifaddrs(ifaddrs);

out:
    if (-1 != sock) {
        close(sock);
    }

    return ret;
}

int
mls_net_show_if_by_name(char* name)
{
    char addr_str[256];
    struct ifreq ifreq;
    struct ifaddrs *ifaddrs;
    struct ifaddrs *ifa;
    int i, ret = 0;

    int sock = -1;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        ret = -errno;
        errlog("socket():%s\n", strerror(errno));
        goto out;
    }

    snprintf(ifreq.ifr_name, sizeof(ifreq.ifr_name), "%s", name);
    if (ioctl(sock, SIOCGIFFLAGS, &ifreq) == -1) {
        ret = -errno;
        errlog("ioctl:SIOCGIFFLAGS:%s\n", strerror(errno));
        goto out;
    }
    if (ifreq.ifr_flags & IFF_UP)
        showlog("UP ");
    if (ifreq.ifr_flags & IFF_BROADCAST)
        showlog("BROADCAST ");
    if (ifreq.ifr_flags & IFF_PROMISC)
        showlog("PROMISC ");
    if (ifreq.ifr_flags & IFF_MULTICAST)
        showlog("MULTICAST ");
    if (ifreq.ifr_flags & IFF_LOOPBACK)
        showlog("LOOPBACK ");
    if (ifreq.ifr_flags & IFF_POINTOPOINT)
        showlog("P2P ");
    showlog("\n");

    if (ioctl(sock, SIOCGIFMTU, &ifreq) == -1) {
        errlog("ioctl:SIOCGIFMTU:%s\n", strerror(errno));
        /* skip error */
    } else {
        showlog("mtu=%d\n", ifreq.ifr_mtu);
    }

    if (getifaddrs(&ifaddrs) == -1) {
        ret = -errno;
        errlog("getifaddrs():%s\n", strerror(errno));
        goto out;
    }
    for (i = 0, ifa = ifaddrs; ifa != NULL; ifa = ifa->ifa_next) {
        if (strcmp(ifa->ifa_name, name) != 0) {
            continue;
        }
        if (ifa->ifa_addr == NULL) {
            continue;
        }
        /* IPv4 */
        if (ifa->ifa_addr->sa_family == AF_INET) {
            showlog("addr[%d]=%s\n", 
                i, inet_ntop(AF_INET, 
                    &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr,
                    addr_str,
                    sizeof(addr_str)));
            /* point-point address */
            if (ifa->ifa_ifu.ifu_dstaddr)
                showlog("dstaddr[%d]=%s\n",
                    i, inet_ntop(AF_INET,
                        &((struct sockaddr_in *)ifa->ifa_ifu.ifu_dstaddr)->sin_addr,
                        addr_str,
                        sizeof(addr_str)));
            /* bloadcast address */
            if (ifa->ifa_ifu.ifu_broadaddr)
                showlog("broadaddr[%d]=%s\n",
                    i, inet_ntop(AF_INET,
                        &((struct sockaddr_in *)ifa->ifa_ifu.ifu_broadaddr)->sin_addr,
                        addr_str,
                        sizeof(addr_str)));
            /* netmask */
            showlog("netmask[%d]=%s\n", i, inet_ntop(AF_INET,
                    &((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr,
                    addr_str,
                    sizeof(addr_str)));
        /* IPv6 */
        } else if (ifa->ifa_addr->sa_family == AF_INET6) {
            showlog("addr6[%d]=%s\n",
                i, inet_ntop(AF_INET6,
                    &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr,
                    addr_str,
                    sizeof(addr_str)));
            /* point-point address */
            if (ifa->ifa_ifu.ifu_dstaddr)
                showlog("dstaddr6[%d]=%s\n",
                    i, inet_ntop(AF_INET6,
                        &((struct sockaddr_in6 *)ifa->ifa_ifu.ifu_dstaddr)->sin6_addr,
                        addr_str,
                        sizeof(addr_str)));
            /* netmask */
            showlog("netmask6[%d]=%s\n",
                i, inet_ntop(AF_INET6,
                    &((struct sockaddr_in6 *)ifa->ifa_netmask)->sin6_addr,
                    addr_str, sizeof(addr_str)));
        } else {
            continue;
        }
        i++;
    }
    freeifaddrs(ifaddrs);

    /* MAC address */
    if (ioctl(sock, SIOCGIFHWADDR, &ifreq) == -1) {
        ret = -errno;
        errlog("ioctl:SIOCGIFHWADDR:%s\n", strerror(errno));
        goto out;
    } else {
        uint8_t *p = (uint8_t*)&ifreq.ifr_hwaddr.sa_data;
        showlog("hwaddr=%02X:%02X:%02X:%02X:%02X:%02X\n",
            *p,
            *(p+1),
            *(p+2),
            *(p+3),
            *(p+4),
            *(p+5));
    }

out:
    if (-1 != sock) {
        close(sock);
    }

    return ret;
}

int
mls_net_show_if_all(void)
{
    struct ifconf ifc;
    int i, if_count;
    int ret = 0;
    int sock = -1;

    ifc.ifc_buf = NULL;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        ret = -errno;
        errlog("socket():%s\n", strerror(errno));
        goto out;
    }

    ifc.ifc_len = 0;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
        ret = -errno;
        errlog("ioctl:SIOCGIFCONF:%s\n", strerror(errno));
        goto out;
    }
    showlog("ifcl=%d\n", ifc.ifc_len);

    if ((ifc.ifc_buf = malloc(ifc.ifc_len)) == NULL) {
        ret = -errno;
        errlog("malloc():%s\n", strerror(errno));
        goto out;
    }
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
        ret = -errno;
        errlog("ioctl:SIOCGIFCONF:%s\n", strerror(errno));
        goto out;
    }

    if_count = ifc.ifc_len / sizeof(struct ifreq);
    showlog("if_count=%d\n", if_count);
    showlog("\n");

    for (i = 0; i < if_count; i++) {
        if (ifc.ifc_req[i].ifr_name == NULL) {
            showlog("ifr_name=null\n");
        } else {
            showlog("ifr_name=%s\n", ifc.ifc_req[i].ifr_name);
            if (ifc.ifc_req[i].ifr_addr.sa_family != AF_INET 
                && ifc.ifc_req[i].ifr_addr.sa_family != AF_INET6)
            {
                showlog("not IP\n");
            } else {
                mls_net_show_if_by_name(ifc.ifc_req[i].ifr_name);
            }
        }
        showlog("\n");
    }

out:
    if (NULL != ifc.ifc_buf) {
        free(ifc.ifc_buf);
    }
    if (-1 != sock) {
        close(sock);
    }

    return ret;
}

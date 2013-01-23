/*
 * network utility.
 */
#ifndef _MULUS_NET_H_
#define _MULUS_NET_H_

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MLS_NET_LEN_ADDR 64
#define MLS_NET_LEN_PORT 16
#define MLS_NET_MULTICAST_TTL 16

struct mls_net_mcast_srv {
    int sock;
    char maddr[MLS_NET_LEN_ADDR];
    char mport[MLS_NET_LEN_PORT];
    char ifaddr[MLS_NET_LEN_ADDR];
};

struct mls_net_mcast_cln {
    int sock;
    struct sockaddr_storage tom;
    socklen_t tomlen;
    char maddr[MLS_NET_LEN_ADDR];
    char mport[MLS_NET_LEN_PORT];
    char ifaddr[MLS_NET_LEN_ADDR];
    char lport[MLS_NET_LEN_PORT];
};

/*
 * @args ifaddr Interface-address -or- INADDR_ANY="0.0.0.0"
 * @args lport  Local-port-number -or- ANY="0"
 * @args af  Address-Family AF_INET -or- AF_INET6
 */
extern struct mls_net_mcast_srv* mls_net_mcast_srv_open(const char* maddr, const char *mport, const char *ifaddr);
extern void mls_net_mcast_srv_close(struct mls_net_mcast_srv*);

extern struct mls_net_mcast_cln* mls_net_mcast_cln_open(const char* maddr, const char *mport, const char* ifaddr, const char *lport);
extern void mls_net_mcast_cln_close(struct mls_net_mcast_cln*);

extern int mls_net_getaddr_by_ifname(char* ifname, int af, char* addr, int addrlen);

extern int mls_net_show_if_all(void);
extern int mls_net_show_if_by_name(char* name);

#endif /* _MULUS_NET_H_ */
/*
 * network utility.
 */
#ifndef _MULUS_NET_H_
#define _MULUS_NET_H_

#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>

#define MLS_NET_LEN_ADDR 96
#define MLS_NET_LEN_PORT 16
#define MLS_NET_MULTICAST_TTL 16

struct mls_net_mcast_srv {
    int sock;
    struct sockaddr_storage to;
    socklen_t tolen;
    char maddr[MLS_NET_LEN_ADDR];
    char mport[MLS_NET_LEN_PORT];
    char ifaddr[MLS_NET_LEN_ADDR];
};

struct mls_net_mcast_cln {
    int sock;
    struct sockaddr_storage to;
    socklen_t tolen;
    struct sockaddr_storage from;
    socklen_t fromlen;
    char maddr[MLS_NET_LEN_ADDR];
    char mport[MLS_NET_LEN_PORT];
    char ifaddr[MLS_NET_LEN_ADDR];
    char lport[MLS_NET_LEN_PORT];
};

#define MLS_NET_UNIX_DOMAIN_SRV_SUFFIX "_srv"
#define MLS_NET_UNIX_DOMAIN_CLN_SUFFIX "_cln"

struct mls_net_ud_srv {
    int sock;
    char addr[MLS_NET_LEN_ADDR];
};

struct mls_net_ud_cln {
    int sock;
    struct sockaddr_un to;
    socklen_t tolen;
    char addr[MLS_NET_LEN_ADDR];
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

extern struct mls_net_ud_srv* mls_net_udgram_srv_open(const char*);
extern void mls_net_udgram_srv_close(struct mls_net_ud_srv*);

extern struct mls_net_ud_cln* mls_net_udgram_cln_open(const char*);
extern void mls_net_udgram_cln_close(struct mls_net_ud_cln*);

extern struct mls_net_ud_srv* mls_net_ustream_srv_open(const char*);
extern void mls_net_ustream_srv_close(struct mls_net_ud_srv*);

extern struct mls_net_ud_cln* mls_net_ustream_cln_open(const char*);
extern void mls_net_ustream_cln_close(struct mls_net_ud_cln*);

extern int mls_net_getaddr_by_ifname(char* ifname, int af, char* addr, int addrlen);
extern int mls_net_get_sockaddr_info(const char *hostnm, const char *portnm, struct sockaddr_storage *saddr, socklen_t *saddr_len);

extern int mls_net_show_if_all(void);
extern int mls_net_show_if_by_name(char* name);

#endif /* _MULUS_NET_H_ */

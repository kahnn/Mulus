/*
 * mulus: ECHONET Lite network.
 */
#ifndef _MULUS_ELNET_H_
#define _MULUS_ELNET_H_

#include "mls_net.h"
#include "mls_evt.h"
#include "mls_node.h"

/*
 * udp multicast binding.
 */
#define MLS_ELNET_MCAST_ADDRESS "224.0.23.0"
#define MLS_ELNET_MCAST_PORT    "3610"

/*
 * ESV
 */
#define MLS_ELNET_ESV_SetI       ((unsigned char)0x60) /* don't need response */
#define MLS_ELNET_ESV_SetC       ((unsigned char)0x61)
#define MLS_ELNET_ESV_Get        ((unsigned char)0x62)
#define MLS_ELNET_ESV_INF_REQ    ((unsigned char)0x63)
#define MLS_ELNET_ESV_SetGet     ((unsigned char)0x6E)

#define MLS_ELNET_ESV_Set_Res    ((unsigned char)0x71)
#define MLS_ELNET_ESV_Get_Res    ((unsigned char)0x72)
#define MLS_ELNET_ESV_INF        ((unsigned char)0x73) /* don't need response */
#define MLS_ELNET_ESV_INFC       ((unsigned char)0x74) /* spontaneous */
#define MLS_ELNET_ESV_INFC_Res   ((unsigned char)0x7A) /* INFC response */
#define MLS_ELNET_ESV_SetGet_Res ((unsigned char)0x7E)

#define MLS_ELNET_ESV_SetI_SNA   ((unsigned char)0x50)
#define MLS_ELNET_ESV_SetC_SNA   ((unsigned char)0x51)
#define MLS_ELNET_ESV_Get_SNA    ((unsigned char)0x52)
#define MLS_ELNET_ESV_INF_SNA    ((unsigned char)0x53)
#define MLS_ELNET_ESV_SetGet_SNA ((unsigned char)0x5E)

/*
 * frame.
 */
#define MLS_ELNET_EHD1_ECHONET_LITE ((unsigned char)0x10)
#define MLS_ELNET_EHD2_REGULAR ((unsigned char)0x81)

#define MLS_ELNET_PACKET_LENGTH (32*1024)
/* EHD1(1) + EHD2(1) + TID(2) + SEOJ(3) + DEOJ(3) + ESV(1) + OPC(1) */
#define MLS_ELNET_PACKET_BASE_LENGTH (1 + 1 + 2 + 3 + 3 + 1 + 1)

struct mls_elnet_frame {
    unsigned char ehd1;
    unsigned char ehd2;
    unsigned short tid;
    struct mls_eoj_code seoj;
    struct mls_eoj_code deoj;
    unsigned char esv;
    unsigned char opc;
    unsigned char data[];
};

/*
 * Context.
 */
struct mls_elnet {
    struct mls_net_mcast_srv *srv;
};

extern struct mls_elnet *mls_elnet_init(char *ifname);
extern void mls_elnet_term(struct mls_elnet*);
extern void mls_elnet_event_handler(struct mls_evt*, void*);
extern void mls_elnet_announce_profile(struct mls_elnet*,struct mls_node*);
extern void mls_elnet_announce_property(struct mls_elnet*,struct mls_node*,struct mls_eoj_code*, unsigned char epc, unsigned char pdc, unsigned char *data);

extern unsigned char* mls_elnet_set_packet_base(unsigned short tid,
    struct mls_eoj_code *seoj, struct mls_eoj_code *deoj,
    unsigned char esv, unsigned char opc,
    unsigned char *buf, unsigned int *blen);

#endif /* _MULUS_ELNET_H_ */

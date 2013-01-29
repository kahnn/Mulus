/*
 * mulus: ECHONET Lite operation command.
 */
#ifndef _MULUS_ELOPE_H_
#define _MULUS_ELOPE_H_

#include "mls_net.h"
#include "mls_evt.h"
#include "mls_obj.h"

/*
 * Socket name.
 */
#define MLS_ELOPE_UD_SOCK_NAME "/tmp/.elope_sock_ud_dgram"

/*
 * Packet Frame.
 */
struct mls_elope_packet {
    unsigned char svc;
    unsigned char rc;
    struct mls_eoj_code deojc;
    unsigned char epc;
    unsigned char pdc;
    unsigned char data[];
};
/* SVC(1) + RES_CODE(1) + EOJ(3) + EPC(1) + PDC(1) */
#define MLS_ELOPE_PACKET_HEADER_LENGTH sizeof(struct mls_elope_packet)
/* header + max-data-length */
#define MLS_ELOPE_PACKET_LENGTH_MAX (MLS_ELOPE_PACKET_HEADER_LENGTH + 256)

static inline int
mls_elope_packet_get_length(struct mls_elope_packet *packet) {
    return (sizeof(struct mls_elope_packet) + packet->pdc);
}

/*
 * Service Code
 */
#define MLS_ELOPE_SVC_GET_REQ  ((unsigned char)0x01)
#define MLS_ELOPE_SVC_GET_RES  ((unsigned char)0x02)
#define MLS_ELOPE_SVC_SET_REQ  ((unsigned char)0x11)
#define MLS_ELOPE_SVC_SET_RES  ((unsigned char)0x12)

/*
 * Result code.
 */
#define MLS_ELOPE_RCODE_SUCCESS        ((unsigned char)0x00)
#define MLS_ELOPE_RCODE_NOENT_INSTANCE ((unsigned char)0x01)
#define MLS_ELOPE_RCODE_NOENT_EPC      ((unsigned char)0x02)
#define MLS_ELOPE_RCODE_NOENT_METHOD   ((unsigned char)0x03)
#define MLS_ELOPE_RCODE_INVALID_DATA   ((unsigned char)0x04)

/*
 * Context.
 */
struct mls_elope {
    struct mls_net_ud_srv *srv;
    struct sockaddr_un from;
    socklen_t from_len;
};

extern struct mls_elope *mls_elope_init(void);
extern void mls_elope_term(struct mls_elope*);
extern void mls_elope_event_handler(struct mls_evt*, void*);
extern int mls_elope_rpc(struct mls_net_ud_cln *cln,
    struct mls_elope_packet *req, struct mls_elope_packet *res, int reslen);

#endif /* _MULUS_ELOPE_H_ */

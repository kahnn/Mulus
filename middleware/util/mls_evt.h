/*
 * event handling.
 */
#ifndef _MULUS_EVENT_H_
#define _MULUS_EVENT_H_

#include "mls_dlink.h"

/*
 * Event Handling framework
 */
enum mls_handle_type {
    MLS_EVT_FD,
    MLS_EVT_TIMEOUT,
    MLS_EVT_MAX
};

struct mls_evt {
    int timeout;
    mls_dlink_t dlink_fd; /* struct mls_evt_handle list for fd */
    mls_dlink_t dlink_to; /* struct mls_evt_handle list for timeout */
};

struct mls_evt_handle {
    enum mls_handle_type type;
    int fd;
    void *tag;
    void (*callback)(struct mls_evt*, struct mls_evt_handle*);
    /* ------------------ */
    struct mls_evt_handle dlink;
    struct mls_evt *ctx;
};

extern struct mls_evt* mls_evt_ini(int timeout);
extern void mls_evt_fin(struct mls_evt*);
extern void mls_evt_add_handle(struct mls_evt*, struct mls_evt_handle*);
extern void mls_evt_del_handle(struct mls_evt*, struct mls_evt_handle*);
extern void mls_evt_dispatch(struct mls_evt*);

#endif /* _MULUS_EVENT_H_ */

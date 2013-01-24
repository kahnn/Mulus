/*
 * event handling.
 */
#ifndef _MULUS_EVENT_H_
#define _MULUS_EVENT_H_

#include "mls_dlink.h"

enum mls_handle_type {
    MLS_EVT_FD,
    MLS_EVT_TIMEOUT
};

struct mls_evt {
    int timeout; /* sec */
    int is_active; /* 1=active, 0=de-active(return dispatch) */
    mls_dlink_t dlink_fd; /* struct mls_evt_handle list for fd */
    mls_dlink_t dlink_to; /* struct mls_evt_handle list for timeout */
};

typedef void (*mls_evt_callback_t)(struct mls_evt*,void*);

extern struct mls_evt* mls_evt_ini(int timeout);
extern void mls_evt_fin(struct mls_evt*);
extern int mls_evt_add_handle(struct mls_evt*,
    enum mls_handle_type, int,
    mls_evt_callback_t, void*);
extern int mls_evt_del_handle(struct mls_evt*,
    enum mls_handle_type, int);
extern void mls_evt_timeout(struct mls_evt*, int);
extern void mls_evt_deactivate(struct mls_evt*);
extern void mls_evt_dispatch(struct mls_evt*);

#endif /* _MULUS_EVENT_H_ */

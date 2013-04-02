/*
 * mulus: ECHONET Lite.
 *
 * @see http://www.echonet.gr.jp/spec/spec_v101_lite.htm
 */
#ifndef _MULUS_EL_H_
#define _MULUS_EL_H_

#include "mls_obj.h"
#include "mls_node.h"
#include "mls_elnet.h"
#include "mls_elope.h"

/* time interval (sec.) */
#define MLS_EL_TIMEINTERVAL_SEC (60)

struct mls_el_ctx;

extern int mls_el_ini(void);
extern void mls_el_fin(void);
extern struct mls_el_ctx* mls_el_create_context(struct mls_node*, struct mls_elnet*, mls_evt_callback_t, void*);
extern void mls_el_destroy_context(struct mls_el_ctx*);
extern void mls_el_announce_profile(struct mls_el_ctx*);
extern void mls_el_run_context(struct mls_el_ctx*);

extern struct mls_node* mls_el_get_node(struct mls_el_ctx *ctx);
extern struct mls_elnet* mls_el_get_elnet(struct mls_el_ctx *ctx);
extern struct mls_elope* mls_el_get_elope(struct mls_el_ctx *ctx);
extern void* mls_el_get_tag(struct mls_el_ctx *ctx);

#endif /* _MULUS_EL_H_ */

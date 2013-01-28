#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>

#include "mls_el.h"

struct mls_el_ctx {
    struct mls_node *lnode;
    struct mls_elnet *elnet;

    struct mls_net_ud_srv *csrv;
    mls_evt_callback_t csrvfunc;

    mls_evt_callback_t tinterval;

    struct mls_evt *evt;
    
    void *tag;
};

struct mls_el_ctx lctx; /* XXX */

/* ------------------------------------------------------ */

struct mls_node*
mls_el_get_node(struct mls_el_ctx *ctx) { return ctx->lnode; }
struct mls_elnet*
mls_el_get_elnet(struct mls_el_ctx *ctx) { return ctx->elnet; }
struct mls_net_ud_srv*
mls_el_get_csrv(struct mls_el_ctx *ctx) { return ctx->csrv; }
void*
mls_el_get_tag(struct mls_el_ctx *ctx) { return ctx->tag; }

/* ------------------------------------------------------ */

/*
 * time interval handler.
 */
static void
_timeinterval_handler(struct mls_evt* evt, void* tag)
{
    struct mls_el_ctx *ctx = (struct mls_el_ctx *)tag;
    struct mls_node *node = ctx->lnode;
    mls_dlink_t *work;

#if 0
    fprintf(stderr, "TIMEOUT: %u %s\n",
        (unsigned int)time(NULL), (char*)tag);
#endif
    /* Call global user handler. */
    if (NULL != ctx->tinterval) {
        ctx->tinterval(evt, tag);
    }

    /* Call node profile handler. */
    if (NULL != node->prof->tinterval) {
        node->prof->tinterval(evt, tag);
    }

    /* Call device opjects handler. */
    mls_dlink_loop(&(node->eojs_list), work)
    {
        struct mls_eoj* eoj =
            mls_dlink_container(work, struct mls_eoj, eojs_list);
        if (NULL != eoj->tinterval) {
            eoj->tinterval(evt, tag);
        }
    }
}

static int
_set_event(struct mls_el_ctx *ctx)
{
    struct mls_evt* evt;
    int ret = 0;

    evt = mls_evt_ini(MLS_EL_TIMEINTERVAL_SEC);

    /* time interval */
    ret = mls_evt_add_handle(evt, MLS_EVT_TIMEOUT, 0 /* timeout key=0 */,
        _timeinterval_handler, ctx);
    if (ret < 0) {
        fprintf(stderr, "ERROR: mls_evt_add_handle(%d)\n", ret);
        goto out;
    }
    /* local command */
    ret = mls_evt_add_handle(evt, MLS_EVT_FD, ctx->csrv->sock,
        ctx->csrvfunc, ctx);
    if (ret < 0) {
        fprintf(stderr, "ERROR: mls_evt_add_handle(%d)\n", ret);
        goto out;
    }
    /* EL multicat */
    ret = mls_evt_add_handle(evt, MLS_EVT_FD, ctx->elnet->srv->sock,
        mls_elnet_event_handler, ctx);
    if (ret < 0) {
        fprintf(stderr, "ERROR: mls_evt_add_handle(%d)\n", ret);
        goto out;
    }

    ctx->evt = evt;

out:
    return ret;
}

struct mls_el_ctx*
mls_el_create_context(struct mls_node *local_node, 
    struct mls_elnet *elnet, 
    struct mls_net_ud_srv *csrv,
    mls_evt_callback_t csrvfunc,
    mls_evt_callback_t tinterval,
    void *tag)
{
    struct mls_el_ctx *ctx = NULL;

    lctx.lnode = local_node;
    lctx.elnet = elnet;
    lctx.csrv = csrv;
    lctx.csrvfunc = csrvfunc;
    lctx.tinterval = tinterval;
    lctx.tag = tag;

    if (_set_event(&lctx) < 0) {
        /* XXX error handling */
        goto out;
    }

    ctx = &lctx;

out:
    return ctx;
}

void
mls_el_destroy_context(struct mls_el_ctx* ctx)
{
    mls_evt_fin(ctx->evt);
    /* XXXX other finish */
}

void
mls_el_announce_profile(struct mls_el_ctx* ctx)
{
    mls_elnet_announce_profile(ctx->elnet, ctx->lnode);
}

void
mls_el_run_context(struct mls_el_ctx* ctx)
{
    mls_evt_dispatch(ctx->evt);
    return;
}

/*
初期化：
  create profile
  create device object
  create node
  initialize network
  * initialize local command.

メイン処理：
  announce profile
  event dispatch
  
EVENT HANDLING:
  local command
    get properties.
    set properties.

  ECHONET Lite
    get properties.
    set properties.
    announce.    

  Time interval
    none.

 */
#include <stdio.h>
#include <errno.h>

#include "mls_obj.h"
#include "mls_node.h"
#include "mls_elnet.h"
#include "mls_el.h"

extern struct mls_eoj* mls_el_get_profile(void); /* XXX */
extern struct mls_eoj* mls_el_get_temperature_sensor(void); /* XXX */

#define UD_SOCK_NAME "/tmp/command_ud_dgram"

/**********************************************************************/
/* timeout */
static void
_lcommand_handler(struct mls_evt* evt, void* tag)
{
#if 0
    struct mls_el_ctx *ctx = (struct mls_el_ctx *)tag;
    struct mls_net_ud_srv *csrv = mls_el_get_csrv(ctx);
#endif
    /* XXX local command: これも形式決めて、object定義の中に入れるか？ */
}

/* timeout */
static void
_timeinterval_handler(struct mls_evt* evt, void* tag)
{
    /* XXX timer event: これも形式決めて、object定義(mls_eoj.tinterval_func?)の中に入れるか？ */
#if 0
    struct mls_el_ctx *ctx = (struct mls_el_ctx *)tag;
    fprintf(stdout, "TIMEOUT: %u %s\n",
        (unsigned int)time(NULL), (char*)tag);
#endif
}

int
init(char *ifname, struct mls_el_ctx **ctxpp)
{
    int ret = 0;
    struct mls_eoj *profile, *temp_sensor;
    struct mls_node *node;
    struct mls_elnet *elnet;
    struct mls_net_ud_srv *csrv;
    struct mls_el_ctx *context;

    /* Get Objects */
    profile = mls_el_get_profile(); 
    temp_sensor = mls_el_get_temperature_sensor();
    if (NULL == profile || NULL == temp_sensor) {
        ret = -ENOENT;
        goto out;
    }

    /* Create node */
    node = mls_el_node_create(profile);
    if (NULL == node) {
        ret = -ENOENT;
        goto out;
    }
    mls_el_node_add_device(node, temp_sensor);

    /* Initialize network */
    elnet = mls_elnet_init(ifname);
    if (NULL == elnet) {
        ret = -ENOENT;
        goto out;
    }

    /* Initialize local command. */
    csrv = mls_net_udgram_srv_open(UD_SOCK_NAME);
    if (NULL == csrv) {
        fprintf(stderr, "ERROR: mls_net_udgram_srv_open()\n");
        goto out;
    }

    /* Initialize EL Context. */
    context = 
        mls_el_create_context(node, elnet, 
            csrv, _lcommand_handler, 
            _timeinterval_handler,
            NULL);
    if (NULL == context) {
        fprintf(stderr, "ERROR: mls_el_create_context()\n");
        goto out;
    }
    *ctxpp = context;

out:
    return ret;
}

void
term(struct mls_el_ctx *ctx)
{
    mls_el_destroy_context(ctx);
}

void
run(struct mls_el_ctx *ctx)
{
    mls_el_announce_profile(ctx);
    mls_el_run_context(ctx);
}

/**********************************************************************/
int
main(int argc, char* argv[])
{
    int ret = 0;
    char* ifname = "eth0";
    struct mls_el_ctx *ctx = NULL;

    if (2 <= argc) {
        ifname = argv[1];
    }

    ret = init(ifname, &ctx);
    if (ret < 0) {
        goto out;
    }

    run(ctx);

out:
    term(ctx);
    return ret;
}

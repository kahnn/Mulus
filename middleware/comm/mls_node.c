#include <string.h>

#include "mls_dlink.h"
#include "mls_node.h"

/* TODO: XXXX local node only, fix for other nodes. */
static struct mls_node lnode;

struct mls_node*
mls_el_node_create(struct mls_eoj* profile)
{
    struct mls_node *node = &lnode;

    memset(node, 0, sizeof(struct mls_node));
    lnode.prof = profile;
    mls_dlink_init(&(lnode.eojs_list));

    profile->node = &lnode;
#if 0
    mls_dlink_init(&(profile->eojs_list));
#endif

    return node;
}

void
mls_el_node_add_device(struct mls_node* node, struct mls_eoj* obj)
{
    obj->node = node;
    mls_dlink_init(&(obj->eojs_list));
    mls_dlink_push(&(node->eojs_list), &(obj->eojs_list));
}

struct mls_eoj*
mls_el_node_get_device(struct mls_node *node, struct mls_eoj_code *code)
{
    struct mls_eoj *found = NULL;
    mls_dlink_t *work;
    struct mls_eoj* profile = node->prof;

    /* first: check profile object */
    if ((profile->code.cgc == code->cgc) &&
        (profile->code.clc == code->clc) &&
        (profile->code.inc == code->inc))
    {
        found = profile;
        goto out;
    }

    /* second: check device object */
    mls_dlink_loop(&(node->eojs_list), work)
    {
        struct mls_eoj* eoj =
            mls_dlink_container(work, struct mls_eoj, eojs_list);
        if ((eoj->code.cgc == code->cgc) &&
            (eoj->code.clc == code->clc) &&
            (eoj->code.inc == code->inc))
        {
            found = eoj;
            break;
        }
    }
out:
    return found;
}

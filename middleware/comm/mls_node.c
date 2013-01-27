#include <string.h>

#include "mls_dlink.h"
#include "mls_node.h"

/* XXXX local node only, fix for other nodes. */
static struct mls_node lnode;

struct mls_node*
mls_el_node_create(struct mls_eoj* profile)
{
    struct mls_node *node = &lnode;

    memset(node, 0, sizeof(struct mls_node));
    lnode.prof = profile;
    mls_dlink_init(&(lnode.eojs_list));

    return node;
}

void
mls_el_node_add_device(struct mls_node* node, struct mls_eoj* obj)
{
    obj->node = node;
    mls_dlink_init(&(obj->eojs_list));
    mls_dlink_push(&(node->eojs_list), &(obj->eojs_list));
}

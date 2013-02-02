/*
 * mulus: ECHONET Lite node.
 */
#ifndef _MULUS_NODE_H_
#define _MULUS_NODE_H_

#include "mls_obj.h"

/*
 * ECHONET Lite Node
 */
struct mls_node {
    /* TODO: Node address, ... */
    struct mls_eoj *prof;  /* profile object */
    mls_dlink_t eojs_list; /* struct mls_obj list */
};

extern struct mls_node* mls_el_node_create(struct mls_eoj* profile);
extern void mls_el_node_add_device(struct mls_node*, struct mls_eoj*);
extern struct mls_eoj* mls_el_node_get_device(struct mls_node*, struct mls_eoj_code*);
extern int mls_el_node_get_instance_list(struct mls_node*,
    unsigned char* buf, unsigned char* len);
extern int mls_el_node_get_class_list(struct mls_node*,
    unsigned char* buf, unsigned char* len);

#endif /* _MULUS_NODE_H_ */

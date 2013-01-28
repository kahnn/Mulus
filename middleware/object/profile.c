#include "mls_obj.h"
#include "mls_node.h"

static int
dummy_epr_get(struct mls_eoj* eoj, unsigned char epc,
    unsigned char* buf, unsigned char* len)
{
    /* XXXX */
    *len -= 1;
    buf[0] = (unsigned char)0x34;
    return 1;
}

static int
dummy_epr_set(struct mls_eoj* eoj, unsigned char epc,
    unsigned char* buf, unsigned char len)
{
    /* XXXX */
    return 1;
}

/* ---------------------------------------------------------------- */

static int
_get_instance_list(struct mls_eoj* eoj, unsigned char epc,
    unsigned char* buf, unsigned char* len)
{
    mls_dlink_t *work;
    struct mls_node* node = eoj->node;
    unsigned char nins = 0;
    /* buf[0] = number of instances */
    unsigned char *cp = &(buf[1]);
    *len -= 1;

    mls_dlink_loop(&(node->eojs_list), work)
    {
        struct mls_eoj* eoj =
            mls_dlink_container(work, struct mls_eoj, eojs_list);
        cp += mls_eoj_set_eojcode(&(eoj->code), cp, (unsigned int*)len);
        nins++;
    }
    buf[0] = nins;

    return (cp - buf);
}

/* ---------------------------------------------------------------- */

static struct mls_epr props[] = {
    /* profile super class */
    {
        .epc = MLS_EL_EPC_MANUFACTURER_CODE,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = dummy_epr_get,
    },
    {
        .epc = MLS_EL_EPC_STATUS_CHANGE_ANNOUNCEMENT_PROPERTY_MAP,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = dummy_epr_get,
    },
    {
        .epc = MLS_EL_EPC_SET_PROPERTY_MAP,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = dummy_epr_get,
    },
    {
        .epc = MLS_EL_EPC_GET_PROPERTY_MAP,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = dummy_epr_get,
    },
    /* node profile class */
    {
        .epc = MLS_EL_EPC_OPERATION_STATUS,
        .access_attr = (MLS_EPR_ACCESS_GET|MLS_EPR_ACCESS_SET),
        .getf = dummy_epr_get,
        .setf = dummy_epr_set,
        .is_anno_when_changed = 1,
    },
    {
        .epc = MLS_EL_EPC_STANDARD_VERSION_INFORMATION,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = dummy_epr_get,
    },
    {
        .epc = MLS_EL_EPC_IDENTIFICATION_NUMBER,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = dummy_epr_get,
    },
    {
        .epc = MLS_EL_EPC_UNIQUE_IDENTIFIER_DATA,
        .access_attr = (MLS_EPR_ACCESS_GET|MLS_EPR_ACCESS_SET),
        .getf = dummy_epr_get,
        .setf = dummy_epr_set,
    },
    {
        .epc = MLS_EL_EPC_NUMBER_OF_SELF_NODE_INSTANCES,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = dummy_epr_get,
    },
    {
        .epc = MLS_EL_EPC_NUMBER_OF_SELF_NODE_CLASSES,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = dummy_epr_get,
    },
    {
        .epc = MLS_EL_EPC_INSTANCE_LIST_NOTIFICATION,
        .access_attr = (MLS_EPR_ACCESS_GET|MLS_EPR_ACCESS_ANNO),
        .getf = _get_instance_list,
        .annof = _get_instance_list,
        .is_anno_when_changed = 1,
    },
    {
        .epc = MLS_EL_EPC_SELF_NODE_INSTANCE_LIST,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = dummy_epr_get,
    },
    {
        .epc = MLS_EL_EPC_SELF_NODE_CLASS_LIST,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = dummy_epr_get,
    },
};

static struct mls_eoj profile = {
    .code = {
        .cgc = MLS_EL_CGC_PROFILE,
        .clc = MLS_EL_CLC_NODEPROFILE,
        .inc = 0x01,
    },
    .nprops = sizeof(props)/sizeof(struct mls_epr),
    .props = props,
};

struct mls_eoj*
mls_el_get_profile(void)
{
    return &profile;
}

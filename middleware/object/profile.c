#include "mls_obj.h"

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

static int
dummy_epr_anno(struct mls_eoj* eoj, unsigned char epc,
    unsigned char* buf, unsigned char* len)
{
    /* XXXX */
    *len -= 1;
    buf[0] = (unsigned char)0x34;
    return 1;
}

static struct mls_epr props[] = {
    /* profile super class */
    {
        .epc = MLS_EL_EPC_MANUFACTURER_CODE,
        .getf = dummy_epr_get,
    },
    {
        .epc = MLS_EL_EPC_STATUS_CHANGE_ANNOUNCEMENT_PROPERTY_MAP,
        .getf = dummy_epr_get,
    },
    {
        .epc = MLS_EL_EPC_SET_PROPERTY_MAP,
        .getf = dummy_epr_get,
    },
    {
        .epc = MLS_EL_EPC_GET_PROPERTY_MAP,
        .getf = dummy_epr_get,
    },
    /* node profile class */
    {
        .epc = MLS_EL_EPC_OPERATION_STATUS,
        .getf = dummy_epr_get,
        .setf = dummy_epr_set,
        .is_anno_when_changed = 1,
    },
    {
        .epc = MLS_EL_EPC_STANDARD_VERSION_INFORMATION,
        .getf = dummy_epr_get,
    },
    {
        .epc = MLS_EL_EPC_IDENTIFICATION_NUMBER,
        .getf = dummy_epr_get,
    },
    {
        .epc = MLS_EL_EPC_UNIQUE_IDENTIFIER_DATA,
        .getf = dummy_epr_get,
        .setf = dummy_epr_set,
    },
    {
        .epc = MLS_EL_EPC_NUMBER_OF_SELF_NODE_INSTANCES,
        .getf = dummy_epr_get,
    },
    {
        .epc = MLS_EL_EPC_NUMBER_OF_SELF_NODE_CLASSES,
        .getf = dummy_epr_get,
    },
    {
        .epc = MLS_EL_EPC_INSTANCE_LIST_NOTIFICATION,
        .annof = dummy_epr_anno,
        .is_anno_when_changed = 1,
    },
    {
        .epc = MLS_EL_EPC_SELF_NODE_INSTANCE_LIST,
        .getf = dummy_epr_get,
    },
    {
        .epc = MLS_EL_EPC_SELF_NODE_CLASS_LIST,
        .getf = dummy_epr_get,
    },
};

static struct mls_eoj profile = {
    .cgc = MLS_EL_CGC_PROFILE,
    .clc = MLS_EL_CLC_NODEPROFILE,
    .inc = 0x01,
    .nprops = sizeof(props)/sizeof(struct mls_epr),
    .props = props,
};

struct mls_eoj*
mls_el_get_profile(void)
{
    return &profile;
}

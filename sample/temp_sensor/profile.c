#include <string.h>

#include "mls_type.h"
#include "mls_obj.h"
#include "mls_node.h"

#define errlog(fmt, ...) do{                   \
        fprintf(stderr, fmt, ##__VA_ARGS__);      \
  }while(0)
#define showlog(fmt, ...) do{                   \
        fprintf(stdout, fmt, ##__VA_ARGS__);      \
  }while(0)

/* ---------------------------------------------------------------- */

static unsigned char _manufacture_code[] = {
    (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00
};
static int
_get_manufacture_code(struct mls_eoj* eoj, unsigned char epc,
    unsigned char* buf, unsigned char* len)
{
    *len -= 3;
    buf[0] = _manufacture_code[0];
    buf[1] = _manufacture_code[1];
    buf[2] = _manufacture_code[2];
    return 3;
}
static int
_set_manufacture_code(struct mls_eoj* eoj, unsigned char epc,
    unsigned char* buf, unsigned char len)
{
    _manufacture_code[0] = buf[0];
    _manufacture_code[1] = buf[1];
    _manufacture_code[2] = buf[2];
    return 3;
}

/* ---------------------------------------------------------------- */

static int
_get_property_map(struct mls_eoj* eoj,
    unsigned char epc, unsigned char* buf, unsigned char* len, int access)
{
    int size;
    struct mls_node* node = eoj->node;
    int pnum = mls_eoj_get_num_properties_in_node(node, access);

    buf[0] = pnum;
    mls_eoj_get_property_map_in_node(node, access, pnum, &(buf[1]));

    if (pnum < MLS_EL_PROPERTY_MAP_BORDER) {
        size = 1 + pnum;
    } else {
        size = 1 + MLS_EL_PROPERTY_MAP_BORDER;
    }

    *len -= size;
    return size;
}

static int
_get_status_change_announcement_property_map(struct mls_eoj* eoj,
    unsigned char epc, unsigned char* buf, unsigned char* len)
{
    return _get_property_map(eoj, epc, buf, len, MLS_EPR_ACCESS_ANNO);
}

static int
_get_set_property_map(struct mls_eoj* eoj,
    unsigned char epc, unsigned char* buf, unsigned char* len)
{
    return _get_property_map(eoj, epc, buf, len, MLS_EPR_ACCESS_SET);
}

static int
_get_get_property_map(struct mls_eoj* eoj,
    unsigned char epc, unsigned char* buf, unsigned char* len)
{
    return _get_property_map(eoj, epc, buf, len, MLS_EPR_ACCESS_GET);
}

/* ---------------------------------------------------------------- */

/* ON=0x30, OFF=0x31 */
static unsigned char _operation_status = (unsigned char)0x31;

static int
_get_operation_status(struct mls_eoj* eoj,
    unsigned char epc, unsigned char* buf, unsigned char* len)
{
    *len -= 1;
    buf[0] = _operation_status;
    return 1;
}
static int
_set_operation_status(struct mls_eoj* eoj, unsigned char epc,
    unsigned char* buf, unsigned char len)
{
    unsigned char code = buf[0];
    /* ON=0x30, OFF=0x31 */
    if ((0x30 != code) && (0x31 != code)) {
        return -1;
    }
    _operation_status = buf[0];
    return 1;
}

/* ---------------------------------------------------------------- */

static unsigned char _standard_version_information[] = {
    (unsigned char)0x01, (unsigned char)0x01,
    (unsigned char)0x01, (unsigned char)0x00
};

static int
_get_standard_version_information(struct mls_eoj* eoj, unsigned char epc,
    unsigned char* buf, unsigned char* len)
{
    *len -= 4;
    buf[0] = _standard_version_information[0];
    buf[1] = _standard_version_information[1];
    buf[2] = _standard_version_information[2];
    buf[3] = _standard_version_information[3];
    return 4;
}
static int
_set_standard_version_information(struct mls_eoj* eoj, unsigned char epc,
    unsigned char* buf, unsigned char len)
{
    _standard_version_information[0] = buf[0];
    _standard_version_information[1] = buf[1];
    _standard_version_information[2] = buf[2];
    _standard_version_information[3] = buf[3];
    return 4;
}

/* ---------------------------------------------------------------- */

static unsigned char _identification_number[] = {
    (unsigned char)0xff,
    (unsigned char)0x00, (unsigned char)0x00,
    (unsigned char)0x00, (unsigned char)0x00,
    (unsigned char)0x00, (unsigned char)0x00,
    (unsigned char)0x00, (unsigned char)0x01
};
static int
_get_identification_number(struct mls_eoj* eoj, unsigned char epc,
    unsigned char* buf, unsigned char* len)
{
    int size = sizeof(_identification_number);
    *len -= size;
    memcpy(buf, _identification_number, size);
    return size;
}
static int
_set_identification_number(struct mls_eoj* eoj, unsigned char epc,
    unsigned char* buf, unsigned char len)
{
    int size = sizeof(_identification_number);
    memcpy(_identification_number, buf, size);
    return size;
}

/* ---------------------------------------------------------------- */

static unsigned short _unique_identifier_data = (unsigned short)0x8421;

static int
_get_unique_identifier_data(struct mls_eoj* eoj, unsigned char epc,
    unsigned char* buf, unsigned char* len)
{
    int size;
    size = mls_type_set_short(buf, (unsigned int*)len, _unique_identifier_data);
    return size;
}
static int
_set_unique_identifier_data(struct mls_eoj* eoj, unsigned char epc,
    unsigned char* buf, unsigned char len)
{
    _unique_identifier_data = 
        (unsigned short)mls_type_get_short(buf, (unsigned int)len);
    return sizeof(_unique_identifier_data);
}

/* ---------------------------------------------------------------- */

static int
_get_number_of_self_node_instances(struct mls_eoj* eoj, unsigned char epc,
    unsigned char* buf, unsigned char* len)
{
    int size = 3;
    *len -= size;
    buf[0] = 0x00;
    buf[1] = 0x00;
    buf[2] = 0x01; /* temperature_sensor */
    return size;
}

/* ---------------------------------------------------------------- */

static int
_get_number_of_self_node_classes(struct mls_eoj* eoj, unsigned char epc,
    unsigned char* buf, unsigned char* len)
{
    int size = 2;
    *len -= size;
    buf[0] = 0x00;
    buf[1] = 0x02; /* profile + temperature_sensor */
    return size;
}

/* ---------------------------------------------------------------- */

static int
_get_self_node_instance_list(struct mls_eoj* eoj, unsigned char epc,
    unsigned char* buf, unsigned char* len)
{
    int total;

    total = mls_el_node_get_instance_list(eoj->node, buf, len);
    *len -= total;

    return total;
}

/* ---------------------------------------------------------------- */

static int
_get_self_node_class_list(struct mls_eoj* eoj, unsigned char epc,
    unsigned char* buf, unsigned char* len)
{
    int total;

    total = mls_el_node_get_class_list(eoj->node, buf, len);
    *len -= total;

    return total;
}

/* ---------------------------------------------------------------- */

/*
 * プロパティ配列
 */
static struct mls_epr props[] = {
    /* profile super class */
    {
        .epc = MLS_EL_EPC_MANUFACTURER_CODE,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = _get_manufacture_code,
        .setf = _set_manufacture_code,
    },
    {
        .epc = MLS_EL_EPC_STATUS_CHANGE_ANNOUNCEMENT_PROPERTY_MAP,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = _get_status_change_announcement_property_map,
    },
    {
        .epc = MLS_EL_EPC_SET_PROPERTY_MAP,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = _get_set_property_map,
    },
    {
        .epc = MLS_EL_EPC_GET_PROPERTY_MAP,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = _get_get_property_map,
    },
    /* node profile class */
    {
        .epc = MLS_EL_EPC_OPERATION_STATUS,
        .access_attr = (MLS_EPR_ACCESS_GET|MLS_EPR_ACCESS_SET),
        .getf = _get_operation_status,
        .setf = _set_operation_status,
        .is_anno_when_changed = 1,
    },
    {
        .epc = MLS_EL_EPC_STANDARD_VERSION_INFORMATION,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = _get_standard_version_information,
        .setf = _set_standard_version_information,
    },
    {
        .epc = MLS_EL_EPC_IDENTIFICATION_NUMBER,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = _get_identification_number,
        .setf = _set_identification_number,
    },
    {
        .epc = MLS_EL_EPC_UNIQUE_IDENTIFIER_DATA,
        .access_attr = (MLS_EPR_ACCESS_GET|MLS_EPR_ACCESS_SET),
        .getf = _get_unique_identifier_data,
        .setf = _set_unique_identifier_data,
    },
    {
        .epc = MLS_EL_EPC_NUMBER_OF_SELF_NODE_INSTANCES,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = _get_number_of_self_node_instances,
    },
    {
        .epc = MLS_EL_EPC_NUMBER_OF_SELF_NODE_CLASSES,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = _get_number_of_self_node_classes,
    },
    {
        .epc = MLS_EL_EPC_INSTANCE_LIST_NOTIFICATION,
        .access_attr = (MLS_EPR_ACCESS_GET|MLS_EPR_ACCESS_ANNO),
        .getf = _get_self_node_instance_list,
        .annof = _get_self_node_instance_list,
        .is_anno_when_changed = 1,
    },
    {
        .epc = MLS_EL_EPC_SELF_NODE_INSTANCE_LIST,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = _get_self_node_instance_list,
    },
    {
        .epc = MLS_EL_EPC_SELF_NODE_CLASS_LIST,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = _get_self_node_class_list,
    },
};

/*
 * プロファイルオブジェクト
 */
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

/*
 * mulus: ECHONET Lite object.
 */
#ifndef _MULUS_OBJ_H_
#define _MULUS_OBJ_H_

#include "mls_dlink.h"
#include "mls_evt.h"
#include "mls_config.h"

/* 
 * Class Group Code.
 */
#define MLS_EL_CGC_SENSOR               ((unsigned char)0x00)
#define MLS_EL_CGC_AIR_CONDITIONER      ((unsigned char)0x01)
#define MLS_EL_CGC_HOUSING_FACILITIES   ((unsigned char)0x02)
#define MLS_EL_CGC_COOKING_HOUSEHOLD    ((unsigned char)0x03)
#define MLS_EL_CGC_HEALTH               ((unsigned char)0x04)
#define MLS_EL_CGC_MANAGEMENT_OPERATION ((unsigned char)0x05)
#define MLS_EL_CGC_AUDIOVISUAL          ((unsigned char)0x06)
#define MLS_EL_CGC_PROFILE              ((unsigned char)0x0E)
#define MLS_EL_CGC_USER_DEFINE          ((unsigned char)0x0F)

/* 
 * Common Class Code.
 */
/* MLS_EL_CGC_MANAGEMENT_OPERATION */
#define MLS_EL_CLC_SWITCH               ((unsigned char)0xFD)
#define MLS_EL_CLC_PORTABLE_TERMINAL    ((unsigned char)0xFE)
#define MLS_EL_CLC_CONTROLLER           ((unsigned char)0xFF)
/* MLS_EL_CGC_PROFILE              */
#define MLS_EL_CLC_NODEPROFILE          ((unsigned char)0xF0)

/* 
 * Common Property Code.
 */
#define MLS_EL_EPC_OPERATION_STATUS              ((unsigned char)0x80)
#define MLS_EL_EPC_INSTALLATION_LOCATION         ((unsigned char)0x81)
#define MLS_EL_EPC_STANDARD_VERSION_INFORMATION  ((unsigned char)0x82)
#define MLS_EL_EPC_IDENTIFICATION_NUMBER         ((unsigned char)0x83)
#define MLS_EL_EPC_MEASURED_INSTANTANEOUS_POWER_CONSUMPTION ((unsigned char)0x84)
#define MLS_EL_EPC_MEASURED_CUMULATIVE_POWER_CONSUMPTION ((unsigned char)0x85)
#define MLS_EL_EPC_MANUFACTURERS_FAULT_CODE      ((unsigned char)0x86)
#define MLS_EL_EPC_CURRENT_LIMIT_SETTING         ((unsigned char)0x87)
#define MLS_EL_EPC_FAULT_STATUS                  ((unsigned char)0x88)
#define MLS_EL_EPC_FAULT_DESCRIPTION             ((unsigned char)0x89)
#define MLS_EL_EPC_MANUFACTURER_CODE             ((unsigned char)0x8A)
#define MLS_EL_EPC_BUSINESS_FACILITY             ((unsigned char)0x8B)
#define MLS_EL_EPC_PRODUCT_CODE                  ((unsigned char)0x8C)
#define MLS_EL_EPC_PRODUCTION_NUMBER             ((unsigned char)0x8D)
#define MLS_EL_EPC_PRODUCTION_DATE               ((unsigned char)0x8E)
#define MLS_EL_EPC_POWER_SAVING_OPERATION_SETTING ((unsigned char)0x8F)

#define MLS_EL_EPC_POSITION_INFORMATION          ((unsigned char)0x93)
#define MLS_EL_EPC_CURRENT_TIME_SETTING          ((unsigned char)0x97)
#define MLS_EL_EPC_CURRENT_DATE_SETTING          ((unsigned char)0x98)
#define MLS_EL_EPC_POWER_LIMIT_SETTING           ((unsigned char)0x99)
#define MLS_EL_EPC_CUMULATIVE_OPERATING_TIME     ((unsigned char)0x9A)
#define MLS_EL_EPC_SETM_PROPERTY                 ((unsigned char)0x9B)
#define MLS_EL_EPC_GETM_PROPERTY                 ((unsigned char)0x9C)
#define MLS_EL_EPC_STATUS_CHANGE_ANNOUNCEMENT_PROPERTY_MAP ((unsigned char)0x9D)
#define MLS_EL_EPC_SET_PROPERTY_MAP              ((unsigned char)0x9E)
#define MLS_EL_EPC_GET_PROPERTY_MAP              ((unsigned char)0x9F)

#define MLS_EL_EPC_UNIQUE_IDENTIFIER_DATA        ((unsigned char)0xBF)
#define MLS_EL_EPC_NUMBER_OF_SELF_NODE_INSTANCES ((unsigned char)0xD3)
#define MLS_EL_EPC_NUMBER_OF_SELF_NODE_CLASSES   ((unsigned char)0xD4)
#define MLS_EL_EPC_INSTANCE_LIST_NOTIFICATION    ((unsigned char)0xD5)
#define MLS_EL_EPC_SELF_NODE_INSTANCE_LIST       ((unsigned char)0xD6)
#define MLS_EL_EPC_SELF_NODE_CLASS_LIST          ((unsigned char)0xD7)

/* 
 * Common Property Value.
 */
/* Basic type - code */
#define MLS_EL_EDT_SCHAR_UNDERFLOW   ((unsigned char)0x80)
#define MLS_EL_EDT_SCHAR_OVERFLOW    ((unsigned char)0x7F)
#define MLS_EL_EDT_SSHORT_UNDERFLOW  ((unsigned char)0x8000)
#define MLS_EL_EDT_SSHORT_OVERFLOW   ((unsigned char)0x7FFF)
#define MLS_EL_EDT_SLONG_UNDERFLOW   ((unsigned char)0x80000000)
#define MLS_EL_EDT_SLONG_OVERFLOW    ((unsigned char)0x7FFFFFFF)

#define MLS_EL_EDT_UCHAR_UNDERFLOW   ((unsigned char)0xFE)
#define MLS_EL_EDT_UCHAR_OVERFLOW    ((unsigned char)0xFF)
#define MLS_EL_EDT_USHORT_UNDERFLOW  ((unsigned char)0xFFFE)
#define MLS_EL_EDT_USHORT_OVERFLOW   ((unsigned char)0xFFFF)
#define MLS_EL_EDT_ULONG_UNDERFLOW   ((unsigned char)0xFFFFFFFE)
#define MLS_EL_EDT_ULONG_OVERFLOW    ((unsigned char)0xFFFFFFFF)

/* MLS_EL_EPC_OPERATION_STATUS */
#define MLS_EL_EDT_OPERATION_STATUS_ON   ((unsigned char)0x30)
#define MLS_EL_EDT_OPERATION_STATUS_OFF  ((unsigned char)0x31)
/* MLS_EL_EPC_INSTALLATION_LOCATION         */
#define MLS_EL_EDT_INSTALLATION_LOCATION_INITIAL_STATE ((unsigned char)0x00)
#define MLS_EL_EDT_INSTALLATION_LOCATION_INDEFINITE    ((unsigned char)0xFF)
/* MLS_EL_EPC_STANDARD_VERSION_INFORMATION  */
/* MLS_EL_EPC_IDENTIFICATION_NUMBER         */
#define MLS_EL_EPC_IDENTIFICATION_NUMBER_MAKER17       ((unsigned char)0xFE)
#define MLS_EL_EPC_IDENTIFICATION_NUMBER_PROTOCOL9     ((unsigned char)0xFF)
#define MLS_EL_EPC_IDENTIFICATION_NUMBER_INITIAL_STATE ((unsigned char)0x00)
/* MLS_EL_EPC_MEASURED_INSTANTANEOUS_POWER_CONSUMPTION */
/* MLS_EL_EPC_MEASURED_CUMULATIVE_POWER_CONSUMPTION */
/* MLS_EL_EPC_MANUFACTURERS_FAULT_CODE      */
/* MLS_EL_EPC_CURRENT_LIMIT_SETTING         */
#define MLS_EL_EDT_CURRENT_LIMIT_SETTING_MIN  ((unsigned char)0x00) /*   0% */
#define MLS_EL_EDT_CURRENT_LIMIT_SETTING_MAX  ((unsigned char)0x64) /* 100% */
/* MLS_EL_EPC_FAULT_STATUS                  */
#define MLS_EL_EDT_FAULT_STATUS_OCCURRENCE ((unsigned char)0x41)
#define MLS_EL_EDT_FAULT_STATUS_NONE       ((unsigned char)0x42)
/* MLS_EL_EPC_FAULT_DESCRIPTION             */
/* MLS_EL_EPC_MANUFACTURER_CODE             */
/* MLS_EL_EPC_BUSINESS_FACILITY             */
/* MLS_EL_EPC_PRODUCT_CODE                  */
/* MLS_EL_EPC_PRODUCTION_NUMBER             */
/* MLS_EL_EPC_PRODUCTION_DATE               */
/* MLS_EL_EPC_POWER_SAVING_OPERATION_SETTING */
#define MLS_EL_EDT_POWER_SAVING_OPERATION_SETTING_ENABLE   ((unsigned char)0x41)
#define MLS_EL_EDT_POWER_SAVING_OPERATION_SETTING_DISABLE  ((unsigned char)0x42)
/* MLS_EL_EPC_POSITION_INFORMATION          */
/* MLS_EL_EPC_CURRENT_TIME_SETTING          */
/* MLS_EL_EPC_CURRENT_DATE_SETTING          */
/* MLS_EL_EPC_POWER_LIMIT_SETTING           */
/* MLS_EL_EPC_CUMULATIVE_OPERATING_TIME     */
#define MLS_EL_EDT_CUMULATIVE_OPERATING_TIME_UNIT_SEC   ((unsigned char)0x41)
#define MLS_EL_EDT_CUMULATIVE_OPERATING_TIME_UNIT_MIN   ((unsigned char)0x42)
#define MLS_EL_EDT_CUMULATIVE_OPERATING_TIME_UNIT_HOUR  ((unsigned char)0x43)
#define MLS_EL_EDT_CUMULATIVE_OPERATING_TIME_UNIT_DAY   ((unsigned char)0x44)
/* MLS_EL_EPC_SETM_PROPERTY                 */
/* MLS_EL_EPC_GETM_PROPERTY                 */
/* MLS_EL_EPC_STATUS_CHANGE_ANNOUNCEMENT_PROPERTY_MAP */
/* MLS_EL_EPC_SET_PROPERTY_MAP              */
/* MLS_EL_EPC_GET_PROPERTY_MAP              */

/* MLS_EL_EPC_UNIQUE_IDENTIFIER_DATA        */
/* MLS_EL_EPC_NUMBER_OF_SELF_NODE_INSTANCES */
/* MLS_EL_EPC_NUMBER_OF_SELF_NODE_CLASSES   */
/* MLS_EL_EPC_INSTANCE_LIST_NOTIFICATION    */
/* MLS_EL_EPC_SELF_NODE_INSTANCE_LIST       */
/* MLS_EL_EPC_SELF_NODE_CLASS_LIST          */

/* ---------------------------------------------------------------- */

struct mls_eoj;

/*
 * initialize function.
 */
typedef int (*mls_epr_init_t)(struct mls_conf* conf);

/*
 * @arg eoj object.
 * @arg epc property code.
 * @arg buf [O] buffer.
 * @arg len [IO] buffer size, property data size.
 * @return property data size. if ret<=0, return error code.
 */
typedef int (*mls_epr_get_t)(struct mls_eoj*,unsigned char,unsigned char*,unsigned char*);

/*
 * @arg eoj object.
 * @arg epc property code.
 * @arg buf [I] buffer.
 * @arg len [I] buffer size.
 * @return property data size. if ret<=0, return error code.
 */
typedef int (*mls_epr_set_t)(struct mls_eoj*,unsigned char,unsigned char*,unsigned char);

/*
 * @arg eoj object.
 * @arg epc property code.
 * @arg buf [O] buffer.
 * @arg len [IO] buffer size, property data size.
 * @return property data size. if ret<=0, return error code.
 */
typedef int (*mls_epr_anno_t)(struct mls_eoj*,unsigned char,unsigned char*,unsigned char*);

#define MLS_EPR_ACCESS_GET  (0x01)
#define MLS_EPR_ACCESS_SET  (0x02)
#define MLS_EPR_ACCESS_ANNO (0x04)

/*
 * ECHONET Lite Property
 */
struct mls_epr {
    unsigned char epc; /* property code */

    /* access attribute */
    int access_attr;

    /* access function */
    mls_epr_init_t initf;
    mls_epr_get_t getf;
    mls_epr_set_t setf;
    mls_epr_anno_t annof;

    /* when property changed, announce. */
    int is_anno_when_changed;
};
/* 
 * Property Map.
 */
#define MLS_EL_PROPERTY_MAP_BORDER  (16)

struct mls_node;

extern int mls_eoj_get_num_properties(struct mls_eoj*, int access);
extern int mls_eoj_get_num_properties_in_node(struct mls_node*, int access);
extern int mls_eoj_get_property_map(struct mls_eoj*, int access,
    int pnum, unsigned char* buf);
extern int mls_eoj_get_property_map_in_node(struct mls_node*, int access,
    int pnum, unsigned char* buf);

/*
 * ECHONET Lite Object
 */
struct mls_eoj_code {
    unsigned char cgc; /* class group code */
    unsigned char clc; /* class code */
    unsigned char inc; /* instance code */
};
#define mls_eoj_equal_eojc(a,b) \
    (((a)->cgc == (b)->cgc) && ((a)->clc == (b)->clc) && ((a)->inc == (b)->inc))

struct mls_eoj {
    /* Object Code */
    struct mls_eoj_code code;

    /* Properties */
    unsigned int nprops;
    struct mls_epr *props;

    /* time interval function. (tag -> mls_el_ctx) */
    mls_evt_callback_t tinterval;

    /* for node */
    struct mls_node* node;
    mls_dlink_t eojs_list; /* struct mls_obj list */
};

/* ---------------------------------------------------------------- */

extern struct mls_epr* mls_eoj_get_property(struct mls_eoj*, unsigned char);
extern int mls_eoj_set_eojclass(struct mls_eoj_code*, unsigned char*, unsigned int*);
extern int mls_eoj_set_eojcode(struct mls_eoj_code*, unsigned char*, unsigned int*);
extern int mls_eoj_get_eojcode(struct mls_eoj_code*, unsigned char*, unsigned int*);

#endif /* _MULUS_OBJ_H_ */

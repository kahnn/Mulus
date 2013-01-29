/*
Class group code : 0x00
Class code : 0x11
Instance code : 0x01–0x7F (0x00: All-instance specification code)

Operation status 0x80 (Set/Get), change announce
This property indicates the ON/OFF status.
(unsigned char) ON=0x30, OFF=0x31

Measured temperature value 0xE0 (Get)
This property indicates the measured temperature value in units of 0.1 C.
(short) 0xF554–0x7FFF (-2732–32766) (-273.2–6.6)
 */
#include "mls_obj.h"
#include "mls_type.h"

#define MLS_EL_CLC_TEMPERATURE_SENSOR         ((unsigned char)0x11)
#define MLS_EL_EPC_MEASURED_TEMPERATURE_VALUE ((unsigned char)0xE0)

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

static unsigned char _installation_location = (unsigned char)0x00;
static int
_get_installation_location(struct mls_eoj* eoj,
    unsigned char epc, unsigned char* buf, unsigned char* len)
{
    *len -= 1;
    buf[0] = _installation_location;
    return 1;
}
static int
_set_installation_location(struct mls_eoj* eoj, unsigned char epc,
    unsigned char* buf, unsigned char len)
{
    _installation_location = buf[0];
    return 1;
}

/* ---------------------------------------------------------------- */

static unsigned char _standard_version_information[] = {
    (unsigned char)0x00, (unsigned char)0x00,
    (unsigned char)0x42, (unsigned char)0x00
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

static unsigned char _fault_status = (unsigned char)0x00;
static int
_get_fault_status(struct mls_eoj* eoj,
    unsigned char epc, unsigned char* buf, unsigned char* len)
{
    *len -= 1;
    buf[0] = _fault_status;
    return 1;
}
static int
_set_fault_status(struct mls_eoj* eoj, unsigned char epc,
    unsigned char* buf, unsigned char len)
{
    _fault_status = buf[0];
    return 1;
}

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
    int pnum = mls_eoj_get_num_properties(eoj, access);

    buf[0] = pnum;
    mls_eoj_get_property_map(eoj, access, pnum, &(buf[1]));

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

static short _temp_value = 200; /* 20.0 */

static int
_get_temp_value(struct mls_eoj* eoj, unsigned char epc,
    unsigned char* buf, unsigned char* len)
{
    return mls_type_set_short(buf, (unsigned int*)len, _temp_value);
}

static int
_set_temp_value(struct mls_eoj* eoj, unsigned char epc,
    unsigned char* buf, unsigned char len)
{
    short val;
    val = mls_type_get_short(buf, len);
    _temp_value = val;

    return sizeof(val);
}

/* ---------------------------------------------------------------- */

static void
_timeinterval_handler(struct mls_evt* evt, void* tag)
{
    /*
      定期処理を記述する。
      この場合、mulusのデフォルトのインターバル(MLS_EL_TIMEINTERVAL_SEC=10sec)
      の周期で毎回、0.1C 温度が上昇する。
     */
    _temp_value++;
}
/* ---------------------------------------------------------------- */

static struct mls_epr props[] = {
    /* device object super class */
    {
        .epc = MLS_EL_EPC_OPERATION_STATUS,
        .access_attr = (MLS_EPR_ACCESS_GET|MLS_EPR_ACCESS_SET),
        .getf = _get_operation_status,
        .setf = _set_operation_status,
        .is_anno_when_changed = 1,
    },
    {
        .epc = MLS_EL_EPC_INSTALLATION_LOCATION,
        .access_attr = (MLS_EPR_ACCESS_GET|MLS_EPR_ACCESS_SET),
        .getf = _get_installation_location,
        .setf = _set_installation_location,
        .is_anno_when_changed = 1,
    },
    {
        .epc = MLS_EL_EPC_STANDARD_VERSION_INFORMATION,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = _get_standard_version_information,
        .setf = _set_standard_version_information,
    },
    {
        .epc = MLS_EL_EPC_FAULT_STATUS,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = _get_fault_status,
        .setf = _set_fault_status,
        .is_anno_when_changed = 1,
    },
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
    /* temperature sensor class */
    {
        .epc = MLS_EL_EPC_MEASURED_TEMPERATURE_VALUE,
        .access_attr = (MLS_EPR_ACCESS_GET),
        .getf = _get_temp_value,
        .setf = _set_temp_value,
    },
};

static struct mls_eoj temp_sensor = {
    .code = {
        .cgc = MLS_EL_CGC_SENSOR,
        .clc = MLS_EL_CLC_TEMPERATURE_SENSOR,
        .inc = 0x01,
    },
    .nprops = sizeof(props)/sizeof(struct mls_epr),
    .props = props,
    .tinterval = _timeinterval_handler,
};

struct mls_eoj*
mls_el_get_temperature_sensor(void)
{
    return &temp_sensor;
}

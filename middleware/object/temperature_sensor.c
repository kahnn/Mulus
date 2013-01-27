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

static short _temp_value = 200; /* 20.0 */

static int
_get_temp_value(struct mls_eoj* eoj, unsigned char epc,
    unsigned char* buf, unsigned char* len)
{
    return mls_type_set_short(buf, len, _temp_value);
}

int
temperature_sensor_set_temp_value(unsigned char* buf, unsigned char len)
{
    short val;
    val = mls_type_get_short(buf, len);
    _temp_value = val;

    return sizeof(val);
}

/* ---------------------------------------------------------------- */

static struct mls_epr props[] = {
    /* device object super class */
    {
        .epc = MLS_EL_EPC_OPERATION_STATUS,
        .getf = dummy_epr_get,
        .setf = dummy_epr_set,
        .is_anno_when_changed = 1,
    },
    {
        .epc = MLS_EL_EPC_INSTALLATION_LOCATION,
        .getf = dummy_epr_get,
        .setf = dummy_epr_set,
        .is_anno_when_changed = 1,
    },
    {
        .epc = MLS_EL_EPC_STANDARD_VERSION_INFORMATION,
        .getf = dummy_epr_get,
    },
    {
        .epc = MLS_EL_EPC_FAULT_STATUS,
        .getf = dummy_epr_get,
        .is_anno_when_changed = 1,
    },
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
    /* temperature sensor class */
    {
        .epc = MLS_EL_EPC_MEASURED_TEMPERATURE_VALUE,
        .getf = _get_temp_value,
    },
};

static struct mls_eoj temp_sensor = {
    .cgc = MLS_EL_CGC_SENSOR,
    .clc = MLS_EL_CLC_TEMPERATURE_SENSOR,
    .inc = 0x01,
    .nprops = sizeof(props)/sizeof(struct mls_epr),
    .props = props,
};

struct mls_eoj*
mls_el_get_temperature_sensor(void)
{
    return &temp_sensor;
}

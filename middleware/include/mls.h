/*
 * mulus: ECHONET Lite.
 */
#ifndef _MULUS_H_
#define _MULUS_H_

/*
 * ECHONET Lite Node
 */
struct mls_eoj {
    /* XXX Address, ... */
    struct mls_eoj prof;  /* profile object */
    struct mls_eoj eojs[0]; /* node local objects */
};

/*
 * ECHONET Lite Object
 */
struct mls_eoj {
    struct mls_eoj_code ecode;
    struct mls_epr eprops[0];
};

/*
 * ECHONET Lite Object Code
 */
struct mls_eoj_code {
    unsigned char cgc; /* class group code */
    unsigned char clc; /* class code */
    unsigned char inc; /* instance code */
};

/*
 * ECHONET Lite Property
 */
struct mls_epr {
    unsigned char epc; /* property code */
    unsigned char attr; /* Get, Set, Anno */
    /* XXX Callback functions ?  */
    unsigned char pdc; /* property data counter */
    unsigned char edt[0]; /* property data */
};

#endif /* _MULUS_H_ */

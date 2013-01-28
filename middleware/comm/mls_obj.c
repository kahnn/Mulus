#include "mls_obj.h"

struct mls_epr*
mls_eoj_get_property(struct mls_eoj* eoj, unsigned char epc)
{
    struct mls_epr *found = NULL;
    int i;

    for (i = 0; i < eoj->nprops; i++) {
        struct mls_epr *prop = &(eoj->props[i]);
        if (epc == prop->epc) {
            found = prop;
            break;
        }
    }
    return found;
}

int
mls_eoj_set_eojcode(struct mls_eoj_code* eoj,
    unsigned char* buf, unsigned int* len)
{
    int size = 3;
    buf[0] = eoj->cgc;
    buf[1] = eoj->clc;
    buf[2] = eoj->inc;
    *len -= size;
    return size;
}

int
mls_eoj_get_eojcode(struct mls_eoj_code* eoj,
    unsigned char* buf, unsigned int* len)
{
    int size = 3;
    eoj->cgc = buf[0];
    eoj->clc = buf[1];
    eoj->inc = buf[2];
    *len -= size;
    return size;
}

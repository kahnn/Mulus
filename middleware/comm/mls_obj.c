#include "mls_obj.h"
#include "mls_node.h"

int
mls_eoj_get_num_properties(struct mls_eoj* eoj, int access)
{
    int num = 0;
    int i;

    for (i = 0; i < eoj->nprops; i++) {
        struct mls_epr* epr = &(eoj->props[i]);
        if (((MLS_EPR_ACCESS_GET & access) &&
                (MLS_EPR_ACCESS_GET & epr->access_attr))
            ||
            ((MLS_EPR_ACCESS_SET & access) &&
                (MLS_EPR_ACCESS_SET & epr->access_attr))
            ||
            ((MLS_EPR_ACCESS_ANNO & access) && epr->is_anno_when_changed))
        {
            num++;
        }
    }
    return num;
}

int
mls_eoj_get_num_properties_in_node(struct mls_node* node, int access)
{
    mls_dlink_t *work;
    int num = 0;

    mls_dlink_loop(&(node->eojs_list), work)
    {
        struct mls_eoj* eoj =
            mls_dlink_container(work, struct mls_eoj, eojs_list);
        num += mls_eoj_get_num_properties(eoj, access);
    }
    return num;
}

static void
_set_prop_map(unsigned char epc, unsigned char *buf)
{
    int h = ((epc >> 4) & 0x0F);
    int l = (epc & 0x0F);
    buf[l] = (unsigned char)((buf[l] & 0xFF) | (1 << (h - 8)));
}

int
mls_eoj_get_property_map(struct mls_eoj* eoj, int access,
    int pnum, unsigned char* buf)
{
    int i, num = 0;

    for (i = 0; i < eoj->nprops; i++) {
        struct mls_epr* epr = &(eoj->props[i]);

        if (((MLS_EPR_ACCESS_GET & access) &&
                (MLS_EPR_ACCESS_GET & epr->access_attr))
            ||
            ((MLS_EPR_ACCESS_SET & access) &&
                (MLS_EPR_ACCESS_SET & epr->access_attr))
            ||
            ((MLS_EPR_ACCESS_ANNO & access) && epr->is_anno_when_changed))
        {
            if (pnum < MLS_EL_PROPERTY_MAP_BORDER) {
                *buf = epr->epc;
                buf++;
                num++;
            } else {
                _set_prop_map(epr->epc, buf);
                num++;
            }
        }
    }

    return num;
}

int
mls_eoj_get_property_map_in_node(struct mls_node* node, int access,
    int pnum, unsigned char* buf)
{
    int num = 0;
    mls_dlink_t *work;

    mls_dlink_loop(&(node->eojs_list), work)
    {
        struct mls_eoj* eoj =
            mls_dlink_container(work, struct mls_eoj, eojs_list);

        num += mls_eoj_get_property_map(eoj, access, pnum, buf);
        if (pnum < MLS_EL_PROPERTY_MAP_BORDER) {
            buf += num;
        }
    }

    return num;
}

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
mls_eoj_set_eojclass(struct mls_eoj_code* eoj,
    unsigned char* buf, unsigned int* len)
{
    int size = 2;
    buf[0] = eoj->cgc;
    buf[1] = eoj->clc;
    *len -= size;
    return size;
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

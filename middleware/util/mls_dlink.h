/*
 * doubly-linked list.
 */
#ifndef _MULUS_DLINK_H_
#define _MULUS_DLINK_H_

#include <stddef.h>

typedef struct mls_dlink {
    struct mls_dlink *prev;
    struct mls_dlink *next;
} mls_dlink_t;

static inline void mls_dlink_init(mls_dlink_t *dl)
{
    dl->prev = dl;
    dl->next = dl;
}

/* append after base. */
static inline void mls_dlink_append(mls_dlink_t *base, mls_dlink_t *dl)
{
    dl->prev = base;
    dl->next = base->next;
    base->next->prev = dl;
    base->next = dl;
}

/* insert before base. */
static inline void mls_dlink_insert(mls_dlink_t *base, mls_dlink_t *dl)
{
    dl->prev = base->prev;
    dl->next = base;
    base->prev->next = dl;
    base->prev = dl;
}

static inline void mls_dlink_remove(mls_dlink_t *dl)
{
    dl->prev->next = dl->next;
    dl->next->prev = dl->prev;
    mls_dlink_init(dl);
}

/*
 * @arg _addr address of dlink_t*.
 * @arg _type container data type.
 * @arg _member dlink_t memner name.
 */
#define mls_dlink_container(_addr, _type, _member)              \
    ((_type *)(((char*)(_addr)) - offsetof(_type, _member)))

/*
 * @arg _head head.
 * @arg _work .
 */
#define mls_dlist_loop(_head, _work)                            \
    for (_work = (_head)->next; _work != (_head); _work = _work->next)

#endif /* _MULUS_DLINK_H_ */

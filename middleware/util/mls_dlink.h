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

static inline mls_dlink_t* mls_dlink_remove(mls_dlink_t *dl)
{
    dl->prev->next = dl->next;
    dl->next->prev = dl->prev;
    mls_dlink_init(dl);
    return dl;
}

#define mls_dlink_head(_base)  ((_base)->next)
#define mls_dlink_tail(_base)  ((_base)->prev)

#define mls_dlink_is_empty(_base) \
    (mls_dlink_head((_base)) == (_base))

/*
 * append to tail.
 */
#define mls_dlink_push(_base, _work) \
    mls_dlink_insert((_base), (_work))

/*
 * remove from tail.
 */
#define mls_dlink_pop(_base) \
    ( (mls_dlink_is_empty(_base)) ? NULL : mls_dlink_remove((_base)->prev) )

/*
 * @arg _base anchor.
 * @arg _work .
 */
#define mls_dlink_loop(_base, _work) \
    for (_work = (_base)->next; _work != (_base); _work = _work->next)

/*
 * @arg _addr address of dlink_t*.
 * @arg _type container data type.
 * @arg _member dlink_t memner name.
 */
#define mls_dlink_container(_addr, _type, _member) \
    ((_type *)(((char*)(_addr)) - offsetof(_type, _member)))

#endif /* _MULUS_DLINK_H_ */

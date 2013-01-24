#include    <stdio.h>
#include    <string.h>
#include    "mls_dlink.h"

struct test_container {
    int data1;
    mls_dlink_t dlink;
    int data2;
};

static void init_container(struct test_container *c, int d1, int d2)
{
    mls_dlink_init(&(c->dlink));
    c->data1 = d1;
    c->data2 = d2;
}

int
main(int argc, char *argv[])
{
    mls_dlink_t head, *work;
    struct test_container c1, c2, c3;

    mls_dlink_init(&head);
    init_container(&c1, 1, 1);
    init_container(&c2, 2, 2);
    init_container(&c3, 3, 3);

    mls_dlink_append(&head, &(c1.dlink));
    mls_dlink_append(&(c1.dlink), &(c2.dlink));
    mls_dlink_insert(&head, &(c3.dlink));

    mls_dlink_loop(&head, work)
    {
        struct test_container *con = 
            mls_dlink_container(work, struct test_container, dlink);
        printf("1>> %p, %d, %d\n", con, con->data1, con->data2);
    }

    mls_dlink_remove(&(c2.dlink));

    mls_dlink_loop(&head, work)
    {
        struct test_container *con = 
            mls_dlink_container(work, struct test_container, dlink);
        printf("2>> %p, %d, %d\n", con, con->data1, con->data2);
    }

    printf("is empty() ==  %d\n", mls_dlink_is_empty(&head));
    
    while (NULL != (work = mls_dlink_pop(&head))) {
        struct test_container *con = 
            mls_dlink_container(work, struct test_container, dlink);
        printf("3>> %p, %d, %d\n", con, con->data1, con->data2);
    }

    printf("is empty() ==  %d\n", mls_dlink_is_empty(&head));

    return 0;
}

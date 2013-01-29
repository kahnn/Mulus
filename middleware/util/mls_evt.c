#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <errno.h>

#include "mls_evt.h"

#ifndef max
#define max(a,b) (((a) <= (b)) ? (b) : (a))
#endif

struct mls_evt_handle {
    enum mls_handle_type type;
    int fd;
    mls_evt_callback_t callback;
    void *tag;
    struct mls_evt *evt;
    mls_dlink_t dlink;
};

/*
 * @arg timeout 0=polloing, 0<timeout(sec.), 0>block
 */
struct mls_evt*
mls_evt_ini(int timeout)
{
    struct mls_evt *evt = NULL;

    evt = malloc(sizeof(struct mls_evt));
    if (NULL == evt) {
        goto out;
    }

    evt->timeout = timeout;
    evt->is_active = 1;
    mls_dlink_init(&(evt->dlink_fd));
    mls_dlink_init(&(evt->dlink_to));
out:
    return evt;
}

void
mls_evt_fin(struct mls_evt *evt)
{
    mls_dlink_t *pdl;

    while (NULL != (pdl = mls_dlink_pop(&(evt->dlink_fd)))) {
        struct mls_evt_handle *handle =
            mls_dlink_container(pdl, struct mls_evt_handle, dlink);
        free(handle);
    }

    while (NULL != (pdl = mls_dlink_pop(&(evt->dlink_to)))) {
        struct mls_evt_handle *handle =
            mls_dlink_container(pdl, struct mls_evt_handle, dlink);
        free(handle);
    }

    free(evt);
}

/*
 * @arg fd when type is MLS_EVT_FD, file descriptor. 
 *         when type is MLS_EVT_TIMEOUT, handle key.
 * @arg tag callback argument.
 */
int
mls_evt_add_handle(struct mls_evt *evt,
    enum mls_handle_type type, int fd,
    mls_evt_callback_t callback, void *tag)
{
    int ret = 0;
    struct mls_evt_handle* handle = NULL;

    handle = malloc(sizeof(struct mls_evt_handle));
    if (NULL == handle) {
        ret = -errno;
        goto out;
    }

    memset(handle, 0, sizeof(*handle));
    handle->type = type;
    handle->fd = fd;
    handle->callback = callback;
    handle->tag = tag;
    handle->evt = evt;
    mls_dlink_init(&(handle->dlink));
    if (MLS_EVT_FD == type) {
        /* for file descriptor */
        mls_dlink_push(&(evt->dlink_fd), &(handle->dlink));
    } else {
        /* for timeout */
        mls_dlink_push(&(evt->dlink_to), &(handle->dlink));
    }

out:
    return ret;
}

/*
 * @return 0==found and success, 0!=not found handler
 */
int
mls_evt_del_handle(struct mls_evt *evt,
    enum mls_handle_type type, int fd)
{
    int ret = -1; /* not found */
    mls_dlink_t *base, *pdl;

    if (MLS_EVT_FD == type) {
        /* for file descriptor */
        base = &(evt->dlink_fd);
    } else {
        /* for timeout */
        base = &(evt->dlink_to);
    }

    mls_dlink_loop(base, pdl)
    {
        struct mls_evt_handle *handle =
            mls_dlink_container(pdl, struct mls_evt_handle, dlink);
        if (handle->fd == fd) {
            mls_dlink_remove(pdl);
            free(handle);
            ret = 0; /* found */
            break;
        }
    }
    return ret;
}

/*
 * @arg timeout 0=polloing, 0<timeout(sec.), 0>block
 */
void
mls_evt_timeout(struct mls_evt *evt, int timeout)
{
    evt->timeout = timeout;
    /* TODO: interrupt sleep at select(). */
}

void
mls_evt_deactivate(struct mls_evt *evt)
{
    evt->is_active = 0;
    /* TODO: interrupt sleep at select. */
}

static void
_setup_fdmask(struct mls_evt *evt, fd_set *mask, int *maxfd)
{
    mls_dlink_t *pdl;

    /* setup mask */
    FD_ZERO(mask);
    *maxfd = 0;

    mls_dlink_loop(&(evt->dlink_fd), pdl)
    {
        struct mls_evt_handle *handle =
            mls_dlink_container(pdl, struct mls_evt_handle, dlink);
        FD_SET(handle->fd, mask);
        *maxfd = max(*maxfd, handle->fd);
    }
}

void
mls_evt_dispatch(struct mls_evt *evt)
{
    time_t prev_timeout;

    prev_timeout = time(NULL);

    /* event dispatch loop */
    while (evt->is_active) {
        int maxfd, nfd;
        fd_set ready;
        struct timeval timeout;

        _setup_fdmask(evt, &ready, &maxfd);
        if (0 <= evt->timeout) {
            timeout.tv_sec = evt->timeout - (time(NULL) - prev_timeout);
            if (timeout.tv_sec < 0) timeout.tv_sec = 0;
        }
        timeout.tv_usec = 0;

        nfd = 
            select((maxfd + 1), &ready, NULL, NULL, 
                (0 <= evt->timeout) ? &timeout : NULL);
        switch (nfd) {
        case -1:
            perror("select"); /* TODO: error handling */
            break;
        case 0:  /* timeout */
        default: /* fire event */
            /* check time exceeded */
            if ((0 < evt->timeout)
                && ((prev_timeout + evt->timeout) <= time(NULL)))
            {
                mls_dlink_t *pdl;
                mls_dlink_loop(&(evt->dlink_to), pdl)
                {
                    struct mls_evt_handle *handle =
                        mls_dlink_container(pdl, struct mls_evt_handle, dlink);
                    if (NULL != handle->callback) {
                        handle->callback(evt, handle->tag);
                    }
                }
                prev_timeout = time(NULL);
            }
            /* check event */
            if (0 < nfd) {
                mls_dlink_t *pdl;
                mls_dlink_loop(&(evt->dlink_fd), pdl)
                {
                    struct mls_evt_handle *handle =
                        mls_dlink_container(pdl, struct mls_evt_handle, dlink);
                    if (FD_ISSET(handle->fd, &ready)
                        && (NULL != handle->callback))
                    {
                        handle->callback(evt, handle->tag);
                    }
                }
            }
            break;
        }
    }
}

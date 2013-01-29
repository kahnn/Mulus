#include <assert.h>
#include "mls_type.h"

int
mls_type_set_char(unsigned char* buf, unsigned int* len, char src)
{
    int size = 1;
    assert(1 <= *len);
    buf[0] = (unsigned char)(src & 0xff);
    *len -= size;
    return size;
}

int
mls_type_set_short(unsigned char* buf, unsigned int* len, short src)
{
    int size = 2;
    assert(2 <= *len);
    buf[0] = (unsigned char)((src >> 8) & 0xff);
    buf[1] = (unsigned char)(src        & 0xff);
    *len -= size;
    return size;
}

int
mls_type_set_int(unsigned char* buf, unsigned int* len, int src)
{
    int size = 4;
    assert(4 <= *len);
    buf[0] = (unsigned char)((src >> 24) & 0xff);
    buf[1] = (unsigned char)((src >> 16) & 0xff);
    buf[2] = (unsigned char)((src >> 8)  & 0xff);
    buf[3] = (unsigned char)(src         & 0xff);
    *len -= size;
    return size;
}

char
mls_type_get_char(unsigned char* buf, unsigned int len)
{
    char dst = 0;
    assert(1 <= len);
    dst |= (char)(buf[0] & 0xff);
    return dst;
}

short
mls_type_get_short(unsigned char* buf, unsigned int len)
{
    short dst = 0;
    assert(2 <= len);;
    dst |= (short)((buf[0] << 8) & 0xff00);
    dst |= (short)(buf[1]        & 0x00ff);
    return dst;
}

int
mls_type_get_int(unsigned char* buf, unsigned int len)
{
    int dst = 0;
    assert(4 <= len);
    dst |= (int)((buf[0] << 24) & 0xff000000);
    dst |= (int)((buf[1] << 16) & 0x00ff0000);
    dst |= (int)((buf[2] << 8)  & 0x0000ff00);
    dst |= (int)(buf[3]         & 0x000000ff);
    return dst;
}

/* --------------------------------------------------------------- */

#if 0 /* XXX TBD */

int
mls_type_bbuf_setup(struct mls_type_bbuf **bbuf, unsigned char *buf, int buflen)
{
    int ret = 0;
    int allocated = 0;

    if (NULL == bbuf) {
        if ((*bbuf = malloc(sizeof(struct mls_type_bbuf))) == NULL) {
            ret = -errno;
            goto out;
        }
        allocated |= MLS_TYPE_BBUF_CONTEXT_ALLOCATED;
    }
    memset(*bbuf, 0 sizeof(struct mls_type_bbuf));

    if (NULL == buf) {
        if ((buf = malloc(buflen)) == NULL) {
            ret = -errno;
            goto out;
        }
        allocated |= MLS_TYPE_BBUF_BUFFER_ALLOCATED;
    }

    (*bbuf)->buf = buf;
    (*bbuf)->buflen = buflen;
    (*bbuf)->is_allocated = allocated;
    (*bbuf)->head = buf;
    (*bbuf)->tail = buf;
    (*bbuf)->restlen = buflen;

out:
    if (0 != ret) {
        if (allocated & MLS_TYPE_BBUF_BUFFER_ALLOCATED)
            free(buf);
        if (allocated & MLS_TYPE_BBUF_CONTEXT_ALLOCATED)
            free(*bbuf);
    }
    return ret;
}

void
mls_type_bbuf_teardown(struct mls_type_bbuf *bbuf)
{
    if ((bbuf->is_allocated) & MLS_TYPE_BBUF_BUFFER_ALLOCATED) {
        free(bbuf->buf);
    }
    if ((bbuf->is_allocated) & MLS_TYPE_BBUF_CONTEXT_ALLOCATED) {
        free(bbuf);
    } else {
        memset(bbuf, 0 sizeof(*bbuf));
    }
}

int mls_type_bbuf_pull_char(struct mls_type_bbuf *bbuf, unsigned char *data);
int mls_type_bbuf_pull_short(struct mls_type_bbuf *bbuf, unsigned short *data);
int mls_type_bbuf_pull_int(struct mls_type_bbuf *bbuf, unsigned int *data);

int mls_type_bbuf_push_char(struct mls_type_bbuf *bbuf, unsigned char data);
int mls_type_bbuf_push_short(struct mls_type_bbuf *bbuf, unsigned short data);
int mls_type_bbuf_push_int(struct mls_type_bbuf *bbuf, unsigned int data);

int mls_type_bbuf_seek(struct mls_type_bbuf *bbuf, unsigned int pos);
int mls_type_bbuf_reset(struct mls_type_bbuf *bbuf);
#endif /* XXX TBD */

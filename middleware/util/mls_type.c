#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
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

/* ------------------------------------------------------------------ */

void
mls_type_data_ushort_to_bytearray(unsigned char* outb, unsigned short inb)
{
    outb[0] = (unsigned char)((inb >> 8) & (unsigned char)0xff);
    outb[1] = (unsigned char)((inb     ) & (unsigned char)0xff);
}
void
mls_type_data_bytearray_to_ushort(unsigned char* inb, unsigned short *outb)
{
    *outb = (((unsigned short)(inb[0]) << 8) & (unsigned short)0xff00) |
            (((unsigned short)(inb[1])     ) & (unsigned short)0x00ff);
}

void
mls_type_data_uint_to_bytearray(unsigned char* outb, unsigned int inb)
{
    outb[0] = (unsigned char)((inb >> 24) & (unsigned char)0xff);
    outb[1] = (unsigned char)((inb >> 16) & (unsigned char)0xff);
    outb[2] = (unsigned char)((inb >>  8) & (unsigned char)0xff);
    outb[3] = (unsigned char)((inb      ) & (unsigned char)0xff);
}
void
mls_type_data_bytearray_to_uint(unsigned char* inb, unsigned int *outb)
{
    *outb = (((unsigned int)(inb[0]) << 24) & (unsigned int)0xff000000) |
            (((unsigned int)(inb[1]) << 16) & (unsigned int)0x00ff0000) |
            (((unsigned int)(inb[2]) <<  8) & (unsigned int)0x0000ff00) |
            (((unsigned int)(inb[3])      ) & (unsigned int)0x000000ff);
}

void
mls_type_data_ull_to_bytearray(unsigned char* outb, unsigned long long inb)
{
    outb[0] = (unsigned char)((inb >> 56) & (unsigned char)0xff);
    outb[1] = (unsigned char)((inb >> 48) & (unsigned char)0xff);
    outb[2] = (unsigned char)((inb >> 40) & (unsigned char)0xff);
    outb[3] = (unsigned char)((inb >> 32) & (unsigned char)0xff);
    outb[4] = (unsigned char)((inb >> 24) & (unsigned char)0xff);
    outb[5] = (unsigned char)((inb >> 16) & (unsigned char)0xff);
    outb[6] = (unsigned char)((inb >>  8) & (unsigned char)0xff);
    outb[7] = (unsigned char)((inb      ) & (unsigned char)0xff);
}
void
mls_type_data_bytearray_to_ull(unsigned char* inb, unsigned long long *outb)
{
    *outb = (((unsigned long long)(inb[0]) << 56) & 0xff00000000000000LLU) |
            (((unsigned long long)(inb[1]) << 48) & 0x00ff000000000000LLU) |
            (((unsigned long long)(inb[2]) << 40) & 0x0000ff0000000000LLU) |
            (((unsigned long long)(inb[3]) << 32) & 0x000000ff00000000LLU) |
            (((unsigned long long)(inb[4]) << 24) & 0x00000000ff000000LLU) |
            (((unsigned long long)(inb[5]) << 16) & 0x0000000000ff0000LLU) |
            (((unsigned long long)(inb[6]) <<  8) & 0x000000000000ff00LLU) |
            (((unsigned long long)(inb[7])      ) & 0x00000000000000ffLLU);
}

/* ------------------------------------------------------------------ */

char*
mls_type_data_get_str(struct mls_conf* storage, unsigned char epc)
{
    char key[2+1];
    char *data;
    
    sprintf(key, "%02X", epc);
    data = mls_conf_get(storage, key);
    return data;
}

/* ------------------------------------------------------------------ */

void
mls_type_data_str_to_bytearray(char* instr, unsigned char *outb, int size)
{
    int i, len = strlen(instr);
    char *str;
    char token[2+1];

    for (i = 0, str = instr; (i < len) && (0 < size); i += 2, str += 2, size--) {
        token[0] = str[0];
        token[1] = str[1];
        token[2] = '\0';

        *outb = (unsigned char)strtoul(token, NULL, 16);
        outb += 1;
    }
}

void
mls_type_data_bytearray_to_str(char* outstr, unsigned char *inb, int size)
{
    int i;
    
    for (i = 0; i < size; i++) {
        sprintf(outstr, "%02X", inb[i]);
        outstr += 2;
    }
}

int
mls_type_data_put_bytearray(struct mls_conf* storage, unsigned char epc,
                         unsigned char *inb, int size)
{
    int ret = 0;
    char key[2+1];
    char val[UCHAR_MAX*2+1]; /* enough space */

    sprintf(key, "%02X", epc);
    mls_type_data_bytearray_to_str(val, inb, size);
    ret = mls_conf_set(storage, key, val);
    if (ret != 0) {
        goto out;
    }
    mls_conf_store(storage);

  out:
    return ret;
}

/* ------------------------------------------------------------------ */

void
mls_type_data_str_to_ushort(char* instr, unsigned short *outb)
{
    char token[4+1];

    memcpy(token, instr, 4);
    token[4] = '\0';
    *outb = (unsigned short)strtoul(token, NULL, 16);
}

void
mls_type_data_ushort_to_str(char* outstr, unsigned short inb)
{
    int i, size = 2;
    
    for (i = 0; i < size; i++) {
        unsigned char byte;
        byte = (unsigned char)((inb >> ((size-i-1)*8)) & (unsigned char)0xff);
        sprintf(outstr, "%02X", byte);
        outstr += 2;
    }
}

int
mls_type_data_put_ushort(struct mls_conf* storage, unsigned char epc, unsigned short inb)
{
    int ret = 0;
    char key[2+1];
    char val[2*2+1]; /* enough space */

    sprintf(key, "%02X", epc);
    mls_type_data_ushort_to_str(val, inb);
    ret = mls_conf_set(storage, key, val);
    if (ret != 0) {
        goto out;
    }
    mls_conf_store(storage);

  out:
    return ret;
}

/* ------------------------------------------------------------------ */

void
mls_type_data_str_to_uint(char* instr, unsigned int *outb)
{
    char token[8+1];

    memcpy(token, instr, 8);
    token[8] = '\0';
    *outb = (unsigned int)strtoul(token, NULL, 16);
}

void
mls_type_data_uint_to_str(char* outstr, unsigned int inb)
{
    int i, size = 4;
    
    for (i = 0; i < size; i++) {
        unsigned char byte;
        byte = (unsigned char)((inb >> ((size-i-1)*8)) & (unsigned char)0xff);
        sprintf(outstr, "%02X", byte);
        outstr += 2;
    }
}

int
mls_type_data_put_uint(struct mls_conf* storage, unsigned char epc, unsigned int inb)
{
    int ret = 0;
    char key[2+1];
    char val[4*2+1]; /* enough space */

    sprintf(key, "%02X", epc);
    mls_type_data_uint_to_str(val, inb);
    ret = mls_conf_set(storage, key, val);
    if (ret != 0) {
        goto out;
    }
    mls_conf_store(storage);

  out:
    return ret;
}

/* --------------------------------------------------------------- */

#if 0 /* TODO: byte buffer */

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
#endif /* TODO: byte buffer */

#include <assert.h>
#include "mls_type.h"

int
mls_type_set_char(unsigned char* buf, unsigned char* len, char src)
{
    int size = 1;
    assert(1 <= *len);
    buf[0] = (unsigned char)(src & 0xff);
    *len -= size;
    return size;
}

int
mls_type_set_short(unsigned char* buf, unsigned char* len, short src)
{
    int size = 2;
    assert(2 <= *len);
    buf[0] = (unsigned char)((src >> 8) & 0xff);
    buf[1] = (unsigned char)(src        & 0xff);
    *len -= size;
    return size;
}

int
mls_type_set_int(unsigned char* buf, unsigned char* len, int src)
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
mls_type_get_char(unsigned char* buf, unsigned char len)
{
    char dst = 0;
    assert(1 <= len);
    dst |= (char)(buf[0] & 0xff);
    return dst;
}

short
mls_type_get_short(unsigned char* buf, unsigned char len)
{
    short dst = 0;
    assert(2 <= len);;
    dst |= (short)((buf[0] << 8) & 0xff00);
    dst |= (short)(buf[1]        & 0x00ff);
    return dst;
}

int
mls_type_get_int(unsigned char* buf, unsigned char len)
{
    int dst = 0;
    assert(4 <= len);
    dst |= (int)((buf[0] << 24) & 0xff000000);
    dst |= (int)((buf[1] << 16) & 0x00ff0000);
    dst |= (int)((buf[2] << 8)  & 0x0000ff00);
    dst |= (int)(buf[3]         & 0x000000ff);
    return dst;
}

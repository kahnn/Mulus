/*
 * mulus type utility.
 */
#ifndef _MULUS_TYPE_H_
#define _MULUS_TYPE_H_

#include "mls_config.h"

extern int mls_type_set_char(unsigned char* buf, unsigned int* len, char src);
extern int mls_type_set_short(unsigned char* buf, unsigned int* len, short src);
extern int mls_type_set_int(unsigned char* buf, unsigned int* len, int src);
extern char mls_type_get_char(unsigned char* buf, unsigned int len);
extern short mls_type_get_short(unsigned char* buf, unsigned int len);
extern int mls_type_get_int(unsigned char* buf, unsigned int len);

/*
 * Utility functions.
 */
extern void mls_type_data_ushort_to_bytearray(unsigned char* outb, unsigned short inb);
extern void mls_type_data_bytearray_to_ushort(unsigned char* inb, unsigned short *outb);
extern void mls_type_data_uint_to_bytearray(unsigned char* outb, unsigned int inb);
extern void mls_type_data_bytearray_to_uint(unsigned char* inb, unsigned int *outb);
extern void mls_type_data_ull_to_bytearray(unsigned char* outb, unsigned long long inb);
extern void mls_type_data_bytearray_to_ull(unsigned char* inb, unsigned long long *outb);

extern char* mls_type_data_get_str(struct mls_conf* storage, unsigned char epc);

extern void mls_type_data_str_to_bytearray(char* instr, unsigned char *outb, int size);
extern void mls_type_data_bytearray_to_str(char* outstr, unsigned char *inb, int size);
extern int mls_type_data_put_bytearray(struct mls_conf* storage, unsigned char epc, unsigned char *inb, int size);

extern void mls_type_data_str_to_ushort(char* instr, unsigned short *outb);
extern void mls_type_data_ushort_to_str(char* outstr, unsigned short inb);
extern int mls_type_data_put_ushort(struct mls_conf* storage, unsigned char epc, unsigned short inb);
extern void mls_type_data_str_to_uint(char* instr, unsigned int *outb);
extern void mls_type_data_uint_to_str(char* outstr, unsigned int inb);
extern int mls_type_data_put_uint(struct mls_conf* storage, unsigned char epc, unsigned int inb);

#if 0 /* TODO: byte buffer */
#define MLS_TYPE_BBUF_BUFFER_ALLOCATED  (0x01)
#define MLS_TYPE_BBUF_CONTEXT_ALLOCATED (0x02)

struct mls_type_bbuf {
    unsigned char *buf;
    int buflen;
    int is_allocated;
    unsigned char *head, *tail;
    int restlen;
};

extern int mls_type_bbuf_setup(struct mls_type_bbuf **bbuf, unsigned char *buf, int buflen);
extern void mls_type_bbuf_teardown(struct mls_type_bbuf *bbuf);
extern int mls_type_bbuf_pull_char(struct mls_type_bbuf *bbuf, unsigned char *data);
extern int mls_type_bbuf_pull_short(struct mls_type_bbuf *bbuf, unsigned short *data);
extern int mls_type_bbuf_pull_int(struct mls_type_bbuf *bbuf, unsigned int *data);

extern int mls_type_bbuf_push_char(struct mls_type_bbuf *bbuf, unsigned char data);
extern int mls_type_bbuf_push_short(struct mls_type_bbuf *bbuf, unsigned short data);
extern int mls_type_bbuf_push_int(struct mls_type_bbuf *bbuf, unsigned int data);
extern int mls_type_bbuf_seek(struct mls_type_bbuf *bbuf, unsigned int pos);
extern int mls_type_bbuf_reset(struct mls_type_bbuf *bbuf);
#endif /* TODO: byte buffer */

#endif /* _MULUS_TYPE_H_ */

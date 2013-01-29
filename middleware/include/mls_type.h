/*
 * mulus type utility.
 */
#ifndef _MULUS_TYPE_H_
#define _MULUS_TYPE_H_

extern int mls_type_set_char(unsigned char* buf, unsigned int* len, char src);
extern int mls_type_set_short(unsigned char* buf, unsigned int* len, short src);
extern int mls_type_set_int(unsigned char* buf, unsigned int* len, int src);
extern char mls_type_get_char(unsigned char* buf, unsigned int len);
extern short mls_type_get_short(unsigned char* buf, unsigned int len);
extern int mls_type_get_int(unsigned char* buf, unsigned int len);

#if 0 /* XXX TBD */
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
#endif /* XXX TBD */

#endif /* _MULUS_TYPE_H_ */

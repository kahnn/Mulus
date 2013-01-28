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

#endif /* _MULUS_TYPE_H_ */

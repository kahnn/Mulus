/*
 * mulus type utility.
 */
#ifndef _MULUS_TYPE_H_
#define _MULUS_TYPE_H_

extern int mls_type_set_char(unsigned char* buf, unsigned char* len, char src);
extern int mls_type_set_short(unsigned char* buf, unsigned char* len, short src);
extern int mls_type_set_int(unsigned char* buf, unsigned char* len, int src);
extern char mls_type_get_char(unsigned char* buf, unsigned char len);
extern short mls_type_get_short(unsigned char* buf, unsigned char len);
extern int mls_type_get_int(unsigned char* buf, unsigned char len);

#endif /* _MULUS_TYPE_H_ */

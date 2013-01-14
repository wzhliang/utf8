#ifndef __W_UTF8_H__
#define __W_UTF8_H__

inline int IS_TRAILING(unsigned char c) { return ( c >> 6 == 0x2); }
inline int IS_7BITS(unsigned char c)   { return ( c < 127 ); }
inline int IS_12BITS(unsigned char c)   { return ( c >> 5 == 0x6); }
inline int IS_16BITS(unsigned char c)   { return ( c >> 4 == 0xE); }
inline int IS_21BITS(unsigned char c)   { return ( c >> 3 == 0x1E); }
inline int IS_26BITS(unsigned char c)   { return ( c >> 2 == 0x3E); }
inline int IS_31BITS(unsigned char c)   { return ( c >> 1 == 0xEE); }

#define UTF8_MAX_LEN 6

#endif

#ifndef COMMON_H
#define COMMON_H

/*
 * Functions and data are declared extern "C" and named after their address
 * (fn_XXXXXXXX, D_XXXXXXXX) until their real names are known, so their
 * linker symbols do not depend on C++ signature mangling.
 */

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed long s32;
typedef unsigned long u32;

#endif

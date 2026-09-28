#ifndef AIL_H
#define AIL_H

/*
 * Miles Sound System (RAD Game Tools) internals, as linked into the game.
 * Public entry points use the __cdecl convention, so their symbols carry a
 * leading underscore (_AIL_...).
 */

typedef signed long S32;
typedef unsigned long U32;
typedef signed short S16;
typedef unsigned short U16;
typedef signed char S8;
typedef unsigned char U8;

#ifndef NULL
#define NULL 0
#endif

typedef struct _SAMPLE {
    U8 unknown0[0x4c];
    S32 playback_rate;
    S32 volume;
} SAMPLE, *HSAMPLE;

#endif

#include "clib.h"

extern const char D_00128890[4];
extern const char D_00128894[4];

int __Nan_Inf(double value, char *buf);
#pragma aux __Nan_Inf parm routine [];

// FUNCTION: 0xfc93b
int __Nan_Inf(double value, char *buf)
{
    union {
        double d;
        unsigned short w[4];
    } u;
    char *p;

    u.d = value;
    if ((int)(unsigned short)(u.w[3] & 0x7ff0) == 0x7ff0) {
        p = buf;
        if ((int)(unsigned short)(u.w[3] & 0x8000) != 0)
            *p++ = '-';
        if ((u.w[0] | u.w[1] | u.w[2] | (u.w[3] & 0x000f)) == 0) {
            *(long *)p = *(const long *)D_00128890;
        } else {
            *(long *)p = *(const long *)D_00128894;
        }
        return 1;
    }
    return 0;
}

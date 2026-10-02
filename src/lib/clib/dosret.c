#include "clib.h"

extern unsigned char _osmajor;
extern const signed char xlat[20];

void __set_doserrno(int err);

int _dosretax(int ax, int carry);
int __set_errno_dos(unsigned err);

// _dosret0: return 0, or -1 with errno set from DOS error ax if carry is set.
// FUNCTION: 0xed140
int _dosret0(int ax, int carry)
{
    if (carry)
        return _dosretax(ax, carry);
    return 0;
}

// _dosretax: return ax, or -1 with errno set from DOS error ax if carry is set.
// FUNCTION: 0xed147
int _dosretax(int ax, int carry)
{
    if (carry)
        return __set_errno_dos(ax);
    return ax;
}

// __set_errno_dos: store a DOS error in _doserrno, map it to errno (high byte, if nonzero, is the errno itself); returns -1.
// FUNCTION: 0xed14b
// NONMATCHING: byte zero-extensions use and instead of xor/mov, and errnum
// is not computed in eax
int __set_errno_dos(unsigned err)
{
    unsigned char code;
    int errnum;

    code = err;
    __set_doserrno(err & 0xff);
    if (err < 0x100) {
        if ((int)_osmajor >= 3) {
            if ((int)code == 0x50)
                code = 0x0e;
            else if ((int)code >= 0x22)
                code = 0x13;
            else if ((int)code >= 0x20)
                code = 0x05;
        }
        if ((int)code > 0x13)
            code = 0x13;
        errnum = xlat[code];
    } else {
        errnum = (err >> 8) & 0xff;
    }
    __set_errno(errnum);
    return -1;
}

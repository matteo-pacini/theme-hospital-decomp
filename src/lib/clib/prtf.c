#include "clib.h"

#define BUF_SIZE 40

#define SPF_ALT        0x01
#define SPF_BLANK      0x02
#define SPF_FORCE_SIGN 0x04
#define SPF_LEFT       0x08
#define SPF_SHORT      0x10
#define SPF_LONG       0x20
#define SPF_NEAR       0x40
#define SPF_LONG_DOUBLE 0x40
#define SPF_FAR        0x80

typedef struct {
    char *_dest;
    int _fld_width;
    int _prec;
    int _n0;
    int _output_count;
    unsigned char _flags;
    char _character;
    char _pad_char;
    char _prefix[4];
} SPECS;

typedef char __far *FAR_STRING;

/*
 * The original compiler tested byte flags as a byte AND widened to int;
 * the cast reproduces that with Watcom 10.6.
 */
#define FLAG(s, f) ((int)(unsigned char)((s)->_flags & (f)))

extern FAR_STRING (*__EFG_printf)(char *buffer, va_list *pargs, SPECS *specs);

char *ltoa(long value, char *buffer, int radix);
char *ultoa(unsigned long value, char *buffer, int radix);
char *itoa(int value, char *buffer, int radix);
int toupper(int c);

const char *getprintspecs(const char *ctl, va_list *pargs, SPECS *specs);
const char *evalflags(const char *ctl, SPECS *specs);
FAR_STRING formstring(char *buffer, va_list *pargs, SPECS *specs, char *null_string);
int far_strlen(FAR_STRING s, int precision);
int far_other_strlen(FAR_STRING s, int precision);
void fmt4hex(unsigned value, char *buf, int maxlen);
void FixedPoint_Format(char *buf, long value, SPECS *specs);
FAR_STRING float_format(char *buffer, va_list *pargs, SPECS *specs);
void zupstr(char *p);

// __prtf: printf engine; format args, emitting each char through out_putc(specs, c) with dest as the destination; returns chars output.
// FUNCTION: 0xcf5c3
int __prtf(void *dest, const char *format, va_list args, void (*out_putc)())
{
    char buffer[BUF_SIZE];
    char null_char;
    FAR_STRING a;
    const char *ctl;
    SPECS specs;
    char *p;

    null_char = '\0';
    specs._dest = dest;
    specs._flags = 0;
    specs._output_count = 0;
    ctl = format;
    while (*ctl != '\0') {
        if ((int)*ctl != '%') {
            (*out_putc)(&specs, *ctl++);
        } else {
            ++ctl;
            ctl = getprintspecs(ctl, (va_list *)args, &specs);
            specs._character = *ctl++;
            if (specs._character == '\0')
                break;
            if ((int)specs._character == 'n') {
                if (FLAG(&specs, SPF_LONG)) {
                    if (FLAG(&specs, SPF_FAR)) {
                        *(*(long __far **)((args[0] += 8) - 8)) = specs._output_count;
                    } else if (FLAG(&specs, SPF_NEAR)) {
                        *(*(long **)((args[0] += 4) - 4)) = specs._output_count;
                    } else {
                        *(*(long **)((args[0] += 4) - 4)) = specs._output_count;
                    }
                } else if (FLAG(&specs, SPF_SHORT)) {
                    if (FLAG(&specs, SPF_FAR)) {
                        *(*(short __far **)((args[0] += 8) - 8)) = specs._output_count;
                    } else if (FLAG(&specs, SPF_NEAR)) {
                        *(*(short **)((args[0] += 4) - 4)) = specs._output_count;
                    } else {
                        *(*(short **)((args[0] += 4) - 4)) = specs._output_count;
                    }
                } else {
                    if (FLAG(&specs, SPF_FAR)) {
                        *(*(int __far **)((args[0] += 8) - 8)) = specs._output_count;
                    } else if (FLAG(&specs, SPF_NEAR)) {
                        *(*(int **)((args[0] += 4) - 4)) = specs._output_count;
                    } else {
                        *(*(int **)((args[0] += 4) - 4)) = specs._output_count;
                    }
                }
            } else {
                a = formstring(buffer, (va_list *)args, &specs, &null_char);
                if (!FLAG(&specs, SPF_LEFT)) {
                    if ((int)specs._pad_char == ' ') {
                        while (--specs._fld_width >= 0) {
                            (*out_putc)(&specs, specs._pad_char);
                        }
                    }
                }
                p = specs._prefix;
                while (*p) {
                    (*out_putc)(&specs, *p++);
                }
                while (specs._n0-- != 0) {
                    (*out_putc)(&specs, '0');
                }
                if (!FLAG(&specs, SPF_LEFT)) {
                    if ((int)specs._pad_char != ' ') {
                        while (--specs._fld_width >= 0) {
                            (*out_putc)(&specs, specs._pad_char);
                        }
                    }
                }
                if ((int)specs._character == 's' || (int)specs._character == 'S') {
                    if (FLAG(&specs, SPF_LONG)) {
                        while (specs._prec-- != 0) {
                            (*out_putc)(&specs, *a);
                            a += 2;
                        }
                    } else {
                        while (specs._prec-- != 0) {
                            (*out_putc)(&specs, *a++);
                        }
                    }
                } else {
                    while (specs._prec-- != 0) {
                        (*out_putc)(&specs, *a++);
                    }
                }
                if (FLAG(&specs, SPF_LEFT)) {
                    if (specs._fld_width > 0) {
                        while (specs._fld_width-- != 0) {
                            (*out_putc)(&specs, ' ');
                        }
                    }
                }
            }
        }
    }
    return specs._output_count;
}

// getprintspecs: parse a conversion's flags, width, precision and size modifier at ctl into specs (consuming '*' arguments); returns the conversion character's position.
// FUNCTION: 0xcf89d
const char *getprintspecs(const char *ctl, va_list *pargs, SPECS *specs)
{
    specs->_prefix[0] = '\0';
    specs->_pad_char = ' ';
    ctl = evalflags(ctl, specs);
    specs->_fld_width = 0;
    if ((int)*ctl >= '0' && (int)*ctl <= '9') {
        do {
            specs->_fld_width = specs->_fld_width * 10 + (*ctl++ - '0');
        } while ((int)*ctl >= '0' && (int)*ctl <= '9');
    } else if ((int)*ctl == '*') {
        specs->_fld_width = *(int *)(((*pargs)[0] += 4) - 4);
        if (specs->_fld_width < 0) {
            specs->_fld_width = -specs->_fld_width;
            specs->_flags |= SPF_LEFT;
        }
        ctl++;
    }
    specs->_prec = -1;
    if ((int)*ctl == '.') {
        ctl++;
        specs->_prec = 0;
        if ((int)*ctl == '*') {
            specs->_prec = *(int *)(((*pargs)[0] += 4) - 4);
            if (specs->_prec < 0)
                specs->_prec = -1;
            ctl++;
        } else {
            while ((int)*ctl >= '0' && (int)*ctl <= '9') {
                specs->_prec = specs->_prec * 10 + (*ctl++ - '0');
            }
        }
    }
    switch (*ctl) {
    case 'l':
    case 'w':
        specs->_flags |= SPF_LONG;
        ctl++;
        break;
    case 'h':
        specs->_flags |= SPF_SHORT;
        ctl++;
        break;
    case 'L':
        specs->_flags |= SPF_LONG_DOUBLE;
        ctl++;
        break;
    case 'F':
        specs->_flags |= SPF_FAR;
        ctl++;
        break;
    case 'N':
        specs->_flags |= SPF_NEAR;
        ctl++;
        break;
    }
    return ctl;
}

// evalflags: parse printf flag characters (-#+ 0) at ctl into specs; returns the first non-flag.
// FUNCTION: 0xcf9db
const char *evalflags(const char *ctl, SPECS *specs)
{
    specs->_flags = 0;
    for (;; ctl++) {
        if ((int)*ctl == '-') {
            specs->_flags |= SPF_LEFT;
        } else if ((int)*ctl == '#') {
            specs->_flags |= SPF_ALT;
        } else if ((int)*ctl == '+') {
            specs->_flags |= SPF_FORCE_SIGN;
            specs->_flags &= ~SPF_BLANK;
        } else if ((int)*ctl == ' ') {
            if (FLAG(specs, SPF_FORCE_SIGN) == 0) {
                specs->_flags |= SPF_BLANK;
            }
        } else if ((int)*ctl == '0') {
            specs->_pad_char = '0';
        } else {
            break;
        }
    }
    return ctl;
}

// far_strlen: length of far string s, at most precision (-1 = unlimited).
// FUNCTION: 0xcfa3a
int far_strlen(FAR_STRING s, int precision)
{
    int len;

    len = 0;
    while (*s++ != '\0') {
        if (len == precision)
            break;
        ++len;
    }
    return len;
}

// far_other_strlen: length in 16-bit characters of wide string s, at most precision.
// FUNCTION: 0xcfa5e
int far_other_strlen(FAR_STRING s, int precision)
{
    int len;
    unsigned short __far *ptr;

    len = 0;
    ptr = (unsigned short __far *)s;
    while (*ptr++ != 0) {
        if (len == precision)
            break;
        ++len;
    }
    return len;
}

// fmt4hex: value as exactly maxlen zero-padded hex digits (low digits kept) in buf.
// FUNCTION: 0xcfa81
void fmt4hex(unsigned value, char *buf, int maxlen)
{
    int i;
    int len;

    itoa(value, buf, 16);
    len = strlen(buf);
    for (i = maxlen - 1; len; --i) {
        --len;
        buf[i] = buf[len];
    }
    while (i >= 0) {
        buf[i] = '0';
        --i;
    }
    buf[maxlen] = '\0';
}

typedef union {
    unsigned long uWhole;
    long sWhole;
    struct {
        unsigned short lo;
        unsigned short hi;
    } wd;
    struct {
        unsigned char b0;
        unsigned char b1;
        unsigned char b2;
        unsigned char b3;
    } bite;
} T32;

// FixedPoint_Format: format 16.16 fixed-point value in decimal (%hf; default 4 places, rounded) into buf.
// FUNCTION: 0xcfadf
void FixedPoint_Format(char *buf, long value, SPECS *specs)
{
    T32 at;
    int i;
    char *bufp;

    at.sWhole = value;
    if (at.sWhole < 0) {
        at.sWhole = -at.sWhole;
        *buf++ = '-';
    }
    if (specs->_prec == -1)
        specs->_prec = 4;
    itoa(at.wd.hi, buf, 10);
    bufp = buf;
    while (*buf)
        ++buf;
    if (specs->_prec != 0) {
        *buf++ = '.';
        for (i = 0; i < specs->_prec; i++) {
            at.wd.hi = 0;
            at.uWhole *= 10;
            *buf++ = at.bite.b2 + '0';
        }
        *buf = '\0';
    }
    if ((int)(unsigned short)(at.wd.lo & 0x8000)) {
        for (;;) {
            if (buf == bufp) {
                *buf++ = '1';
                while ((int)*buf == '0')
                    ++buf;
                if ((int)*buf == '.') {
                    *buf++ = '0';
                    *buf++ = '.';
                    while ((int)*buf == '0')
                        ++buf;
                }
                *buf++ = '0';
                *buf = '\0';
                break;
            }
            --buf;
            if ((int)*buf == '.')
                --buf;
            if ((int)*buf != '9') {
                ++*buf;
                break;
            }
            *buf = '0';
        }
    }
}

// float_format: format a floating-point argument through the installed __EFG_printf handler.
// FUNCTION: 0xcfbd4
FAR_STRING float_format(char *buffer, va_list *pargs, SPECS *specs)
{
    return (*__EFG_printf)(buffer, pargs, specs);
}

// formstring: convert the current argument for specs's conversion into text (in buffer or the argument string), setting prefix, zero padding and remaining field width; returns the text.
// FUNCTION: 0xcfbdb
// NONMATCHING: flip_sign and the null_string spill swap stack slots; the
// original merges the f/e/g far_strlen tails into no_zero's call (ours into
// the 'f' case's), loads a far %s argument with two movs instead of les, and
// picks other registers in the va_arg reads
FAR_STRING formstring(char *buffer, va_list *pargs, SPECS *specs, char *null_string)
{
    FAR_STRING arg;
    int length;
    int radix;
    unsigned long long_value;
    int flip_sign;
    char *temp;

    arg = buffer;
    flip_sign = 1;
    specs->_n0 = 0;
    switch (specs->_character) {
    case 'd':
    case 'i':
        break;
    case 'o':
    case 'u':
    case 'x':
    case 'X':
        if (FLAG(specs, SPF_LONG)) {
            long_value = *(unsigned long *)(((*pargs)[0] += 4) - 4);
        } else if (FLAG(specs, SPF_SHORT)) {
            long_value = *(unsigned short *)(((*pargs)[0] += 4) - 4);
        } else {
            long_value = *(unsigned *)(((*pargs)[0] += 4) - 4);
        }
        specs->_flags &= ~(SPF_BLANK | SPF_FORCE_SIGN);
        break;
    default:
        goto no_prec;
    }
    if (specs->_prec != -1)
        specs->_pad_char = ' ';
no_prec:
    radix = 10;
    switch (specs->_character) {
    case 'f':
    case 'F':
        if (FLAG(specs, SPF_SHORT)) {
            FixedPoint_Format(buffer, *(long *)(((*pargs)[0] += 4) - 4), specs);
            arg = buffer;
            length = far_strlen(arg, -1);
            break;
        }
    case 'e':
    case 'E':
    case 'g':
    case 'G':
        arg = float_format(buffer, pargs, specs);
        length = far_strlen(arg, -1);
        break;
    case 'o':
        radix = 8;
        if (FLAG(specs, SPF_ALT)) {
            *buffer++ = '0';
        }
        ltoa(long_value, buffer, radix);
    check_zero:
        if (specs->_prec == 0) {
            if ((int)*buffer == '0') {
                *arg = '\0';
            }
        }
    no_zero:
        length = far_strlen(arg, -1);
        break;
    case 'd':
    case 'i':
        if (FLAG(specs, SPF_LONG)) {
            ltoa(*(long *)(((*pargs)[0] += 4) - 4), buffer, radix);
        } else if (FLAG(specs, SPF_SHORT)) {
            ltoa(*(short *)(((*pargs)[0] += 4) - 4), buffer, radix);
        } else {
            itoa(*(int *)(((*pargs)[0] += 4) - 4), buffer, radix);
        }
        goto check_zero;
    case 's':
    case 'S':
        if (FLAG(specs, SPF_FAR)) {
            arg = *(FAR_STRING *)(((*pargs)[0] += 8) - 8);
            if (NULL == arg)
                arg = null_string;
        } else if (FLAG(specs, SPF_NEAR)) {
            if ((temp = *(char **)(((*pargs)[0] += 4) - 4)) != NULL)
                arg = temp;
            else
                arg = null_string;
        } else {
            if ((temp = *(char **)(((*pargs)[0] += 4) - 4)) != NULL)
                arg = temp;
            else
                arg = null_string;
        }
        flip_sign = 0;
        specs->_flags &= ~(SPF_BLANK | SPF_FORCE_SIGN);
        if ((int)specs->_character == 'S') {
            if (FLAG(specs, SPF_LONG)) {
                length = *(unsigned short __far *)arg;
                arg += 2;
            } else {
                length = *arg++;
            }
        } else if (FLAG(specs, SPF_LONG)) {
            length = far_other_strlen(arg, specs->_prec);
        } else {
            length = far_strlen(arg, specs->_prec);
        }
        goto done;
    case 'x':
    case 'X':
        if (FLAG(specs, SPF_ALT) && long_value != 0) {
            specs->_prefix[0] = '0';
            specs->_prefix[1] = specs->_character;
            specs->_prefix[2] = '\0';
        }
        radix = 16;
    case 'u':
        ultoa(long_value, buffer, radix);
        if ((int)specs->_character == 'X') {
            zupstr(buffer);
        }
        goto check_zero;
    case 'p':
    case 'P':
        if (specs->_fld_width == 0) {
            if (FLAG(specs, SPF_FAR)) {
                specs->_fld_width = 13;
            } else {
                specs->_fld_width = 8;
            }
        }
        specs->_flags &= ~(SPF_BLANK | SPF_FORCE_SIGN);
        long_value = *(unsigned long *)(((*pargs)[0] += 4) - 4);
        if (FLAG(specs, SPF_FAR)) {
            fmt4hex(*(unsigned *)(((*pargs)[0] += 4) - 4) & 0xffff, buffer, 4);
            buffer[4] = ':';
            fmt4hex(long_value, buffer + 5, 8);
        } else {
            fmt4hex(long_value, buffer, 8);
        }
        if ((int)specs->_character == 'P') {
            zupstr(buffer);
        }
        goto no_zero;
    case 'c':
        buffer[0] = *(int *)(((*pargs)[0] += 4) - 4);
        buffer[1] = '\0';
        specs->_prec = 1;
        flip_sign = 0;
        specs->_flags &= ~(SPF_BLANK | SPF_FORCE_SIGN);
        length = 1;
        goto done;
    default:
        specs->_fld_width = 0;
        buffer[0] = specs->_character;
        buffer[1] = '\0';
        specs->_prec = 1;
        flip_sign = 0;
        specs->_flags &= ~(SPF_BLANK | SPF_FORCE_SIGN);
        length = 1;
        goto done;
    }
done:
    if (flip_sign) {
        if ((int)*arg == '-') {
            ++arg;
            specs->_prefix[0] = '-';
            --length;
            specs->_prefix[1] = '\0';
        } else if (FLAG(specs, SPF_BLANK)) {
            specs->_prefix[0] = ' ';
            specs->_prefix[1] = '\0';
        } else if (FLAG(specs, SPF_FORCE_SIGN)) {
            specs->_prefix[0] = '+';
            specs->_prefix[1] = '\0';
        }
        if (length > specs->_prec) {
            specs->_prec = length;
        } else {
            specs->_n0 = specs->_prec - length;
        }
    }
    if ((int)specs->_pad_char == '*') {
        specs->_prefix[0] = '\0';
        specs->_flags &= ~(SPF_BLANK | SPF_FORCE_SIGN);
    }
    if (specs->_prec == -1 || length < specs->_prec) {
        if ((int)specs->_character != 'c') {
            specs->_prec = length;
        }
    }
    specs->_fld_width -= far_strlen(specs->_prefix, -1) + specs->_prec + specs->_n0;
    return arg;
}

// zupstr: uppercase string p in place.
// FUNCTION: 0xd00c4
void zupstr(char *p)
{
    while (*p) {
        *p = toupper(*p);
        ++p;
    }
}

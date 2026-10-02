#include "clib.h"

#define SPF_ALT 0x01

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

#define _RND(t) ((sizeof(t) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#define va_arg(ap, t) (*(t *)(((ap)[0] += _RND(t)) - _RND(t)))

void _FtoS(char *buf, double *value, int digits, char plus, int width,
           int scale, int exp_digits, char exp_char, char fmt);
double strtod(const char *s, char **endptr);

static void forcedecpt(char *p, SPECS *specs);

// _EFG_Format: printf's %e/%f/%g conversion; formats the next double from args per specs into buffer; returns the start of the text.
// FUNCTION: 0xf744b
FAR_STRING _EFG_Format(char *buffer, va_list *args, SPECS *specs)
{
    int digits;
    char fmt;
    double value;
    FAR_STRING result;

    result = (FAR_STRING)buffer;
    specs->_n0 = 0;
    fmt = specs->_character;
    digits = 0;
    switch (fmt) {
    case 'G':
    case 'g':
        if (specs->_prec == 0)
            specs->_prec = 1;
        fmt -= 2;
    case 'E':
    case 'e':
        digits = 1;
    case 'F':
    case 'f':
        value = va_arg(*args, double);
        if (specs->_prec == -1)
            specs->_prec = 6;
        _FtoS(buffer, &value, specs->_prec, 0, 39, digits, 3, fmt, specs->_character & 0x5f);
        if ((int)*buffer == '*')
            specs->_pad_char = '*';
        buffer[39] = '\0';
        while ((int)*buffer == ' ')
            ++buffer;
        forcedecpt(buffer, specs);
        result = (FAR_STRING)buffer;
        specs->_prec = -1;
        break;
    }
    return result;
}

// forcedecpt: unless '#' was given, remove a trailing '.' (and, for %g, trailing zeros) before any exponent in p.
// FUNCTION: 0xf752f
static void forcedecpt(char *p, SPECS *specs)
{
    char *exp;

    if ((int)(unsigned char)(specs->_flags & SPF_ALT) != 0)
        return;
    while (*p != '\0' && (int)*p != 'e' && (int)*p != 'E')
        ++p;
    exp = p;
    --p;
    if ((int)specs->_character == 'G' || (int)specs->_character == 'g') {
        while ((int)*p == '0')
            --p;
    }
    if ((int)*p != '.')
        ++p;
    while ((*p++ = *exp++) != '\0')
        ;
}

void __cnvs2d(char *buf, double *value);

// __cnvs2d: scanf's floating-point conversion; *value = strtod(buf).
// FUNCTION: 0xf758e
void __cnvs2d(char *buf, double *value)
{
    *value = strtod(buf, NULL);
}

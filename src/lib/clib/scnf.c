#include "clib.h"

typedef struct SCNF_SPECS {
    int (*cget_rtn)(struct SCNF_SPECS *specs);
    void (*uncget_rtn)(int c, struct SCNF_SPECS *specs);
    char *ptr;
    int width;
    unsigned assign : 1;
    unsigned eoinp : 1;
    unsigned far_ptr : 1;
    unsigned near_ptr : 1;
    unsigned short_var : 1;
    unsigned long_var : 1;
    unsigned long_double_var : 1;
    unsigned p_format : 1;
} SCNF_SPECS;

#define CHARVECTSIZE 32

#undef isspace
#define isspace(c) ((int)(unsigned char)(_IsTable[(unsigned char)((c) + 1)] & _SPACE) != 0)
#undef isdigit
#define isdigit(c) ((int)(unsigned char)(_IsTable[(unsigned char)((c) + 1)] & _DIGIT) != 0)

#define _RND(t) ((sizeof(t) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#define va_arg(ap, t) (*(t *)(((ap)[0] += _RND(t)) - _RND(t)))

extern const unsigned char lst_mask[8];

void *memset(void *s, int c, size_t n);
int tolower(int c);

static int scan_white(SCNF_SPECS *specs);
static int scan_char(SCNF_SPECS *specs, va_list *arg);
static int scan_string(SCNF_SPECS *specs, va_list *arg);
static int scan_arb(SCNF_SPECS *specs, va_list *arg, const char **format);
static int scan_float(SCNF_SPECS *specs, va_list *arg);
static int scan_int(SCNF_SPECS *specs, va_list *arg, int base, int sign_flag);
static const char *get_opt(const char *opt_str, SCNF_SPECS *specs);
static void report_scan(SCNF_SPECS *specs, va_list *arg, int match);

// FUNCTION: 0xfaaf2
static int cget(SCNF_SPECS *specs)
{
    return (*specs->cget_rtn)(specs);
}

// FUNCTION: 0xfaaf9
static void uncget(int c, SCNF_SPECS *specs)
{
    (*specs->uncget_rtn)(c, specs);
}

// NONMATCHING: format and items_assigned swap stack slots
// FUNCTION: 0xfab01
int __scnf(SCNF_SPECS *specs, const char *format, va_list *args)
{
    int char_match;
    int items_assigned;
    int match_len;
    int c;
    int fmt_chr;

    char_match = items_assigned = 0;
    specs->eoinp = 0;
    do {
        fmt_chr = *format++;
        if (fmt_chr == '\0')
            goto fail;
        if (isspace(fmt_chr)) {
            char_match += scan_white(specs);
            continue;
        }
        if (fmt_chr != '%') {
            if ((c = cget(specs)) != fmt_chr) {
                if (!specs->eoinp)
                    uncget(c, specs);
                goto fail;
            }
            ++char_match;
            continue;
        }
        format = get_opt(format, specs);
        if ((fmt_chr = *format) != '\0')
            ++format;
        switch (fmt_chr) {
        case 'd':
            match_len = scan_int(specs, args, 10, 1);
            goto check_match;
        case 'i':
            match_len = scan_int(specs, args, 0, 1);
            goto check_match;
        case 'o':
            match_len = scan_int(specs, args, 8, 1);
            goto check_match;
        case 'u':
            match_len = scan_int(specs, args, 10, 0);
            goto check_match;
        case 'p':
        case 'x':
        case 'X':
            match_len = scan_int(specs, args, 16, 1);
            goto check_match;
        case 'e':
        case 'f':
        case 'g':
        case 'E':
        case 'G':
            match_len = scan_float(specs, args);
            goto check_match;
        case 's':
            match_len = scan_string(specs, args);
            goto check_match;
        case '[':
            match_len = scan_arb(specs, args, &format);
            goto check_match;
        case 'c':
            match_len = scan_char(specs, args);
        check_match:
            if (match_len <= 0)
                goto fail;
            char_match += match_len;
            if (specs->assign)
                ++items_assigned;
            break;
        case 'n':
            report_scan(specs, args, char_match);
            break;
        case '%':
            if ((c = cget(specs)) != fmt_chr) {
                if (!specs->eoinp)
                    uncget(c, specs);
                goto fail;
            }
            ++char_match;
            break;
        }
    } while (!specs->eoinp);
    if ((int)*format == '%') {
        ++format;
        format = get_opt(format, specs);
        if ((int)*format == 'n')
            report_scan(specs, args, char_match);
    }
fail:
    if (items_assigned == 0 && specs->eoinp)
        return EOF;
    return items_assigned;
}

// FUNCTION: 0xfad48
static const char *get_opt(const char *opt_str, SCNF_SPECS *specs)
{
    int c;
    int width;

    specs->assign = 1;
    specs->far_ptr = 0;
    specs->near_ptr = 0;
    specs->short_var = 0;
    specs->long_var = 0;
    specs->long_double_var = 0;
    specs->p_format = 0;
    specs->width = -1;
    if ((int)*opt_str == '*') {
        specs->assign = 0;
        ++opt_str;
    }
    c = *opt_str;
    if (isdigit(c)) {
        width = 0;
        do {
            width *= 10;
            width += c - '0';
            c = *++opt_str;
        } while (isdigit(c));
        specs->width = width;
    }
    if ((int)*opt_str == 'N') {
        specs->near_ptr = 1;
        ++opt_str;
    } else if ((int)*opt_str == 'F') {
        specs->far_ptr = 1;
        ++opt_str;
    }
    switch (*opt_str) {
    case 'h':
        specs->short_var = 1;
        ++opt_str;
        break;
    case 'l':
    case 'w':
        specs->long_var = 1;
        ++opt_str;
        break;
    case 'L':
        specs->long_double_var = 1;
        ++opt_str;
        break;
    }
    return opt_str;
}

// FUNCTION: 0xfae3d
static int scan_white(SCNF_SPECS *specs)
{
    int c;
    int len;

    len = 0;
    for (;;) {
        c = cget(specs);
        if (!isspace(c))
            break;
        ++len;
    }
    if (!specs->eoinp)
        uncget(c, specs);
    return len;
}

// FUNCTION: 0xfae7c
static int scan_char(SCNF_SPECS *specs, va_list *arg)
{
    int len;
    int width;
    char __far *str;
    int c;

    if (specs->assign) {
        if (specs->far_ptr) {
            str = va_arg(*arg, char __far *);
        } else if (specs->near_ptr) {
            str = va_arg(*arg, char *);
        } else {
            str = va_arg(*arg, char *);
        }
    }
    len = 0;
    if ((width = specs->width) == -1)
        width = 1;
    while (width > 0) {
        c = cget(specs);
        if (specs->eoinp)
            break;
        ++len;
        --width;
        if (specs->assign) {
            if (specs->long_var) {
                *(short __far *)str = c;
                str += 2;
            } else {
                *str++ = c;
            }
        }
    }
    return len;
}

// FUNCTION: 0xfaf11
static int scan_string(SCNF_SPECS *specs, va_list *arg)
{
    int c;
    int len;
    char __far *str;
    char chsize;

    if (specs->long_var) {
        chsize = 2;
    } else if (specs->short_var) {
        chsize = 1;
    } else {
        chsize = 1;
    }
    if (specs->assign) {
        if (specs->far_ptr) {
            str = va_arg(*arg, char __far *);
        } else if (specs->near_ptr) {
            str = va_arg(*arg, char *);
        } else {
            str = va_arg(*arg, char *);
        }
    }
    len = 0;
    for (;;) {
        c = cget(specs);
        if (!isspace(c))
            break;
        ++len;
    }
    if (specs->eoinp) {
        len = 0;
        goto done;
    }
    if (specs->width-- == 0)
        goto ugdone;
    do {
        ++len;
        if (specs->assign) {
            if ((int)chsize == 1) {
                *str = c;
            } else {
                *(short __far *)str = c;
            }
            str += chsize;
        }
        if ((c = cgetw(specs)) == EOF)
            goto done;
    } while (!isspace(c));
ugdone:
    uncget(c, specs);
done:
    if (specs->assign && len > 0) {
        if ((int)chsize == 1) {
            *str = '\0';
        } else {
            *(short __far *)str = 0;
        }
    }
    return len;
}

// FUNCTION: 0xfb034
static void report_scan(SCNF_SPECS *specs, va_list *arg, int match)
{
    int __far *iptr;

    if (specs->assign) {
        if (specs->far_ptr) {
            iptr = va_arg(*arg, int __far *);
        } else if (specs->near_ptr) {
            iptr = va_arg(*arg, int *);
        } else {
            iptr = va_arg(*arg, int *);
        }
        if (specs->short_var) {
            *(short __far *)iptr = match;
        } else if (specs->long_var) {
            *(long __far *)iptr = match;
        } else {
            *iptr = match;
        }
    }
}

// FUNCTION: 0xfb092
static const char *makelist(const char *format, char *charset)
{
    int c;

    memset(charset, 0, CHARVECTSIZE);
    if ((c = *format++) != '\0') {
        do {
            charset[c >> 3] |= lst_mask[c & 7];
            if ((c = *format) == '\0')
                break;
            ++format;
        } while (c != ']');
    }
    return format;
}

// FUNCTION: 0xfb0d5
static int scan_arb(SCNF_SPECS *specs, va_list *arg, const char **format)
{
    unsigned width;
    int c;
    int len;
    int not_flag;
    char __far *str;
    char charset[CHARVECTSIZE];

    if (not_flag = ((int)**format == '^'))
        ++(*format);
    *format = makelist(*format, charset);
    if (specs->assign) {
        if (specs->far_ptr) {
            str = va_arg(*arg, char __far *);
        } else if (specs->near_ptr) {
            str = va_arg(*arg, char *);
        } else {
            str = va_arg(*arg, char *);
        }
    }
    len = 0;
    width = specs->width;
    while (width > 0) {
        c = cget(specs);
        if (specs->eoinp)
            break;
        if (((charset[c >> 3] & lst_mask[c & 7]) == 0) != not_flag) {
            uncget(c, specs);
            break;
        }
        ++len;
        --width;
        if (specs->assign)
            *str++ = c;
    }
    if (specs->assign && len > 0)
        *str = '\0';
    return len;
}

typedef union {
    unsigned long uWhole;
    long sWhole;
    struct {
        unsigned short lo;
        unsigned short hi;
    } wd;
    struct {
        unsigned char b1;
        unsigned char b2;
        unsigned char b3;
        unsigned char b4;
    } bt;
} T32;

extern void (*__EFG_scanf)(char *buf, double *ld);

// NONMATCHING: the sign branch counts pref_len in ecx instead of ebp, and
// at is cleared through ebp (shared with digit_found) instead of eax
// FUNCTION: 0xfb1cf
static int scan_float(SCNF_SPECS *specs, va_list *arg)
{
    T32 at;
    double ld;
    char *p;
    T32 ft;
    char buf[80];
    int c;
    int pref_len;
    int len;
    int digit_found;
    float __far *fptr;
    char *num_str;

    num_str = buf;
    pref_len = len = 0;
    for (;;) {
        c = cget(specs);
        if (!isspace(c))
            break;
        ++pref_len;
    }
    if (specs->eoinp)
        goto done;
    if (specs->width-- == 0)
        goto ugdone;
    if (c == '+' || c == '-') {
        *num_str++ = c;
        ++pref_len;
        if ((c = cgetw(specs)) == EOF)
            goto done;
    }
    if (!(isdigit(c) || c == '.'))
        goto ugdone;
    at.uWhole = 0;
    digit_found = 0;
    if (isdigit(c)) {
        digit_found = 1;
        do {
            *num_str++ = c;
            if (specs->short_var)
                at.wd.hi = at.wd.hi * 10 + c - '0';
            ++len;
            if ((c = cgetw(specs)) == EOF)
                goto done;
        } while (isdigit(c));
    }
    if (c == '.') {
        *num_str++ = c;
        if ((c = cgetw(specs)) == EOF)
            goto done;
        if (!(digit_found || isdigit(c)))
            goto ugdone;
        ++len;
        while (isdigit(c)) {
            ++len;
            *num_str++ = c;
            if ((c = cgetw(specs)) == EOF)
                break;
        }
        if (specs->short_var) {
            p = num_str;
            ft.uWhole = 0;
            while ((int)*--p != '.') {
                ft.bt.b3 = *p - '0';
                ft.uWhole /= 10;
            }
            at.wd.lo = ft.wd.lo;
        }
        if (c == EOF)
            goto done;
    }
    if (specs->short_var)
        goto ugdone;
    if (c == 'e' || c == 'E') {
        ++len;
        *num_str++ = c;
        if ((c = cgetw(specs)) == EOF)
            goto done;
        if (c == '+' || c == '-') {
            ++len;
            *num_str++ = c;
            if ((c = cgetw(specs)) == EOF)
                goto done;
        }
        if (!isdigit(c)) {
            len = 0;
            goto ugdone;
        }
        do {
            ++len;
            *num_str++ = c;
            if ((c = cgetw(specs)) == EOF)
                goto done;
        } while (isdigit(c));
    }
ugdone:
    uncget(c, specs);
done:
    if (len > 0) {
        len += pref_len;
        if (specs->assign) {
            *num_str = '\0';
            if (specs->short_var) {
                if ((int)buf[0] == '-')
                    at.sWhole = -at.sWhole;
            } else {
                (*__EFG_scanf)(buf, &ld);
            }
            if (specs->far_ptr) {
                fptr = va_arg(*arg, float __far *);
            } else if (specs->near_ptr) {
                fptr = va_arg(*arg, float *);
            } else {
                fptr = va_arg(*arg, float *);
            }
            if (specs->short_var) {
                *(long __far *)fptr = at.uWhole;
            } else if (specs->long_var || specs->long_double_var) {
                *(double __far *)fptr = ld;
            } else {
                *fptr = ld;
            }
        }
    }
    return len;
}

// NONMATCHING: stack slot assignment of pref_len, sign and arg
// FUNCTION: 0xfb4e6
static int scan_int(SCNF_SPECS *specs, va_list *arg, int base, int sign_flag)
{
    long value;
    int len;
    int pref_len;
    int c;
    int sign;
    int digit;
    int __far *iptr;

    value = 0;
    len = 0;
    pref_len = 0;
    for (;;) {
        c = cget(specs);
        if (!isspace(c))
            break;
        ++pref_len;
    }
    if (specs->eoinp)
        goto done;
    if (specs->width-- == 0)
        goto ugdone;
    sign = '+';
    if (sign_flag && (c == '+' || c == '-')) {
        ++pref_len;
        sign = c;
        if ((c = cgetw(specs)) == EOF)
            goto done;
    }
    if (base == 0) {
        if (c == '0') {
            len = 1;
            if ((c = cgetw(specs)) == EOF)
                goto done;
            if (c == 'x' || c == 'X') {
                c = cgetw(specs);
                len = 0;
                pref_len += 2;
                if (c == EOF)
                    goto done;
                base = 16;
            } else {
                base = 8;
            }
        } else {
            base = 10;
        }
    } else if (base == 16) {
        if (c == '0') {
            len = 1;
            if ((c = cgetw(specs)) == EOF)
                goto done;
            if (c == 'x' || c == 'X') {
                c = cgetw(specs);
                pref_len += 2;
                len = 0;
                if (c == EOF)
                    goto done;
            }
        }
    }
    for (;;) {
        digit = radix_value(c);
        if (digit >= base)
            break;
        value = value * base + digit;
        ++len;
        c = cgetw(specs);
    }
    if (c == ':' && specs->p_format) {
        for (;;) {
            ++len;
            if ((c = cgetw(specs)) == EOF)
                goto done;
            digit = radix_value(c);
            if (digit >= base)
                break;
            value = value * base + digit;
        }
    }
ugdone:
    uncget(c, specs);
done:
    if (sign == '-')
        value = -value;
    if (len > 0) {
        len += pref_len;
        if (specs->assign) {
            if (specs->far_ptr) {
                iptr = va_arg(*arg, int __far *);
            } else if (specs->near_ptr) {
                iptr = va_arg(*arg, int *);
            } else {
                iptr = va_arg(*arg, int *);
            }
            if (specs->short_var) {
                *(short __far *)iptr = value;
            } else if (specs->long_var) {
                *(long __far *)iptr = value;
            } else {
                *iptr = value;
            }
        }
    }
    return len;
}

static int radix_value(int c);
/* config/functions.txt has no name for radix_value. */
#pragma aux radix_value "fn_000fb6d6_"

// FUNCTION: 0xfb6d6
static int radix_value(int c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    c = tolower(c);
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return 16;
}

// FUNCTION: 0xfb6fd
static int cgetw(SCNF_SPECS *specs)
{
    int c;

    if (specs->width-- == 0)
        return EOF;
    c = cget(specs);
    return !specs->eoinp ? c : EOF;
}

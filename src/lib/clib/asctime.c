#include "clib.h"

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

typedef long time_t;

typedef struct {
    int quot;
    int rem;
} div_t;

div_t div(int numer, int denom);
#pragma intrinsic(div)

extern const char D_00128e4c[3][12];
extern const char D_00128e70[3][7];
extern char result[];

struct tm *_localtime(const time_t *timer, struct tm *tm);
char *asctime(const struct tm *time);

// FUNCTION: 0xfa9a7
static void convDec(int num, int off, char *buf)
{
    div_t d;

    d = div(num, 10);
    buf[off] = d.quot + '0';
    buf[off + 1] = d.rem + '0';
}

// FUNCTION: 0xfa9da
char *_asctime(const struct tm *time, char *buf)
{
    div_t d;
    int i;

    i = time->tm_wday;
    buf[0] = D_00128e70[0][i];
    buf[1] = D_00128e70[1][i];
    buf[2] = D_00128e70[2][i];
    buf[3] = ' ';
    i = time->tm_mon;
    buf[4] = D_00128e4c[0][i];
    buf[5] = D_00128e4c[1][i];
    buf[6] = D_00128e4c[2][i];
    buf[7] = ' ';
    convDec(time->tm_mday, 8, buf);
    if ((int)buf[8] == '0')
        buf[8] = ' ';
    buf[10] = ' ';
    convDec(time->tm_hour, 11, buf);
    buf[13] = ':';
    convDec(time->tm_min, 14, buf);
    buf[16] = ':';
    convDec(time->tm_sec, 17, buf);
    buf[19] = ' ';
    d = div(time->tm_year, 100);
    convDec(d.quot + 19, 20, buf);
    convDec(d.rem, 22, buf);
    buf[24] = '\n';
    buf[25] = '\0';
    return buf;
}

// FUNCTION: 0xfaac7
char *_ctime(const time_t *timer, char *buf)
{
    struct tm tm;

    return _asctime(_localtime(timer, &tm), buf);
}

struct tm *localtime(const time_t *timer);

// FUNCTION: 0xfaae0
char *ctime(const time_t *timer)
{
    return asctime(localtime(timer));
}

// FUNCTION: 0xfaae5
char *asctime(const struct tm *time)
{
    return _asctime(time, result);
}

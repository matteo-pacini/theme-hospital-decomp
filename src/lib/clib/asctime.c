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

static const unsigned char Months[3][12] = {
    "JFMAMJJASOND",
    "aeapauuuecoe",
    "nbrrynlgptvc"
};
static const unsigned char Days[3][7] = {
    "SMTWTFS",
    "uouehra",
    "nneduit"
};
extern char result[];

struct tm *_localtime(const time_t *timer, struct tm *tm);
char *asctime(const struct tm *timeptr);

// convDec: write num (0-99) as two decimal digits at buf[off].
// FUNCTION: 0xfa9a7
static void convDec(int num, int off, char *buf)
{
    div_t d;

    d = div(num, 10);
    buf[off] = d.quot + '0';
    buf[off + 1] = d.rem + '0';
}

// _asctime: format *timeptr as "Ddd Mmm dd hh:mm:ss yyyy\n" into buf; returns buf.
// FUNCTION: 0xfa9da
char *_asctime(const struct tm *timeptr, char *buf)
{
    div_t d;
    int i;

    i = timeptr->tm_wday;
    buf[0] = Days[0][i];
    buf[1] = Days[1][i];
    buf[2] = Days[2][i];
    buf[3] = ' ';
    i = timeptr->tm_mon;
    buf[4] = Months[0][i];
    buf[5] = Months[1][i];
    buf[6] = Months[2][i];
    buf[7] = ' ';
    convDec(timeptr->tm_mday, 8, buf);
    if ((int)buf[8] == '0')
        buf[8] = ' ';
    buf[10] = ' ';
    convDec(timeptr->tm_hour, 11, buf);
    buf[13] = ':';
    convDec(timeptr->tm_min, 14, buf);
    buf[16] = ':';
    convDec(timeptr->tm_sec, 17, buf);
    buf[19] = ' ';
    d = div(timeptr->tm_year, 100);
    convDec(d.quot + 19, 20, buf);
    convDec(d.rem, 22, buf);
    buf[24] = '\n';
    buf[25] = '\0';
    return buf;
}

// _ctime: format *timer as local time into buf (asctime layout); returns buf.
// FUNCTION: 0xfaac7
char *_ctime(const time_t *timer, char *buf)
{
    struct tm tm;

    return _asctime(_localtime(timer, &tm), buf);
}

struct tm *localtime(const time_t *timer);

// ctime: *timer as a local-time asctime string in a static buffer.
// FUNCTION: 0xfaae0
char *ctime(const time_t *timer)
{
    return asctime(localtime(timer));
}

// asctime: format *timeptr as "Ddd Mmm dd hh:mm:ss yyyy\n" in a static buffer; returns it.
// FUNCTION: 0xfaae5
char *asctime(const struct tm *timeptr)
{
    return _asctime(timeptr, result);
}

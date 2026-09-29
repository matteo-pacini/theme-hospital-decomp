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

#define DAYS_1900_TO_1970 25567

extern long timezone;
extern long __dst_adjust;
static struct tm The_time;

struct tm *__brktime(unsigned long days, time_t wallclock, long gmtdelta, struct tm *t);
int __isindst(struct tm *t);
void tzset(void);

// FUNCTION: 0xfa949
struct tm *_localtime(const time_t *timer, struct tm *tm)
{
    time_t t;

    tzset();
    t = *timer;
    __brktime(DAYS_1900_TO_1970, t, timezone, tm);
    if (__isindst(tm))
        __brktime(DAYS_1900_TO_1970, t, timezone - __dst_adjust, tm);
    return tm;
}

// FUNCTION: 0xfa99a
struct tm *localtime(const time_t *timer)
{
    return _localtime(timer, &The_time);
}

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

#define SECONDS_PER_DAY (24UL * 60UL * 60UL)

extern short const __diyr[];
extern short const __dilyr[];
extern struct tm The_time;

int __leapyear(unsigned year);

// FUNCTION: 0xf37a0
// NONMATCHING: register allocation (the target keeps t in ecx)
struct tm *__brktime(unsigned long days, unsigned long wallclock, long gmtdelta, struct tm *t)
{
    unsigned long secs;
    unsigned year;
    unsigned day_of_year;
    unsigned month;
    unsigned year_days;
    short const *month_start;

    if (wallclock < 12UL * 60UL * 60UL && gmtdelta > 0) {
        wallclock = wallclock + SECONDS_PER_DAY - gmtdelta;
        days += wallclock / SECONDS_PER_DAY;
        --days;
    } else {
        wallclock -= gmtdelta;
        days += wallclock / SECONDS_PER_DAY;
    }
    secs = wallclock % SECONDS_PER_DAY;
    t->tm_hour = secs / 3600;
    secs = secs % 3600;
    t->tm_min = secs / 60;
    t->tm_sec = secs % 60;
    year = days / 366;
    day_of_year = days - year * 365;
    if (year > 0)
        day_of_year -= (year - 1) >> 2;
    while (day_of_year >= (year_days = __leapyear(year + 1900) + 365)) {
        ++year;
        day_of_year -= year_days;
    }
    t->tm_year = year;
    t->tm_yday = day_of_year;
    month_start = __diyr;
    if (__leapyear(year + 1900))
        month_start = __dilyr;
    month = day_of_year / 31;
    if (day_of_year >= month_start[month + 1])
        ++month;
    t->tm_mon = month;
    t->tm_mday = day_of_year - month_start[month] + 1;
    t->tm_wday = (days + 1) % 7;
    return t;
}

// FUNCTION: 0xf38b5
struct tm *_gmtime(const time_t *timer, struct tm *tm)
{
    tm->tm_isdst = 0;
    return __brktime(25567, *timer, 0, tm);
}

// FUNCTION: 0x000f38d3
struct tm *gmtime(const time_t *timer)
{
    return _gmtime(timer, &The_time);
}

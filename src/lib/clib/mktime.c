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

#define MONTH_YR 12
#define DAY_YR 365
#define SECONDS_DAY (24L * 60L * 60L)
#define DAYS_1900_TO_1970 25567
#define SMALLEST_YEAR_VALUE (-184844639)

extern short const __diyr[];
extern short const D_00128e0e[];
extern long timezone;
extern long __dst_adjust;

int __leapyear(unsigned year);
struct tm *__brktime(unsigned long days, time_t wallclock, long gmtdelta, struct tm *t);
void tzset(void);
int __isindst(struct tm *t);

// FUNCTION: 0xeb86a
// NONMATCHING: days is summed in a different order and register
time_t mktime(struct tm *t)
{
    long days;
    long seconds;
    int month_index;
    int year;
    short const *month_start;

    month_start = __diyr;
    month_index = t->tm_mon % MONTH_YR;
    year = t->tm_year;
    if (year < SMALLEST_YEAR_VALUE)
        return -1;
    year += t->tm_mon / MONTH_YR;
    while (month_index < 0) {
        --year;
        month_index += MONTH_YR;
    }
    if (year < 0)
        return -1;
    if (__leapyear(year + 1900))
        month_start = D_00128e0e;
    days = year * DAY_YR + ((year + 3) >> 2) + month_start[month_index] + t->tm_mday - 1;
    if (year != 0)
        --days;
    seconds = (t->tm_hour * 60L + t->tm_min) * 60L + t->tm_sec;
    while (seconds < 0) {
        --days;
        seconds += SECONDS_DAY;
    }
    __brktime(days, seconds, 0L, t);
    tzset();
    seconds += timezone;
    if (t->tm_isdst < 0)
        __isindst(t);
    if (t->tm_isdst > 0)
        seconds -= __dst_adjust;
    while (seconds < 0) {
        --days;
        seconds += SECONDS_DAY;
    }
    if (days < DAYS_1900_TO_1970 - 1)
        return -1;
    if (days == DAYS_1900_TO_1970 - 1) {
        seconds -= SECONDS_DAY;
        if (timezone <= 0 || seconds < 0)
            return -1;
    } else {
        seconds += (days - DAYS_1900_TO_1970) * SECONDS_DAY;
    }
    return seconds;
}

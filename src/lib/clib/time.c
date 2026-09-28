#include "clib.h"

typedef long time_t;

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

int __getctime(struct tm *t);
time_t mktime(struct tm *t);

// FUNCTION: 0xce95a
time_t time(time_t *timer)
{
    struct tm t;
    time_t seconds;
    int milliseconds;

    milliseconds = __getctime(&t);
    if (milliseconds >= 500)
        t.tm_sec++;
    seconds = mktime(&t);
    if (timer != NULL)
        *timer = seconds;
    return seconds;
}

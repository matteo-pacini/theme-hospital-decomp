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

#define TZNAME_MAX 30

void *memcpy(void *dst, const void *src, size_t n);
#pragma intrinsic(memcpy)

char *getenv(const char *name);

extern const char D_00128484[];
extern char D_00152294[];
extern char dtzone[];
extern long timezone;
extern int daylight;
extern long __dst_adjust;
extern struct tm __start_dst;
extern struct tm __end_dst;

struct tzflags {
    unsigned cache_OS_TZ : 1;
    unsigned have_OS_TZ : 1;
};
extern struct tzflags D_001522da;

void tryOSTimeZone(void);
char *parse_time(char *tz, int *val);
char *parse_offset(char *tz, char *name, long *offset);
char *parse_rule(char *tz, struct tm *timeptr);
void __parse_tz(char *tz);

// FUNCTION: 0xf38e0
int __CacheOSTZ(int flag)
{
    int old;

    old = D_001522da.cache_OS_TZ;
    D_001522da.cache_OS_TZ = ~old;
    D_001522da.have_OS_TZ = flag != 0;
    return old;
}

// FUNCTION: 0xf3934
void tryOSTimeZone(void)
{
    if (!D_001522da.cache_OS_TZ || !D_001522da.have_OS_TZ)
        D_001522da.have_OS_TZ = 1;
}

// FUNCTION: 0xf394c
void tzset(void)
{
    char *tz;

    if ((tz = getenv(D_00128484)) != NULL) {
        __parse_tz(tz);
    } else {
        tryOSTimeZone();
    }
}

// FUNCTION: 0xf3991
char *parse_time(char *tz, int *val)
{
    int value;

    value = 0;
    while ((int)*tz >= '0' && (int)*tz <= '9') {
        value = value * 10 + *tz - '0';
        ++tz;
    }
    *val = value;
    return tz;
}

// FUNCTION: 0xf39b5
// NONMATCHING: name and len swap stack slots, and the digit test after the
// sign zero-extends *tz with xor/mov instead of and
char *parse_offset(char *tz, char *name, long *offset)
{
    int len;
    int hours;
    int minutes;
    int seconds;
    int neg;
    char *tzstart;
    char c;

    if ((int)*tz == ':')
        ++tz;
    tzstart = tz;
    while ((c = *tz) != '\0' && (int)c != ',' && (int)c != '-' && (int)c != '+'
           && ((int)c < '0' || (int)c > '9'))
        ++tz;
    len = tz - tzstart;
    if (len > TZNAME_MAX)
        len = TZNAME_MAX;
    memcpy(name, tzstart, len);
    name[len] = '\0';
    neg = 0;
    if ((int)c == '-') {
        neg = 1;
        ++tz;
    } else if ((int)c == '+') {
        ++tz;
    }
    if ((int)*tz >= '0' && (int)*tz <= '9') {
        hours = minutes = seconds = 0;
        tz = parse_time(tz, &hours);
        if ((int)*tz == ':') {
            tz = parse_time(tz + 1, &minutes);
            if ((int)*tz == ':')
                tz = parse_time(tz + 1, &seconds);
        }
        *offset = seconds + ((minutes + (hours * 60)) * 60L);
        if (neg)
            *offset = -*offset;
    }
    return tz;
}

// FUNCTION: 0xf3ade
char *parse_rule(char *tz, struct tm *timeptr)
{
    int date_form;
    int days;
    int hours;
    int minutes;
    int seconds;

    date_form = -1;
    if ((int)*tz == 'J') {
        date_form = 1;
        tz++;
    }
    if ((int)*tz == 'M') {
        date_form = 0;
        tz++;
    }
    timeptr->tm_isdst = date_form;
    tz = parse_time(tz, &days);
    if (date_form != 0) {
        timeptr->tm_yday = days;
    } else {
        timeptr->tm_mon = days - 1;
        if ((int)*tz == '.') {
            tz = parse_time(tz + 1, &days);
            timeptr->tm_mday = days;
            if ((int)*tz == '.') {
                tz = parse_time(tz + 1, &days);
                timeptr->tm_wday = days;
            }
        }
        timeptr->tm_yday = 0;
    }
    hours = 2;
    minutes = seconds = 0;
    if ((int)*tz == '/') {
        tz = parse_time(tz + 1, &hours);
        if ((int)*tz == ':') {
            tz = parse_time(tz + 1, &minutes);
            if ((int)*tz == ':')
                tz = parse_time(tz + 1, &seconds);
        }
    }
    timeptr->tm_sec = seconds;
    timeptr->tm_min = minutes;
    timeptr->tm_hour = hours;
    return tz;
}

// FUNCTION: 0xf395a
void __parse_tz(char *tz)
{
    long dayzone;

    daylight = 0;
    tz = parse_offset(tz, D_00152294, &timezone);
    if (*tz == '\0') {
        dtzone[0] = '\0';
        return;
    }
    daylight = 1;
    dayzone = timezone - (60 * 60);
    tz = parse_offset(tz, dtzone, &dayzone);
    __dst_adjust = timezone - dayzone;
    if ((int)*tz == ',')
        tz = parse_rule(tz + 1, &__start_dst);
    if ((int)*tz == ',') {
        parse_rule(tz + 1, &__end_dst);
        __end_dst.tm_hour -= __dst_adjust / 3600;
        __end_dst.tm_min -= (__dst_adjust / 60) % 60;
        __end_dst.tm_sec -= __dst_adjust % 60;
    }
}

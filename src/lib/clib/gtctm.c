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

typedef union {
    unsigned long value;
    struct {
        unsigned char day;
        unsigned char month;
        unsigned char year;
        unsigned char day_of_week;
    } s;
} dos_date;

typedef union {
    unsigned long value;
    struct {
        unsigned char hundredths;
        unsigned char seconds;
        unsigned char minutes;
        unsigned char hours;
    } s;
} dos_time;

unsigned long __getdate(void);
#pragma aux __getdate = \
    "mov ah,2ah" \
    "int 21h" \
    "sub cx,1900" \
    "mov ch,al" \
    "shl ecx,16" \
    "mov cx,dx" \
    value [ecx] \
    modify [eax edx];

unsigned long __gettime(void);
#pragma aux __gettime = \
    "mov ah,2ch" \
    "int 21h" \
    "shl ecx,16" \
    "mov cx,dx" \
    value [ecx] \
    modify [eax edx];

// FUNCTION: 0xeb7a7
int __getctime(struct tm *t)
{
    dos_date date;
    dos_date tmp;
    int milliseconds;
    dos_time time;

    date.value = __getdate();
    t->tm_year = date.s.year;
    t->tm_mon = date.s.month - 1;
    t->tm_mday = date.s.day;
    time.value = __gettime();
    t->tm_hour = time.s.hours;
    t->tm_min = time.s.minutes;
    t->tm_sec = time.s.seconds;
    milliseconds = time.s.hundredths * 10;
    tmp.value = __getdate();
    if (tmp.s.day != date.s.day && t->tm_hour != 23) {
        t->tm_year = tmp.s.year;
        t->tm_mon = tmp.s.month - 1;
        t->tm_mday = tmp.s.day;
    }
    t->tm_isdst = -1;
    return milliseconds;
}

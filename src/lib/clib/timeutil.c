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

extern short const __diyr[];
extern short const D_00128e0e[];
extern char *tzname[2];
extern struct tm __start_dst;
extern struct tm __end_dst;

time_t mktime(struct tm *t);

// FUNCTION: 0xf3359
int __leapyear(int year)
{
    if (year & 3)
        return 0;
    if ((year % 100) != 0)
        return 1;
    if ((year % 400) == 0)
        return 1;
    return 0;
}

// FUNCTION: 0xf3394
// NONMATCHING: register allocation; the target names the leap-year table
// read at __dilyr-2 as ___diyr+0x18
int calc_yday(const struct tm *timerule, int year)
{
    struct tm tmbuf;
    int n;
    int first_wday;
    int month_days;

    if (timerule->tm_isdst == 0) {
        if (__leapyear(year + 1900)) {
            month_days = D_00128e0e[timerule->tm_mon + 1] - D_00128e0e[timerule->tm_mon];
        } else {
            month_days = __diyr[timerule->tm_mon + 1] - __diyr[timerule->tm_mon];
        }
        tmbuf.tm_sec = 0;
        tmbuf.tm_min = 0;
        tmbuf.tm_hour = 0;
        tmbuf.tm_mday = 1;
        tmbuf.tm_mon = timerule->tm_mon;
        tmbuf.tm_year = year;
        tmbuf.tm_isdst = 0;
        mktime(&tmbuf);
        first_wday = (timerule->tm_wday - tmbuf.tm_wday + 7) % 7;
        if (timerule->tm_mday == 5) {
            n = timerule->tm_mday - 1;
            if ((1 + first_wday + n * 7) > month_days)
                n = timerule->tm_mday - 2;
        } else {
            n = timerule->tm_mday - 1;
        }
        return tmbuf.tm_yday + first_wday + n * 7;
    }
    if (timerule->tm_isdst == 1)
        return timerule->tm_yday - 1;
    return timerule->tm_yday;
}

// FUNCTION: 0xf3479
int check_order(const struct tm *start, const struct tm *end, int year)
{
    if (start->tm_mon > end->tm_mon)
        return 1;
    if (start->tm_mon < end->tm_mon)
        return 0;
    if (calc_yday(start, year) > calc_yday(end, year))
        return 1;
    return 0;
}

int time_less(const struct tm *t1, const struct tm *t2);

// FUNCTION: 0xf34ad
// NONMATCHING: register allocation and stack slots; the target names the
// leap-year table read at __dilyr-2 as ___diyr+0x18
int __isindst(struct tm *t)
{
    int month_days;
    struct tm const *start;
    int month;
    int swap;
    int dst;
    struct tm const *end;
    int n1;
    int time_check;
    int n2;

    dst = 0;
    if (tzname[1][0] != '\0') {
        swap = check_order(&__start_dst, &__end_dst, t->tm_year);
        if (swap) {
            start = &__end_dst;
            end = &__start_dst;
        } else {
            start = &__start_dst;
            end = &__end_dst;
        }
        month = t->tm_mon;
        if (__leapyear(t->tm_year + 1900)) {
            month_days = D_00128e0e[month + 1] - D_00128e0e[month];
        } else {
            month_days = __diyr[month + 1] - __diyr[month];
        }
        time_check = 0;
        if (start->tm_isdst == 0) {
            if (month > start->tm_mon) {
                dst = 1;
            } else if (month == start->tm_mon) {
                n1 = t->tm_mday - (t->tm_wday + 7 - start->tm_wday) % 7;
                n2 = t->tm_mday - 1 - (t->tm_wday + 6 - start->tm_wday) % 7;
                if (start->tm_mday == 5) {
                    if (n1 > month_days - 7) {
                        dst = 1;
                        if (n2 <= month_days - 7)
                            time_check = 1;
                    }
                } else {
                    if (n1 >= (start->tm_mday - 1) * 7 + 1) {
                        dst = 1;
                        if (n2 < (start->tm_mday - 1) * 7 + 1)
                            time_check = 1;
                    }
                }
            }
        } else {
            n1 = start->tm_yday;
            if (start->tm_isdst == 1) {
                if (__leapyear(t->tm_year + 1900)) {
                    if (n1 > __diyr[2])
                        n1++;
                }
                n1--;
            }
            if (t->tm_yday >= n1) {
                dst = 1;
                if (t->tm_yday == n1)
                    time_check = 1;
            }
        }
        if (time_check)
            dst = !time_less(t, start);
        if (dst) {
            time_check = 0;
            if (end->tm_isdst == 0) {
                if (month > end->tm_mon) {
                    dst = 0;
                } else if (month == end->tm_mon) {
                    dst = 0;
                    n1 = t->tm_mday - (t->tm_wday + 7 - end->tm_wday) % 7;
                    n2 = t->tm_mday - 1 - (t->tm_wday + 6 - end->tm_wday) % 7;
                    if (end->tm_mday == 5) {
                        if (n1 <= month_days - 7) {
                            dst = 1;
                        } else if (n2 <= month_days - 7) {
                            time_check = 1;
                        }
                    } else {
                        if (n1 < (end->tm_mday - 1) * 7 + 1) {
                            dst = 1;
                        } else if (n2 < (end->tm_mday - 1) * 7 + 1) {
                            time_check = 1;
                        }
                    }
                }
            } else {
                n1 = end->tm_yday;
                if (end->tm_isdst == 1) {
                    if (__leapyear(t->tm_year + 1900)) {
                        if (n1 > __diyr[2])
                            n1++;
                    }
                    n1--;
                }
                if (t->tm_yday >= n1) {
                    dst = 0;
                    if (t->tm_yday == n1)
                        time_check = 1;
                }
            }
            if (time_check)
                dst = time_less(t, end);
        }
        if (swap)
            dst = swap - dst;
    }
    t->tm_isdst = dst;
    return dst;
}

// FUNCTION: 0xf3770
int time_less(const struct tm *t1, const struct tm *t2)
{
    int before;

    before = 0;
    if (t1->tm_hour < t2->tm_hour) {
        before = 1;
    } else if (t1->tm_hour == t2->tm_hour) {
        if (t1->tm_min < t2->tm_min
            || t1->tm_min == t2->tm_min && t1->tm_sec < t2->tm_sec)
            before = 1;
    }
    return before;
}

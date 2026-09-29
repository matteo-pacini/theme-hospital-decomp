#include "bflib.h"

struct TbDate {
    u8 Day;
    u8 Month;
    u16 Year;
    u8 DayOfWeek;
};

struct TbTime {
    u8 Hour;
    u8 Minute;
    u8 Second;
    u8 HSecond;
};

struct dosdate_t {
    u8 day;
    u8 month;
    u16 year;
    u8 dayofweek;
};

struct dostime_t {
    u8 hour;
    u8 minute;
    u8 second;
    u8 hsecond;
};

extern "C" {
void fn_000dc932(dosdate_t *date);
void fn_000dc94d(dostime_t *time);

// FUNCTION: 0x000bf778
int LbDate(void)
{
    return 1;
}

// FUNCTION: 0x000c2140
int fn_000c2140(TbDate *curr_date)
{
    dosdate_t date;

    fn_000dc932(&date);
    curr_date->Day = date.day;
    curr_date->Month = date.month;
    curr_date->Year = date.year;
    curr_date->DayOfWeek = date.dayofweek;
    return 1;
}

// FUNCTION: 0x000c2180
int fn_000c2180(TbTime *curr_time)
{
    dostime_t time;

    fn_000dc94d(&time);
    curr_time->Hour = time.hour;
    curr_time->Minute = time.minute;
    curr_time->Second = time.second;
    curr_time->HSecond = time.hsecond;
    return 1;
}

// FUNCTION: 0x000c21c0
s32 LbTimerClock(void)
{
    return clock() * 1000 / 100;
}

}

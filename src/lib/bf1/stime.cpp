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
void _dos_getdate(dosdate_t *date);
void _dos_gettime(dostime_t *time);

// FUNCTION: 0x000c2140
int LbDate(TbDate *curr_date)
{
    dosdate_t date;

    _dos_getdate(&date);
    curr_date->Day = date.day;
    curr_date->Month = date.month;
    curr_date->Year = date.year;
    curr_date->DayOfWeek = date.dayofweek;
    return 1;
}

// FUNCTION: 0x000c2180
int LbTime(TbTime *curr_time)
{
    dostime_t time;

    _dos_gettime(&time);
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

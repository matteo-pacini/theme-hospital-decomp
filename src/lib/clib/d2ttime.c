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

time_t mktime(struct tm *t);

// FUNCTION: 0xf4b64
time_t _d2ttime(unsigned date, unsigned time)
{
    struct tm t;

    t.tm_year = ((date & 0xfe00) >> 9) + 80;
    t.tm_mon = ((date & 0x01e0) >> 5) - 1;
    t.tm_mday = date & 0x001f;
    t.tm_hour = (time & 0xf800) >> 11;
    t.tm_min = (time & 0x07e0) >> 5;
    t.tm_sec = (time & 0x001f) * 2;
    t.tm_isdst = -1;
    return mktime(&t);
}

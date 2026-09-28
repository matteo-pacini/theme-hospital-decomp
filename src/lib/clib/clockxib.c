typedef unsigned long clock_t;

clock_t __clock(void);

// FUNCTION: 0xcf432
clock_t clock(void)
{
    return __clock();
}

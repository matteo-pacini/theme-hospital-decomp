typedef unsigned long clock_t;

clock_t __clock(void);

// FUNCTION: 0xcf432
// NONMATCHING: the jump target lies inside __InitFiles_ in
// config/functions.txt
clock_t clock(void)
{
    return __clock();
}

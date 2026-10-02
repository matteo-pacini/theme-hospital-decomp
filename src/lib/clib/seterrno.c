#include "clib.h"

#define EINVAL 9
#define EDOM   13
#define ERANGE 14

int *__get_errno_ptr(void);
int *__get_doserrno_ptr(void);

// __set_EDOM: set errno to EDOM.
// FUNCTION: 0xec52c
void __set_EDOM(void)
{
    __set_errno(EDOM);
}

// __set_errno: set errno to err.
// FUNCTION: 0xec531
void __set_errno(int err)
{
    *__get_errno_ptr() = err;
}

// __set_ERANGE: set errno to ERANGE.
// FUNCTION: 0xec53d
void __set_ERANGE(void)
{
    __set_errno(ERANGE);
}

// __set_EINVAL: set errno to EINVAL; returns -1.
// FUNCTION: 0x000ec544
int __set_EINVAL(void)
{
    __set_errno(EINVAL);
    return -1;
}

// __set_doserrno: set _doserrno to err.
// FUNCTION: 0xec554
void __set_doserrno(int err)
{
    *__get_doserrno_ptr() = err;
}

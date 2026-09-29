#include "clib.h"

#define EBADF  9
#define EDOM   13
#define EINVAL 14

int *__get_errno_ptr(void);
int *__get_doserrno_ptr(void);

// FUNCTION: 0xec52c
void __set_EDOM(void)
{
    __set_errno(EDOM);
}

// FUNCTION: 0xec531
void __set_errno(int err)
{
    *__get_errno_ptr() = err;
}

// FUNCTION: 0xec53d
void __set_ERANGE(void)
{
    __set_errno(EINVAL);
}

// FUNCTION: 0x000ec544
int __set_EINVAL(void)
{
    __set_errno(EBADF);
    return -1;
}

// FUNCTION: 0xec554
void __set_doserrno(int err)
{
    *__get_doserrno_ptr() = err;
}

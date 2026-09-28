#include "clib.h"

#define EBADF  9
#define EDOM   13
#define EINVAL 14

int *__get_errno_ptr(void);
int *__get_doserrno_ptr(void);

// FUNCTION: 0xec52c
// NONMATCHING: the original falls through into __set_errno, which follows
// it in the same segment; -zm keeps the tail jump
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
// NONMATCHING: config/functions.txt extends __set_EINVAL_ over __set_EBADF
// (0xec544)
void __set_EINVAL(void)
{
    __set_errno(EINVAL);
}

int __set_EBADF(void)
{
    __set_errno(EBADF);
    return -1;
}

// FUNCTION: 0xec554
// NONMATCHING: config/functions.txt extends __set_doserrno_ over utoa's
// digit table (0xec560)
void __set_doserrno(int err)
{
    *__get_doserrno_ptr() = err;
}

extern int errno;
extern int _doserrno;

// FUNCTION: 0xf4b0e
int *__get_errno_ptr(void)
{
    return &errno;
}

// FUNCTION: 0xf4b14
int *__get_doserrno_ptr(void)
{
    return &_doserrno;
}

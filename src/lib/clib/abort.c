#include "clib.h"

typedef void abort_handler(void);
void abort(void);
#pragma aux abort aborts;

void __terminate(void);

/* What abort calls; signal(SIGABRT, ...) replaces it. */
abort_handler *__abort = __terminate;

void __fatal_runtime_error(const char *msg, unsigned retcode);

// FUNCTION: 0xfc7a4
void abort(void)
{
    __abort();
}

// FUNCTION: 0xfc7aa
void __terminate(void)
{
    __fatal_runtime_error("ABNORMAL TERMINATION\r\n", 0xff);
}

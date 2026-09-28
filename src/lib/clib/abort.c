#include "clib.h"

typedef void abort_handler(void);
void abort(void);
#pragma aux abort aborts;

extern abort_handler *D_00153acc;
extern const char D_00128878[];

void __fatal_runtime_error(const char *msg, unsigned retcode);

// FUNCTION: 0xfc7a4
void abort(void)
{
    D_00153acc();
}

// FUNCTION: 0xfc7aa
void __terminate(void)
{
    __fatal_runtime_error(D_00128878, 0xff);
}

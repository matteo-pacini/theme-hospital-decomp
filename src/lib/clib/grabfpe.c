#include "clib.h"

void __GrabFP87(void);
#pragma aux __GrabFP87 "__GrabFP87"

// FUNCTION: 0xfc7bc
void __grab_fpe(void)
{
    __GrabFP87();
}

#include "clib.h"

void __GrabFP87(void);

// NONMATCHING: jumps to __GrabFP87 (0xfcefe), which config/functions.txt
// includes in fn_000fcead_
// FUNCTION: 0xfc7bc
void __grab_fpe(void)
{
    __GrabFP87();
}

#include "clib.h"

/* ::operator new(unsigned), called from C through its C++ symbol. */
void *__op_new(unsigned size);
#pragma aux __op_new "W?$nwn(ui)pnv"

void *__op_new_array(unsigned size);
#pragma aux __op_new_array "W?$nan(ui)pnv"

// operator new[]: allocates size bytes through operator new.
// FUNCTION: 0xed344
void *__op_new_array(unsigned size)
{
    return __op_new(size);
}

#include "clib.h"

#pragma intrinsic(strlen)

size_t lib_strlen(const char *s);
/* config/functions.txt has no name for strlen. */
#pragma aux lib_strlen "strlen_"

// FUNCTION: 0xcd716
size_t lib_strlen(const char *s)
{
    return strlen(s);
}

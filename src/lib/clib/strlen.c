#include "clib.h"

#pragma intrinsic(strlen)

size_t lib_strlen(const char *s);
/* Defined under another name so that the strlen in its body is the
   intrinsic. */
#pragma aux lib_strlen "strlen_"

// strlen: length of s, excluding the terminator.
// FUNCTION: 0xcd716
size_t lib_strlen(const char *s)
{
    return strlen(s);
}

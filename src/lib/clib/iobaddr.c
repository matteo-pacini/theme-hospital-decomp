#include "clib.h"

// FUNCTION: 0xfdf95
FILE *__get_std_stream(unsigned handle)
{
    if (handle > 5)
        return NULL;
    return &__iob[handle];
}

#include "clib.h"

// __get_std_stream: &__iob[handle] for a standard stream handle (0-5), else NULL.
// FUNCTION: 0xfdf95
FILE *__get_std_stream(unsigned handle)
{
    if (handle > 5)
        return NULL;
    return &__iob[handle];
}

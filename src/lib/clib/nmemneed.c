#include "clib.h"

// __nmemneed: out-of-memory hook for the near heap; returns 0 (no memory freed).
// FUNCTION: 0xec321
int __nmemneed(size_t size)
{
    return 0;
}

#include "clib.h"

// FUNCTION: 0xfbac4
static void default_handler(void)
{
}

void (*__longjmp_handler)(void) = default_handler;

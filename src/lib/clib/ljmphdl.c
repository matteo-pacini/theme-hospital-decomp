#include "clib.h"

// default_handler: empty default for __longjmp_handler.
// FUNCTION: 0xfbac4
static void default_handler(void)
{
}

void (*__longjmp_handler)(void) = default_handler;

/* Stand-ins for the Open Watcom build headers demangle.c expects, for a
 * native build on a 32/64-bit host. */
#include <stdbool.h>
#include <stdint.h>
typedef uintptr_t pointer_uint;
typedef unsigned boolbit;
#define _WCI86FAR
#define _WCI86NEAR
#define _WCNEAR
#define _WCFAR

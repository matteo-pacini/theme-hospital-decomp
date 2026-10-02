#include "clib.h"

extern unsigned _ASTACKSIZ;
extern char *_ASTACKPTR;
extern int ___Argc;
extern char **___Argv;

unsigned stackavail(void);
#pragma aux stackavail __modify __nomemory;
void *__doalloca(size_t __size);
#pragma aux __doalloca = \
    "sub esp,eax" \
    __parm __nomemory [eax] __value [esp] __modify __exact __nomemory [esp];

#define __ALLOCA_ALIGN(s) (((s) + (sizeof(int) - 1)) & ~(sizeof(int) - 1))
#define __alloca(s) __doalloca(__ALLOCA_ALIGN(s))
#define alloca(s) ((__ALLOCA_ALIGN(s) < stackavail()) ? __alloca(s) : NULL)

void __CommonInit(void);
int main(int argc, char **argv);
void exit(int status);

#pragma aux __CMain "*";

// __CMain: allocate the alloca stack area, run __CommonInit, then exit(main(argc, argv)).
// FUNCTION: 0xd0560
void __CMain(void)
{
    _ASTACKPTR = (char *)alloca(_ASTACKSIZ) + _ASTACKSIZ;
    __CommonInit();
    exit(main(___Argc, ___Argv));
}

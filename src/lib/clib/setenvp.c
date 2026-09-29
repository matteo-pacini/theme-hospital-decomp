#include "clib.h"

extern char **environ;
extern char *D_001516f8;
extern char *_Envptr;
extern unsigned short _Envseg;

void *memset(void *s, int c, size_t n);
void *_nmalloc(size_t size);
/* config/functions.txt names _nmalloc after a C++ runtime function. */
void _nfree(void *p);
/* config/functions.txt names _nfree after a C++ runtime function. */

void __setenvp(void);
#pragma aux __setenvp "*"

// NONMATCHING: the original keeps the environment segment in fs and reloads
// es from it; 10.6 keeps it in dx, and register allocation differs
// FUNCTION: 0xf3285
void __setenvp(void)
{
    __segment seg;
    char __based(seg) *startp;
    char __based(seg) *p;
    char *ep;
    char **my_environ;
    int count;
    unsigned length;

    if (environ != NULL)
        return;
    seg = _Envseg;
    startp = (char __based(seg) *)_Envptr;
    count = 0;
    p = startp;
    while (*p != '\0') {
        while (*p++ != '\0')
            ;
        ++count;
    }
    length = p - startp;
    if (length == 0)
        length = 1;
    ep = _nmalloc(length);
    if (ep == NULL)
        return;
    my_environ = _nmalloc((count + 1) * sizeof(char *) + count);
    if (my_environ != NULL) {
        environ = my_environ;
        count = 0;
        p = startp;
        while (*p != '\0') {
            environ[count] = ep;
            while ((*ep++ = *p++) != '\0')
                ;
            ++count;
        }
        environ[count] = NULL;
        D_001516f8 = (char *)&environ[count + 1];
        memset(D_001516f8, 0, count);
    } else {
        _nfree(ep);
    }
}

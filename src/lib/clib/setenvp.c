#include "clib.h"

extern char **environ;
extern char *__env_mask;
extern char *_Envptr;
extern unsigned short _Envseg;

void *memset(void *s, int c, size_t n);
void *_nmalloc(size_t size);
void _nfree(void *p);

void __setenvp(void);
#pragma aux __setenvp "*"

// NONMATCHING: the original keeps the environment segment in fs and reloads
// es from it; 10.6 keeps it in dx, and register allocation differs
// __setenvp: build environ (and __env_mask) from a near-heap copy of the DOS environment block, once.
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
        __env_mask = (char *)&environ[count + 1];
        memset(__env_mask, 0, count);
    } else {
        _nfree(ep);
    }
}

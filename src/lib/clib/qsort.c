#include "clib.h"

/* The comparison function is user code, which need not preserve es, fs, gs. */
#pragma aux __outside_CLIB modify [es fs gs];
typedef int qcomp(const void *, const void *);
#pragma aux (__outside_CLIB) qcomp;
typedef qcomp *_compfn;

/* Exchanges size bytes at p and q: dwords, then the remaining bytes. */
void inline_swap(char *p, char *q, size_t size);
#pragma aux inline_swap = \
    0x06                    /* push es */ \
    0x1e                    /* push ds */ \
    0x07                    /* pop es */ \
    0x0f 0xb6 0xd1          /* movzx edx,cl */ \
    0xc1 0xe9 0x02          /* shr ecx,2 */ \
    0x74 0x0b               /* je bytes */ \
    0x8b 0x07               /* dwords: mov eax,[edi] */ \
    0x87 0x06               /* xchg [esi],eax */ \
    0xab                    /* stosd */ \
    0x83 0xc6 0x04          /* add esi,4 */ \
    0x49                    /* dec ecx */ \
    0x75 0xf5               /* jne dwords */ \
    0x80 0xe2 0x03          /* bytes: and dl,3 */ \
    0x74 0x09               /* je done */ \
    0x8a 0x07               /* next: mov al,[edi] */ \
    0x86 0x06               /* xchg [esi],al */ \
    0xaa                    /* stosb */ \
    0x46                    /* inc esi */ \
    0x4a                    /* dec edx */ \
    0x75 0xf7               /* jne next */ \
    0x07                    /* done: pop es */ \
    parm [esi] [edi] [ecx] modify [eax ecx edx esi edi];

/* config/functions.txt has swap under a wrong demo name. */
#pragma aux swap "__wcpp_2_module_dtor__" parm [esi] [edi] [ecx] modify [eax ecx edx esi edi];

// FUNCTION: 0xced1f
void swap(char *p, char *q, size_t size)
{
    inline_swap(p, q, size);
}

/* config/functions.txt has med3 under a wrong demo name. */
#pragma aux med3 "W?fs_state_table$n(pn$ro_dtreg$$)pn$__7a4u3RO_STATE$$"

// FUNCTION: 0xced45
char *med3(char *a, char *b, char *c, _compfn cmp)
{
    if (cmp(a, b) > 0) {
        if (cmp(a, c) > 0) {
            if (cmp(b, c) > 0)
                return b;
            return c;
        }
        return a;
    }
    if (cmp(a, c) >= 0)
        return a;
    if (cmp(b, c) > 0)
        return c;
    return b;
}

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

#pragma aux byteswap parm [esi] [edi] [ecx] modify [eax ecx edx esi edi];

// byteswap: exchange size bytes at p and q.
// FUNCTION: 0xced1f
void byteswap(char *p, char *q, size_t size)
{
    inline_swap(p, q, size);
}

// med3: median of the elements a, b and c under cmp.
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

#define MAXDEPTH 32

typedef int WORD;

#define SWAP(a, b)                          \
    if (swaptype != 0) {                    \
        byteswap(a, b, size);               \
    } else {                                \
        WORD tmp = *(WORD *)(a);            \
        *(WORD *)(a) = *(WORD *)(b);        \
        *(WORD *)(b) = tmp;                 \
    }

void qsort(char *base, size_t n, size_t size, _compfn compar);

// NONMATCHING: the stack slots of the shell-sort step constants and of pv/pn
// are swapped; the pb/pc exchange and the first min() pick the other
// register order
// qsort: sort n elements of size bytes at base in ascending order of compar.
// FUNCTION: 0xced9d
void qsort(char *base, size_t n, size_t size, _compfn compar)
{
    char *mid;
    char *p1;
    char *pv;
    char *pc;
    char *pa;
    char *p2;
    char *pb;
    char *pn;
    char *pd;
    int comparison;
    char *base_stack[MAXDEPTH];
    WORD t;
    size_t s;
    size_t n_stack[MAXDEPTH];
    int shell;
    size_t r;
    _compfn cmp;
    int swaptype;
    unsigned sp;

    cmp = compar;
    swaptype = ((unsigned)base | size) & 3 ? 2 : size > 4;
    sp = 0;
    for (;;) {
        while (n > 1) {
            if (n < 16) {
                shell = size * 3;
                pn = base + n * size;
                for (; shell > 0; shell -= size * 2) {
                    for (p1 = base + shell; p1 < pn; p1 += shell) {
                        for (p2 = p1; p2 > base && cmp(p2 - shell, p2) > 0; p2 -= shell) {
                            SWAP(p2, p2 - shell);
                        }
                    }
                }
                break;
            }
            mid = base + (n >> 1) * size;
            if (n > 29) {
                p1 = base;
                p2 = base + (n - 1) * size;
                if (n > 42) {
                    s = (n >> 3) * size;
                    p1 = med3(p1, p1 + s, p1 + (s << 1), cmp);
                    mid = med3(mid - s, mid, mid + s, cmp);
                    p2 = med3(p2 - (s << 1), p2 - s, p2, cmp);
                }
                mid = med3(p1, mid, p2, cmp);
            }
            if (swaptype != 0) {
                pv = base;
                SWAP(pv, mid);
            } else {
                pv = (char *)&t;
                t = *(WORD *)mid;
            }
            pa = pb = base;
            pc = pd = base + (n - 1) * size;
            for (;;) {
                while (pb <= pc && (comparison = cmp(pb, pv)) <= 0) {
                    if (comparison == 0) {
                        SWAP(pa, pb);
                        pa += size;
                    }
                    pb += size;
                }
                while (pb <= pc && (comparison = cmp(pc, pv)) >= 0) {
                    if (comparison == 0) {
                        SWAP(pc, pd);
                        pd -= size;
                    }
                    pc -= size;
                }
                if (pb > pc)
                    break;
                SWAP(pb, pc);
                pb += size;
                pc -= size;
            }
            pn = base + n * size;
            s = pb - pa > pa - base ? pa - base : pb - pa;
            if (s > 0)
                inline_swap(base, pb - s, s);
            s = pd - pc < pn - pd - size ? pd - pc : pn - pd - size;
            if (s > 0)
                inline_swap(pb, pn - s, s);
            r = pb - pa;
            s = pd - pc;
            if (s >= r) {
                n_stack[sp] = s / size;
                n = r / size;
                base_stack[sp] = pn - s;
            } else {
                if (r <= size)
                    break;
                base_stack[sp] = base;
                n_stack[sp] = r / size;
                base = pn - s;
                n = s / size;
            }
            ++sp;
        }
        if (sp == 0)
            break;
        --sp;
        base = base_stack[sp];
        n = n_stack[sp];
    }
}

#include "clib.h"

typedef struct freelistp {
    unsigned len;
    struct freelistp *prev;
    struct freelistp *next;
} freelistp, *frlptr;

typedef struct heapblkp {
    unsigned len;
    struct heapblkp *prev;
    struct heapblkp *next;
    frlptr rover;
    unsigned b4rover;
    unsigned largest_blk;
    unsigned numalloc;
    unsigned numfree;
    freelistp freehead;
} heapblkp, *mheapptr;

extern mheapptr __nheapbeg;
extern mheapptr D_001505e4;
extern unsigned D_001505e8;
extern mheapptr D_0018ba94;
extern unsigned char D_0018cfa8;

void __MemFree(unsigned ptr, unsigned seg, unsigned heap);
#pragma aux __MemFree "__MemFree" parm [eax] [edx] [ebx];

unsigned short FP_SEG(const volatile void __far *);
#pragma aux FP_SEG = parm caller [eax dx] value [dx];
#define _DGroup() FP_SEG((void __far *)&__nheapbeg)

void _nfree(void *stg);
/* config/functions.txt has _nfree under a wrong demo name. */

// FUNCTION: 0x000cf342
void _nfree(void *stg)
{
    mheapptr p1;

    if (stg == NULL)
        return;
    p1 = D_0018ba94;
    if (p1 == NULL || (unsigned)p1 > (unsigned)stg || (unsigned)stg >= (unsigned)p1->next) {
        for (p1 = __nheapbeg; p1->next != NULL; p1 = p1->next) {
            if ((unsigned)p1 <= (unsigned)stg && (unsigned)stg < (unsigned)p1->next)
                break;
        }
    }
    __MemFree((unsigned)stg, _DGroup(), (unsigned)p1);
    D_0018ba94 = p1;
    if (p1 < D_001505e4) {
        if (p1->largest_blk > D_001505e8)
            D_001505e8 = p1->largest_blk;
    }
    D_0018cfa8 = 0;
}

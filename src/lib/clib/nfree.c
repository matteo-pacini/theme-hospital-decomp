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
extern mheapptr __MiniHeapRover;
extern unsigned __LargestSizeB4MiniHeapRover;
extern mheapptr __MiniHeapFreeRover;
extern unsigned char __nheap_clean;

void __MemFree(unsigned ptr, unsigned seg, unsigned heap);
#pragma aux __MemFree "__MemFree" parm [eax] [edx] [ebx];

unsigned short FP_SEG(const volatile void __far *);
#pragma aux FP_SEG = parm caller [eax dx] value [dx];
#define _DGroup() FP_SEG((void __far *)&__nheapbeg)

void _nfree(void *stg);

// _nfree: free near-heap block stg (NULL is ignored), finding its miniheap and updating the allocation rovers.
// FUNCTION: 0x000cf342
void _nfree(void *stg)
{
    mheapptr p1;

    if (stg == NULL)
        return;
    p1 = __MiniHeapFreeRover;
    if (p1 == NULL || (unsigned)p1 > (unsigned)stg || (unsigned)stg >= (unsigned)p1->next) {
        for (p1 = __nheapbeg; p1->next != NULL; p1 = p1->next) {
            if ((unsigned)p1 <= (unsigned)stg && (unsigned)stg < (unsigned)p1->next)
                break;
        }
    }
    __MemFree((unsigned)stg, _DGroup(), (unsigned)p1);
    __MiniHeapFreeRover = p1;
    if (p1 < __MiniHeapRover) {
        if (p1->largest_blk > __LargestSizeB4MiniHeapRover)
            __LargestSizeB4MiniHeapRover = p1->largest_blk;
    }
    __nheap_clean = 0;
}

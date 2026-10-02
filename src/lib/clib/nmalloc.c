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

#define TRAILER_SIZE 0
#define ROUND_SIZE   3
#define FRL_SIZE     12

extern mheapptr __nheapbeg;
extern mheapptr __MiniHeapRover;
extern unsigned __LargestSizeB4MiniHeapRover;
extern unsigned char __nheap_clean;

unsigned __MemAllocator(unsigned size, unsigned seg, unsigned heap);
#pragma aux __MemAllocator "__MemAllocator" parm [eax] [edx] [ebx] value [eax] modify [es fs gs];
int __ExpandDGROUP(unsigned amount);
int __nmemneed(size_t size);

unsigned short FP_SEG(const volatile void __far *);
#pragma aux FP_SEG = parm caller [eax dx] value [dx];
#define _DGroup() FP_SEG((void __far *)&__nheapbeg)

void *_nmalloc(size_t amt);

// _nmalloc: allocate amt bytes from the near heap, growing DGROUP or calling __nmemneed when full; NULL on failure or amt == 0.
// FUNCTION: 0x000cf25f
void *_nmalloc(size_t amt)
{
    unsigned largest;
    unsigned size;
    unsigned ptr;
    unsigned char expanded;
    mheapptr miniheap_ptr;

    if (amt == 0 || amt > -sizeof(heapblkp))
        return NULL;
    ptr = 0;
    expanded = 0;
    size = (amt + TRAILER_SIZE + ROUND_SIZE) & ~ROUND_SIZE;
    for (;;) {
        largest = size;
        if (largest < FRL_SIZE)
            largest = FRL_SIZE;
        if (largest > __LargestSizeB4MiniHeapRover) {
            miniheap_ptr = __MiniHeapRover;
            if (miniheap_ptr == NULL) {
                __LargestSizeB4MiniHeapRover = 0;
                miniheap_ptr = __nheapbeg;
            }
        } else {
            __LargestSizeB4MiniHeapRover = 0;
            miniheap_ptr = __nheapbeg;
        }
        for (; miniheap_ptr != NULL; miniheap_ptr = miniheap_ptr->next) {
            __MiniHeapRover = miniheap_ptr;
            ptr = __MemAllocator(amt, _DGroup(), (unsigned)miniheap_ptr);
            if (ptr != 0)
                goto release;
            largest = miniheap_ptr->largest_blk;
            if (largest > __LargestSizeB4MiniHeapRover)
                __LargestSizeB4MiniHeapRover = largest;
        }
        if (expanded || !__ExpandDGROUP(amt)) {
            if (!__nmemneed(amt))
                break;
            expanded = 0;
        } else {
            expanded = 1;
        }
    }
release:
    __nheap_clean = 0;
    return (void *)ptr;
}

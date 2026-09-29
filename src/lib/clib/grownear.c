#include "clib.h"

#define TAG_SIZE 4
#define END_TAG  0xffffffff

typedef unsigned tag;

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

struct dpmi_hdr {
    unsigned long dpmi_handle;
    unsigned dos_seg_value;
};

typedef int tiny_ret_t;
#define TINY_ERROR(rc) ((rc) < 0)

extern mheapptr __nheapbeg;
extern mheapptr D_001505e4;
extern mheapptr D_0018ba94;
extern unsigned _curbrk;
extern int __heap_enabled;
extern unsigned long __minreal;
extern unsigned _amblksiz;
extern unsigned char _Extender;
extern unsigned char _ExtenderSubtype;
extern unsigned char _X32VM;

#define _IsRational()         ((int)_Extender == 1)
#define _IsRationalZeroBase() (_IsRational() && _ExtenderSubtype == 0)
#define _IsCodeBuilder()      ((int)_Extender == 9)
#define _IsPharLap()          ((int)_Extender >= 2 && (int)_Extender <= 8)

void *__brk(unsigned brk_value);
void _nfree(void *p);
/* config/functions.txt has _nfree under a wrong demo name. */

unsigned _SegmentLimit(void);
#pragma aux _SegmentLimit = \
    "xor eax,eax" \
    "mov ax,ds" \
    "lsl eax,eax" \
    "inc eax" \
    value [eax];

void _TinyDPMIFree(unsigned short handle_lo, unsigned long handle);
#pragma aux _TinyDPMIFree = \
    "shr esi,16" \
    "mov ah,5" \
    "mov al,2" \
    "int 31h" \
    parm caller [di] [esi] modify [eax esi];

tiny_ret_t _TinyFreeBlock(unsigned seg);
#pragma aux _TinyFreeBlock = \
    "push es" \
    "mov es,ax" \
    "mov ah,49h" \
    "int 21h" \
    "rcl eax,1" \
    "ror eax,1" \
    "pop es" \
    parm caller [eax] value [eax];

tiny_ret_t _TinyAllocBlock(unsigned paras);
#pragma aux _TinyAllocBlock = \
    "mov ah,48h" \
    "int 21h" \
    "rcl eax,1" \
    "ror eax,1" \
    parm caller [ebx] value [eax];

struct dpmi_hdr *_TinyDPMIAlloc(unsigned size_hi, unsigned size_lo);
#pragma aux _TinyDPMIAlloc = \
    "mov ah,5" \
    "mov al,1" \
    "int 31h" \
    "sbb eax,eax" \
    "inc eax" \
    "je L1" \
    "mov ax,bx" \
    "shl eax,16" \
    "mov ax,cx" \
    "mov [eax],di" \
    "mov [eax+2],si" \
    "L1:" \
    parm caller [ebx] [ecx] value [eax] modify [ebx ecx esi edi];

struct dpmi_hdr *_TinyDPMIRealloc(struct dpmi_hdr *dpmi, unsigned size_hi, unsigned size_lo);
#pragma aux _TinyDPMIRealloc = \
    "mov di,[eax]" \
    "mov si,2[eax]" \
    "mov ah,5" \
    "mov al,3" \
    "int 31h" \
    "sbb eax,eax" \
    "inc eax" \
    "je L1" \
    "mov ax,bx" \
    "shl eax,16" \
    "mov ax,cx" \
    "mov [eax],di" \
    "mov 2[eax],si" \
    "L1:" \
    parm caller [eax] [ebx] [ecx] value [eax] modify [ebx ecx esi edi];

void *_TinyDPMIBase(unsigned seg);
#pragma aux _TinyDPMIBase = \
    "mov ah,0" \
    "mov al,6" \
    "int 31h" \
    "mov eax,ecx" \
    "shl eax,16" \
    "mov ax,dx" \
    parm caller [ebx] value [eax] modify [ecx edx];

void *_TinyCBAlloc(unsigned size);
#pragma aux _TinyCBAlloc = \
    "mov eax,80004800h" \
    "int 21h" \
    "sbb ebx,ebx" \
    "not ebx" \
    "and eax,ebx" \
    parm caller [ebx] value [eax] modify [ebx];

/* config/functions.txt has no name for __unlink. */
#pragma aux __unlink "__unlink_"

// FUNCTION: 0xebd67
void __unlink(mheapptr heap)
{
    mheapptr prev_heap;
    mheapptr next_heap;

    if (heap == D_001505e4)
        D_001505e4 = heap->next;
    if (heap == D_0018ba94)
        D_0018ba94 = NULL;
    if (heap == __nheapbeg)
        __nheapbeg = heap->next;
    prev_heap = heap->prev;
    next_heap = heap->next;
    if (prev_heap != NULL)
        prev_heap->next = next_heap;
    if (next_heap != NULL)
        next_heap->prev = prev_heap;
}

// FUNCTION: 0xebdb8
// NONMATCHING: next is kept in esi instead of ecx
void __FreeDPMIBlocks(void)
{
    mheapptr mhp;
    mheapptr next;
    struct dpmi_hdr *dpmi;

    mhp = __nheapbeg;
    while (mhp != NULL) {
        if (mhp->freehead.prev->len + sizeof(heapblkp) == mhp->len) {
            dpmi = (struct dpmi_hdr *)mhp - 1;
            next = mhp->next;
            __unlink(mhp);
            mhp = next;
            if (dpmi->dos_seg_value == 0)
                _TinyDPMIFree(dpmi->dpmi_handle, dpmi->dpmi_handle);
            else
                _TinyFreeBlock(dpmi->dos_seg_value);
        } else {
            mhp = mhp->next;
        }
    }
}

frlptr __LinkUpNewMHeap(mheapptr p1);

// FUNCTION: 0xebe14
void *__ReAllocDPMIBlock(frlptr p1, unsigned req_size)
{
    mheapptr mhp;
    struct dpmi_hdr *dpmi;
    unsigned size;
    frlptr flp;
    frlptr flp2;

    __FreeDPMIBlocks();
    for (mhp = __nheapbeg; mhp != NULL; mhp = mhp->next) {
        if ((char *)mhp > (char *)p1)
            break;
        if ((char *)(mhp + 1) == (char *)p1 && mhp->numalloc == 1) {
            __unlink(mhp);
            dpmi = (struct dpmi_hdr *)mhp - 1;
            if (dpmi->dos_seg_value != 0)
                break;
            size = mhp->len + sizeof(struct dpmi_hdr) + TAG_SIZE;
            size += req_size - (p1->len - TAG_SIZE);
            size += 0xfff;
            size &= ~0xfff;
            dpmi = _TinyDPMIRealloc(dpmi, size >> 16, size);
            if (dpmi == NULL)
                return NULL;
            dpmi->dos_seg_value = 0;
            mhp = (mheapptr)(dpmi + 1);
            mhp->len = size - sizeof(struct dpmi_hdr) - TAG_SIZE;
            flp = __LinkUpNewMHeap(mhp);
            mhp->numalloc = 1;
            req_size = (req_size + 1) & ~1;
            size = flp->len - req_size;
            if (size >= sizeof(freelistp)) {
                flp->len = req_size | 1;
                flp2 = (frlptr)((char *)flp + req_size);
                flp2->len = size | 1;
                mhp->numalloc++;
                mhp->largest_blk = ~0;
                _nfree((char *)flp2 + TAG_SIZE);
            } else {
                flp->len |= 1;
            }
            return flp;
        }
    }
    return NULL;
}

// FUNCTION: 0xebf13
frlptr __LinkUpNewMHeap(mheapptr p1)
{
    mheapptr p2;
    mheapptr p2_prev;
    tag *last_tag;
    unsigned amount;
    frlptr fh;

    for (p2 = __nheapbeg, p2_prev = NULL; p2 != NULL; p2 = p2->next) {
        if (p1 < p2)
            break;
        p2_prev = p2;
    }
    p1->prev = p2_prev;
    p1->next = p2;
    if (p2_prev != NULL)
        p2_prev->next = p1;
    else
        __nheapbeg = p1;
    if (p2 != NULL)
        p2->prev = p1;
    p1->freehead.len = 0;
    p1->b4rover = 0;
    p1->numalloc = 0;
    p1->numfree = 0;
    fh = &p1->freehead;
    amount = p1->len;
    p1++;
    (p1 - 1)->freehead.prev = fh;
    (p1 - 1)->freehead.next = fh;
    amount -= sizeof(heapblkp);
    (p1 - 1)->rover = fh;
    ((frlptr)p1)->len = amount;
    last_tag = (tag *)((char *)p1 + amount);
    *last_tag = END_TAG;
    return (frlptr)p1;
}

// FUNCTION: 0xebf87
unsigned __LastFree(void)
{
    frlptr p1;
    unsigned brk_value;

    if (__nheapbeg == NULL)
        return 0;
    p1 = __nheapbeg->freehead.prev;
    brk_value = (unsigned)((char *)p1 + p1->len + TAG_SIZE);
    if (_IsPharLap() && !_X32VM)
        _curbrk = _SegmentLimit();
    if (brk_value == _curbrk)
        return p1->len;
    return 0;
}

// FUNCTION: 0xebfd7
// NONMATCHING: the original keeps size in its stack slot (10.6 would use ebp);
// volatile reproduces the slot, but orders the reloads of size differently
// and keeps the DPMI block pointer in ebx instead of eax
void *RationalAlloc(volatile size_t size)
{
    struct dpmi_hdr *dpmi;
    mheapptr mhp;
    tiny_ret_t save_DOS_block;
    tiny_ret_t DOS_block;

    __FreeDPMIBlocks();
    dpmi = _TinyDPMIAlloc(size >> 16, size);
    if (dpmi != NULL) {
        mhp = (mheapptr)(dpmi + 1);
        mhp->len = size - sizeof(struct dpmi_hdr);
        dpmi->dos_seg_value = 0;
        return mhp;
    }
    if (__minreal & 0xfff00000)
        __minreal = 0xfffff;
    if (size > 0x10000)
        return NULL;
    save_DOS_block = _TinyAllocBlock((__minreal >> 4) | 1);
    if (!TINY_ERROR(save_DOS_block)) {
        DOS_block = _TinyAllocBlock(size >> 4);
        _TinyFreeBlock(save_DOS_block);
        if (!TINY_ERROR(DOS_block)) {
            dpmi = _TinyDPMIBase(DOS_block);
            dpmi->dos_seg_value = DOS_block;
            mhp = (mheapptr)(dpmi + 1);
            mhp->len = size - sizeof(struct dpmi_hdr);
            return mhp;
        }
    }
    return NULL;
}

int __AdjustAmount(unsigned *amount);

// FUNCTION: 0xec0a8
int __CreateNewNHeap(unsigned amount)
{
    mheapptr p1;
    frlptr flp;
    unsigned brk_value;

    if (!__heap_enabled)
        return 0;
    if (_curbrk == ~1u)
        return 0;
    if (__AdjustAmount(&amount) == 0)
        return 0;
    if (_IsRational()) {
        brk_value = (unsigned)RationalAlloc(amount);
        if (brk_value != 0)
            amount = ((mheapptr)brk_value)->len;
    } else {
        brk_value = (unsigned)_TinyCBAlloc(amount);
        amount -= TAG_SIZE;
    }
    if (brk_value == 0)
        return 0;
    if (amount - TAG_SIZE > amount)
        return 0;
    amount -= TAG_SIZE;
    if (amount < sizeof(heapblkp) + sizeof(freelistp))
        return 0;
    p1 = (mheapptr)brk_value;
    p1->len = amount;
    flp = __LinkUpNewMHeap(p1);
    amount = flp->len;
    flp->len = amount | 1;
    ++p1->numalloc;
    p1->largest_blk = ~0;
    _nfree((char *)flp + TAG_SIZE);
    return 1;
}

// FUNCTION: 0xec158
// NONMATCHING: brk_value is copied to ebx before the -1 test instead of after
int __ExpandDGROUP(unsigned amount)
{
    mheapptr p1;
    frlptr flp;
    unsigned brk_value;
    tag *last_tag;
    unsigned new_brk_value;

    if (_IsRationalZeroBase() || _IsCodeBuilder())
        return __CreateNewNHeap(amount);
    if (!__heap_enabled)
        return 0;
    if (_curbrk == ~1u)
        return 0;
    if (__AdjustAmount(&amount) == 0)
        return 0;
    if (_IsPharLap() && !_X32VM)
        _curbrk = _SegmentLimit();
    new_brk_value = amount + _curbrk;
    if (new_brk_value < _curbrk)
        new_brk_value = ~1u;
    brk_value = (unsigned)__brk(new_brk_value);
    if (brk_value == -1)
        return 0;
    if (brk_value > (unsigned)-8)
        return 0;
    if (new_brk_value <= brk_value)
        return 0;
    amount = new_brk_value - brk_value;
    if (amount - TAG_SIZE > amount)
        return 0;
    amount -= TAG_SIZE;
    for (p1 = __nheapbeg; p1 != NULL; p1 = p1->next) {
        if (p1->next == NULL)
            break;
        if ((unsigned)p1 <= brk_value && (unsigned)p1->next > brk_value)
            break;
    }
    if (p1 != NULL && brk_value - TAG_SIZE == (unsigned)((char *)p1 + p1->len)) {
        brk_value -= TAG_SIZE;
        amount += TAG_SIZE;
        flp = (frlptr)brk_value;
        p1->len += amount;
        last_tag = (tag *)((char *)flp + amount);
        *last_tag = END_TAG;
    } else {
        if (amount < sizeof(heapblkp) + sizeof(freelistp))
            return 0;
        p1 = (mheapptr)brk_value;
        p1->len = amount;
        flp = __LinkUpNewMHeap(p1);
        amount = flp->len;
    }
    flp->len = amount | 1;
    ++p1->numalloc;
    p1->largest_blk = ~0;
    _nfree((char *)flp + TAG_SIZE);
    return 1;
}

// FUNCTION: 0xec2a4
int __AdjustAmount(unsigned *amount)
{
    unsigned amt;

    amt = (*amount + TAG_SIZE - 1) & ~(TAG_SIZE - 1);
    if (amt == 0)
        return 0;
    if (_IsRationalZeroBase() || _IsCodeBuilder())
        amt += sizeof(struct dpmi_hdr);
    else
        amt -= __LastFree();
    *amount = amt;
    amt += TAG_SIZE + sizeof(freelistp) + sizeof(heapblkp);
    if (amt < *amount)
        return 0;
    if (amt < _amblksiz)
        amt = _amblksiz & ~1u;
    *amount = amt;
    amt += 0xfff;
    if (amt < *amount)
        return 0;
    amt &= ~0xfff;
    *amount = amt;
    return amt != 0;
}

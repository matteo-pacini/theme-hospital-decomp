#include "bflib.h"

extern "C" {
extern u32 lbMemoryAvailable[5];
extern u32 D_0015642f;
extern u32 D_00156433;

void fn_000bb0d0(_CMemory *mem);
#pragma aux fn_000bb0d0 "W?ReleaseAll$:_CMemory$n()v"
u32 fn_000bb10c(_CMemory *mem, void *p);
#pragma aux fn_000bb10c "W?Free$:_CMemory$n(pnv)ul"
u32 fn_000bb2b0(void);
}

/* The static initialiser for these objects (.fn_init) constructs both. */
// FUNCTION: 0xbabb0
_CMemory _lbMemory;
CDosHeap _lbHeap;

u32 dos_alloc(u32 size);
void dos_free(u16 seg);

// FUNCTION: 0x000bac20
void LbPackHeap(void)
{
    _lbHeap.Repack();
}

extern "C" {

// FUNCTION: 0x000bac40
void *LbMemoryAlloc(u32 size)
{
    return _lbHeap.Alloc(size);
}

// FUNCTION: 0x000bac70
void *LbMemoryAllocLow(u32 size)
{
    return _lbMemory.Alloc(size, 1);
}

// FUNCTION: 0x000baca4
int LbMemoryFree(void *p)
{
    _lbHeap.Free(p);
    return 1;
}

// FUNCTION: 0x000bacd8
int LbMemorySetup(void)
{
    void *mem;
    u32 size;
    u32 try_size;

    mem = _lbMemory.Alloc(0x565720, fn_000bb2b0());
    size = 0x565720;
    if (mem == 0) {
        try_size = 0x4ac4a0;
        size = try_size;
        while (try_size < 0x565720) {
            mem = _lbMemory.Alloc(try_size, fn_000bb2b0());
            if (mem) {
                fn_000bb10c(&_lbMemory, mem);
                size = try_size;
            } else
                break;
            try_size += 0x25800;
        }
        mem = _lbMemory.Alloc(size, fn_000bb2b0());
    }
    if (mem == 0)
        return -1;
    if (size >= 0x503ca0) {
        D_0015642f = 0xc8000;
        D_00156433 = 2;
    } else if (size >= 0x4d1ca0) {
        D_0015642f = 0x96000;
        D_00156433 = 1;
    } else {
        D_0015642f = 0x70800;
        D_00156433 = 0;
    }
    _lbHeap.Init(mem, size);
    return 1;
}

// FUNCTION: 0x000badfc
int LbMemoryReset(void)
{
    fn_000bb0d0(&_lbMemory);
    return 1;
}

// FUNCTION: 0x000bae28
void LbMemorySetupMaxSize(u32 size)
{
}

// FUNCTION: 0x000bae44
int LbMemoryShrink(void *p, u32 size)
{
    return -1;
}

// FUNCTION: 0x000bae6c
int LbMemoryCheck(void)
{
    lbMemoryAvailable[0] = 0;
    lbMemoryAvailable[1] = 0;
    lbMemoryAvailable[2] = 0;
    lbMemoryAvailable[3] = 0;
    lbMemoryAvailable[4] = -1;
    return -1;
}

// FUNCTION: 0x000baec0
int fn_000baec0(void *p, u32 size)
{
    return -1;
}

}

// FUNCTION: 0x000baee8
_CMemory::_CMemory()
{
    u32 i;

    used_list = 0;
    for (i = 0; i < 99; i++)
        blocks[i].next = &blocks[i + 1];
    blocks[99].next = 0;
    free_list = blocks;
}

// FUNCTION: 0x000baf64
_CMemory::~_CMemory()
{
    if (used_list) {}
}

// FUNCTION: 0x000baf94
void *_CMemory::Alloc(u32 size, u32 low)
{
    _CMemoryBlock *b;

    if (size == 0)
        return 0;
    if (free_list == 0)
        return 0;
    b = free_list;
    if (low) {
        u32 seg;

        seg = dos_alloc(size);
        if (seg == 0)
            return 0;
        b->seg = seg >> 16;
        b->ptr = (void *)((seg & 0xffff) << 4);
        b->size = size;
        memset(b->ptr, 0, size);
    } else {
        void *p;

        p = malloc(((size + 31) >> 5) << 5);
        if (p == 0)
            return 0;
        memset(p, 0, ((size + 31) >> 5) << 5);
        b->seg = 0;
        b->ptr = p;
        b->size = size;
    }
    free_list = free_list->next;
    b->next = used_list;
    used_list = b;
    return b->ptr;
}

extern "C" {

// FUNCTION: 0x000bb0d0
void fn_000bb0d0(_CMemory *mem)
{
    while (mem->used_list)
        fn_000bb10c(mem, mem->used_list->ptr);
}

// FUNCTION: 0x000bb10c
u32 fn_000bb10c(_CMemory *mem, void *p)
{
    _CMemoryBlock *b;
    _CMemoryBlock *prev;

    if (p == 0)
        return 1;
    b = mem->used_list;
    prev = mem->used_list;
    while (b) {
        if (b->ptr == p)
            break;
        prev = b;
        b = b->next;
    }
    if (b == 0)
        return 0;
    if (b->seg)
        dos_free(b->seg);
    else
        free(b->ptr);
    if (b == mem->used_list)
        mem->used_list = b->next;
    else
        prev->next = b->next;
    b->next = mem->free_list;
    mem->free_list = b;
    return 1;
}

}

// FUNCTION: 0x000bb1f8
u32 dos_alloc(u32 size)
{
    u32 result;
    REGS in;
    REGS out;

    result = 0;
    if (size < 0xa0000) {
        in.x.eax = 0x100;
        in.x.ebx = (size + 15) >> 4;
        int386(0x31, &in, &out);
        if (out.x.cflag == 0)
            result = (out.x.edx << 16) | (out.x.eax & 0xffff);
    }
    return result;
}

// FUNCTION: 0x000bb268
void dos_free(u16 seg)
{
    REGS in;
    REGS out;

    if (seg) {
        in.x.eax = 0x101;
        in.x.edx = seg & 0xffff;
        int386(0x31, &in, &out);
    }
}

// FUNCTION: 0x000bb2b0 COMDAT
extern "C" inline u32 fn_000bb2b0(void)
{
    return 0;
}

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

// Compacts the game heap (_lbHeap), merging adjacent free blocks.
// FUNCTION: 0x000bac20
void LbPackHeap(void)
{
    _lbHeap.Repack();
}

extern "C" {

// Allocates size bytes from the game heap (_lbHeap) that LbMemorySetup set up.
// Returns 0 on failure.
// FUNCTION: 0x000bac40
void *LbMemoryAlloc(u32 size)
{
    return _lbHeap.Alloc(size);
}

// Allocates size bytes of zeroed DOS conventional memory (below 1 MB) through
// DPMI. Returns its linear address, or 0 on failure.
// FUNCTION: 0x000bac70
void *LbMemoryAllocLow(u32 size)
{
    return _lbMemory.Alloc(size, 1);
}

// Frees a block from LbMemoryAlloc. Always returns 1.
// FUNCTION: 0x000baca4
int LbMemoryFree(void *p)
{
    _lbHeap.Free(p);
    return 1;
}

// Reserves the game heap: about 5.4 MB if available, else the largest size from
// 4.7 MB upwards in 150 KB steps that can be allocated. Picks the sound heap
// size (D_0015642f) and sound file set (D_00156433) to match the memory
// obtained. Returns 1, or -1 if no heap could be allocated.
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

// Releases every block _lbMemory allocated, including the game heap. Returns 1.
// FUNCTION: 0x000badfc
int LbMemoryReset(void)
{
    fn_000bb0d0(&_lbMemory);
    return 1;
}

// Does nothing in this build.
// FUNCTION: 0x000bae28
void LbMemorySetupMaxSize(u32 size)
{
}

// Not supported: always returns -1.
// FUNCTION: 0x000bae44
int LbMemoryShrink(void *p, u32 size)
{
    return -1;
}

// Clears lbMemoryAvailable (its last entry to -1) and returns -1: the free
// memory is not measured in this build.
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

// Not supported: always returns -1.
// FUNCTION: 0x000baec0
int fn_000baec0(void *p, u32 size)
{
    return -1;
}

}

// Puts all 100 block records on the free list, with nothing allocated.
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

// Allocates size bytes, zeroed: from DOS conventional memory if low is nonzero,
// else from the C heap (rounded up to 32 bytes). Returns 0 if size is 0, the
// 100 block records are used up, or the allocation fails.
// FUNCTION: 0x000baf94
void *_CMemory::Alloc(u32 size, u32 low)
{
    _CMemoryBlock *block;

    if (size == 0)
        return 0;
    if (free_list == 0)
        return 0;
    block = free_list;
    if (low) {
        u32 seg;

        seg = dos_alloc(size);
        if (seg == 0)
            return 0;
        block->seg = seg >> 16;
        block->ptr = (void *)((seg & 0xffff) << 4);
        block->size = size;
        memset(block->ptr, 0, size);
    } else {
        void *p;

        p = malloc(((size + 31) >> 5) << 5);
        if (p == 0)
            return 0;
        memset(p, 0, ((size + 31) >> 5) << 5);
        block->seg = 0;
        block->ptr = p;
        block->size = size;
    }
    free_list = free_list->next;
    block->next = used_list;
    used_list = block;
    return block->ptr;
}

extern "C" {

// _CMemory::ReleaseAll: frees every block mem has allocated.
// FUNCTION: 0x000bb0d0
void fn_000bb0d0(_CMemory *mem)
{
    while (mem->used_list)
        fn_000bb10c(mem, mem->used_list->ptr);
}

// _CMemory::Free: frees the block of mem whose address is p. Returns 1 when
// freed or p is 0, 0 if p is not one of mem's blocks.
// FUNCTION: 0x000bb10c
u32 fn_000bb10c(_CMemory *mem, void *p)
{
    _CMemoryBlock *block;
    _CMemoryBlock *prev;

    if (p == 0)
        return 1;
    block = mem->used_list;
    prev = mem->used_list;
    while (block) {
        if (block->ptr == p)
            break;
        prev = block;
        block = block->next;
    }
    if (block == 0)
        return 0;
    if (block->seg)
        dos_free(block->seg);
    else
        free(block->ptr);
    if (block == mem->used_list)
        mem->used_list = block->next;
    else
        prev->next = block->next;
    block->next = mem->free_list;
    mem->free_list = block;
    return 1;
}

}

// Allocates size bytes (under 640 KB) of DOS memory with DPMI function 0x100.
// Returns the selector in the high word and the real-mode segment in the low
// word, or 0 on failure.
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

// Frees DOS memory by its selector (DPMI function 0x101); 0 is ignored.
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

// The low-memory flag LbMemorySetup passes to _CMemory::Alloc: always 0, so
// the game heap comes from the C heap.
// FUNCTION: 0x000bb2b0 COMDAT
extern "C" inline u32 fn_000bb2b0(void)
{
    return 0;
}

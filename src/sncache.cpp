#include "hosp.h"

extern "C" {
void fn_000a2770(TbCacheUsedBlock *block);
}

// FUNCTION: 0x000a2260
void TbCache::UsedAndLocked(long &used, long &locked)
{
    TbCacheUsedBlock *b;

    used = 0;
    locked = 0;
    for (b = oldest; b != 0; b = b->next) {
        ++used;
        if (b->locked)
            ++locked;
    }
}

// FUNCTION: 0x000a22bc
int TbCache::Initialise(void *mem, u32 size)
{
    if (size <= 0x14)
        return 0;
    this->mem = mem;
    this->size = size;
    oldest = newest = 0;
    free = (TbCacheFreeBlock *)mem;
    free->size = size;
    end = (u32)mem + size;
    count = 0;
    return 1;
}

// FUNCTION: 0x000a2344
void TbCache::UnlockAll(void)
{
    TbCacheUsedBlock *b;

    for (b = oldest; b != 0; b = b->next)
        fn_000a2770(b);
}

// FUNCTION: 0x000a2384
TbCacheUsedBlock *TbCache::Allocate(u32 size, u32 tag)
{
    u32 total;
    TbCacheFreeBlock *prev;
    TbCacheFreeBlock *f;
    TbCacheFreeBlock *rest;
    TbCacheUsedBlock *b;

    total = size + 0x14;
    for (f = free, prev = 0; f != 0; prev = f, f = f->next) {
        if (f->size < total)
            continue;
        if (total + 0x14 < f->size) {
            rest = (TbCacheFreeBlock *)((u32)f + total);
            rest->next = f->next;
            f->next = rest;
            rest->size = f->size - total;
        }
        if (prev)
            prev->next = f->next;
        else
            free = f->next;
        count++;
        b = (TbCacheUsedBlock *)f;
        b->next = 0;
        b->prev = newest;
        if (newest)
            newest->next = b;
        newest = b;
        if (count == 1)
            oldest = b;
        b->tag = tag;
        b->size = total;
        return b;
    }
    return 0;
}

// NONMATCHING: register choice
// FUNCTION: 0x000a24b0
void TbCache::Free(TbCacheUsedBlock *block)
{
    TbCacheFreeBlock *f;
    TbCacheFreeBlock *p;
    u32 size;
    u32 gap;
    int merged;

    f = (TbCacheFreeBlock *)block;
    size = block->size;
    if (--count == 0) {
        oldest = newest = 0;
        free = (TbCacheFreeBlock *)mem;
        free->size = this->size;
        free->next = 0;
        return;
    }
    if (block->next)
        block->next->prev = block->prev;
    else
        newest = block->prev;
    if (block->prev)
        block->prev->next = block->next;
    else
        oldest = block->next;
    f->size = size;
    if (free == 0) {
        free = f;
        f->next = 0;
    } else if (free > f) {
        f->next = free;
        free = f;
    } else {
        for (p = free; p->next != 0; p = p->next) {
            if (p->next > f)
                break;
        }
        f->next = p->next;
        p->next = f;
    }
    for (p = free; p != 0 && p->next != 0; p = p->next) {
        do {
            gap = (u32)p->next - ((u32)p + p->size);
            if (gap < 8) {
                p->size = gap + (p->size + p->next->size);
                p->next = p->next->next;
                merged = 1;
            } else {
                merged = 0;
            }
        } while (merged && p->next != 0);
    }
}

// FUNCTION: 0x000a2678
void TbCache::MakeNewest(TbCacheUsedBlock *block)
{
    if (block == newest)
        return;
    block->next->prev = block->prev;
    if (block->prev)
        block->prev->next = block->next;
    if (block == oldest)
        oldest = block->next;
    block->prev = newest;
    block->next = 0;
    newest->next = block;
    newest = block;
}

// FUNCTION: 0x000a2704
u32 TbCache::ThrowOutOldest(void)
{
    TbCacheUsedBlock *b;
    u32 tag;

    for (b = oldest; b != 0; b = b->next) {
        if (b->locked == 0) {
            tag = b->tag;
            Free(b);
            return tag;
        }
    }
    return 0;
}

// FUNCTION: 0x000a2770
void fn_000a2770(TbCacheUsedBlock *block)
{
    block->locked = 0;
}

#include "hosp.h"

extern "C" {
void fn_000a2770(TbCacheUsedBlock *block);
}

// FUNCTION: 0x000a2260
void TbCache::UsedAndLocked(long &used, long &locked)
{
    TbCacheUsedBlock *block;

    used = 0;
    locked = 0;
    for (block = oldest; block != 0; block = block->next) {
        ++used;
        if (block->locked)
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
    TbCacheUsedBlock *block;

    for (block = oldest; block != 0; block = block->next)
        fn_000a2770(block);
}

// FUNCTION: 0x000a2384
TbCacheUsedBlock *TbCache::Allocate(u32 size, u32 tag)
{
    u32 total;
    TbCacheFreeBlock *prev;
    TbCacheFreeBlock *free_block;
    TbCacheFreeBlock *rest;
    TbCacheUsedBlock *used_block;

    total = size + 0x14;
    for (free_block = free, prev = 0; free_block != 0; prev = free_block, free_block = free_block->next) {
        if (free_block->size < total)
            continue;
        if (total + 0x14 < free_block->size) {
            rest = (TbCacheFreeBlock *)((u32)free_block + total);
            rest->next = free_block->next;
            free_block->next = rest;
            rest->size = free_block->size - total;
        }
        if (prev)
            prev->next = free_block->next;
        else
            free = free_block->next;
        count++;
        used_block = (TbCacheUsedBlock *)free_block;
        used_block->next = 0;
        used_block->prev = newest;
        if (newest)
            newest->next = used_block;
        newest = used_block;
        if (count == 1)
            oldest = used_block;
        used_block->tag = tag;
        used_block->size = total;
        return used_block;
    }
    return 0;
}

// FUNCTION: 0x000a24b0
void TbCache::Free(TbCacheUsedBlock *block)
{
    TbCacheFreeBlock *freed;
    TbCacheFreeBlock *node;
    u32 size;
    u32 gap;
    int merged;

    freed = (TbCacheFreeBlock *)block;
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
    freed->size = size;
    if (free == 0) {
        free = freed;
        freed->next = 0;
    } else if (free > freed) {
        freed->next = free;
        free = freed;
    } else {
        for (node = free; node->next != 0; node = node->next) {
            if (node->next > freed)
                break;
        }
        freed->next = node->next;
        node->next = freed;
    }
    for (node = free; node != 0 && node->next != 0; node = node->next) {
        do {
            gap = (u32)node->next - ((u32)node + node->size);
            if (gap < 8) {
                node->size = gap + (node->size + node->next->size);
                node->next = node->next->next;
                merged = 1;
            } else {
                merged = 0;
            }
        } while (merged && node->next != 0);
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
    TbCacheUsedBlock *block;
    u32 tag;

    for (block = oldest; block != 0; block = block->next) {
        if (block->locked == 0) {
            tag = block->tag;
            Free(block);
            return tag;
        }
    }
    return 0;
}

// FUNCTION: 0x000a2770 COMDAT
inline void fn_000a2770(TbCacheUsedBlock *block)
{
    block->locked = 0;
}

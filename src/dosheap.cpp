#include "bflib.h"

typedef char *va_list[1];
#define va_start(ap, last) ((ap)[0] = (char *)&(last) + sizeof(last))
#define va_end(ap) ((ap)[0] = 0)

extern "C" {

int vsprintf(char *buf, char *fmt, va_list args);
void fn_000bb5cc(CDosHeapStats *stats);
u32 GetFreeFragCount(CDosHeap *heap);
}

// Builds an empty heap: all 200 block records unused, no free or used blocks,
// statistics zeroed, reporting and error messages off.
// FUNCTION: 0x000bb2e0
CDosHeap::CDosHeap()
{
    u32 i;

    for (i = 0; i < 200; i++)
        ((CDosHeapStats *)&items[i])->Init();
    free_list = 0;
    used_list = 0;
    fn_000bb5cc(&stats);
    errors = 0;
    reporting = 0;
}

// Zeroes the statistics.
// FUNCTION: 0x000bb36c
void CDosHeapStats::Init(void)
{
    total = 0;
    count = 0;
}

// Allocates size bytes from the start of the first free block big enough
// (first fit; no alignment). Returns 0, with an error message if enabled, when
// no block or record is free.
// FUNCTION: 0x000bb39c
void *CDosHeap::Alloc(u32 size)
{
    CDosHeapItem *rec;

    rec = GetNextRecord();
    if (rec) {
        CDosHeapItem *free_block;

        free_block = free_list;
        while (free_block && free_block->size < size)
            free_block = free_block->next;
        if (free_block) {
            rec->ptr = free_block->ptr;
            rec->size = size;
            free_block->ptr += size;
            free_block->size -= size;
            rec->next = used_list;
            used_list = rec;
            stats.RecordAlloc(rec);
            Report("Allocated block (Count%d,Frags%d)of %d bytes, \t%d allocated", stats.GetAllocCount(), GetFreeFragCount(this), size, stats.GetTotalAllocation());
            return rec->ptr;
        }
    }
    Error("-*-*-*-*-> allocation failed! (%d) <-*-*-*-*-", size);
    return 0;
}

// Returns an unused block record (ptr 0), or 0 with an error message if all
// 200 are in use.
// FUNCTION: 0x000bb4b8
CDosHeapItem *CDosHeap::GetNextRecord(void)
{
    u32 i;

    for (i = 0; i < 200; i++) {
        if (items[i].ptr == 0)
            return &items[i];
    }
    Error("ran out of records");
    return 0;
}

// Prints a printf-style error message, if errors are enabled.
// FUNCTION: 0x000bb524
void CDosHeap::Error(char *fmt, ...)
{
    va_list args;
    char buf[512];

    if (errors) {
        va_start(args, fmt);
        vsprintf(buf, fmt, args);
        va_end(args);
        printf("Dos Heap Error : %s\n", buf);
    }
}

// Makes the size bytes at mem the heap's single free block.
// FUNCTION: 0x000bb57c
void CDosHeap::Init(void *mem, u32 size)
{
    CDosHeapItem *rec;

    rec = GetNextRecord();
    rec->ptr = (u8 *)mem;
    rec->size = size;
    rec->next = 0;
    free_list = rec;
}

extern "C" {

// Zeroes stats (as CDosHeapStats::Init).
// FUNCTION: 0x000bb5cc
void fn_000bb5cc(CDosHeapStats *stats)
{
    stats->total = 0;
    stats->count = 0;
}

}

// Frees the block allocated at p, merging it into an adjacent free block where
// possible. On an unknown p it reads through a null record to report the
// error.
// FUNCTION: 0x000bb5fc
void CDosHeap::Free(void *p)
{
    CDosHeapItem *rec;

    rec = RemoveAllocMatch(p);
    if (rec) {
        stats.RecordFree(rec);
        if (!MergeRecord(rec)) {
            rec->next = free_list;
            free_list = rec;
            Report("freed block (Count%d,Frags%d) of %d bytes,\t%d allocated", stats.GetAllocCount(), GetFreeFragCount(this), rec->size, stats.GetTotalAllocation());
        } else {
            ReleaseRecord(rec);
            Report("freed & merged block (Count%d,Frags%d) of %d bytes,\t%d allocated", stats.GetAllocCount(), GetFreeFragCount(this), rec->size, stats.GetTotalAllocation());
        }
    } else
        Error("Free failed to free block at address %d", rec->ptr);
}

// Unlinks and returns the used block record at address p, or 0 if there is
// none.
// FUNCTION: 0x000bb708
CDosHeapItem *CDosHeap::RemoveAllocMatch(void *p)
{
    CDosHeapItem *cur;
    CDosHeapItem *prev;

    cur = used_list;
    prev = 0;
    while (cur && cur->ptr != p) {
        prev = cur;
        cur = cur->next;
    }
    if (cur) {
        if (prev)
            prev->next = cur->next;
        else
            used_list = cur->next;
    }
    return cur;
}

// Unlinks free block item from the free list and releases its record. Returns
// 1 if it was on the list, else 0.
// FUNCTION: 0x000bb78c
u32 CDosHeap::ReleaseFreeBlock(CDosHeapItem *item)
{
    CDosHeapItem *cur;
    CDosHeapItem *prev;

    cur = free_list;
    prev = 0;
    while (cur && cur != item) {
        prev = cur;
        cur = cur->next;
    }
    if (cur) {
        if (prev)
            prev->next = cur->next;
        else
            free_list = cur->next;
        ReleaseRecord(cur);
    }
    return cur ? 1 : 0;
}

// Grows a free block that directly precedes or follows item to cover item too.
// Returns 1 if one was found, else 0 (item is unchanged either way).
// FUNCTION: 0x000bb830
u32 CDosHeap::MergeRecord(CDosHeapItem *item)
{
    CDosHeapItem *free_block;
    u8 *start;
    u8 *end;

    free_block = free_list;
    start = item->ptr;
    end = item->ptr + item->size;
    while (free_block && free_block->ptr != end && free_block->ptr + free_block->size != start)
        free_block = free_block->next;
    if (free_block) {
        if (free_block->ptr == end)
            free_block->ptr -= item->size;
        free_block->size += item->size;
        return 1;
    }
    return 0;
}

// Merges free blocks that are next to each other, releasing the records of
// those merged into others.
// FUNCTION: 0x000bb8dc
void CDosHeap::Repack(void)
{
    CDosHeapItem *free_block;
    u32 before;
    u32 merged;

    free_block = free_list;
    before = GetFreeFragCount(this);
    merged = 1;
    while (merged) {
        merged = 0;
        while (free_block && !merged) {
            merged = MergeRecord(free_block);
            if (merged)
                ReleaseFreeBlock(free_block);
            free_block = free_block->next;
        }
    }
    Report("Repacking the heap, %d to %d", before, GetFreeFragCount(this));
}

// Prints a printf-style trace message, if reporting is enabled.
// FUNCTION: 0x000bb97c
void CDosHeap::Report(char *fmt, ...)
{
    va_list args;
    char buf[512];

    if (reporting) {
        va_start(args, fmt);
        vsprintf(buf, fmt, args);
        va_end(args);
        printf("Dos Heap -- %s\n", buf);
    }
}

// Turns trace messages on (nonzero) or off.
// FUNCTION: 0x000bb9d4
void CDosHeap::SetReporting(u32 on)
{
    reporting = on;
}

// Turns error messages on (nonzero) or off.
// FUNCTION: 0x000bb9fc
void CDosHeap::SetErrors(u32 on)
{
    errors = on;
}

// Marks block record item unused.
// FUNCTION: 0x000bba24
void CDosHeap::ReleaseRecord(CDosHeapItem *item)
{
    item->ptr = 0;
}

// Counts item as allocated in the statistics.
// FUNCTION: 0x000bba4c
void CDosHeapStats::RecordAlloc(CDosHeapItem *item)
{
    total += item->size;
    count++;
}

// Removes item from the statistics.
// FUNCTION: 0x000bba78
void CDosHeapStats::RecordFree(CDosHeapItem *item)
{
    total -= item->size;
    count--;
}

// Returns the bytes currently allocated.
// FUNCTION: 0x000bbaa4
u32 CDosHeapStats::GetTotalAllocation(void)
{
    return total;
}

// Returns the number of blocks currently allocated.
// FUNCTION: 0x000bbacc
u32 CDosHeapStats::GetAllocCount(void)
{
    return count;
}

extern "C" {

// Returns the number of blocks on heap's free list.
// FUNCTION: 0x000bbaf4
u32 GetFreeFragCount(CDosHeap *heap)
{
    u32 frags;
    CDosHeapItem *free_block;

    frags = 0;
    free_block = heap->free_list;
    while (free_block) {
        frags++;
        free_block = free_block->next;
    }
    return frags;
}

}

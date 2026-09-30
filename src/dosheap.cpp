#include "bflib.h"

typedef char *va_list[1];
#define va_start(ap, last) ((ap)[0] = (char *)&(last) + sizeof(last))
#define va_end(ap) ((ap)[0] = 0)

extern "C" {

int vsprintf(char *buf, char *fmt, va_list args);
void fn_000bb5cc(CDosHeapStats *stats);
u32 fn_000bbaf4(CDosHeap *heap);
}

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

// FUNCTION: 0x000bb36c
void CDosHeapStats::Init(void)
{
    total = 0;
    count = 0;
}

// FUNCTION: 0x000bb39c
void *CDosHeap::Alloc(u32 size)
{
    CDosHeapItem *rec;

    rec = GetNextRecord();
    if (rec) {
        CDosHeapItem *f;

        f = free_list;
        while (f && f->size < size)
            f = f->next;
        if (f) {
            rec->ptr = f->ptr;
            rec->size = size;
            f->ptr += size;
            f->size -= size;
            rec->next = used_list;
            used_list = rec;
            stats.RecordAlloc(rec);
            Report("Allocated block (Count%d,Frags%d)of %d bytes, \t%d allocated", stats.GetAllocCount(), fn_000bbaf4(this), size, stats.GetTotalAllocation());
            return rec->ptr;
        }
    }
    Error("-*-*-*-*-> allocation failed! (%d) <-*-*-*-*-", size);
    return 0;
}

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

// FUNCTION: 0x000bb5cc
void fn_000bb5cc(CDosHeapStats *stats)
{
    stats->total = 0;
    stats->count = 0;
}

}

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
            Report("freed block (Count%d,Frags%d) of %d bytes,\t%d allocated", stats.GetAllocCount(), fn_000bbaf4(this), rec->size, stats.GetTotalAllocation());
        } else {
            ReleaseRecord(rec);
            Report("freed & merged block (Count%d,Frags%d) of %d bytes,\t%d allocated", stats.GetAllocCount(), fn_000bbaf4(this), rec->size, stats.GetTotalAllocation());
        }
    } else
        Error("Free failed to free block at address %d", rec->ptr);
}

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

// FUNCTION: 0x000bb830
u32 CDosHeap::MergeRecord(CDosHeapItem *item)
{
    CDosHeapItem *f;
    u8 *start;
    u8 *end;

    f = free_list;
    start = item->ptr;
    end = item->ptr + item->size;
    while (f && f->ptr != end && f->ptr + f->size != start)
        f = f->next;
    if (f) {
        if (f->ptr == end)
            f->ptr -= item->size;
        f->size += item->size;
        return 1;
    }
    return 0;
}

// FUNCTION: 0x000bb8dc
void CDosHeap::Repack(void)
{
    CDosHeapItem *f;
    u32 before;
    u32 merged;

    f = free_list;
    before = fn_000bbaf4(this);
    merged = 1;
    while (merged) {
        merged = 0;
        while (f && !merged) {
            merged = MergeRecord(f);
            if (merged)
                ReleaseFreeBlock(f);
            f = f->next;
        }
    }
    Report("Repacking the heap, %d to %d", before, fn_000bbaf4(this));
}

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

// FUNCTION: 0x000bb9d4
void CDosHeap::SetReporting(u32 on)
{
    reporting = on;
}

// FUNCTION: 0x000bb9fc
void CDosHeap::SetErrors(u32 on)
{
    errors = on;
}

// FUNCTION: 0x000bba24
void CDosHeap::ReleaseRecord(CDosHeapItem *item)
{
    item->ptr = 0;
}

// FUNCTION: 0x000bba4c
void CDosHeapStats::RecordAlloc(CDosHeapItem *item)
{
    total += item->size;
    count++;
}

// FUNCTION: 0x000bba78
void CDosHeapStats::RecordFree(CDosHeapItem *item)
{
    total -= item->size;
    count--;
}

// FUNCTION: 0x000bbaa4
u32 CDosHeapStats::GetTotalAllocation(void)
{
    return total;
}

// FUNCTION: 0x000bbacc
u32 CDosHeapStats::GetAllocCount(void)
{
    return count;
}

extern "C" {

// FUNCTION: 0x000bbaf4
u32 fn_000bbaf4(CDosHeap *heap)
{
    u32 n;
    CDosHeapItem *f;

    n = 0;
    f = heap->free_list;
    while (f) {
        n++;
        f = f->next;
    }
    return n;
}

}

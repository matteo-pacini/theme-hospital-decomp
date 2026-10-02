#include "ail.h"

#define AIL_NO_ERROR 0
#define AIL_OUT_OF_MEMORY 2
#define AIL_FILE_NOT_FOUND 3
#define AIL_CANT_WRITE_FILE 4
#define AIL_CANT_READ_FILE 5
#define AIL_DISK_FULL 6

#define FILE_READ_WITH_SIZE ((void *)-1)

extern S32 disk_err;

// Makes `fn` the allocator behind AIL_mem_alloc_lock; returns the previous one.
// FUNCTION: 0x000dda40
void *__cdecl AIL_mem_use_malloc(void *(__pascal *fn)(U32 size))
{
    void *(__pascal *old)(U32 size);

    old = AIL_mem_alloc;
    AIL_mem_alloc = fn;
    return old;
}

// Makes `fn` the deallocator behind AIL_mem_free_lock; returns the previous
// one.
// FUNCTION: 0x000dda50
void *__cdecl AIL_mem_use_free(void (__pascal *fn)(void *ptr))
{
    void (__pascal *old)(void *ptr);

    old = AIL_mem_free;
    AIL_mem_free = fn;
    return old;
}

// Allocates `size` bytes through the current allocator and locks them against
// paging; a hidden size word precedes the block. Returns NULL on failure.
// FUNCTION: 0x000dda60
void *__cdecl AIL_mem_alloc_lock(U32 size)
{
    U32 *ptr;

    size += 4;
    ptr = AIL_mem_alloc(size);
    if (ptr != NULL) {
        AIL_vmm_lock(ptr, size);
        *ptr = size;
        ptr++;
    }
    return ptr;
}

// Unlocks and frees a block from AIL_mem_alloc_lock; NULL is ignored.
// FUNCTION: 0x000dda90
void __cdecl AIL_mem_free_lock(void *ptr)
{
    U32 *p;

    if (ptr == NULL) {
        return;
    }
    p = ptr;
    AIL_vmm_unlock(p, p[-1]);
    AIL_mem_free(p - 1);
}

// Allocates `n_paras` paragraphs of DOS memory (DPMI 0100h) and locks them. On
// success stores its linear address in `*protected_ptr`, its real-mode far
// pointer (segment:0) in `*segment_far_ptr` and its selector in `*selector`,
// and returns 1; returns 0 if the DPMI call fails.
// FUNCTION: 0x000ddac0
S32 __cdecl AIL_mem_alloc_DOS(U32 n_paras, void **protected_ptr,
                              U32 *segment_far_ptr, U32 *selector)
{
    S32 result;
    union REGS inregs, outregs;

    inregs.x.eax = 0x100;
    inregs.x.ebx = n_paras;
    AIL_int386(0x31, &inregs, &outregs);
    if (outregs.x.cflag) {
        result = 0;
    } else {
        *segment_far_ptr = outregs.x.eax << 16;
        *selector = outregs.x.edx & 0xffff;
        *protected_ptr = (void *)((outregs.x.eax & 0xffff) << 4);
        result = 1;
    }
    if (result) {
        AIL_vmm_lock_range((void *)(*segment_far_ptr >> 12),
                           (void *)((*segment_far_ptr >> 12) + n_paras * 16 - 1));
    }
    return result;
}

// Frees DOS memory from AIL_mem_alloc_DOS by its `selector` (DPMI 0101h); the
// other two parameters are unused.
// FUNCTION: 0x000ddb50
void __cdecl AIL_mem_free_DOS(void *protected_ptr, U32 segment_far_ptr,
                              U32 selector)
{
    union REGS inregs, outregs;

    inregs.x.eax = 0x101;
    inregs.x.edx = selector & 0xffff;
    AIL_int386(0x31, &inregs, &outregs);
    /* No-ops that reference the otherwise unused parameters; they leave two dead compares. */
    if (protected_ptr == NULL) {
        protected_ptr = NULL;
    }
    if (segment_far_ptr == 0) {
        segment_far_ptr = 0;
    }
}

// Locks the linear range between `p1` and `p2` inclusive (either order) against
// paging (DPMI 0600h). Returns 1 on success, 0 on failure.
// FUNCTION: 0x000ddb90
S32 __cdecl AIL_vmm_lock_range(void *p1, void *p2)
{
    union REGS inregs, outregs;
    U32 start, len;

    memset(&inregs, 0, sizeof(inregs));
    start = (U32)((p1 < p2) ? p1 : p2);
    len = (U32)((p1 > p2) ? p1 : p2) - start + 1;
    inregs.x.eax = 0x600;
    inregs.x.ebx = start >> 16;
    inregs.x.ecx = start & 0xffff;
    inregs.x.esi = len >> 16;
    inregs.x.edi = len & 0xffff;
    AIL_int386(0x31, &inregs, &outregs);
    return !outregs.x.cflag;
}

// Unlocks the linear range between `p1` and `p2` inclusive (either order) (DPMI
// 0601h). Returns 1 on success, 0 on failure.
// FUNCTION: 0x000ddc30
S32 __cdecl AIL_vmm_unlock_range(void *p1, void *p2)
{
    union REGS inregs, outregs;
    U32 start, len;

    start = (U32)((p1 < p2) ? p1 : p2);
    len = (U32)((p1 > p2) ? p1 : p2) - start + 1;
    memset(&inregs, 0, sizeof(inregs));
    inregs.x.eax = 0x601;
    inregs.x.ebx = start >> 16;
    inregs.x.ecx = start & 0xffff;
    inregs.x.esi = len >> 16;
    inregs.x.edi = len & 0xffff;
    AIL_int386(0x31, &inregs, &outregs);
    return !outregs.x.cflag;
}

// Locks the `size` bytes at `start` against paging. Returns 1 on success.
// FUNCTION: 0x000ddcd0
S32 __cdecl AIL_vmm_lock(void *start, U32 size)
{
    return AIL_vmm_lock_range(start, (U8 *)start + size - 1);
}

// Unlocks the `size` bytes at `start`. Returns 1 on success.
// FUNCTION: 0x000ddcf0
S32 __cdecl AIL_vmm_unlock(void *start, U32 size)
{
    return AIL_vmm_unlock_range(start, (U8 *)start + size - 1);
}

// Returns the AIL_... error code of the last AIL_file_* call (0 for none).
// FUNCTION: 0xddd10
S32 __cdecl AIL_file_error(void)
{
    return disk_err;
}

// Returns the size of file `filename` in bytes, or -1 if it cannot be opened
// (AIL_file_error then gives AIL_FILE_NOT_FOUND) or the seek to its end fails
// (AIL_CANT_READ_FILE).
// FUNCTION: 0x000ddd20
S32 __cdecl AIL_file_size(char *filename)
{
    S32 handle;
    S32 len;

    disk_err = AIL_NO_ERROR;
    handle = AIL_fopen(filename, 0);
    if (handle == 0 || handle == -1) {
        disk_err = AIL_FILE_NOT_FOUND;
        return -1;
    }
    len = AIL_fseekend(handle, 0);
    if (len == -1) {
        disk_err = AIL_CANT_READ_FILE;
    }
    AIL_fclose(handle);
    return len;
}

// Reads all of file `filename`. With `dest` NULL the data goes into a new
// locked block (free with AIL_mem_free_lock); with FILE_READ_WITH_SIZE
// likewise, but the block starts with the U32 file size and the data follows;
// otherwise into `dest`. Returns the buffer, or NULL with AIL_file_error set.
// FUNCTION: 0x000ddd80
void *__cdecl AIL_file_read(char *filename, void *dest)
{
    S32 handle;
    U32 len;
    U8 *buf;
    U8 *read_ptr;

    disk_err = AIL_NO_ERROR;
    handle = AIL_fopen(filename, 0);
    if (handle == 0 || handle == -1) {
        disk_err = AIL_FILE_NOT_FOUND;
        return NULL;
    }
    len = AIL_fseekend(handle, 0);
    AIL_fseekbegin(handle, 0);
    buf = (dest == NULL || dest == FILE_READ_WITH_SIZE)
              ? AIL_mem_alloc_lock(len + 4)
              : dest;
    read_ptr = buf;
    if (buf == NULL) {
        disk_err = AIL_OUT_OF_MEMORY;
        AIL_fclose(handle);
        return NULL;
    }
    if (dest == FILE_READ_WITH_SIZE) {
        *(U32 *)buf = len;
        read_ptr = buf + 4;
    }
    if (AIL_fread(handle, read_ptr, len) != len) {
        if (dest != buf) {
            AIL_mem_free_lock(buf);
        }
        disk_err = AIL_CANT_READ_FILE;
        AIL_fclose(handle);
        return NULL;
    }
    AIL_fclose(handle);
    return buf;
}

// Creates file `filename` holding the `len` bytes at `buf`. Returns 1, or 0
// with AIL_file_error set to AIL_CANT_WRITE_FILE (a short write leaves the file
// open; AIL_DISK_FULL is never reported).
// FUNCTION: 0x000dde70
S32 __cdecl AIL_file_write(char *filename, void *buf, U32 len)
{
    S32 handle;
    U32 written;

    disk_err = AIL_NO_ERROR;
    handle = AIL_fcreate(filename);
    if (handle == 0 || handle == -1) {
        disk_err = AIL_CANT_WRITE_FILE;
        return 0;
    }
    written = AIL_fwrite(handle, buf, len);
    if (written != len) {
        disk_err = AIL_CANT_WRITE_FILE;
        return 0;
    }
    if (written != len) {
        disk_err = AIL_DISK_FULL;
        return 0;
    }
    AIL_fclose(handle);
    return 1;
}

// Appends the `len` bytes at `buf` to file `filename`, creating it if needed.
// Returns 1, or 0 with AIL_file_error set to AIL_CANT_WRITE_FILE (a short write
// leaves the file open; AIL_DISK_FULL is never reported).
// FUNCTION: 0xddf00
S32 __cdecl AIL_file_append(char *filename, void *buf, U32 len)
{
    S32 handle;
    U32 written;

    disk_err = AIL_NO_ERROR;
    handle = AIL_fappend(filename);
    if (handle == 0 || handle == -1) {
        disk_err = AIL_CANT_WRITE_FILE;
        return 0;
    }
    written = AIL_fwrite(handle, buf, len);
    if (written != len) {
        disk_err = AIL_CANT_WRITE_FILE;
        return 0;
    }
    if (written != len) {
        disk_err = AIL_DISK_FULL;
        return 0;
    }
    AIL_fclose(handle);
    return 1;
}

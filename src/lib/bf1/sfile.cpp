#include "bflib.h"

struct find_t {
    char reserved[21];
    s8 attrib;
    u16 wr_time;
    u16 wr_date;
    u32 size;
    char name[13];
};

enum TbFileOpenMode {
    Lb_FILE_MODE_NEW = 0,
    Lb_FILE_MODE_OLD = 1,
    Lb_FILE_MODE_READ_ONLY = 2
};

struct TbDate {
    u8 Day;
    u8 Month;
    u16 Year;
    u8 DayOfWeek;
};

struct TbTime {
    u8 Hour;
    u8 Minute;
    u8 Second;
    u8 HSecond;
};

struct TbFileFind {
    char Filename[144];
    char AlternateFilename[14];
    s32 Attributes;
    u32 Length;
    TbDate CreationDate;
    TbTime CreationTime;
    TbDate LastWriteDate;
    TbTime LastWriteTime;
    find_t Reserved;
};

extern "C" {
int access(char *path, int mode);
int tell(int handle);
int creat(char *name, int pmode);
int setmode(int handle, int mode);
int close(int handle);
int sopen(char *name, int mode, int shflag, ...);
int lseek(int handle, int offset, int origin);
int read(int handle, void *buf, u32 len);
int write(int handle, void *buffer, u32 len);
int filelength(int handle);
u32 _dos_findfirst(char *path, u32 attr, find_t *buf);
u32 _dos_findnext(find_t *buf);
int remove(const char *filename);
int rename(char *from, char *to);
char *strcpy(char *dst, const char *src);
#pragma intrinsic(strcpy)
}

void convert_find_info(TbFileFind *ff);

extern "C" {

// Returns 1 if file path exists, else 0.
// FUNCTION: 0x000be290
int LbFileExists(char *path)
{
    return access(path, 0) == 0;
}

// Returns the current position in the open file, or -1.
// FUNCTION: 0x000be2b4
int LbFilePosition(int handle)
{
    int pos;

    pos = tell(handle);
    if (pos == -1)
        return -1;
    return pos;
}

int LbFileOpenMode(char *path, TbFileOpenMode mode);
#pragma aux LbFileOpenMode "LbFileOpen_"

// Opens file path: Lb_FILE_MODE_NEW creates or empties it, OLD opens it for
// reading and writing (creating it if missing), READ_ONLY opens an existing
// file for reading. Binary, shared. Returns the handle, or -1.
// FUNCTION: 0x000be2e0
int LbFileOpenMode(char *path, TbFileOpenMode mode)
{
    int handle;

    if (!LbFileExists(path)) {
        if (mode == Lb_FILE_MODE_READ_ONLY)
            return -1;
        else if (mode == Lb_FILE_MODE_OLD)
            mode = Lb_FILE_MODE_NEW;
    }
    if (mode == Lb_FILE_MODE_NEW) {
        handle = creat(path, 0x1c0);
        setmode(handle, 0x200);
        close(handle);
    }
    handle = -1;
    switch (mode) {
    case Lb_FILE_MODE_READ_ONLY:
        handle = sopen(path, 0x200, 0x40);
        break;
    case Lb_FILE_MODE_NEW:
        handle = sopen(path, 0x222, 0x40);
        break;
    case Lb_FILE_MODE_OLD:
        handle = sopen(path, 0x202, 0x40);
        break;
    }
    if (handle == -1)
        return -1;
    return handle;
}

// Closes the open file. Returns 1, or -1 on failure.
// FUNCTION: 0x000be3bc
int LbFileClose(int handle)
{
    if (close(handle) == 0)
        return 1;
    return -1;
}

// Moves the open file's position to offset from the start (origin 0), the
// current position (1) or the end (2). Returns the new position, or -1.
// FUNCTION: 0x000be3e4
int LbFileSeek(int handle, int offset, u8 origin)
{
    int result;

    result = -1;
    switch (origin) {
    case 0:
        result = lseek(handle, offset, 0);
        break;
    case 1:
        result = lseek(handle, offset, 1);
        break;
    case 2:
        result = lseek(handle, offset, 2);
        break;
    }
    return result;
}

// Reads up to len bytes from the open file into buf. Returns the count read,
// or -1.
// FUNCTION: 0x000be458
int LbFileRead(int handle, void *buf, int len)
{
    int result;

    result = read(handle, buf, len);
    if (result == -1)
        return -1;
    return result;
}

// Writes len bytes from buf to the open file. Returns the count written, or
// -1.
// FUNCTION: 0x000be490
int LbFileWrite(int handle, void *buf, int len)
{
    int result;

    result = write(handle, buf, len);
    if (result == -1)
        return -1;
    return result;
}

// Returns the size of file path, or -1.
// FUNCTION: 0x000be4c8
int LbFileLength(char *path)
{
    int handle;
    int length;

    handle = LbFileOpen(path, 2);
    if (handle == -1)
        return -1;
    length = filelength(handle);
    LbFileClose(handle);
    if (length == -1)
        return -1;
    return length;
}

// Starts a search for files matching path with attributes attr, filling ff
// with the first match. Returns 1, or -1 if none.
// FUNCTION: 0x000be51c
int LbFileFindFirst(char *path, TbFileFind *ff, u32 attr)
{
    if (_dos_findfirst(path, attr, &ff->Reserved))
        return -1;
    convert_find_info(ff);
    return 1;
}

// Fills ff with the search's next match. Returns 1, or -1 if none is left.
// FUNCTION: 0x000be560
int LbFileFindNext(TbFileFind *ff)
{
    if (_dos_findnext(&ff->Reserved))
        return -1;
    convert_find_info(ff);
    return 1;
}

// Ends a file search (nothing to do under DOS). Returns 1.
// FUNCTION: 0x000be594
int LbFileFindEnd(TbFileFind *ff)
{
    return 1;
}

// Renames file from to to. Returns 1, or -1 on failure.
// FUNCTION: 0x000be5a8
int LbFileRename(char *from, char *to)
{
    if (rename(from, to))
        return -1;
    return 1;
}

// Deletes file path. Returns 1, or -1 on failure.
// FUNCTION: 0x000be5d8
int LbFileDelete(char *path)
{
    if (remove(path))
        return -1;
    return 1;
}

}

// Fills ff's name, size, attributes and last-write date and time from the
// DOS search record (the creation date and time are zeroed).
// FUNCTION: 0x000be600
void convert_find_info(TbFileFind *ff)
{
    strcpy(ff->Filename, ff->Reserved.name);
    strcpy(ff->AlternateFilename, ff->Reserved.name);
    ff->Length = ff->Reserved.size;
    ff->Attributes = ff->Reserved.attrib;
    ff->CreationDate.Day = 0;
    ff->CreationDate.Month = 0;
    ff->CreationDate.Year = 0;
    ff->CreationDate.DayOfWeek = 0;
    ff->CreationTime.HSecond = 0;
    ff->CreationTime.Second = 0;
    ff->CreationTime.Minute = 0;
    ff->CreationTime.Hour = 0;
    ff->LastWriteDate.Day = ff->Reserved.wr_date & 0x1f;
    ff->LastWriteDate.Month = ((u32)ff->Reserved.wr_date & 0x1e0) >> 5;
    ff->LastWriteDate.Year = (((u32)ff->Reserved.wr_date & 0xfe00) >> 9) + 1980;
    ff->LastWriteDate.DayOfWeek = 0;
    ff->LastWriteTime.HSecond = 0;
    ff->LastWriteTime.Second = (ff->Reserved.wr_time & 0x1f) * 2;
    ff->LastWriteTime.Minute = ((u32)ff->Reserved.wr_time & 0x7e0) >> 5;
    ff->LastWriteTime.Hour = ((u32)ff->Reserved.wr_time & 0xf800) >> 11;
}

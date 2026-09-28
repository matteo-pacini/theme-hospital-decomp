#include "clib.h"

// FUNCTION: 0xd11eb
long filelength(int handle)
{
    long current_posn;
    long file_len;

    current_posn = lseek(handle, 0L, SEEK_CUR);
    if (current_posn == -1L)
        return -1L;
    file_len = lseek(handle, 0L, SEEK_END);
    lseek(handle, current_posn, SEEK_SET);
    return file_len;
}

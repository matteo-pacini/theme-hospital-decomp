#include "clib.h"

#define _BINARY 0x0040
#define _APPEND 0x0080
#define _DYNAMIC 0x4000
#define EBADF   4
#define EINVAL  9
#define S_IWRITE 0000200
#define S_IREAD  0000400

int tolower(int c);
int fseek(FILE *fp, long offset, int origin);
FILE *__allocfp(int handle);
void __freefp(FILE *fp);
int __doclose(FILE *fp, int close_handle);
void __chktty(FILE *fp);

extern int _fmode;
extern __stream_link *__ClosedStreams;

// FUNCTION: 0xfbd31
int __open_flags(const char *modestr)
{
    int flags;
    char actionChar;

    flags = 0;
    actionChar = tolower(*modestr);
    if ((int)actionChar != 'r' && (int)actionChar != 'w' && (int)actionChar != 'a') {
        __set_errno(EINVAL);
        return 0;
    }
    if ((int)modestr[1] == '+') {
        flags |= _READ | _WRITE;
        if ((int)modestr[2] == 'b') {
            flags |= _BINARY;
        } else if ((int)modestr[2] != 't' && _fmode == O_BINARY) {
            flags |= _BINARY;
        }
    } else if ((int)modestr[1] == 'b') {
        flags |= _BINARY;
        if ((int)modestr[2] == '+')
            flags |= _READ | _WRITE;
    } else if ((int)modestr[1] == 't') {
        if ((int)modestr[2] == '+')
            flags |= _READ | _WRITE;
    } else if (_fmode == O_BINARY) {
        flags |= _BINARY;
    }
    if ((int)actionChar == 'w') {
        flags |= _WRITE;
    } else if ((int)actionChar == 'a') {
        flags |= _WRITE | _APPEND;
    } else {
        flags |= _READ;
    }
    return flags;
}

// FUNCTION: 0xfbe02
static FILE *__doopen(const char *name, char mode, int file_flags, int shflag, FILE *fp)
{
    int open_mode;

    fp->_flag &= ~(_READ | _WRITE);
    fp->_flag |= file_flags;
    if ((int)(char)tolower(mode) == 'r') {
        open_mode = O_RDONLY;
        if (file_flags & _WRITE)
            open_mode = O_RDWR;
        if (file_flags & _BINARY) {
            open_mode |= O_BINARY;
        } else {
            open_mode |= O_TEXT;
        }
        fp->_handle = sopen(name, open_mode, shflag, 0);
    } else {
        open_mode = (file_flags & _READ) ? O_RDWR | O_CREAT : O_WRONLY | O_CREAT;
        if (file_flags & _APPEND) {
            open_mode |= O_APPEND;
        } else {
            open_mode |= O_TRUNC;
        }
        if (file_flags & _BINARY) {
            open_mode |= O_BINARY;
        } else {
            open_mode |= O_TEXT;
        }
        fp->_handle = sopen(name, open_mode, shflag, S_IREAD | S_IWRITE);
    }
    if (fp->_handle == -1) {
        __freefp(fp);
        return NULL;
    }
    fp->_cnt = 0;
    fp->_base = NULL;
    fp->_bufsize = 0;
    if (file_flags & _APPEND)
        fseek(fp, 0L, SEEK_END);
    __chktty(fp);
    return fp;
}

// FUNCTION: 0xfbec6
FILE *_fsopen(const char *name, const char *access_mode, int shflag)
{
    int file_flags;
    FILE *fp;

    file_flags = __open_flags(access_mode);
    if (file_flags == 0)
        return NULL;
    fp = __allocfp(0);
    if (fp != NULL)
        fp = __doopen(name, *access_mode, file_flags, shflag, fp);
    return fp;
}

// FUNCTION: 0xfbeff
FILE *fopen(const char *name, const char *access_mode)
{
    return _fsopen(name, access_mode, SH_COMPAT);
}

// FUNCTION: 0xfbf09
static FILE *fn_000fbf09(FILE *fp)
{
    __stream_link *link;
    __stream_link **owner;
    __stream_link *next;

    link = __OpenStreams;
    for (;;) {
        if (link == NULL)
            break;
        if (link->stream == fp) {
            if (fp->_flag & (_READ | _WRITE))
                __doclose(fp, 1);
            return fp;
        }
        link = link->next;
    }
    owner = &__ClosedStreams;
    for (;;) {
        link = *owner;
        if (link == NULL)
            break;
        if (link->stream == fp) {
            *owner = link->next;
            next = __OpenStreams;
            __OpenStreams = link;
            link->next = next;
            return fp;
        }
        owner = &link->next;
    }
    __set_errno(EBADF);
    return NULL;
}

// FUNCTION: 0xfbf71
FILE *freopen(const char *name, const char *access_mode, FILE *fp)
{
    int file_flags;

    file_flags = __open_flags(access_mode);
    if (file_flags == 0)
        return NULL;
    fp = fn_000fbf09(fp);
    if (fp != NULL) {
        fp->_flag &= _DYNAMIC;
        fp = __doopen(name, *access_mode, file_flags, 0, fp);
    }
    return fp;
}

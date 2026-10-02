#ifndef CLIB_H
#define CLIB_H

/* Declarations shared by the Watcom C runtime sources (src/lib/clib). */

typedef unsigned size_t;

#define NULL ((void *)0)

extern const char _IsTable[257];

#define _CNTRL 0x01
#define _SPACE 0x02
#define _PUNCT 0x04
#define _PRINT 0x08
#define _XDIGT 0x10
#define _DIGIT 0x20
#define _LOWER 0x40
#define _UPPER 0x80

#define isspace(__c) (_IsTable[(unsigned char)((__c) + 1)] & _SPACE)
#define isdigit(__c) (_IsTable[(unsigned char)((__c) + 1)] & _DIGIT)

size_t strlen(const char *s);
void *memchr(const void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
char *strchr(const char *s, int c);
#ifdef __INLINE_FUNCTIONS__
#pragma intrinsic(memchr, memcmp, strchr, strlen)
#endif

typedef struct __iobuf {
    unsigned char *_ptr;
    // Reading: bytes left in the buffer; writing: bytes buffered.
    int _cnt;
    unsigned char *_base;
    unsigned _flag;
    int _handle;
    unsigned _bufsize;
    // One-byte buffer used when allocating the real one fails.
    unsigned char _ungotten;
    // Number of the tmpfile, for removing it on close.
    unsigned char _tmpfchar;
} FILE;

typedef struct __stream_link {
    struct __stream_link *next;
    FILE *stream;
} __stream_link;

#define _READ 0x0001
#define _WRITE 0x0002
#define _EOF 0x0010
#define _SFERR 0x0020
#define _DIRTY 0x1000
#define _ISTTY 0x2000

#define EOF (-1)
#define ENOSPC 12

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

extern FILE __iob[];
#define stdout (&__iob[1])

extern __stream_link *__OpenStreams;

typedef char *va_list[1];

int __flush(FILE *fp);
int __flushall(int mask);
int flushall(void);
int __qwrite(int handle, const void *buffer, unsigned len);
void __set_errno(int err);
long lseek(int handle, long offset, int origin);
int __fprtf(FILE *fp, const char *format, va_list arg);
int __prtf(void *dest, const char *format, va_list args, void (*out_putc)());

#define O_RDONLY 0x0000
#define O_WRONLY 0x0001
#define O_RDWR 0x0002
#define O_APPEND 0x0010
#define O_CREAT 0x0020
#define O_TRUNC 0x0040
#define O_NOINHERIT 0x0080
#define O_TEXT 0x0100
#define O_BINARY 0x0200
#define O_EXCL 0x0400

#define SH_COMPAT 0x00

int open(const char *name, int mode, ...);
int sopen(const char *name, int mode, int shflag, ...);

#endif

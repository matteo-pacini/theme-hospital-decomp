#include "clib.h"

#define ENOMEM 5
#define _NFILES 20
#define _DYNAMIC 0x4000

extern __stream_link *__ClosedStreams;

void *malloc(size_t size);
#pragma aux malloc "W?init$n()v"
void _nfree(void *ptr);
#pragma aux _nfree "__wcpp_2_mod_register__"
void *memset(void *s, int c, size_t n);

// FUNCTION: 0xf48ce
FILE *__allocfp(int handle)
{
    FILE *end;
    FILE *fp;
    __stream_link *link;
    __stream_link *next;
    unsigned flags;

    link = __ClosedStreams;
    if (link != NULL) {
        fp = link->stream;
        flags = (fp->_flag & (_DYNAMIC | _READ | _WRITE)) | (_READ | _WRITE);
        __ClosedStreams = link->next;
        goto got_one;
    }
    end = &__iob[_NFILES];
    for (fp = __iob; fp < end; ++fp) {
        if ((fp->_flag & (_READ | _WRITE)) == 0) {
            link = malloc(sizeof(__stream_link));
            if (link == NULL)
                goto no_mem;
            flags = _READ | _WRITE;
            goto got_one;
        }
    }
    flags = _DYNAMIC | _READ | _WRITE;
    link = malloc(sizeof(__stream_link) + sizeof(FILE));
    if (link == NULL)
        goto no_mem;
    fp = (FILE *)(link + 1);
got_one:
    memset(fp, 0, sizeof(*fp));
    fp->_flag = flags;
    next = __OpenStreams;
    link->stream = fp;
    __OpenStreams = link;
    link->next = next;
    return fp;
no_mem:
    __set_errno(ENOMEM);
    return NULL;
}

// FUNCTION: 0xf4974
// NONMATCHING: config/functions.txt extends __freefp_ over __purgefp
// (0xf49ad)
void __freefp(FILE *fp)
{
    __stream_link **owner;
    __stream_link *link;
    __stream_link *next;

    owner = &__OpenStreams;
    for (;;) {
        link = *owner;
        if (link == NULL)
            return;
        if (link->stream == fp)
            break;
        owner = &link->next;
    }
    fp->_flag |= _READ | _WRITE;
    *owner = link->next;
    next = __ClosedStreams;
    __ClosedStreams = link;
    link->next = next;
}

// FUNCTION: 0x000f49ad
void __purgefp(void)
{
    __stream_link *next;

    while (__ClosedStreams != NULL) {
        next = __ClosedStreams->next;
        _nfree(__ClosedStreams);
        __ClosedStreams = next;
    }
}

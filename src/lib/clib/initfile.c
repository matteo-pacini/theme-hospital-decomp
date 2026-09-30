#include "clib.h"

#define _IOFBF 0x0100
#define _IOLBF 0x0200
#define _IONBF 0x0400

extern __stream_link *__ClosedStreams;

void *malloc(size_t size);
/* malloc_ is an alias of _nmalloc_, the name config/functions.txt uses. */
#pragma aux malloc "_nmalloc_"
void __fatal_runtime_error(char *msg, unsigned retcode);

// FUNCTION: 0xec3d8
void __InitFiles(void)
{
    FILE *fp;
    __stream_link *link;
    __stream_link *next;

    __iob[2]._flag &= ~(_IONBF | _IOLBF | _IOFBF);
    __iob[2]._flag |= _IONBF;
    for (fp = __iob; fp->_flag != 0; ++fp) {
        link = malloc(sizeof(__stream_link));
        if (link == NULL) {
            link = malloc(sizeof(__stream_link));
            if (link == NULL)
                __fatal_runtime_error("Not enough memory to allocate file structures\r\n", 1);
        }
        next = __OpenStreams;
        link->stream = fp;
        __OpenStreams = link;
        link->next = next;
    }
    __ClosedStreams = NULL;
}

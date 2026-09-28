#include "clib.h"

#define _IOFBF 0x0100
#define _IOLBF 0x0200
#define _IONBF 0x0400

extern __stream_link *__ClosedStreams;
extern char D_001271e4[];

void *malloc(size_t size);
/* config/functions.txt has malloc under a wrong demo name. */
#pragma aux malloc "W?init$n()v"
void __fatal_runtime_error(char *msg, unsigned retcode);

/* __InitFiles; config/functions.txt gives its name to the clock code before it. */
// FUNCTION: 0xec3d8
void fn_000ec3d8(void)
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
                __fatal_runtime_error(D_001271e4, 1);
        }
        next = __OpenStreams;
        link->stream = fp;
        __OpenStreams = link;
        link->next = next;
    }
    __ClosedStreams = NULL;
}

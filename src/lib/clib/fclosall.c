#include "clib.h"

#define _TMPFIL  0x0800
#define _DYNAMIC 0x4000
#define NUM_STD_STREAMS 5

int __shutdown_stream(FILE *fp, int close_handle);
void __purgefp(void);
int docloseall(int exclude);

/* config/functions.txt has no name for docloseall. */
#pragma aux docloseall "fn_000ec460_"

void __full_io_exit(void);
/* config/functions.txt has no name for __full_io_exit. */
#pragma aux __full_io_exit "fn_000ec44f_"

// FUNCTION: 0xec44f
void __full_io_exit(void)
{
    docloseall(0);
    __purgefp();
}

int fcloseall(void);
/* config/functions.txt has no name for fcloseall. */
#pragma aux fcloseall "fn_000ec45b_"

// FUNCTION: 0xec45b
int fcloseall(void)
{
    return docloseall(NUM_STD_STREAMS);
}

// FUNCTION: 0xec460
int docloseall(int exclude)
{
    FILE *fp;
    int number_of_files_closed;
    __stream_link *link;
    __stream_link *next;
    int close_handle;
    FILE *first;

    number_of_files_closed = 0;
    first = &__iob[exclude];
    for (link = __OpenStreams; link != NULL; link = next) {
        next = link->next;
        fp = link->stream;
        close_handle = 1;
        if (!(fp->_flag & _DYNAMIC) && !(fp->_flag & _TMPFIL)) {
            if (fp < first)
                continue;
            if (fp < &__iob[NUM_STD_STREAMS])
                close_handle = 0;
        }
        __shutdown_stream(fp, close_handle);
        ++number_of_files_closed;
    }
    return number_of_files_closed;
}

#include "clib.h"

// FUNCTION: 0xbc07c
int flushall(void)
{
    return __flushall(~0);
}

// FUNCTION: 0xbc081
int __flushall(int mask)
{
    __stream_link *link;
    FILE *fp;
    int number_of_open_files;

    number_of_open_files = 0;
    for (link = __OpenStreams; link != NULL; link = link->next) {
        fp = link->stream;
        if (fp->_flag & mask) {
            ++number_of_open_files;
            if (fp->_flag & _DIRTY) {
                __flush(fp);
            }
        }
    }
    return number_of_open_files;
}

#include "clib.h"

// flushall: flush all open streams; returns the number of open streams.
// FUNCTION: 0xbc07c
int flushall(void)
{
    return __flushall(~0);
}

// __flushall: flush every open stream whose flags intersect mask; returns how many matched.
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

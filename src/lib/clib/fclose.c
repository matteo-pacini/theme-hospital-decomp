#include "clib.h"

#define _BIGBUF 0x0008
#define _TMPFIL 0x0800

void _nfree(void *ptr);
#pragma aux _nfree "__wcpp_2_mod_register__"
unsigned getpid(void);
/* config/functions.txt names getpid default_handler_. */
#pragma aux getpid "default_handler_"
int unlink(const char *name);
/* config/functions.txt has no name for unlink. */
#pragma aux unlink "fn_000d12d8_"
long ftell(FILE *fp);
int __close(int handle);
void __freefp(FILE *fp);
int __doclose(FILE *fp, int close_handle);
int __shutdown_stream(FILE *fp, int close_handle);

int fclose(FILE *fp);
/* config/functions.txt has no name for fclose. */
#pragma aux fclose "fn_000f49cb_"

// FUNCTION: 0xf49cb
int fclose(FILE *fp)
{
    __stream_link *link;

    link = __OpenStreams;
    for (;;) {
        if (link == NULL)
            return -1;
        if (link->stream == fp)
            break;
        link = link->next;
    }
    return __shutdown_stream(fp, 1);
}

// FUNCTION: 0xf49f8
int __shutdown_stream(FILE *fp, int close_handle)
{
    int ret;

    ret = __doclose(fp, close_handle);
    __freefp(fp);
    return ret;
}

// FUNCTION: 0xf4a0d
char __hex(int num)
{
    num += '0';
    if (num > '9')
        num += 'a' - '0' - 10;
    return num;
}

// FUNCTION: 0xf4a19
void __MkTmpFile(char *buf, int num)
{
    unsigned pid;
    int i;

    pid = getpid();
    buf[0] = 't';
    for (i = 4; i > 0; --i) {
        buf[i] = __hex(pid & 0x000f);
        pid >>= 4;
    }
    buf[5] = '_';
    buf[6] = __hex((num >> 4) & 0x000f);
    buf[7] = __hex(num & 0x000f);
    buf[8] = '.';
    buf[9] = 't';
    buf[10] = 'm';
    buf[11] = 'p';
    buf[12] = '\0';
}

// FUNCTION: 0xf4a7e
int __doclose(FILE *fp, int close_handle)
{
    int ret;
    long pos;
    char name[13];

    if (fp->_flag == 0)
        return -1;
    ret = 0;
    if (fp->_flag & _DIRTY)
        ret = __flush(fp);
    pos = ftell(fp);
    if (pos != -1)
        lseek(fp->_handle, pos, SEEK_SET);
    if (close_handle)
        ret |= __close(fp->_handle);
    if (fp->_flag & _BIGBUF) {
        _nfree(fp->_base);
        fp->_base = NULL;
    }
    if (fp->_flag & _TMPFIL) {
        __MkTmpFile(name, fp->_tmpfchar);
        unlink(name);
    }
    return ret;
}

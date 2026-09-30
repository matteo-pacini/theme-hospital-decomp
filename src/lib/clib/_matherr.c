#include "clib.h"

struct exception {
    int type;
    char *name;
    double arg1;
    double arg2;
    double retval;
};

#define DOMAIN 1

int __matherr(struct exception *excp);
#pragma aux __matherr "__matherr"

int (*_RWD_matherr)(struct exception *) = __matherr;

/* The library's compiler put this read-only table in CONST2; 10.6 puts
   data with relocations in _DATA unless told otherwise. */
#pragma data_seg("CONST2", "DATA")
static const char * const Msgs[] = {
    0,
    "Domain error",
    "Argument singularity",
    "Overflow range error",
    "Underflow range error",
    "Total loss of significance",
    "Partial loss of significance"
};
#pragma data_seg()

FILE *__get_std_stream(unsigned handle);
int fputs(const char *s, FILE *fp);
int fputc(int c, FILE *fp);
void __set_EDOM(void);
void __set_ERANGE(void);

// FUNCTION: 0xfdf13
void _set_matherr(int (*rtn)(struct exception *))
{
    _RWD_matherr = rtn;
}

// FUNCTION: 0xfdf19
void __rterrmsg(int errcode, char *funcname)
{
    FILE *fp;

    fp = __get_std_stream(2);
    fputs(Msgs[errcode], fp);
    fputs(" in ", fp);
    fputs(funcname, fp);
    fputc('\n', fp);
}

// FUNCTION: 0xfdf61
double _matherr(struct exception *excp)
{
    if (_RWD_matherr(excp) == 0) {
        __rterrmsg(excp->type, excp->name);
        if (excp->type == DOMAIN)
            __set_EDOM();
        else
            __set_ERANGE();
    }
    return excp->retval;
}

int matherr(struct exception *excp);

/* The default _RWD_matherr: the user's matherr, or the library's stub. */
// FUNCTION: 0xfdf90
int __matherr(struct exception *excp)
{
    return matherr(excp);
}

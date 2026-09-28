#include "clib.h"

struct exception {
    int type;
    char *name;
    double arg1;
    double arg2;
    double retval;
};

#define DOMAIN 1

extern int (*_RWD_matherr)(struct exception *);
extern char *Msgs[];
extern const char D_00128999[];

FILE *__get_std_stream(unsigned handle);
int fputs(const char *s, FILE *fp);
int fputc(int c, FILE *fp);
void __set_EDOM(void);
void __set_EINVAL(void);

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
    fputs(D_00128999, fp);
    fputs(funcname, fp);
    fputc('\n', fp);
}

/* config/functions.txt names __set_ERANGE after __set_EINVAL. */
// NONMATCHING: the target has a stray jmp into fputs after the epilogue
// FUNCTION: 0xfdf61
double _matherr(struct exception *excp)
{
    if (_RWD_matherr(excp) == 0) {
        __rterrmsg(excp->type, excp->name);
        if (excp->type == DOMAIN)
            __set_EDOM();
        else
            __set_EINVAL();
    }
    return excp->retval;
}

#include "clib.h"

#define SIGABRT  1
#define SIGFPE   2
#define SIGILL   3
#define SIGINT   4
#define SIGSEGV  5
#define SIGTERM  6
#define SIGBREAK 7
#define SIGUSR1  8
#define SIGUSR2  9
#define SIGUSR3  10
#define SIGIDIVZ 11
#define SIGIOVFL 12

#define FPE_EXPLICITGEN 0x8c

typedef void (*__sig_func)(int);

#define SIG_IGN ((__sig_func)1)
#define SIG_DFL ((__sig_func)2)
#define SIG_ERR ((__sig_func)3)

extern __sig_func D_00152a94[];

#define EINVAL 9

typedef void (*__fpe_func)(int sig, int fpe_type);

extern void (*D_00153acc)(void);

void __sigfpe_handler(int fpe_type);
void __terminate(void);
void __restore_int23(void);
void __restore_int_ctrl_break(void);
void __grab_int23(void);
void __grab_int_ctrl_break(void);
void __grab_fpe(void);
void __set_errno(int err);

int raise(int sig);
/* config/functions.txt has no name for raise. */
#pragma aux raise "fn_000fbbef_"

// FUNCTION: 0xfbafb
void __sigabort(void)
{
    raise(SIGABRT);
}

// FUNCTION: 0xfbb05
void __sigfpe_handler(int fpe_type)
{
    __sig_func func;

    func = D_00152a94[SIGFPE];
    if (func != SIG_IGN && func != SIG_DFL && func != SIG_ERR) {
        D_00152a94[SIGFPE] = SIG_DFL;
        (*(__fpe_func)func)(SIGFPE, fpe_type);
    }
}

// FUNCTION: 0xfbb3a
__sig_func signal(int sig, __sig_func func)
{
    __sig_func prev_func;

    if (sig < SIGABRT || sig > SIGIOVFL) {
        __set_errno(EINVAL);
        return SIG_ERR;
    }
    D_00153acc = __sigabort;
    if (sig == SIGINT) {
        if (func == SIG_DFL) {
            __restore_int23();
        } else if (func != SIG_ERR) {
            __grab_int23();
        }
    } else if (sig == SIGBREAK) {
        if (func == SIG_DFL) {
            __restore_int_ctrl_break();
        } else if (func != SIG_ERR) {
            __grab_int_ctrl_break();
        }
    } else if (sig == SIGFPE) {
        if (func != SIG_DFL && func != SIG_ERR)
            __grab_fpe();
    }
    prev_func = D_00152a94[sig];
    D_00152a94[sig] = func;
    return prev_func;
}

// FUNCTION: 0xfbbef
int raise(int sig)
{
    __sig_func func;

    func = D_00152a94[sig];
    switch (sig) {
    case SIGFPE:
        __sigfpe_handler(FPE_EXPLICITGEN);
        break;
    case SIGABRT:
        if (func == SIG_DFL)
            __terminate();
    case SIGINT:
        if (func != SIG_IGN && func != SIG_DFL && func != SIG_ERR) {
            D_00152a94[sig] = SIG_DFL;
            __restore_int23();
            (*func)(sig);
        }
        break;
    case SIGBREAK:
        if (func != SIG_IGN && func != SIG_DFL && func != SIG_ERR) {
            D_00152a94[sig] = SIG_DFL;
            __restore_int_ctrl_break();
            (*func)(sig);
        }
        break;
    case SIGILL:
    case SIGSEGV:
    case SIGTERM:
    case SIGUSR1:
    case SIGUSR2:
    case SIGUSR3:
    case SIGIDIVZ:
    case SIGIOVFL:
        if (func != SIG_IGN && func != SIG_DFL && func != SIG_ERR) {
            D_00152a94[sig] = SIG_DFL;
            (*func)(sig);
        }
        break;
    default:
        return -1;
    }
    return 0;
}

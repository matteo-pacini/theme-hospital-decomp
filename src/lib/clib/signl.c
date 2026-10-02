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

static __sig_func SignalTable[] = {
    SIG_IGN,
    SIG_DFL,    /* SIGABRT */
    SIG_IGN,    /* SIGFPE */
    SIG_IGN,    /* SIGILL */
    SIG_IGN,    /* SIGINT */
    SIG_IGN,    /* SIGSEGV */
    SIG_IGN,    /* SIGTERM */
    SIG_IGN,    /* SIGBREAK */
    SIG_IGN,    /* SIGUSR1 */
    SIG_IGN,    /* SIGUSR2 */
    SIG_IGN,    /* SIGUSR3 */
    SIG_IGN,    /* SIGIDIVZ */
    SIG_IGN     /* SIGIOVFL */
};

#define EINVAL 9

typedef void (*__fpe_func)(int sig, int fpe_type);

extern void (*__abort)(void);

void __sigfpe_handler(int fpe_type);
void __terminate(void);
void __restore_int23(void);
void __restore_int_ctrl_break(void);
void __grab_int23(void);
void __grab_int_ctrl_break(void);
void __grab_fpe(void);
void __set_errno(int err);

int raise(int sig);

// __sigabort: abort handler once signal() is used; raise(SIGABRT).
// FUNCTION: 0xfbafb
void __sigabort(void)
{
    raise(SIGABRT);
}

// __sigfpe_handler: call the user's SIGFPE handler (reset to SIG_DFL first) with fpe_type, if one is installed.
// FUNCTION: 0xfbb05
void __sigfpe_handler(int fpe_type)
{
    __sig_func func;

    func = SignalTable[SIGFPE];
    if (func != SIG_IGN && func != SIG_DFL && func != SIG_ERR) {
        SignalTable[SIGFPE] = SIG_DFL;
        (*(__fpe_func)func)(SIGFPE, fpe_type);
    }
}

// signal: set the handler for sig, hooking the Ctrl-C/Ctrl-Break/FPE interrupts as needed; returns the previous handler or SIG_ERR.
// FUNCTION: 0xfbb3a
__sig_func signal(int sig, __sig_func func)
{
    __sig_func prev_func;

    if (sig < SIGABRT || sig > SIGIOVFL) {
        __set_errno(EINVAL);
        return SIG_ERR;
    }
    __abort = __sigabort;
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
    prev_func = SignalTable[sig];
    SignalTable[sig] = func;
    return prev_func;
}

// raise: deliver sig to its handler (reset to SIG_DFL first); SIGABRT with SIG_DFL terminates; returns 0, or -1 for an unknown sig.
// FUNCTION: 0xfbbef
int raise(int sig)
{
    __sig_func func;

    func = SignalTable[sig];
    switch (sig) {
    case SIGFPE:
        __sigfpe_handler(FPE_EXPLICITGEN);
        break;
    case SIGABRT:
        if (func == SIG_DFL)
            __terminate();
    case SIGINT:
        if (func != SIG_IGN && func != SIG_DFL && func != SIG_ERR) {
            SignalTable[sig] = SIG_DFL;
            __restore_int23();
            (*func)(sig);
        }
        break;
    case SIGBREAK:
        if (func != SIG_IGN && func != SIG_DFL && func != SIG_ERR) {
            SignalTable[sig] = SIG_DFL;
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
            SignalTable[sig] = SIG_DFL;
            (*func)(sig);
        }
        break;
    default:
        return -1;
    }
    return 0;
}

void __null_int23_exit(void);

void (*__atexit)(void) = __null_int23_exit;
void (*__int23_exit)(void) = __null_int23_exit;
void (*__FPE_handler_exit)(void) = __null_int23_exit;

void __exit(int status);
#pragma aux __exit aborts;

// __null_int23_exit: empty default for the exit hooks.
// FUNCTION: 0xc2110
void __null_int23_exit(void)
{
}

// exit: run the atexit and Ctrl-C hooks, then _exit(status).
// FUNCTION: 0xc2111
void exit(int status)
{
    (*__atexit)();
    (*__int23_exit)();
    _exit(status);
}

// _exit: run the Ctrl-C and FPE handler cleanup hooks, then terminate with status (no atexit processing).
// FUNCTION: 0xc2129
void _exit(int status)
{
    (*__int23_exit)();
    (*__FPE_handler_exit)();
    __exit(status);
}

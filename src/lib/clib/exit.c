extern void (*__atexit)(void);
extern void (*__int23_exit)(void);
extern void (*__FPE_handler_exit)(void);

void __exit(int status);

// FUNCTION: 0xc2110
void fn_000c2110(void)
{
}

// FUNCTION: 0xc2111
void exit(int status)
{
    (*__atexit)();
    (*__int23_exit)();
    _exit(status);
}

// FUNCTION: 0xc2129
// NONMATCHING: __exit is a label inside _cstart_ in config/functions.txt
// (_cstart_+0x28f)
void _exit(int status)
{
    (*__int23_exit)();
    (*__FPE_handler_exit)();
    __exit(status);
}

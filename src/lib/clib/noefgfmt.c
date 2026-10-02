
void __fatal_runtime_error(const char *msg, unsigned retcode);

void _no_support_loaded(void);

// _no_support_loaded: fatal error "Floating-point support not loaded".
// FUNCTION: 0xec5ea
void _no_support_loaded(void)
{
    __fatal_runtime_error("Floating-point support not loaded\r\n", 1);
}

typedef char __far *FAR_STRING;

/* The floating-point printf and scanf conversions; __setEFGfmt installs
   them when a module uses floating point. */
FAR_STRING (*__EFG_printf)() = (FAR_STRING (*)())_no_support_loaded;
void (*__EFG_scanf)() = _no_support_loaded;

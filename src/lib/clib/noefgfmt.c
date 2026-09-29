extern const char D_00127214[];

void __fatal_runtime_error(const char *msg, unsigned retcode);

void _no_support_loaded(void);

// FUNCTION: 0xec5ea
void _no_support_loaded(void)
{
    __fatal_runtime_error(D_00127214, 1);
}

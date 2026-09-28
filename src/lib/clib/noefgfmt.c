extern const char D_00127214[];

void __fatal_runtime_error(const char *msg, unsigned retcode);

void _no_support_loaded(void);
/* config/functions.txt gives _no_support_loaded_ to itoa, before it. */
#pragma aux _no_support_loaded "fn_000ec5ea_"

// FUNCTION: 0xec5ea
// NONMATCHING: config/functions.txt extends this function over ultoa's digit
// table (0xec5fc)
void _no_support_loaded(void)
{
    __fatal_runtime_error(D_00127214, 1);
}

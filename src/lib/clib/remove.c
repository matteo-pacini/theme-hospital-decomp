typedef int tiny_ret_t;

int __set_errno_dos(unsigned err);

tiny_ret_t TinyDelete(const char *name);
#pragma aux TinyDelete = \
    "mov ah,41h" \
    "int 21h" \
    "rcl eax,1" \
    "ror eax,1" \
    parm [edx] \
    value [eax];

int remove(const char *filename);
/* config/functions.txt has no name for remove. */
#pragma aux remove "fn_000ed2dd_"

// FUNCTION: 0xed2dd
int remove(const char *filename)
{
    tiny_ret_t rc;

    rc = TinyDelete(filename);
    if (rc < 0)
        return __set_errno_dos((unsigned short)rc);
    return 0;
}

#include "clib.h"

typedef struct SCNF_SPECS {
    int (*cget_rtn)(struct SCNF_SPECS *specs);
    void (*uncget_rtn)(int c, struct SCNF_SPECS *specs);
    char *ptr;
    int width;
    unsigned assign : 1;
    unsigned eoinp : 1;
} SCNF_SPECS;

int __scnf(SCNF_SPECS *specs, const char *format, va_list args);

int cget_string(SCNF_SPECS *specs);
/* config/functions.txt has no name for cget_string. */
#pragma aux cget_string "cget_string_"

// FUNCTION: 0xefe5a
int cget_string(SCNF_SPECS *specs)
{
    int c;

    if ((c = *(unsigned char *)specs->ptr) != '\0') {
        ++specs->ptr;
    } else {
        c = EOF;
        specs->eoinp = 1;
    }
    return c;
}

void uncget_string(int c, SCNF_SPECS *specs);
/* config/functions.txt has no name for uncget_string. */
#pragma aux uncget_string "uncget_string_"

// FUNCTION: 0xefe83
void uncget_string(int c, SCNF_SPECS *specs)
{
    --specs->ptr;
}

// FUNCTION: 0xefe87
int vsscanf(const char *s, const char *format, va_list args)
{
    SCNF_SPECS specs;

    specs.ptr = (char *)s;
    specs.cget_rtn = cget_string;
    specs.uncget_rtn = uncget_string;
    return __scnf(&specs, format, args);
}

// FUNCTION: 0xefeae
int sscanf(const char *s, const char *format, ...)
{
    va_list args;

    args[0] = (char *)&format + sizeof(format);
    return vsscanf(s, format, args);
}

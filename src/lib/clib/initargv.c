#include "clib.h"

// FUNCTION: 0xd070c
int _SplitParms(char *p, char **argv, char **endptr)
{
    int argc;
    char *start;
    char *new;

    argc = 0;
    for (;;) {
        while ((int)*p == ' ' || (int)*p == '\t') {
            ++p;
        }
        if (*p == '\0')
            break;
        if ((int)*p == '"') {
            p++;
            new = start = p;
            for (;;) {
                if ((int)*p == '"')
                    break;
                if (*p == '\0')
                    break;
                if ((int)*p == '\\') {
                    if ((int)p[1] == '"' || (int)p[1] == '\\') {
                        ++p;
                    }
                }
                if (argv) {
                    *(new++) = *(p++);
                } else {
                    ++p;
                }
            }
        } else {
            new = start = p;
            for (;;) {
                if (*p == '\0')
                    break;
                if ((int)*p == ' ')
                    break;
                if ((int)*p == '\t')
                    break;
                if ((int)*p == '\\') {
                    if ((int)p[1] == '"') {
                        ++p;
                    }
                }
                if (argv) {
                    *(new++) = *(p++);
                } else {
                    ++p;
                }
            }
        }
        if (argv) {
            argv[argc] = start;
            ++argc;
            if (*p == '\0') {
                *new = '\0';
                break;
            }
            *new = '\0';
            ++p;
        } else {
            ++argc;
            if (*p == '\0') {
                break;
            }
            ++p;
        }
    }
    *endptr = p;
    return argc;
}

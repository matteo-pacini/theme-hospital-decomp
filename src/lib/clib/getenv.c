#include "clib.h"

extern char **environ;

int strnicmp(const char *s, const char *t, size_t n);

// FUNCTION: 0xce2f0
// NONMATCHING: vsprintf's mem_putc (0xce351) is counted as part of getenv_
char *getenv(const char *name)
{
    char **envp;
    char *p;
    int len;

    envp = environ;
    if (envp != NULL && name != NULL) {
        len = strlen(name);
        for (; p = *envp; ++envp) {
            if (strnicmp(p, name, len) == 0) {
                if ((int)p[len] == '=')
                    return &p[len + 1];
            }
        }
    }
    return NULL;
}

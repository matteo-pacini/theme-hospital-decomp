#include "clib.h"

extern char **environ;

int strnicmp(const char *s, const char *t, size_t n);

// getenv: value of environment variable name (case-insensitive), or NULL.
// FUNCTION: 0xce2f0
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

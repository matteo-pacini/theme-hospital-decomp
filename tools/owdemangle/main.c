#include <stdio.h>
#include <string.h>
#include "demangle.h"
int main(void)
{
    char line[4096], out[4096];
    while (fgets(line, sizeof line, stdin)) {
        line[strcspn(line, "\r\n")] = 0;
        __demangle_l(line, 0, out, sizeof out);
        printf("%s\n", out);
    }
    return 0;
}

#include "clib.h"

extern char *nexttok;
extern const unsigned char _Bits[8];

void __setbits(unsigned char *vector, const char *charset);

// strtok: next token of str (NULL continues the previous string) delimited by chars in charset; NULs the delimiter; NULL when none left.
// FUNCTION: 0xce37f
char *strtok(char *str, const char *charset)
{
    unsigned tc;
    unsigned char vector[32];
    char *p1;

    if (str == NULL) {
        str = nexttok;
        if (str == NULL)
            return NULL;
    }
    __setbits(vector, charset);
    for (; tc = (unsigned char)*str; ++str) {
        if ((vector[tc >> 3] & _Bits[tc & 0x07]) == 0)
            break;
    }
    if (tc == '\0')
        return NULL;
    p1 = str;
    for (; tc = (unsigned char)*p1; ++p1) {
        if ((vector[tc >> 3] & _Bits[tc & 0x07]) != 0) {
            *p1 = '\0';
            p1++;
            nexttok = p1;
            return str;
        }
    }
    nexttok = NULL;
    return str;
}

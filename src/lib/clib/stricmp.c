// FUNCTION: 0xce439
int stricmp(const char *s, const char *t)
{
    char c1;
    char c2;

    for (;;) {
        c1 = *s;
        c2 = *t;
        if ((int)c1 >= 'A' && (int)c1 <= 'Z')
            c1 += 'a' - 'A';
        if ((int)c2 >= 'A' && (int)c2 <= 'Z')
            c2 += 'a' - 'A';
        if (c1 != c2)
            break;
        if (c1 == '\0')
            break;
        ++s;
        ++t;
    }
    return (unsigned char)c1 - (unsigned char)c2;
}

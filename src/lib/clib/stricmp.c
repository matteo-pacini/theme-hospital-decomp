// stricmp: compare s1 and s2 ignoring ASCII case; <0, 0 or >0.
// FUNCTION: 0xce439
int stricmp(const char *s1, const char *s2)
{
    char c1;
    char c2;

    for (;;) {
        c1 = *s1;
        c2 = *s2;
        if ((int)c1 >= 'A' && (int)c1 <= 'Z')
            c1 += 'a' - 'A';
        if ((int)c2 >= 'A' && (int)c2 <= 'Z')
            c2 += 'a' - 'A';
        if (c1 != c2)
            break;
        if (c1 == '\0')
            break;
        ++s1;
        ++s2;
    }
    return (unsigned char)c1 - (unsigned char)c2;
}

// FUNCTION: 0xcaa9a
int tolower(int c)
{
    if (c >= 'A' && c <= 'Z') {
        c = c - 'A' + 'a';
    }
    return c;
}

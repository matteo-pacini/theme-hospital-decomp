// toupper: uppercase of ASCII letter c; other values unchanged.
// FUNCTION: 0xcaaa8
int toupper(int c)
{
    if (c >= 'a' && c <= 'z') {
        c = c - 'a' + 'A';
    }
    return c;
}

int unlink(const char *filename);

// FUNCTION: 0xd12d8
int remove(const char *filename)
{
    return unlink(filename);
}

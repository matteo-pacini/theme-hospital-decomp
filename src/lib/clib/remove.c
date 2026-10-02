int unlink(const char *filename);

// remove: delete file filename; returns 0, or -1 with errno set.
// FUNCTION: 0xd12d8
int remove(const char *filename)
{
    return unlink(filename);
}

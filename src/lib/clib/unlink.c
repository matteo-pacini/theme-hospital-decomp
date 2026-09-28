int remove(const char *filename);
/* config/functions.txt has no name for remove. */
#pragma aux remove "fn_000ed2dd_"

int unlink(const char *filename);
/* config/functions.txt has no name for unlink. */
#pragma aux unlink "fn_000d12d8_"

// FUNCTION: 0xd12d8
int unlink(const char *filename)
{
    return remove(filename);
}

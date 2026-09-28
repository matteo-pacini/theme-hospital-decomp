extern "C" {

extern const unsigned char D_00152a88[8];

}

char testFlag(unsigned char *flags, unsigned index);
#pragma aux testFlag "fn_000fb810_"

// FUNCTION: 0xfb810
char testFlag(unsigned char *flags, unsigned index)
{
    unsigned byte;

    byte = index / 8;
    index -= byte * 8;
    return D_00152a88[index] & flags[byte];
}

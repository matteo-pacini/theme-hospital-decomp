static unsigned char mask[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

char testFlag(unsigned char *flags, unsigned index);
#pragma aux testFlag "__wcpp_2_bit_test__"

// FUNCTION: 0xfb810
char testFlag(unsigned char *flags, unsigned index)
{
    unsigned byte;

    byte = index / 8;
    index -= byte * 8;
    return mask[index] & flags[byte];
}

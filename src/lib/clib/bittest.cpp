extern "C" {

extern const unsigned char D_00152a88[8];

}

char testFlag(unsigned char *flags, unsigned index);
/* config/functions.txt names the flag test after an unrelated demo function. */
#pragma aux testFlag "W?arr_state_table$n(pn$ro_dtreg$$)pn$__7a4u3RO_STATE$$"

// FUNCTION: 0xfb810
char testFlag(unsigned char *flags, unsigned index)
{
    unsigned byte;

    byte = index / 8;
    index -= byte * 8;
    return D_00152a88[index] & flags[byte];
}

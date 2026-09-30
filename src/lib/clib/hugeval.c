/* HUGE_VAL: the double positive infinity (0x7ff0000000000000), written as
   its bytes since 10.6 folds an overflowing constant such as 1e309 to zero.
   In CONST2, where a double constant goes; 10.6 puts a union in _DATA. */

#pragma data_seg("CONST2", "DATA")
const union {
    unsigned char bytes[8];
    double value;
} _HugeValue = { 0, 0, 0, 0, 0, 0, 0xf0, 0x7f };
#pragma data_seg()

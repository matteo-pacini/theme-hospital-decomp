/* Powers of ten by which _Scale10V scales a value, largest first. */

typedef struct {
    short exp;
    double value;
} BIGPOW10;

/* In CONST2, where the library's compiler put it; 10.6 puts a const array
   of structures in _DATA. */
#pragma data_seg("CONST2", "DATA")
const BIGPOW10 _BigPow10Table[] = {
    { 216, 1e216 },
    { 108, 1e108 },
    { 54, 1e54 },
    { 27, 1e27 },
    { 14, 1e14 },
    { 8, 1e8 },
    { 4, 1e4 },
    { 1, 1e1 }
};
#pragma data_seg()

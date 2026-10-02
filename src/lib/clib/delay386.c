#include "rtinit.h"

void __delay(unsigned ms);
void __delay_init(void);

// delay: suspend execution for ms milliseconds.
// FUNCTION: 0xce37a
void delay(unsigned ms)
{
    __delay(ms);
}

AXI(__delay_init, 0x20)

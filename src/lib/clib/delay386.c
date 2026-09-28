void __delay(unsigned ms);
/* config/functions.txt has no name for __delay. */
#pragma aux __delay "fn_000eb733_"

// FUNCTION: 0xce37a
void delay(unsigned ms)
{
    __delay(ms);
}

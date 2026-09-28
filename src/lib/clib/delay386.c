void __delay(unsigned ms);

// FUNCTION: 0xce37a
// NONMATCHING: the jump target lies inside strnicmp_ in config/functions.txt
void delay(unsigned ms)
{
    __delay(ms);
}

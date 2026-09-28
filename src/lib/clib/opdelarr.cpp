extern "C" {

/* operator delete[]; config/functions.txt names it after its address. */
// FUNCTION: 0xeeeb4
void fn_000eeeb4(void *p)
{
    ::operator delete(p);
}

}

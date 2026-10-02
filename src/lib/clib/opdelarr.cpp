// operator delete[]: free p through operator delete.
// FUNCTION: 0xeeeb4
void operator delete[](void *p)
{
    ::operator delete(p);
}

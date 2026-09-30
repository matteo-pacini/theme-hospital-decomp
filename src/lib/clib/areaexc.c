/* The memory the C++ runtime allocates thrown exceptions from (see
   allocexc.cpp): its first word is its size. */

unsigned __EXC_AREA[0x1000 / sizeof(unsigned)] = { sizeof(__EXC_AREA) };

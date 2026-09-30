#include "bflib.h"

/* Bytes 4..7 are the reference number LbExeReferenceNumber returns (0 in
   this build), between two marker dwords. */
static const u8 exe_key[] = { 0x89, 0x0c, 0xd5, 0x11, 0, 0, 0, 0, 0xe9, 0x2a, 0x80, 0xc5 };

extern "C" {

// FUNCTION: 0x000d12e0
s32 LbExeReferenceNumber(void)
{
    return *(s32 *)&exe_key[4];
}

}

#include "clib.h"
#include "rtinit.h"

/* The list of open streams, and the routines that set it up at startup
   and flush and close every stream at exit. */

__stream_link *__OpenStreams;

void __InitFiles(void);
void __full_io_exit(void);

AXI(__InitFiles, 0x20)
AYI(__full_io_exit, 0x20)

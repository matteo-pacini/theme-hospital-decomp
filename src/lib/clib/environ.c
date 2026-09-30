#include "clib.h"
#include "rtinit.h"

/* The environment, which __setenvp builds at startup. */

char **environ = NULL;
char *__env_mask = NULL;

void __setenvp(void);

AXI(__setenvp, 0x20)

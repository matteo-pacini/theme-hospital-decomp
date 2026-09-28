#include "net.h"

extern "C" {
SerialSP *fn_000ed3c4(SerialSP *self, void *init_data);
ModemSP *fn_000ee6b0(ModemSP *self, void *init_data);
#pragma aux fn_000ee6b0 "W?Release$:ModemSP$n()$TbError$$"
extern void *ModemSP_vftable[];
#pragma aux ModemSP_vftable "W?$Wvf0do2sm:ModemSP$$nx[]pn()v"
}

// FUNCTION: 0xee6b0
// NONMATCHING: ModemSP::ModemSP(void *) under a misnamed symbol (the range also holds the real Release,
// Init and other ModemSP methods); no exception-state bookkeeping
ModemSP *fn_000ee6b0(ModemSP *self, void *init_data)
{
    self = (ModemSP *)fn_000ed3c4(self, init_data);
    *(void **)((u8 *)self + 0xafc) = ModemSP_vftable;
    return self;
}

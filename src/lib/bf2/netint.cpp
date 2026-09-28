#include "net.h"

extern "C" {
SerialSP *fn_000ed3c4(SerialSP *self, void *init_data);
void *ServiceProvider_ct(void *self);
#pragma aux ServiceProvider_ct "W?$ct:ServiceProvider$n()_"
void *__wcpp_2_ctor_array_(void *array, unsigned count, void *sig);
extern u8 D_00128d14[];
extern u8 D_00128d28[];
}

// FUNCTION: 0xed3c4
// NONMATCHING: SerialSP::SerialSP(void *) under a misnamed symbol; no exception-state bookkeeping
SerialSP *fn_000ed3c4(SerialSP *self, void *init_data)
{
    self = (SerialSP *)ServiceProvider_ct(self);
    self = (SerialSP *)((u8 *)__wcpp_2_ctor_array_((u8 *)self + 0xb4a, 10, D_00128d14) - 0xb4a);
    *(void **)((u8 *)self + 0xafc) = D_00128d28;
    memcpy(&self->serial_data, init_data, sizeof(self->serial_data));
    self->port = ((u32 *)init_data)[0];
    self->send_seq = 0;
    self->recv_seq = 0;
    self->awaiting_ack = 0;
    self->baud = ((u32 *)init_data)[1];
    return self;
}

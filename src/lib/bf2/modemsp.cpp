#include "net.h"

extern "C" {
extern char D_00127c08[];
extern char D_00127fd8[];
}

// FUNCTION: 0xee6b0
// NONMATCHING: exception-table relocation (known tool gap: CONST2 never compares equal)
ModemSP::ModemSP(__4dn7ojSerialInitData *init_data) : SerialSP(init_data)
{
}

// FUNCTION: 0xee718
TbError ModemSP::Release(void)
{
    fn_000c20c0(D_00127c08);
    if (unknown528)
        Stop();
    return SerialSP::Release();
}

// FUNCTION: 0xee748
TbError ModemSP::Init(__tibcaGUID guid, __ic76sReceiveCallbacks *callbacks, void *data)
{
    return SerialSP::Init(guid, 0, callbacks, data);
}

// FUNCTION: 0xeedcc
TbError ModemSP::SendMessage(u32 to, void *buf, TbMessageType type)
{
    return SerialSP::SendMessage(0, buf, type);
}

// FUNCTION: 0xeedd8
TbError ModemSP::Stop(void)
{
    TbError ret;

    if (unknown528) {
        ret = SerialSP::Stop();
        fn_000c20c0(D_00127fd8);
        comm->HangUpModem();
    }
    return ret;
}

// FUNCTION: 0xeee0c
u32 ModemSP::ReadMessage(u32 *from, void *buf, u32 *size)
{
    return SerialSP::ReadMessage(from, buf, size);
}

// FUNCTION: 0xeee14
u32 ModemSP::PeekMessage(u32 *from, void *buf, u32 *size)
{
    return SerialSP::PeekMessage(from, buf, size);
}

/*
 * ModemSP::~ModemSP() is compiler-generated in this module, together with
 * the class's vtable (the constructor is defined here).
 */
// FUNCTION: 0xeee20
// NONMATCHING: relocations only: the array-delete helpers at 0xeee94/0xeeeb4 are unnamed in
// config/functions.txt, and the exception table is CONST2 (known tool gap)

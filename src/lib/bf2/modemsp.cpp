#include "net.h"

extern "C" {
extern char D_00127c08[];
extern char D_00127fd8[];
extern char D_00127c29[];
extern char D_00127c50[];
extern char D_00127c82[];
extern char D_00127cb0[];
extern char D_00127cca[];
extern char D_00127ce4[];
extern char D_00127d05[];
extern char D_00127d3e[];
extern char D_00127d71[];
extern char D_00127da2[];
extern char D_00127dd3[];
extern char D_00127ded[];
extern char D_00127e0e[];
extern char D_00127e3c[];
extern char D_00127e6c[];
extern char D_00127e86[];
extern char D_00127eaa[];
extern char D_00127ed7[];
extern char D_00127ef7[];
extern char D_00127f24[];
extern char D_00127f3e[];
extern char D_00127f5f[];
extern char D_00127f7a[];
extern char D_00127f97[];
extern char D_00127fbd[];
}

struct ModemStartData {
    long timeout;
    long dial_timeout;
    char response[0x50];
    char number[0x50];
    long result;
    long (*init_func)(void);
    long (*dial_func)(void);
};

// FUNCTION: 0xee6b0
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

// FUNCTION: 0xee77c
// NONMATCHING: register choice: the target keeps the InitModem result on the stack and the
// response buffer address in ecx
TbError ModemSP::Start(char *session, char *name, u32 max_players, void *data)
{
    long response;
    long result;
    ModemStartData *init;
    TbError ret = Lb_FAIL;

    if (started) {
        if (unknown528)
            Stop();
        if (data) {
            init = (ModemStartData *)data;
            if (init->timeout)
                CH(comm)->timeout = init->timeout * 100;
            if (init->dial_timeout)
                CH(comm)->dial_timeout = init->dial_timeout * 100;
            if (init->init_func)
                comm->SetInitModemFunc(init->init_func);
        } else {
            return Lb_FAIL;
        }
        comm->SetDtr();
        comm->SetRts();
        result = comm->InitModem();
        comm->SetInitModemFunc(0);
        if (result == -7) {
            fn_000c20c0(D_00127c29);
            init->result = result;
            return Lb_FAIL;
        }
        if (result == -1) {
            fn_000c20c0(D_00127c50);
            init->result = result;
            return Lb_FAIL;
        }
        if (result != 1) {
            fn_000c20c0(D_00127c82);
            strcpy(init->response, CH(comm)->response);
            init->result = result;
            fn_000c20c0(D_00127cb0, comm);
            return Lb_FAIL;
        }
        strcpy(init->response, CH(comm)->response);
        init->result = result;
        fn_000c20c0(D_00127cca, comm);
        comm->SendModemString(CH(comm)->reset_str);
        response = comm->GetModemResponse(CH(comm)->timeout);
        fn_000c20c0(D_00127ce4, comm);
        if (response == 1 || response == 3 || response == 2) {
            ret = SerialSP::Start(session, name, max_players, 0);
            if (ret != Lb_OK) {
                if (response != 1) {
                    fn_000c20c0(D_00127d05);
                    comm->HangUpModem();
                }
                fn_000c20c0(D_00127d3e);
                comm->InitModem();
            }
        } else if (response == -1) {
            fn_000c20c0(D_00127d71);
            init->result = response;
            return Lb_FAIL;
        } else {
            fn_000c20c0(D_00127da2);
            strcpy(init->response, CH(comm)->response);
            init->result = response;
            fn_000c20c0(D_00127dd3, comm);
            return Lb_FAIL;
        }
    }
    return ret;
}

// FUNCTION: 0xeea58
// NONMATCHING: register choice for the first timeout multiply (eax/edx, here ebx)
TbError ModemSP::Start(TbNetworkSessionNameEntry *session, char *name, void *data)
{
    long result;
    ModemStartData *init;
    TbError ret = Lb_FAIL;

    if (started) {
        if (unknown528)
            Stop();
        if (data) {
            init = (ModemStartData *)data;
            if (init->timeout)
                CH(comm)->timeout = init->timeout * 100;
            if (init->dial_timeout)
                CH(comm)->dial_timeout = init->dial_timeout * 100;
            if (init->init_func)
                comm->SetInitModemFunc(init->init_func);
            if (init->dial_func)
                comm->SetDialModemFunc(init->dial_func);
            comm->SetDtr();
            comm->SetRts();
            result = comm->InitModem();
            comm->SetInitModemFunc(0);
            if (result == -7) {
                fn_000c20c0(D_00127ded);
                init->result = result;
                return Lb_FAIL;
            }
            if (result == -1) {
                fn_000c20c0(D_00127e0e);
                init->result = result;
                return Lb_FAIL;
            }
            if (result != 1) {
                strcpy(init->response, CH(comm)->response);
                init->result = result;
                fn_000c20c0(D_00127e3c, comm);
                return Lb_FAIL;
            }
            strcpy(init->response, CH(comm)->response);
            init->result = result;
            fn_000c20c0(D_00127e6c, comm);
            result = comm->DialModem(init->number);
            comm->SetDialModemFunc(0);
            if (result == -7) {
                fn_000c20c0(D_00127e86);
                init->result = result;
                fn_000c20c0(D_00127eaa);
                comm->HangUpModem();
                return Lb_FAIL;
            }
            if (result == -1) {
                fn_000c20c0(D_00127ed7);
                init->result = result;
                fn_000c20c0(D_00127ef7);
                comm->HangUpModem();
                return Lb_FAIL;
            }
            strcpy(init->response, CH(comm)->response);
            init->result = result;
            fn_000c20c0(D_00127f24, comm);
            if (result == 2) {
                ret = SerialSP::Start((TbNetworkSessionNameEntry *)0, name, 0);
                if (ret == Lb_FAIL) {
                    memset(init->response, 0, sizeof(init->response));
                    init->result = -2;
                    fn_000c20c0(D_00127f3e);
                    comm->HangUpModem();
                    return ret;
                }
            } else if (result == 4) {
                fn_000c20c0(D_00127f5f);
                return Lb_FAIL;
            } else if (result == 7) {
                fn_000c20c0(D_00127f7a);
                return Lb_FAIL;
            } else if (result == 5) {
                fn_000c20c0(D_00127f97);
                return Lb_FAIL;
            } else if (result == 6) {
                fn_000c20c0(D_00127fbd);
                return Lb_FAIL;
            }
        }
    }
    return ret;
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
// NONMATCHING: the code matches; only our symbol also covers 2 bytes of
// padding (mov eax,eax) after the code, which the target's does not

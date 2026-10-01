#include "net.h"

extern "C" {
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

// Constructs a modem provider for the port, baud and modem settings in
// init_data.
// FUNCTION: 0xee6b0
ModemSP::ModemSP(__4dn7ojSerialInitData *init_data) : SerialSP(init_data)
{
}

// Leaves any session (hanging up), then releases as the serial provider.
// FUNCTION: 0xee718
TbError ModemSP::Release(void)
{
    NetworkDebugPrintf("  LbNETWORK : Releasing ModemSP\n");
    if (in_session)
        Stop();
    return SerialSP::Release();
}

// Initialises the provider for game guid with callbacks (as the serial
// provider).
// FUNCTION: 0xee748
TbError ModemSP::Init(__tibcaGUID guid, __ic76sReceiveCallbacks *callbacks, void *data)
{
    return SerialSP::Init(guid, 0, callbacks, data);
}

// Hosts session `session` as player name: initialises the modem with the
// timeouts and waiting callback in data (a ModemStartData, which also
// receives the modem's result and response), sets it to auto-answer and
// starts the serial session. Returns Lb_OK, or Lb_FAIL.
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
        if (in_session)
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
            NetworkDebugPrintf("MODEM: USER ABORTS MODEM INIT FUNCTION");
            init->result = result;
            return Lb_FAIL;
        }
        if (result == -1) {
            NetworkDebugPrintf("MODEM: Didin't get modem Response from InitModem\n");
            init->result = result;
            return Lb_FAIL;
        }
        if (result != 1) {
            NetworkDebugPrintf("MODEM: Modem Response wasn't OK on InitModem\n");
            strcpy(init->response, CH(comm)->response);
            init->result = result;
            NetworkDebugPrintf("MODEM RESPONSE CODE = %s\n", comm);
            return Lb_FAIL;
        }
        strcpy(init->response, CH(comm)->response);
        init->result = result;
        NetworkDebugPrintf("MODEM RESPONSE CODE = %s\n", comm);
        comm->SendModemString(CH(comm)->reset_str);
        response = comm->GetModemResponse(CH(comm)->timeout);
        NetworkDebugPrintf("ANSWER MODEM RESPONSE CODE = %s\n", comm);
        if (response == 1 || response == 3 || response == 2) {
            ret = SerialSP::Start(session, name, max_players, 0);
            if (ret != Lb_OK) {
                if (response != 1) {
                    NetworkDebugPrintf("MODEM: I COULDNT JOIN SERVER SO I MUST HANGUP THE MODEM\n");
                    comm->HangUpModem();
                }
                NetworkDebugPrintf("MODEM: TURN OFF AUTO ANSWER BY CALLING INITIALISE\n");
                comm->InitModem();
            }
        } else if (response == -1) {
            NetworkDebugPrintf("MODEM: Didn't responsded to Auto Answer Command\n");
            init->result = response;
            return Lb_FAIL;
        } else {
            NetworkDebugPrintf("MODEM: Responded with the following return Code\n");
            strcpy(init->response, CH(comm)->response);
            init->result = response;
            NetworkDebugPrintf("MODEM RESPONSE CODE = %s\n", comm);
            return Lb_FAIL;
        }
    }
    return ret;
}

// Joins by modem as player name: initialises the modem, dials data's
// number and, once connected, joins the serial session. data (a
// ModemStartData) gives the timeouts and waiting callbacks and receives the
// modem's result (-2 if the join failed) and response. Returns Lb_OK, or
// Lb_FAIL.
// FUNCTION: 0xeea58
// NONMATCHING: register choice for the first timeout multiply (eax/edx, here ebx)
TbError ModemSP::Start(TbNetworkSessionNameEntry *session, char *name, void *data)
{
    long result;
    ModemStartData *init;
    TbError ret = Lb_FAIL;

    if (started) {
        if (in_session)
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
                NetworkDebugPrintf("USER ABORTS MODEM INIT FUNCTION\n");
                init->result = result;
                return Lb_FAIL;
            }
            if (result == -1) {
                NetworkDebugPrintf("DIDN'T GET MODEM RESPONSE FROM INIT FUNCTION\n");
                init->result = result;
                return Lb_FAIL;
            }
            if (result != 1) {
                strcpy(init->response, CH(comm)->response);
                init->result = result;
                NetworkDebugPrintf("DIDN'T GET OK MODEM RESPONSE CODE FOR INIT= %s\n", comm);
                return Lb_FAIL;
            }
            strcpy(init->response, CH(comm)->response);
            init->result = result;
            NetworkDebugPrintf("MODEM RESPONSE CODE = %s\n", comm);
            result = comm->DialModem(init->number);
            comm->SetDialModemFunc(0);
            if (result == -7) {
                NetworkDebugPrintf("USER ABORTS MODEM DIALING FUNCTION\n");
                init->result = result;
                NetworkDebugPrintf("OK ILL BETTER HANGUP THE MODEM JUST IN-CASE\n");
                comm->HangUpModem();
                return Lb_FAIL;
            }
            if (result == -1) {
                NetworkDebugPrintf("NO MODEM RESPONSE FROM DIALING\n");
                init->result = result;
                NetworkDebugPrintf("OK ILL BETTER HANGUP THE MODEM JUST IN-CASE\n");
                comm->HangUpModem();
                return Lb_FAIL;
            }
            strcpy(init->response, CH(comm)->response);
            init->result = result;
            NetworkDebugPrintf("MODEM RESPONSE CODE = %s\n", comm);
            if (result == 2) {
                ret = SerialSP::Start((TbNetworkSessionNameEntry *)0, name, 0);
                if (ret == Lb_FAIL) {
                    memset(init->response, 0, sizeof(init->response));
                    init->result = -2;
                    NetworkDebugPrintf("NO SERVER FOUND HANGINGUP MODEM\n");
                    comm->HangUpModem();
                    return ret;
                }
            } else if (result == 4) {
                NetworkDebugPrintf("MODEM: IS BUSY OR ENGAGED\n");
                return Lb_FAIL;
            } else if (result == 7) {
                NetworkDebugPrintf("MODEM: THERE IS NO DIALTONE\n");
                return Lb_FAIL;
            } else if (result == 5) {
                NetworkDebugPrintf("MODEM DIDN'T UNDERSTAND MODEM STRING\n");
                return Lb_FAIL;
            } else if (result == 6) {
                NetworkDebugPrintf("MODEM: THERE IS NOCARRIER\n");
                return Lb_FAIL;
            }
        }
    }
    return ret;
}

// Sends message buf to the other machine (as the serial provider).
// FUNCTION: 0xeedcc
TbError ModemSP::SendMessage(u32 to, void *buf, TbMessageType type)
{
    return SerialSP::SendMessage(0, buf, type);
}

// Leaves the running session and hangs up. Returns the serial Stop's result
// (undefined if no session was running).
// FUNCTION: 0xeedd8
TbError ModemSP::Stop(void)
{
    TbError ret;

    if (in_session) {
        ret = SerialSP::Stop();
        NetworkDebugPrintf("CALLING HANGUPMODEM\n");
        comm->HangUpModem();
    }
    return ret;
}

// Takes the next received message (as the serial provider).
// FUNCTION: 0xeee0c
u32 ModemSP::ReadMessage(u32 *from, void *buf, u32 *size)
{
    return SerialSP::ReadMessage(from, buf, size);
}

// Looks at the next received message (as the serial provider).
// FUNCTION: 0xeee14
u32 ModemSP::PeekMessage(u32 *from, void *buf, u32 *size)
{
    return SerialSP::PeekMessage(from, buf, size);
}

/*
 * ModemSP::~ModemSP() is compiler-generated in this module, together with
 * the class's vtable (the constructor is defined here).
 */
// FUNCTION: 0xeee20 COMDAT

#include "net.h"

/* Built without inline expansion: callees in this file stay calls. */
#pragma inline_depth(0)

extern "C" {
extern char D_00127244[];
extern char D_00127266[];
extern char D_0012729e[];
extern char D_001272dc[];
extern char D_00127301[];
extern char D_00127319[];
extern char D_00127328[];
extern char D_00127340[];
extern char D_00127389[];
extern char D_001273cf[];
extern char D_00127419[];
extern char D_0012744f[];
extern char D_00127485[];
extern char D_001274c1[];
extern char D_00127503[];
extern char D_00127539[];
extern char D_00127575[];
extern char D_001275b8[];
extern char D_001275f5[];
extern char D_001279b1[];
extern char D_001279e6[];
extern char D_00127a26[];
extern char D_00127b32[];
extern char D_00127b7d[];
extern char D_00127bbf[];
extern char D_00127632[];
extern char D_00127671[];
extern char D_001276b9[];
extern char D_001276f6[];
extern char D_00127747[];
extern char D_00127798[];
extern char D_001277e9[];
extern char D_0012783a[];
extern char D_0012788b[];
extern char D_001278c5[];
extern char D_001278d4[];
extern char D_0012790e[];
extern char D_00127956[];
extern char D_001279a3[];
extern char D_00127a6b[];
extern char D_00127aaf[];
extern u8 D_0018d01c[];
extern TbNetworkPlayerEntry D_0018cfc4;
}

// FUNCTION: 0xed350
SerialSP::SerialSP()
{
}

// FUNCTION: 0xed3c4
SerialSP::SerialSP(__4dn7ojSerialInitData *init_data)
{
    memcpy(&serial_data, init_data, sizeof(serial_data));
    port = init_data->port;
    baud = init_data->baud;
    send_seq = 0;
    recv_seq = 0;
    awaiting_ack = 0;
}

// FUNCTION: 0xed494
TbError SerialSP::ChangeSettings(u32 setting, void *data)
{
    return Lb_OK;
}

// FUNCTION: 0xed498
TbError SerialSP::Release(void)
{
    u32 start;

    fn_000c20c0(D_00127244);
    if (!started) {
        fn_000c20c0(D_00127266);
        return Lb_FAIL;
    }
    if (!comm) {
        fn_000c20c0(D_0012729e);
    } else {
        if (unknown528)
            Stop();
        start = LbTimerClock();
        while (LbTimerClock() - start < 1000)
            Receive(8);
        comm->DeInit();
        delete comm;
        comm = 0;
        started = 0;
    }
    return ServiceProvider::Release();
}

// FUNCTION: 0xed558
// NONMATCHING: operator new and exception-table relocations carry the target's misnamed symbols
TbError SerialSP::Init(__tibcaGUID guid, __tibcaGUID *pguid, __ic76sReceiveCallbacks *cb, void *data)
{
    TbError ret = Lb_FAIL;

    fn_000c20c0(D_001272dc);
    send_seq = 0;
    recv_seq = 0;
    started = 0;
    awaiting_ack = 0;
    if (ServiceProvider::Initialise(cb, data) == Lb_OK) {
        this->guid = guid;
        comm = new CommHandler;
        if (comm) {
            fn_000c20c0(D_00127301);
            fn_000c20c0(D_00127319, serial_data.port);
            comm->ReadSerialDev(&serial_data);
            if (comm->Init(port)) {
                fn_000c20c0(D_00127328);
                if (comm->SetBaud(baud) == 0)
                    ret = Lb_OK;
                else
                    fn_000c20c0(D_00127340);
            } else {
                fn_000c20c0(D_00127389);
            }
        }
        if (ret == Lb_OK)
            started = 1;
    } else {
        fn_000c20c0(D_001273cf);
    }
    return ret;
}

// FUNCTION: 0xed6c8
// NONMATCHING: register allocation differs
TbError SerialSP::Start(char *session, char *name, u32 max_players, void *data)
{
    TbError ret = Lb_FAIL;

    if (!started) {
        fn_000c20c0(D_00127419);
    } else {
        if (max_players != 2) {
            fn_000c20c0(D_0012744f);
            return ret;
        }
        StrCpyTrunc(this->name, session, 0x20);
        if (!comm) {
            fn_000c20c0(D_00127485);
            return ret;
        }
        if (unknown528)
            Stop();
        local_id = 0;
        is_host = 1;
        ClearPlayers();
        if (AddPlayer(local_id, name, 1, 1) != Lb_OK)
            fn_000c20c0(D_001274c1);
        unknown528 = 1;
        ret = Lb_OK;
    }
    return ret;
}

// FUNCTION: 0xed798
TbError SerialSP::Start(TbNetworkSessionNameEntry *session, char *name, void *data)
{
    TbError ret = Lb_FAIL;
    u32 start;
    u32 waiting;

    if (!started) {
        fn_000c20c0(D_00127503);
        return Lb_FAIL;
    }
    ClearPlayers();
    if (!comm) {
        fn_000c20c0(D_00127539);
    } else {
        if (unknown528)
            Stop();
        local_id = 1;
        is_host = 0;
        if (GetAddPlayerMsgSize(name) > 0x28) {
            fn_000c20c0(D_00127575);
            return ret;
        }
        EncodeAddPlayerMsg(addPlayerBuffer, local_id, name);
        if (SendMessage(is_host, addPlayerBuffer, (TbMessageType)1) == Lb_OK) {
            start = LbTimerClock();
            waiting = 1;
            do {
                LookForSystemMessages();
                if (player_count > 0) {
                    waiting = 0;
                    if (AddPlayer(local_id, name, 1, 0) != Lb_OK)
                        fn_000c20c0(D_001275b8);
                    unknown528 = 1;
                    ret = Lb_OK;
                } else if (LbTimerClock() - start > 10000) {
                    waiting = 0;
                }
            } while (waiting);
        } else {
            fn_000c20c0(D_001275f5);
        }
    }
    return ret;
}

// FUNCTION: 0xed8ec
// NONMATCHING: stack slots of the buf spill and total swapped
TbError SerialSP::SendMessage(u32 to, void *buf, TbMessageType type)
{
    u32 total;
    TbError ret = Lb_FAIL;
    u16 sum = 0;
    u8 *data = (u8 *)buf;
    u32 guaranteed;
    u32 size;
    u32 retries;
    u32 start;
    u32 i;

    switch (type) {
    case 1:
        guaranteed = 1;
        break;
    case 0:
        guaranteed = 0;
        break;
    default:
        fn_000c20c0(D_00127632);
        return ret;
    }
    DecodeMessageStub(buf, &size, 0, 0);
    if (guaranteed) {
        total = size + 0xb;
        retries = 0;
        if (total > 0x40b) {
            fn_000c20c0(D_00127671);
            return ret;
        }
        memcpy(D_0018d01c, D_00151a78, 4);
        memcpy(D_0018d01c + 4, &send_seq, 1);
        memcpy(D_0018d01c + 5, buf, size + 4);
        for (i = 0; i < 4; i++)
            sum += D_00151a78[i];
        sum += send_seq;
        for (i = 0; i < size + 4; i++)
            sum += data[i];
        memcpy(&D_0018d01c[size + 9], &sum, 2);
        if (comm->WriteBytes(D_0018d01c, total) == total) {
            awaiting_ack = 1;
            start = LbTimerClock();
            while (awaiting_ack) {
                Receive(8);
                if (awaiting_ack) {
                    if (LbTimerClock() - start > 2500) {
                        if (++retries > 8) {
                            fn_000c20c0(D_001276b9);
                            awaiting_ack = 0;
                        } else if (comm->WriteBytes(D_0018d01c, total) == total) {
                            start = LbTimerClock();
                        } else {
                            fn_000c20c0(D_001276f6);
                            awaiting_ack = 0;
                        }
                    }
                } else {
                    ret = Lb_OK;
                }
            }
        } else {
            fn_000c20c0(D_00127747);
        }
    } else {
        if (comm->WriteBytes(D_00151a74, 4) == 4) {
            if (comm->WriteBytes((u8 *)buf, size + 4) == size + 4) {
                for (i = 0; i < 4; i++)
                    sum += D_00151a74[i];
                for (i = 0; i < size + 4; i++)
                    sum += data[i];
                if (comm->WriteBytes((u8 *)&sum, 2) == 2)
                    ret = Lb_OK;
                else
                    fn_000c20c0(D_00127798);
            } else {
                fn_000c20c0(D_001277e9);
            }
        } else {
            fn_000c20c0(D_0012783a);
        }
    }
    return ret;
}

// FUNCTION: 0xedc2c
TbError SerialSP::Enumerate(void (__stdcall *callback)(TbNetworkCallbackData *, void *), void *data)
{
    TbNetworkSessionNameEntry *session;

    if (!started) {
        fn_000c20c0(D_0012788b);
        return Lb_FAIL;
    }
    ClearSessions();
    session = AddSession(0, D_001278c5);
    if (callback)
        callback((TbNetworkCallbackData *)session, data);
    return Lb_OK;
}

// FUNCTION: 0xedc7c
TbError SerialSP::Enumerate(TbNetworkSessionNameEntry *session, void (__stdcall *callback)(TbNetworkCallbackData *, void *), void *data)
{
    TbError ret;

    if (!started) {
        fn_000c20c0(D_001278d4);
        return Lb_FAIL;
    }
    ret = Lb_OK;
    if (unknown528) {
        if (LookForSystemMessages() == Lb_OK) {
            if (EnumeratePlayers(callback, data) != Lb_OK) {
                ret = Lb_FAIL;
                fn_000c20c0(D_0012790e);
            }
        } else {
            fn_000c20c0(D_00127956);
        }
    } else {
        D_0018cfc4.unknown5 = 0;
        D_0018cfc4.host = 0;
        D_0018cfc4.id = 0;
        strcpy(D_0018cfc4.name, D_001279a3);
        callback((TbNetworkCallbackData *)&D_0018cfc4, data);
    }
    return ret;
}

// FUNCTION: 0xedd3c
TbError SerialSP::Stop(void)
{
    u32 size;
    TbError ret = Lb_FAIL;

    size = GetRequestCompositeExchangeDataMsgSize();
    if (!started) {
        fn_000c20c0(D_001279b1);
    } else {
        if (!unknown528) {
            fn_000c20c0(D_001279e6);
            return ret;
        }
        if (size > 8) {
            fn_000c20c0(D_00127a26);
            return ret;
        }
        EncodeDeletePlayerMsg(D_0018d428, local_id);
        BroadcastSystemMessage(D_0018d428);
        unknown528 = 0;
        ClearPlayers();
        ret = Lb_OK;
    }
    return ret;
}

// FUNCTION: 0xeddc8
u32 SerialSP::ReadMessage(u32 *from, void *buf, u32 *size)
{
    u32 ret = 0;

    if (PrivateReceive(buf, size, (TbMessageType)0)) {
        switch (local_id) {
        case 0:
            *from = 1;
            break;
        case 1:
            *from = 0;
            break;
        default:
            fn_000c20c0(D_00127a6b);
            break;
        }
        ret = 1;
    }
    return ret;
}

// FUNCTION: 0xede14
u32 SerialSP::PeekMessage(u32 *from, void *buf, u32 *size)
{
    u32 ret = 0;

    if (PrivateReceive(buf, size, (TbMessageType)1)) {
        switch (local_id) {
        case 0:
            *from = 1;
            break;
        case 1:
            *from = 0;
            break;
        default:
            fn_000c20c0(D_00127aaf);
            break;
        }
        ret = 1;
    }
    return ret;
}

// FUNCTION: 0xede60
void SerialSP::SendAck(u8 seq)
{
    u16 sum = 0;
    u32 len = 7;
    u32 i;

    memcpy(ackBuffer, acknowledgementHeader, 4);
    memcpy(ackBuffer + 4, &seq, 1);
    for (i = 0; i < 4; i++)
        sum += acknowledgementHeader[i];
    sum += seq;
    memcpy(ackBuffer + 5, &sum, 2);
    if (comm->WriteBytes(ackBuffer, len) != len)
        fn_000c20c0(D_00127b32);
}

// FUNCTION: 0xedf24
// NONMATCHING: register allocation differs
u32 SerialSP::PrivateReceive(void *buf, u32 *size, TbMessageType mode)
{
    u32 keep_going = 1;
    u32 max_size;
    u32 remove;
    u32 got;
    u32 found;
    long c;
    Parser *p;

    max_size = *size;
    switch (mode) {
    case 0:
        remove = 1;
        break;
    case 1:
        remove = 0;
        break;
    default:
        fn_000c20c0(D_00127b7d);
        return 0;
    }
    got = ProcessAvailableMsg(buf, size, remove);
    if (got || *size)
        return got;
    while (keep_going) {
        c = comm->ReadByte();
        if (c == -1) {
            keep_going = 0;
            continue;
        }
        found = 0;
        for (p = parsers; p != parsers + 10; p++) {
            if (p->active) {
                p->InputByte(c);
            } else if (!found) {
                found = 1;
                p->state = PS_HEADER;
                p->count = 0;
                p->active = 1;
                p->InputByte(c);
            }
        }
        *size = max_size;
        got = ProcessAvailableMsg(buf, size, remove);
        if (got || *size)
            keep_going = 0;
    }
    return got;
}

// FUNCTION: 0xee050
u32 SerialSP::ProcessAvailableMsg(void *buf, u32 *size, u32 remove)
{
    Parser *p;
    Parser *q;
    u32 max_size;
    u32 msg_size;
    u32 got = 0;
    u32 deliver;
    u32 i;
    u32 j;
    u32 k;
    char *name;
    u8 type;

    max_size = *size;
    *size = 0;
    for (i = 0, p = parsers; i < 10; i++, p++) {
        if (!p->active || p->state != PS_COMPLETE)
            continue;
        deliver = 0;
        for (j = 0, q = parsers; j < 10; j++, q++) {
            if (q->active && j != i)
                q->active = 0;
        }
        switch (p->header[0]) {
        case 0x1b:
            deliver = 1;
            break;
        case 0x1e:
            if (p->seq == recv_seq) {
                deliver = 1;
            } else {
                SendAck(p->seq);
                p->active = 0;
            }
            break;
        case 4:
            if (p->seq == send_seq) {
                if (awaiting_ack) {
                    awaiting_ack = 0;
                    send_seq++;
                    p->active = 0;
                } else {
                    p->active = awaiting_ack;
                }
            } else {
                p->active = 0;
            }
            break;
        }
        if (!deliver) {
            p->active = deliver;
            continue;
        }
        DecodeMessageStub(p->msg, &msg_size, 0, 0);
        *size = msg_size + 4;
        if (*size > max_size)
            continue;
        memcpy(buf, p->msg, *size);
        if (remove) {
            if (p->header[0] == 0x1e) {
                SendAck(recv_seq);
                recv_seq++;
            }
            DecodeMessageStub(p->msg, 0, (TbMessageType *)&type, 0);
            if (type == 2) {
                send_seq = 0;
                recv_seq = 0;
                awaiting_ack = 0;
            }
            if (type == 1) {
                p->active = 0;
                name = 0;
                for (k = 0; k < player_count; k++) {
                    if (players[k].id == 0)
                        name = players[k].name;
                }
                if (name) {
                    if (GetAddPlayerMsgSize(name) > 0x28) {
                        fn_000c20c0(D_00127bbf);
                    } else {
                        EncodeAddPlayerMsg(addPlayerBuffer, 0, name);
                        Send(1, addPlayerBuffer);
                    }
                }
            }
            p->active = 0;
        }
        got = 1;
    }
    return got;
}

// FUNCTION: 0xee308
Parser::Parser()
{
    active = 0;
    count = 0;
    state = PS_HEADER;
}

// FUNCTION: 0xee31c
void Parser::Init(void)
{
    state = PS_HEADER;
    count = 0;
    active = 1;
}

// FUNCTION: 0xee330
u32 Parser::InUse(void)
{
    return active;
}

// FUNCTION: 0xee334
void Parser::TakeDown(void)
{
    active = 0;
}

// FUNCTION: 0xee33c
u8 *Parser::GetMessageBuffer(void)
{
    return msg;
}

// FUNCTION: 0xee340
u32 Parser::GotCompleteMessage(void)
{
    u32 ret = 0;

    if (state == PS_COMPLETE)
        ret = 1;
    return ret;
}

// FUNCTION: 0xee358
u8 Parser::GetFirstHeaderByte(void)
{
    return header[0];
}

// FUNCTION: 0xee360
u8 Parser::GetSequenceNumber(void)
{
    return seq;
}

// FUNCTION: 0xee380
// NONMATCHING: return value kept in a register instead of its frame slot; checksum loop differs
TbError Parser::InputByte(u8 c)
{
    TbError ret = Lb_OK;
    u16 checksum;
    u16 sum;
    u32 i;

    switch (state) {
    case PS_HEADER:
        if (count == 0) {
            count++;
            switch (c) {
            case 0x1b:
                memcpy(header, D_00151a74, 4);
                break;
            case 0x1e:
                memcpy(header, D_00151a78, 4);
                break;
            case 4:
                memcpy(header, acknowledgementHeader, 4);
                break;
            default:
                active = 0;
                return ret;
            }
        } else {
            if (c != header[count]) {
                active = 0;
                return ret;
            }
            count++;
            if (count >= 4) {
                if (header[0] == 0x1e || header[0] == 4)
                    state = PS_SEQUENCE;
                else
                    state = PS_SIZE;
                count = 0;
            }
        }
        break;
    case PS_SEQUENCE:
        seq = c;
        if (header[0] == 0x1b || header[0] == 0x1e) {
            state = PS_SIZE;
        } else {
            state = PS_CHECKSUM;
            size = 0;
        }
        count = 0;
        break;
    case PS_SIZE:
        msg[count] = c;
        count++;
        if (count >= 4) {
            DecodeMessageStub(msg, &size, 0, 0);
            if (size > 0x401) {
                active = 0;
                return ret;
            }
            state = PS_DATA;
            count = 0;
        }
        break;
    case PS_DATA:
        msg[4 + count] = c;
        count++;
        if (count >= size) {
            state = PS_CHECKSUM;
            count = 0;
        }
        break;
    case PS_CHECKSUM:
        msg[4 + size + count] = c;
        count++;
        if (count >= 2) {
            memcpy(&checksum, &msg[size + 4], 2);
            sum = 0;
            for (i = 0; i < 4; i++)
                sum += header[i];
            if (header[0] == 0x1e || header[0] == 4)
                sum += seq;
            if (header[0] == 0x1e || header[0] == 0x1b) {
                for (i = 0; i < size + 4; i++)
                    sum += msg[i];
            }
            if ((u16)sum != checksum) {
                active = 0;
                return ret;
            }
            state = PS_COMPLETE;
        }
        break;
    case PS_COMPLETE:
        break;
    }
    return ret;
}

/*
 * SerialSP::~SerialSP() is compiler-generated in this module, together with
 * the class's vtable (the constructors are defined here).
 */
// FUNCTION: 0xee630

#include "net.h"

static u8 messageHeader[] = { 0x1b, 0x9c, 0x52, 0x8f };
static u8 deliveryHeader[] = { 0x1e, 0xca, 0x67, 0x72 };
static u8 acknowledgementHeader[] = { 0x04, 0x3e, 0xbe, 0x38 };

extern "C" {
extern u8 deliveryBuffer[];
extern TbNetworkPlayerEntry playerData;
}

// Constructs an unconfigured serial provider.
// FUNCTION: 0xed350
SerialSP::SerialSP()
{
}

// Constructs a serial provider for the port, baud and modem settings in
// init_data.
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

// No settings to change over a serial link. Returns Lb_OK.
// FUNCTION: 0xed494
TbError SerialSP::ChangeSettings(u32 setting, void *data)
{
    return Lb_OK;
}

// Leaves any session, keeps handling messages for one second (1000 ms), then
// closes the port. Returns Lb_FAIL if not initialised, else the base
// Release's result.
// FUNCTION: 0xed498
TbError SerialSP::Release(void)
{
    u32 start;

    NetworkDebugPrintf("  LbNETWORK : Releasing SerialSP\n");
    if (!started) {
        NetworkDebugPrintf("  WARNING network: SerialSP::Release() not initialised\n");
        return Lb_FAIL;
    }
    if (!comm) {
        NetworkDebugPrintf("  WARNING network: SerialSP::Release() NULL m_commHandlerPtr\n");
    } else {
        if (in_session)
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

// Initialises the provider for game guid with callbacks: opens the COM port
// and sets its speed. Returns Lb_OK, or Lb_FAIL.
// FUNCTION: 0xed558
TbError SerialSP::Init(__tibcaGUID guid, __tibcaGUID *pguid, __ic76sReceiveCallbacks *cb, void *data)
{
    TbError ret = Lb_FAIL;

    NetworkDebugPrintf("  LbNETWORK : Initialising SerialSP\n");
    send_seq = 0;
    recv_seq = 0;
    started = 0;
    awaiting_ack = 0;
    if (ServiceProvider::Initialise(cb, data) == Lb_OK) {
        this->guid = guid;
        comm = new CommHandler;
        if (comm) {
            NetworkDebugPrintf("CALLING SERIALSP INIT \n");
            NetworkDebugPrintf("\t COM PORT %d\n", serial_data.port);
            comm->ReadSerialDev(&serial_data);
            if (comm->Init(port)) {
                NetworkDebugPrintf("SERIAL SP INIT SUCCESS\n");
                if (comm->SetBaud(baud) == 0)
                    ret = Lb_OK;
                else
                    NetworkDebugPrintf("  WARNING network: SerialSP::SerialSP() failure on CommHandler::SetBaud\n");
            } else {
                NetworkDebugPrintf("  WARNING network: SerialSP::SerialSP() failure on CommHandler::Init\n");
            }
        }
        if (ret == Lb_OK)
            started = 1;
    } else {
        NetworkDebugPrintf("  WARNING network: SerialSP::SerialSP() failure on parent initialisation\n");
    }
    return ret;
}

// Hosts session `session` as player name (player 0); only two-player
// sessions are allowed. Returns Lb_OK, or Lb_FAIL.
// FUNCTION: 0xed6c8
// NONMATCHING: register allocation differs
TbError SerialSP::Start(char *session, char *name, u32 max_players, void *data)
{
    TbError ret = Lb_FAIL;

    if (!started) {
        NetworkDebugPrintf("  WARNING network: SerialSP::Start() not initialised\n");
    } else {
        if (max_players != 2) {
            NetworkDebugPrintf("  WARNING network: SerialSP::Start() maxPlayers != 2\n");
            return ret;
        }
        StrCpyTrunc(this->name, session, 0x20);
        if (!comm) {
            NetworkDebugPrintf("  WARNING network: SerialSP::Start() NULL m_commHandlerPtr\n");
            return ret;
        }
        if (in_session)
            Stop();
        local_id = 0;
        is_host = 1;
        ClearPlayers();
        if (AddPlayer(local_id, name, 1, 1) != Lb_OK)
            NetworkDebugPrintf("  WARNING network: Win95SerialSP::Start() failure on AddPlayer()\n");
        in_session = 1;
        ret = Lb_OK;
    }
    return ret;
}

// Joins the other machine's session as player name (player 1): sends an
// add-player message and waits up to 10 seconds for the host's reply.
// Returns Lb_OK, or Lb_FAIL.
// FUNCTION: 0xed798
TbError SerialSP::Start(TbNetworkSessionNameEntry *session, char *name, void *data)
{
    TbError ret = Lb_FAIL;
    u32 start;
    u32 waiting;

    if (!started) {
        NetworkDebugPrintf("  WARNING network: SerialSP::Start() not initialised\n");
        return Lb_FAIL;
    }
    ClearPlayers();
    if (!comm) {
        NetworkDebugPrintf("  WARNING network: SerialSP::Start() NULL m_commHandlerPtr\n");
    } else {
        if (in_session)
            Stop();
        local_id = 1;
        is_host = 0;
        if (GetAddPlayerMsgSize(name) > 0x28) {
            NetworkDebugPrintf("  WARNING network: SerialSP::Start() addPlayerBuffer is too small\n");
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
                        NetworkDebugPrintf("  WARNING network: SerialSP::Start() failure on AddPlayer()\n");
                    in_session = 1;
                    ret = Lb_OK;
                } else if (LbTimerClock() - start > 10000) {
                    waiting = 0;
                }
            } while (waiting);
        } else {
            NetworkDebugPrintf("  WARNING network: SerialSP::Start() failure on Broadcast()\n");
        }
    }
    return ret;
}

// Sends message buf over the link: guaranteed (type 1, with a sequence
// number, resent every 2.5 seconds until acknowledged, up to eight
// retries) or not (type 0). Each packet carries a header and a 16-bit
// checksum. Returns Lb_OK, or Lb_FAIL.
// FUNCTION: 0xed8ec
TbError SerialSP::SendMessage(u32 to, void *buf, TbMessageType type)
{
    u32 total;
    TbError ret = Lb_FAIL;
    u8 *data = (u8 *)buf;
    u32 guaranteed;
    u32 size;
    u16 sum = 0;
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
        NetworkDebugPrintf("  WARNING network: SerialSP::SendMessage() invalid sendMode()\n");
        return ret;
    }
    DecodeMessageStub(buf, &size, 0, 0);
    if (guaranteed) {
        total = size + 0xb;
        retries = 0;
        if (total > 0x40b) {
            NetworkDebugPrintf("  WARNING network: SerialSP::SendMessage() deliveryBuffer is too small\n");
            return ret;
        }
        memcpy(deliveryBuffer, deliveryHeader, 4);
        memcpy(deliveryBuffer + 4, &send_seq, 1);
        memcpy(deliveryBuffer + 5, buf, size + 4);
        for (i = 0; i < 4; i++)
            sum += deliveryHeader[i];
        sum += send_seq;
        for (i = 0; i < size + 4; i++)
            sum += data[i];
        memcpy(&deliveryBuffer[size + 9], &sum, 2);
        if (comm->WriteBytes(deliveryBuffer, total) == total) {
            awaiting_ack = 1;
            start = LbTimerClock();
            while (awaiting_ack) {
                Receive(8);
                if (awaiting_ack) {
                    if (LbTimerClock() - start > 2500) {
                        if (++retries > 8) {
                            NetworkDebugPrintf("  WARNING network: SerialSP::SendMessage() delivery failure\n");
                            awaiting_ack = 0;
                        } else if (comm->WriteBytes(deliveryBuffer, total) == total) {
                            start = LbTimerClock();
                        } else {
                            NetworkDebugPrintf("  WARNING network: SerialSP::SendMessage() failure on CommHandler::WriteBytes()\n");
                            awaiting_ack = 0;
                        }
                    }
                } else {
                    ret = Lb_OK;
                }
            }
        } else {
            NetworkDebugPrintf("  WARNING network: SerialSP::SendMessage() failure on CommHandler::WriteBytes()\n");
        }
    } else {
        if (comm->WriteBytes(messageHeader, 4) == 4) {
            if (comm->WriteBytes((u8 *)buf, size + 4) == size + 4) {
                for (i = 0; i < 4; i++)
                    sum += messageHeader[i];
                for (i = 0; i < size + 4; i++)
                    sum += data[i];
                if (comm->WriteBytes((u8 *)&sum, 2) == 2)
                    ret = Lb_OK;
                else
                    NetworkDebugPrintf("  WARNING network: SerialSP::SendMessage() failure on CommHandler::WriteBytes()\n");
            } else {
                NetworkDebugPrintf("  WARNING network: SerialSP::SendMessage() failure on CommHandler::WriteBytes()\n");
            }
        } else {
            NetworkDebugPrintf("  WARNING network: SerialSP::SendMessage() failure on CommHandler::WriteBytes()\n");
        }
    }
    return ret;
}

// Calls callback with the one serial session ("SERIAL_SESSION"). Returns
// Lb_OK, or Lb_FAIL if not initialised.
// FUNCTION: 0xedc2c
TbError SerialSP::Enumerate(void (__stdcall *callback)(TbNetworkCallbackData *, void *), void *data)
{
    TbNetworkSessionNameEntry *session;

    if (!started) {
        NetworkDebugPrintf("  WARNING network: SerialSP::Enumerate() not initialised\n");
        return Lb_FAIL;
    }
    ClearSessions();
    session = AddSession(0, "SERIAL_SESSION");
    if (callback)
        callback((TbNetworkCallbackData *)session, data);
    return Lb_OK;
}

// Calls callback with each player of the running session, or with a
// placeholder "REMOTE_PLAYER" when none is running. Returns Lb_OK, or
// Lb_FAIL.
// FUNCTION: 0xedc7c
TbError SerialSP::Enumerate(TbNetworkSessionNameEntry *session, void (__stdcall *callback)(TbNetworkCallbackData *, void *), void *data)
{
    TbError ret;

    if (!started) {
        NetworkDebugPrintf("  WARNING network: SerialSP::Enumerate() not initialised\n");
        return Lb_FAIL;
    }
    ret = Lb_OK;
    if (in_session) {
        if (LookForSystemMessages() == Lb_OK) {
            if (EnumeratePlayers(callback, data) != Lb_OK) {
                ret = Lb_FAIL;
                NetworkDebugPrintf("  WARNING network: SerialSP::Enumerate() failure on EnumeratePlayers()\n");
            }
        } else {
            NetworkDebugPrintf("  WARNING network: SerialSP::Enumerate() failure on LookForSystemMessages()\n");
        }
    } else {
        playerData.local = 0;
        playerData.host = 0;
        playerData.id = 0;
        strcpy(playerData.name, "REMOTE_PLAYER");
        callback((TbNetworkCallbackData *)&playerData, data);
    }
    return ret;
}

// Leaves the running session, telling the other machine. Returns Lb_OK, or
// Lb_FAIL if none was running or the provider is not initialised.
// FUNCTION: 0xedd3c
TbError SerialSP::Stop(void)
{
    u32 size;
    TbError ret = Lb_FAIL;

    size = GetRequestCompositeExchangeDataMsgSize();
    if (!started) {
        NetworkDebugPrintf("  WARNING network: SerialSP::Stop() not initialised\n");
    } else {
        if (!in_session) {
            NetworkDebugPrintf("  WARNING network: SerialSP::Stop() not connected to a session\n");
            return ret;
        }
        if (size > 8) {
            NetworkDebugPrintf("  WARNING network: SerialSP::Stop() deletePlayerBuffer is too small\n");
            return ret;
        }
        EncodeDeletePlayerMsg(serialDeletePlayerBuffer, local_id);
        BroadcastSystemMessage(serialDeletePlayerBuffer);
        in_session = 0;
        ClearPlayers();
        ret = Lb_OK;
    }
    return ret;
}

// Takes the next complete message received into buf (*size: its room, then
// its length); *from is the other player. Returns 1 if there was one.
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
            NetworkDebugPrintf("  WARNING network: SerialSP::ReadMessage() invalid m_localPlayerId\n");
            break;
        }
        ret = 1;
    }
    return ret;
}

// As ReadMessage, leaving the message waiting.
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
            NetworkDebugPrintf("  WARNING network: SerialSP::PeekMessage() invalid m_localPlayerId\n");
            break;
        }
        ret = 1;
    }
    return ret;
}

// Sends the acknowledgement for guaranteed message seq.
// FUNCTION: 0xede60
void SerialSP::SendAck(u8 seq)
{
    u16 sum = 0;
    u32 len = 7;
    u32 i;

    if (len > sizeof(ackBuffer)) {
        NetworkDebugPrintf("  WARNING network: SerialSP::SendAck() ackBuffer is too small\n");
        return;
    }
    memcpy(ackBuffer, acknowledgementHeader, 4);
    memcpy(ackBuffer + 4, &seq, 1);
    for (i = 0; i < 4; i++)
        sum += acknowledgementHeader[i];
    sum += seq;
    memcpy(ackBuffer + 5, &sum, 2);
    if (comm->WriteBytes(ackBuffer, len) != len)
        NetworkDebugPrintf("  WARNING network: SerialSP::SendAck() failure on CommHandler::WriteBytes\n");
}

// Feeds the received bytes to the packet parsers (up to ten partial
// packets) until a message is complete, then copies it into buf (removing
// it if mode is 0, peeking if 1). Returns 1 if a message was copied.
// FUNCTION: 0xedf24
// NONMATCHING: register allocation differs
u32 SerialSP::PrivateReceive(void *buf, u32 *size, TbMessageType mode)
{
    u32 keep_going = 1;
    u32 max_size;
    u32 remove;
    u32 got;
    u32 found;
    long byte;
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
        NetworkDebugPrintf("  WARNING network: SerialSP::PrivateReceive() invalid mode value\n");
        return 0;
    }
    got = ProcessAvailableMsg(buf, size, remove);
    if (got || *size)
        return got;
    while (keep_going) {
        byte = comm->ReadByte();
        if (byte == -1) {
            keep_going = 0;
            continue;
        }
        found = 0;
        for (p = parsers; p != parsers + 10; p++) {
            if (p->active) {
                p->InputByte(byte);
            } else if (!found) {
                found = 1;
                p->state = PS_HEADER;
                p->count = 0;
                p->active = 1;
                p->InputByte(byte);
            }
        }
        *size = max_size;
        got = ProcessAvailableMsg(buf, size, remove);
        if (got || *size)
            keep_going = 0;
    }
    return got;
}

// Handles the first complete packet, dropping every other partial one:
// applies an acknowledgement, re-acknowledges and drops an out-of-sequence
// guaranteed message, and copies a plain or in-sequence guaranteed message
// into buf if it fits. If remove is set, a copied message is consumed: a
// guaranteed one is acknowledged, type 2 resets the sequence numbers and an
// add-player message is answered with player 0's name. *size (on entry the
// room in buf) is set to the message's length, header included, or 0.
// Returns 1 if a message was copied.
// FUNCTION: 0xee050
// NONMATCHING: stack slots and register choice
u32 SerialSP::ProcessAvailableMsg(void *buf, u32 *size, u32 remove)
{
    u32 j;
    TbMessageType type;
    Parser *other;
    u32 msg_size;
    u32 got = 0;
    u32 deliver;
    u32 i;
    Parser *p;
    u32 k;
    char *name;
    u32 max_size;

    max_size = *size;
    *size = 0;
    for (i = 0, p = parsers; i < 10; i++, p++) {
        if (!p->InUse() || !p->GotCompleteMessage())
            continue;
        deliver = 0;
        for (j = 0, other = parsers; j < 10; j++, other++) {
            if (other->InUse() && j != i)
                other->TakeDown();
        }
        switch (p->GetFirstHeaderByte()) {
        case 0x1b:
            deliver = 1;
            break;
        case 0x1e:
            if (p->GetSequenceNumber() == recv_seq) {
                deliver = 1;
            } else {
                SendAck(p->GetSequenceNumber());
                p->TakeDown();
            }
            break;
        case 4:
            if (p->GetSequenceNumber() == send_seq) {
                if (awaiting_ack) {
                    awaiting_ack = 0;
                    send_seq++;
                    p->TakeDown();
                } else {
                    p->TakeDown();
                }
            } else {
                p->TakeDown();
            }
            break;
        }
        if (deliver) {
            DecodeMessageStub(p->GetMessageBuffer(), &msg_size, 0, 0);
            *size = msg_size + 4;
            if (*size <= max_size) {
                memcpy(buf, p->GetMessageBuffer(), *size);
                if (remove) {
                    if (p->GetFirstHeaderByte() == 0x1e) {
                        SendAck(recv_seq);
                        recv_seq++;
                    }
                    DecodeMessageStub(p->GetMessageBuffer(), 0, &type, 0);
                    if (type == (TbMessageType)2) {
                        send_seq = 0;
                        recv_seq = 0;
                        awaiting_ack = 0;
                    }
                    if (type == (TbMessageType)1) {
                        name = 0;
                        p->TakeDown();
                        for (k = 0; k < player_count; k++) {
                            if (players[k].id == 0)
                                name = players[k].name;
                        }
                        if (name) {
                            if (GetAddPlayerMsgSize(name) > 0x28) {
                                NetworkDebugPrintf("  WARNING network: SerialSP::PrivateReceive addPlayerBuffer too small\n");
                            } else {
                                EncodeAddPlayerMsg(addPlayerBuffer, 0, name);
                                Send(1, addPlayerBuffer);
                            }
                        }
                    }
                    p->TakeDown();
                }
                got = 1;
            }
        } else {
            p->TakeDown();
        }
    }
    return got;
}

// Constructs an idle packet parser.
// FUNCTION: 0xee308
Parser::Parser()
{
    active = 0;
    count = 0;
    state = PS_HEADER;
}

// Starts the parser on a new packet.
// FUNCTION: 0xee31c
void Parser::Init(void)
{
    state = PS_HEADER;
    count = 0;
    active = 1;
}

// Returns nonzero if the parser is working on a packet.
// FUNCTION: 0xee330
u32 Parser::InUse(void)
{
    return active;
}

// Frees the parser.
// FUNCTION: 0xee334
void Parser::TakeDown(void)
{
    active = 0;
}

// Returns the parser's message buffer.
// FUNCTION: 0xee33c
u8 *Parser::GetMessageBuffer(void)
{
    return msg;
}

// Returns 1 if the parser holds a complete, checksummed packet, else 0.
// FUNCTION: 0xee340
u32 Parser::GotCompleteMessage(void)
{
    u32 ret = 0;

    if (state == PS_COMPLETE)
        ret = 1;
    return ret;
}

// Returns the first byte of the parser's packet header (0x1b message, 0x1e
// guaranteed message, 4 acknowledgement).
// FUNCTION: 0xee358
u8 Parser::GetFirstHeaderByte(void)
{
    return header[0];
}

// Returns the parsed packet's sequence number.
// FUNCTION: 0xee360
u8 Parser::GetSequenceNumber(void)
{
    return seq;
}

// Feeds one received byte to the parser: header, sequence number, message
// size and data, then the checksum; a bad header, size or checksum frees
// the parser. Returns Lb_OK.
// FUNCTION: 0xee380
TbError Parser::InputByte(u8 byte)
{
    TbError ret = Lb_OK;
    u16 checksum;
    u16 sum;
    u32 i;

    switch (state) {
    case PS_HEADER:
        if (count == 0) {
            count++;
            switch (byte) {
            case 0x1b:
                memcpy(header, messageHeader, 4);
                break;
            case 0x1e:
                memcpy(header, deliveryHeader, 4);
                break;
            case 4:
                memcpy(header, acknowledgementHeader, 4);
                break;
            default:
                goto fail;
            }
        } else {
            if (byte != header[count])
                goto fail;
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
        seq = byte;
        if (header[0] == 0x1b || header[0] == 0x1e) {
            state = PS_SIZE;
        } else {
            state = PS_CHECKSUM;
            size = 0;
        }
        count = 0;
        break;
    case PS_SIZE:
        msg[count] = byte;
        count++;
        if (count >= 4) {
            DecodeMessageStub(msg, &size, 0, 0);
            if (size > 0x401) {
            fail:
                active = 0;
                break;
            }
            state = PS_DATA;
            count = 0;
        }
        break;
    case PS_DATA:
        msg[4 + count] = byte;
        count++;
        if (count >= size) {
            state = PS_CHECKSUM;
            count = 0;
        }
        break;
    case PS_CHECKSUM:
        msg[4 + size + count] = byte;
        count++;
        if (count >= 2) {
            sum = 0;
            memcpy(&checksum, &msg[size + 4], 2);
            for (i = 0; i < 4; i++)
                sum += header[i];
            if (header[0] == 0x1e || header[0] == 4)
                sum += seq;
            if (header[0] == 0x1e || header[0] == 0x1b) {
                for (i = 0; i < size + 4; i++)
                    sum += msg[i];
            }
            if ((u16)sum != checksum)
                goto fail;
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
// FUNCTION: 0xee630 COMDAT

#include "net.h"

static u32 actualTimeout;

// Creates and initialises the serial-cable service provider with the port
// settings in init_data, switching to the two-player exchange. Returns
// Lb_OK, or Lb_FAIL.
// FUNCTION: 0xd2ba0
TbError GenericSerialInit(__tibcaGUID guid, void *init_data)
{
    __4dn7ojSerialInitData *serial = (__4dn7ojSerialInitData *)init_data;
    TbError ret = Lb_FAIL;

    if (spPtr) {
        spPtr->Release();
        delete spPtr;
        spPtr = 0;
    }
    runningTwoPlayerModel = 1;
    receiveCallbacks.multiPlayer = (void *(__stdcall *)(u32, u32, u32, void *))TwoPlayerCallback;
    receiveCallbacks.mpReqExDataMsg = (void (__stdcall *)(u32, u32, void *))TwoPlayerReqExDataMsgCallback;
    if (init_data)
        serial->mode = 1;
    spPtr = new SerialSP(serial);
    if (spPtr) {
        if (spPtr->Init(guid, 0, &receiveCallbacks, 0) == Lb_OK)
            ret = Lb_OK;
        else
            fn_000c20c0("  WARNING network: GenericSerialInit() failure on ServiceProvider::Init()\n");
    }
    return ret;
}

// Two-player model: sends this player's packet buf to the other player
// (keeping the last two sent for resends) and stores it in the exchange
// buffer. Returns Lb_OK, or Lb_FAIL if the send failed.
// FUNCTION: 0xd2cb0
TbError StartTwoPlayerExchange(void *buf)
{
    TbError ret = Lb_FAIL;

    if (!clientDataTable[remotePlayerIndex].active)
        spPtr->Receive(2);
    gotCompositeData = 0;
    if (clientDataTable[remotePlayerIndex].active) {
        spPtr->Receive(8);
        spPtr->Receive(0x10);
    }
    memcpy(&exchangeBuffer[localPlayerIndex * exchangeSize], buf, exchangeSize);
    if (clientDataTable[remotePlayerIndex].active) {
        EncodeMessageStub(buf, exchangeSize - 4, (TbMessageType)0, sequenceNumber);
        if (spPtr->Send(remotePlayerId, buf) == Lb_OK) {
            ret = Lb_OK;
            startTime = LbTimerClock();
            if (exchangeTimeout == 0)
                actualTimeout = basicTimeout;
            else
                actualTimeout = exchangeTimeout;
            memcpy(lastButOneMessage, lastMessage, exchangeSize);
            memcpy(lastMessage, buf, exchangeSize);
        } else {
            fn_000c20c0("  WARNING network: StartTwoPlayerExchange() failure on SP::Send()\n");
        }
    } else {
        ret = Lb_OK;
    }
    return ret;
}

// Two-player model: waits for the other player's packet, asking for it
// again on timeout and dropping the other player after ten retries; adapts
// the timeout to the measured round trip and advances the sequence number.
// Returns Lb_OK, or Lb_FAIL.
// FUNCTION: 0xd2e40
TbError CompleteTwoPlayerExchange(void *buf)
{
    TbError ret = Lb_FAIL;
    u32 keep_going = 1;
    u32 retries = 0;
    long elapsed;

    if (!clientDataTable[remotePlayerIndex].active)
        return Lb_OK;
    while (keep_going) {
        spPtr->Receive(8);
        if (gotCompositeData) {
            keep_going = 0;
            if (retries == 0) {
                elapsed = LbTimerClock() - startTime;
                if (elapsed < 0)
                    elapsed = -elapsed;
                if (timeCount == 0 || elapsed > D_0018ba9c)
                    D_0018ba9c = elapsed;
                timeCount++;
                if (timeCount >= 50) {
                    timeCount = 0;
                    basicTimeout = D_0018ba9c * 3;
                    if (basicTimeout < 250)
                        basicTimeout = 250;
                }
            }
            ret = Lb_OK;
        } else {
            elapsed = LbTimerClock() - startTime;
            if (elapsed < 0)
                elapsed = -elapsed;
            if (elapsed > actualTimeout) {
                fn_000c20c0("Timed out (%d) waiting for seq %d - %d ms\n", retries, sequenceNumber, actualTimeout);
                retries++;
                fn_000c20c0("Retries = %d\n", retries);
                if (retries > 10) {
                    if (GetRequestCompositeExchangeDataMsgSize() > 8) {
                        fn_000c20c0("  WARNING network: CompleteTwoPlayerExchange() deletePlayerBuffer is too small\n");
                    } else {
                        EncodeDeletePlayerMsg(deletePlayerBuffer, remotePlayerId);
                        if (spPtr->Send(localPlayerId, deletePlayerBuffer) == Lb_OK)
                            fn_000c20c0("Sent delete player message\n");
                        else
                            fn_000c20c0("  WARNING network: CompleteTwoPlayerExchange() failure on SP::Send()\n");
                        return ret;
                    }
                } else if (GetRequestCompositeExchangeDataMsgSize() > 8) {
                    fn_000c20c0("  WARNING network: CompleteTwoPlayerExchange() requestExchangeDataBuffer is too small\n");
                } else {
                    EncodeRequestExchangeDataMsg(requestExchangeDataBuffer, localPlayerId, sequenceNumber);
                    fn_000c20c0("Requesting resend of packet (%d)\n", sequenceNumber);
                    if (spPtr->Send(remotePlayerId, requestExchangeDataBuffer) != Lb_OK)
                        fn_000c20c0("  WARNING network: CompleteTwoPlayerExchange() failure on SP::Send()\n");
                }
                startTime = LbTimerClock();
                if (exchangeTimeout == 0) {
                    if (retries < 3)
                        actualTimeout = basicTimeout;
                    else if (retries == 3)
                        actualTimeout = basicTimeout * 2;
                    else
                        actualTimeout = basicTimeout * 4 * (retries - 3);
                } else {
                    actualTimeout = exchangeTimeout;
                }
            }
        }
        if (!clientDataTable[remotePlayerIndex].active) {
            ret = Lb_OK;
            keep_going = 0;
        }
    }
    if (sequenceNumber != 15) {
        sequenceNumber++;
        if (sequenceNumber >= 15)
            sequenceNumber = 0;
    }
    return ret;
}

extern "C" {
}

extern "C" {
}

// Two-player provider callback for a packet with sequence number seq:
// returns where to store it, or 0 (asking for a resend if it is a packet
// behind).
// NONMATCHING: sequenceNumber cached in eax instead of ebx (size stays live in eax in the target)
// FUNCTION: 0xd309c
void *TwoPlayerCallback(u32 id, u32 size, u32 seq, void *data)
{
    u8 *ret = 0;
    u32 ok;
    u32 msg_size;
    TbMessageType msg_type;

    if (localPlayerId == hostId) {
        if (size == exchangeSize) {
            if (seq == sequenceNumber || seq == 15) {
                gotCompositeData = 1;
                ret = exchangeBuffer + exchangeSize;
            } else {
                fn_000c20c0("  WARNING network: TwoPlayerCallback() Unexpected sequence number: Got %d, expected %d\n", seq, sequenceNumber);
                if (seq > 0)
                    seq--;
                else
                    seq = 14;
                if (seq == sequenceNumber) {
                    if (GetRequestCompositeExchangeDataMsgSize() > 8) {
                        fn_000c20c0("  WARNING network: CompleteTwoPlayerExchange() requestExchangeDataBuffer is too small\n");
                    } else {
                        EncodeRequestExchangeDataMsg(requestExchangeDataBuffer, localPlayerId, sequenceNumber);
                        fn_000c20c0("Requesting resend of seq %d because we received the next one\n", sequenceNumber);
                        if (spPtr->Send(remotePlayerId, requestExchangeDataBuffer) != Lb_OK)
                            fn_000c20c0("  WARNING network: CompleteTwoPlayerExchange() failure on SP::Send()\n");
                    }
                }
            }
        } else {
            fn_000c20c0("  WARNING network: TwoPlayerCallback() invalid length: %d\n", size);
        }
    } else {
        if (size == exchangeSize) {
            ok = 1;
            if (sequenceNumber == 15) {
                sequenceNumber = seq;
                DecodeMessageStub(lastMessage, &msg_size, &msg_type, 0);
                EncodeMessageStub(lastMessage, msg_size, msg_type, sequenceNumber);
            } else if (seq != sequenceNumber) {
                ok = 0;
            }
            if (ok) {
                gotCompositeData = ok;
                ret = exchangeBuffer;
            } else {
                fn_000c20c0("  WARNING network: TwoPlayerCallback() Unexpected sequence number: Got %d, expected %d\n", seq, sequenceNumber);
                if (seq > 0)
                    seq--;
                else
                    seq = 14;
                if (seq == sequenceNumber) {
                    if (GetRequestCompositeExchangeDataMsgSize() > 8) {
                        fn_000c20c0("  WARNING network: CompleteTwoPlayerExchange() requestExchangeDataBuffer is too small\n");
                    } else {
                        EncodeRequestExchangeDataMsg(requestExchangeDataBuffer, localPlayerId, sequenceNumber);
                        fn_000c20c0("Requesting resend of seq %d because we received the next one\n", sequenceNumber);
                        if (spPtr->Send(remotePlayerId, requestExchangeDataBuffer) != Lb_OK)
                            fn_000c20c0("  WARNING network: CompleteTwoPlayerExchange() failure on SP::Send()\n");
                    }
                }
            }
        } else {
            fn_000c20c0("  WARNING network: TwoPlayerCallback() invalid length: %d\n", size);
        }
    }
    return ret;
}

// Two-player provider callback: the other player asks again for packet
// seq; resends the last or last-but-one packet sent.
// FUNCTION: 0xd32fc
void TwoPlayerReqExDataMsgCallback(u32 id, u32 seq, void *data)
{
    u8 *msg = 0;
    u32 msg_seq;

    fn_000c20c0("Received Resend Request for seq no %d\n", seq);
    if (sequenceNumber == 15) {
        msg = lastMessage;
    } else {
        DecodeMessageStub(lastMessage, 0, 0, &msg_seq);
        if (seq == msg_seq)
            msg = lastMessage;
        DecodeMessageStub(lastButOneMessage, 0, 0, &msg_seq);
        if (seq == msg_seq)
            msg = lastButOneMessage;
    }
    if (msg == 0) {
        fn_000c20c0("  WARNING network: TwoPlayerReqExDataMsgCallback() could not find requested message()\n");
    } else if (spPtr->Send(id, msg) == Lb_OK) {
        if (msg == lastButOneMessage) {
            spPtr->Send(id, lastMessage);
            if (seq == 14)
                fn_000c20c0("Resending lastMessage for seq no %d as well\n", 0);
            else
                fn_000c20c0("Resending lastMessage for seq no %d as well\n", seq + 1);
        }
    } else {
        fn_000c20c0("  WARNING network: TwoPlayerReqExDataMsgCallback() failure on SP::Send()\n");
    }
    startTime = LbTimerClock();
}

// Creates and initialises the modem service provider with the settings in
// init_data, switching to the two-player exchange. Returns Lb_OK, or
// Lb_FAIL.
// FUNCTION: 0xd33e0
TbError GenericModemInit(__tibcaGUID guid, void *init_data)
{
    __4dn7ojSerialInitData *serial = (__4dn7ojSerialInitData *)init_data;
    TbError ret = Lb_FAIL;

    if (spPtr) {
        spPtr->Release();
        delete spPtr;
        spPtr = 0;
    }
    runningTwoPlayerModel = 1;
    receiveCallbacks.multiPlayer = (void *(__stdcall *)(u32, u32, u32, void *))TwoPlayerCallback;
    receiveCallbacks.mpReqExDataMsg = (void (__stdcall *)(u32, u32, void *))TwoPlayerReqExDataMsgCallback;
    if (init_data)
        serial->mode = 2;
    spPtr = new ModemSP(serial);
    if (spPtr) {
        if (spPtr->Init(guid, 0, &receiveCallbacks, 0) == Lb_OK)
            ret = Lb_OK;
        else
            fn_000c20c0("  WARNING network: GenericModemInit() failure on ServiceProvider::Init()\n");
    }
    return ret;
}

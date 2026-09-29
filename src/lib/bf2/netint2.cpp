#include "net.h"

#define ABS(x) ((x) < 0 ? -(x) : (x))

// FUNCTION: 0xd2ba0
// NONMATCHING: callee and callback relocations carry the target's misnamed symbols; register allocation differs
TbError GenericSerialInit(__tibcaGUID guid, void *init_data)
{
    TbError ret = Lb_FAIL;

    if (spPtr) {
        spPtr->Release();
        delete spPtr;
        spPtr = 0;
    }
    runningTwoPlayerModel = 1;
    receiveCallbacks.multiPlayer = (void *(__stdcall *)(u32, u32, u32, void *))TwoPlayerMultiPlayerCallback;
    receiveCallbacks.mpReqExDataMsg = (void (__stdcall *)(u32, u32, void *))TwoPlayerReqExDataCallback;
    if (init_data)
        ((u32 *)init_data)[3] = 1;
    spPtr = new SerialSP((__4dn7ojSerialInitData *)init_data);
    if (spPtr) {
        if (spPtr->Init(guid, 0, &receiveCallbacks, 0) == Lb_OK)
            ret = Lb_OK;
        else
            fn_000c20c0(D_001257f0);
    }
    return ret;
}

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
            fn_000c20c0(D_0012583b);
        }
    } else {
        ret = Lb_OK;
    }
    return ret;
}

// FUNCTION: 0xd2e40
// NONMATCHING: ours keeps requestExchangeDataBuffer in ebp where the target keeps the Receive argument 8; timeCount compare reads through registers
TbError CompleteTwoPlayerExchange(void *buf)
{
    TbError ret = Lb_FAIL;
    u32 keep_going = 1;
    u32 retries = 0;
    u32 elapsed;

    if (!clientDataTable[remotePlayerIndex].active)
        return Lb_OK;
    while (keep_going) {
        spPtr->Receive(8);
        if (gotCompositeData) {
            keep_going = 0;
            if (retries == 0) {
                elapsed = LbTimerClock() - startTime;
                elapsed = ABS((s32)elapsed);
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
            elapsed = ABS((s32)elapsed);
            if (elapsed > actualTimeout) {
                fn_000c20c0(D_0012587e, retries, sequenceNumber, actualTimeout);
                retries++;
                fn_000c20c0(D_001258a9, retries);
                if (retries > 10) {
                    if (GetRequestCompositeExchangeDataMsgSize() <= 8) {
                        EncodeDeletePlayerMsg(deletePlayerBuffer, remotePlayerId);
                        if (spPtr->Send(localPlayerId, deletePlayerBuffer) == Lb_OK)
                            fn_000c20c0(D_00125907);
                        else
                            fn_000c20c0(D_00125923);
                        return ret;
                    }
                    fn_000c20c0(D_001258b7);
                } else if (GetRequestCompositeExchangeDataMsgSize() > 8) {
                    fn_000c20c0(D_00125969);
                } else {
                    EncodeRequestExchangeDataMsg(requestExchangeDataBuffer, localPlayerId, sequenceNumber);
                    fn_000c20c0(D_001259c0, sequenceNumber);
                    if (spPtr->Send(remotePlayerId, requestExchangeDataBuffer) != Lb_OK)
                        fn_000c20c0(D_001259e2);
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

// FUNCTION: 0xd33e0
// NONMATCHING: callee and callback relocations carry the target's misnamed symbols; register allocation differs
TbError GenericModemInit(__tibcaGUID guid, void *init_data)
{
    TbError ret = Lb_FAIL;

    if (spPtr) {
        spPtr->Release();
        delete spPtr;
        spPtr = 0;
    }
    runningTwoPlayerModel = 1;
    receiveCallbacks.multiPlayer = (void *(__stdcall *)(u32, u32, u32, void *))TwoPlayerMultiPlayerCallback;
    receiveCallbacks.mpReqExDataMsg = (void (__stdcall *)(u32, u32, void *))TwoPlayerReqExDataCallback;
    if (init_data)
        ((u32 *)init_data)[3] = 2;
    spPtr = new ModemSP((__4dn7ojSerialInitData *)init_data);
    if (spPtr) {
        if (spPtr->Init(guid, 0, &receiveCallbacks, 0) == Lb_OK)
            ret = Lb_OK;
        else
            fn_000c20c0(D_00125e26);
    }
    return ret;
}

extern "C" {
extern char D_00125d04[];
extern char D_00125d2b[];
extern char D_00125d82[];
extern char D_00125daf[];
extern char D_00125ddc[];
}

extern "C" {
extern char D_00125a28[];
extern char D_00125a80[];
extern char D_00125ad7[];
extern char D_00125b15[];
extern char D_00125b5b[];
extern char D_00125b96[];
extern char D_00125bee[];
extern char D_00125c45[];
extern char D_00125c83[];
extern char D_00125cc9[];
}

// NONMATCHING: sequenceNumber cached in eax instead of ebx (size stays live in eax in the target)
// FUNCTION: 0xd309c
void *TwoPlayerMultiPlayerCallback(u32 id, u32 size, u32 seq, void *data)
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
                fn_000c20c0(D_00125a28, seq, sequenceNumber);
                if (seq > 0)
                    seq--;
                else
                    seq = 14;
                if (seq == sequenceNumber) {
                    if (GetRequestCompositeExchangeDataMsgSize() > 8) {
                        fn_000c20c0(D_00125a80);
                    } else {
                        EncodeRequestExchangeDataMsg(requestExchangeDataBuffer, localPlayerId, sequenceNumber);
                        fn_000c20c0(D_00125ad7, sequenceNumber);
                        if (spPtr->Send(remotePlayerId, requestExchangeDataBuffer) != Lb_OK)
                            fn_000c20c0(D_00125b15);
                    }
                }
            }
        } else {
            fn_000c20c0(D_00125b5b, size);
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
                gotCompositeData = 1;
                ret = exchangeBuffer;
            } else {
                fn_000c20c0(D_00125b96, seq, sequenceNumber);
                if (seq > 0)
                    seq--;
                else
                    seq = 14;
                if (seq == sequenceNumber) {
                    if (GetRequestCompositeExchangeDataMsgSize() > 8) {
                        fn_000c20c0(D_00125bee);
                    } else {
                        EncodeRequestExchangeDataMsg(requestExchangeDataBuffer, localPlayerId, sequenceNumber);
                        fn_000c20c0(D_00125c45, sequenceNumber);
                        if (spPtr->Send(remotePlayerId, requestExchangeDataBuffer) != Lb_OK)
                            fn_000c20c0(D_00125c83);
                    }
                }
            }
        } else {
            fn_000c20c0(D_00125cc9, size);
        }
    }
    return ret;
}

// FUNCTION: 0xd32fc
void TwoPlayerReqExDataCallback(u32 id, u32 seq, void *data)
{
    u8 *msg = 0;
    u32 msg_seq;

    fn_000c20c0(D_00125d04, seq);
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
        fn_000c20c0(D_00125d2b);
    } else if (spPtr->Send(id, msg) == Lb_OK) {
        if (msg == lastButOneMessage) {
            spPtr->Send(id, lastMessage);
            if (seq == 14)
                fn_000c20c0(D_00125d82, 0);
            else
                fn_000c20c0(D_00125daf, seq + 1);
        }
    } else {
        fn_000c20c0(D_00125ddc);
    }
    startTime = LbTimerClock();
}

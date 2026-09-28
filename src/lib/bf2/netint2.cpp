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
    receiveCallbacks.multiPlayer = TwoPlayerMultiPlayerCallback;
    receiveCallbacks.mpReqExDataMsg = TwoPlayerReqExDataCallback;
    if (init_data)
        ((u32 *)init_data)[3] = 1;
    spPtr = new SerialSP(init_data);
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
// NONMATCHING: range also holds the two two-player receive callbacks
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
    receiveCallbacks.multiPlayer = TwoPlayerMultiPlayerCallback;
    receiveCallbacks.mpReqExDataMsg = TwoPlayerReqExDataCallback;
    if (init_data)
        ((u32 *)init_data)[3] = 2;
    spPtr = new ModemSP(init_data);
    if (spPtr) {
        if (spPtr->Init(guid, 0, &receiveCallbacks, 0) == Lb_OK)
            ret = Lb_OK;
        else
            fn_000c20c0(D_00125e26);
    }
    return ret;
}

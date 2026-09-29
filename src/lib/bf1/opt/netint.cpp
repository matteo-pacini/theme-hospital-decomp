#include "net.h"

struct IPXHandler {
    u8 unknown0[0x4a];

    IPXHandler();
    ~IPXHandler();
};

struct IPXServiceProvider : ServiceProvider {
    IPXHandler ipx;
    u8 unknownb4a[0x11e2 - 0xb4a];
    TbNetworkSessionNameEntry entry;
    u8 unknown120b[0x155e - 0x120b];

    IPXServiceProvider();
    ~IPXServiceProvider();
    TbError Start(TbNetworkSessionNameEntry *session, char *name, void *data);
    TbError Start(char *session, char *name, u32 max_players, void *data);
    TbError Stop(void);
    TbError Enumerate(TbNetworkCallbackFunc callback, void *data);
    TbError Enumerate(TbNetworkSessionNameEntry *session, TbNetworkCallbackFunc callback, void *data);
    TbError Init(__tibcaGUID guid, __tibcaGUID *pguid, __ic76sReceiveCallbacks *callbacks, void *data);
    TbError Release(void);
    TbError ChangeSettings(u32 setting, void *data);
    TbError EnableNewPlayers(u32 allow);
    u32 ReadMessage(u32 *from, void *buf, u32 *size);
    u32 PeekMessage(u32 *from, void *buf, u32 *size);
    TbError SendMessage(u32 to, void *buf, TbMessageType type);
};

struct __ic76sReceiveCallbacks {
    void (__stdcall *addMsg)(u32 id, char *name, void *data);
    void (__stdcall *deleteMsg)(u32 id, void *data);
    void (__stdcall *hostMsg)(u32 id, void *data);
    void (__stdcall *unknownc)(void);
    void *(__stdcall *multiPlayer)(u32 id, u32 size, u32 seq, void *data);
    void (__stdcall *mpReqExDataMsg)(u32 id, u32 seq, void *data);
    void (__stdcall *mpReqCompsExDataMsg)(u32 id, u32 seq, void *data);
    void *(__stdcall *unidirectionalMsg)(u32 id, u32 size, void *data);
    void (__stdcall *systemUserMsg)(u32 id, void *msg, u32 size, void *data);
};

extern u32 exchangeTimeout;
extern u32 runningTwoPlayerModel;
extern u32 localPlayerId;
extern u32 localPlayerIndex;
extern u32 remotePlayerId;
extern u32 remotePlayerIndex;
extern u32 hostId;
extern u32 sequenceNumber;
extern u32 waitingForPlayerMapResponse;
extern u32 gotCompositeData;
extern u32 exchangeSize;
extern u8 *exchangeBuffer;
extern void *localDataPtr;
extern void *compositeBuffer;
extern u32 startTime;
extern u32 actualTimeout;
extern u32 basicTimeout;
extern u32 timeCount;
extern u32 maxTime;
extern __7308jnTbNetworkPlayerInfo *localPlayerInfoPtr;
extern __ic76sReceiveCallbacks receiveCallbacks;
extern u8 deletePlayerBuffer[];
extern u8 requestExchangeDataBuffer[];
extern u8 requestCompositeExchangeDataBuffer[];
extern u8 systemUserBuffer[];
struct __2unp93UnidirectionalDataMessage;
extern __2unp93UnidirectionalDataMessage incomingUnidirectionalMessage;
extern u32 unidirectionalMsgReceived;

TbError GenericSerialInit(__tibcaGUID guid, void *init_data);
TbError GenericModemInit(__tibcaGUID guid, void *init_data);
TbError GenericIPXInit(__tibcaGUID guid);
TbError StartTwoPlayerExchange(void *buf);
TbError CompleteTwoPlayerExchange(void *buf);
u32 GetRequestCompositeExchangeDataMsgSize(void);
void EncodeDeletePlayerMsg(u8 *buf, u32 id);
void EncodeRequestExchangeDataMsg(u8 *buf, u32 id, u32 seq);
void EncodeRequestCompositeExchangeDataMsg(u8 *buf, u32 id, u32 seq);
// The original does not inline HostDataCollection or HostDataBroadcast into
// CompleteMultiPlayerExchange although -oe=400 would; binding them to their symbols
// with #pragma aux keeps them out of line. Why the original did not inline them is unknown.
TbError HostDataCollection(void);
#pragma aux HostDataCollection "W?HostDataCollection$n()$TbError$$"
TbError LbNetwork_Init(u32 index, __tibcaGUID guid, u32 max_players, void *exchange_buf, u32 exchange_size,
                       __7308jnTbNetworkPlayerInfo *info, void *init_data);

extern "C" {
extern u32 D_00188cb8;
extern u32 D_0014da60;
extern char D_00121eb4[];
extern char D_00121eb8[];
extern char D_00121ef4[];
extern char D_00121f15[];
extern char D_00121f3a[];
extern char D_00121f41[];
extern char D_00121f47[];
extern char D_00121f4b[];
extern char D_00121f79[];
extern char D_00121fc4[];
extern char D_00121fc5[];
extern char D_00121fde[];
extern char D_00122022[];
extern char D_0012203a[];
extern char D_0012207d[];
extern char D_00122093[];
extern char D_001220db[];
extern char D_0012211c[];
extern char D_0012214a[];
extern char D_0012218f[];
extern char D_001221df[];
extern char D_00122225[];
extern char D_001222ad[];
extern char D_001222f1[];
extern char D_00122326[];
extern char D_0012235e[];
extern char D_001223a5[];
extern char D_001223eb[];
extern char D_0012242f[];
extern char D_0012247f[];
extern char D_001224d0[];
extern char D_00122517[];
extern char D_00122532[];
extern char D_00122583[];
extern char D_001225c2[];
extern char D_00122623[];
extern char D_0012266a[];
extern char D_001226bb[];
extern char D_00122709[];
extern char D_0012275b[];
extern char D_001227ad[];
extern char D_001227f6[];
extern char D_00122843[];
extern char D_00122860[];
extern char D_001228a9[];
extern char D_001228e8[];
extern char D_00122938[];
extern char D_00122977[];
extern char D_00122c70[];
extern char D_001229be[];
extern char D_00122a02[];
extern char D_00122a5c[];
extern char D_00122a95[];
extern char D_00122ad2[];
extern char D_00122b2c[];
extern char D_00122b65[];
extern char D_00122ba5[];
extern char D_00122bfa[];
extern char D_00122c33[];
extern char D_00122263[];
extern char D_00122ea6[];
extern char D_00122ee2[];
extern char D_00122f32[];
extern char D_00122cb0[];
extern char D_00122ced[];
extern char D_00122d34[];
extern char D_00122d7f[];
extern char D_00122de4[];
extern char D_00122e38[];

}

void *__stdcall MultiPlayerCallback(u32 id, u32 size, u32 seq, void *data);
void __stdcall AddMsgCallback(u32 id, char *name, void *data);
void __stdcall DeleteMsgCallback(u32 id, void *data);
void *__stdcall UnidirectionalMsgCallback(u32 id, u32 size, void *data);
void __stdcall MultiPlayerReqExDataMsgCallback(u32 id, u32 seq, void *data);
void __stdcall RequestCompositeExchangeDataMsgCallback(u32 id, u32 seq, void *data);
TbError GetCurrentPlayers(void);
TbError HostDataBroadcast(void);
#pragma aux HostDataBroadcast "W?HostDataBroadcast$n()$TbError$$"
TbError SendSystemUserMessage(u32 id, TbMessageType type, void *buf, u32 size);
u32 IdToIndex(u32 &id);

extern "C" {

void fn_000bf7a0(u32 timeout);
#pragma aux fn_000bf7a0 "W?LbNetwork_ChangeExchangeTimeout$n(ul)v"
TbError fn_000c197c(TbNetworkPlayerEntry *player);
#pragma aux fn_000c197c "W?AddAPlayer$n(pn$TbNetworkPlayerNameEntry$$)$TbError$$"
u32 fn_000cd716(char *s);
#pragma aux fn_000cd716 "fn_000cd716_"

}

// FUNCTION: 0x000bf790
void *operator new(unsigned size)
{
    return LbMemoryAlloc(size);
}

// FUNCTION: 0x000bf798
void operator delete(void *p)
{
    LbMemoryFree(p);
}

extern "C" {

// FUNCTION: 0x000bf7a0
void fn_000bf7a0(u32 timeout)
{
    exchangeTimeout = timeout * 1000;
}

}

// FUNCTION: 0x000bf7b8
void LbNetwork_LibraryVersion(__ctcl4TbNetworkVersion *version)
{
    char os[0x28];

    if (version) {
        strcpy(os, D_00121eb4);
        sprintf(version->text, D_00121eb8, 1.1, os);
        version->version = 1.1f;
    }
}

extern "C" {

// FUNCTION: 0x000bf810
TbError fn_000bf810(u32 a, u32 b, u32 c, u32 d, u32 e)
{
    return Lb_FAIL;
}

}

void __stdcall GetCurrentPlayersCallback(TbNetworkCallbackData *data, void *arg);

// FUNCTION: 0x000c18d0
TbError GetCurrentPlayers(void)
{
    TbError ret;

    ret = Lb_FAIL;
    localPlayerIndex = maximumPlayers;
    if (spPtr->Enumerate(0, GetCurrentPlayersCallback, 0) == Lb_OK) {
        if (localPlayerIndex < maximumPlayers)
            ret = Lb_OK;
        else
            fn_000c20c0(D_0012214a);
    } else {
        fn_000c20c0(D_0012218f);
    }
    return ret;
}

// FUNCTION: 0x000bf818
TbError LbNetwork_EnableNewPlayers(u32 allow)
{
    TbError ret;

    ret = Lb_FAIL;
    if (spPtr) {
        if (allow)
            fn_000c20c0(D_00121ef4);
        else
            fn_000c20c0(D_00121f15);
        ret = spPtr->EnableNewPlayers(allow);
    }
    return ret;
}

struct ServiceEntry {
    u8 flags;
    char name[0x43];
};

// FUNCTION: 0x000bf854
TbError LbNetwork_EnumerateServices(TbNetworkCallbackFunc callback, void *data)
{
    ServiceEntry service;

    strcpy(service.name, D_00121f3a);
    callback((TbNetworkCallbackData *)&service, data);
    strcpy(service.name, D_00121f41);
    callback((TbNetworkCallbackData *)&service, data);
    strcpy(service.name, D_00121f47);
    callback((TbNetworkCallbackData *)&service, data);
    return Lb_OK;
}

// NONMATCHING: the target calls LbNetwork_Init(u32, ...); with -oe=400 (needed to inline
// CompleteMultiPlayerExchange into LbNetwork_CompleteExchange) it is inlined here
// FUNCTION: 0x000bfac0
TbError LbNetwork_Init(TbNetworkService service, __tibcaGUID guid, u32 max_players, void *exchange_buf,
                       u32 exchange_size, __7308jnTbNetworkPlayerInfo *info, void *init_data)
{
    TbError ret;
    u32 index;

    ret = Lb_OK;
    switch (service) {
    case (TbNetworkService)1:
    case (TbNetworkService)6:
        index = 2;
        break;
    case (TbNetworkService)2:
    case (TbNetworkService)5:
        index = 1;
        break;
    case (TbNetworkService)3:
    case (TbNetworkService)4:
        index = 0;
        break;
    default:
        fn_000c20c0(D_00121f4b, (s32)service);
        ret = Lb_FAIL;
        break;
    }
    if (ret == Lb_OK)
        ret = LbNetwork_Init(index, guid, max_players, exchange_buf, exchange_size, info, init_data);
    return ret;
}

// FUNCTION: 0x000bfc4c
TbError LbNetwork_Init(u32 index, __tibcaGUID guid, u32 max_players, void *exchange_buf,
                       u32 exchange_size, __7308jnTbNetworkPlayerInfo *info, void *init_data)
{
    TbError ret;
    TbError sp_ret;
    __tibcaGUID sp_guid;
    u32 i;

    ret = Lb_FAIL;
    exchangeSize = exchange_size;
    maximumPlayers = max_players;
    basicTimeout = 250;
    localDataPtr = 0;
    compositeBuffer = 0;
    sequenceNumber = 0;
    timeCount = 0;
    maxTime = 0;
    runningTwoPlayerModel = 0;
    waitingForPlayerMapResponse = 0;
    D_00188cb8 = 0;
    exchangeBuffer = (u8 *)exchange_buf;
    localPlayerInfoPtr = info;
    receiveCallbacks.multiPlayer = MultiPlayerCallback;
    D_0014da60 = 0;
    receiveCallbacks.mpReqExDataMsg = MultiPlayerReqExDataMsgCallback;
    if (exchange_size > 0) {
        D_00188cb8 = max_players * exchange_size;
        compositeBuffer = LbMemoryAlloc(D_00188cb8);
        if (!compositeBuffer) {
            fn_000c20c0(D_00121f79);
            return ret;
        }
    }
    for (i = 0; i < maximumPlayers; i++)
        clientDataTable[i].active = 0;
    for (i = 0; i < maximumPlayers; i++) {
        localPlayerInfoPtr->active = 0;
        strcpy(localPlayerInfoPtr->name, D_00121fc4);
    }
    switch (index) {
    case 0:
        fn_000c20c0(D_00121fc5);
        if (GenericSerialInit(guid, init_data) == Lb_OK)
            ret = Lb_OK;
        else
            fn_000c20c0(D_00121fde);
        break;
    case 1:
        fn_000c20c0(D_00122022);
        if (GenericModemInit(guid, init_data) == Lb_OK)
            ret = Lb_OK;
        else
            fn_000c20c0(D_0012203a);
        break;
    case 2:
        fn_000c20c0(D_0012207d);
        if (GenericIPXInit(guid) == Lb_OK)
            ret = Lb_OK;
        else
            fn_000c20c0(D_001220db);
        break;
    default:
        fn_000c20c0(D_0012211c, index);
        break;
    }
    return ret;
}

// NONMATCHING: register choice
// FUNCTION: 0x000bff98
TbError LbNetwork_Create(char *session, char *name, u32 *player, void *data)
{
    u32 i;
    TbError ret;

    ret = Lb_FAIL;
    if (runningTwoPlayerModel) {
        localPlayerId = 0;
        localPlayerIndex = 0;
        remotePlayerId = 1;
        remotePlayerIndex = 1;
    }
    if (spPtr->Start(session, name, maximumPlayers, data) == Lb_OK) {
        if (GetCurrentPlayers() == Lb_OK) {
            *player = localPlayerIndex;
            ret = Lb_OK;
            for (i = 0; i < maximumPlayers; i++) {
                if (clientDataTable[i].active) {
                    localPlayerInfoPtr[i].active = 1;
                    strcpy(localPlayerInfoPtr[i].name, clientDataTable[i].name);
                } else {
                    localPlayerInfoPtr[i].active = clientDataTable[i].active;
                }
            }
            LbNetwork_EnableNewPlayers(1);
        } else {
            fn_000c20c0(D_001221df);
        }
    } else {
        fn_000c20c0(D_00122225);
    }
    return ret;
}

// NONMATCHING: register choice
// FUNCTION: 0x000c0110
TbError LbNetwork_Join(TbNetworkSessionNameEntry *session, char *name, u32 *player, void *data)
{
    TbError ret;
    u32 start;
    u32 i;

    ret = Lb_FAIL;
    start = LbTimerClock();
    if (runningTwoPlayerModel) {
        remotePlayerId = 0;
        remotePlayerIndex = 0;
        localPlayerId = 1;
        localPlayerIndex = 1;
    }
    sequenceNumber = 15;
    if (spPtr->Start(session, name, data) == Lb_OK) {
        if (!runningTwoPlayerModel) {
            SendSystemUserMessage(0, MSG_TYPE_0, 0, 0);
            waitingForPlayerMapResponse = 1;
            do {
                spPtr->Receive(8);
                if (waitingForPlayerMapResponse && LbTimerClock() - start > 10000) {
                    waitingForPlayerMapResponse = 0;
                    return ret;
                }
            } while (waitingForPlayerMapResponse);
        }
        if (GetCurrentPlayers() == Lb_OK) {
            *player = localPlayerIndex;
            ret = Lb_OK;
            for (i = 0; i < maximumPlayers; i++) {
                if (clientDataTable[i].active) {
                    localPlayerInfoPtr[i].active = 1;
                    strcpy(localPlayerInfoPtr[i].name, clientDataTable[i].name);
                } else {
                    localPlayerInfoPtr[i].active = clientDataTable[i].active;
                }
            }
        } else {
            fn_000c20c0(D_001222ad);
        }
    } else {
        fn_000c20c0(D_001222f1);
    }
    return ret;
}

// FUNCTION: 0x000c02fc
TbError LbNetwork_Stop(void)
{
    TbError ret;

    ret = Lb_FAIL;
    if (spPtr) {
        if (spPtr->Release() == Lb_OK)
            ret = Lb_OK;
        else
            fn_000c20c0(D_00122326);
        delete spPtr;
        spPtr = 0;
    }
    if (compositeBuffer) {
        LbMemoryFree(compositeBuffer);
        compositeBuffer = 0;
    }
    return ret;
}

// FUNCTION: 0x000c036c
TbError LbNetwork_EnumerateSessions(TbNetworkCallbackFunc callback, void *data)
{
    TbError ret;

    ret = Lb_FAIL;
    if (!spPtr)
        return Lb_OK;
    if (spPtr->Enumerate(callback, data) == Lb_OK)
        ret = Lb_OK;
    else
        fn_000c20c0(D_0012235e);
    return ret;
}

// FUNCTION: 0x000c03b8
TbError LbNetwork_EnumeratePlayers(TbNetworkSessionNameEntry *session, TbNetworkCallbackFunc callback, void *data)
{
    TbError ret;

    ret = Lb_FAIL;
    if (spPtr->Enumerate(session, callback, data) == Lb_OK)
        ret = Lb_OK;
    else
        fn_000c20c0(D_001223a5);
    return ret;
}

TbError StartMultiPlayerExchange(void *buf);

// FUNCTION: 0x000c03f8
TbError LbNetwork_StartExchange(void *buf)
{
    if (runningTwoPlayerModel)
        return StartTwoPlayerExchange(buf);
    return StartMultiPlayerExchange(buf);
}

// FUNCTION: 0x000c0524
TbError StartMultiPlayerExchange(void *buf)
{
    TbError ret;
    u32 i;

    ret = Lb_FAIL;
    localDataPtr = buf;
    spPtr->Receive(6);
    for (i = 0; i < maximumPlayers; i++) {
        if (clientDataTable[i].active)
            clientDataTable[i].got_data = 0;
    }
    memcpy(exchangeBuffer + localPlayerIndex * exchangeSize, buf, exchangeSize);
    clientDataTable[localPlayerIndex].got_data = 1;
    startTime = LbTimerClock();
    actualTimeout = basicTimeout;
    if (localPlayerId == hostId) {
        ret = Lb_OK;
    } else {
        EncodeMessageStub(buf, exchangeSize - 4, MSG_TYPE_0, sequenceNumber);
        if (spPtr->Send(hostId, buf) == Lb_OK)
            ret = Lb_OK;
        else
            fn_000c20c0(D_001223eb);
    }
    return ret;
}

TbError CompleteMultiPlayerExchange(void *buf);

// FUNCTION: 0x000c0640
// NONMATCHING: register choice in the inlined timeout bookkeeping, as in CompleteMultiPlayerExchange
TbError LbNetwork_CompleteExchange(void *buf)
{
    if (runningTwoPlayerModel)
        return CompleteTwoPlayerExchange(buf);
    return CompleteMultiPlayerExchange(buf);
}

// FUNCTION: 0x000c08e4
// NONMATCHING: register choice: the target computes the elapsed time in ebp and reloads timeCount
TbError CompleteMultiPlayerExchange(void *buf)
{
    TbError ret;
    u32 keep_going;
    u32 retries;
    u32 host_changed;
    u32 old_host;
    u32 elapsed;

    ret = Lb_FAIL;
    if (localPlayerId == hostId) {
        HostDataCollection();
        ret = HostDataBroadcast();
    } else {
        keep_going = 1;
        retries = 0;
        host_changed = 0;
        gotCompositeData = 0;
        do {
            old_host = hostId;
            spPtr->Receive(8);
            if (old_host != hostId)
                host_changed = 1;
            if (!host_changed) {
                if (gotCompositeData) {
                    if (!retries) {
                        elapsed = LbTimerClock() - startTime;
                        if (!timeCount || elapsed > maxTime)
                            maxTime = elapsed;
                        if (++timeCount >= 50) {
                            timeCount = 0;
                            basicTimeout = maxTime * 4;
                            if (basicTimeout < 250)
                                basicTimeout = 250;
                        }
                    }
                    ret = Lb_OK;
                    keep_going = 0;
                }
            } else {
                if (hostId == localPlayerId) {
                    if (HostDataCollection() == Lb_OK) {
                        if (HostDataBroadcast() == Lb_OK)
                            ret = Lb_OK;
                        else
                            fn_000c20c0(D_0012242f);
                    } else {
                        fn_000c20c0(D_0012247f);
                    }
                    keep_going = 0;
                } else {
                    EncodeMessageStub(buf, exchangeSize - 4, MSG_TYPE_0, sequenceNumber);
                    if (spPtr->Send(hostId, buf) == Lb_OK)
                        ret = Lb_OK;
                    else
                        fn_000c20c0(D_001224d0);
                }
                host_changed = 0;
            }
            if (keep_going && LbTimerClock() - startTime > actualTimeout) {
                fn_000c20c0(D_00122517);
                retries++;
                if (retries > 10) {
                    if (GetRequestCompositeExchangeDataMsgSize() > 8) {
                        fn_000c20c0(D_00122532);
                    } else {
                        EncodeDeletePlayerMsg(deletePlayerBuffer, hostId);
                        if (spPtr->Send(localPlayerId, deletePlayerBuffer) != Lb_OK)
                            fn_000c20c0(D_00122583);
                    }
                    host_changed = 1;
                } else if (GetRequestCompositeExchangeDataMsgSize() > 8) {
                    fn_000c20c0(D_001225c2);
                } else {
                    EncodeRequestCompositeExchangeDataMsg(requestCompositeExchangeDataBuffer, localPlayerId, sequenceNumber);
                    if (spPtr->Send(hostId, requestCompositeExchangeDataBuffer) != Lb_OK)
                        fn_000c20c0(D_00122623);
                }
                startTime = LbTimerClock();
                actualTimeout = basicTimeout * (retries + 1);
                basicTimeout += 100;
            }
        } while (keep_going);
    }
    localDataPtr = 0;
    if (sequenceNumber != 15) {
        sequenceNumber++;
        if (sequenceNumber >= 15)
            sequenceNumber = 0;
    }
    return ret;
}

// FUNCTION: 0x000c0b74
// NONMATCHING: register choice in the inlined exchange code
TbError LbNetwork_Exchange(void *buf)
{
    TbError ret;

    ret = Lb_FAIL;
    if (LbNetwork_StartExchange(buf) != Lb_OK) {
        fn_000c20c0(D_001226bb);
        return ret;
    }
    if (LbNetwork_CompleteExchange(buf) == Lb_OK)
        ret = Lb_OK;
    else
        fn_000c20c0(D_0012266a);
    return ret;
}

// FUNCTION: 0x000c0f68
// NONMATCHING: the target keeps ret in bl throughout; here the failure returns are folded to constants
TbError LbNetwork_ChangeExchangeBuffer(void *buf, u32 size)
{
    TbError ret;
    u32 total;
    void *new_buf;

    total = maximumPlayers * size;
    ret = Lb_FAIL;
    exchangeBuffer = (u8 *)buf;
    exchangeSize = size;
    if (compositeBuffer == 0) {
        compositeBuffer = LbMemoryAlloc(total);
        if (!compositeBuffer) {
            fn_000c20c0(D_00122709);
            return ret;
        }
        D_00188cb8 = total;
    } else if (total > D_00188cb8) {
        new_buf = LbMemoryAlloc(total);
        if (!new_buf) {
            fn_000c20c0(D_0012275b);
            return ret;
        }
        memcpy(new_buf, compositeBuffer, D_00188cb8);
        ret = Lb_OK;
        LbMemoryFree(compositeBuffer);
        compositeBuffer = new_buf;
        D_00188cb8 = total;
        return ret;
    }
    return Lb_OK;
}

TbError GetPlayerInfo(void);

// FUNCTION: 0x000c1028
// NONMATCHING: stack slots of the return values of the two inlined calls, and register use in the
// inlined GetPlayerInfo loop
TbError LbNetwork_GetPlayerInfo(void)
{
    TbError ret;

    ret = Lb_FAIL;
    if (GetCurrentPlayers() == Lb_OK) {
        if (GetPlayerInfo() == Lb_OK)
            ret = Lb_OK;
        else
            fn_000c20c0(D_001227ad);
    } else {
        fn_000c20c0(D_001227f6);
    }
    return ret;
}

// FUNCTION: 0xc1130
TbError GetPlayerInfo(void)
{
    TbError ret = Lb_OK;
    u32 i;

    for (i = 0; i < maximumPlayers; i++) {
        if (clientDataTable[i].active) {
            localPlayerInfoPtr[i].active = 1;
            strcpy(localPlayerInfoPtr[i].name, clientDataTable[i].name);
        } else {
            localPlayerInfoPtr[i].active = clientDataTable[i].active;
        }
    }
    return ret;
}

// NONMATCHING: register choice and block layout
// FUNCTION: 0x000c11b8
TbError HostDataCollection(void)
{
    TbError ret;
    u32 keep_going;
    u32 retries;
    u32 all_got;
    u32 elapsed;
    u32 i;
    u32 j;

    keep_going = 1;
    ret = Lb_FAIL;
    retries = 0;
    do {
        all_got = 1;
        for (i = 0; i < maximumPlayers; i++) {
            if (clientDataTable[i].active && !clientDataTable[i].got_data)
                all_got = clientDataTable[i].got_data;
        }
        if (all_got) {
            keep_going = 0;
            if (!retries) {
                elapsed = LbTimerClock() - startTime;
                if (!timeCount || elapsed > maxTime)
                    maxTime = elapsed;
                if (++timeCount >= 50) {
                    timeCount = 0;
                    basicTimeout = maxTime * 4;
                    if (basicTimeout < 250)
                        basicTimeout = 250;
                }
            }
            ret = Lb_OK;
        } else {
            if (LbTimerClock() - startTime > actualTimeout) {
                fn_000c20c0(D_00122843);
                retries++;
                if (retries > 10) {
                    if (GetRequestCompositeExchangeDataMsgSize() > 8) {
                        fn_000c20c0(D_00122860);
                    } else {
                        for (i = 0; i < maximumPlayers; i++) {
                            if (clientDataTable[i].active && !clientDataTable[i].got_data) {
                                EncodeDeletePlayerMsg(deletePlayerBuffer, clientDataTable[i].id);
                                for (j = 0; j < maximumPlayers; j++) {
                                    if (clientDataTable[j].active && clientDataTable[j].id != clientDataTable[i].id
                                        && spPtr->Send(clientDataTable[j].id, deletePlayerBuffer) != Lb_OK)
                                        fn_000c20c0(D_001228a9);
                                }
                            }
                        }
                    }
                } else if (GetRequestCompositeExchangeDataMsgSize() > 8) {
                    fn_000c20c0(D_001228e8);
                } else {
                    EncodeRequestExchangeDataMsg(requestExchangeDataBuffer, hostId, sequenceNumber);
                    for (i = 0; i < maximumPlayers; i++) {
                        if (clientDataTable[i].active && !clientDataTable[i].got_data
                            && spPtr->Send(clientDataTable[i].id, requestExchangeDataBuffer) != Lb_OK)
                            fn_000c20c0(D_00122938);
                    }
                }
                startTime = LbTimerClock();
                actualTimeout = basicTimeout * (retries + 1);
                basicTimeout += 100;
            }
            spPtr->Receive(8);
        }
    } while (keep_going);
    return ret;
}

// FUNCTION: 0x000c1458
// NONMATCHING: the target returns through GetPlayerInfo's epilogue (the module was built without -zm)
TbError HostDataBroadcast(void)
{
    TbError ret;
    u32 i;

    ret = Lb_OK;
    EncodeMessageStub(exchangeBuffer, exchangeSize * maximumPlayers - 4, MSG_TYPE_0, sequenceNumber);
    memcpy(compositeBuffer, exchangeBuffer, exchangeSize * maximumPlayers);
    for (i = 0; i < maximumPlayers; i++) {
        if (clientDataTable[i].active && clientDataTable[i].id != hostId
            && spPtr->Send(clientDataTable[i].id, exchangeBuffer) != Lb_OK) {
            fn_000c20c0(D_00122977);
            ret = Lb_FAIL;
        }
    }
    return ret;
}

// FUNCTION: 0x000c1518
// NONMATCHING: register choice in the inlined IdToIndex loops
void *__stdcall MultiPlayerCallback(u32 id, u32 size, u32 seq, void *data)
{
    void *ret = 0;
    u32 ok;

    if (localPlayerId == hostId) {
        if (size == exchangeSize) {
            if (id == localPlayerId) {
                fn_000c20c0(D_001229be);
            } else if (IdToIndex(id)) {
                if (seq == sequenceNumber || seq == 15) {
                    ret = exchangeBuffer + exchangeSize * id;
                    clientDataTable[id].got_data = 1;
                } else {
                    fn_000c20c0(D_00122a02, seq, sequenceNumber);
                }
            } else {
                fn_000c20c0(D_00122a5c, id);
            }
        } else {
            fn_000c20c0(D_00122a95, size);
        }
    } else if (size == exchangeSize * maximumPlayers) {
        if (id == hostId) {
            if (IdToIndex(id)) {
                ok = 1;
                if (sequenceNumber == 15)
                    sequenceNumber = seq;
                else if (sequenceNumber != seq)
                    ok = 0;
                if (ok) {
                    gotCompositeData = 1;
                    ret = exchangeBuffer;
                } else {
                    fn_000c20c0(D_00122ad2, seq, sequenceNumber);
                }
            } else {
                fn_000c20c0(D_00122b2c, id);
            }
        } else {
            fn_000c20c0(D_00122b65);
        }
    } else if (size == exchangeSize) {
        if (id == localPlayerId) {
            fn_000c20c0(D_00122ba5);
        } else if (IdToIndex(id)) {
            ret = exchangeBuffer + exchangeSize * id;
            clientDataTable[id].got_data = 1;
        } else {
            fn_000c20c0(D_00122bfa, id);
        }
    } else {
        fn_000c20c0(D_00122c33, size);
    }
    return ret;
}

// FUNCTION: 0x000c1810
void __stdcall AddMsgCallback(u32 id, char *name, void *data)
{
    TbNetworkPlayerEntry player;

    player.id = id;
    strcpy(player.name, name);
    player.unknown5 = 0;
    player.host = 0;
    fn_000c197c(&player);
}

// FUNCTION: 0x000c1858
void __stdcall DeleteMsgCallback(u32 id, void *data)
{
    u32 i;

    for (i = 0; i < maximumPlayers; i++) {
        if (clientDataTable[i].active && clientDataTable[i].id == id) {
            clientDataTable[i].active = 0;
            if (!localPlayerInfoPtr)
                fn_000c20c0(D_00122c70);
            else
                localPlayerInfoPtr[i].active = 0;
        }
    }
}

// FUNCTION: 0x000c18c4
void __stdcall HostMsgCallback(u32 id, void *data)
{
    hostId = id;
}

// FUNCTION: 0x000c1934
void __stdcall GetCurrentPlayersCallback(TbNetworkCallbackData *data, void *arg)
{
    fn_000c197c((TbNetworkPlayerEntry *)data);
}

// FUNCTION: 0x000c1940
void *__stdcall UnidirectionalMsgCallback(u32 id, u32 size, void *data)
{
    void *ret = 0;

    if (size > 0x20c) {
        fn_000c20c0(D_00122cb0, size, 0x20c);
    } else {
        ret = &incomingUnidirectionalMessage;
        unidirectionalMsgReceived = 1;
    }
    return ret;
}

extern "C" {

// NONMATCHING: register choice and block layout
// FUNCTION: 0x000c197c
TbError fn_000c197c(TbNetworkPlayerEntry *player)
{
    u32 i;
    u32 found;
    u32 searching;
    u32 index;
    TbError ret;

    ret = Lb_FAIL;
    found = 0;
    if (player) {
        for (i = 0; i < maximumPlayers; i++) {
            if (clientDataTable[i].active && clientDataTable[i].id == player->id) {
                found = 1;
                index = i;
            }
        }
        if (found) {
            ret = Lb_OK;
        } else {
            searching = 1;
            i = 0;
            while (searching) {
                if (clientDataTable[i].active) {
                    if (++i >= maximumPlayers)
                        searching = 0;
                } else {
                    found = 1;
                    searching = 0;
                }
            }
            if (found) {
                clientDataTable[i].id = player->id;
                clientDataTable[i].active = 1;
                index = i;
                strcpy(clientDataTable[i].name, player->name);
                ret = Lb_OK;
                localPlayerInfoPtr[i].active = 1;
                strcpy(localPlayerInfoPtr[i].name, player->name);
            }
        }
        if (ret == Lb_OK) {
            if (player->host)
                hostId = player->id;
            if (player->unknown5) {
                localPlayerId = player->id;
                localPlayerIndex = index;
            }
        }
    }
    return ret;
}

}

// FUNCTION: 0x000c1ad0
u32 IdToIndex(u32 &id)
{
    u32 found = 0;
    u32 more = 1;
    u32 i = 0;

    while (more) {
        if (i >= maximumPlayers) {
            more = 0;
        } else {
            if (clientDataTable[i].active && clientDataTable[i].id == id) {
                found = 1;
                id = i;
            }
            i++;
        }
    }
    return found;
}

// FUNCTION: 0x000c1b24
void __stdcall MultiPlayerReqExDataMsgCallback(u32 id, u32 seq, void *data)
{
    u32 ok;

    if (!localDataPtr) {
        fn_000c20c0(D_00122ced);
        return;
    }
    ok = 1;
    if (sequenceNumber == 15)
        sequenceNumber = seq;
    else if (seq != sequenceNumber)
        ok = 0;
    if (ok) {
        EncodeMessageStub(localDataPtr, exchangeSize - 4, MSG_TYPE_0, sequenceNumber);
        if (spPtr->Send(id, localDataPtr) != Lb_OK)
            fn_000c20c0(D_00122d34);
    } else {
        fn_000c20c0(D_00122d7f, seq, sequenceNumber);
    }
}

// FUNCTION: 0x000c1bc8
void __stdcall RequestCompositeExchangeDataMsgCallback(u32 id, u32 seq, void *data)
{
    if (seq == sequenceNumber || seq == ((sequenceNumber - 1) & 0xf)) {
        if (spPtr->Send(id, compositeBuffer) != Lb_OK)
            fn_000c20c0(D_00122de4);
    } else {
        fn_000c20c0(D_00122e38, seq, sequenceNumber);
    }
}

// FUNCTION: 0x000c1c20
TbError GenericIPXInit(__tibcaGUID guid)
{
    TbError ret = Lb_FAIL;

    if (spPtr) {
        spPtr->Release();
        delete spPtr;
        spPtr = 0;
    }
    spPtr = new IPXServiceProvider;
    if (spPtr) {
        if (spPtr->Init(guid, 0, &receiveCallbacks, 0) == Lb_OK)
            ret = Lb_OK;
        else
            fn_000c20c0(D_00122093);
    }
    return ret;
}

// FUNCTION: 0x000c1d5c
TbError SendSystemUserMessage(u32 id, TbMessageType type, void *buf, u32 size)
{
    TbError ret = Lb_FAIL;

    if (size + 5 > 0x404) {
        fn_000c20c0(D_00122263, type);
    } else {
        EncodeMessageStub(systemUserBuffer, size + 1, (TbMessageType)4, 0);
        systemUserBuffer[4] = type;
        if (buf && size > 0)
            memcpy(systemUserBuffer + 5, buf, size);
        ret = spPtr->Send(id, systemUserBuffer);
    }
    return ret;
}

void PlayerMapMsgHandler(u32 id, void *buf, u32 size);

// FUNCTION: 0x000c1de8
// NONMATCHING: block layout: the target tests the message type inline and its early return
// jumps into MultiPlayerCallback's epilogue (the module was built without -zm)
void __stdcall SystemUserMsgCallback(u32 id, void *msg, u32 size, void *data)
{
    if (!msg || !size)
        return;
    switch (*(TbMessageType *)msg) {
    case MSG_TYPE_0:
        PlayerMapMsgHandler(id, (u8 *)msg + 1, size - 1);
        break;
    default:
        fn_000c20c0(D_00122f32, *(TbMessageType *)msg);
        break;
    }
}

// FUNCTION: 0x000c1f24
void PlayerMapMsgHandler(u32 id, void *buf, u32 size)
{
    u32 table_size = maximumPlayers * sizeof(__4kqpm7ClientDataEntry);

    if (size == 0) {
        SendSystemUserMessage(id, MSG_TYPE_0, clientDataTable, table_size);
    } else if (waitingForPlayerMapResponse) {
        if (size == table_size) {
            memcpy(clientDataTable, buf, size);
            waitingForPlayerMapResponse = 0;
        } else {
            fn_000c20c0(D_00122ea6, size);
        }
    } else {
        fn_000c20c0(D_00122ee2);
    }
}

// FUNCTION: 0x000c2020
IPXServiceProvider::IPXServiceProvider()
{
}

// FUNCTION: 0x000c20d0
TbNetworkSessionNameEntry::TbNetworkSessionNameEntry()
{
    id = 0;
    in_use = 0;
    name[0] = 0;
}

extern "C" {

// FUNCTION: 0x000c20c0
void fn_000c20c0(char *fmt, ...)
{
}

}

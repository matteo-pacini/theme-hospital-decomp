#include "net.h"

struct IPXServiceProvider : ServiceProvider {
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
    TbError SendMessage(u32 to, void *buf, u32 guaranteed);
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

TbError GenericSerialInit(__tibcaGUID guid, void *init_data);
TbError GenericModemInit(__tibcaGUID guid, void *init_data);
TbError StartTwoPlayerExchange(void *buf);
TbError CompleteTwoPlayerExchange(void *buf);
u32 GetRequestCompositeExchangeDataMsgSize(void);
void EncodeDeletePlayerMsg(u8 *buf, u32 id);
void EncodeRequestExchangeDataMsg(u8 *buf, u32 id, u32 seq);
void EncodeRequestCompositeExchangeDataMsg(u8 *buf, u32 id, u32 seq);
TbError HostDataCollection(void);
TbError LbNetwork_Init(u32 index, __tibcaGUID guid, u32 max_players, void *exchange_buf, u32 exchange_size,
                       __7308jnTbNetworkPlayerInfo *info, void *init_data);

extern "C" {
extern u32 D_00188cb8;
extern u32 D_0014da60;
extern char D_00121ef4[];
extern char D_00121f15[];
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
extern char D_00122843[];
extern char D_00122860[];
extern char D_001228a9[];
extern char D_001228e8[];
extern char D_00122938[];
extern char D_00122977[];

void __stdcall fn_000c1934(TbNetworkCallbackData *data, void *arg);
void *__stdcall fn_000c1518(u32 id, u32 size, u32 seq, void *data);
void __stdcall fn_000c1b24(u32 id, u32 seq, void *data);

void *fn_000bf790(u32 size);
#pragma aux fn_000bf790 "W?$dln(pnv)v"
void fn_000bf798(void *p);
#pragma aux fn_000bf798 "W?LbNetwork_ChangeExchangeTimeout$n(ul)v"
void fn_000bf7a0(u32 timeout);
#pragma aux fn_000bf7a0 "W?LbNetwork_LibraryVersion$n(pn$__ctcl4TbNetworkVersion$$)v"
TbError fn_000c1458(void);
#pragma aux fn_000c1458 "W?MultiPlayerCallback$n(ulululpnv)pnv"
TbError fn_000c197c(TbNetworkPlayerEntry *player);
#pragma aux fn_000c197c "W?MultiPlayerReqExDataMsgCallback$n(ululpnv)v"
u32 fn_000cd716(char *s);
#pragma aux fn_000cd716 "W?GetCurrentPlayersCallback$n(pn$TbNetworkCallbackData$$pnv)v"

// FUNCTION: 0x000bf790
void *fn_000bf790(u32 size)
{
    return LbMemoryAlloc(size);
}

// FUNCTION: 0x000bf798
void fn_000bf798(void *p)
{
    LbMemoryFree(p);
}

// NONMATCHING: target range also covers LbNetwork_LibraryVersion (0xbf7b8)
// FUNCTION: 0x000bf7a0
void fn_000bf7a0(u32 timeout)
{
    exchangeTimeout = timeout * 1000;
}

// FUNCTION: 0x000bf810
TbError fn_000bf810(u32 a, u32 b, u32 c, u32 d, u32 e)
{
    return Lb_FAIL;
}

}

TbError GetCurrentPlayers(void)
{
    TbError ret;

    ret = Lb_FAIL;
    localPlayerIndex = maximumPlayers;
    if (spPtr->Enumerate(0, fn_000c1934, 0) == Lb_OK) {
        if (localPlayerIndex < maximumPlayers)
            ret = Lb_OK;
        else
            fn_000c20c0(D_0012214a);
    } else {
        fn_000c20c0(D_0012218f);
    }
    return ret;
}

// NONMATCHING: target range also covers the two following functions (0xbf854, 0xbf91c)
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

// NONMATCHING: target registers an exception frame
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

// NONMATCHING: target registers an exception frame and constructs IPXServiceProvider inline
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
    D_0014da60 = 0;
    exchangeBuffer = (u8 *)exchange_buf;
    localPlayerInfoPtr = info;
    receiveCallbacks.multiPlayer = fn_000c1518;
    receiveCallbacks.mpReqExDataMsg = fn_000c1b24;
    if (exchange_size) {
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
        sp_ret = Lb_FAIL;
        sp_guid = guid;
        if (spPtr) {
            spPtr->Release();
            delete spPtr;
            spPtr = 0;
        }
        spPtr = new IPXServiceProvider;
        if (spPtr) {
            if (spPtr->Init(sp_guid, 0, &receiveCallbacks, 0) == Lb_OK)
                sp_ret = Lb_OK;
            else
                fn_000c20c0(D_00122093);
        }
        if (sp_ret == Lb_OK)
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

// NONMATCHING: register choice and block layout
// FUNCTION: 0x000bff98
TbError LbNetwork_Create(char *session, char *name, u32 *player, void *data)
{
    TbError ret;
    u32 i;

    ret = Lb_FAIL;
    if (runningTwoPlayerModel) {
        localPlayerId = 0;
        localPlayerIndex = 0;
        remotePlayerId = 1;
        remotePlayerIndex = 1;
    }
    if (spPtr->Start(session, name, maximumPlayers, data) != Lb_OK) {
        fn_000c20c0(D_00122225);
        return ret;
    }
    if (GetCurrentPlayers() != Lb_OK) {
        fn_000c20c0(D_001221df);
        return ret;
    }
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
    return ret;
}

// NONMATCHING: register choice and block layout
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
    if (spPtr->Start(session, name, data) != Lb_OK) {
        fn_000c20c0(D_001222f1);
        return ret;
    }
    if (!runningTwoPlayerModel) {
        EncodeMessageStub(systemUserBuffer, 1, (TbMessageType)4, 0);
        systemUserBuffer[4] = 0;
        spPtr->Send(0, systemUserBuffer);
        waitingForPlayerMapResponse = 1;
        do {
            spPtr->Receive(8);
            if (waitingForPlayerMapResponse && LbTimerClock() - start > 10000) {
                waitingForPlayerMapResponse = 0;
                return ret;
            }
        } while (waitingForPlayerMapResponse);
    }
    if (GetCurrentPlayers() != Lb_OK) {
        fn_000c20c0(D_001222ad);
        return ret;
    }
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
    return ret;
}

// NONMATCHING: register choice and block layout
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
        if (spPtr)
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

// NONMATCHING: target range also covers the following exchange function (0xc03f8)
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

// NONMATCHING: register choice and block layout
// FUNCTION: 0x000c0b74
TbError LbNetwork_Exchange(void *buf)
{
    TbError ret;
    TbError start_ret;
    TbError done_ret;
    u32 keep_going;
    u32 retries;
    u32 host_changed;
    u32 old_host;
    u32 elapsed;
    u32 i;

    ret = Lb_FAIL;
    if (runningTwoPlayerModel) {
        start_ret = StartTwoPlayerExchange(buf);
    } else {
        start_ret = Lb_FAIL;
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
        if (hostId == localPlayerId) {
            start_ret = Lb_OK;
        } else {
            EncodeMessageStub(buf, exchangeSize - 4, MSG_TYPE_0, sequenceNumber);
            if (spPtr->Send(hostId, buf) == Lb_OK)
                start_ret = Lb_OK;
            else
                fn_000c20c0(D_001223eb);
        }
    }
    if (start_ret != Lb_OK) {
        fn_000c20c0(D_001226bb);
        return ret;
    }
    if (runningTwoPlayerModel) {
        done_ret = CompleteTwoPlayerExchange(buf);
    } else {
        done_ret = Lb_FAIL;
        if (localPlayerId == hostId) {
            HostDataCollection();
            done_ret = fn_000c1458();
        } else {
            keep_going = 1;
            host_changed = 0;
            gotCompositeData = 0;
            retries = 0;
            do {
                old_host = hostId;
                spPtr->Receive(8);
                if (old_host != hostId)
                    host_changed = 1;
                if (host_changed) {
                    if (hostId == localPlayerId) {
                        if (HostDataCollection() == Lb_OK) {
                            if (fn_000c1458() == Lb_OK)
                                done_ret = Lb_OK;
                            else
                                fn_000c20c0(D_0012242f);
                        } else {
                            fn_000c20c0(D_0012247f);
                        }
                        keep_going = 0;
                    } else {
                        EncodeMessageStub(buf, exchangeSize - 4, MSG_TYPE_0, sequenceNumber);
                        if (spPtr->Send(hostId, buf) == Lb_OK)
                            done_ret = Lb_OK;
                        else
                            fn_000c20c0(D_001224d0);
                    }
                    host_changed = 0;
                } else if (gotCompositeData) {
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
                    done_ret = Lb_OK;
                    keep_going = 0;
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
    }
    if (done_ret == Lb_OK)
        ret = Lb_OK;
    else
        fn_000c20c0(D_0012266a);
    return ret;
}

// NONMATCHING: target range also covers the following function (0xc1028)
// FUNCTION: 0x000c0f68
TbError LbNetwork_ChangeExchangeBuffer(void *buf, u32 size)
{
    TbError ret;
    u32 total;
    void *new_buf;

    total = maximumPlayers * size;
    ret = Lb_FAIL;
    exchangeBuffer = (u8 *)buf;
    exchangeSize = size;
    if (!compositeBuffer) {
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

extern "C" {

// NONMATCHING: shares its epilogue with the function at 0xc1028; range also covers MultiPlayerCallback (0xc1518)
// FUNCTION: 0x000c1458
TbError fn_000c1458(void)
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

// NONMATCHING: register choice and block layout
// FUNCTION: 0x000c197c
TbError fn_000c197c(TbNetworkPlayerEntry *player)
{
    TbError ret;
    u32 found;
    u32 searching;
    u32 index;
    u32 i;

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
            do {
                if (!clientDataTable[i].active) {
                    found = 1;
                    searching = 0;
                } else if (++i >= maximumPlayers) {
                    searching = 0;
                }
            } while (searching);
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

// NONMATCHING: C library strlen; target saves and reloads es around the scan
// FUNCTION: 0x000cd716
u32 fn_000cd716(char *s)
{
    return strlen(s);
}

// FUNCTION: 0x000c20c0
void fn_000c20c0(char *fmt, ...)
{
}

}

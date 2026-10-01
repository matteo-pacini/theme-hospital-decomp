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
    void (__stdcall *unknown24)(u32 id, void *msg);
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
static u32 actualTimeout;
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
// The module's internal functions are static, as the demo's debug info has them. Under -oe,
// Watcom inlines a static function called from one place whatever its size
// (GenericIPXInit, Start/CompleteMultiPlayerExchange); others are inlined only below the
// -oe size, which keeps HostDataCollection, HostDataBroadcast and LbNetwork_Init(u32, ...)
// out of line.
static TbError GenericIPXInit(__tibcaGUID guid);
TbError StartTwoPlayerExchange(void *buf);
TbError CompleteTwoPlayerExchange(void *buf);
u32 GetRequestCompositeExchangeDataMsgSize(void);
void EncodeDeletePlayerMsg(u8 *buf, u32 id);
void EncodeRequestExchangeDataMsg(u8 *buf, u32 id, u32 seq);
void EncodeRequestCompositeExchangeDataMsg(u8 *buf, u32 id, u32 seq);
static TbError HostDataCollection(void);
TbError LbNetwork_Init(u32 index, __tibcaGUID guid, u32 max_players, void *exchange_buf, u32 exchange_size,
                       __7308jnTbNetworkPlayerInfo *info, void *init_data);

extern "C" {
extern u32 compositeBufferSize;

}

static void *__stdcall MultiPlayerCallback(u32 id, u32 size, u32 seq, void *data);
void __stdcall AddMsgCallback(u32 id, char *name, void *data);
void __stdcall DeleteMsgCallback(u32 id, void *data);
void *__stdcall UnidirectionalMsgCallback(u32 id, u32 size, void *data);
void __stdcall MultiPlayerReqExDataMsgCallback(u32 id, u32 seq, void *data);
void __stdcall RequestCompositeExchangeDataMsgCallback(u32 id, u32 seq, void *data);
static TbError GetCurrentPlayers(void);
static TbError GetPlayerInfo(void);
static TbError HostDataBroadcast(void);
static TbError SendSystemUserMessage(u32 id, TbMessageType type, void *buf, u32 size);
static u32 IdToIndex(u32 &id);
void __stdcall HostMsgCallback(u32 id, void *data);
void __stdcall SystemUserMsgCallback(u32 id, void *msg, u32 size, void *data);

u32 basicTimeout = 250;
__ic76sReceiveCallbacks receiveCallbacks = {
    AddMsgCallback,
    DeleteMsgCallback,
    HostMsgCallback,
    0,
    MultiPlayerCallback,
    MultiPlayerReqExDataMsgCallback,
    RequestCompositeExchangeDataMsgCallback,
    UnidirectionalMsgCallback,
    SystemUserMsgCallback,
    0,
};

extern "C" {

void fn_000bf7a0(u32 timeout);
#pragma aux fn_000bf7a0 "W?LbNetwork_ChangeExchangeTimeout$n(ul)v"
TbError AddAPlayer(TbNetworkPlayerEntry *player);
#pragma aux AddAPlayer "W?AddAPlayer$n(pn$TbNetworkPlayerNameEntry$$)$TbError$$"
u32 fn_000cd716(char *s);
#pragma aux fn_000cd716 "fn_000cd716_"

}

// Global operator new: allocates with LbMemoryAlloc.
// FUNCTION: 0x000bf790
void *operator new(unsigned size)
{
    return LbMemoryAlloc(size);
}

// Global operator delete: frees with LbMemoryFree.
// FUNCTION: 0x000bf798
void operator delete(void *p)
{
    LbMemoryFree(p);
}

extern "C" {

// Sets the exchange timeout to timeout seconds
// (LbNetwork_ChangeExchangeTimeout).
// FUNCTION: 0x000bf7a0
void fn_000bf7a0(u32 timeout)
{
    exchangeTimeout = timeout * 1000;
}

}

// Fills version with the network library's version text and number (1.1).
// FUNCTION: 0x000bf7b8
void LbNetwork_LibraryVersion(__ctcl4TbNetworkVersion *version)
{
    char os[0x28];

    if (version) {
        strcpy(os, "DOS");
        sprintf(version->text, "BULLFROG NET LIBRARY VERSION %f FOR %s LAST BUILT __DATE__\n", 1.1, os);
        version->version = 1.1f;
    }
}

extern "C" {

TbError LbNetwork_AreWeBeingLobbied(u32 a, u32 b, u32 c, u32 d, u32 e);
#pragma aux LbNetwork_AreWeBeingLobbied "W?LbNetwork_AreWeBeingLobbied$n(ulpnvulpn$__7308jnTbNetworkPlayerInfo$$pnul)$TbError$$"

// Lobbies are not supported under DOS: returns Lb_FAIL.
// FUNCTION: 0x000bf810
TbError LbNetwork_AreWeBeingLobbied(u32 a, u32 b, u32 c, u32 d, u32 e)
{
    return Lb_FAIL;
}

}

void __stdcall GetCurrentPlayersCallback(TbNetworkCallbackData *data, void *arg);

// Lets new players join the session (allow nonzero) or not. Returns the
// service provider's result, Lb_FAIL if none is running.
// FUNCTION: 0x000bf818
TbError LbNetwork_EnableNewPlayers(u32 allow)
{
    TbError ret;

    ret = Lb_FAIL;
    if (spPtr) {
        if (allow)
            fn_000c20c0("NEW PLAYERS ARE ALLOWED TO JOIN\n");
        else
            fn_000c20c0("NEW PLAYERS ARE NOT ALLOWED TO JOIN\n");
        ret = spPtr->EnableNewPlayers(allow);
    }
    return ret;
}

struct ServiceEntry {
    u8 flags;
    char name[0x43];
};

// Calls callback with each available service: Serial, Modem and IPX.
// Returns Lb_OK.
// FUNCTION: 0x000bf854
TbError LbNetwork_EnumerateServices(TbNetworkCallbackFunc callback, void *data)
{
    ServiceEntry service;

    strcpy(service.name, "Serial");
    callback((TbNetworkCallbackData *)&service, data);
    strcpy(service.name, "Modem");
    callback((TbNetworkCallbackData *)&service, data);
    strcpy(service.name, "IPX");
    callback((TbNetworkCallbackData *)&service, data);
    return Lb_OK;
}

struct __4dn7ojSerialInitData {
    u32 port;
    u32 baud;
    s32 irq;
    u32 mode;
    char *init_str;
    char *dial_str;
    char *hangup_str;
    char *reset_str;
};

struct TbSerialInitParams {
    u8 unknown0;
    u32 port;
    u32 baud;
    s32 irq;
};

struct TbModemInitParams {
    u8 unknown0;
    u32 port;
    u32 baud;
    char *init_str;
    char *dial_str;
    char *hangup_str;
    char *reset_str;
    s32 irq;
};

union TbNetworkInitParams {
    TbSerialInitParams serial;
    TbModemInitParams modem;
};

extern "C" {

// LbNetwork_Init for service `service` taking the serial or modem settings
// as TbNetworkInitParams (0 for none), converted to the provider's form.
// FUNCTION: 0x000bf91c
TbError fn_000bf91c(TbNetworkService service, __tibcaGUID guid, u32 max_players, void *exchange_buf,
                    u32 exchange_size, __7308jnTbNetworkPlayerInfo *info, TbNetworkInitParams *params)
{
    __4dn7ojSerialInitData data;
    void *init_data = &data;

    if (!params) {
        init_data = 0;
    } else {
        switch (service) {
        case NS_SERIAL:
            data.port = params->serial.port;
            data.baud = params->serial.baud;
            data.irq = params->serial.irq;
            break;
        case (TbNetworkService)4:
            data.port = params->serial.port;
            data.baud = params->serial.baud;
            break;
        case NS_MODEM:
            data.port = params->modem.port;
            data.baud = params->modem.baud;
            data.irq = params->modem.irq;
            data.init_str = params->modem.init_str;
            data.dial_str = params->modem.dial_str;
            data.hangup_str = params->modem.hangup_str;
            data.reset_str = params->modem.reset_str;
            break;
        case (TbNetworkService)5:
            data.port = params->modem.port;
            data.baud = params->modem.baud;
            data.init_str = params->modem.init_str;
            data.dial_str = params->modem.dial_str;
            data.hangup_str = params->modem.hangup_str;
            data.reset_str = params->modem.reset_str;
            break;
        default:
            init_data = params;
            break;
        }
    }
    return LbNetwork_Init(service, guid, max_players, exchange_buf, exchange_size, info, init_data);
}

}

// Starts network service `service` (NS_SERIAL, NS_MODEM, NS_IPX) for up to
// max_players players exchanging exchange_size-byte packets in exchange_buf,
// with player names in info. Returns Lb_OK, or Lb_FAIL for an unknown
// service or a failed start.
// FUNCTION: 0x000bfac0
TbError LbNetwork_Init(TbNetworkService service, __tibcaGUID guid, u32 max_players, void *exchange_buf,
                       u32 exchange_size, __7308jnTbNetworkPlayerInfo *info, void *init_data)
{
    TbError ret;
    u32 index;

    ret = Lb_OK;
    switch (service) {
    case NS_SERIAL:
    case (TbNetworkService)4:
        index = 0;
        break;
    case NS_MODEM:
    case (TbNetworkService)5:
        index = 1;
        break;
    case NS_IPX:
    case (TbNetworkService)6:
        index = 2;
        break;
    default:
        fn_000c20c0("The requested serviceType %d is out of range\n", (s32)service);
        ret = Lb_FAIL;
        break;
    }
    if (ret == Lb_OK)
        ret = LbNetwork_Init(index, guid, max_players, exchange_buf, exchange_size, info, init_data);
    return ret;
}

extern "C" {

// As fn_000bf91c, by provider index (0 serial, 1 modem, 2 IPX).
// FUNCTION: 0x000bfb78
TbError fn_000bfb78(u32 index, __tibcaGUID guid, u32 max_players, void *exchange_buf,
                    u32 exchange_size, __7308jnTbNetworkPlayerInfo *info, TbNetworkInitParams *params)
{
    __4dn7ojSerialInitData data;
    void *init_data = &data;

    if (!params) {
        init_data = 0;
    } else {
        switch (index) {
        case 0:
            data.port = params->serial.port;
            data.baud = params->serial.baud;
            data.irq = params->serial.irq;
            break;
        case 1:
            data.port = params->modem.port;
            data.baud = params->modem.baud;
            data.irq = params->modem.irq;
            data.init_str = params->modem.init_str;
            data.dial_str = params->modem.dial_str;
            data.hangup_str = params->modem.hangup_str;
            data.reset_str = params->modem.reset_str;
            break;
        default:
            init_data = params;
            break;
        }
    }
    return LbNetwork_Init(index, guid, max_players, exchange_buf, exchange_size, info, init_data);
}

}

// Starts the service provider at index (0 serial, 1 modem, 2 IPX) with the
// exchange settings, allocating the composite buffer and clearing the
// player table. Returns Lb_OK, or Lb_FAIL.
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
    compositeBufferSize = 0;
    exchangeBuffer = (u8 *)exchange_buf;
    localPlayerInfoPtr = info;
    receiveCallbacks.multiPlayer = MultiPlayerCallback;
    receiveCallbacks.unknown24 = 0;
    receiveCallbacks.mpReqExDataMsg = MultiPlayerReqExDataMsgCallback;
    if (exchange_size > 0) {
        compositeBufferSize = max_players * exchange_size;
        compositeBuffer = LbMemoryAlloc(compositeBufferSize);
        if (!compositeBuffer) {
            fn_000c20c0("  WARNING network: LbNetwork_Init() failure on compositeBuffer allocation\n");
            return ret;
        }
    }
    for (i = 0; i < maximumPlayers; i++)
        clientDataTable[i].active = 0;
    for (i = 0; i < maximumPlayers; i++) {
        localPlayerInfoPtr->active = 0;
        strcpy(localPlayerInfoPtr->name, "");
    }
    switch (index) {
    case 0:
        fn_000c20c0("Selecting DOS Serial SP\n");
        if (GenericSerialInit(guid, init_data) == Lb_OK)
            ret = Lb_OK;
        else
            fn_000c20c0("  WARNING network: LbNetwork_Init() failure on GenericSerialInit()\n");
        break;
    case 1:
        fn_000c20c0("Selecting DOS Modem SP\n");
        if (GenericModemInit(guid, init_data) == Lb_OK)
            ret = Lb_OK;
        else
            fn_000c20c0("  WARNING network: LbNetwork_Init() failure on GenericModemInit()\n");
        break;
    case 2:
        fn_000c20c0("Selecting DOS IPX SP\n");
        if (GenericIPXInit(guid) == Lb_OK)
            ret = Lb_OK;
        else
            fn_000c20c0("  WARNING network: LbNetwork_Init() failure on GenericIPXInit()\n");
        break;
    default:
        fn_000c20c0("The serviceIndex value of %d is out of range\n", index);
        break;
    }
    return ret;
}

// Creates session `session` as player name and allows new players. Stores
// this player's index in *player. Returns Lb_OK, or Lb_FAIL.
// FUNCTION: 0x000bff98
TbError LbNetwork_Create(char *session, char *name, u32 *player, void *data)
{
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
            GetPlayerInfo();
            LbNetwork_EnableNewPlayers(1);
        } else {
            fn_000c20c0("  WARNING network: LbNetwork_Create() failure on GetCurrentPlayers()\n");
        }
    } else {
        fn_000c20c0("  WARNING network: LbNetwork_Create() failure on SP::Start()\n");
    }
    return ret;
}

// Joins session as player name, waiting up to 10 seconds for the host's
// player map. Stores this player's index in *player. Returns Lb_OK, or
// Lb_FAIL.
// FUNCTION: 0x000c0110
TbError LbNetwork_Join(TbNetworkSessionNameEntry *session, char *name, u32 *player, void *data)
{
    TbError ret;
    u32 start;

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
            SendSystemUserMessage(0, MSG_TYPE_PLAYER_MAP, 0, 0);
            waitingForPlayerMapResponse = 1;
            while (waitingForPlayerMapResponse) {
                spPtr->Receive(RECV_ALL);
                if (waitingForPlayerMapResponse) {
                    if (LbTimerClock() - start > 10000) {
                        waitingForPlayerMapResponse = 0;
                        return ret;
                    }
                }
            }
        }
        if (GetCurrentPlayers() == Lb_OK) {
            *player = localPlayerIndex;
            ret = Lb_OK;
            GetPlayerInfo();
        } else {
            fn_000c20c0("  WARNING network: LbNetwork_Join() failure on GetCurrentPlayers()\n");
        }
    } else {
        fn_000c20c0("  WARNING network: LbNetwork_Join() failure on Join\n");
    }
    return ret;
}

// Shuts down the service provider and frees the composite buffer. Returns
// Lb_OK, or Lb_FAIL if the release failed or nothing was running.
// FUNCTION: 0x000c02fc
TbError LbNetwork_Stop(void)
{
    TbError ret;

    ret = Lb_FAIL;
    if (spPtr) {
        if (spPtr->Release() == Lb_OK)
            ret = Lb_OK;
        else
            fn_000c20c0("  WARNING network: LbNetwork_Stop() failure on Release\n");
        delete spPtr;
        spPtr = 0;
    }
    if (compositeBuffer) {
        LbMemoryFree(compositeBuffer);
        compositeBuffer = 0;
    }
    return ret;
}

// Calls callback with each session found. Returns Lb_OK (also with no
// provider running), or Lb_FAIL.
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
        fn_000c20c0("  WARNING network: LbNetwork_EnumerateSessions() failure on Enumerate\n");
    return ret;
}

// Calls callback with each player of session. Returns Lb_OK, or Lb_FAIL.
// FUNCTION: 0x000c03b8
TbError LbNetwork_EnumeratePlayers(TbNetworkSessionNameEntry *session, TbNetworkCallbackFunc callback, void *data)
{
    TbError ret;

    ret = Lb_FAIL;
    if (spPtr->Enumerate(session, callback, data) == Lb_OK)
        ret = Lb_OK;
    else
        fn_000c20c0("  WARNING network: LbNetwork_EnumeratePlayers() failure on Enumerate\n");
    return ret;
}

static TbError StartMultiPlayerExchange(void *buf);

// Starts an exchange of this player's packet buf with everyone's.
// FUNCTION: 0x000c03f8
TbError LbNetwork_StartExchange(void *buf)
{
    if (runningTwoPlayerModel)
        return StartTwoPlayerExchange(buf);
    return StartMultiPlayerExchange(buf);
}

// Starts a multi-player exchange: stores buf in this player's slot of the
// exchange buffer and, on a client, sends it to the host. Returns Lb_OK, or
// Lb_FAIL if the send failed.
// FUNCTION: 0x000c0524
static TbError StartMultiPlayerExchange(void *buf)
{
    TbError ret;
    u32 i;

    ret = Lb_FAIL;
    localDataPtr = buf;
    spPtr->Receive(RECV_ADD_PLAYER | RECV_DELETE_PLAYER);
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
        EncodeMessageStub(buf, exchangeSize - 4, MSG_TYPE_MULTIPLAYER, sequenceNumber);
        if (spPtr->Send(hostId, buf) == Lb_OK)
            ret = Lb_OK;
        else
            fn_000c20c0("  WARNING network: LbNetwork_StartExchange() failure on SP::Send()\n");
    }
    return ret;
}

static TbError CompleteMultiPlayerExchange(void *buf);

// Completes an exchange started by LbNetwork_StartExchange, leaving every
// player's packet in the exchange buffer. Returns Lb_OK, or Lb_FAIL.
// FUNCTION: 0x000c0640
// NONMATCHING: register choice in the inlined timeout bookkeeping, as in CompleteMultiPlayerExchange
TbError LbNetwork_CompleteExchange(void *buf)
{
    if (runningTwoPlayerModel)
        return CompleteTwoPlayerExchange(buf);
    return CompleteMultiPlayerExchange(buf);
}

// Completes a multi-player exchange: the host collects every packet and
// broadcasts the composite; a client waits for it, asking again on timeout,
// dropping the host after ten retries and taking over if it becomes host.
// Adapts the timeout to the measured round trip. Advances the sequence
// number (0-14). Returns Lb_OK, or Lb_FAIL.
// FUNCTION: 0x000c08e4
// NONMATCHING: register choice: the target copies the elapsed time into ebp and tests timeCount in eax
static TbError CompleteMultiPlayerExchange(void *buf)
{
    TbError ret;
    u32 keep_going;
    u32 retries;
    u32 host_changed;
    u32 old_host;

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
            spPtr->Receive(RECV_ALL);
            if (old_host != hostId)
                host_changed = 1;
            if (!host_changed) {
                if (gotCompositeData) {
                    if (!retries) {
                        // keep_going, about to be cleared, holds the elapsed time.
                        keep_going = LbTimerClock() - startTime;
                        if (!timeCount || keep_going > maxTime)
                            maxTime = keep_going;
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
                            fn_000c20c0("  WARNING network: LbNetwork_CompleteExchange() failure on HostDataBroadcast()\n");
                    } else {
                        fn_000c20c0("  WARNING network: LbNetwork_CompleteExchange() failure on HostDataCollection()\n");
                    }
                    keep_going = 0;
                } else {
                    EncodeMessageStub(buf, exchangeSize - 4, MSG_TYPE_MULTIPLAYER, sequenceNumber);
                    if (spPtr->Send(hostId, buf) == Lb_OK)
                        ret = Lb_OK;
                    else
                        fn_000c20c0("  WARNING network: LbNetwork_CompleteExchange() failure on SP::Send()\n");
                }
                host_changed = 0;
            }
            if (keep_going && LbTimerClock() - startTime > actualTimeout) {
                fn_000c20c0("Timed out waiting for host");
                retries++;
                if (retries > 10) {
                    if (GetRequestCompositeExchangeDataMsgSize() > 8) {
                        fn_000c20c0("  WARNING network: LbNetwork_CompleteExchange() deletePlayerBuffer is too small\n");
                    } else {
                        EncodeDeletePlayerMsg(deletePlayerBuffer, hostId);
                        if (spPtr->Send(localPlayerId, deletePlayerBuffer) != Lb_OK)
                            fn_000c20c0("  WARNING network: HostDataCollection() failure on SP::Send()\n");
                    }
                    host_changed = 1;
                } else if (GetRequestCompositeExchangeDataMsgSize() > 8) {
                    fn_000c20c0("  WARNING network: LbNetwork_CompleteExchange() requestCompositeExchangeDataBuffer is too small\n");
                } else {
                    EncodeRequestCompositeExchangeDataMsg(requestCompositeExchangeDataBuffer, localPlayerId, sequenceNumber);
                    if (spPtr->Send(hostId, requestCompositeExchangeDataBuffer) != Lb_OK)
                        fn_000c20c0("  WARNING network: LbNetwork_CompleteExchange() failure on SP::Send()\n");
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

// Exchanges this player's packet buf with everyone's (start and complete).
// Returns Lb_OK, or Lb_FAIL.
// FUNCTION: 0x000c0b74
// NONMATCHING: register choice in the inlined exchange code
TbError LbNetwork_Exchange(void *buf)
{
    TbError ret;

    ret = Lb_FAIL;
    if (LbNetwork_StartExchange(buf) == Lb_OK) {
        if (LbNetwork_CompleteExchange(buf) == Lb_OK)
            ret = Lb_OK;
        else
            fn_000c20c0("  WARNING network: LbNetwork_Exchange() failure on LbNetwork_CompleteExchange()\n");
    } else {
        fn_000c20c0("  WARNING network: LbNetwork_Exchange() failure on LbNetwork_StartExchange()\n");
    }
    return ret;
}

// Makes buf (size bytes per player) the exchange buffer, growing the
// composite buffer if needed. Returns Lb_OK, or Lb_FAIL if it could not be
// allocated.
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
    if (compositeBuffer == 0) {
        compositeBuffer = LbMemoryAlloc(total);
        if (!compositeBuffer) {
            fn_000c20c0("  WARNING network: LbNetwork_ChangeExchangeBuffer() failure on buffer allocation\n");
        } else {
            compositeBufferSize = total;
            ret = Lb_OK;
        }
    } else if (total > compositeBufferSize) {
        new_buf = LbMemoryAlloc(total);
        if (!new_buf) {
            fn_000c20c0("  WARNING network: LbNetwork_ChangeExchangeBuffer() failure on buffer allocation\n");
        } else {
            memcpy(new_buf, compositeBuffer, compositeBufferSize);
            ret = Lb_OK;
            LbMemoryFree(compositeBuffer);
            compositeBuffer = new_buf;
            compositeBufferSize = total;
        }
    } else {
        ret = Lb_OK;
    }
    return ret;
}

// Refreshes the player table and the caller's player info. Returns Lb_OK,
// or Lb_FAIL.
// FUNCTION: 0x000c1028
TbError LbNetwork_GetPlayerInfo(void)
{
    TbError ret;

    ret = Lb_FAIL;
    if (GetCurrentPlayers() == Lb_OK) {
        if (GetPlayerInfo() == Lb_OK)
            ret = Lb_OK;
        else
            fn_000c20c0("  WARNING network: LbNetwork_GetPlayerInfo() failure on GetPlayerInfo()\n");
    } else {
        fn_000c20c0("  WARNING network: LbNetwork_GetPlayerInfo() failure on GetCurrentPlayers()\n");
    }
    return ret;
}

// Copies the active players' names into the caller's player info. Returns
// Lb_OK.
// FUNCTION: 0xc1130
static TbError GetPlayerInfo(void)
{
    TbError ret = Lb_OK;
    u32 i;

    for (i = 0; i < maximumPlayers; i++) {
        if (clientDataTable[i].active) {
            localPlayerInfoPtr[i].active = 1;
            strcpy(localPlayerInfoPtr[i].name, clientDataTable[i].name);
        } else {
            localPlayerInfoPtr[i].active = 0;
        }
    }
    return ret;
}

// Host: waits until every active client's packet has arrived, asking
// missing clients again on timeout and dropping them after ten retries.
// Adapts the timeout to the measured round trip. Returns Lb_OK.
// NONMATCHING: register choice and block layout
// FUNCTION: 0x000c11b8
static TbError HostDataCollection(void)
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
                fn_000c20c0("Timed out waiting for client");
                retries++;
                if (retries > 10) {
                    if (GetRequestCompositeExchangeDataMsgSize() > 8) {
                        fn_000c20c0("  WARNING network: HostDataCollection() deletePlayerBuffer is too small\n");
                    } else {
                        for (i = 0; i < maximumPlayers; i++) {
                            if (clientDataTable[i].active && !clientDataTable[i].got_data) {
                                EncodeDeletePlayerMsg(deletePlayerBuffer, clientDataTable[i].id);
                                for (j = 0; j < maximumPlayers; j++) {
                                    if (clientDataTable[j].active && clientDataTable[j].id != clientDataTable[i].id
                                        && spPtr->Send(clientDataTable[j].id, deletePlayerBuffer) != Lb_OK)
                                        fn_000c20c0("  WARNING network: HostDataCollection() failure on SP::Send()\n");
                                }
                            }
                        }
                    }
                } else if (GetRequestCompositeExchangeDataMsgSize() > 8) {
                    fn_000c20c0("  WARNING network: HostDataCollection() requestExchangeDataBuffer is too small\n");
                } else {
                    EncodeRequestExchangeDataMsg(requestExchangeDataBuffer, hostId, sequenceNumber);
                    for (i = 0; i < maximumPlayers; i++) {
                        if (clientDataTable[i].active && !clientDataTable[i].got_data
                            && spPtr->Send(clientDataTable[i].id, requestExchangeDataBuffer) != Lb_OK)
                            fn_000c20c0("  WARNING network: HostDataCollection() failure on SP::Send()\n");
                    }
                }
                startTime = LbTimerClock();
                actualTimeout = basicTimeout * (retries + 1);
                basicTimeout += 100;
            }
            spPtr->Receive(RECV_ALL);
        }
    } while (keep_going);
    return ret;
}

// Host: sends the composite of all packets to every client. Returns Lb_OK,
// or Lb_FAIL if a send failed.
// FUNCTION: 0x000c1458
// The target returns through GetPlayerInfo's epilogue (the module was built without -zm)
static TbError HostDataBroadcast(void)
{
    TbError ret;
    u32 i;

    ret = Lb_OK;
    EncodeMessageStub(exchangeBuffer, exchangeSize * maximumPlayers - 4, MSG_TYPE_MULTIPLAYER, sequenceNumber);
    memcpy(compositeBuffer, exchangeBuffer, exchangeSize * maximumPlayers);
    for (i = 0; i < maximumPlayers; i++) {
        if (clientDataTable[i].active && clientDataTable[i].id != hostId
            && spPtr->Send(clientDataTable[i].id, exchangeBuffer) != Lb_OK) {
            fn_000c20c0("  WARNING network: LbNetwork_CompleteExchange() failure on SP::Send()\n");
            ret = Lb_FAIL;
        }
    }
    return ret;
}

// Provider callback for an exchange packet from player id: returns where to
// store it (a client's slot on the host, the whole exchange buffer for the
// host's composite on a client), checking its size and sequence number
// seq; 0 to discard it.
// FUNCTION: 0x000c1518
static void *__stdcall MultiPlayerCallback(u32 id, u32 size, u32 seq, void *data)
{
    void *ret = 0;

    if (localPlayerId == hostId) {
        if (size == exchangeSize) {
            if (id == localPlayerId) {
                fn_000c20c0("  WARNING network: UserDataMsgCallback() host got data from itself\n");
            } else if (IdToIndex(id)) {
                if (seq == sequenceNumber || seq == 15) {
                    ret = exchangeBuffer + exchangeSize * id;
                    clientDataTable[id].got_data = 1;
                } else {
                    fn_000c20c0("  WARNING network: UserDataMsgCallback() Unexpected sequence number: Got %d, expected %d\n", seq, sequenceNumber);
                }
            } else {
                fn_000c20c0("  WARNING network: UserDataMsgCallback() invalid id: %d\n", id);
            }
        } else {
            fn_000c20c0("  WARNING network: UserDataMsgCallback() invalid length: %d\n", size);
        }
    } else if (size == exchangeSize * maximumPlayers) {
        if (id == hostId) {
            if (IdToIndex(id)) {
                u32 ok = 1;
                if (sequenceNumber == 15)
                    sequenceNumber = seq;
                else if (sequenceNumber != seq)
                    ok = 0;
                if (ok) {
                    gotCompositeData = 1;
                    ret = exchangeBuffer;
                } else {
                    fn_000c20c0("  WARNING network: UserDataMsgCallback() Unexpected sequence number: Got %d, expected %d\n", seq, sequenceNumber);
                }
            } else {
                fn_000c20c0("  WARNING network: UserDataMsgCallback() invalid id: %d\n", id);
            }
        } else {
            fn_000c20c0("  WARNING network: UserDataMsgCallback() data is not from host\n");
        }
    } else if (size == exchangeSize) {
        if (id == localPlayerId) {
            fn_000c20c0("  WARNING network: UserDataMsgCallback() client acting as host got data from itself\n");
        } else if (IdToIndex(id)) {
            ret = exchangeBuffer + exchangeSize * id;
            clientDataTable[id].got_data = 1;
        } else {
            fn_000c20c0("  WARNING network: UserDataMsgCallback() invalid id: %d\n", id);
        }
    } else {
        fn_000c20c0("  WARNING network: UserDataMsgCallback() invalid length: %d\n", size);
    }
    return ret;
}

// Provider callback: a player id named name has joined.
// FUNCTION: 0x000c1810
void __stdcall AddMsgCallback(u32 id, char *name, void *data)
{
    TbNetworkPlayerEntry player;

    player.id = id;
    strcpy(player.name, name);
    player.local = 0;
    player.host = 0;
    AddAPlayer(&player);
}

// Provider callback: player id has left; marks them inactive.
// FUNCTION: 0x000c1858
void __stdcall DeleteMsgCallback(u32 id, void *data)
{
    u32 i;

    for (i = 0; i < maximumPlayers; i++) {
        if (clientDataTable[i].active && clientDataTable[i].id == id) {
            clientDataTable[i].active = 0;
            if (!localPlayerInfoPtr)
                fn_000c20c0("  WARNING network: DeleteMsgCallback() NULL localPlayerInfoPtr\n");
            else
                localPlayerInfoPtr[i].active = 0;
        }
    }
}

// Provider callback: player id is now the host.
// FUNCTION: 0x000c18c4
void __stdcall HostMsgCallback(u32 id, void *data)
{
    hostId = id;
}

// Rebuilds the player table from the provider's player list. Returns Lb_OK
// if this player was found, else Lb_FAIL.
// FUNCTION: 0x000c18d0
static TbError GetCurrentPlayers(void)
{
    TbError ret;

    ret = Lb_FAIL;
    localPlayerIndex = maximumPlayers;
    if (spPtr->Enumerate(0, GetCurrentPlayersCallback, 0) == Lb_OK) {
        if (localPlayerIndex < maximumPlayers)
            ret = Lb_OK;
        else
            fn_000c20c0("  WARNING network: GetCurrentPlayers() localPlayerIndex not updated\n");
    } else {
        fn_000c20c0("  WARNING network: GetCurrentPlayers() failure on ServiceProvider::Enumerate()\n");
    }
    return ret;
}

// Player enumeration callback: adds the player to the table.
// FUNCTION: 0x000c1934
void __stdcall GetCurrentPlayersCallback(TbNetworkCallbackData *data, void *arg)
{
    AddAPlayer((TbNetworkPlayerEntry *)data);
}

// Provider callback for a one-way message of size bytes: returns the buffer
// to receive it into, or 0 if it is too big (over 0x20c bytes).
// FUNCTION: 0x000c1940
void *__stdcall UnidirectionalMsgCallback(u32 id, u32 size, void *data)
{
    void *ret = 0;

    if (size > 0x20c) {
        fn_000c20c0("  Warning: UnidirectionalMsgCallback invalid length %d vs %d", size, 0x20c);
    } else {
        ret = &incomingUnidirectionalMessage;
        unidirectionalMsgReceived = 1;
    }
    return ret;
}

extern "C" {

// Adds player to the player table (if not already there), recording the
// host and this player's own id and index. Returns Lb_OK, or Lb_FAIL if the
// table is full.
// NONMATCHING: register choice and block layout
// FUNCTION: 0x000c197c
TbError AddAPlayer(TbNetworkPlayerEntry *player)
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
            if (player->local) {
                localPlayerId = player->id;
                localPlayerIndex = index;
            }
        }
    }
    return ret;
}

}

// Replaces player id id by its index in the player table. Returns 1 if
// found, else 0 (id unchanged).
// FUNCTION: 0x000c1ad0
static u32 IdToIndex(u32 &id)
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

// Provider callback: the host asks for this player's packet again; resends
// it if the sequence number seq matches.
// FUNCTION: 0x000c1b24
void __stdcall MultiPlayerReqExDataMsgCallback(u32 id, u32 seq, void *data)
{
    u32 ok;

    if (!localDataPtr) {
        fn_000c20c0("  WARNING network: RequestExchangeDataMsgCallback() NULL data pointer\n");
        return;
    }
    ok = 1;
    if (sequenceNumber == 15)
        sequenceNumber = seq;
    else if (seq != sequenceNumber)
        ok = 0;
    if (ok) {
        EncodeMessageStub(localDataPtr, exchangeSize - 4, MSG_TYPE_MULTIPLAYER, sequenceNumber);
        if (spPtr->Send(id, localDataPtr) != Lb_OK)
            fn_000c20c0("  WARNING network: RequestExchangeDataMsgCallback() failure on SP::Send()\n");
    } else {
        fn_000c20c0("  WARNING network: RequestExchangeDataMsgCallback() unexpected sequence number, got %d, expected %d\n", seq, sequenceNumber);
    }
}

// Provider callback: a client asks for the composite again; resends it if
// seq is the current or previous sequence number.
// FUNCTION: 0x000c1bc8
void __stdcall RequestCompositeExchangeDataMsgCallback(u32 id, u32 seq, void *data)
{
    if (seq == sequenceNumber || seq == ((sequenceNumber - 1) & 0xf)) {
        if (spPtr->Send(id, compositeBuffer) != Lb_OK)
            fn_000c20c0("  WARNING network: RequestCompositeExchangeDataMsgCallback() failure on SP::Send()\n");
    } else {
        fn_000c20c0("  WARNING network: RequestCompositeExchangeDataMsgCallback() unexpected sequence number, got %d, expected %d\n", seq, sequenceNumber);
    }
}

// Creates and initialises the IPX service provider (replacing any other).
// Returns Lb_OK, or Lb_FAIL.
// FUNCTION: 0x000c1c20
static TbError GenericIPXInit(__tibcaGUID guid)
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
            fn_000c20c0("  WARNING network: GenericIPXInit() failure on ServiceProvider::Init()\n");
    }
    return ret;
}

// Sends system message type with size bytes of buf to player id. Returns
// the send's result, or Lb_FAIL if it does not fit.
// FUNCTION: 0x000c1d5c
static TbError SendSystemUserMessage(u32 id, TbMessageType type, void *buf, u32 size)
{
    TbError ret = Lb_FAIL;

    if (size + 5 > 0x404) {
        fn_000c20c0("  WARNING network: SendSystemUserMessage() systemUserBuffer is too small\n", type);
    } else {
        EncodeMessageStub(systemUserBuffer, size + 1, (TbMessageType)4, 0);
        systemUserBuffer[4] = type;
        if (buf && size > 0)
            memcpy(systemUserBuffer + 5, buf, size);
        ret = spPtr->Send(id, systemUserBuffer);
    }
    return ret;
}

static void PlayerMapMsgHandler(u32 id, void *buf, u32 size);

// Provider callback for a system message from player id: passes player-map
// messages on to PlayerMapMsgHandler.
// FUNCTION: 0x000c1de8
void __stdcall SystemUserMsgCallback(u32 id, void *msg, u32 size, void *data)
{
    if (msg == 0 || size == 0)
        return;
    switch (*(TbMessageType *)msg) {
    case MSG_TYPE_PLAYER_MAP:
        PlayerMapMsgHandler(id, (u8 *)msg + 1, size - 1);
        break;
    default:
        fn_000c20c0("  WARNING network: SystemUserMsgCallback() illegal sysMsgType %d\n", *(TbMessageType *)msg);
        break;
    }
}

// The module's three inline functions are each emitted in their own segment after the
// module's code, and the linker places those segments in reverse order of generation. The
// compiler pads an inline function's segment for the next function's alignment only when that
// function is inline too, so the original's padding (the session entry's constructor alone
// padded) needs the generation order below: these two before PlayerMapMsgHandler, the
// IPXServiceProvider constructor last. In the module's segment (built without -zm), the empty
// fn_000c20c0 would let the optimiser drop every call to it.

// Constructs an empty session entry.
// FUNCTION: 0x000c20d0 COMDAT
inline TbNetworkSessionNameEntry::TbNetworkSessionNameEntry()
{
    id = 0;
    in_use = 0;
    name[0] = 0;
}

extern "C" {

// Debug printf; does nothing in this build.
// FUNCTION: 0x000c20c0 COMDAT
inline void fn_000c20c0(char *fmt, ...)
{
}

}

// Player-map message: an empty one asks for the player table, which is sent
// back; a full one answers this player's request and replaces the table.
// FUNCTION: 0x000c1f24
static void PlayerMapMsgHandler(u32 id, void *buf, u32 size)
{
    u32 table_size = maximumPlayers * sizeof(__4kqpm7ClientDataEntry);

    if (size == 0) {
        SendSystemUserMessage(id, MSG_TYPE_PLAYER_MAP, clientDataTable, table_size);
    } else if (waitingForPlayerMapResponse) {
        if (size == table_size) {
            memcpy(clientDataTable, buf, size);
            waitingForPlayerMapResponse = 0;
        } else {
            fn_000c20c0("  WARNING network: PlayerMapMsgHandler() invalid length %d\n", size);
        }
    } else {
        fn_000c20c0("  WARNING network: PlayerMapMsgHandler() received unexpected SU_PLAYER_MAP_MSG\n");
    }
}

// Constructs the IPX service provider.
// FUNCTION: 0x000c2020 COMDAT
inline IPXServiceProvider::IPXServiceProvider()
{
}

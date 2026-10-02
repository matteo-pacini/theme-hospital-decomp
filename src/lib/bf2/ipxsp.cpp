#include "net.h"

#define MSG_BUCI 0x49435542
#define MSG_BUCJ 0x4a435542
#define MSG_BUCL 0x4c435542
#define MSG_BUDG 0x47445542
#define MSG_BUDN 0x4e445542
#define MSG_BUHI 0x49485542
#define MSG_BUHL 0x4c485542
#define MSG_BUHP 0x50485542
#define MSG_BUHQ 0x48515542
#define MSG_BUNP 0x504e5542
#define MSG_BUQP 0x50515542
#define MSG_BUSA 0x41535542
#define MSG_BUSD 0x44535542
#define MSG_BUSK 0x4b535542

extern "C" {
char *strcat(char *dst, const char *src);
#pragma intrinsic(strcat)
unsigned long strtoul(const char *nptr, char **endptr, int base);
void IPXServiceProvider_AddIPXSession(IPXServiceProvider *sp, u8 *info, u8 *address);
#pragma aux IPXServiceProvider_AddIPXSession "W?AddIPXSession$:IPXServiceProvider$n(pn$__1snc1aHOSTINFOPACKET$$pn$__3ljac5NETSOCKET$$)v"
}

// No settings to change over IPX. Returns Lb_OK.
// FUNCTION: 0xd5350
TbError IPXServiceProvider::ChangeSettings(u32 setting, void *data)
{
    return Lb_OK;
}

// Lets new players join the running session (allow nonzero) or not. Returns
// Lb_OK, or Lb_FAIL if no session is running.
// FUNCTION: 0xd5354
TbError IPXServiceProvider::EnableNewPlayers(u32 allow)
{
    TbError ret = Lb_FAIL;

    if (in_session) {
        ret = Lb_OK;
        accepting = allow;
    }
    return ret;
}

// Destroys the IPX service provider.
// FUNCTION: 0xd5374
IPXServiceProvider::~IPXServiceProvider()
{
    ServiceProvider::~ServiceProvider();
    session_count = 0;
}

// Initialises the IPX provider for game guid with callbacks: allocates the
// receive queue, opens a socket and starts listening. Returns Lb_OK, or
// Lb_FAIL.
// FUNCTION: 0xd5428
TbError IPXServiceProvider::Init(__tibcaGUID guid, __tibcaGUID *pguid, __ic76sReceiveCallbacks *callbacks, void *data)
{
    TbError ret = Lb_FAIL;
    u32 ok = 0;
    TbError alloc;

    alloc = AllocateSpStuff();
    if (alloc == Lb_FAIL)
        return alloc;
    if (Initialise(callbacks, data) == Lb_OK) {
        ok = 1;
        ret = Lb_OK;
    } else {
        NetworkDebugPrintf("  WARNING network: IPXServiceProvider::Init() failure on Initialise()\n");
    }
    if (ok) {
        NetworkDebugPrintf("MAX PACKET SIZE = %d\n", 0x212);
        initialised = 1;
        this->guid = guid;
        session_count = 0;
        memset(session_list, 0, sizeof(session_list));
        host_state = 0;
        players = 0;
        accepting = 1;
    } else {
        initialised = ok;
    }
    return ret;
}

void IPXServiceProvider::BatchProcess(u8 delay)
{
    static u32 start_batch_time = clock();
    static u32 start_time = clock();
    static u32 no_batchs;
    u32 now;

    now = clock();
    if (delay) {
        if (now - start_time > 50) {
            start_time = now;
            no_batchs++;
            ipx.BatchProcess();
        }
    } else {
        start_time = now;
        no_batchs++;
        ipx.BatchProcess();
    }
    if (now - start_batch_time >= 100) {
        start_batch_time = now;
        no_batchs = 0;
    }
}

// Base initialisation plus IPX setup: opens a dynamic socket and queues 32
// receive packets. Returns Lb_OK, or Lb_FAIL.
// FUNCTION: 0xd5510
TbError IPXServiceProvider::Initialise(__ic76sReceiveCallbacks *cb, void *data)
{
    IPXPacket *p;
    TbError ret = Lb_FAIL;

    NetworkDebugPrintf("  LbNETWORK : Initialising IPXServiceProvider\n");
    if (ServiceProvider::Initialise(cb, data) == Lb_OK) {
        if (ipx.Init() == 0) {
            socket.socket = 0;
            if (ipx.OpenSocket(&socket, 0) == 0) {
                packets = ipx.AllocIPXPackets(0x20, &packets_sel);
                if (packets) {
                    for (p = packets; p < packets + 0x20; p++)
                        ipx.Listen(p, &socket);
                    BatchProcess(0);
                    ret = Lb_OK;
                } else {
                    ipx.CloseSocket(&socket);
                    ipx.DeInit();
                }
            } else {
                ipx.DeInit();
            }
        } else {
            ServiceProvider::Release();
        }
    } else {
        NetworkDebugPrintf("  WARNING network: SERVICE PROVIDER FAILURE\n");
    }
    return ret;
}

// Stops any session, closes the socket, shuts down IPX and frees the
// provider's memory. Returns Lb_FAIL if not initialised, else the base
// Release's result.
// FUNCTION: 0xd569c
TbError IPXServiceProvider::Release(void)
{
    NetworkDebugPrintf("  LbNETWORK : Releasing IPXServiceProvider\n");
    if (!initialised) {
        NetworkDebugPrintf("  WARNING network: IPXServiceProvider::Release() not initialised\n");
        return Lb_FAIL;
    }
    if (in_session)
        Stop();
    memset(session_list, 0, sizeof(session_list));
    host_state = 0;
    ipx.CloseSocket(&socket);
    ipx.DeInit();
    initialised = 0;
    if (packets)
        ipx.FreeIPXPackets(packets_sel);
    FreeAllocSpStuff();
    return ServiceProvider::Release();
}

extern "C" {

TbError IPXServiceProvider_SendMessage(IPXServiceProvider *sp, u32 to, void *buf, u8 guaranteed);
#pragma aux IPXServiceProvider_SendMessage "W?SendMessage$:IPXServiceProvider$n(ulpnv$$TE$$)$TbError$$"
u32 IPXServiceProvider_ReadMessage(IPXServiceProvider *sp, u32 *from, void *buf, u32 *size);
#pragma aux IPXServiceProvider_ReadMessage "W?ReadMessage$:IPXServiceProvider$n(pnulpnvpnul)ul"
u32 IPXServiceProvider_PeekMessage(IPXServiceProvider *sp, u32 *from, void *buf, u32 *size);
#pragma aux IPXServiceProvider_PeekMessage "W?PeekMessage$:IPXServiceProvider$n(pnulpnvpnul)ul"

// Hosts session session_name (up to max_players, at most 32) as player
// player_name: advertises it over SAP as "BU.GI.<guid>" and makes this
// player 0 and the host. Returns Lb_OK, or Lb_FAIL.
// FUNCTION: 0xd5760
TbError IPXServiceProvider::Start(char *session_name, char *player_name, u32 max_players, void *data)
{
    u32 size;
    TbError ret = Lb_FAIL;
    u32 i;

    if (!initialised) {
        NetworkDebugPrintf("  WARNING network: IPXServiceProvider::Start() not initialised\n");
        return Lb_FAIL;
    }
    if (in_session)
        Stop();
    if (max_players > 32)
        return Lb_FAIL;
    if (max_players > 5)
        size = max_players * sizeof(IPXPlayerEntry);
    else
        size = 5 * sizeof(IPXPlayerEntry);
    players = (IPXPlayerEntry *)LbMemoryAlloc(size);
    if (!players)
        return Lb_FAIL;
    memset(players, 0, size);
    strcpy(server_name, "BU.GI.");
    ConvertGuidToString(&guid, server_name + 6);
    NetworkDebugPrintf("%s\n", server_name);
    if (ipx.AdvertiseServer(0xbe08, server_name, &socket) == 0) {
        session.session_id = 0;
        session.host_id = 0;
        session.guid = guid;
        session.num_players = 1;
        local_id = 0;
        for (i = 0; i < 32; i++) {
            send_dseq[i] = 0;
            recv_dseq[i] = 0;
        }
        session.max_players = max_players;
        msg_seq = 0;
        StrCpyTrunc(session.name, session_name, 0x20);
        host_state = 1;
        players[0].id = 0;
        strcpy(players[0].name, player_name);
        memcpy(&players[0].addr, &socket, sizeof(__3ljac5NETSOCKET));
        ret = Lb_OK;
        ClearPlayers();
        if (AddPlayer(local_id, player_name, 1, 1) != Lb_OK) {
            NetworkDebugPrintf("  WARNING network: IPXServiceProvider::Start() failure on AddPlayer()\n");
        } else {
            in_session = 1;
            accepting = 1;
        }
    } else {
        LbMemoryFree(players);
        players = 0;
        NetworkDebugPrintf(" COULD NOT ADVERTISE SERVICE\n");
    }
    return ret;
}

// Joins the session listed as entry: asks its host for its details, sends a
// join request (resent every 25 ticks for up to 10 seconds) and fetches the
// player list. Returns Lb_FAIL if not initialised or already in a session,
// if the session is full or memory runs out; otherwise Lb_OK, also when
// entry lists no session, the host does not answer or the player list
// cannot be fetched (then without joining).
// FUNCTION: 0xd59ec
// NONMATCHING: stack slots only: the target places done and begin above the other dword
// temporaries, and orders the ret and inlined BatchProcess argument bytes differently
TbError IPXServiceProvider::Start(TbNetworkSessionNameEntry *entry, char *player_name, void *data)
{
    __35ct5sCLIENTJOINPACKET join;
    u8 *info;
    __3ljac5NETSOCKET from;
    __3ljac5NETSOCKET server;
    u8 *reply;
    IPXPacket *p;
    u32 max;
    TbError ret = Lb_FAIL;
    u32 num;
    u32 start;
    u32 last;
    u32 now;
    u32 size;
    u32 found = 0;
    u32 done;
    u32 i;
    __5303ctCLIENTINFOPACKET query;
    u32 begin;

    if (initialised && !in_session) {
        ret = Lb_OK;
        if (entry->id < 32 && *(u16 *)session_list[entry->id].address) {
            memcpy(&server, session_list[entry->id].address, sizeof(server));
            memset(&query, 0, sizeof(query));
            query.magic = MSG_BUHQ;
            query.guid = guid;
            ipx.Send(&socket, &server, (u8 *)&query, sizeof(query));
            BatchProcess(0);
            start = clock();
            while (found == 0) {
                if (clock() - start > 200)
                    break;
                for (p = packets; p < packets + 32; p++) {
                    info = p->data;
                    if (ipx.RecPkt(p, &socket, &server)) {
                        if (BullfrogIPXMessage((__407tigIPXMESSAGEBLOCK *)info, (__tibcaGUID *)(info + 0xc)) == Lb_OK
                            && ((IPXSessionInfo *)info)->magic == MSG_BUHI) {
                            found = 1;
                            max = ((IPXSessionInfo *)info)->max_players;
                            num = ((IPXSessionInfo *)info)->num_players;
                            ipx.Listen(p, &socket);
                            break;
                        }
                        ipx.Listen(p, &socket);
                    }
                }
            }
            if (found == 1) {
                if (num >= max)
                    return Lb_FAIL;
                if (max > 5)
                    size = max * sizeof(IPXPlayerEntry);
                else
                    size = 5 * sizeof(IPXPlayerEntry);
                players = (IPXPlayerEntry *)LbMemoryAlloc(size);
                if (!players)
                    return Lb_FAIL;
                memset(players, 0, size);
                memset(&join, 0, sizeof(join));
                join.magic = MSG_BUCJ;
                join.guid = guid;
                memcpy(&join.addr, &socket, sizeof(__3ljac5NETSOCKET));
                StrCpyTrunc(join.name, player_name, 0x20);
                ipx.Send(&socket, &server, (u8 *)&join, sizeof(join));
                BatchProcess(0);
                last = begin = clock();
                done = 0;
                while (done == 0) {
                    now = clock();
                    if (now - begin > 1000)
                        break;
                    if (now - last > 25) {
                        last = now;
                        ipx.Send(&socket, &server, (u8 *)&join, sizeof(join));
                        BatchProcess(0);
                    }
                    for (p = packets; p < packets + 32; p++) {
                        reply = p->data;
                        if (ipx.RecPkt(p, &socket, &from)) {
                            if (BullfrogIPXMessage((__407tigIPXMESSAGEBLOCK *)reply, (__tibcaGUID *)(reply + 0xc)) == Lb_OK
                                && *(u32 *)reply == MSG_BUNP && !memcmp(&socket, reply + 0x74, sizeof(__3ljac5NETSOCKET))) {
                                memset(&remote, 0, sizeof(remote));
                                strcpy(remote.name, session_list[entry->id].name);
                                remote.session_id = entry->id;
                                remote.guid = guid;
                                remote.local_id = *(u32 *)(reply + 0x50);
                                remote.host_id = *(u32 *)(reply + 0x40);
                                ipx.Listen(p, &socket);
                                done = 1;
                                break;
                            }
                            ipx.Listen(p, &socket);
                        }
                    }
                }
                if (done == 1) {
                    ClearPlayers();
                    if (EnumerateAndAddPlayers(&from) == Lb_OK) {
                        host_state = 2;
                        msg_seq = 0;
                        in_session = 1;
                        for (i = 0; i < 32; i++) {
                            send_dseq[i] = 0;
                            recv_dseq[i] = 0;
                        }
                        ret = Lb_OK;
                        local_id = remote.local_id;
                    } else {
                        LbMemoryFree(players);
                    }
                } else {
                    LbMemoryFree(players);
                }
            }
        }
    }
    return ret;
}

// Leaves the running session, telling the other players (BUSD) after a
// short wait, and frees the player list. Returns Lb_FAIL.
// FUNCTION: 0xd63e4
TbError IPXServiceProvider::Stop(void)
{
    __1p5pi1IPXSYSTEMMESSAGE msg;
    u32 start;
    u32 me;
    u32 num;
    u32 i;

    if (initialised && in_session) {
        start = LbTimerClock();
        while (LbTimerClock() - start < 700)
            ;
        accepting = 0;
        if (host_state == 1 || host_state == 2) {
            if (host_state == 1) {
                me = session.host_id;
                num = session.num_players;
            } else {
                me = remote.local_id;
                num = remote.num_players;
            }
            if (num > 1) {
                memset(&msg, 0, sizeof(msg));
                msg.magic = MSG_BUSD;
                msg.seq = msg_seq++;
                msg.guid = guid;
                msg.host = me;
                memcpy(&msg.player, &players[me], sizeof(IPXPlayerEntry));
                BroadcastSystemMessage(&msg);
            }
            if (players) {
                LbMemoryFree(players);
                players = 0;
                host_state = 0;
                msg_seq = 0;
                for (i = 0; i < 32; i++) {
                    send_dseq[i] = 0;
                    recv_dseq[i] = 0;
                }
            }
        }
        in_session = 0;
        ClearSpStuff();
    }
    return Lb_FAIL;
}

// Sends message buf to player to, guaranteed (BUDG) or not (BUDN).
// Returns SendMessageToPlayerId's result.
// FUNCTION: 0x000d6550
TbError IPXServiceProvider_SendMessage(IPXServiceProvider *sp, u32 to, void *buf, u8 guaranteed)
{
    u32 magic;
    TbError ret;

    switch (guaranteed) {
    case 0:
        magic = MSG_BUDN;
        break;
    case 1:
        magic = MSG_BUDG;
        break;
    }
    ret = sp->SendMessageToPlayerId(to, buf, magic);
    if (ret == Lb_FAIL)
        NetworkDebugPrintf("ERROR SendMessageToPlayer\n");
    return ret;
}

// Takes the next received message (see IPXServiceProvider_PrivateReceive).
// FUNCTION: 0xd6588
u32 IPXServiceProvider_ReadMessage(IPXServiceProvider *sp, u32 *from, void *buf, u32 *size)
{
    return IPXServiceProvider_PrivateReceive(sp, from, buf, size, 0);
}

// Looks at the next received message without taking it (see
// IPXServiceProvider_PrivateReceive).
// FUNCTION: 0xd6590
u32 IPXServiceProvider_PeekMessage(IPXServiceProvider *sp, u32 *from, void *buf, u32 *size)
{
    return IPXServiceProvider_PrivateReceive(sp, from, buf, size, 1);
}

}

static inline IPXQueueEntry *GetNextFullReceiveBuffer(IPXServiceProvider *self)
{
    IPXQueueEntry *entry;
    u32 found = 0;

    for (entry = self->queue; entry < self->queue + 32; entry++) {
        if (entry->used) {
            found = 1;
            break;
        }
    }
    if (found == 1)
        return entry;
    return 0;
}

// Copies the next queued received message (and its sender, into *from)
// into buf if it fits in *size, setting *size; with peek 0 it is removed
// from the queue. With nothing queued, sets *size to 0 and polls the
// network. Returns 1 if a message was copied, else 0.
// FUNCTION: 0xd6598
u32 IPXServiceProvider_PrivateReceive(IPXServiceProvider *self, u32 *from, void *buf, u32 *size, u8 peek)
{
    IPXQueueEntry *entry;
    u32 msg_size;
    u32 ret = 0;

    entry = GetNextFullReceiveBuffer(self);
    if (entry) {
        *from = entry->from;
        DecodeMessageStub(entry, &msg_size, 0, 0);
        if (msg_size + 4 <= *size) {
            ret = 1;
            memcpy(buf, entry, msg_size + 4);
            *size = msg_size + 4;
            if (!peek)
                entry->used = 0;
        }
    } else {
        *size = 0;
        self->Update();
    }
    return ret;
}

// Calls callback with the running session, or else queries the network
// for sessions of this game (SAP, then a host query to each), keeps the
// session list (dropping ones silent for 3000 ticks) and calls callback with
// each. Returns Lb_OK, or Lb_FAIL if not initialised.
// FUNCTION: 0xd6668
// NONMATCHING: stack slots; the session-expiry loop keeps the memset size 0x34 in ebp where
// ours keeps 0, and the AddSession name pointer is built in another register
TbError IPXServiceProvider::Enumerate(TbNetworkCallbackFunc callback, void *data)
{
    IPXPacket *p;
    u32 now;
    TbError ret = Lb_FAIL;
    __5303ctCLIENTINFOPACKET query;
    u8 *info;
    s32 i;
    __3ljac5NETSOCKET from;
    u8 *sap;
    TbNetworkSessionNameEntry *session_entry;

    if (initialised) {
        if (in_session) {
            if (host_state == 1) {
                strcpy(entry.name, session.name);
                entry.id = session.session_id;
            } else if (host_state == 2) {
                strcpy(entry.name, remote.name);
                entry.id = remote.session_id;
            }
            callback((TbNetworkCallbackData *)&entry, data);
            ret = Lb_OK;
        } else {
            ClearSessions();
            now = clock();
            ret = Lb_OK;
            ipx.QueryServer(0x100, 0xbe08, &socket);
            for (p = packets; p < packets + 32; p++) {
                if (!ipx.RecPkt(p, &socket, &from))
                    continue;
                sap = p->data;
                if (*(u16 *)sap == 0x200) {
                    if (BullfrogIPXServer((char *)sap + 4) == Lb_OK) {
                        memcpy(from.node, sap + 0x38, 6);
                        memcpy(from.net, sap + 0x34, 4);
                        from.socket = *(u16 *)(sap + 0x3e);
                        memset(&query, 0, sizeof(query));
                        query.magic = MSG_BUHQ;
                        query.guid = guid;
                        ipx.Send(&socket, &from, (u8 *)&query, sizeof(query));
                    }
                } else {
                    info = p->data;
                    if (BullfrogIPXMessage((__407tigIPXMESSAGEBLOCK *)info, (__tibcaGUID *)(info + 0xc)) == Lb_OK
                        && *(u32 *)info == MSG_BUHI) {
                        for (i = 0; i <= session_count; i++) {
                            if (memcmp(&session_list[i].address[10], from.node, 6) == 0) {
                                session_list[i].time = now;
                                memcpy(session_list[i].address, &from, 16);
                                StrCpyTrunc(session_list[i].name, (char *)info + 0x1c, 0x20);
                                i = -1;
                                break;
                            }
                        }
                        if (i != -1) {
                            for (i = 0; i <= session_count; i++) {
                                if (session_list[i].time == 0) {
                                    memcpy(session_list[i].address, &from, 16);
                                    StrCpyTrunc(session_list[i].name, (char *)info + 0x1c, 0x20);
                                    session_list[i].time = now;
                                    i = -1;
                                    break;
                                }
                            }
                            if (i != -1 && session_count < 32)
                                session_count++;
                        }
                    }
                }
                ipx.Listen(p, &socket);
            }
            BatchProcess(0);
            for (i = 0; i <= session_count; i++) {
                if (session_list[i].time && now - session_list[i].time > 3000)
                    memset(&session_list[i], 0, sizeof(IPXSessionEntry));
            }
            for (i = 0; i <= session_count; i++) {
                if (session_list[i].time) {
                    session_entry = AddSession(i, session_list[i].name);
                    callback((TbNetworkCallbackData *)session_entry, data);
                    ret = Lb_OK;
                }
            }
        }
    }
    return ret;
}

// Player data handed to Enumerate's callback.
struct IPXEnumPlayer {
    u8 unknown0;
    u32 id;
    // Nonzero for the session's host.
    u32 host;
    char name[0x20];
};

// Calls callback with each player of session: the running one, or a listed
// one whose host it asks page by page, giving up after 10 seconds without
// an answer. Returns Lb_FAIL if not initialised, or if the host does not
// answer or the pages run out (the session is then dropped from the list);
// otherwise Lb_OK.
// count is read uninitialised if the first reply already reports 32
// players.
// FUNCTION: 0xd6e2c
// NONMATCHING: frame 4 bytes smaller, stack slots and register choice; done is tested as a byte
// here, the target loads it with the sar trick
TbError IPXServiceProvider::Enumerate(TbNetworkSessionNameEntry *session, TbNetworkCallbackFunc callback, void *data)
{
    u32 count;
    u32 time;
    IPXPacket *p;
    __3ljac5NETSOCKET from;
    u32 now;
    u8 *info;
    __3ljac5NETSOCKET *server;
    IPXPlayerEntry *entry;
    u32 start;
    u32 last;
    __5303ctCLIENTINFOPACKET query;
    TbError ret = Lb_FAIL;
    u32 total = 32;
    u32 max = 32;
    IPXEnumPlayer player;
    s8 done = 0;
    u32 i;

    if (initialised) {
        if (in_session) {
            ret = EnumeratePlayersForSessionRunning(callback, data);
        } else if (session->id < 32) {
            if (session_list[session->id].time) {
                memset(&query, 0, sizeof(query));
                query.magic = MSG_BUQP;
                query.guid = guid;
                query.page = 0;
                server = (__3ljac5NETSOCKET *)session_list[session->id].address;
                last = clock();
                start = clock();
                ipx.Send(&socket, server, (u8 *)&query, sizeof(query));
                BatchProcess(0);
                while (!done) {
                    now = clock();
                    time = clock();
                    if (time - start > 1000) {
                        done = -1;
                        break;
                    }
                    if (now - last > 25) {
                        last = now;
                        server = (__3ljac5NETSOCKET *)session_list[session->id].address;
                        ipx.Send(&socket, server, (u8 *)&query, sizeof(query));
                        BatchProcess(0);
                    }
                    for (p = packets; p < packets + 32; p++) {
                        if (!ipx.RecPkt(p, &socket, &from))
                            continue;
                        info = p->data;
                        if (BullfrogIPXMessage((__407tigIPXMESSAGEBLOCK *)info, (__tibcaGUID *)(info + 0xc)) == Lb_OK) {
                            switch (*(u32 *)info) {
                            case MSG_BUHI:
                                IPXServiceProvider_AddIPXSession(this, info, (u8 *)&from);
                                break;
                            case MSG_BUHP:
                                if (memcmp(from.node, &session_list[session->id].address[10], 6))
                                    break;
                                if (((IPXSessionInfo *)info)->num_players == total) {
                                    if (((IPXSessionInfo *)info)->page == query.page) {
                                        start = time;
                                        last = now;
                                        player.id = 0;
                                        for (i = 0, entry = (IPXPlayerEntry *)(info + 0x50); i < 6; i++, entry++) {
                                            if (entry->addr.socket) {
                                                count++;
                                                player.host = entry->id == ((IPXSessionInfo *)info)->host_id;
                                                strcpy(player.name, entry->name);
                                                player.id = entry->id;
                                                callback((TbNetworkCallbackData *)&player, data);
                                            }
                                        }
                                        if (count >= total) {
                                            done = 1;
                                            ipx.Listen(p, &socket);
                                        } else if (++query.page >= max * 6) {
                                            done = -1;
                                            ipx.Listen(p, &socket);
                                        } else {
                                            ipx.Send(&socket, server, (u8 *)&query, sizeof(query));
                                            BatchProcess(0);
                                        }
                                    }
                                } else {
                                    total = ((IPXSessionInfo *)info)->num_players;
                                    count = 0;
                                    max = ((IPXSessionInfo *)info)->max_players;
                                    query.page = 0;
                                    now = time = start = last = clock();
                                    ipx.Send(&socket, server, (u8 *)&query, sizeof(query));
                                    BatchProcess(0);
                                }
                                break;
                            }
                            if (done != 0)
                                break;
                        }
                        ipx.Listen(p, &socket);
                    }
                }
            }
            if (done == -1) {
                memset(&session_list[session->id], 0, sizeof(IPXSessionEntry));
                return Lb_FAIL;
            }
            ret = Lb_OK;
        }
    }
    return ret;
}

// Polls the IPX packets: as host, answers session and player queries and
// join requests and keeps advertising; as host or client, acknowledges
// system messages and queues data messages. Returns Lb_OK if a session is
// running, else Lb_FAIL.
// FUNCTION: 0xd7768
TbError IPXServiceProvider::Update(void)
{
    u8 *data;
    IPXPacket *p;
    __3ljac5NETSOCKET from;
    u8 info[0x54];
    u8 join[0x8c];
    TbError ret = Lb_FAIL;

    if (in_session) {
    if (host_state == 1) {
        for (p = packets; p < packets + 32; p++) {
            if (ipx.RecPkt(p, &socket, &from)) {
                data = p->data;
                if (BullfrogIPXMessage((__407tigIPXMESSAGEBLOCK *)data, (__tibcaGUID *)(data + 0xc)) == Lb_OK)
                switch (*(u32 *)data) {
                case MSG_BUHQ:
                    if (accepting) {
                        session.magic = MSG_BUHI;
                        ipx.Listen(p, &socket);
                        ipx.Send(&socket, &from, (u8 *)&session, sizeof(IPXSessionInfo));
                        BatchProcess(0);
                    }
                    break;
                case MSG_BUQP:
                    memcpy(info, data, sizeof(info));
                    ipx.Listen(p, &socket);
                    SendPlayerInfo((__5303ctCLIENTINFOPACKET *)info, &from);
                    BatchProcess(0);
                    break;
                case MSG_BUCJ:
                    memcpy(join, data, sizeof(join));
                    ipx.Listen(p, &socket);
                    AddPlayerToSession((__35ct5sCLIENTJOINPACKET *)join, &from);
                    BatchProcess(0);
                    break;
                case MSG_BUSA:
                case MSG_BUSD:
                    ResponsedToSystemMessage((__1p5pi1IPXSYSTEMMESSAGE *)data, &from);
                    ipx.Listen(p, &socket);
                    break;
                case MSG_BUSK:
                    ipx.Listen(p, &socket);
                    break;
                case MSG_BUDG:
                case MSG_BUDN:
                default:
                    ProcessDataMessage((__6eas9nIPXDATAMESSAGE *)data);
                    ipx.Listen(p, &socket);
                    break;
                }
            }
        }
        if (accepting)
            ipx.UpdateServer(0xbe08, server_name, &socket);
        BatchProcess(1);
    } else if (host_state == 2) {
        for (p = packets; p < packets + 32; p++) {
            if (ipx.RecPkt(p, &socket, &from)) {
                data = p->data;
                if (BullfrogIPXMessage((__407tigIPXMESSAGEBLOCK *)data, (__tibcaGUID *)(data + 0xc)) == Lb_OK)
                switch (*(u32 *)data) {
                case MSG_BUSA:
                case MSG_BUSD:
                    ResponsedToSystemMessage((__1p5pi1IPXSYSTEMMESSAGE *)data, &from);
                    ipx.Listen(p, &socket);
                    break;
                case MSG_BUSK:
                    ipx.Listen(p, &socket);
                    break;
                case MSG_BUDG:
                case MSG_BUDN:
                default:
                    ProcessDataMessage((__6eas9nIPXDATAMESSAGE *)data);
                    ipx.Listen(p, &socket);
                    break;
                }
            }
        }
        BatchProcess(1);
    }
    ret = Lb_OK;
    }
    return ret;
}

// Returns Lb_OK if SAP server name is this game's ("BU.GI." and its guid),
// else Lb_FAIL.
// FUNCTION: 0xd8138
TbError IPXServiceProvider::BullfrogIPXServer(char *name)
{
    __tibcaGUID id;
    TbError ret = Lb_FAIL;

    if (memcmp(name, "BU.GI.", 6) == 0) {
        ConvertStringToGuid(name + 6, &id);
        if (memcmp(&guid, &id, sizeof(__tibcaGUID)) == 0)
            ret = Lb_OK;
    }
    return ret;
}

// Writes guid as four hex words, each followed by '.', into str.
// FUNCTION: 0xd8214
void IPXServiceProvider::ConvertGuidToString(__tibcaGUID *guid, char *str)
{
    char buf[0xc];
    u32 *word = guid->data;
    u32 i;

    str[0] = 0;
    for (i = 0; i < 16; i += 4, word++) {
        sprintf(buf, "%x.", *word);
        strcat(str, buf);
    }
}

// Reads four '.'-separated hex words from str into guid.
// FUNCTION: 0xd8274
void IPXServiceProvider::ConvertStringToGuid(char *str, __tibcaGUID *guid)
{
    char buf[0x14];
    u32 *word = guid->data;
    char *p = str;
    u32 j;
    u8 i;

    i = 0;
    while (i < 4) {
        memset(buf, 0, sizeof(buf));
        for (j = 0; j < strlen(str); j++) {
            if (*p == '.') {
                p++;
                break;
            }
            buf[j] = *p++;
        }
        *word++ = strtoul(buf, 0, 16);
        i++;
    }
}

// Host: sends a page of the player list (six players, page info->page) to
// `to`. Returns Lb_OK, or Lb_FAIL if not hosting or the page is past the
// end.
// FUNCTION: 0xd82f4
TbError IPXServiceProvider::SendPlayerInfo(__5303ctCLIENTINFOPACKET *info, __3ljac5NETSOCKET *to)
{
    u8 buf[0x1a0];
    IPXPlayerEntry *p;
    IPXPlayerEntry *dst;
    u32 skip;
    u32 i;
    TbError ret = Lb_FAIL;

    if (initialised && in_session && host_state == 1) {
        memset(buf, 0, sizeof(buf));
        memcpy(buf, &session, sizeof(IPXSessionInfo));
        ((IPXSessionInfo *)buf)->page = info->page;
        ((IPXSessionInfo *)buf)->magic = MSG_BUHP;
        ((IPXSessionInfo *)buf)->guid = guid;
        skip = info->page * 6;
        if (skip < session.max_players) {
            p = players;
            if (skip) {
                for (i = 0; p < players + session.max_players && i < skip; p++) {
                    if (p->addr.socket)
                        i++;
                }
            }
            IPXPlayerEntry *end;
            dst = (IPXPlayerEntry *)(buf + 0x50);
            end = dst + 6;
            do {
                if (p >= players + session.max_players)
                    break;
                if (p->addr.socket) {
                    memcpy(dst, p, sizeof(IPXPlayerEntry));
                    dst++;
                }
                p++;
            } while (dst < end);
            ipx.Send(&socket, to, buf, sizeof(buf));
            BatchProcess(0);
            ret = Lb_OK;
        }
    }
    return ret;
}

// Host: adds the player asking to join (if there is room and new players
// are allowed), replies with their slot (BUNP) and tells everyone (BUSA).
// Returns Lb_FAIL if not hosting or AddPlayer fails, else Lb_OK (also when
// the session is full or the player is already in it).
// If no free slot is found, BUSA still goes out with players[i], one past
// the table.
// FUNCTION: 0xd852c
// NONMATCHING: frame layout and register allocation differ
TbError IPXServiceProvider::AddPlayerToSession(__35ct5sCLIENTJOINPACKET *join, __3ljac5NETSOCKET *from)
{
    __1p5pi1IPXSYSTEMMESSAGE msg;
    IPXPlayerEntry *p;
    u32 i;
    u8 buf[0x88];
    TbError ret = Lb_FAIL;

    if (initialised && in_session && host_state == 1) {
        ret = Lb_OK;
        if (session.num_players < session.max_players) {
            if (CheckIfPlayerWantingToJoinIsInSession(join, from) == 1)
                return Lb_OK;
            if (accepting == 1) {
                for (i = 0, p = players; p < players + session.max_players; i++, p++) {
                    if (!p->addr.socket) {
                        memcpy(&p->addr, &join->addr, sizeof(p->addr));
                        strcpy(p->name, join->name);
                        p->id = i;
                        if (AddPlayer(i, p->name, 0, 0) != Lb_OK) {
                            NetworkDebugPrintf("  WARNING network: IPXServiceProvider::Start() failure on AddPlayer()\n");
                            return Lb_FAIL;
                        }
                        session.num_players++;
                        memset(buf, 0, sizeof(buf));
                        memcpy(buf, &session, sizeof(IPXSessionInfo));
                        memcpy(buf + 0x50, p, sizeof(IPXPlayerEntry));
                        *(u32 *)buf = MSG_BUNP;
                        *(u32 *)(buf + 4) = session.host_id;
                        ipx.Send(&socket, from, buf, sizeof(buf));
                        BatchProcess(0);
                        ret = Lb_OK;
                        break;
                    }
                }
                memset(&msg, 0, sizeof(msg));
                msg.magic = MSG_BUSA;
                msg.seq = msg_seq++;
                msg.host = session.host_id;
                msg.guid = guid;
                memcpy(&msg.player, &players[i], sizeof(IPXPlayerEntry));
                AddToReceiveBuffer((__6eas9nIPXDATAMESSAGE *)&msg);
                BroadcastSystemMessage(&msg);
            }
        }
    }
    return ret;
}

// Host: if the player asking to join is already in the session (same
// node), updates their socket and repeats the reply. Returns 1 if so, else
// 0.
// FUNCTION: 0xd8c58
u32 IPXServiceProvider::CheckIfPlayerWantingToJoinIsInSession(__35ct5sCLIENTJOINPACKET *join, __3ljac5NETSOCKET *from)
{
    IPXPlayerEntry *p;
    u8 buf[0x88];
    u32 ret = 0;

    for (p = players; p < players + session.max_players; p++) {
        if (!memcmp(p->addr.node, join->addr.node, 6)) {
            p->addr.socket = join->addr.socket;
            memset(buf, 0, sizeof(buf));
            memcpy(buf, &session, sizeof(IPXSessionInfo));
            memcpy(buf + 0x50, p, sizeof(IPXPlayerEntry));
            *(u32 *)buf = MSG_BUNP;
            ipx.Send(&socket, from, buf, sizeof(buf));
            BatchProcess(0);
            ret = 1;
            break;
        }
    }
    return ret;
}

// Client: fetches the session's player list from server page by page into
// the player tables, giving up after 30 seconds without an answer. Returns
// Lb_OK, or Lb_FAIL.
// FUNCTION: 0xd8e34
// NONMATCHING: stack slots, register choice in the inlined BatchProcess, and the
// target shares one Listen-and-break tail between the done and give-up paths
TbError IPXServiceProvider::EnumerateAndAddPlayers(__3ljac5NETSOCKET *server)
{
    u32 total = 32;
    IPXSessionInfo *msg;
    u32 max = 32;
    IPXPlayerEntry *entry;
    u32 last_send;
    u32 start;
    u32 now;
    IPXPacket *pk;
    __3ljac5NETSOCKET from;
    u32 added = 0;
    TbError ret = Lb_FAIL;
    u32 is_local;
    u32 is_host;
    s32 done = 0;
    u32 i;

    if (initialised) {
        remote.magic = MSG_BUQP;
        remote.page = 0;
        remote.unused_set_by_EnumerateAndAddPlayers = 0;
        last_send = clock();
        start = clock();
        for (;;) {
            now = clock();
            if (now - start > 3000)
                break;
            if (now - last_send > 25) {
                last_send = now;
                remote.magic = MSG_BUQP;
                ipx.Send(&socket, server, (u8 *)&remote, sizeof(remote));
                BatchProcess(0);
            }
            for (pk = packets; pk < packets + 32; pk++) {
                if (!ipx.RecPkt(pk, &socket, &from))
                    continue;
                msg = (IPXSessionInfo *)pk->data;
                if (BullfrogIPXMessage((__407tigIPXMESSAGEBLOCK *)pk->data, (__tibcaGUID *)(pk->data + 0xc)) == Lb_OK && msg->magic == MSG_BUHP) {
                    if (msg->num_players == total) {
                        if (msg->page == remote.page) {
                            last_send = now;
                            start = now;
                            for (i = 0, entry = (IPXPlayerEntry *)(msg + 1); i < 6; i++, entry++) {
                                if (entry->addr.socket) {
                                    added++;
                                    is_local = 0;
                                    is_host = 0;
                                    if (entry->id == remote.local_id)
                                        is_local = 1;
                                    if (entry->id == msg->host_id)
                                        is_host = 1;
                                    if (AddPlayer(entry->id, entry->name, is_local, is_host) != Lb_OK)
                                        NetworkDebugPrintf("  WARNING network: IPXServiceProvider::Start() failure on AddPlayer()\n");
                                    memcpy(&players[entry->id], entry, sizeof(IPXPlayerEntry));
                                }
                            }
                            if (added >= total) {
                                remote.num_players = msg->num_players;
                                done = 1;
                                remote.max_players = msg->max_players;
                                ret = Lb_OK;
                                ipx.Listen(pk, &socket);
                                break;
                            }
                            if (++remote.page >= max * 6) {
                                done = -1;
                                ipx.Listen(pk, &socket);
                                break;
                            }
                            ipx.Send(&socket, server, (u8 *)&remote, sizeof(remote));
                            BatchProcess(0);
                            now = start = last_send = clock();
                        }
                    } else {
                        total = msg->num_players;
                        max = msg->max_players;
                        memset(players, 0, max * sizeof(IPXPlayerEntry));
                        remote.page = 0;
                        added = 0;
                        now = start = last_send = clock();
                        ipx.Send(&socket, server, (u8 *)&remote, sizeof(remote));
                        BatchProcess(0);
                    }
                }
                ipx.Listen(pk, &socket);
            }
            if (done)
                break;
        }
    }
    return ret;
}

// Sends system message msg to every other player and waits up to 10
// seconds for all to acknowledge (resending every 50 ticks, answering
// queries meanwhile); players who do not are removed. Returns Lb_FAIL.
// FUNCTION: 0xd9480
// NONMATCHING: stack slots of last, start and the ret and inlined BatchProcess argument
// bytes, and the register of the ack store
TbError IPXServiceProvider::BroadcastSystemMessage(__1p5pi1IPXSYSTEMMESSAGE *msg)
{
    __3ljac5NETSOCKET from;
    IPXPlayerEntry *p;
    u32 start;
    IPXPacket *pk;
    u32 count;
    u32 me;
    u32 num;
    TbError ret = Lb_FAIL;
    __1p5pi1IPXSYSTEMMESSAGE *data;
    u32 last;
    u32 now;
    u32 done = 0;
    u32 max;

    if (initialised && in_session && (host_state == 1 || host_state == 2)) {
        if (host_state == 1) {
            max = session.max_players;
            me = session.host_id;
            num = session.num_players;
        } else {
            max = remote.max_players;
            me = remote.local_id;
            num = remote.num_players;
        }
        if (max > 1) {
            for (p = players; p < players + max; p++)
                p->ack = 0;
            for (p = players; p < players + max; p++) {
                if (p->addr.socket && p->id != me)
                    ipx.Send(&socket, &p->addr, (u8 *)msg, sizeof(*msg));
            }
            BatchProcess(0);
            last = clock();
            start = clock();
            while (!done) {
                now = clock();
                if (now - start > 1000)
                    break;
                if (now - last > 50) {
                    last = now;
                    for (p = players; p < players + max; p++) {
                        if (p->addr.socket && p->id != me)
                            ipx.Send(&socket, &p->addr, (u8 *)msg, sizeof(*msg));
                    }
                    BatchProcess(0);
                }
                for (pk = packets; pk < packets + 32; pk++) {
                    if (!ipx.RecPkt(pk, &socket, &from))
                        continue;
                    data = (__1p5pi1IPXSYSTEMMESSAGE *)pk->data;
                    switch (data->magic) {
                    case MSG_BUSK:
                        if (data->seq == msg->seq) {
                            players[data->player.id].ack = 1;
                            count = 1;
                            for (p = players; p < players + max; p++) {
                                if (p->ack)
                                    count++;
                            }
                            if (count >= num)
                                done = 1;
                        }
                        break;
                    case MSG_BUHQ:
                        if (host_state == 1 && accepting) {
                            session.magic = MSG_BUHI;
                            ipx.Send(&socket, &from, (u8 *)&session, sizeof(IPXSessionInfo));
                            BatchProcess(0);
                        }
                        break;
                    case MSG_BUQP:
                        if (host_state == 1)
                            SendPlayerInfo((__5303ctCLIENTINFOPACKET *)data, &from);
                        break;
                    case MSG_BUCJ:
                        if (host_state == 1)
                            CheckIfPlayerWantingToJoinIsInSession((__35ct5sCLIENTJOINPACKET *)data, &from);
                        break;
                    case MSG_BUSA:
                    case MSG_BUSD:
                        num = 1;
                        ResponsedToSystemMessage(data, &from);
                        for (p = players; p < players + max; p++) {
                            if (p->addr.socket && p->id != me && p->ack)
                                num++;
                        }
                        break;
                    }
                    ipx.Listen(pk, &socket);
                }
            }
            if (!done)
                RemoveAbortedPlayer();
        }
    }
    return ret;
}

// Removes each player who did not acknowledge the last system message,
// telling the others (BUSD) and waiting for their acknowledgements. Returns
// Lb_OK, or Lb_FAIL if no session is running.
// FUNCTION: 0xd9a98
// NONMATCHING: stack slots and register choice; the target places the packet loop's RecPkt
// call ahead of the loop body, ours after it
TbError IPXServiceProvider::RemoveAbortedPlayer(void)
{
    IPXPlayerEntry *aborted;
    u32 count;
    IPXPlayerEntry *p;
    __3ljac5NETSOCKET from;
    u32 max;
    u32 me;
    IPXPacket *pk;
    u32 num;
    u32 last;
    __1p5pi1IPXSYSTEMMESSAGE *data;
    u32 start;
    u32 now;
    __1p5pi1IPXSYSTEMMESSAGE msg;
    u32 done;
    TbError ret = Lb_FAIL;

    if (initialised && in_session && (host_state == 1 || host_state == 2)) {
        if (host_state == 1) {
            me = session.host_id;
            max = session.max_players;
        } else {
            me = remote.local_id;
            max = remote.max_players;
        }
        ret = Lb_OK;
        for (aborted = players; aborted < players + max; aborted++) {
            if (!aborted->addr.socket || aborted->id == me || aborted->ack)
                continue;
            memset(&msg, 0, sizeof(msg));
            msg.magic = MSG_BUSD;
            msg.seq = msg_seq++;
            msg.guid = guid;
            msg.host = me;
            memcpy(&msg.player, aborted, sizeof(IPXPlayerEntry));
            send_dseq[aborted->id] = 0;
            recv_dseq[aborted->id] = 0;
            num = 1;
            for (p = players; p < players + max; p++) {
                if (p->addr.socket && p->id != me) {
                    if (p->ack) {
                        ipx.Send(&socket, &p->addr, (u8 *)&msg, sizeof(msg));
                        num++;
                    }
                    p->abort_acked = 0;
                }
            }
            BatchProcess(0);
            remote.num_players = num;
            session.num_players = num;
            if (num <= 1)
                goto none;
            start = last = clock();
            done = 0;
            do {
                now = clock();
                if (now - start > 1000)
                    break;
                if (now - last > 100) {
                    last = now;
                    for (p = players; p < players + max; p++) {
                        if (p->addr.socket && p->id != me && p->ack && !p->abort_acked)
                            ipx.Send(&socket, &p->addr, (u8 *)&msg, sizeof(msg));
                    }
                    BatchProcess(0);
                }
                for (pk = packets; pk < packets + 32; pk++) {
                    if (!ipx.RecPkt(pk, &socket, &from))
                        continue;
                    data = (__1p5pi1IPXSYSTEMMESSAGE *)pk->data;
                    switch (data->magic) {
                    case MSG_BUSK:
                        if (data->seq == msg.seq) {
                            count = 1;
                            p = &players[data->player.id];
                            if (p->ack) {
                                p->abort_acked = 1;
                                for (p = players; p < players + max; p++) {
                                    if (p->abort_acked)
                                        count++;
                                }
                                if (count >= num)
                                    done = 1;
                            }
                        }
                        break;
                    case MSG_BUHQ:
                        if (host_state == 1 && accepting) {
                            session.magic = MSG_BUHI;
                            ipx.Send(&socket, &from, (u8 *)&session, sizeof(IPXSessionInfo));
                            BatchProcess(0);
                        }
                        break;
                    case MSG_BUQP:
                        if (host_state == 1) {
                            SendPlayerInfo((__5303ctCLIENTINFOPACKET *)data, &from);
                            BatchProcess(0);
                        }
                        break;
                    case MSG_BUCJ:
                        if (host_state == 1) {
                            CheckIfPlayerWantingToJoinIsInSession((__35ct5sCLIENTJOINPACKET *)data, &from);
                            BatchProcess(0);
                        }
                        break;
                    case MSG_BUSA:
                    case MSG_BUSD:
                        num = 1;
                        ResponsedToSystemMessage(data, &from);
                        for (p = players; p < players + max; p++) {
                            if (p->addr.socket && p->id != me && p->ack)
                                num++;
                        }
                        remote.num_players = num;
                        session.num_players = num;
                        break;
                    }
                    ipx.Listen(pk, &socket);
                }
            } while (!done);
            memset(aborted, 0, sizeof(IPXPlayerEntry));
            if (!done) {
                for (p = players; p < players + max; p++)
                    p->ack = p->abort_acked;
                aborted = players;
            }
            continue;
        none:
            for (p = players; p < players + max; p++) {
                if (p->addr.socket && p->id != me && !p->ack)
                    memset(p, 0, sizeof(IPXPlayerEntry));
            }
        }
    }
    return ret;
}

// Acknowledges system message msg to its sender (BUSK) and queues the
// player addition or removal it carries. Returns Lb_FAIL.
// FUNCTION: 0xda35c
// NONMATCHING: stack slots of msg, ret, the inlined BatchProcess argument and the inlined
// AddToReceiveBuffer temporaries
TbError IPXServiceProvider::ResponsedToSystemMessage(__1p5pi1IPXSYSTEMMESSAGE *msg, __3ljac5NETSOCKET *from)
{
    TbError ret = Lb_FAIL;
    __1p5pi1IPXSYSTEMMESSAGE ack;
    u32 me;

    if (initialised && in_session) {
        if (host_state == 1)
            me = session.host_id;
        else
            me = remote.local_id;
        memset(&ack, 0, sizeof(ack));
        ack.magic = MSG_BUSK;
        ack.guid = guid;
        ack.seq = msg->seq;
        memcpy(&ack.player, &players[me], sizeof(IPXPlayerEntry));
        ipx.Send(&socket, from, (u8 *)&ack, sizeof(ack));
        BatchProcess(0);
        switch (msg->magic) {
        case MSG_BUSD:
            send_dseq[msg->host] = 0;
            recv_dseq[msg->host] = 0;
            AddToReceiveBuffer((__6eas9nIPXDATAMESSAGE *)msg);
            break;
        case MSG_BUSA:
            AddToReceiveBuffer((__6eas9nIPXDATAMESSAGE *)msg);
            break;
        }
    }
    return ret;
}

// Polls for the acknowledgement (BUDA) of guaranteed message seq sent to
// player id, queuing data messages meanwhile. Returns 1 if it arrived.
// FUNCTION: 0xdba58
u32 IPXServiceProvider::HaveIGotAckDataMessage(u8 seq, u32 id)
{
    __3ljac5NETSOCKET from;
    u8 *data;
    IPXPacket *p;
    u32 got = 0;

    if (initialised && in_session) {
        for (p = packets; p < packets + 32; p++) {
            if (ipx.RecPkt(p, &socket, &from)) {
                data = p->data;
                switch (*(u32 *)data) {
                case 0x41445542:
                    if (data[0xa] == seq && id == *(u32 *)(data + 4))
                        got = 1;
                    ipx.Listen(p, &socket);
                    break;
                case MSG_BUDG:
                case MSG_BUDN:
                    ProcessDataMessage((__6eas9nIPXDATAMESSAGE *)data);
                    ipx.Listen(p, &socket);
                    break;
                }
                if (got == 1)
                    break;
            }
        }
    }
    return got;
}

// As Update, without advertising: as host answers session and player
// queries and join requests; handles system messages and queues data
// messages (other message types are dropped). Returns 1 if any message
// other than a data message was handled, else 0.
// FUNCTION: 0xdac14
// NONMATCHING: stack slots of the hoisted member addresses, register choice in the
// inlined BatchProcess
u32 IPXServiceProvider::CheckForSystemMessages(void)
{
    u8 join[0x8c];
    u8 info[0x54];
    u8 *data;
    IPXPacket *p;
    __3ljac5NETSOCKET from;
    u32 got = 0;

    if (!initialised || !in_session)
        return got;
    if (host_state == 1) {
        for (p = packets; p < packets + 32; p++) {
            if (!ipx.RecPkt(p, &socket, &from))
                continue;
            data = p->data;
            if (BullfrogIPXMessage((__407tigIPXMESSAGEBLOCK *)data, (__tibcaGUID *)(data + 0xc)) == Lb_OK)
            switch (*(u32 *)data) {
            case MSG_BUHQ:
                if (accepting) {
                    session.magic = MSG_BUHI;
                    ipx.Send(&socket, &from, (u8 *)&session, sizeof(IPXSessionInfo));
                    BatchProcess(0);
                    got = 1;
                }
                ipx.Listen(p, &socket);
                break;
            case MSG_BUQP:
                memcpy(info, data, sizeof(info));
                ipx.Listen(p, &socket);
                SendPlayerInfo((__5303ctCLIENTINFOPACKET *)info, &from);
                BatchProcess(0);
                got = 1;
                break;
            case MSG_BUCJ:
                memcpy(join, data, sizeof(join));
                ipx.Listen(p, &socket);
                AddPlayerToSession((__35ct5sCLIENTJOINPACKET *)join, &from);
                BatchProcess(0);
                got = 1;
                break;
            case MSG_BUSA:
            case MSG_BUSD:
                ResponsedToSystemMessage((__1p5pi1IPXSYSTEMMESSAGE *)data, &from);
                ipx.Listen(p, &socket);
                got = 1;
                break;
            case MSG_BUSK:
                ipx.Listen(p, &socket);
                got = 1;
                break;
            case MSG_BUDG:
            case MSG_BUDN:
                ProcessDataMessage((__6eas9nIPXDATAMESSAGE *)data);
                ipx.Listen(p, &socket);
                break;
            }
        }
    } else if (host_state == 2 && in_session) {
        for (p = packets; p < packets + 32; p++) {
            if (!ipx.RecPkt(p, &socket, &from))
                continue;
            data = p->data;
            if (BullfrogIPXMessage((__407tigIPXMESSAGEBLOCK *)data, (__tibcaGUID *)(data + 0xc)) == Lb_OK)
            switch (*(u32 *)data) {
            case MSG_BUSA:
            case MSG_BUSD:
                ResponsedToSystemMessage((__1p5pi1IPXSYSTEMMESSAGE *)data, &from);
                ipx.Listen(p, &socket);
                got = 1;
                break;
            case MSG_BUSK:
                ipx.Listen(p, &socket);
                got = 1;
                break;
            case MSG_BUDG:
            case MSG_BUDN:
                ProcessDataMessage((__6eas9nIPXDATAMESSAGE *)data);
                ipx.Listen(p, &socket);
                break;
            }
        }
    }
    return got;
}

// Allocates and clears the receive queue (32 entries). Returns Lb_OK, or
// Lb_FAIL.
// FUNCTION: 0xdb668
TbError IPXServiceProvider::AllocateSpStuff(void)
{
    TbError ret = Lb_FAIL;

    queue = (IPXQueueEntry *)LbMemoryAlloc(0x4380);
    if (queue) {
        memset(queue, 0, 0x4380);
        ret = Lb_OK;
    }
    return ret;
}

// Frees the receive queue.
// FUNCTION: 0xdb698
void IPXServiceProvider::FreeAllocSpStuff(void)
{
    if (queue) {
        LbMemoryFree(queue);
        queue = 0;
    }
}

// Empties the receive queue.
// FUNCTION: 0xdb6bc
void IPXServiceProvider::ClearSpStuff(void)
{
    memset(queue, 0, 0x4380);
}

// Sends message buf to player id wrapped as magic (BUDG guaranteed, BUDN
// not); a guaranteed message is resent every 100 ticks until acknowledged,
// for up to 5 seconds. Returns Lb_OK, or Lb_FAIL.
// FUNCTION: 0xdb6d4
TbError IPXServiceProvider::SendMessageToPlayerId(u32 id, void *buf, u32 magic)
{
    u32 now;
    u32 size;
    u32 max;
    u32 start;
    u32 me;
    __3ljac5NETSOCKET *addr;
    TbError ret = Lb_FAIL;

    if (in_session) {
        if (host_state == 1) {
            me = session.host_id;
            max = session.max_players;
        } else {
            me = remote.local_id;
            max = remote.max_players;
        }
        if (id < max) {
            addr = &players[id].addr;
            if (addr->socket) {
                DecodeMessageStub(buf, &size, 0, 0);
                if (size <= 0x212) {
                    out.magic = magic;
                    out.from = me;
                    out.seq = msg_seq++;
                    memcpy(out.data, buf, size + 4);
                    if (magic == MSG_BUDG) {
                        out.dseq = send_dseq[id];
                        send_dseq[id]++;
                    }
                    ipx.Send(&socket, addr, (u8 *)&out, sizeof(out));
                    BatchProcess(0);
                    if (magic == MSG_BUDG) {
                        u32 last;
                        start = last = clock();
                        while (ret == Lb_FAIL) {
                            now = clock();
                            if (now - start > 500)
                                break;
                            if (now - last > 100) {
                                last = now;
                                ipx.Send(&socket, addr, (u8 *)&out, sizeof(out));
                                BatchProcess(0);
                            }
                            if (HaveIGotAckDataMessage(out.seq, id) == 1)
                                ret = Lb_OK;
                            CheckForSystemMessages();
                        }
                        if (ret == Lb_FAIL)
                            NetworkDebugPrintf("DIDN'T GET AN ACK\n");
                    } else {
                        ret = Lb_OK;
                    }
                } else {
                    NetworkDebugPrintf("Invalid length: %d vs %d\n", size, 0x212);
                }
            } else {
                NetworkDebugPrintf("sender->SocketNumber = FALSE\n");
            }
        } else {
            NetworkDebugPrintf("Invalid player id %d\n", id);
        }
    } else {
        NetworkDebugPrintf("m_connectToSession = FALSE\n");
    }
    if (ret == Lb_FAIL)
        NetworkDebugPrintf("Ended up with ret == LB_ERROR\n");
    return ret;
}

// Queues received data message msg; a guaranteed one (BUDG) only if it is
// the next expected from its sender, and it is acknowledged either way.
// Returns 1 if an acknowledgement was sent.
// FUNCTION: 0xdbb58
// NONMATCHING: stack slots, and register choice in the inlined AddToReceiveBuffer copies
u32 IPXServiceProvider::ProcessDataMessage(__6eas9nIPXDATAMESSAGE *msg)
{
    __3ljac5NETSOCKET *addr;
    IPXAckMessage ack;
    u32 me;
    u32 max;
    u32 ret = 0;
    u32 from;

    if (host_state == 1) {
        me = session.host_id;
        max = session.max_players;
    } else {
        me = remote.local_id;
        max = remote.max_players;
    }
    switch (msg->magic) {
    case MSG_BUDN:
        AddToReceiveBuffer(msg);
        break;
    case MSG_BUDG:
        from = msg->from;
        if (from < max) {
            if (msg->dseq == recv_dseq[from]) {
                AddToReceiveBuffer(msg);
                recv_dseq[from]++;
            }
            addr = &players[from].addr;
            if (addr->socket) {
                memset(&ack, 0, sizeof(ack));
                ack.magic = 0x41445542;
                ack.from = me;
                ack.seq = msg->seq;
                ipx.Send(&socket, addr, (u8 *)&ack, sizeof(ack));
                BatchProcess(0);
                ret = 1;
            }
        }
        break;
    }
    return ret;
}

// Puts message msg into the next free receive queue entry, as a delete- or
// add-player message for system messages and for a type 2 data message (a
// player leaving), and keeps the player tables in step. When the player
// list changes it re-adds every player with their local and host flags.
// FUNCTION: 0x000dc440 COMDAT
// NONMATCHING: stack slots of msg, q, id, size, seq and changed
void IPXServiceProvider::AddToReceiveBuffer(__6eas9nIPXDATAMESSAGE *msg)
{
    IPXQueueEntry *q;
    IPXPlayerEntry *p;
    u32 changed = 0;
    u32 seq;
    u32 id;
    u32 size;
    u32 me;
    u32 host;
    u32 max;
    u32 count;
    u32 is_local;
    u32 is_host;
    u8 type;

    q = GetNextEmptyReceiveBuffer();
    switch (msg->magic) {
    case MSG_BUSD: {
        __1p5pi1IPXSYSTEMMESSAGE *sys = (__1p5pi1IPXSYSTEMMESSAGE *)msg;
        if (q) {
            EncodeDeletePlayerMsg(q->data, sys->player.id);
            q->from = sys->host;
            q->used = 1;
        }
        memset(&players[sys->player.id], 0, sizeof(IPXPlayerEntry));
        changed = 1;
        break;
    }
    case MSG_BUSA: {
        __1p5pi1IPXSYSTEMMESSAGE *sys = (__1p5pi1IPXSYSTEMMESSAGE *)msg;
        if (q) {
            EncodeAddPlayerMsg(q->data, sys->player.id, sys->player.name);
            q->from = sys->host;
            q->used = 1;
        }
        memcpy(&players[sys->player.id], &sys->player, sizeof(IPXPlayerEntry));
        changed = 1;
        break;
    }
    case MSG_BUDG:
    case MSG_BUDN:
        DecodeMessageStub(msg->data, &size, (TbMessageType *)&type, &seq);
        switch (type) {
        case 2:
            DecodeRequestExchangeDataMsg(msg->data, id);
            if (q) {
                EncodeDeletePlayerMsg(q->data, id);
                q->from = msg->from;
                q->used = 1;
            }
            memset(&players[id], 0, sizeof(IPXPlayerEntry));
            changed = 1;
            break;
        default:
            if (q) {
                memcpy(q->data, msg->data, size + 4);
                q->from = msg->from;
                q->used = 1;
            }
            break;
        }
        break;
    }
    if (changed == 1) {
        if (host_state == 1) {
            max = session.max_players;
            me = session.host_id;
            host = session.host_id;
        } else {
            // A client's remote.local_id is its own player id (from the join
            // reply's player entry) and remote.host_id the host's.
            max = remote.max_players;
            me = remote.local_id;
            host = remote.host_id;
        }
        count = 0;
        for (p = players; p < players + max; p++) {
            if (p->addr.socket) {
                count++;
                is_local = 0;
                is_host = 0;
                if (me == p->id)
                    is_local = 1;
                if (host == p->id)
                    is_host = 1;
                AddPlayer(p->id, p->name, is_local, is_host);
            }
        }
        if (host_state == 1)
            session.num_players = count;
        else
            remote.num_players = count;
    }
}

// Returns the first free receive queue entry, or 0.
// FUNCTION: 0x000dc400 COMDAT
inline IPXQueueEntry *IPXServiceProvider::GetNextEmptyReceiveBuffer(void)
{
    IPXQueueEntry *entry;
    u32 found = 0;

    for (entry = queue; entry < queue + 32; entry++) {
        if (!entry->used) {
            found = 1;
            break;
        }
    }
    if (found == 1)
        return entry;
    return 0;
}

// Returns Lb_OK if msg is a message of this protocol (with game guid id,
// except for data messages), else Lb_FAIL.
// FUNCTION: 0xdc740
TbError IPXServiceProvider::BullfrogIPXMessage(__407tigIPXMESSAGEBLOCK *msg, __tibcaGUID *id)
{
    TbError ret = Lb_FAIL;

    switch (msg->magic) {
    case 0x4a435542:
    case 0x504e5542:
    case 0x50515542:
    case 0x50485542:
    case 0x4c435542:
    case 0x4c485542:
    case 0x4b535542:
    case 0x49435542:
    case 0x49485542:
    case 0x48515542:
    case 0x41535542:
    case 0x44535542:
        if (memcmp(&guid, id, sizeof(__tibcaGUID)) == 0)
            ret = Lb_OK;
        break;
    case 0x4e445542:
    case 0x47445542:
        ret = Lb_OK;
        break;
    }
    return ret;
}

extern "C" {


// Records the host info packet info from address in the session list
// (updating or adding its entry) and rebuilds the provider's session
// table.
// FUNCTION: 0xdb4c8
void IPXServiceProvider_AddIPXSession(IPXServiceProvider *sp, u8 *info, u8 *address)
{
    s32 now;
    s32 i;

    now = clock();
    for (i = 0; i <= sp->session_count; i++) {
        if (memcmp(&sp->session_list[i].address[10], address + 10, 6) == 0) {
            sp->session_list[i].time = now;
            memcpy(sp->session_list[i].address, address, 16);
            StrCpyTrunc(sp->session_list[i].name, (char *)info + 0x1c, 0x20);
            i = -1;
            break;
        }
    }
    if (i != -1) {
        for (i = 0; i <= sp->session_count; i++) {
            if (sp->session_list[i].time == 0) {
                memcpy(sp->session_list[i].address, address, 16);
                StrCpyTrunc(sp->session_list[i].name, (char *)info + 0x1c, 0x20);
                sp->session_list[i].time = now;
                i = -1;
                break;
            }
        }
        if (i != -1 && sp->session_count < 32)
            sp->session_count++;
    }
    sp->ClearSessions();
    for (i = 0; i <= sp->session_count; i++) {
        if (sp->session_list[i].time != 0)
            sp->AddSession(i, sp->session_list[i].name);
    }
}

}

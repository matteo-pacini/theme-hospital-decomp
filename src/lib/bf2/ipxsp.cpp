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
extern char D_0012608c[];
extern char D_001260d3[];
extern char D_001261fa[];
extern char D_00126145[];
extern char D_001261b3[];
extern char D_001261f3[];
extern char D_001261fe[];
extern char D_00126202[];
extern char D_00126249[];
extern char D_00126171[];
extern char D_00126282[];
char *strcat(char *dst, const char *src);
#pragma intrinsic(strcat)
unsigned long strtoul(const char *s, char **end, int base);
void fn_000db4c8(IPXServiceProvider *sp, u8 *info, u8 *address);
#pragma aux fn_000db4c8 "W?AddIPXSession$:IPXServiceProvider$n(pn$__1snc1aHOSTINFOPACKET$$pn$__3ljac5NETSOCKET$$)v"
}

// FUNCTION: 0xd5350
TbError IPXServiceProvider::ChangeSettings(u32 setting, void *data)
{
    return Lb_OK;
}

// FUNCTION: 0xd5354
TbError IPXServiceProvider::EnableNewPlayers(u32 allow)
{
    TbError ret = Lb_FAIL;

    if (unknown528) {
        ret = Lb_OK;
        accepting = allow;
    }
    return ret;
}

// FUNCTION: 0xd5374
IPXServiceProvider::~IPXServiceProvider()
{
    ServiceProvider::~ServiceProvider();
    session_count = 0;
}

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
        fn_000c20c0(D_0012608c);
    }
    if (ok) {
        fn_000c20c0(D_001260d3, 0x212);
        started = 1;
        this->guid = guid;
        session_count = 0;
        memset(session_list, 0, sizeof(session_list));
        host_state = 0;
        players = 0;
        accepting = 1;
    } else {
        started = ok;
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

// FUNCTION: 0xd5510
TbError IPXServiceProvider::Initialise(__ic76sReceiveCallbacks *cb, void *data)
{
    IPXPacket *p;
    TbError ret = Lb_FAIL;

    fn_000c20c0(D_001260e9);
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
        fn_000c20c0(D_00126118);
    }
    return ret;
}

// FUNCTION: 0xd569c
TbError IPXServiceProvider::Release(void)
{
    fn_000c20c0(D_00126145);
    if (!started) {
        fn_000c20c0(D_00126171);
        return Lb_FAIL;
    }
    if (unknown528)
        Stop();
    memset(session_list, 0, sizeof(session_list));
    host_state = 0;
    ipx.CloseSocket(&socket);
    ipx.DeInit();
    started = 0;
    if (packets)
        ipx.FreeIPXPackets(packets_sel);
    FreeAllocSpStuff();
    return ServiceProvider::Release();
}

extern "C" {
extern char D_00126267[];

TbError fn_000d6550(IPXServiceProvider *sp, u32 to, void *buf, u8 guaranteed);
#pragma aux fn_000d6550 "W?SendMessage$:IPXServiceProvider$n(ulpnv$$TE$$)$TbError$$"
u32 fn_000d6588(IPXServiceProvider *sp, u32 *from, void *buf, u32 *size);
#pragma aux fn_000d6588 "W?ReadMessage$:IPXServiceProvider$n(pnulpnvpnul)ul"
u32 fn_000d6590(IPXServiceProvider *sp, u32 *from, void *buf, u32 *size);
#pragma aux fn_000d6590 "W?PeekMessage$:IPXServiceProvider$n(pnulpnvpnul)ul"

// FUNCTION: 0xd5760
TbError IPXServiceProvider::Start(char *session_name, char *player_name, u32 max_players, void *data)
{
    u32 size;
    TbError ret = Lb_FAIL;
    u32 i;

    if (!started) {
        fn_000c20c0(D_001261b3);
        return Lb_FAIL;
    }
    if (unknown528)
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
    strcpy(server_name, D_001261f3);
    ConvertGuidToString(&guid, server_name + 6);
    fn_000c20c0(D_001261fe, server_name);
    if (ipx.AdvertiseServer(0xbe08, server_name, &socket) == 0) {
        session.local_id = 0;
        session.host_id = 0;
        session.guid = guid;
        session.num_players = 1;
        local_id = 0;
        for (i = 0; i < 32; i++) {
            acked_a[i] = 0;
            acked_b[i] = 0;
        }
        session.max_players = max_players;
        sys_seq = 0;
        StrCpyTrunc(session.name, session_name, 0x20);
        host_state = 1;
        players[0].id = 0;
        strcpy(players[0].name, player_name);
        memcpy(&players[0].addr, &socket, sizeof(__3ljac5NETSOCKET));
        ret = Lb_OK;
        ClearPlayers();
        if (AddPlayer(local_id, player_name, 1, 1) != Lb_OK) {
            fn_000c20c0(D_00126202);
        } else {
            unknown528 = 1;
            accepting = 1;
        }
    } else {
        LbMemoryFree(players);
        players = 0;
        fn_000c20c0(D_00126249);
    }
    return ret;
}

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

    if (started && !unknown528) {
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
                                remote.host_id = *(u32 *)(reply + 0x50);
                                remote.local_id = *(u32 *)(reply + 0x40);
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
                        sys_seq = 0;
                        unknown528 = 1;
                        for (i = 0; i < 32; i++) {
                            acked_a[i] = 0;
                            acked_b[i] = 0;
                        }
                        ret = Lb_OK;
                        local_id = remote.host_id;
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

// FUNCTION: 0xd63e4
TbError IPXServiceProvider::Stop(void)
{
    __1p5pi1IPXSYSTEMMESSAGE msg;
    u32 start;
    u32 me;
    u32 num;
    u32 i;

    if (started && unknown528) {
        start = LbTimerClock();
        while (LbTimerClock() - start < 700)
            ;
        accepting = 0;
        if (host_state == 1 || host_state == 2) {
            if (host_state == 1) {
                me = session.host_id;
                num = session.num_players;
            } else {
                me = remote.host_id;
                num = remote.num_players;
            }
            if (num > 1) {
                memset(&msg, 0, sizeof(msg));
                msg.magic = MSG_BUSD;
                msg.seq = sys_seq++;
                msg.guid = guid;
                msg.host = me;
                memcpy(&msg.player, &players[me], sizeof(IPXPlayerEntry));
                BroadcastSystemMessage(&msg);
            }
            if (players) {
                LbMemoryFree(players);
                players = 0;
                host_state = 0;
                sys_seq = 0;
                for (i = 0; i < 32; i++) {
                    acked_a[i] = 0;
                    acked_b[i] = 0;
                }
            }
        }
        unknown528 = 0;
        ClearSpStuff();
    }
    return Lb_FAIL;
}

// FUNCTION: 0x000d6550
TbError fn_000d6550(IPXServiceProvider *sp, u32 to, void *buf, u8 guaranteed)
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
        fn_000c20c0(D_00126267);
    return ret;
}

// FUNCTION: 0xd6588
u32 fn_000d6588(IPXServiceProvider *sp, u32 *from, void *buf, u32 *size)
{
    return fn_000d6598(sp, from, buf, size, 0);
}

// FUNCTION: 0xd6590
u32 fn_000d6590(IPXServiceProvider *sp, u32 *from, void *buf, u32 *size)
{
    return fn_000d6598(sp, from, buf, size, 1);
}

}

static inline IPXQueueEntry *GetNextFullReceiveBuffer(IPXServiceProvider *self)
{
    IPXQueueEntry *e;
    u32 found = 0;

    for (e = self->queue; e < self->queue + 32; e++) {
        if (e->used) {
            found = 1;
            break;
        }
    }
    if (found == 1)
        return e;
    return 0;
}

// FUNCTION: 0xd6598
u32 fn_000d6598(IPXServiceProvider *self, u32 *from, void *buf, u32 *size, u8 peek)
{
    IPXQueueEntry *e;
    u32 msg_size;
    u32 ret = 0;

    e = GetNextFullReceiveBuffer(self);
    if (e) {
        *from = e->from;
        DecodeMessageStub(e, &msg_size, 0, 0);
        if (msg_size + 4 <= *size) {
            ret = 1;
            memcpy(buf, e, msg_size + 4);
            *size = msg_size + 4;
            if (!peek)
                e->used = 0;
        }
    } else {
        *size = 0;
        self->Update();
    }
    return ret;
}

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
    TbNetworkSessionNameEntry *ne;

    if (started) {
        if (unknown528) {
            if (host_state == 1) {
                strcpy(entry.name, session.name);
                entry.id = session.local_id;
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
                    ne = AddSession(i, session_list[i].name);
                    callback((TbNetworkCallbackData *)ne, data);
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
    u32 host;
    char name[0x20];
};

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
    IPXPlayerEntry *e;
    u32 start;
    u32 last;
    __5303ctCLIENTINFOPACKET query;
    TbError ret = Lb_FAIL;
    u32 total = 32;
    u32 max = 32;
    IPXEnumPlayer player;
    s8 done = 0;
    u32 i;

    if (started) {
        if (unknown528) {
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
                                fn_000db4c8(this, info, (u8 *)&from);
                                break;
                            case MSG_BUHP:
                                if (memcmp(from.node, &session_list[session->id].address[10], 6))
                                    break;
                                if (((IPXSessionInfo *)info)->num_players == total) {
                                    if (((IPXSessionInfo *)info)->page == query.page) {
                                        start = time;
                                        last = now;
                                        player.id = 0;
                                        for (i = 0, e = (IPXPlayerEntry *)(info + 0x50); i < 6; i++, e++) {
                                            if (e->addr.socket) {
                                                count++;
                                                player.host = e->id == ((IPXSessionInfo *)info)->host_id;
                                                strcpy(player.name, e->name);
                                                player.id = e->id;
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

// FUNCTION: 0xd7768
TbError IPXServiceProvider::Update(void)
{
    u8 *data;
    IPXPacket *p;
    __3ljac5NETSOCKET from;
    u8 info[0x54];
    u8 join[0x8c];
    TbError ret = Lb_FAIL;

    if (unknown528) {
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

// FUNCTION: 0xd8138
TbError IPXServiceProvider::BullfrogIPXServer(char *name)
{
    __tibcaGUID id;
    TbError ret = Lb_FAIL;

    if (memcmp(name, D_00126282, 6) == 0) {
        ConvertStringToGuid(name + 6, &id);
        if (memcmp(&guid, &id, sizeof(__tibcaGUID)) == 0)
            ret = Lb_OK;
    }
    return ret;
}

// FUNCTION: 0xd8214
void IPXServiceProvider::ConvertGuidToString(__tibcaGUID *guid, char *str)
{
    char buf[0xc];
    u32 *d = guid->data;
    u32 i;

    str[0] = 0;
    for (i = 0; i < 16; i += 4, d++) {
        sprintf(buf, D_001261fa, *d);
        strcat(str, buf);
    }
}

// FUNCTION: 0xd8274
void IPXServiceProvider::ConvertStringToGuid(char *str, __tibcaGUID *guid)
{
    char buf[0x14];
    u32 *d = guid->data;
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
        *d++ = strtoul(buf, 0, 16);
        i++;
    }
}

// FUNCTION: 0xd82f4
// NONMATCHING: the stack slots of to and end are swapped
TbError IPXServiceProvider::SendPlayerInfo(__5303ctCLIENTINFOPACKET *info, __3ljac5NETSOCKET *to)
{
    u8 buf[0x1a0];
    IPXPlayerEntry *p;
    IPXPlayerEntry *dst;
    IPXPlayerEntry *end;
    u32 skip;
    u32 i;
    TbError ret = Lb_FAIL;

    if (started && unknown528 && host_state == 1) {
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

// FUNCTION: 0xd852c
// NONMATCHING: frame layout and register allocation differ
TbError IPXServiceProvider::AddPlayerToSession(__35ct5sCLIENTJOINPACKET *join, __3ljac5NETSOCKET *from)
{
    __1p5pi1IPXSYSTEMMESSAGE msg;
    IPXPlayerEntry *p;
    u32 i;
    u8 buf[0x88];
    TbError ret = Lb_FAIL;

    if (started && unknown528 && host_state == 1) {
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
                            fn_000c20c0(D_00126289);
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
                msg.seq = sys_seq++;
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

// FUNCTION: 0xd8e34
// NONMATCHING: stack slots, register choice in the inlined BatchProcess, and the
// target shares one Listen-and-break tail between the done and give-up paths
TbError IPXServiceProvider::EnumerateAndAddPlayers(__3ljac5NETSOCKET *server)
{
    u32 total = 32;
    IPXSessionInfo *msg;
    u32 max = 32;
    IPXPlayerEntry *e;
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

    if (started) {
        remote.magic = MSG_BUQP;
        remote.page = 0;
        remote.unknown50 = 0;
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
                            for (i = 0, e = (IPXPlayerEntry *)(msg + 1); i < 6; i++, e++) {
                                if (e->addr.socket) {
                                    added++;
                                    is_local = 0;
                                    is_host = 0;
                                    if (e->id == remote.host_id)
                                        is_local = 1;
                                    if (e->id == msg->host_id)
                                        is_host = 1;
                                    if (AddPlayer(e->id, e->name, is_local, is_host) != Lb_OK)
                                        fn_000c20c0(D_001262d0);
                                    memcpy(&players[e->id], e, sizeof(IPXPlayerEntry));
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

    if (started && unknown528 && (host_state == 1 || host_state == 2)) {
        if (host_state == 1) {
            max = session.max_players;
            me = session.host_id;
            num = session.num_players;
        } else {
            max = remote.max_players;
            me = remote.host_id;
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

// FUNCTION: 0xd9a98
// NONMATCHING: stack slots and register choice; the target places the packet loop's RecPkt
// call ahead of the loop body, ours after it
TbError IPXServiceProvider::RemoveAbortedPlayer(void)
{
    IPXPlayerEntry *ap;
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

    if (started && unknown528 && (host_state == 1 || host_state == 2)) {
        if (host_state == 1) {
            me = session.host_id;
            max = session.max_players;
        } else {
            me = remote.host_id;
            max = remote.max_players;
        }
        ret = Lb_OK;
        for (ap = players; ap < players + max; ap++) {
            if (!ap->addr.socket || ap->id == me || ap->ack)
                continue;
            memset(&msg, 0, sizeof(msg));
            msg.magic = MSG_BUSD;
            msg.seq = sys_seq++;
            msg.guid = guid;
            msg.host = me;
            memcpy(&msg.player, ap, sizeof(IPXPlayerEntry));
            acked_a[ap->id] = 0;
            acked_b[ap->id] = 0;
            num = 1;
            for (p = players; p < players + max; p++) {
                if (p->addr.socket && p->id != me) {
                    if (p->ack) {
                        ipx.Send(&socket, &p->addr, (u8 *)&msg, sizeof(msg));
                        num++;
                    }
                    p->unknown36 = 0;
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
                        if (p->addr.socket && p->id != me && p->ack && !p->unknown36)
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
                                p->unknown36 = 1;
                                for (p = players; p < players + max; p++) {
                                    if (p->unknown36)
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
            memset(ap, 0, sizeof(IPXPlayerEntry));
            if (!done) {
                for (p = players; p < players + max; p++)
                    p->ack = p->unknown36;
                ap = players;
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

// FUNCTION: 0xda35c
// NONMATCHING: stack slots of msg, ret, the inlined BatchProcess argument and the inlined
// AddToReceiveBuffer temporaries
TbError IPXServiceProvider::ResponsedToSystemMessage(__1p5pi1IPXSYSTEMMESSAGE *msg, __3ljac5NETSOCKET *from)
{
    TbError ret = Lb_FAIL;
    __1p5pi1IPXSYSTEMMESSAGE ack;
    u32 me;

    if (started && unknown528) {
        if (host_state == 1)
            me = session.host_id;
        else
            me = remote.host_id;
        memset(&ack, 0, sizeof(ack));
        ack.magic = MSG_BUSK;
        ack.guid = guid;
        ack.seq = msg->seq;
        memcpy(&ack.player, &players[me], sizeof(IPXPlayerEntry));
        ipx.Send(&socket, from, (u8 *)&ack, sizeof(ack));
        BatchProcess(0);
        switch (msg->magic) {
        case MSG_BUSD:
            acked_a[msg->host] = 0;
            acked_b[msg->host] = 0;
            AddToReceiveBuffer((__6eas9nIPXDATAMESSAGE *)msg);
            break;
        case MSG_BUSA:
            AddToReceiveBuffer((__6eas9nIPXDATAMESSAGE *)msg);
            break;
        }
    }
    return ret;
}

// FUNCTION: 0xdba58
u32 IPXServiceProvider::HaveIGotAckDataMessage(u8 seq, u32 id)
{
    __3ljac5NETSOCKET from;
    u8 *data;
    IPXPacket *p;
    u32 got = 0;

    if (started && unknown528) {
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

    if (!started || !unknown528)
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
    } else if (host_state == 2 && unknown528) {
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

// FUNCTION: 0xdb698
void IPXServiceProvider::FreeAllocSpStuff(void)
{
    if (queue) {
        LbMemoryFree(queue);
        queue = 0;
    }
}

// FUNCTION: 0xdb6bc
void IPXServiceProvider::ClearSpStuff(void)
{
    memset(queue, 0, 0x4380);
}

// FUNCTION: 0xdb6d4
// NONMATCHING: frame layout and register allocation differ
TbError IPXServiceProvider::SendMessageToPlayerId(u32 id, void *buf, u32 magic)
{
    u32 now;
    u32 size;
    u32 last;
    u32 max;
    u32 start;
    u32 me;
    __3ljac5NETSOCKET *addr;
    TbError ret = Lb_FAIL;

    if (unknown528) {
        if (host_state == 1) {
            me = session.host_id;
            max = session.max_players;
        } else {
            me = remote.host_id;
            max = remote.max_players;
        }
        if (id < max) {
            addr = &players[id].addr;
            if (addr->socket) {
                DecodeMessageStub(buf, &size, 0, 0);
                if (size <= 0x212) {
                    out.magic = magic;
                    out.from = me;
                    out.seq = sys_seq++;
                    memcpy(out.data, buf, size + 4);
                    if (magic == MSG_BUDG) {
                        out.dseq = acked_a[id];
                        acked_a[id]++;
                    }
                    ipx.Send(&socket, addr, (u8 *)&out, sizeof(out));
                    BatchProcess(0);
                    if (magic == MSG_BUDG) {
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
                            fn_000c20c0(D_00126317);
                    } else {
                        ret = Lb_OK;
                    }
                } else {
                    fn_000c20c0(D_0012632a, size, 0x212);
                }
            } else {
                fn_000c20c0(D_00126344);
            }
        } else {
            fn_000c20c0(D_00126362, id);
        }
    } else {
        fn_000c20c0(D_00126378);
    }
    if (ret == Lb_FAIL)
        fn_000c20c0(D_00126394);
    return ret;
}

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
        me = remote.host_id;
        max = remote.max_players;
    }
    switch (msg->magic) {
    case MSG_BUDN:
        AddToReceiveBuffer(msg);
        break;
    case MSG_BUDG:
        from = msg->from;
        if (from < max) {
            if (msg->dseq == acked_b[from]) {
                AddToReceiveBuffer(msg);
                acked_b[from]++;
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
    u32 host;
    u32 local;
    u32 max;
    u32 count;
    u32 is_host;
    u32 is_local;
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
            host = session.host_id;
            local = session.host_id;
        } else {
            max = remote.max_players;
            host = remote.host_id;
            local = remote.local_id;
        }
        count = 0;
        for (p = players; p < players + max; p++) {
            if (p->addr.socket) {
                count++;
                is_host = 0;
                is_local = 0;
                if (host == p->id)
                    is_host = 1;
                if (local == p->id)
                    is_local = 1;
                AddPlayer(p->id, p->name, is_host, is_local);
            }
        }
        if (host_state == 1)
            session.num_players = count;
        else
            remote.num_players = count;
    }
}

// FUNCTION: 0x000dc400 COMDAT
inline IPXQueueEntry *IPXServiceProvider::GetNextEmptyReceiveBuffer(void)
{
    IPXQueueEntry *e;
    u32 found = 0;

    for (e = queue; e < queue + 32; e++) {
        if (!e->used) {
            found = 1;
            break;
        }
    }
    if (found == 1)
        return e;
    return 0;
}

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


// FUNCTION: 0xdb4c8
void fn_000db4c8(IPXServiceProvider *sp, u8 *info, u8 *address)
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

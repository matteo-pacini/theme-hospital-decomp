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
extern u8 bp_guard;
#pragma aux bp_guard "W?$Wbi00$:.0$:?BatchProcess$n(uc)v:IPXServiceProvider$n[]uc"
extern u32 bp_start_batch_time;
#pragma aux bp_start_batch_time "W?start_batch_time$:.0$:?BatchProcess$n(uc)v:IPXServiceProvider$nul"
extern u32 bp_start_time;
#pragma aux bp_start_time "W?start_time$:.0$:?BatchProcess$n(uc)v:IPXServiceProvider$nul"
extern u32 bp_no_batchs;
#pragma aux bp_no_batchs "W?no_batchs$:.0$:?BatchProcess$n(uc)v:IPXServiceProvider$nul"
}

// FUNCTION: 0xd5340
void IPXDebug(char *fmt, ...)
{
}

void IPXServiceProvider::BatchProcess(u8 delay)
{
    u32 now;

    if (!(bp_guard & 1)) {
        bp_guard |= 1;
        bp_start_batch_time = clock();
    }
    if (!(bp_guard & 2)) {
        bp_guard |= 2;
        bp_start_time = clock();
    }
    now = clock();
    if (!delay || now - bp_start_time > 50) {
        bp_start_time = now;
        bp_no_batchs++;
        ipx.BatchProcess();
    }
    if (now - bp_start_batch_time >= 100) {
        bp_start_batch_time = now;
        bp_no_batchs = 0;
    }
}

TbError IPXServiceProvider::CheckMessage(u8 *data)
{
    TbError ret = Lb_FAIL;

    switch (*(u32 *)data) {
    case MSG_BUCJ:
    case MSG_BUNP:
    case MSG_BUQP:
    case MSG_BUHP:
    case MSG_BUCL:
    case MSG_BUHL:
    case MSG_BUSK:
    case MSG_BUCI:
    case MSG_BUHI:
    case MSG_BUHQ:
    case MSG_BUSA:
    case MSG_BUSD:
        if (memcmp(data + 0xc, &guid, sizeof(guid)) == 0)
            ret = Lb_OK;
        break;
    case MSG_BUDN:
    case MSG_BUDG:
        ret = Lb_OK;
        break;
    }
    return ret;
}

extern "C" {

// FUNCTION: 0x000dc400
IPXQueueEntry *fn_000dc400(IPXServiceProvider *sp)
{
    IPXQueueEntry *e;
    u32 found = 0;

    for (e = sp->queue; e < sp->queue + 32; e++) {
        if (!e->used) {
            found = 1;
            break;
        }
    }
    if (found == 1)
        return e;
    return 0;
}

}

#define PROCESS_SYSTEM_MESSAGE(self, msg) \
    { \
        IPXQueueEntry *q; \
        IPXPlayerEntry *p; \
        u32 changed = 0; \
        u32 size; \
        u32 seq; \
        u32 id; \
        u32 host; \
        u32 local; \
        u32 max; \
        u32 count; \
        u8 type; \
        q = fn_000dc400(self); \
        switch (msg->magic) { \
        case MSG_BUSD: \
            if (q) { \
                EncodeDeletePlayerMsg(q->data, msg->player.id); \
                q->used = 1; \
                q->from = msg->host; \
            } \
            memset(&self->players[msg->player.id], 0, sizeof(IPXPlayerEntry)); \
            changed = 1; \
            break; \
        case MSG_BUSA: \
            if (q) { \
                self->EncodeAddPlayerMsg(q->data, msg->player.id, msg->player.name); \
                q->used = 1; \
                q->from = msg->host; \
            } \
            memcpy(&self->players[msg->player.id], &msg->player, sizeof(IPXPlayerEntry)); \
            changed = 1; \
            break; \
        case MSG_BUDG: \
        case MSG_BUDN: \
            fn_000d1b90((u8 *)msg + 0xc, &size, &type, &seq); \
            if (type == 2) { \
                DecodeRequestExchangeDataMsg((u8 *)msg + 0xc, id); \
                if (q) { \
                    EncodeDeletePlayerMsg(q->data, id); \
                    q->used = 1; \
                    q->from = msg->host; \
                } \
                memset(&self->players[id], 0, sizeof(IPXPlayerEntry)); \
                changed = 1; \
            } else if (q) { \
                memcpy(q->data, (u8 *)msg + 0xc, size + 4); \
                q->used = 1; \
                q->from = msg->host; \
            } \
            break; \
        } \
        if (changed == 1) { \
            if (self->host_state == 1) { \
                host = self->session.host_id; \
                local = self->session.host_id; \
                max = self->session.max_players; \
            } else { \
                host = self->remote.host_id; \
                local = self->remote.local_id; \
                max = self->remote.max_players; \
            } \
            count = 0; \
            for (p = self->players; p < self->players + max; p++) { \
                if (p->addr.socket) { \
                    count++; \
                    self->AddPlayer(p->id, p->name, host == p->id, local == p->id); \
                } \
            } \
            if (self->host_state == 1) \
                self->session.num_players = count; \
            else \
                self->remote.num_players = count; \
        } \
    }

// FUNCTION: 0xd5510
// NONMATCHING: range also holds Release, both Start overloads and Stop
TbError IPXServiceProvider::Initialise(__ic76sReceiveCallbacks *cb, void *data)
{
    TbError ret = Lb_FAIL;
    IPXPacket *p;

    fn_000c20c0(D_001260e9);
    if (ServiceProvider::Initialise(cb, data) == Lb_OK) {
        if (ipx.Init() == 0) {
            socket.socket = 0;
            if (ipx.OpenSocket(&socket, 0) == 0) {
                packets = ipx.AllocIPXPackets(0x20, &packets_sel);
                if (packets) {
                    for (p = packets; p < packets + 0x20; p++)
                        ipx.Listen(p, &socket);
                    ret = Lb_OK;
                    BatchProcess(0);
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

extern "C" {
extern char D_00126267[];

// NONMATCHING: target range also covers the two following thunks (0xd6588, 0xd6590)
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

}

// FUNCTION: 0xd6598
// NONMATCHING: range is the private message-queue read (misnamed Enumerate) followed by both Enumerate overloads
u32 fn_000d6598(IPXServiceProvider *self, u32 *from, void *buf, u32 *size, u8 peek)
{
    IPXQueueEntry *e;
    u32 found = 0;
    u32 msg_size;
    u32 ret = 0;

    for (e = self->queue; e < self->queue + 32; e++) {
        if (e->used) {
            found = 1;
            break;
        }
    }
    if (found != 1)
        e = 0;
    if (e) {
        *from = e->from;
        fn_000d1b90(e, &msg_size, 0, 0);
        if (msg_size + 4 <= *size) {
            ret = 1;
            memcpy(buf, e, msg_size + 4);
            *size = msg_size + 4;
            if (!peek)
                e->used = 0;
        }
    } else {
        *size = 0;
        fn_000d7768(self);
    }
    return ret;
}

// FUNCTION: 0xd7768
// NONMATCHING: range is the packet pump (misnamed BullfrogIPXServer) followed by three more functions
TbError fn_000d7768(IPXServiceProvider *self)
{
    u8 join[0x8c];
    u8 info[0x54];
    __3ljac5NETSOCKET from;
    IPXPacket *p;
    u8 *data;

    if (!self->unknown528)
        return Lb_FAIL;
    if (self->host_state == 1) {
        for (p = self->packets; p < self->packets + 32; p++) {
            if (self->ipx.RecPkt(p, &self->socket, &from)) {
                data = p->data;
                if (self->CheckMessage(data) != Lb_OK)
                    continue;
                switch (*(u32 *)data) {
                case MSG_BUHQ:
                    if (self->accepting) {
                        self->session.magic = MSG_BUHI;
                        self->ipx.Listen(p, &self->socket);
                        self->ipx.Send(&self->socket, &from, (u8 *)&self->session, sizeof(IPXSessionInfo));
                        self->BatchProcess(0);
                    }
                    break;
                case MSG_BUQP:
                    memcpy(info, data, sizeof(info));
                    self->ipx.Listen(p, &self->socket);
                    self->SendPlayerInfo((__5303ctCLIENTINFOPACKET *)info, &from);
                    self->BatchProcess(0);
                    break;
                case MSG_BUCJ:
                    memcpy(join, data, sizeof(join));
                    self->ipx.Listen(p, &self->socket);
                    self->AddPlayerToSession((__35ct5sCLIENTJOINPACKET *)join, &from);
                    self->BatchProcess(0);
                    break;
                case MSG_BUSA:
                case MSG_BUSD:
                    self->ResponsedToSystemMessage((__1p5pi1IPXSYSTEMMESSAGE *)data, &from);
                    self->ipx.Listen(p, &self->socket);
                    break;
                case MSG_BUSK:
                    self->ipx.Listen(p, &self->socket);
                    break;
                default:
                    self->ProcessDataMessage((__6eas9nIPXDATAMESSAGE *)data);
                    self->ipx.Listen(p, &self->socket);
                    break;
                }
            }
        }
        if (self->accepting)
            fn_000d508c(&self->ipx, 0xbe08, self->server_name, &self->socket);
        self->BatchProcess(1);
    } else if (self->host_state == 2) {
        for (p = self->packets; p < self->packets + 32; p++) {
            if (self->ipx.RecPkt(p, &self->socket, &from)) {
                data = p->data;
                if (self->CheckMessage(data) != Lb_OK)
                    continue;
                switch (*(u32 *)data) {
                case MSG_BUSA:
                case MSG_BUSD:
                    self->ResponsedToSystemMessage((__1p5pi1IPXSYSTEMMESSAGE *)data, &from);
                    self->ipx.Listen(p, &self->socket);
                    break;
                case MSG_BUSK:
                    self->ipx.Listen(p, &self->socket);
                    break;
                default:
                    self->ProcessDataMessage((__6eas9nIPXDATAMESSAGE *)data);
                    self->ipx.Listen(p, &self->socket);
                    break;
                }
            }
        }
        self->BatchProcess(1);
    }
    return Lb_OK;
}

// FUNCTION: 0xd82f4
// NONMATCHING: player-table scan loops allocate registers differently
TbError IPXServiceProvider::SendPlayerInfo(__5303ctCLIENTINFOPACKET *info, __3ljac5NETSOCKET *to)
{
    u8 buf[0x1a0];
    IPXPlayerEntry *p;
    IPXPlayerEntry *dst;
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
            for (dst = (IPXPlayerEntry *)(buf + 0x50); dst < (IPXPlayerEntry *)(buf + 0x50) + 6; p++) {
                if (p >= players + session.max_players)
                    break;
                if (p->addr.socket) {
                    memcpy(dst, p, sizeof(IPXPlayerEntry));
                    dst++;
                }
            }
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
    u8 buf[0x88];
    __1p5pi1IPXSYSTEMMESSAGE msg;
    IPXPlayerEntry *p;
    u32 i;
    TbError ret = Lb_FAIL;

    if (started && unknown528 && host_state == 1) {
        ret = Lb_OK;
        if (session.num_players < session.max_players) {
            if (CheckIfPlayerWantingToJoinIsInSession(join, from) == 1)
                return Lb_OK;
            if (accepting == 1) {
                for (i = 0, p = players; p < players + session.max_players; i++, p++) {
                    if (p->addr.socket == 0) {
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
                PROCESS_SYSTEM_MESSAGE(this, (&msg));
                BroadcastSystemMessage(&msg);
            }
        }
    }
    return ret;
}

// FUNCTION: 0xd8c58
// NONMATCHING: frame layout and register allocation differ
u32 IPXServiceProvider::CheckIfPlayerWantingToJoinIsInSession(__35ct5sCLIENTJOINPACKET *join, __3ljac5NETSOCKET *from)
{
    u8 buf[0x88];
    IPXPlayerEntry *p;
    u32 ret = 0;

    for (p = players; p < players + session.max_players; p++) {
        if (memcmp(join->addr.node, p->addr.node, 6) == 0) {
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
// NONMATCHING: loop structure and frame layout differ
TbError IPXServiceProvider::EnumerateAndAddPlayers(__3ljac5NETSOCKET *server)
{
    __3ljac5NETSOCKET from;
    IPXPacket *pk;
    IPXSessionInfo *msg;
    IPXPlayerEntry *e;
    u32 last_send;
    u32 start;
    u32 now;
    u32 total = 32;
    u32 max = 32;
    u32 added = 0;
    u32 i;
    s32 done = 0;
    TbError ret = Lb_FAIL;

    if (!started)
        return ret;
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
            if (CheckMessage(pk->data) == Lb_OK && msg->magic == MSG_BUHP) {
                if (msg->num_players == total) {
                    if (msg->page == remote.page) {
                        last_send = now;
                        start = now;
                        for (i = 0, e = (IPXPlayerEntry *)(msg + 1); i < 6; i++, e++) {
                            if (e->addr.socket) {
                                added++;
                                if (AddPlayer(e->id, e->name, e->id == remote.host_id, e->id == msg->host_id) != Lb_OK)
                                    fn_000c20c0(D_001262d0);
                                memcpy(&players[e->id], e, sizeof(IPXPlayerEntry));
                            }
                        }
                        if (added >= total) {
                            remote.num_players = msg->num_players;
                            done = 1;
                            remote.max_players = msg->max_players;
                            ret = Lb_OK;
                        } else if (++remote.page >= max * 6) {
                            done = -1;
                        } else {
                            ipx.Send(&socket, server, (u8 *)&remote, sizeof(remote));
                            BatchProcess(0);
                            now = start = last_send = clock();
                        }
                        if (done) {
                            ipx.Listen(pk, &socket);
                            break;
                        }
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
    return ret;
}

void IPXServiceProvider::SendToAll(u32 me, u32 max, __1p5pi1IPXSYSTEMMESSAGE *msg)
{
    IPXPlayerEntry *p;

    for (p = players; p < players + max; p++) {
        if (p->addr.socket && me != p->id)
            ipx.Send(&socket, &p->addr, (u8 *)msg, sizeof(*msg));
    }
    BatchProcess(0);
}

// FUNCTION: 0xd9480
// NONMATCHING: frame layout and register allocation differ
TbError IPXServiceProvider::BroadcastSystemMessage(__1p5pi1IPXSYSTEMMESSAGE *msg)
{
    __3ljac5NETSOCKET from;
    IPXPacket *pk;
    IPXPlayerEntry *p;
    __1p5pi1IPXSYSTEMMESSAGE *data;
    u32 max;
    u32 me;
    u32 num;
    u32 count;
    u32 last;
    u32 start;
    u32 now;
    u32 done = 0;
    TbError ret = Lb_FAIL;

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
            SendToAll(me, max, msg);
            last = clock();
            start = clock();
            while (!done) {
                now = clock();
                if (now - start > 1000)
                    break;
                if (now - last > 50) {
                    last = now;
                    SendToAll(me, max, msg);
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
                            if (p->addr.socket != 0 && me != p->id && p->ack != 0)
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

// FUNCTION: 0xda35c
// NONMATCHING: frame layout and register allocation differ
TbError IPXServiceProvider::ResponsedToSystemMessage(__1p5pi1IPXSYSTEMMESSAGE *msg, __3ljac5NETSOCKET *from)
{
    __1p5pi1IPXSYSTEMMESSAGE ack;
    u32 me;
    TbError ret = Lb_FAIL;

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
            PROCESS_SYSTEM_MESSAGE(this, msg);
            break;
        case MSG_BUSA:
            PROCESS_SYSTEM_MESSAGE(this, msg);
            break;
        }
    }
    return ret;
}

// FUNCTION: 0xdba58
// NONMATCHING: loop register allocation differs
u32 IPXServiceProvider::HaveIGotAckDataMessage(u8 seq, u32 id)
{
    __3ljac5NETSOCKET from;
    IPXPacket *p;
    u32 got = 0;

    if (started && unknown528) {
        for (p = packets; p < packets + 32; p++) {
            if (ipx.RecPkt(p, &socket, &from)) {
                switch (*(u32 *)p->data) {
                case 0x41445542:
                    if (p->data[0xa] == seq && id == *(u32 *)(p->data + 4))
                        got = 1;
                    ipx.Listen(p, &socket);
                    break;
                case MSG_BUDG:
                case MSG_BUDN:
                    ProcessDataMessage((__6eas9nIPXDATAMESSAGE *)p->data);
                    ipx.Listen(p, &socket);
                    break;
                }
            }
            if (got == 1)
                break;
        }
    }
    return got;
}

// FUNCTION: 0xd9a98
// NONMATCHING: loop structure and frame layout differ
TbError IPXServiceProvider::RemoveAbortedPlayer(void)
{
    __1p5pi1IPXSYSTEMMESSAGE msg;
    __3ljac5NETSOCKET from;
    IPXPlayerEntry *ap;
    IPXPlayerEntry *p;
    IPXPacket *pk;
    __1p5pi1IPXSYSTEMMESSAGE *data;
    u32 me;
    u32 max;
    u32 num;
    u32 count;
    u32 done;
    u32 start;
    u32 last;
    u32 now;
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
            if (ap->addr.socket == 0 || me == ap->id || ap->ack != 0)
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
                if (p->addr.socket && me != p->id) {
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
            if (num <= 1) {
                for (p = players; p < players + max; p++) {
                    if (p->addr.socket != 0 && me != p->id && p->ack == 0)
                        memset(p, 0, sizeof(IPXPlayerEntry));
                }
                continue;
            }
            start = last = clock();
            done = 0;
            do {
                now = clock();
                if (now - start > 1000)
                    break;
                if (now - last > 100) {
                    last = now;
                    for (p = players; p < players + max; p++) {
                        if (p->addr.socket && me != p->id && p->ack && !p->unknown36)
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
                            p = &players[data->player.id];
                            if (p->ack) {
                                p->unknown36 = 1;
                                count = 1;
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
                            if (p->addr.socket != 0 && me != p->id && p->ack != 0)
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
        }
    }
    return ret;
}

// FUNCTION: 0xdac14
// NONMATCHING: frame layout and register allocation differ
u32 IPXServiceProvider::CheckForSystemMessages(void)
{
    u8 join[0x8c];
    u8 info[0x54];
    __3ljac5NETSOCKET from;
    IPXPacket *p;
    u8 *data;
    u32 got = 0;

    if (!started || !unknown528)
        return got;
    if (host_state == 1) {
        for (p = packets; p < packets + 32; p++) {
            if (!ipx.RecPkt(p, &socket, &from))
                continue;
            data = p->data;
            if (CheckMessage(data) != Lb_OK)
                continue;
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
            if (CheckMessage(data) != Lb_OK)
                continue;
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

// FUNCTION: 0xdb6d4
// NONMATCHING: frame layout and register allocation differ
TbError IPXServiceProvider::SendMessageToPlayerId(u32 id, void *buf, u32 magic)
{
    __3ljac5NETSOCKET *addr;
    u32 size;
    u32 me;
    u32 max;
    u32 start;
    u32 last;
    u32 now;
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
                fn_000d1b90(buf, &size, 0, 0);
                if (size <= 0x212) {
                    out.magic = magic;
                    out.from = me;
                    out.seq = sys_seq++;
                    memcpy(out.data, buf, size + 4);
                    if (magic == MSG_BUDG)
                        out.dseq = acked_a[id]++;
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
// NONMATCHING: range also holds the following function; frame layout differs
u32 IPXServiceProvider::ProcessDataMessage(__6eas9nIPXDATAMESSAGE *msg)
{
    __6eas9nIPXDATAMESSAGE ack;
    __3ljac5NETSOCKET *addr;
    u32 me;
    u32 max;
    u32 ret = 0;

    if (host_state == 1) {
        me = session.host_id;
        max = session.max_players;
    } else {
        me = remote.host_id;
        max = remote.max_players;
    }
    switch (msg->magic) {
    case MSG_BUDN:
        PROCESS_SYSTEM_MESSAGE(this, ((__1p5pi1IPXSYSTEMMESSAGE *)msg));
        break;
    case MSG_BUDG:
        if (msg->from < max) {
            if (msg->dseq == acked_b[msg->from]) {
                PROCESS_SYSTEM_MESSAGE(this, ((__1p5pi1IPXSYSTEMMESSAGE *)msg));
                acked_b[msg->from]++;
            }
            addr = &players[msg->from].addr;
            if (addr->socket) {
                memset(&ack, 0, 0xc);
                ack.magic = 0x41445542;
                ack.from = me;
                ack.seq = msg->seq;
                ipx.Send(&socket, addr, (u8 *)&ack, 0xc);
                BatchProcess(0);
                ret = 1;
            }
        }
        break;
    }
    return ret;
}

extern "C" {

// NONMATCHING: frame pointer and register allocation differ
// FUNCTION: 0x000dc440
void fn_000dc440(IPXServiceProvider *sp, __1p5pi1IPXSYSTEMMESSAGE *msg)
{
    PROCESS_SYSTEM_MESSAGE(sp, msg);
}

}

extern "C" {

// NONMATCHING: config/functions.txt extends this function over two jmp thunks
// (0xdc812, 0xdc817) that follow it
// FUNCTION: 0xdc740
TbError fn_000dc740(IPXServiceProvider *sp, __tibcaGUID *type, __tibcaGUID *guid)
{
    TbError ret = Lb_FAIL;

    switch (type->data[0]) {
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
        if (memcmp(&sp->guid, guid, sizeof(__tibcaGUID)) == 0)
            ret = Lb_OK;
        break;
    case 0x4e445542:
    case 0x47445542:
        ret = Lb_OK;
        break;
    }
    return ret;
}

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

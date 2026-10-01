#include "net.h"

void __stdcall NilAddMsgCallback(u32 id, char *name, void *data);
void __stdcall NilDeleteMsgCallback(u32 id, void *data);
void __stdcall NilHostMsgCallback(u32 id, void *data);
void __stdcall NilUserSysMsgCallback(void *data);
void *__stdcall NilUserDataMsgCallback(u32 id, u32 size, u32 seq, void *data);
void __stdcall NilRequestExchangeDataMsgCallback(u32 id, u32 seq, void *data);
void __stdcall NilRequestCompositeExchangeDataMsgCallback(u32 id, u32 seq, void *data);
void *__stdcall NilUnidirectionalMsgCallback(u32 id, u32 size, void *data);
void __stdcall NilSystemUserMsgCallback(u32 id, void *msg, u32 size, void *data);

static __ic76sReceiveCallbacks nilReceiveAspect = {
    NilAddMsgCallback,
    NilDeleteMsgCallback,
    NilHostMsgCallback,
    NilUserSysMsgCallback,
    NilUserDataMsgCallback,
    NilRequestExchangeDataMsgCallback,
    NilRequestCompositeExchangeDataMsgCallback,
    NilUnidirectionalMsgCallback,
    NilSystemUserMsgCallback,
    0,
};

/* Copies at most size - 1 characters and always terminates. */
inline void NameCopy(char *dst, char *src, u32 size)
{
    if (strlen(src) < size) {
        strcpy(dst, src);
    } else {
        memcpy(dst, src, size - 1);
        dst[size - 1] = 0;
    }
}

extern "C" {
}

// FUNCTION: 0x000d1310
ServiceProvider::ServiceProvider()
{
    u32 i;

    local_id = 0;
    for (i = 0; i < 32; i++)
        sessions[i].in_use = 0;
    init_count = 0;
    unknown528 = 0;
    player_count = 0;
    strcpy(name, "");
    started = 0;
    callback_data = 0;
    callbacks = &nilReceiveAspect;
}

// FUNCTION: 0x000d13bc
ServiceProvider::~ServiceProvider()
{
    fn_000c20c0("  LbNETWORK : Destructing ServiceProvider\n");
}

// FUNCTION: 0x000d13dc
TbError ServiceProvider::Initialise(__ic76sReceiveCallbacks *cbs, void *data)
{
    u32 i;

    fn_000c20c0("  LbNETWORK : Initialising ServiceProvider\n");
    if (init_count)
        fn_000c20c0("Service Provider already setup!\n");
    player_count = 0;
    for (i = 0; i < 32; i++)
        sessions[i].in_use = 0;
    callback_data = data;
    if (cbs)
        callbacks = cbs;
    unknown528 = 0;
    started = 0;
    init_count++;
    return Lb_OK;
}

// FUNCTION: 0x000d145c
TbError ServiceProvider::Release(void)
{
    fn_000c20c0("  LbNETWORK : Releasing ServiceProvider\n");
    unknown528 = 0;
    if (init_count != 1)
        fn_000c20c0("Service Provider not setup!\n");
    init_count--;
    return Lb_OK;
}

// FUNCTION: 0x000d14a4
TbError ServiceProvider::AddPlayer(u32 id, char *name, u32 is_local, u32 is_host)
{
    u32 i;
    u32 found;
    TbError ret;
    char *dst;

    ret = Lb_FAIL;
    found = 0;
    for (i = 0; i < player_count; i++) {
        if (id == players[i].id)
            found = 1;
    }
    if (found) {
        ret = Lb_OK;
    } else if (player_count >= 32) {
        fn_000c20c0("  WARNING network: ServiceProvider::AddPlayer() player table is full\n");
    } else {
        i = player_count;
        players[i].id = id;
        dst = players[i].name;
        if (dst && name)
            NameCopy(dst, name, 32);
        players[i].unknown5 = is_local;
        players[i].host = is_host;
        ret = Lb_OK;
        player_count++;
    }
    return ret;
}

// FUNCTION: 0x000d15d8
TbError ServiceProvider::DeletePlayer(u32 id)
{
    u32 i;
    u32 found;
    u32 index;

    found = 0;
    for (i = 0; i < player_count; i++) {
        if (id == players[i].id) {
            found = 1;
            index = i;
        }
    }
    if (found) {
        if (index != player_count - 1) {
            for (i = index; i < player_count - 1; i++)
                players[i] = players[i + 1];
        }
        player_count--;
    }
    return Lb_OK;
}

// FUNCTION: 0x000d1668
void ServiceProvider::ClearPlayers(void)
{
    player_count = 0;
}

// FUNCTION: 0x000d1674
TbNetworkSessionNameEntry *ServiceProvider::AddSession(u32 id, char *name)
{
    TbNetworkSessionNameEntry *session;
    u32 found;
    u32 i;
    u32 more;
    char *dst;

    found = 0;
    session = 0;
    for (i = 0; i < 32; i++) {
        if (sessions[i].in_use && sessions[i].id == id) {
            found = 1;
            session = &sessions[i];
        }
    }
    if (!found) {
        i = 0;
        more = 1;
        do {
            if (!sessions[i].in_use) {
                sessions[i].id = id;
                dst = sessions[i].name;
                if (dst && name)
                    NameCopy(dst, name, 32);
                session = &sessions[i];
                sessions[i].in_use = 1;
                more = 0;
            } else {
                i++;
                if (i >= 32)
                    more = 0;
            }
        } while (more);
    }
    return session;
}

// FUNCTION: 0xd17a8
void ServiceProvider::ClearSessions(void)
{
    u32 i;

    for (i = 0; i < 32; i++)
        sessions[i].in_use = 0;
}

// FUNCTION: 0xd17c4
TbError ServiceProvider::EnumeratePlayers(void (__stdcall *callback)(TbNetworkCallbackData *, void *), void *data)
{
    TbError ret = Lb_OK;
    u32 i;

    for (i = 0; i < player_count; i++)
        callback((TbNetworkCallbackData *)&players[i], data);
    return ret;
}

// FUNCTION: 0xd180c
u32 ServiceProvider::GetAddPlayerMsgSize(char *name)
{
    return strlen(name) + 9;
}

// FUNCTION: 0xd1820
void ServiceProvider::EncodeAddPlayerMsg(u8 *buf, u32 id, char *name)
{
    EncodeMessageStub(buf, strlen(name) + 5, (TbMessageType)1, 0);
    memcpy(buf + 4, &id, 4);
    strcpy((char *)buf + 8, name);
}

// FUNCTION: 0xd189c
u32 ServiceProvider::DecodeAddPlayerMsg(u8 *buf, u32 &id, char *name)
{
    u32 ret = 1;

    memcpy(&id, buf + 4, 4);
    if (name)
        strcpy(name, (char *)buf + 8);
    return ret;
}

// FUNCTION: 0xd18f8
u32 GetRequestCompositeExchangeDataMsgSize(void)
{
    return 8;
}

// FUNCTION: 0xd1900
void EncodeDeletePlayerMsg(u8 *buf, u32 id)
{
    EncodeMessageStub(buf, 4, (TbMessageType)2, 0);
    memcpy(buf + 4, &id, 4);
}

// FUNCTION: 0xd1948
u32 DecodeRequestExchangeDataMsg(u8 *buf, u32 &id)
{
    memcpy(&id, buf + 4, 4);
    return 1;
}

// FUNCTION: 0xd1970
void EncodeRequestExchangeDataMsg(u8 *buf, u32 id, u32 seq)
{
    EncodeMessageStub(buf, 4, (TbMessageType)5, seq);
    memcpy(buf + 4, &id, 4);
}

// FUNCTION: 0xd19c0
void EncodeRequestCompositeExchangeDataMsg(u8 *buf, u32 id, u32 seq)
{
    EncodeMessageStub(buf, 4, (TbMessageType)6, seq);
    memcpy(buf + 4, &id, 4);
}

// FUNCTION: 0x000d1a10
u32 DecodeRequestCompositeExchangeDataMsg(u8 *buf, u32 &id)
{
    memcpy(&id, buf + 4, 4);
    return 1;
}

// FUNCTION: 0xd1a38
TbError ServiceProvider::SystemAddPlayerHandler(u8 *msg)
{
    TbError ret = Lb_FAIL;
    u32 id;
    char name[32];

    DecodeAddPlayerMsg(msg, id, name);
    if (AddPlayer(id, name, 0, 0) == Lb_OK)
        ret = Lb_OK;
    return ret;
}

// FUNCTION: 0xd1ab4
TbError ServiceProvider::SystemDeletePlayerHandler(u8 *msg)
{
    TbError ret = Lb_FAIL;
    u32 id;

    CheckForDeletedHost(msg);
    DecodeRequestCompositeExchangeDataMsg(msg, id);
    if (DeletePlayer(id) == Lb_OK)
        ret = Lb_OK;
    return ret;
}

// FUNCTION: 0xd1b04
void StrCpyTrunc(char *dst, char *src, u32 size)
{
    if (dst == 0 || src == 0)
        return;
    if (strlen(src) < size) {
        strcpy(dst, src);
    } else {
        memcpy(dst, src, size - 1);
        dst[size - 1] = 0;
    }
}

// FUNCTION: 0xd1b64
void EncodeMessageStub(void *buf, u32 size, TbMessageType type, u32 seq)
{
    if (buf == 0) {
        fn_000c20c0("  WARNING network: ServiceProvider::EncodeMessageStub() NULL ptr\n");
        return;
    }
    *(u32 *)buf = (size & 0xfffff) | ((seq & 0xf) << 20) | (type << 24);
}

// FUNCTION: 0xd1b90
void DecodeMessageStub(void *buf, u32 *size, TbMessageType *type, u32 *seq)
{
    u32 *p = (u32 *)buf;

    if (p == 0) {
        fn_000c20c0("  WARNING network: ServiceProvider::DecodeMessageStub() NULL ptr\n");
        return;
    }
    if (size)
        *size = *p & 0xfffff;
    if (seq)
        *seq = (*p >> 20) & 0xf;
    if (type)
        *type = (TbMessageType)((*p >> 24) & 0xff);
}

// FUNCTION: 0xd1bd4
void __stdcall NilAddMsgCallback(u32 id, char *name, void *data)
{
    fn_000c20c0("  WARNING network: NilAddMsgCallback(%d, %s) hit\n", id, name);
}

// FUNCTION: 0xd1bf0
void __stdcall NilDeleteMsgCallback(u32 id, void *data)
{
    fn_000c20c0("  WARNING network: NilDeleteMsgCallback(%d) hit\n", id);
}

// FUNCTION: 0xd1c08
void __stdcall NilHostMsgCallback(u32 id, void *data)
{
    fn_000c20c0("  WARNING network: NilHostMsgCallback(%d) hit\n", id);
}

// FUNCTION: 0xd1c20
void __stdcall NilUserSysMsgCallback(void *data)
{
    fn_000c20c0("  WARNING network: NilUserSysMsgCallback() hit\n");
}

// FUNCTION: 0xd1c30
void *__stdcall NilUserDataMsgCallback(u32 id, u32 size, u32 seq, void *data)
{
    fn_000c20c0("  WARNING network: NilUserDataMsgCallback(%d, %d %d) hit\n", id, size, seq);
    return 0;
}

// FUNCTION: 0xd1c54
void __stdcall NilRequestExchangeDataMsgCallback(u32 id, u32 seq, void *data)
{
    fn_000c20c0("  WARNING network: NilRequestExchangeDataMsgCallback(%d %d) hit\n", id, seq);
}

// FUNCTION: 0xd1c70
void __stdcall NilRequestCompositeExchangeDataMsgCallback(u32 id, u32 seq, void *data)
{
    fn_000c20c0("  WARNING network: NilRequestCompositeExchangeDataMsgCallback(%d %d) hit\n", id, seq);
}

// FUNCTION: 0xd1c8c
void *__stdcall NilUnidirectionalMsgCallback(u32 id, u32 size, void *data)
{
    fn_000c20c0("  WARNING network: NilUnidirectionalMsgCallback(%d, %d) hit\n", id, size);
    return 0;
}

// FUNCTION: 0xd1cac
void __stdcall NilSystemUserMsgCallback(u32 id, void *msg, u32 size, void *data)
{
    fn_000c20c0("  WARNING network: NilSystemUserMsgCallback(%d %d) hit\n", id, size);
}

// FUNCTION: 0xd1cc8
// NONMATCHING: found flag kept in a register instead of its frame slot; range also holds the Receive jump table
TbError ServiceProvider::CheckForDeletedHost(u8 *msg)
{
    u32 id;
    u32 host_index;
    u32 found;
    u32 new_index;
    u32 i;
    u32 have;
    u32 lowest;

    DecodeRequestCompositeExchangeDataMsg(msg, id);
    for (i = found = 0; i < player_count; i++) {
        if (id == players[i].id && players[i].host) {
            found = 1;
            host_index = i;
        }
    }
    if (found) {
        have = 0;
        for (i = 0; i < player_count; i++) {
            if (!players[i].host) {
                if (!have) {
                    have = 1;
                    new_index = i;
                    lowest = players[i].id;
                } else if (lowest > players[i].id) {
                    lowest = players[i].id;
                    new_index = i;
                }
            }
        }
        if (have) {
            players[new_index].host = 1;
            players[host_index].host = 0;
            if (callbacks->hostMsg)
                callbacks->hostMsg(lowest, callback_data);
        }
    }
    return Lb_OK;
}

// FUNCTION: 0xd1e24
// NONMATCHING: the inlined DecodeMessageStub swaps ecx/edx (buffer and sequence pointer); its jump table sits in the preceding range
TbError ServiceProvider::Receive(u32 flags)
{
    char added_name[0x20];
    u32 keep_going;
    u32 from;
    u32 size;
    u32 msg_size;
    u32 seq;
    u32 added_id;
    u32 comp_req_id;
    u32 deleted_id;
    u8 type;
    u32 req_id;
    char buf[0x404];
    TbError ret;
    u32 sys_msg_size;
    void *dst;

    keep_going = 1;
    ret = Lb_OK;
    if (!started) {
        fn_000c20c0("  WARNING network: ServiceProvider::Receive() not initialised\n");
        return Lb_FAIL;
    }
    while (keep_going) {
        size = 0x404;
        if (PeekMessage(&from, buf, &size)) {
            dst = 0;
            DecodeMessageStub(buf, &msg_size, (TbMessageType *)&type, &seq);
            if (flags & 8)
                flags = 0xffffffff;
            switch (type) {
            case 8:
                if (flags & 0x100) {
                    if (callbacks->unknown24) {
                        dst = fn_000ed344(msg_size + 4);
                        size = msg_size + 4;
                        ReadMessage(&from, dst, &size);
                        callbacks->unknown24(from, dst);
                    }
                }
                break;
            case 0:
                if (flags & 1) {
                    if (callbacks->multiPlayer)
                        dst = callbacks->multiPlayer(from, msg_size + 4, seq, callback_data);
                    else
                        fn_000c20c0("  WARNING network: ServiceProvider::Receive() NIL target for userDataMsgCallbackProc\n");
                    if (!dst) {
                        dst = buf;
                        size = 0x404;
                    }
                    if (!ReadMessage(&from, dst, &size)) {
                        fn_000c20c0("  WARNING network: ServiceProvider::ReadMessage() Inconsistancy between reads\n");
                        ret = Lb_FAIL;
                    }
                }
                keep_going = 0;
                break;
            case 1:
                if (flags & 2) {
                    size = 0x404;
                    if (ReadMessage(&from, buf, &size)) {
                        SystemAddPlayerHandler((u8 *)buf);
                        DecodeAddPlayerMsg((u8 *)buf, added_id, added_name);
                        if (callbacks->addMsg)
                            callbacks->addMsg(added_id, added_name, callback_data);
                    } else {
                        fn_000c20c0("  WARNING network: ServiceProvider::ReadMessage() consistency error on Receive()\n");
                    }
                } else {
                    keep_going = 0;
                }
                break;
            case 2:
                if (flags & 4) {
                    size = 0x404;
                    if (ReadMessage(&from, buf, &size)) {
                        SystemDeletePlayerHandler((u8 *)buf);
                        DecodeRequestCompositeExchangeDataMsg((u8 *)buf, deleted_id);
                        if (callbacks->deleteMsg)
                            callbacks->deleteMsg(deleted_id, callback_data);
                    } else {
                        fn_000c20c0("  WARNING network: ServiceProvider::ReadMessage() consistency error\n");
                    }
                } else {
                    keep_going = 0;
                }
                break;
            case 3:
                break;
            case 4:
                if (flags & 0x80) {
                    size = 0x404;
                    if (ReadMessage(&from, buf, &size)) {
                        DecodeMessageStub(buf, &sys_msg_size, 0, 0);
                        if (callbacks->systemUserMsg)
                            callbacks->systemUserMsg(from, buf + 4, sys_msg_size, callback_data);
                    } else {
                        fn_000c20c0("  WARNING network: ServiceProvider::ReadMessage() consistency error\n");
                    }
                } else {
                    keep_going = 0;
                }
                break;
            case 5:
                if (flags & 0x10) {
                    size = 0x404;
                    if (ReadMessage(&from, buf, &size)) {
                        DecodeRequestExchangeDataMsg((u8 *)buf, req_id);
                        if (callbacks->mpReqExDataMsg)
                            callbacks->mpReqExDataMsg(req_id, seq, callback_data);
                    } else {
                        fn_000c20c0("  WARNING network: ServiceProvider::ReadMessage() consistency error\n");
                    }
                } else {
                    keep_going = 0;
                }
                break;
            case 6:
                if (flags & 0x20) {
                    size = 0x404;
                    if (ReadMessage(&from, buf, &size)) {
                        DecodeRequestCompositeExchangeDataMsg((u8 *)buf, comp_req_id);
                        if (callbacks->mpReqCompsExDataMsg)
                            callbacks->mpReqCompsExDataMsg(comp_req_id, seq, callback_data);
                    } else {
                        fn_000c20c0("  WARNING network: ServiceProvider::ReadMessage() consistency error\n");
                    }
                } else {
                    keep_going = 0;
                }
                break;
            case 7:
                if (flags & 0x40) {
                    if (callbacks->unidirectionalMsg)
                        dst = callbacks->unidirectionalMsg(from, msg_size + 4, callback_data);
                    else
                        fn_000c20c0("  WARNING network: ServiceProvider::Receive() NIL target for unidirectionalMsgCallbackProc\n");
                    if (!dst) {
                        dst = buf;
                        size = 0x404;
                    }
                    if (!ReadMessage(&from, dst, &size)) {
                        fn_000c20c0("  WARNING network: ServiceProvider::ReadMessage() Inconsistancy between reads\n");
                        ret = Lb_FAIL;
                    }
                }
                keep_going = 0;
                break;
            default:
                fn_000c20c0("  WARNING network: ServiceProvider::Receive() messageType is out of range\n");
                break;
            }
        } else {
            keep_going = 0;
        }
    }
    return ret;
}

// FUNCTION: 0xd25ac
TbError ServiceProvider::Send(u32 to, void *buf)
{
    TbError ret;
    u32 msg_size;
    TbMessageType type;
    u32 player_id;
    u32 seq;
    char added_name[0x20];
    void *dst;
    s8 guaranteed;

    ret = Lb_FAIL;
    if (!started) {
        fn_000c20c0("  WARNING network: ServiceProvider::Send() not initialised\n");
        return Lb_FAIL;
    }
    DecodeMessageStub(buf, &msg_size, &type, &seq);
    if (to == local_id) {
        dst = 0;
        switch (type) {
        case 8:
            if (callbacks->unknown24)
                callbacks->unknown24(player_id, buf);
            break;
        case 0:
            if (callbacks->multiPlayer)
                dst = callbacks->multiPlayer(local_id, msg_size + 4, seq, callback_data);
            else
                fn_000c20c0("  WARNING network: ServiceProvider::Send() NIL target for userDataMsgCallbackProc\n");
            if (dst)
                memcpy(dst, buf, msg_size + 4);
            break;
        case 1:
            SystemAddPlayerHandler((u8 *)buf);
            DecodeAddPlayerMsg((u8 *)buf, player_id, added_name);
            if (callbacks->addMsg)
                callbacks->addMsg(player_id, added_name, callback_data);
            break;
        case 2:
            SystemDeletePlayerHandler((u8 *)buf);
            DecodeRequestCompositeExchangeDataMsg((u8 *)buf, player_id);
            if (callbacks->deleteMsg)
                callbacks->deleteMsg(player_id, callback_data);
            break;
        case 3:
            break;
        case 4:
            if (callbacks->systemUserMsg)
                callbacks->systemUserMsg(local_id, (u8 *)buf + 4, msg_size, callback_data);
            else
                fn_000c20c0("  WARNING network: ServiceProvider::Send() NIL target for systemUserMsgCallbackProc\n");
            break;
        case 5:
            DecodeRequestExchangeDataMsg((u8 *)buf, player_id);
            if (callbacks->mpReqExDataMsg)
                callbacks->mpReqExDataMsg(player_id, seq, callback_data);
            break;
        case 6:
            DecodeRequestCompositeExchangeDataMsg((u8 *)buf, player_id);
            if (callbacks->mpReqCompsExDataMsg)
                callbacks->mpReqCompsExDataMsg(player_id, seq, callback_data);
            break;
        default:
            fn_000c20c0("  WARNING network: ServiceProvider::Send() messageType is out of range\n");
            break;
        }
        ret = Lb_OK;
    } else {
        guaranteed = (type == MSG_TYPE_0 || type == MSG_TYPE_5 || type == MSG_TYPE_6 || type == MSG_TYPE_8) ? (s8)0 : (s8)1;
        if (SendMessage(to, buf, (TbMessageType)guaranteed) == Lb_OK)
            ret = Lb_OK;
        else
            fn_000c20c0("  WARNING network: ServiceProvider::Send() failure on SendMessage()\n");
    }
    return ret;
}

// FUNCTION: 0xd29d8
TbError ServiceProvider::LookForSystemMessages(void)
{
    return Receive(0xb6);
}

// FUNCTION: 0xd29e8
TbError ServiceProvider::BroadcastSystemMessage(u8 *msg)
{
    u32 i;
    u32 id;
    TbError ret;
    u8 type;
    u32 invalid;

    ret = Lb_OK;
    invalid = 0;
    DecodeMessageStub(msg, 0, (TbMessageType *)&type, 0);
    switch (type) {
    case 1:
        DecodeAddPlayerMsg(msg, id, 0);
        break;
    case 2:
        DecodeRequestCompositeExchangeDataMsg(msg, id);
        break;
    default:
        invalid = 1;
        break;
    }
    if (invalid) {
        fn_000c20c0("  WARNING network: ServiceProvider::BroadcastSystemMessage() invalid MessageType: %02X\n", *msg);
        ret = Lb_FAIL;
    } else {
        for (i = 0; i < player_count; i++) {
            if (players[i].id != local_id && players[i].id != id) {
                if (SendMessage(players[i].id, msg, (TbMessageType)1) != Lb_OK) {
                    ret = Lb_FAIL;
                    fn_000c20c0("  WARNING network: ServiceProvider::BroadcastSystemMessage() failure on SendMessage\n");
                }
            }
        }
    }
    return ret;
}

// FUNCTION: 0xd2af8
// NONMATCHING: register allocation: the target keeps this in esi and copies it to ebp for the
// inlined EnumeratePlayers
TbError ServiceProvider::EnumeratePlayersForSessionRunning(void (__stdcall *callback)(TbNetworkCallbackData *, void *), void *data)
{
    TbError ret = Lb_FAIL;
    TbError result;

    if (Receive(0xb6) == Lb_OK) {
        result = EnumeratePlayers(callback, data);
        if (result == Lb_OK)
            ret = result;
        else
            fn_000c20c0("  WARNING network: ServiceProvider::EnumeratePlayersForSessionRunning() failure on EnumeratePlayers()\n");
    } else {
        fn_000c20c0("  WARNING network: ServiceProvider::EnumeratePlayersForSessionRunning() failure on LookForSystemMessages()\n");
    }
    return ret;
}

// FUNCTION: 0xd2b78
TbError ServiceProvider::EnableNewPlayers(u32 allow)
{
    return Lb_OK;
}

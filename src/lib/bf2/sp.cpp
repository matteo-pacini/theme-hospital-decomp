#include "net.h"

extern "C" {
extern char D_00124e04[];
extern char D_00124e05[];
extern char D_00124e30[];
extern char D_00124e5c[];
extern char D_00124e7d[];
extern char D_00124ea6[];
extern char D_00124ec3[];
extern __ic76sReceiveCallbacks *D_001507dc;
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
    strcpy(name, D_00124e04);
    started = 0;
    callback_data = 0;
    callbacks = (__ic76sReceiveCallbacks *)&D_001507dc;
}

// FUNCTION: 0x000d13bc
ServiceProvider::~ServiceProvider()
{
    fn_000c20c0(D_00124e05);
}

// FUNCTION: 0x000d13dc
TbError ServiceProvider::Initialise(__ic76sReceiveCallbacks *cbs, void *data)
{
    u32 i;

    fn_000c20c0(D_00124e30);
    if (init_count)
        fn_000c20c0(D_00124e5c);
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
    fn_000c20c0(D_00124e7d);
    unknown528 = 0;
    if (init_count != 1)
        fn_000c20c0(D_00124ea6);
    init_count--;
    return Lb_OK;
}

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

// FUNCTION: 0x000d14a4
TbError ServiceProvider::AddPlayer(u32 id, char *name, u32 a, u32 b)
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
        fn_000c20c0(D_00124ec3);
    } else {
        i = player_count;
        players[i].id = id;
        dst = players[i].name;
        if (dst && name)
            NameCopy(dst, name, 32);
        players[i].unknown5 = a;
        players[i].host = b;
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

// FUNCTION: 0xd1b64
void EncodeMessageStub(void *buf, u32 size, TbMessageType type, u32 seq)
{
    if (buf == 0) {
        fn_000c20c0(D_00124f09);
        return;
    }
    *(u32 *)buf = (size & 0xfffff) | ((seq & 0xf) << 20) | (type << 24);
}

// FUNCTION: 0xd1820
void ServiceProvider::EncodeAddPlayerMsg(u8 *buf, u32 id, char *name)
{
    EncodeMessageStub(buf, strlen(name) + 5, (TbMessageType)1, 0);
    memcpy(buf + 4, &id, 4);
    strcpy((char *)buf + 8, name);
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

// FUNCTION: 0xd189c
u32 ServiceProvider::DecodeAddPlayerMsg(u8 *buf, u32 &id, char *name)
{
    u32 ret = 1;

    memcpy(&id, buf + 4, 4);
    if (name)
        strcpy(name, (char *)buf + 8);
    return ret;
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

// FUNCTION: 0xd29d8
TbError ServiceProvider::LookForSystemMessages(void)
{
    return Receive(0xb6);
}

// FUNCTION: 0xd1b90
void DecodeMessageStub(void *buf, u32 *size, TbMessageType *type, u32 *seq)
{
    u32 *p = (u32 *)buf;

    if (p == 0) {
        fn_000c20c0(D_00124f4b);
        return;
    }
    if (size)
        *size = *p & 0xfffff;
    if (seq)
        *seq = (*p >> 20) & 0xf;
    if (type)
        *type = (TbMessageType)((*p >> 24) & 0xff);
}

extern "C" {
extern char D_00124f8d[];
extern char D_00124fbf[];
extern char D_00124ff0[];
extern char D_0012501f[];
extern char D_0012504f[];
extern char D_00125089[];
extern char D_001250ca[];
extern char D_00125114[];
extern char D_00125151[];
}

// FUNCTION: 0xd1bd4
void __stdcall NilAddMsgCallback(u32 id, char *name, void *data)
{
    fn_000c20c0(D_00124f8d, id, name);
}

// FUNCTION: 0xd1bf0
void __stdcall NilDeleteMsgCallback(u32 id, void *data)
{
    fn_000c20c0(D_00124fbf, id);
}

// FUNCTION: 0xd1c08
void __stdcall NilHostMsgCallback(u32 id, void *data)
{
    fn_000c20c0(D_00124ff0, id);
}

// FUNCTION: 0xd1c20
void __stdcall NilUserSysMsgCallback(void *data)
{
    fn_000c20c0(D_0012501f);
}

// FUNCTION: 0xd1c30
void *__stdcall NilUserDataMsgCallback(u32 id, u32 size, u32 seq, void *data)
{
    fn_000c20c0(D_0012504f, id, size, seq);
    return 0;
}

// FUNCTION: 0xd1c54
void __stdcall NilRequestExchangeDataMsgCallback(u32 id, u32 seq, void *data)
{
    fn_000c20c0(D_00125089, id, seq);
}

// FUNCTION: 0xd1c70
void __stdcall NilRequestCompositeExchangeDataMsgCallback(u32 id, u32 seq, void *data)
{
    fn_000c20c0(D_001250ca, id, seq);
}

// FUNCTION: 0xd1c8c
void *__stdcall NilUnidirectionalMsgCallback(u32 id, u32 size, void *data)
{
    fn_000c20c0(D_00125114, id, size);
    return 0;
}

// FUNCTION: 0xd1cac
void __stdcall NilSystemUserMsgCallback(u32 id, void *msg, u32 size, void *data)
{
    fn_000c20c0(D_00125151, id, size);
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
    char name2[0x20];
    u32 keep_going;
    u32 from;
    u32 size;
    u32 msg_size;
    u32 seq;
    u32 id;
    u32 id5;
    u32 id3;
    u8 type;
    u32 id4;
    char buf[0x404];
    TbError ret;
    u32 msg_size2;
    void *dst;

    keep_going = 1;
    ret = Lb_OK;
    if (!started) {
        fn_000c20c0(D_00125189);
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
                        fn_000c20c0(D_001251c8);
                    if (!dst) {
                        dst = buf;
                        size = 0x404;
                    }
                    if (!ReadMessage(&from, dst, &size)) {
                        fn_000c20c0(D_0012521e);
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
                        DecodeAddPlayerMsg((u8 *)buf, id, name2);
                        if (callbacks->addMsg)
                            callbacks->addMsg(id, name2, callback_data);
                    } else {
                        fn_000c20c0(D_0012526d);
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
                        DecodeRequestCompositeExchangeDataMsg((u8 *)buf, id3);
                        if (callbacks->deleteMsg)
                            callbacks->deleteMsg(id3, callback_data);
                    } else {
                        fn_000c20c0(D_001252bf);
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
                        DecodeMessageStub(buf, &msg_size2, 0, 0);
                        if (callbacks->systemUserMsg)
                            callbacks->systemUserMsg(from, buf + 4, msg_size2, callback_data);
                    } else {
                        fn_000c20c0(D_00125304);
                    }
                } else {
                    keep_going = 0;
                }
                break;
            case 5:
                if (flags & 0x10) {
                    size = 0x404;
                    if (ReadMessage(&from, buf, &size)) {
                        DecodeRequestExchangeDataMsg((u8 *)buf, id4);
                        if (callbacks->mpReqExDataMsg)
                            callbacks->mpReqExDataMsg(id4, seq, callback_data);
                    } else {
                        fn_000c20c0(D_00125349);
                    }
                } else {
                    keep_going = 0;
                }
                break;
            case 6:
                if (flags & 0x20) {
                    size = 0x404;
                    if (ReadMessage(&from, buf, &size)) {
                        DecodeRequestCompositeExchangeDataMsg((u8 *)buf, id5);
                        if (callbacks->mpReqCompsExDataMsg)
                            callbacks->mpReqCompsExDataMsg(id5, seq, callback_data);
                    } else {
                        fn_000c20c0(D_0012538e);
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
                        fn_000c20c0(D_001253d3);
                    if (!dst) {
                        dst = buf;
                        size = 0x404;
                    }
                    if (!ReadMessage(&from, dst, &size)) {
                        fn_000c20c0(D_0012542f);
                        ret = Lb_FAIL;
                    }
                }
                keep_going = 0;
                break;
            default:
                fn_000c20c0(D_0012547e);
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
    u32 id2;
    u32 seq;
    char name2[0x20];
    void *dst;
    s8 guaranteed;

    ret = Lb_FAIL;
    if (!started) {
        fn_000c20c0(D_001254c9);
        return Lb_FAIL;
    }
    DecodeMessageStub(buf, &msg_size, &type, &seq);
    if (to == local_id) {
        dst = 0;
        switch (type) {
        case 8:
            if (callbacks->unknown24)
                callbacks->unknown24(id2, buf);
            break;
        case 0:
            if (callbacks->multiPlayer)
                dst = callbacks->multiPlayer(local_id, msg_size + 4, seq, callback_data);
            else
                fn_000c20c0(D_00125505);
            if (dst)
                memcpy(dst, buf, msg_size + 4);
            break;
        case 1:
            SystemAddPlayerHandler((u8 *)buf);
            DecodeAddPlayerMsg((u8 *)buf, id2, name2);
            if (callbacks->addMsg)
                callbacks->addMsg(id2, name2, callback_data);
            break;
        case 2:
            SystemDeletePlayerHandler((u8 *)buf);
            DecodeRequestCompositeExchangeDataMsg((u8 *)buf, id2);
            if (callbacks->deleteMsg)
                callbacks->deleteMsg(id2, callback_data);
            break;
        case 3:
            break;
        case 4:
            if (callbacks->systemUserMsg)
                callbacks->systemUserMsg(local_id, (u8 *)buf + 4, msg_size, callback_data);
            else
                fn_000c20c0(D_00125558);
            break;
        case 5:
            DecodeRequestExchangeDataMsg((u8 *)buf, id2);
            if (callbacks->mpReqExDataMsg)
                callbacks->mpReqExDataMsg(id2, seq, callback_data);
            break;
        case 6:
            DecodeRequestCompositeExchangeDataMsg((u8 *)buf, id2);
            if (callbacks->mpReqCompsExDataMsg)
                callbacks->mpReqCompsExDataMsg(id2, seq, callback_data);
            break;
        default:
            fn_000c20c0(D_001255ad);
            break;
        }
        ret = Lb_OK;
    } else {
        guaranteed = (type == MSG_TYPE_0 || type == MSG_TYPE_5 || type == MSG_TYPE_6 || type == MSG_TYPE_8) ? (s8)0 : (s8)1;
        if (SendMessage(to, buf, (TbMessageType)guaranteed) == Lb_OK)
            ret = Lb_OK;
        else
            fn_000c20c0(D_001255f5);
    }
    return ret;
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
        fn_000c20c0(D_0012563a, *msg);
        ret = Lb_FAIL;
    } else {
        for (i = 0; i < player_count; i++) {
            if (players[i].id != local_id && players[i].id != id) {
                if (SendMessage(players[i].id, msg, (TbMessageType)1) != Lb_OK) {
                    ret = Lb_FAIL;
                    fn_000c20c0(D_00125692);
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
    TbError r;

    if (Receive(0xb6) == Lb_OK) {
        r = EnumeratePlayers(callback, data);
        if (r == Lb_OK)
            ret = r;
        else
            fn_000c20c0(D_001256e7);
    } else {
        fn_000c20c0(D_0012574e);
    }
    return ret;
}

// FUNCTION: 0xd2b78
TbError ServiceProvider::EnableNewPlayers(u32 allow)
{
    return Lb_OK;
}

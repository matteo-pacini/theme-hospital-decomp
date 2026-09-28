#include "net.h"

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
// NONMATCHING: range also holds the following DecodeAddPlayerMsg
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

extern "C" {

// NONMATCHING: target range also covers the next two functions (0xd1a38, 0xd1ab4)
// FUNCTION: 0x000d1a10
u32 fn_000d1a10(u8 *buf, u32 &id)
{
    memcpy(&id, buf + 4, 4);
    return 1;
}

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

void DecodeMessageStub(void *buf, u32 *size, u8 *type, u32 *seq)
{
    if (buf == 0) {
        fn_000c20c0(D_00124f4b);
        return;
    }
    if (size)
        *size = *(u32 *)buf & 0xfffff;
    if (seq)
        *seq = (*(u32 *)buf >> 20) & 0xf;
    if (type)
        *type = (*(u32 *)buf >> 24) & 0xff;
}

// FUNCTION: 0xd1b90
// NONMATCHING: the range also holds three __stdcall Nil*Callback stubs
void fn_000d1b90(void *buf, u32 *size, u8 *type, u32 *seq)
{
    if (buf == 0) {
        fn_000c20c0(D_00124f4b);
        return;
    }
    if (size)
        *size = *(u32 *)buf & 0xfffff;
    if (seq)
        *seq = (*(u32 *)buf >> 20) & 0xf;
    if (type)
        *type = (*(u32 *)buf >> 24) & 0xff;
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

    memcpy(&id, msg + 4, 4);
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
// NONMATCHING: frame layout and register allocation differ; its jump table sits in the preceding range
TbError ServiceProvider::Receive(u32 flags)
{
    char buf[0x404];
    char name2[0x20];
    char name[0x20];
    u32 from;
    u32 size;
    u32 msg_size;
    u32 seq;
    u32 unused;
    u32 id;
    u32 id2;
    u32 id3;
    u32 id4;
    u32 id5;
    u32 msg_size2;
    void *self;
    char *msg_name;
    char *msg_data;
    u32 keep_going;
    TbError ret;
    u8 type;
    void *dst;

    keep_going = 1;
    ret = Lb_OK;
    if (!started) {
        fn_000c20c0(D_00125189);
        return Lb_FAIL;
    }
    msg_name = buf + 8;
    msg_data = buf + 4;
    while (keep_going) {
        size = 0x404;
        if (PeekMessage(&from, buf, &size)) {
            dst = 0;
            DecodeMessageStub(buf, &msg_size, &type, &seq);
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
                        memcpy(&id, msg_data, 4);
                        if (name)
                            strcpy(name, msg_name);
                        AddPlayer(id, name, 0, 0);
                        memcpy(&id, msg_data, 4);
                        if (name2)
                            strcpy(name2, msg_name);
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
                        CheckForDeletedHost((u8 *)buf);
                        memcpy(&id2, buf + 4, 4);
                        DeletePlayer(id2);
                        memcpy(&id3, msg_data, 4);
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
                        memcpy(&id4, msg_data, 4);
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
                        memcpy(&id5, msg_data, 4);
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
// NONMATCHING: message type compared as a promoted int instead of a byte
TbError ServiceProvider::Send(u32 to, void *buf)
{
    char name[0x20];
    char name2[0x20];
    u32 msg_size;
    u32 seq;
    u32 id2;
    u32 id;
    u32 id3;
    void *self;
    void *msg;
    u8 type;
    TbError ret;
    void *dst;
    s8 guaranteed;

    ret = Lb_FAIL;
    if (!started) {
        fn_000c20c0(D_001254c9);
        return Lb_FAIL;
    }
    DecodeMessageStub(buf, &msg_size, &type, &seq);
    if (to != local_id) {
        guaranteed = (type == 0 || type == 5 || type == 6 || type == 8) ? (s8)0 : (s8)1;
        if (SendMessage(to, buf, guaranteed) == Lb_OK)
            ret = Lb_OK;
        else
            fn_000c20c0(D_001255f5);
    } else {
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
            memcpy(&id, (u8 *)buf + 4, 4);
            if (name)
                strcpy(name, (char *)buf + 8);
            AddPlayer(id, name, 0, 0);
            memcpy(&id2, (u8 *)buf + 4, 4);
            if (name2)
                strcpy(name2, (char *)buf + 8);
            if (callbacks->addMsg)
                callbacks->addMsg(id2, name2, callback_data);
            break;
        case 2:
            CheckForDeletedHost((u8 *)buf);
            memcpy(&id3, (u8 *)buf + 4, 4);
            DeletePlayer(id3);
            memcpy(&id2, (u8 *)buf + 4, 4);
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
            memcpy(&id2, (u8 *)buf + 4, 4);
            if (callbacks->mpReqExDataMsg)
                callbacks->mpReqExDataMsg(id2, seq, callback_data);
            break;
        case 6:
            memcpy(&id2, (u8 *)buf + 4, 4);
            if (callbacks->mpReqCompsExDataMsg)
                callbacks->mpReqCompsExDataMsg(id2, seq, callback_data);
            break;
        default:
            fn_000c20c0(D_001255ad);
            break;
        }
        ret = Lb_OK;
    }
    return ret;
}

// FUNCTION: 0xd29e8
// NONMATCHING: register allocation differs
TbError ServiceProvider::BroadcastSystemMessage(u8 *msg)
{
    u32 id;
    u32 i;
    u8 *m;
    TbError ret;
    u8 type;
    u32 invalid;

    m = msg;
    ret = Lb_OK;
    invalid = 0;
    DecodeMessageStub(m, 0, &type, 0);
    switch (type) {
    case 1:
    case 2:
        memcpy(&id, m + 4, 4);
        break;
    default:
        invalid = 1;
        break;
    }
    if (invalid) {
        fn_000c20c0(D_0012563a, *m);
        ret = Lb_FAIL;
    } else {
        for (i = 0; i < player_count; i++) {
            if (players[i].id != local_id && players[i].id != id) {
                if (SendMessage(players[i].id, m, 1) != Lb_OK) {
                    ret = Lb_FAIL;
                    fn_000c20c0(D_00125692);
                }
            }
        }
    }
    return ret;
}

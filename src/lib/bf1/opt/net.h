#ifndef OPT_NET_H
#define OPT_NET_H

#include "bflib.h"

extern "C" {
void *memcpy(void *dst, const void *src, unsigned size);
unsigned strlen(const char *s);
char *strcpy(char *dst, const char *src);
#pragma intrinsic(memcpy, strlen, strcpy)
void fn_000c20c0(char *fmt, ...);
}

struct TbNetworkCallbackData;
struct __ic76sReceiveCallbacks;

struct TbNetworkPlayerEntry {
    u8 unknown0;
    u32 id;
    u32 local;
    u32 host;
    char name[0x20];
};

// A provider message's type (its stub header, EncodeMessageStub).
typedef enum {
    // Exchange data for the multiPlayer callback.
    MSG_TYPE_MULTIPLAYER,
    // Also the first byte of a system user message: the player map
    // (SystemUserMsgCallback).
    MSG_TYPE_PLAYER_MAP = 0,
    MSG_TYPE_UNIDIRECTIONAL = 7
} TbMessageType;

// ServiceProvider::Receive flags: which message types to take from the
// queue (RECV_ALL: every type).
#define RECV_MULTIPLAYER 0x1
#define RECV_ADD_PLAYER 0x2
#define RECV_DELETE_PLAYER 0x4
#define RECV_ALL 0x8
#define RECV_REQ_EXCHANGE_DATA 0x10
#define RECV_REQ_COMPOSITE_EXCHANGE_DATA 0x20
#define RECV_UNIDIRECTIONAL 0x40
#define RECV_SYSTEM_USER 0x80
#define RECV_MSG_TYPE_8 0x100

struct ServiceProvider {
    u32 local_id;
    TbNetworkSessionNameEntry sessions[32];
    u32 init_count;
    u32 in_session;
    u32 player_count;
    TbNetworkPlayerEntry players[32];
    char name[0x20];
    __ic76sReceiveCallbacks *callbacks;
    u32 started;
    void *callback_data;

    ServiceProvider();
    virtual ~ServiceProvider();
    virtual TbError Start(TbNetworkSessionNameEntry *session, char *name, void *data) = 0;
    virtual TbError Start(char *session, char *name, u32 max_players, void *data) = 0;
    virtual TbError Stop(void) = 0;
    virtual TbError Enumerate(TbNetworkCallbackFunc callback, void *data) = 0;
    virtual TbError Enumerate(TbNetworkSessionNameEntry *session, TbNetworkCallbackFunc callback, void *data) = 0;
    virtual TbError Init(__tibcaGUID guid, __tibcaGUID *pguid, __ic76sReceiveCallbacks *callbacks, void *data) = 0;
    virtual TbError Release(void);
    virtual TbError ChangeSettings(u32 setting, void *data) = 0;
    virtual TbError EnableNewPlayers(u32 allow);
    virtual u32 ReadMessage(u32 *from, void *buf, u32 *size) = 0;
    virtual u32 PeekMessage(u32 *from, void *buf, u32 *size) = 0;
    virtual TbError SendMessage(u32 to, void *buf, TbMessageType type) = 0;

    TbError Receive(u32 flags);
    TbError Send(u32 to, void *buf);
};

void EncodeMessageStub(void *buf, u32 size, TbMessageType type, u32 seq);

struct __4kqpm7ClientDataEntry {
    u32 id;
    u32 active;
    u32 got_data;
    char name[0x20];
};

extern ServiceProvider *spPtr;
extern __4kqpm7ClientDataEntry clientDataTable[];
extern u32 maximumPlayers;

#endif

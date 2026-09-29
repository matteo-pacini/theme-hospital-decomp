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
    u32 unknown5;
    u32 host;
    char name[0x20];
};

struct ServiceProvider {
    u32 local_id;
    TbNetworkSessionNameEntry sessions[32];
    u32 init_count;
    u32 unknown528;
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
    virtual TbError SendMessage(u32 to, void *buf, u32 guaranteed) = 0;

    TbError Receive(u32 flags);
    TbError Send(u32 to, void *buf);
};

typedef enum {
    MSG_TYPE_0,
    MSG_TYPE_UNIDIRECTIONAL = 7
} TbMessageType;

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

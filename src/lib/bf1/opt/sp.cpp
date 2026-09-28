#include "bflib.h"

struct __ic76sReceiveCallbacks;

struct TbNetworkPlayerEntry {
    u8 flags;
    u32 id;
    u32 unknown5;
    u32 unknown9;
    char name[0x20];
};

/*
 * The polymorphic class as seen from its own module, with the vtable pointer
 * spelled out rather than compiler-emitted.
 */
struct ServiceProvider {
    u32 unknown0;
    TbNetworkSessionNameEntry sessions[32];
    u32 initialised;
    u32 unknown528;
    u32 num_players;
    TbNetworkPlayerEntry players[32];
    char local_name[0x20];
    __ic76sReceiveCallbacks *callbacks;
    u32 unknownaf4;
    void *callback_data;
    void **vftbl;

    ServiceProvider();
    ~ServiceProvider();
    TbError Initialise(__ic76sReceiveCallbacks *callbacks, void *data);
    TbError Release(void);
    TbError AddPlayer(u32 id, char *name, u32 a, u32 b);
    TbError DeletePlayer(u32 id);
    void ClearPlayers(void);
    TbNetworkSessionNameEntry *AddSession(u32 id, char *name);
};

extern "C" {
void fn_000c20c0(char *fmt, ...);
extern void *ServiceProvider_vftbl[];
#pragma aux ServiceProvider_vftbl "W?$Wvf0lo2sm:ServiceProvider$$nx[]pn()v"
extern char D_00124e04[];
extern char D_00124e05[];
extern char D_00124e30[];
extern char D_00124e5c[];
extern char D_00124e7d[];
extern char D_00124ea6[];
extern char D_00124ec3[];
extern __ic76sReceiveCallbacks *D_001507dc;
char *strcpy(char *dst, const char *src);
#pragma intrinsic(strcpy)
u32 strlen(const char *s);
#pragma intrinsic(strlen)
void *memcpy(void *dst, const void *src, u32 n);
#pragma intrinsic(memcpy)
}

// NONMATCHING: target builds the sessions with __wcpp_2_ctor_array__
// FUNCTION: 0x000d1310
ServiceProvider::ServiceProvider()
{
    u32 i;

    vftbl = ServiceProvider_vftbl;
    unknown0 = 0;
    for (i = 0; i < 32; i++)
        sessions[i].in_use = 0;
    initialised = 0;
    unknown528 = 0;
    num_players = 0;
    strcpy(local_name, D_00124e04);
    unknownaf4 = 0;
    callback_data = 0;
    callbacks = (__ic76sReceiveCallbacks *)&D_001507dc;
}

// FUNCTION: 0x000d13bc
ServiceProvider::~ServiceProvider()
{
    vftbl = ServiceProvider_vftbl;
    fn_000c20c0(D_00124e05);
}

// FUNCTION: 0x000d13dc
TbError ServiceProvider::Initialise(__ic76sReceiveCallbacks *cbs, void *data)
{
    u32 i;

    fn_000c20c0(D_00124e30);
    if (initialised)
        fn_000c20c0(D_00124e5c);
    num_players = 0;
    for (i = 0; i < 32; i++)
        sessions[i].in_use = 0;
    callback_data = data;
    if (cbs)
        callbacks = cbs;
    unknown528 = 0;
    unknownaf4 = 0;
    initialised++;
    return Lb_OK;
}

// FUNCTION: 0x000d145c
TbError ServiceProvider::Release(void)
{
    fn_000c20c0(D_00124e7d);
    unknown528 = 0;
    if (initialised != 1)
        fn_000c20c0(D_00124ea6);
    initialised--;
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
    for (i = 0; i < num_players; i++) {
        if (id == players[i].id)
            found = 1;
    }
    if (found) {
        ret = Lb_OK;
    } else if (num_players >= 32) {
        fn_000c20c0(D_00124ec3);
    } else {
        i = num_players;
        players[i].id = id;
        dst = players[i].name;
        if (dst && name)
            NameCopy(dst, name, 32);
        players[i].unknown5 = a;
        players[i].unknown9 = b;
        ret = Lb_OK;
        num_players++;
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
    for (i = 0; i < num_players; i++) {
        if (id == players[i].id) {
            found = 1;
            index = i;
        }
    }
    if (found) {
        if (index != num_players - 1) {
            for (i = index; i < num_players - 1; i++)
                players[i] = players[i + 1];
        }
        num_players--;
    }
    return Lb_OK;
}

// FUNCTION: 0x000d1668
void ServiceProvider::ClearPlayers(void)
{
    num_players = 0;
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

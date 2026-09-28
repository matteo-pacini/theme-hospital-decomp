#include "hosp.h"

struct GameSpeedPrefs {
    u8 unknown0;
    u8 f1;
    u16 speed;
};

extern "C" {
extern char D_001563c5[];
extern GameSpeedPrefs D_001547e0;
extern u8 D_001547e4;
extern u8 D_001547e6;
extern u8 D_001547e7[];
extern u8 D_00154805[];
extern u8 D_00154819;
extern char D_00121be8[];
extern char D_00121bef[];
extern char D_00121c21[];
extern char D_00121c28[];
extern char D_00121c5a[];

void fn_000baad0(void);
void fn_0008bdd4(void);
void FadeMusic(u32 time, u8 volume);
}

void debug_msg(char *fmt, ...);
void set_default_player_details(void);

// FUNCTION: 0x000ba8b0
u8 save_player_details(void)
{
    char name[144];
    s32 handle;
    s32 written;

    sprintf(name, D_00121be8, D_001563c5, statvar.f6c);
    handle = LbFileOpen(name, 0);
    if (handle == -1) {
        debug_msg(D_00121bef, name);
        return -1;
    }
    written = LbFileWrite(handle, &myplayer, 0x5c);
    if (written != 0x5c) {
        LbFileClose(handle);
        return 0;
    }
    LbFileClose(handle);
    return 1;
}

// FUNCTION: 0x000ba954
u8 load_player_details(void)
{
    char name[256];
    s32 handle;

    sprintf(name, D_00121c21, D_001563c5, statvar.f6c);
    if (!LbFileExists(name)) {
        set_default_player_details();
        fn_000baad0();
        return 0;
    }
    handle = LbFileOpen(name, 2);
    if (handle == -1) {
        debug_msg(D_00121c28, name);
        return 0;
    }
    LbFileRead(handle, &myplayer, 0x5c);
    if (LbFileClose(handle) == -1) {
        debug_msg(D_00121c5a, name);
        return 0;
    }
    fn_000baad0();
    return 1;
}

// FUNCTION: 0x000baa28
void set_default_player_details(void)
{
    D_001547e0.speed = 0x19;
    D_001547e4 = 2;
    myplayer.f9 = 0;
    myplayer.fb = 0;
    D_001547e0.f1 = 1;
    D_001547e6 = 1;
    LbMemorySet(D_001547e7, 0x1e, 0);
    LbMemorySet(D_00154805, 0x14, 0);
    LbMemorySet(myplayer.unknown12, 0xb, 0);
    D_00154819 = 1;
    myplayer.ff = 1;
    myplayer.fd = 0x7f;
    myplayer.unknown11 = 1;
    myplayer.fc = 0x7f;
    myplayer.f10 = 1;
    myplayer.fe = 0x7f;
}

// FUNCTION: 0x000baad0
void fn_000baad0(void)
{
    FadeMusic(1000, myplayer.fc);
    fn_0008bdd4();
    switch (D_001547e0.speed) {
    case 1:
        vary_speed(10);
        break;
    case 5:
        vary_speed(20);
        break;
    case 0x16:
        vary_speed(30);
        break;
    case 0x19:
        vary_speed(40);
        break;
    case 0x7d00:
        vary_speed(50);
        break;
    default:
        break;
    }
}

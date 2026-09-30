#include "hosp.h"

extern "C" {

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

    sprintf(name, "%s%sdt", statvar.f575, statvar.f6c);
    handle = LbFileOpen(name, 0);
    if (handle == -1) {
        debug_msg("Cannot open player details file for writing - %s\n", name);
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

    sprintf(name, "%s%sdt", statvar.f575, statvar.f6c);
    if (!LbFileExists(name)) {
        set_default_player_details();
        fn_000baad0();
        return 0;
    }
    handle = LbFileOpen(name, 2);
    if (handle == -1) {
        debug_msg("Cannot open an existing player details file - %s\n", name);
        return 0;
    }
    LbFileRead(handle, &myplayer, 0x5c);
    if (LbFileClose(handle) == -1) {
        debug_msg("Error closing file after reading from it\n", name);
        return 0;
    }
    fn_000baad0();
    return 1;
}

// FUNCTION: 0x000baa28
void set_default_player_details(void)
{
    myplayer.f1e = 0x19;
    myplayer.f20 = 2;
    myplayer.f9 = 0;
    myplayer.fb = 0;
    myplayer.f1d = 1;
    myplayer.f22 = 1;
    LbMemorySet(myplayer.f23, 0x1e, 0);
    LbMemorySet(myplayer.f41, 0x14, 0);
    LbMemorySet(myplayer.unknown12, 0xb, 0);
    myplayer.f41[20] = 1;
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
    switch (myplayer.f1e) {
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

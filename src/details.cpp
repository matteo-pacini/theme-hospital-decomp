#include "hosp.h"

extern "C" {

void fn_000baad0(void);
void setup_pulldown_menu_options(void);
void FadeMusic(u32 time, u8 volume);
}

void debug_msg(char *fmt, ...);
void set_default_player_details(void);

// Writes the player's settings (myplayer) to the player details file
// (statvar.save_root + statvar.save_prefix + "dt"). Returns 1, 0 if the write was short, or
// 0xff (-1) if the file cannot be opened.
// FUNCTION: 0x000ba8b0
u8 save_player_details(void)
{
    char name[144];
    s32 handle;
    s32 written;

    sprintf(name, "%s%sdt", statvar.save_root, statvar.save_prefix);
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

// Reads the player's settings (myplayer) from the player details file and
// applies them; without a file, applies the defaults. Returns 1 if read, else
// 0.
// FUNCTION: 0x000ba954
u8 load_player_details(void)
{
    char name[256];
    s32 handle;

    sprintf(name, "%s%sdt", statvar.save_root, statvar.save_prefix);
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

// Sets myplayer to the default settings. The three LbMemorySet calls pass
// their size and value swapped, so they clear nothing.
// FUNCTION: 0x000baa28
void set_default_player_details(void)
{
    myplayer.game_speed = 0x19;
    myplayer.speed_choice = 2;
    myplayer.years_played = 0;
    myplayer.fb = 0;
    myplayer.shadows_on = 1;
    myplayer.autosave = 1;
    LbMemorySet(myplayer.room_help_seen, 0x1e, 0);
    LbMemorySet(myplayer.advisor_flags, 0x14, 0);
    LbMemorySet(myplayer.track_enabled, 0xb, 0);
    myplayer.advisor_flags[20] = 1;
    myplayer.sfx_on = 1;
    myplayer.sfx_volume = 0x7f;
    myplayer.music_on = 1;
    myplayer.music_volume = 0x7f;
    myplayer.ambients_on = 1;
    myplayer.ambient_volume = 0x7f;
}

// Applies the player's settings: music volume (myplayer.music_volume), pulldown menu
// ticks, and the game speed from myplayer.game_speed.
// FUNCTION: 0x000baad0
void fn_000baad0(void)
{
    FadeMusic(1000, myplayer.music_volume);
    setup_pulldown_menu_options();
    switch (myplayer.game_speed) {
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

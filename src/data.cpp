#include "hosp.h"

// Data only: config/sections.txt places the module's _DATA.

long face_file = -1;

extern "C" {
// demo: loaded_face (a Picture)
u8 D_001290d0 = 0xff, D_001290d1 = 0xff, D_001290d2 = 0xff;
// demo: save. The entries with no address are filled in by setup_addresses.
MemRegion D_001290d4[] = {
    { (u32)&version, 2 },
    { (u32)&level_number, 1 },
    { (u32)balance_id, 10 },
    { 0, 0x20000 },
    { 0, 0x40164 },
    { 0, 0xdac },
    { 0, 0x14a },
    { 0, 0x651 },
    { 0, 0x3a8 },
    { 0, 0x4964 },
    { 0, 0x258 },
    { 0, 0xc8 },
    { (u32)&window, 4 },
    { (u32)&window.scroll_y, 4 },
    { (u32)&game_turn, 4 },
    { (u32)&actual_game_turn, 4 },
    { (u32)&seed, 4 },
    { (u32)&gamevar, 0x178 },
    { (u32)&award_counts, 0x155 },
    { (u32)&NRAdcounts, 0x11 },
    { (u32)&advisor.event_flags_u32, 4 },
    { 0, 0 },
};
}

char fingchk[20] = "markwebleykjwrdkjf";
char namechk[44] = "Theme Hospital Beta 5 V5                ";

const u8 *pointer_second_frame_check = ingame_pointer_second_frame;

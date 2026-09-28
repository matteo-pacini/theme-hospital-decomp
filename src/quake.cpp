#include "hosp.h"

struct QuakeControl {
    u16 start;
    u16 end;
    u8 magnitude;
    u8 unknown5;
};

/*
 * The earthquake state at 0x15532d. The signed bytes at +5 and +7 are read
 * through the dword three bytes below them, so those reads go through this
 * overlay; their stores address them directly (D_00155332, D_00155334).
 */
struct QuakeState {
    u8 unknown0[2];
    u8 state;
    u8 magnitude;
    u8 unknown4;
    s8 unknown5;
    u8 unknown6;
    s8 unknown7;
};

extern QuakeControl quake_control[];

void setup_next_quake(void);
void do_earthquake(void);
void disaster_peeps(u8 kind, s8 owner);
u8 modify_machine(Thing *w);

extern "C" {

extern QuakeState D_0015532d;
extern s8 D_00155332;
extern s8 D_00155334;
extern u32 D_00155367;
extern u8 D_0015536b;
extern u8 D_0015536c;
extern u8 D_0015538b[];

}

// FUNCTION: 0x00043b90
void generate_quake(void)
{
    switch (D_0015532d.state) {
    case 0:
        if (actual_game_turn >= D_00155367 && D_0015536c) {
            D_0015532d.state = 1;
        }
        break;
    case 1:
        D_0015532d.magnitude = 4;
        D_0015532d.unknown4 = 4;
        D_00155332 = 0;
        D_00155334 = 1;
        D_0015532d.state = 2;
        gamevar.feb = actual_game_turn + old_normal_rand(5, 10) * 50;
        play_ambient(saferand() % 3 + 0xab);
        break;
    case 2:
        if (actual_game_turn > gamevar.feb) {
            D_0015532d.magnitude = 0x10;
            D_0015532d.unknown4 = 4;
            D_00155332 = 0;
            D_00155334 = 1;
            D_0015532d.state = 3;
            gamevar.feb = actual_game_turn + 200;
            disaster_peeps(1, -1);
            play_ambient(saferand() % 3 + 0xab);
        }
        break;
    case 3:
        if (D_0015532d.unknown5 >= D_0015532d.magnitude * 3 / 4 || actual_game_turn > gamevar.feb) {
            D_0015532d.state = 4;
            gamevar.feb = actual_game_turn + 100;
            do_earthquake();
        }
        break;
    case 4:
        if ((D_0015532d.unknown7 == -1 && D_0015532d.unknown5 < 2) || actual_game_turn > gamevar.feb) {
            disaster_peeps(2, -1);
            D_0015532d.state = 0;
            setup_next_quake();
        }
        break;
    }
}

// FUNCTION: 0x00043d88
void setup_next_quake(void)
{
    u32 i;
    u32 offset;
    u32 month;

    month = mycontrol->f648 * 12 + mycontrol->unknown650;
    D_0015536c = 0;
    for (i = 0; i < 16; i++) {
        if (D_0015538b[i] == 0) {
            D_0015538b[i] = 1;
            if (quake_control[i].end > month && quake_control[i].end > quake_control[i].start) {
                offset = old_newrand() % ((quake_control[i].end + 1 - quake_control[i].start) * 1500);
                D_00155367 = offset + quake_control[i].start * 1500;
                D_0015536b = quake_control[i].magnitude;
                D_0015536c = 1;
                break;
            }
        }
    }
}

// FUNCTION: 0x00043eac
void do_earthquake(void)
{
    RoomInfo *room;
    Thing *t;

    for (t = thing + 1; t < end_things; t++) {
        if (t->renderer == 6) {
            if (objects[t->subtype].unknown1e) {
            if (statvar.unknownfd > 1) {
                room = &room_info[t->unknown89];
                if (room > room_info && (room->f3 == 4 || room->f22)) {
                    continue;
                }
            } else {
                if (t->unknown89 && t->unknown89 == allhosps[t->owner].unknown11b7) {
                    continue;
                }
            }
            if (t->kind != 0x12) {
                t->mode += D_0015536b;
                if (t->value * 75 / 100 <= t->mode) {
                    modify_machine(t);
                }
            }
            }
        }
    }
}

#include "hosp.h"

struct QuakeControl {
    u16 start;
    u16 end;
    u8 magnitude;
    u8 unknown5;
};

extern QuakeControl quake_control[];

void setup_next_quake(void);
void do_earthquake(void);
void disaster_peeps(u8 kind, s8 owner);
u8 modify_machine(Thing *w);

extern "C" {


}

// Earthquake state machine (gamevar.fef): at the scheduled turn
// (gamevar.f127) a warning tremor, then the main quake shaking the screen
// and scaring people, damage to the machines (do_earthquake), and once it
// dies down the next quake is scheduled.
// FUNCTION: 0x00043b90
void generate_quake(void)
{
    switch (gamevar.fef) {
    case 0:
        if (actual_game_turn >= gamevar.f127 && gamevar.f12c) {
            gamevar.fef = 1;
        }
        break;
    case 1:
        gamevar.ff0 = 4;
        gamevar.ff1 = 4;
        gamevar.ff2 = 0;
        gamevar.ff4 = 1;
        gamevar.fef = 2;
        gamevar.feb = actual_game_turn + old_normal_rand(5, 10) * 50;
        play_ambient(saferand() % 3 + 0xab);
        break;
    case 2:
        if (actual_game_turn > gamevar.feb) {
            gamevar.ff0 = 0x10;
            gamevar.ff1 = 4;
            gamevar.ff2 = 0;
            gamevar.ff4 = 1;
            gamevar.fef = 3;
            gamevar.feb = actual_game_turn + 200;
            disaster_peeps(1, -1);
            play_ambient(saferand() % 3 + 0xab);
        }
        break;
    case 3:
        if (gamevar.ff2 >= gamevar.ff0 * 3 / 4 || actual_game_turn > gamevar.feb) {
            gamevar.fef = 4;
            gamevar.feb = actual_game_turn + 100;
            do_earthquake();
        }
        break;
    case 4:
        if ((gamevar.ff4 == -1 && gamevar.ff2 < 2) || actual_game_turn > gamevar.feb) {
            disaster_peeps(2, -1);
            gamevar.fef = 0;
            setup_next_quake();
        }
        break;
    }
}

// Schedules the next unused quake_control entry that ends after the current
// month at a random turn within its months (1500 turns a month), with its
// magnitude (gamevar.f12b). Clears gamevar.f12c if none is left.
// FUNCTION: 0x00043d88
void setup_next_quake(void)
{
    u32 i;
    u32 offset;
    u32 month;

    month = mycontrol->f648 * 12 + mycontrol->unknown650;
    gamevar.f12c = 0;
    for (i = 0; i < 16; i++) {
        if (gamevar.f14b[i] == 0) {
            gamevar.f14b[i] = 1;
            if (quake_control[i].end > month && quake_control[i].end > quake_control[i].start) {
                offset = old_newrand() % ((quake_control[i].end + 1 - quake_control[i].start) * 1500);
                gamevar.f127 = offset + quake_control[i].start * 1500;
                gamevar.f12b = quake_control[i].magnitude;
                gamevar.f12c = 1;
                break;
            }
        }
    }
}

// Wears every machine by the quake's magnitude (gamevar.f12b uses),
// updating those past 75% of their strength (modify_machine). Machines in a
// room being edited or rebuilt are spared.
// FUNCTION: 0x00043eac
void do_earthquake(void)
{
    RoomInfo *room;
    Thing *obj;

    for (obj = thing + 1; obj < end_things; obj++) {
        if (obj->type == THING_OBJECT) {
            if (objects[obj->subtype].unknown1e) {
            if (statvar.unknownfd > 1) {
                room = &room_info[obj->room_id];
                if (room > room_info && (room->f3 == 4 || room->f22)) {
                    continue;
                }
            } else {
                if (obj->room_id && obj->room_id == allhosps[obj->owner].unknown11b7) {
                    continue;
                }
            }
            if (obj->state != 0x12) {
                obj->times_used += gamevar.f12b;
                if (obj->strength * 75 / 100 <= obj->times_used) {
                    modify_machine(obj);
                }
            }
            }
        }
    }
}

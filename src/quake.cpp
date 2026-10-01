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

// Earthquake state machine (gamevar.quake_phase): at the scheduled turn
// (gamevar.next_quake_turn) a warning tremor, then the main quake shaking the screen
// and scaring people, damage to the machines (do_earthquake), and once it
// dies down the next quake is scheduled.
// FUNCTION: 0x00043b90
void generate_quake(void)
{
    switch (gamevar.quake_phase) {
    case 0:
        if (actual_game_turn >= gamevar.next_quake_turn && gamevar.quake_pending) {
            gamevar.quake_phase = 1;
        }
        break;
    case 1:
        gamevar.quake_strength = 4;
        gamevar.quake_shake_chance = 4;
        gamevar.quake_amplitude = 0;
        gamevar.quake_ramp = 1;
        gamevar.quake_phase = 2;
        gamevar.quake_timer = actual_game_turn + old_normal_rand(5, 10) * 50;
        play_ambient(saferand() % 3 + 0xab);
        break;
    case 2:
        if (actual_game_turn > gamevar.quake_timer) {
            gamevar.quake_strength = 0x10;
            gamevar.quake_shake_chance = 4;
            gamevar.quake_amplitude = 0;
            gamevar.quake_ramp = 1;
            gamevar.quake_phase = 3;
            gamevar.quake_timer = actual_game_turn + 200;
            disaster_peeps(1, -1);
            play_ambient(saferand() % 3 + 0xab);
        }
        break;
    case 3:
        if (gamevar.quake_amplitude >= gamevar.quake_strength * 3 / 4 || actual_game_turn > gamevar.quake_timer) {
            gamevar.quake_phase = 4;
            gamevar.quake_timer = actual_game_turn + 100;
            do_earthquake();
        }
        break;
    case 4:
        if ((gamevar.quake_ramp == -1 && gamevar.quake_amplitude < 2) || actual_game_turn > gamevar.quake_timer) {
            disaster_peeps(2, -1);
            gamevar.quake_phase = 0;
            setup_next_quake();
        }
        break;
    }
}

// Schedules the next unused quake_control entry that ends after the current
// month at a random turn within its months (1500 turns a month), with its
// magnitude (gamevar.quake_severity). Clears gamevar.quake_pending if none is left.
// FUNCTION: 0x00043d88
void setup_next_quake(void)
{
    u32 i;
    u32 offset;
    u32 month;

    month = mycontrol->year * 12 + mycontrol->month;
    gamevar.quake_pending = 0;
    for (i = 0; i < 16; i++) {
        if (gamevar.quake_used[i] == 0) {
            gamevar.quake_used[i] = 1;
            if (quake_control[i].end > month && quake_control[i].end > quake_control[i].start) {
                offset = old_newrand() % ((quake_control[i].end + 1 - quake_control[i].start) * 1500);
                gamevar.next_quake_turn = offset + quake_control[i].start * 1500;
                gamevar.quake_severity = quake_control[i].magnitude;
                gamevar.quake_pending = 1;
                break;
            }
        }
    }
}

// Wears every machine by the quake's magnitude (gamevar.quake_severity uses),
// updating those past 75% of their strength (modify_machine). Machines in a
// room being edited or rebuilt are spared.
// FUNCTION: 0x00043eac
void do_earthquake(void)
{
    RoomInfo *room;
    Thing *obj;

    for (obj = thing + 1; obj < end_things; obj++) {
        if (obj->type == THING_OBJECT) {
            if (objects[obj->subtype].overlay_anim) {
            if (statvar.num_players > 1) {
                room = &room_info[obj->room_id];
                if (room > room_info && (room->state == 4 || room->being_edited)) {
                    continue;
                }
            } else {
                if (obj->room_id && obj->room_id == allhosps[obj->owner].edited_room) {
                    continue;
                }
            }
            if (obj->state != 0x12) {
                obj->times_used += gamevar.quake_severity;
                if (obj->strength * 75 / 100 <= obj->times_used) {
                    modify_machine(obj);
                }
            }
            }
        }
    }
}

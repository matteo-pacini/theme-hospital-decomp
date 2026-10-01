#include "hosp.h"

void reset_all_count(void);
void check_win_status(void);
void final_count(void);
void person_count(Thing *w);
void staff_count(Thing *w);
void object_count(const Thing *w);
void update_icon_counter(Thing *const w);
s16 person_goto_destination(Thing *w);
void person_complex_retry(Thing *w);
s16 person_goto_object(Thing *w);
void person_goto_queue(Thing *w);
void peep_stuck(Thing *w);
void peep_bp_goto(Thing *w);
void modify_staff(Thing *w);
s16 staff_waits(Thing *w);
void doctor_in_office(Thing *w);
void doctor_works_at_desk(Thing *w);
void receptionist_works(Thing *w);
void receptionist_finds_reception(Thing *w);
void play_complex_staff_anim(Thing *w);
void surgeon_waits(Thing *w);
s16 staff_goto_object(Thing *w);
void doctor_waits_for_patient_to_arrive(Thing *w);
void doctor_carries_out_examination(Thing *w);
void handyman_works(Thing *w);
void recept_waits_for_patient(Thing *w);
void recept_admits_patient(Thing *w);
void doctor_in_machine_room(Thing *w);
void staff_wait_for_complex_slave(Thing *w);
void staff_in_corridor(Thing *w);
void staff_complex_retry(Thing *w);
void remove_staff_in_struct(Thing *w);
void staff_in_staff_room(Thing *w);
void nurse_in_ward(Thing *w);
void staff_wanders(Thing *w);
void doctor_researches(Thing *w);
void nurse_vaccinates(Thing *w);
void doctor_in_theatre(Thing *w);
void doctor_is_trained(Thing *w);
void consultant_trains(Thing *w);
void receptionist_setup(Thing *w);
void play_aniseq(Thing *w);
void object_static(Thing *w);
void plant_ages(Thing *w);
void double_door_swings(Thing *w);
void sliding_door_closed(Thing *w);
void sliding_door_opens(Thing *w);
void sliding_door_open(Thing *w);
void sliding_door_closes(Thing *w);
void bomb_ticks(Thing *w);
void reaper_play_anim(Thing *w);
void reaper_waits(Thing *w);
void remove_reaper(Thing *w);
s16 reaper_goto_hell(Thing *w);
void rat_scurries(Thing *w);
void rat_arrives(Thing *w);
void rat_leaves(Thing *w);
void process_bang(Thing *w);
void helicopter_lands(Thing *w);
void helicopter_leaves(Thing *w);
void saucer_abducts(Thing *w);
void numfx_ages(Thing *w);
s16 mayor_goto(Thing *w);
void mayor_pauses(Thing *w);
void mayor_gone(Thing *w);
void inspector_pauses(Thing *w);

extern "C" {
void fn_00015b68(Thing *w);
void fn_0001673c(Thing *w);
void fn_00017cb0(Thing *w);
void fn_0003751c(Thing *w);
void fn_00041a54(Thing *w);
void fn_00041b08(Thing *w);
void fn_00054870(Thing *w);
}

// FUNCTION: 0x0005cc70
void process_things(Thing *first, Thing *last)
{
    Thing *cur;
    s32 happiness_sum;
    s32 happiness_count;

    happiness_sum = 0;
    happiness_count = 0;
    debug.thing_count = 0;
    debug.bang_count = 0;
    reset_all_count();
    check_win_status();
    for (cur = first; cur < last; cur++) {
        switch (cur->renderer) {
        case 0:
            break;
        case 2:
            debug.thing_count++;
            person_count(cur);
            if (cur->owner == player) {
                happiness_sum += cur->unknown81s;
                happiness_count++;
            }
            update_icon_counter(cur);
            switch (cur->kind) {
            case 4:
                person_waits_for_busy(cur);
                break;
            case 6:
                person_goto_destination(cur);
                break;
            case 8:
                return_to_queue_area(cur);
                break;
            case 0xc:
                person_go_home(cur);
                break;
            case 0xe:
                fn_0001673c(cur);
                break;
            case 0x10:
                person_complex_retry(cur);
                break;
            case 0x12:
                person_arrives_at_door(cur);
                break;
            case 0x14:
                person_in_queue(cur);
                break;
            case 0x16:
                person_goto_object(cur);
                break;
            case 0x18:
                person_entertains_themself(cur);
                break;
            case 0x1a:
                fn_00041b08(cur);
                break;
            case 0x1c:
                break;
            case 0x20:
            case 0x42:
                break;
            case 0x22:
                fn_00054870(cur);
                break;
            case 0x24:
                person_goto_queue(cur);
                break;
            case 0x26:
                person_go_for_treatment(cur);
                break;
            case 0x28:
            case 0xf2:
                break;
            case 0xf0:
                nav_debug_state(cur);
                break;
            case 0x2a:
                fn_00041a54(cur);
                break;
            case 0x2c:
                person_in_bed(cur);
                break;
            case 0x2e:
                person_fills(cur);
                break;
            case 0x30:
                person_energises(cur);
                break;
            case 0x32:
                person_dead(cur);
                break;
            case 0x34:
                fn_00017cb0(cur);
                break;
            case 0x36:
                person_in_bogs(cur);
                break;
            case 0x38:
                leave_toilets(cur);
                break;
            case 0x3a:
                play_person_showanim(cur);
                break;
            case 0x3c:
                peep_stuck(cur);
                break;
            case 0x40:
                peep_bp_goto(cur);
                break;
            case 0x44:
                person_waits_for_decision(cur);
                break;
            case 0x46:
                person_arrived_from_call(cur);
                break;
            case 0x48:
                break;
            default:
                break;
            }
            break;
        case 4:
            debug.thing_count++;
            staff_count(cur);
            update_icon_counter(cur);
            modify_staff(cur);
            switch (cur->kind) {
            case 2:
            case 4:
                staff_waits(cur);
                break;
            case 6:
                doctor_in_office(cur);
                break;
            case 8:
                doctor_works_at_desk(cur);
                break;
            case 0xa:
                receptionist_works(cur);
                break;
            case 0xc:
                receptionist_finds_reception(cur);
                break;
            case 0xe:
                play_complex_staff_anim(cur);
                break;
            case 0x10:
                surgeon_waits(cur);
                break;
            case 0x12:
                staff_goto_object(cur);
                break;
            case 0x1e:
                break;
            case 0x14:
                doctor_waits_for_patient_to_arrive(cur);
                break;
            case 0x18:
                break;
            case 0x1a:
                fn_00054870(cur);
                break;
            case 0x1c:
                doctor_carries_out_examination(cur);
                break;
            case 0x3c:
                handyman_works(cur);
                break;
            case 0x20:
                recept_waits_for_patient(cur);
                break;
            case 0x22:
                recept_admits_patient(cur);
                break;
            case 0x24:
                doctor_in_machine_room(cur);
                break;
            case 0x26:
                staff_wait_for_complex_slave(cur);
                break;
            case 0x28:
                staff_in_corridor(cur);
                break;
            case 0x2a:
            case 0x3a:
            case 0xf2:
                break;
            case 0x2c:
                staff_complex_retry(cur);
                break;
            case 0x2e:
                if (actual_game_turn > cur->unknown148d) {
                    if (statvar.unknownfd <= 1) {
                        if ((u16)(cur - thing) != gamevar.unknown75)
                            remove_staff_in_struct(cur);
                    } else {
                        if ((u16)(cur - thing) == gamevar.unknown75) {
                            gamevar.unknown75 = 0;
                            gamevar.unknown77 = 0;
                            set_mouse_mode(0);
                        }
                        remove_staff_in_struct(cur);
                    }
                }
                break;
            case 0xf0:
                nav_debug_state(cur);
                break;
            case 0x30:
                staff_in_staff_room(cur);
                break;
            case 0x32:
                nurse_in_ward(cur);
                break;
            case 0x34:
                staff_wanders(cur);
                break;
            case 0x36:
                process_staff_complex(cur);
                break;
            case 0x38:
                fn_00015b68(cur);
                cur->kind = 0x2e;
                cur->unknown130 = actual_game_turn;
                gamevar.f6[cur->subtype]++;
                gamevar.f4++;
                break;
            case 0x3e:
                doctor_researches(cur);
                break;
            case 0x40:
                nurse_vaccinates(cur);
                break;
            case 0x42:
                doctor_in_theatre(cur);
                break;
            case 0x46:
                doctor_is_trained(cur);
                break;
            case 0x48:
                consultant_trains(cur);
                break;
            case 0x4a:
                receptionist_setup(cur);
                break;
            case 0x44:
                break;
            case 0x4c:
                peep_bp_goto(cur);
                break;
            case 0x4e:
                peep_stuck(cur);
                break;
            case 0x50:
                play_aniseq(cur);
                break;
            default:
                break;
            }
            break;
        case 6:
            debug.thing_count++;
            object_count(cur);
            update_icon_counter(cur);
            switch (cur->kind) {
            case 2:
                object_static(cur);
                break;
            case 4:
                if (cur->icon)
                    cur->icon = frame[cur->icon].next;
            case 0xe:
            case 0x12:
                break;
            case 6:
                fn_0003751c(cur);
                break;
            case 0xa:
                plant_ages(cur);
                break;
            case 0x10:
                double_door_swings(cur);
                break;
            case 0x14:
                sliding_door_closed(cur);
                break;
            case 0x16:
                sliding_door_opens(cur);
                break;
            case 0x18:
                sliding_door_open(cur);
                break;
            case 0x1a:
                sliding_door_closes(cur);
                break;
            case 0x1c:
                bomb_ticks(cur);
                break;
            default:
                break;
            }
            break;
        case 8:
            debug.thing_count++;
            switch (cur->kind) {
            case 0x10:
                double_door_swings(cur);
                break;
            default:
                if (actual_game_turn % cur->anim_speed == 0)
                    cur->sprite = frame[cur->sprite].next;
                break;
            }
            break;
        case 0xa:
            debug.thing_count++;
            switch (cur->kind) {
            case 2:
                reaper_play_anim(cur);
                break;
            case 4:
                reaper_waits(cur);
                break;
            case 6:
                remove_reaper(cur);
                break;
            case 8:
                reaper_goto_hell(cur);
                break;
            case 0xf2:
                break;
            default:
                break;
            }
            break;
        case 0xc:
            debug.thing_count++;
            switch (cur->kind) {
            case 2:
                rat_scurries(cur);
                break;
            case 4:
                rat_arrives(cur);
                break;
            case 6:
                rat_leaves(cur);
                break;
            default:
                break;
            }
            break;
        case 0xe:
            debug.thing_count++;
            debug.bang_count++;
            debug.bang_max < debug.bang_count ? (debug.bang_max = debug.bang_count) : 0;
            process_bang(cur);
            break;
        case 0x10:
            debug.thing_count++;
            switch (cur->kind) {
            case 2:
                helicopter_lands(cur);
                break;
            case 4:
                cur->sprite = frame[cur->sprite].next;
                break;
            case 6:
                helicopter_leaves(cur);
                break;
            case 8:
                saucer_abducts(cur);
                break;
            default:
                break;
            }
            break;
        case 0x12:
            debug.thing_count++;
            numfx_ages(cur);
            break;
        case 0x14:
            debug.thing_count++;
            switch (cur->kind) {
            case 2:
                mayor_goto(cur);
                break;
            case 4:
                mayor_pauses(cur);
                break;
            case 6:
                mayor_gone(cur);
                break;
            default:
                break;
            }
            break;
        case 0x16:
            debug.thing_count++;
            switch (cur->kind) {
            case 2:
                mayor_goto(cur);
                break;
            case 4:
                inspector_pauses(cur);
                break;
            case 6:
                mayor_gone(cur);
                break;
            case 8:
                inspector_pauses(cur);
                break;
            default:
                break;
            }
            break;
        case 0x18:
            break;
        default:
            break;
        }
    }
    final_count();
}

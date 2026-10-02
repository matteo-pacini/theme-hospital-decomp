#include "hosp.h"

void reset_all_count(void);
void check_win_status(void);
void final_count(void);
void person_count(Thing *person);
void staff_count(Thing *employee);
void object_count(const Thing *obj);
void update_icon_counter(Thing *const peep);
s16 person_goto_destination(Thing *person);
void play_complex_person_anim(Thing *person);
s16 person_goto_object(Thing *person);
void person_goto_queue(Thing *person);
void peep_stuck(Thing *peep);
void peep_bp_goto(Thing *peep);
void modify_staff(Thing *employee);
s16 staff_waits(Thing *worker);
void doctor_in_office(Thing *doctor);
void doctor_works_at_desk(Thing *doctor);
void receptionist_works(Thing *receptionist);
void receptionist_finds_reception(Thing *receptionist);
void play_complex_staff_anim(Thing *employee);
void surgeon_waits(Thing *surgeon);
s16 staff_goto_object(Thing *employee);
void doctor_waits_for_patient_to_arrive(Thing *doctor);
void doctor_carries_out_examination(Thing *doctor);
void handyman_works(Thing *handyman);
void recept_waits_for_patient(Thing *receptionist);
void recept_admits_patient(Thing *receptionist);
void doctor_in_machine_room(Thing *doctor);
void staff_wait_for_complex_slave(Thing *employee);
void staff_in_corridor(Thing *staff);
void staff_complex_retry(Thing *employee);
void remove_staff_in_struct(Thing *staff);
void staff_in_staff_room(Thing *staff);
void nurse_in_ward(Thing *nurse);
void staff_wanders(Thing *employee);
void doctor_researches(Thing *doctor);
void nurse_vaccinates(Thing *nurse);
void doctor_in_theatre(Thing *doctor);
void doctor_is_trained(Thing *doctor);
void consultant_trains(Thing *consultant);
void receptionist_setup(Thing *receptionist);
void play_aniseq(Thing *w);
void object_static(Thing *obj);
void plant_ages(Thing *plant);
void double_door_swings(Thing *door);
void sliding_door_closed(Thing *door);
void sliding_door_opens(Thing *door);
void sliding_door_open(Thing *door);
void sliding_door_closes(Thing *door);
void bomb_ticks(Thing *bomb);
void reaper_play_anim(Thing *reaper);
void reaper_waits(Thing *reaper);
void remove_reaper(Thing *reaper);
s16 reaper_goto_hell(Thing *reaper);
void rat_scurries(Thing *rat);
void rat_arrives(Thing *rat);
void rat_leaves(Thing *rat);
void process_bang(Thing *bang);
void helicopter_lands(Thing *heli);
void helicopter_leaves(Thing *heli);
void saucer_abducts(Thing *w);
void numfx_ages(Thing *fx);
s16 mayor_goto(Thing *mayor);
void mayor_pauses(Thing *mayor);
void mayor_gone(Thing *mayor);
void inspector_pauses(Thing *inspector);

extern "C" {
void remove_node(Thing *node);
void person_gone(Thing *person);
void person_flys(Thing *person);
void object_hell(Thing *obj);
void person_complex_retry(Thing *person);
void person_wait_for_complex_slave(Thing *person);
void two_peeps_interact(Thing *w);
}

// Runs one turn of every thing from first up to (not including) last: the
// state routine for each type and state, and the per-turn counts
// (reset_all_count before, final_count after). Checks the win status first.
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
        switch (cur->type) {
        case THING_FREE:
            break;
        case THING_PATIENT:
            debug.thing_count++;
            person_count(cur);
            if (cur->owner == player) {
                happiness_sum += cur->happiness_s;
                happiness_count++;
            }
            update_icon_counter(cur);
            switch (cur->state) {
            case PS_WAITS_FOR_BUSY:
                person_waits_for_busy(cur);
                break;
            case PS_GOTO_DESTINATION:
                person_goto_destination(cur);
                break;
            case PS_RETURN_TO_QUEUE_AREA:
                return_to_queue_area(cur);
                break;
            case PS_GO_HOME:
                person_go_home(cur);
                break;
            case PS_GONE:
                person_gone(cur);
                break;
            case PS_PLAY_COMPLEX_ANIM:
                play_complex_person_anim(cur);
                break;
            case PS_ARRIVES_AT_DOOR:
                person_arrives_at_door(cur);
                break;
            case PS_IN_QUEUE:
                person_in_queue(cur);
                break;
            case PS_GOTO_OBJECT:
                person_goto_object(cur);
                break;
            case PS_ENTERTAINS_THEMSELF:
                person_entertains_themself(cur);
                break;
            case PS_WAIT_FOR_COMPLEX_SLAVE:
                person_wait_for_complex_slave(cur);
                break;
            case 0x1c:
                break;
            case PS_SLAVE_READY:
            case PS_AT_HELL_GATE:
                break;
            case PS_TWO_PEEPS_INTERACT:
                two_peeps_interact(cur);
                break;
            case PS_GOTO_QUEUE:
                person_goto_queue(cur);
                break;
            case PS_GO_FOR_TREATMENT:
                person_go_for_treatment(cur);
                break;
            case PS_MERGED:
            case 0xf2:
                break;
            case PS_NAV_DEBUG:
                nav_debug_state(cur);
                break;
            case PS_COMPLEX_RETRY:
                person_complex_retry(cur);
                break;
            case PS_IN_BED:
                person_in_bed(cur);
                break;
            case PS_FILLS:
                person_fills(cur);
                break;
            case PS_ENERGISES:
                person_energises(cur);
                break;
            case PS_DEAD:
                person_dead(cur);
                break;
            case PS_FLYS:
                person_flys(cur);
                break;
            case PS_IN_BOGS:
                person_in_bogs(cur);
                break;
            case PS_LEAVE_TOILETS:
                leave_toilets(cur);
                break;
            case PS_SHOWANIM:
                play_person_showanim(cur);
                break;
            case PS_STUCK:
                peep_stuck(cur);
                break;
            case PS_BP_GOTO:
                peep_bp_goto(cur);
                break;
            case PS_WAITS_FOR_DECISION:
                person_waits_for_decision(cur);
                break;
            case PS_ARRIVED_FROM_CALL:
                person_arrived_from_call(cur);
                break;
            case 0x48:
                break;
            default:
                break;
            }
            break;
        case THING_STAFF:
            debug.thing_count++;
            staff_count(cur);
            update_icon_counter(cur);
            modify_staff(cur);
            switch (cur->state) {
            case 2:
            case 4:
                staff_waits(cur);
                break;
            case SS_DOCTOR_IN_OFFICE:
                doctor_in_office(cur);
                break;
            case SS_DOCTOR_WORKS_AT_DESK:
                doctor_works_at_desk(cur);
                break;
            case SS_RECEPTIONIST_WORKS:
                receptionist_works(cur);
                break;
            case SS_RECEPTIONIST_FINDS_RECEPTION:
                receptionist_finds_reception(cur);
                break;
            case SS_PLAY_COMPLEX_ANIM:
                play_complex_staff_anim(cur);
                break;
            case SS_SURGEON_WAITS:
                surgeon_waits(cur);
                break;
            case SS_GOTO_OBJECT:
                staff_goto_object(cur);
                break;
            case 0x1e:
                break;
            case SS_DOCTOR_WAITS_FOR_PATIENT:
                doctor_waits_for_patient_to_arrive(cur);
                break;
            case SS_ASSISTANT_READY:
                break;
            case SS_TWO_PEEPS_INTERACT:
                two_peeps_interact(cur);
                break;
            case SS_DOCTOR_EXAMINES:
                doctor_carries_out_examination(cur);
                break;
            case SS_HANDYMAN_WORKS:
                handyman_works(cur);
                break;
            case SS_RECEPT_WAITS_FOR_PATIENT:
                recept_waits_for_patient(cur);
                break;
            case SS_RECEPT_ADMITS_PATIENT:
                recept_admits_patient(cur);
                break;
            case SS_DOCTOR_IN_MACHINE_ROOM:
                doctor_in_machine_room(cur);
                break;
            case SS_WAIT_FOR_COMPLEX_SLAVE:
                staff_wait_for_complex_slave(cur);
                break;
            case SS_IN_CORRIDOR:
                staff_in_corridor(cur);
                break;
            case 0x2a:
            case 0x3a:
            case 0xf2:
                break;
            case SS_COMPLEX_RETRY:
                staff_complex_retry(cur);
                break;
            case SS_APPLICANT:
                if (actual_game_turn > cur->hire_expiry) {
                    if (statvar.num_players <= 1) {
                        if ((u16)(cur - thing) != gamevar.held_staff)
                            remove_staff_in_struct(cur);
                    } else {
                        if ((u16)(cur - thing) == gamevar.held_staff) {
                            gamevar.held_staff = 0;
                            gamevar.held_staff_kind = 0;
                            set_mouse_mode(MM_NORMAL);
                        }
                        remove_staff_in_struct(cur);
                    }
                }
                break;
            case SS_NAV_DEBUG:
                nav_debug_state(cur);
                break;
            case SS_IN_STAFF_ROOM:
                staff_in_staff_room(cur);
                break;
            case SS_NURSE_IN_WARD:
                nurse_in_ward(cur);
                break;
            case SS_WANDERS:
                staff_wanders(cur);
                break;
            case SS_EVACUATE:
                evacuate_staff(cur);
                break;
            case SS_BACK_TO_APPLICANTS:
                remove_node(cur);
                cur->state = SS_APPLICANT;
                cur->hire_time = actual_game_turn;
                gamevar.applicants[cur->subtype]++;
                gamevar.applicant_count++;
                break;
            case SS_DOCTOR_RESEARCHES:
                doctor_researches(cur);
                break;
            case SS_NURSE_VACCINATES:
                nurse_vaccinates(cur);
                break;
            case SS_DOCTOR_IN_THEATRE:
                doctor_in_theatre(cur);
                break;
            case SS_DOCTOR_IS_TRAINED:
                doctor_is_trained(cur);
                break;
            case SS_CONSULTANT_TRAINS:
                consultant_trains(cur);
                break;
            case SS_RECEPTIONIST_SETUP:
                receptionist_setup(cur);
                break;
            case SS_HELD:
                break;
            case SS_BP_GOTO:
                peep_bp_goto(cur);
                break;
            case SS_STUCK:
                peep_stuck(cur);
                break;
            case SS_PLAY_ANISEQ:
                play_aniseq(cur);
                break;
            default:
                break;
            }
            break;
        case THING_OBJECT:
            debug.thing_count++;
            object_count(cur);
            update_icon_counter(cur);
            switch (cur->state) {
            case OS_STATIC:
                object_static(cur);
                break;
            case OS_IN_USE:
                if (cur->overlay_frame)
                    cur->overlay_frame = frame[cur->overlay_frame].next;
            case OS_BEING_WATERED:
            case OS_REMOVED:
                break;
            case OS_HELL:
                object_hell(cur);
                break;
            case OS_PLANT_AGES:
                plant_ages(cur);
                break;
            case OS_DOUBLE_DOOR_SWINGS:
                double_door_swings(cur);
                break;
            case OS_SLIDING_DOOR_CLOSED:
                sliding_door_closed(cur);
                break;
            case OS_SLIDING_DOOR_OPENS:
                sliding_door_opens(cur);
                break;
            case OS_SLIDING_DOOR_OPEN:
                sliding_door_open(cur);
                break;
            case OS_SLIDING_DOOR_CLOSES:
                sliding_door_closes(cur);
                break;
            case OS_BOMB_TICKS:
                bomb_ticks(cur);
                break;
            default:
                break;
            }
            break;
        case THING_DOOR:
            debug.thing_count++;
            switch (cur->state) {
            case OS_DOUBLE_DOOR_SWINGS:
                double_door_swings(cur);
                break;
            default:
                if (actual_game_turn % cur->anim_speed == 0)
                    cur->sprite = frame[cur->sprite].next;
                break;
            }
            break;
        case THING_REAPER:
            debug.thing_count++;
            switch (cur->state) {
            case REAPER_PLAY_ANIM:
                reaper_play_anim(cur);
                break;
            case REAPER_WAITS:
                reaper_waits(cur);
                break;
            case REAPER_REMOVE:
                remove_reaper(cur);
                break;
            case REAPER_GOTO_HELL:
                reaper_goto_hell(cur);
                break;
            case 0xf2:
                break;
            default:
                break;
            }
            break;
        case THING_RAT:
            debug.thing_count++;
            switch (cur->state) {
            case RAT_SCURRIES:
                rat_scurries(cur);
                break;
            case RAT_ARRIVES:
                rat_arrives(cur);
                break;
            case RAT_LEAVES:
                rat_leaves(cur);
                break;
            default:
                break;
            }
            break;
        case THING_DEBRIS:
            debug.thing_count++;
            debug.bang_count++;
            debug.bang_max < debug.bang_count ? (debug.bang_max = debug.bang_count) : 0;
            process_bang(cur);
            break;
        case THING_HELICOPTER:
            debug.thing_count++;
            switch (cur->state) {
            case HELI_LANDS:
                helicopter_lands(cur);
                break;
            case HELI_WAITS:
                cur->sprite = frame[cur->sprite].next;
                break;
            case HELI_LEAVES:
                helicopter_leaves(cur);
                break;
            case HELI_ABDUCTS:
                saucer_abducts(cur);
                break;
            default:
                break;
            }
            break;
        case THING_NUMFX:
            debug.thing_count++;
            numfx_ages(cur);
            break;
        case THING_MAYOR:
            debug.thing_count++;
            switch (cur->state) {
            case MAYOR_GOTO:
                mayor_goto(cur);
                break;
            case MAYOR_PAUSES:
                mayor_pauses(cur);
                break;
            case MAYOR_GONE:
                mayor_gone(cur);
                break;
            default:
                break;
            }
            break;
        case THING_INSPECTOR:
            debug.thing_count++;
            switch (cur->state) {
            case MAYOR_GOTO:
                mayor_goto(cur);
                break;
            case MAYOR_PAUSES:
                inspector_pauses(cur);
                break;
            case MAYOR_GONE:
                mayor_gone(cur);
                break;
            case MAYOR_INSPECTS:
                inspector_pauses(cur);
                break;
            default:
                break;
            }
            break;
        case THING_SHADOW:
            break;
        default:
            break;
        }
    }
    final_count();
}

#include "hosp.h"

extern s16 dying_place[];
extern PossibleDiag possible_diagnoses[];
extern s16 diag_register[][8];

extern "C" {

extern char **illness_text;

void strcat(char *dst, char *src);
}

// staff has given patient their current treatment: applies its diagnosis
// progress, records it and sets up the next step (unless the room has been
// destroyed).
// FUNCTION: 0x0003c360
void done_treatment(Thing *staff, Thing *patient)
{
    if (patient->room != 0 && patient->room->type == ROOM_DESTROYED) {
        return;
    }
    affect_diagnosis(staff, patient);
    add_treatment_to_history(patient);
    setup_next_treatment(staff, patient);
}

// For a diagnosis treatment (GP or a diagnosis room), raises patient's
// diagnosis progress by staff's diagnostic ability (calc_ability) plus the
// room's bonus for the illness, less the patient's handicap; a room bad for
// the illness costs 5 health. staff's ability rises by 1.
// FUNCTION: 0x0003c3b8
void affect_diagnosis(Thing *staff, Thing *patient)
{
    s32 total;
    s16 bonus;

    bonus = 0;
    if (patient->treatment >= 0x24 || patient->treatment == ILL_GENERAL_PRACTICE) {
        total = patient->diagnosis - patient->diagnosis_handicap;
        total += calc_ability(staff);
        if (patient->treatment >= 0x24) {
            bonus = diag_register[patient->illness][patient->treatment - 0x24];
        }
        total += bonus;
        if (patient->diagnosis < total) {
            patient->diagnosis = total;
        }
        if (bonus < 0) {
            change_health(patient, -5);
        }
    }
    staff->ability++;
    if (staff->ability < 30) {
        staff->ability = 30;
    } else if (staff->ability > 1000) {
        staff->ability = 1000;
    }
}

// After a treatment step: charges for it and moves patient on, by the
// treatment's kind: home, back to the GP, cured (or killed after a wrong
// guess), diagnosed, or on to the next treatment. When diagnosis is
// inconclusive, guesses the illness or gives up by the hospital's policy
// thresholds (guess_cure_threshold, send_home_threshold). Then sets up the next state
// (next_treatment_now_setup).
// FUNCTION: 0x0003c4ec
void setup_next_treatment(Thing *staff, Thing *patient)
{
    u32 pct;
    s32 pay;

    pay = 1;
    patient->impatience = 0;
    patient->waiting_time = 0;
    reputation_affected(patient->treatment, REP_NEXT_TREATMENT, 0, patient->owner, 0);
    switch (expertise[patient->treatment].outcome) {
    case 0:
        pay_for_treatment(patient, 2);
        patient->treatment = 0;
        break;
    case 1:
        pay_for_treatment(patient, 2);
        patient->treatment = ILL_GENERAL_PRACTICE;
        pay = shall_peep_pay(patient);
        if (pay) {
            pay_for_treatment(patient, 3);
        }
        break;
    case 2:
        pay_for_treatment(patient, 2);
        patient->treatment = 0;
        effect_illness(patient, patient->illness, 1);
        if (staff > thing) {
            staff->happiness += 5;
        }
        if (patient->contagion) {
            clean_contagious_peep(patient);
        }
        patient->flags &= ~PF_AWAITING_DECISION;
        reset_icon_value(patient, ICON_AWAITING_DECISION);
        if (patient->flags & PF_WRONG_GUESS) {
            set_icon_value(patient, ICON_KILLED);
            change_health(patient, -1000);
            play_sound(SNDSRC_GENERAL, SFX_BOO, -1, -1);
        } else {
            set_icon_value(patient, ICON_CURED);
            change_health(patient, 1000);
            patient->flags |= PF_CURED;
            play_sound(SNDSRC_GENERAL, SFX_CHEER, -1, -1);
        }
        break;
    case 4:
        pay_for_treatment(patient, 2);
        diagnose_patient(staff, patient);
        pay = shall_peep_pay(patient);
        if (pay) {
            pay_for_treatment(patient, 3);
        }
        break;
    case 5:
        pay_for_treatment(patient, 2);
        if (patient->next_treatment) {
            patient->treatment = patient->next_treatment;
            patient->next_treatment = 0;
        } else {
            patient->treatment = ILL_GENERAL_PRACTICE;
        }
        pay = shall_peep_pay(patient);
        if (pay) {
            pay_for_treatment(patient, 3);
        }
        break;
    case 3:
    default:
        break;
    }
    if (pay == 0) {
        patient->treatment = 0;
        pay_for_treatment(patient, 2);
    }
    if (patient->flags & PF_NEEDS_DIAGNOSIS_ROOM) {
        pct = patient->diagnosis * 100 / gbv.diagnosis_complete;
        if (allhosps[patient->owner].guess_cure_threshold <= pct &&
            allhosps[patient->owner].rooms[patient->illness].discovered == 1 && pct > 0) {
            best_guess_treatment(patient);
        } else if (allhosps[patient->owner].send_home_threshold >= pct) {
            patient->flags32 = 0;
            patient->treatment = 0;
            allhosps[patient->owner].rooms[patient->illness].turned_away++;
            patient->flags2 |= (PF_NO_DIAGNOSIS >> 16);
        }
    }
    next_treatment_now_setup(patient, 1);
}

// Chooses patient's next state: while the diagnosis awaits a decision
// (PF_NEEDS_DIAGNOSIS_ROOM) or the treatment's room is missing (PF_AWAITING_DECISION),
// calls the player (if no more than 5 such calls are pending, else gives up);
// otherwise goes for treatment. Walks them out of the room first if in_room
// is set.
// FUNCTION: 0x0003c8d8
void next_treatment_now_setup(Thing *patient, u8 in_room)
{
    if (patient->flags & PF_NEEDS_DIAGNOSIS_ROOM) {
        if (check_playercall(8, 0, patient->owner) <= 5) {
            patient->flags |= PF_AWAITING_DECISION;
            set_icon_value(patient, ICON_AWAITING_DECISION);
            patient->next_state = PS_WAITS_FOR_DECISION;
            patient->timer = othervar.cur_turn;
            patient->playercall = make_a_playercall(8, patient - thing, 0, patient->owner);
            if (patient->owner == player) {
                advisor.event_flags |= 4;
            }
        } else {
            patient->treatment = 0;
            patient->next_state = PS_GO_FOR_TREATMENT;
            allhosps[patient->owner].rooms[patient->illness].turned_away++;
            patient->flags2 |= (PF_NO_DIAGNOSIS >> 16);
        }
    } else if (patient->flags & PF_AWAITING_DECISION) {
        if (check_playercall(4, (u16)patient->illness, patient->owner) <= 5) {
            patient->next_state = PS_WAITS_FOR_DECISION;
            patient->timer = othervar.cur_turn;
            patient->playercall = make_a_playercall(4, patient - thing, 0, patient->owner);
            if (patient->owner == player) {
                advisor.event_flags |= 4;
            }
        } else {
            patient->treatment = 0;
            patient->next_state = PS_GO_FOR_TREATMENT;
            allhosps[patient->owner].rooms[patient->illness].turned_away++;
            patient->flags2 |= (PF_NO_TREATMENT >> 16);
        }
    } else {
        patient->next_state = PS_GO_FOR_TREATMENT;
    }
    if (patient->room != 0) {
        if (in_room) {
            if (set_complex(patient, 0x5a, patient->room->door_inside_tile)) {
                process_person_complex(patient);
            }
        } else {
            if (set_complex(patient, 0xf0, 0)) {
                process_person_complex(patient);
            }
        }
    } else {
        patient->state = patient->next_state;
        patient->next_state = 0;
    }
}

// Adds amount to patient's health (0-1000), setting the health icon; while
// they are in a room the change is held back (health_delta) until they
// leave. The patient dies at 0.
// FUNCTION: 0x0003cb5c
void change_health(Thing *patient, s16 amount)
{
    s32 health;

    health = patient->health;
    if (health <= 20) {
        set_icon_value(patient, ICON_HEALTH_5);
    } else if (health <= 40) {
        set_icon_value(patient, ICON_HEALTH_4);
    } else if (health <= 60) {
        set_icon_value(patient, ICON_HEALTH_3);
    } else if (health <= 80) {
        set_icon_value(patient, ICON_HEALTH_2);
    } else if (health <= 100) {
        reset_icon_value(patient, patient->icon_id);
        set_icon_value(patient, ICON_HEALTH_1);
    } else if (patient->icon_id >= ICON_HEALTH_1 && patient->icon_id <= ICON_HEALTH_5) {
        reset_icon_value(patient, patient->icon_id);
    }
    if (health > 0) {
        if (patient->room != 0) {
            patient->health_delta += amount;
        } else {
            amount += patient->health_delta;
            patient->health_delta = 0;
            health += amount;
            if (health < 0) {
                health = 0;
            } else if (health > 1000) {
                health = 1000;
            }
            patient->health = health;
            add_health_to_history(patient);
            if (patient->health <= 0) {
                person_dies(patient);
            }
        }
    }
}

// Patient dies: walks to a free spot nearby to fall over (PS_DEAD), and is
// counted as a death, with reputation lost and their pending calls (types 8
// and 4) removed.
// FUNCTION: 0x0003ccc8
void person_dies(Thing *patient)
{
    u16 i;
    u16 pos;
    u16 tile;
    Map *cell;

    reset_icon_value(patient, ICON_HEALTH_5);
    leave_queue(patient);
    tile = (u16)patient->x.tile + ((u16)patient->y.tile << 7);
    for (i = 1; i <= dying_place[0]; i++) {
        pos = tile + dying_place[i];
        if (map[pos].first == 0 && can_i_get_to(patient, pos) > 0) {
            cell = &map[pos + 1];
            if (cell->first == 0 && cell->wall_w == 0) {
                break;
            }
            cell = &map[pos + 0x80];
            if (cell->first == 0 && cell->wall_n == 0) {
                break;
            }
        }
    }
    patient->next_state = PS_DEAD;
    patient->substate = 0;
    patient->complex_pc = 0;
    patient->complex_queued = 0;
    patient->complex_queued2 = 0;
    if (set_complex(patient, 0x6e, pos)) {
        process_person_complex(patient);
    } else {
        patient->state = PS_DEAD;
    }
    allhosps[patient->owner].rooms[patient->illness].deaths++;
    allhosps[patient->owner].deaths++;
    if (patient->owner == player) {
        advisor.event_flags |= 0x80;
    }
    reputation_affected(patient->illness, REP_DIED, 0, patient->owner, 0);
    record_person_as_patient(patient);
    if (patient->owner == player) {
        mycontrol->deaths_this_month++;
    }
    patient->flags2 |= (PF_DYING >> 16);
    if (patient->playercall) {
        get_playercall(8, patient->playercall, patient->owner);
        get_playercall(4, patient->playercall, patient->owner);
        patient->playercall = 0;
    }
}

// Patient state PS_DEAD: plays the death animation, then either floats up
// (substate 0x2f) or, for some patients at random, summons the grim reaper
// (0x30), floating up instead if the reaper cannot come.
// FUNCTION: 0x0003cf5c
void person_dead(Thing *patient)
{
    Map *right;
    Map *down;
    u16 tile;

    if (patient->substate < 0x1e) {
        tile = (s16)((u16)patient->x.tile + (patient->y.tile << 7));
        right = &map[tile + 1];
        down = &map[tile + 0x80];
        if ((old_newrand() & 1) == 0) {
            if (right->first == 0 && right->wall_w == 0) {
                patient->facing = 0;
            } else {
                patient->facing = 1;
            }
        } else {
            if (down->first == 0 && down->wall_n == 0) {
                patient->facing = 1;
            } else {
                patient->facing = 0;
            }
        }
        patient->next_state = PS_DEAD;
        patient->depth = -8;
        if (anim_starts[patient->subtype].reaper_test_anim == 0x4c1) {
            patient->substate = (old_newrand() & 1) + 0x2f;
        } else {
            patient->substate = 0x2f;
        }
        if (patient->subtype == PK_ALIEN) {
            patient->subtype = patient->orig_subtype;
            if (set_complex(patient, 0x280, 0)) {
                process_person_complex(patient);
            }
        } else {
            if (set_complex(patient, 0x140, 0)) {
                process_person_complex(patient);
            }
        }
    } else if (patient->substate == 0x2f) {
        if (patient->layers[0] > 5) {
            patient->layers[0] = 1;
        }
        patient->next_state = PS_FLYS;
        patient->countdown = 0;
        patient->speed = 8;
        if (set_complex(patient, 0x1cc, 0)) {
            process_person_complex(patient);
        }
    } else if (patient->substate == 0x30) {
        if (summon_the_reaper(patient)) {
            effect_illness(patient, patient->illness, 1);
            patient->substate = 0x63;
            patient->next_state = PS_DEAD;
        } else {
            patient->substate = 0x2f;
        }
    }
}

// Records patient's health (in tens) in health_history when it has changed
// by 50 or more since the last entry, dropping the oldest (after entry 0)
// when full.
// FUNCTION: 0x0003d188
void add_health_to_history(Thing *patient)
{
    s32 diff;
    u32 i;
    s16 last;

    last = 0;
    for (i = 0; i < 10; i++) {
        if (patient->health_history[i] == 0) {
            break;
        }
        last = patient->health_history[i] * 10;
    }
    if (last - patient->health < 0) {
        diff = -(last - patient->health);
    } else {
        diff = last - patient->health;
    }
    if (diff >= 50) {
        if (i >= 10) {
            for (i = 1; i < 9; i++) {
                patient->health_history[i] = patient->health_history[i + 1];
            }
        }
        patient->health_history[i] = patient->health / 10;
    }
}

// Applies a diagnosis to patient: if complete, the illness is known (new
// ones announced) and treatment set, or awaits a room; if hopeless, the
// patient may be sent to autopsy or dies; otherwise a treatment to try is
// guessed (find_possible_diagnosis). Ward illnesses go via the ward.
// FUNCTION: 0x0003d28c
void diagnose_patient(Thing *staff, Thing *patient)
{
    u16 illness;
    s32 ok;
    s16 diag;
    u16 coin;
    s32 guess;

    guess = 1;
    diag = patient->diagnosis;
    if (diag >= gbv.diagnosis_complete) {
        guess = 0;
        illness = (u16)patient->illness;
        if (patient->known_illness != illness) {
            staff->happiness += 5;
            announce_new_illness(illness, (u16)patient->owner);
            reputation_affected(illness, REP_ILLNESS_FOUND, 0, patient->owner, 0);
            record_person_as_patient(patient);
            patient->known_illness = illness;
            allhosps[patient->owner].diagnosed++;
        }
        if (patient->contagion) {
            alert_epidemic(patient->illness, patient->owner);
        }
        patient->flags2 |= (PF_DIAGNOSED >> 16);
        ok = 1;
        if (diag < allhosps[patient->owner].diag_threshold) {
            find_possible_diagnosis(patient);
            if (patient->treatment == 0) {
                patient->flags |= PF_NEEDS_DIAGNOSIS_ROOM;
                reputation_affected(patient->illness, REP_NO_TREATMENT_FOUND, 0, patient->owner, 0);
            } else {
                ok = 0;
            }
        }
        if (ok) {
            set_icon_value(staff, ICON_DIAGNOSED);
            if (allhosps[patient->owner].rooms[illness].cure_available == 0) {
                patient->flags |= (PF_AWAITING_DECISION | PF_WAITING_FOR_ROOM);
                patient->treatment = patient->illness;
                set_icon_value(patient, ICON_AWAITING_DECISION);
            } else {
                reputation_affected(patient->illness, REP_DIAGNOSED, 0, patient->owner, 0);
                patient->treatment = patient->illness;
            }
        }
    } else if (diag <= gbv.diagnosis_hopeless) {
        coin = old_newrand() & 1;
        if (coin == 0) {
            if (allhosps[patient->owner].rooms[ILL_AUTOPSY].cure_available == 1) {
                patient->treatment = ILL_AUTOPSY;
            } else {
                coin = 1;
            }
        }
        if (coin == 1) {
            change_health(patient, -(patient->health - 3));
        }
        reputation_affected(patient->illness, REP_DIAGNOSIS_FAILED, 0, patient->owner, 0);
        allhosps[patient->owner].mishaps++;
        guess = 0;
    }
    if (guess) {
        find_possible_diagnosis(patient);
        if (patient->treatment == 0) {
            set_icon_value(staff, ICON_NO_DIAGNOSIS);
            patient->flags |= PF_NEEDS_DIAGNOSIS_ROOM;
            reputation_affected(patient->illness, REP_NO_TREATMENT_FOUND, 0, patient->owner, 0);
        } else {
            set_icon_value(staff, ICON_GUESSED);
        }
    }
    patient->next_treatment = 0;
    if (expertise[patient->treatment].needs_ward & 1) {
        patient->next_treatment = patient->treatment;
        patient->treatment = ILL_DIAG_WARD;
    }
}

// Returns staff's diagnostic ability: their ability with random, grade,
// tiredness, crazy-doctor and room adjustments.
// FUNCTION: 0x0003d658
s32 calc_ability(Thing *staff)
{
    s32 total;
    s32 random_adj;
    s32 skill_adj;
    s32 tired_adj;
    s32 quack_adj;
    s32 room_adj;

    skill_adj = 0;
    tired_adj = 0;
    quack_adj = 0;
    room_adj = 0;
    random_adj = 250 - old_normal_rand(0, 500);
    if (staff->skills & SKILL_CONSULTANT) {
        skill_adj = 200;
    } else if (staff->skills & SKILL_JUNIOR) {
        skill_adj = -100;
    }
    if (staff->tiredness < gbv.not_tired) {
        tired_adj = 100;
    } else if (staff->tiredness >= gbv.tired) {
        tired_adj = -100;
    } else if (staff->tiredness >= gbv.very_tired) {
        tired_adj = -200;
    }
    if (staff->subtype == STAFF_DOCTOR && staff->crazy) {
        quack_adj = -400;
    }
    room_adj = calc_room_value(staff->room);
    total = staff->ability;
    total += random_adj;
    total += skill_adj;
    total += tired_adj;
    total += quack_adj;
    total += room_adj;
    {
        char buf[200];

        sprintf(buf,
                "Diag Ability %d      Staff Ability %3d    Random %3d    Skill %3d    Tired %3d   QuackDoc %3d    "
                "RoomAdd %3d",
                total, staff->ability, random_adj, skill_adj, tired_adj, quack_adj, room_adj);
    }
    return total;
}

// Adds patient's current treatment to their treatment history (the last
// ten).
// FUNCTION: 0x0003d7c0
void add_treatment_to_history(Thing *patient)
{
    u16 i;

    if (patient->treatment != 0) {
        for (i = 0; i < 10; i++) {
            if (patient->treatments[i] == 0) {
                patient->treatments[i] = patient->treatment;
                break;
            }
        }
        if (i >= 10) {
            for (i = 0; i < 9; i++) {
                patient->treatments[i] = patient->treatments[i + 1];
            }
            patient->treatments[9] = patient->treatment;
        }
    }
}

// Returns 1 if patient's treatment history contains treatment.
// FUNCTION: 0x0003d89c
u8 treatment_already_done(Thing *patient, u16 treatment)
{
    u16 i;
    u8 found;

    found = 0;
    for (i = 0; i < 10; i++) {
        if (patient->treatments[i] == 0) {
            break;
        }
        if (patient->treatments[i] == treatment) {
            found = 1;
            break;
        }
    }
    return found;
}

// Patient state PS_GO_FOR_TREATMENT: sends them to the room for their
// treatment (the ward, research for autopsy), home if they will not pay or
// it is unavailable, or to wait for a decision if the room is not built.
// FUNCTION: 0x0003d91c
void person_go_for_treatment(Thing *patient)
{
    RoomInfo *room;
    Expertise *exp;
    u16 action;
    u16 pos;
    u16 treatment;

    treatment = (u16)patient->treatment;
    if (shall_peep_pay(patient) == 0) {
        treatment = 0;
    }
    exp = &expertise[treatment];
    if (patient->flags & PF_EMERGENCY) {
        if (listhead[patient->owner].rooms[exp->category] == 0) {
            treatment = 0;
        }
    } else if (allhosps[patient->owner].rooms[treatment].cure_available == 0) {
        reputation_affected(treatment, REP_GO_FOR_TREATMENT, 0, patient->owner, 0);
        treatment = 0;
    }
    if (treatment != 0 && listhead[patient->owner].rooms[exp->category] == 0) {
        patient->flags |= PF_WAITING_FOR_ROOM;
        patient->timer = othervar.cur_turn;
        set_complex(patient, 0xf0, 0);
        patient->next_state = PS_WAITS_FOR_DECISION;
        process_person_complex(patient);
        return;
    }
    switch (treatment) {
    case 0: {
        u16 anim;

        action = 20;
        pos = get_room_info(ROOM_ENTRANCE, 0x65, patient->owner)->door_tile;
        anim = check_frame(anim_starts[patient->subtype].walk, patient) + (u16)patient->facing;
        patient->sprite = start[anim].frame;
        patient->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
        set_complex(patient, action, pos);
        process_person_complex(patient);
        break;
    }
    case ILL_DIAG_WARD:
        send_person_to_bed(patient);
        break;
    case ILL_AUTOPSY:
        send_person_for_research(patient);
        break;
    default: {
        if (allhosps[patient->owner].rooms[treatment].cure_available == 1) {
            room = get_expertise_dest(treatment, patient);
        } else {
            room = &room_info[listhead[patient->owner].rooms[exp->category]];
        }
        room->queue_incoming++;
        pos = room->door_tile;
        page_staff_i_need(patient, room);
        patient->dest_room = room - room_info;
        {
            u16 anim;

            anim = check_frame(anim_starts[patient->subtype].walk, patient) + (u16)patient->facing;
            patient->sprite = start[anim].frame;
            patient->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
        }
        patient->state = PS_GOTO_QUEUE;
        set_destination_coords(patient, pos);
        break;
    }
    }
}

// Returns 1 if the hospital has a drug for patient's illness, else 0.
// FUNCTION: 0x0003dcfc
s16 pre_administer_drugs(Thing *patient)
{
    HospitalDrug *drug;
    AllHosps *hosp;
    u8 illness;

    hosp = &allhosps[patient->owner];
    illness = patient->illness;
    drug = &hosp->rooms[illness].drug;
    if (drug->effectiveness == 0) {
        return 0;
    }
    return 1;
}

// staff gives patient their drug: cured with a chance equal to its
// effectiveness (never after a wrong guess); otherwise health is lost (all
// of it for emergencies and some illnesses) and they go back to the GP. The
// drug's cost is charged.
// FUNCTION: 0x0003dd7c
void administer_drugs(Thing *staff, Thing *patient)
{
    HospitalDrug *drug;
    AllHosps *hosp;
    s32 effect;
    u8 illness;

    patient->impatience = 0;
    patient->waiting_time = 0;
    add_treatment_to_history(patient);
    hosp = &allhosps[patient->owner];
    illness = patient->illness;
    drug = &hosp->rooms[illness].drug;
    effect = drug->effectiveness;
    if (patient->flags & PF_WRONG_GUESS) {
        effect = 0;
    }
    if (old_normal_rand(0, 99) < effect) {
        pay_for_treatment(patient, 2);
        patient->treatment = 0;
        effect_illness(patient, patient->illness, 1);
        staff->happiness += 5;
        patient->flags &= ~PF_AWAITING_DECISION;
        reset_icon_value(patient, ICON_AWAITING_DECISION);
        patient->flags |= PF_CURED;
        change_health(patient, 1000);
        play_sound(SNDSRC_GENERAL, SFX_CHEER, -1, -1);
        if (patient->contagion) {
            clean_contagious_peep(patient);
        }
    } else {
        pay_for_treatment(patient, 2);
        if ((patient->flags & PF_EMERGENCY) || expertise[patient->illness].not_contagious) {
            change_health(patient, -1000);
            play_sound(SNDSRC_GENERAL, SFX_BOO, -1, -1);
        } else {
            if (patient->health <= 1000 - effect * 10) {
                play_sound(SNDSRC_GENERAL, SFX_BOO, -1, -1);
            }
            change_health(patient, -(1000 - effect * 10));
        }
        patient->treatment = ILL_GENERAL_PRACTICE;
        pay_for_treatment(patient, 3);
    }
    patient->next_state = PS_GO_FOR_TREATMENT;
    if (set_complex(patient, 0x5a, staff->room->door_inside_tile)) {
        process_person_complex(patient);
    }
    change_balance(TX_DRUG_COST, -drug->cost, 0, (u16)staff->owner);
}

// Marks illness as discovered for player `owner` (rooms[].discovered),
// sending the new illness fax the first time.
// FUNCTION: 0x0003dfcc
void announce_new_illness(const u16 illness, const u16 owner)
{
    if (allhosps[owner].rooms[illness].discovered == 0) {
        if (owner == player) {
            advisor.event_flags |= 4;
        }
        make_a_playercall(7, illness, 0xf, owner);
    }
    allhosps[owner].rooms[illness].discovered = 1;
}

// Draws the "no diagnosis" call for patient: the diagnosis so far and,
// when the treatment is available, the best guess. Disables the guess
// button otherwise. Returns 0.
// FUNCTION: 0x0003e06c
u16 call_no_diagnosis(Thing *patient, char **strs)
{
    char buf2[0x7d0];
    char buf1[0xc8];
    u16 text_x;
    u16 text_y;
    s16 pct;
    u16 height;
    char new_line[] = "\n";

    if (patient <= thing) {
        return 0;
    }
    height = lbFontPtr ? lbFontPtr[34].height : 0;
    pct = patient->diagnosis * 100 / gbv.diagnosis_complete;
    if (pct < 0) {
        pct = 0;
    } else if (pct > 99) {
        pct = 99;
    }
    sprintf(buf1, strs[2], pct, illness_text[patient->illness]);
    if (allhosps[patient->owner].rooms[patient->illness].cure_available != 1 ||
        allhosps[patient->owner].rooms[patient->illness].discovered == 0) {
        available_buttons &= ~2;
    }
    text_x = 0;
    text_y = 0;
    sprintf(buf2, strs[0]);
    strcat(buf2, new_line);
    strcat(buf2, strs[1]);
    strcat(buf2, new_line);
    if (allhosps[patient->owner].rooms[patient->illness].cure_available == 1 &&
        allhosps[patient->owner].rooms[patient->illness].discovered == 1) {
        strcat(buf2, buf1);
    }
    LbTextDraw(text_x, text_y, buf2);
    return 0;
}

// "No diagnosis" call buttons 0-2: sends that choice (packet_choice_result
// CH_DIAG_*).
// FUNCTION: 0x0003e2c4
void process_no_diagnosis_buttons(u8 button)
{
    switch (button) {
    case 0:
        packet_choice_result(CH_DIAG_SEND_HOME);
        break;
    case 1:
        packet_choice_result(CH_DIAG_GUESS);
        break;
    case 2:
        packet_choice_result(CH_DIAG_WAIT);
        break;
    }
}

// Treats patient's real illness as the diagnosis (PF_GUESSED): the guess is
// wrong (PF_WRONG_GUESS, fatal when treated) with a chance falling as the
// diagnosis progresses.
// FUNCTION: 0x0003e330
void best_guess_treatment(Thing *patient)
{
    u8 pct;

    patient->flags &= ~PF_NEEDS_DIAGNOSIS_ROOM;
    patient->flags |= PF_GUESSED;
    patient->known_illness = patient->treatment = patient->illness;
    patient->next_treatment = 0;
    if (expertise[patient->treatment].needs_ward & 1) {
        patient->next_treatment = patient->treatment;
        patient->treatment = ILL_DIAG_WARD;
    }
    if (allhosps[patient->owner].rooms[patient->illness].cure_available == 0) {
        patient->flags |= PF_AWAITING_DECISION;
        set_icon_value(patient, ICON_AWAITING_DECISION);
    }
    pct = patient->diagnosis * 100 / gbv.diagnosis_complete;
    if (pct < 0) {
        pct = 0;
    } else if (pct > 99) {
        pct = 99;
    }
    if (pct < old_newrand() % 100) {
        patient->flags |= PF_WRONG_GUESS;
    }
}

// Picks a diagnosis room for patient's illness that the hospital has and
// they have not tried yet, as their treatment. Returns it, or 0 (also set
// as the treatment) if none is left.
// FUNCTION: 0x0003e468
u16 find_possible_diagnosis(Thing *patient)
{
    u16 list[9];
    u16 treatment;
    u16 pick;
    PossibleDiag *diag;
    u16 count;
    u16 j;
    u16 i;
    u16 result;

    result = 0;
    diag = &possible_diagnoses[expertise[patient->illness].diag_set];
    patient->treatment = 0;
    if (diag->count) {
        for (i = 0, count = 0; i < diag->count; i++) {
            treatment = (u16)diag->treatments[i];
            if (allhosps[patient->owner].rooms[treatment].cure_available == 1) {
                list[count++] = treatment;
            }
        }
        if (count > 0) {
            for (i = 0; i < 9; i++) {
                pick = old_newrand() % count;
                treatment = list[pick];
                if (allhosps[patient->owner].rooms[treatment].cure_available != 0) {
                    if (treatment_already_done(patient, treatment) == 0) {
                        result = treatment;
                        break;
                    }
                    for (j = pick; j < count - 1; j++) {
                        list[j] = list[j + 1];
                    }
                    --count;
                    if (count == 0) {
                        break;
                    }
                }
            }
            patient->treatment = result;
        }
    }
    return result;
}

// Counts patient once in the hospital's patient statistics (PF_RECORDED).
// FUNCTION: 0x0003e668
void record_person_as_patient(Thing *patient)
{
    if (!(patient->flags2 & (PF_RECORDED >> 16))) {
        reputation_affected(patient->illness, REP_CASE, 0, patient->owner, 0);
        patient->flags2 |= (PF_RECORDED >> 16);
    }
}

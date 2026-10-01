#include "hosp.h"

extern s16 dying_place[];
extern PossibleDiag possible_diagnoses[];
extern s16 diag_register[][8];

extern "C" {

extern char **D_001602a4;

void fn_000c93bf(char *dst, char *src);

}

// FUNCTION: 0x0003c360
void done_treatment(Thing *staff, Thing *patient)
{
    if (patient->room != 0 && patient->room->type == 6) {
        return;
    }
    affect_diagnosis(staff, patient);
    add_treatment_to_history(patient);
    setup_next_treatment(staff, patient);
}

// FUNCTION: 0x0003c3b8
void affect_diagnosis(Thing *staff, Thing *patient)
{
    s32 total;
    s16 bonus;

    bonus = 0;
    if (patient->treatment >= 0x24 || patient->treatment == 1) {
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

// FUNCTION: 0x0003c4ec
void setup_next_treatment(Thing *staff, Thing *patient)
{
    u32 pct;
    s32 pay;

    pay = 1;
    patient->impatience = 0;
    patient->waiting_time = 0;
    reputation_affected(patient->treatment, 10, 0, patient->owner, 0);
    switch (expertise[patient->treatment].unknown8) {
    case 0:
        pay_for_treatment(patient, 2);
        patient->treatment = 0;
        break;
    case 1:
        pay_for_treatment(patient, 2);
        patient->treatment = 1;
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
        patient->flags &= ~0x10;
        reset_icon_value(patient, 0x1b);
        if (patient->flags & 0x800) {
            set_icon_value(patient, 0x16);
            change_health(patient, -1000);
            play_sound(10, 0xae, -1, -1);
        } else {
            set_icon_value(patient, 0x17);
            change_health(patient, 1000);
            patient->flags |= 0x2000;
            play_sound(10, 0xaf, -1, -1);
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
            patient->treatment = 1;
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
    if (patient->flags & 0x400) {
        pct = patient->diagnosis * 100 / gbv.ffc;
        if (allhosps[patient->owner].f219 <= pct && allhosps[patient->owner].rooms[patient->illness].unknown1D == 1 && pct > 0) {
            best_guess_treatment(patient);
        } else if (allhosps[patient->owner].f21b >= pct) {
            patient->flags32 = 0;
            patient->treatment = 0;
            allhosps[patient->owner].rooms[patient->illness].unknown24++;
            patient->flags2 |= 8;
        }
    }
    next_treatment_now_setup(patient, 1);
}

// FUNCTION: 0x0003c8d8
void next_treatment_now_setup(Thing *patient, u8 in_room)
{
    if (patient->flags & 0x400) {
        if (check_playercall(8, 0, patient->owner) <= 5) {
            patient->flags |= 0x10;
            set_icon_value(patient, 0x1b);
            patient->next_state = 0x44;
            patient->timer = othervar.f82;
            patient->playercall = make_a_playercall(8, patient - thing, 0, patient->owner);
            if (patient->owner == player) {
                advisor.f128 |= 4;
            }
        } else {
            patient->treatment = 0;
            patient->next_state = 0x26;
            allhosps[patient->owner].rooms[patient->illness].unknown24++;
            patient->flags2 |= 8;
        }
    } else if (patient->flags & 0x10) {
        if (check_playercall(4, (u16)patient->illness, patient->owner) <= 5) {
            patient->next_state = 0x44;
            patient->timer = othervar.f82;
            patient->playercall = make_a_playercall(4, patient - thing, 0, patient->owner);
            if (patient->owner == player) {
                advisor.f128 |= 4;
            }
        } else {
            patient->treatment = 0;
            patient->next_state = 0x26;
            allhosps[patient->owner].rooms[patient->illness].unknown24++;
            patient->flags2 |= 0x10;
        }
    } else {
        patient->next_state = 0x26;
    }
    if (patient->room != 0) {
        if (in_room) {
            if (set_complex(patient, 0x5a, patient->room->unknown6)) {
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

// FUNCTION: 0x0003cb5c
void change_health(Thing *patient, s16 amount)
{
    s32 health;

    health = patient->health;
    if (health <= 20) {
        set_icon_value(patient, 5);
    } else if (health <= 40) {
        set_icon_value(patient, 4);
    } else if (health <= 60) {
        set_icon_value(patient, 3);
    } else if (health <= 80) {
        set_icon_value(patient, 2);
    } else if (health <= 100) {
        reset_icon_value(patient, patient->icon_id);
        set_icon_value(patient, 1);
    } else if (patient->icon_id >= 1 && patient->icon_id <= 5) {
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

// FUNCTION: 0x0003ccc8
void person_dies(Thing *patient)
{
    u16 i;
    u16 pos;
    u16 tile;
    Map *cell;

    reset_icon_value(patient, 5);
    leave_queue(patient);
    tile = (u16)patient->x.tile + ((u16)patient->y.tile << 7);
    for (i = 1; i <= dying_place[0]; i++) {
        pos = tile + dying_place[i];
        if (map[pos].first == 0 && can_i_get_to(patient, pos) > 0) {
            cell = &map[pos + 1];
            if (cell->first == 0 && cell->unknown4 == 0) {
                break;
            }
            cell = &map[pos + 0x80];
            if (cell->first == 0 && cell->unknown3 == 0) {
                break;
            }
        }
    }
    patient->next_state = 0x32;
    patient->substate = 0;
    patient->complex_pc = 0;
    patient->complex_queued = 0;
    patient->complex_queued2 = 0;
    if (set_complex(patient, 0x6e, pos)) {
        process_person_complex(patient);
    } else {
        patient->state = 0x32;
    }
    allhosps[patient->owner].rooms[patient->illness].unknown20++;
    allhosps[patient->owner].f1eb++;
    if (patient->owner == player) {
        advisor.f128 |= 0x80;
    }
    reputation_affected(patient->illness, 3, 0, patient->owner, 0);
    record_person_as_patient(patient);
    if (patient->owner == player) {
        mycontrol->unknown10++;
    }
    patient->flags2 |= 4;
    if (patient->playercall) {
        get_playercall(8, patient->playercall, patient->owner);
        get_playercall(4, patient->playercall, patient->owner);
        patient->playercall = 0;
    }
}

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
            if (right->first == 0 && right->unknown4 == 0) {
                patient->facing = 0;
            } else {
                patient->facing = 1;
            }
        } else {
            if (down->first == 0 && down->unknown3 == 0) {
                patient->facing = 1;
            } else {
                patient->facing = 0;
            }
        }
        patient->next_state = 0x32;
        patient->depth = -8;
        if (anim_starts[patient->subtype].unknownA2 == 0x4c1) {
            patient->substate = (old_newrand() & 1) + 0x2f;
        } else {
            patient->substate = 0x2f;
        }
        if (patient->subtype == 0xe) {
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
        patient->next_state = 0x34;
        patient->countdown = 0;
        patient->speed = 8;
        if (set_complex(patient, 0x1cc, 0)) {
            process_person_complex(patient);
        }
    } else if (patient->substate == 0x30) {
        if (summon_the_reaper(patient)) {
            effect_illness(patient, patient->illness, 1);
            patient->substate = 0x63;
            patient->next_state = 0x32;
        } else {
            patient->substate = 0x2f;
        }
    }
}

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
    if (diag >= gbv.ffc) {
        guess = 0;
        illness = (u16)patient->illness;
        if (patient->known_illness != illness) {
            staff->happiness += 5;
            announce_new_illness(illness, (u16)patient->owner);
            reputation_affected(illness, 0x15, 0, patient->owner, 0);
            record_person_as_patient(patient);
            patient->known_illness = illness;
            allhosps[patient->owner].f1ef++;
        }
        if (patient->contagion) {
            alert_epidemic(patient->illness, patient->owner);
        }
        patient->flags2 |= 2;
        ok = 1;
        if (diag < allhosps[patient->owner].f217) {
            find_possible_diagnosis(patient);
            if (patient->treatment == 0) {
                patient->flags |= 0x400;
                reputation_affected(patient->illness, 7, 0, patient->owner, 0);
            } else {
                ok = 0;
            }
        }
        if (ok) {
            set_icon_value(staff, 0x15);
            if (allhosps[patient->owner].rooms[illness].unknown1C == 0) {
                patient->flags |= 0x90;
                patient->treatment = patient->illness;
                set_icon_value(patient, 0x1b);
            } else {
                reputation_affected(patient->illness, 4, 0, patient->owner, 0);
                patient->treatment = patient->illness;
            }
        }
    } else if (diag <= gbv.ffe) {
        coin = old_newrand() & 1;
        if (coin == 0) {
            if (allhosps[patient->owner].rooms[0x2c].unknown1C == 1) {
                patient->treatment = 0x2c;
            } else {
                coin = 1;
            }
        }
        if (coin == 1) {
            change_health(patient, -(patient->health - 3));
        }
        reputation_affected(patient->illness, 5, 0, patient->owner, 0);
        allhosps[patient->owner].fc98++;
        guess = 0;
    }
    if (guess) {
        find_possible_diagnosis(patient);
        if (patient->treatment == 0) {
            set_icon_value(staff, 0x18);
            patient->flags |= 0x400;
            reputation_affected(patient->illness, 7, 0, patient->owner, 0);
        } else {
            set_icon_value(staff, 0x14);
        }
    }
    patient->next_treatment = 0;
    if (expertise[patient->treatment].unknown5[2] & 1) {
        patient->next_treatment = patient->treatment;
        patient->treatment = 0x2a;
    }
}

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
    if (staff->skills & 0x80) {
        skill_adj = 200;
    } else if (staff->skills & 8) {
        skill_adj = -100;
    }
    if (staff->tiredness < gbv.fe6) {
        tired_adj = 100;
    } else if (staff->tiredness >= gbv.fe8) {
        tired_adj = -100;
    } else if (staff->tiredness >= gbv.fea) {
        tired_adj = -200;
    }
    if (staff->subtype == 1 && staff->crazy) {
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

        sprintf(buf, "Diag Ability %d      Staff Ability %3d    Random %3d    Skill %3d    Tired %3d   QuackDoc %3d    RoomAdd %3d", total, staff->ability, random_adj, skill_adj, tired_adj, quack_adj, room_adj);
    }
    return total;
}

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
    if (patient->flags & 0x40) {
        if (listhead[patient->owner].rooms[exp->category] == 0) {
            treatment = 0;
        }
    } else if (allhosps[patient->owner].rooms[treatment].unknown1C == 0) {
        reputation_affected(treatment, 2, 0, patient->owner, 0);
        treatment = 0;
    }
    if (treatment != 0 && listhead[patient->owner].rooms[exp->category] == 0) {
        patient->flags |= 0x80;
        patient->timer = othervar.f82;
        set_complex(patient, 0xf0, 0);
        patient->next_state = 0x44;
        process_person_complex(patient);
        return;
    }
    switch (treatment) {
    case 0: {
        u16 frame;

        action = 20;
        pos = get_room_info(1, 0x65, patient->owner)->unknown4;
        frame = check_frame(anim_starts[patient->subtype].idle, patient) + (u16)patient->facing;
        patient->sprite = start[frame].frame;
        patient->flip = start[frame].flags > 0 ? (u8)1 : (u8)0;
        set_complex(patient, action, pos);
        process_person_complex(patient);
        break;
    }
    case 0x2a:
        send_person_to_bed(patient);
        break;
    case 0x2c:
        send_person_for_research(patient);
        break;
    default: {
        if (allhosps[patient->owner].rooms[treatment].unknown1C == 1) {
            room = get_expertise_dest(treatment, patient);
        } else {
            room = &room_info[listhead[patient->owner].rooms[exp->category]];
        }
        room->unknown32++;
        pos = room->unknown4;
        page_staff_i_need(patient, room);
        patient->dest_room = room - room_info;
        {
            u16 frame;

            frame = check_frame(anim_starts[patient->subtype].idle, patient) + (u16)patient->facing;
            patient->sprite = start[frame].frame;
            patient->flip = start[frame].flags > 0 ? (u8)1 : (u8)0;
        }
        patient->state = 0x24;
        fn_0004195c(patient, pos);
        break;
    }
    }
}

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
    if (patient->flags & 0x800) {
        effect = 0;
    }
    if (old_normal_rand(0, 99) < effect) {
        pay_for_treatment(patient, 2);
        patient->treatment = 0;
        effect_illness(patient, patient->illness, 1);
        staff->happiness += 5;
        patient->flags &= ~0x10;
        reset_icon_value(patient, 0x1b);
        patient->flags |= 0x2000;
        change_health(patient, 1000);
        play_sound(10, 0xaf, -1, -1);
        if (patient->contagion) {
            clean_contagious_peep(patient);
        }
    } else {
        pay_for_treatment(patient, 2);
        if ((patient->flags & 0x40) || expertise[patient->illness].unknown10[1]) {
            change_health(patient, -1000);
            play_sound(10, 0xae, -1, -1);
        } else {
            if (patient->health <= 1000 - effect * 10) {
                play_sound(10, 0xae, -1, -1);
            }
            change_health(patient, -(1000 - effect * 10));
        }
        patient->treatment = 1;
        pay_for_treatment(patient, 3);
    }
    patient->next_state = 0x26;
    if (set_complex(patient, 0x5a, staff->room->unknown6)) {
        process_person_complex(patient);
    }
    change_balance(0x16, -drug->cost, 0, (u16)staff->owner);
}

// FUNCTION: 0x0003dfcc
void announce_new_illness(const u16 illness, const u16 owner)
{
    if (allhosps[owner].rooms[illness].unknown1D == 0) {
        if (owner == player) {
            advisor.f128 |= 4;
        }
        make_a_playercall(7, illness, 0xf, owner);
    }
    allhosps[owner].rooms[illness].unknown1D = 1;
}

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
    pct = patient->diagnosis * 100 / gbv.ffc;
    if (pct < 0) {
        pct = 0;
    } else if (pct > 99) {
        pct = 99;
    }
    sprintf(buf1, strs[2], pct, D_001602a4[patient->illness]);
    if (allhosps[patient->owner].rooms[patient->illness].unknown1C != 1 || allhosps[patient->owner].rooms[patient->illness].unknown1D == 0) {
        available_buttons &= ~2;
    }
    text_x = 0;
    text_y = 0;
    sprintf(buf2, strs[0]);
    fn_000c93bf(buf2, new_line);
    fn_000c93bf(buf2, strs[1]);
    fn_000c93bf(buf2, new_line);
    if (allhosps[patient->owner].rooms[patient->illness].unknown1C == 1 && allhosps[patient->owner].rooms[patient->illness].unknown1D == 1) {
        fn_000c93bf(buf2, buf1);
    }
    LbTextDraw(text_x, text_y, buf2);
    return 0;
}

// FUNCTION: 0x0003e2c4
void process_no_diagnosis_buttons(u8 button)
{
    switch (button) {
    case 0:
        packet_choice_result(0x28);
        break;
    case 1:
        packet_choice_result(0x29);
        break;
    case 2:
        packet_choice_result(0x2a);
        break;
    }
}

// FUNCTION: 0x0003e330
void best_guess_treatment(Thing *patient)
{
    u8 pct;

    patient->flags &= ~0x400;
    patient->flags |= 0x1000;
    patient->known_illness = patient->treatment = patient->illness;
    patient->next_treatment = 0;
    if (expertise[patient->treatment].unknown5[2] & 1) {
        patient->next_treatment = patient->treatment;
        patient->treatment = 0x2a;
    }
    if (allhosps[patient->owner].rooms[patient->illness].unknown1C == 0) {
        patient->flags |= 0x10;
        set_icon_value(patient, 0x1b);
    }
    pct = patient->diagnosis * 100 / gbv.ffc;
    if (pct < 0) {
        pct = 0;
    } else if (pct > 99) {
        pct = 99;
    }
    if (pct < old_newrand() % 100) {
        patient->flags |= 0x800;
    }
}

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
    diag = &possible_diagnoses[expertise[patient->illness].unknown5[1]];
    patient->treatment = 0;
    if (diag->count) {
        for (i = 0, count = 0; i < diag->count; i++) {
            treatment = (u16)diag->ids[i];
            if (allhosps[patient->owner].rooms[treatment].unknown1C == 1) {
                list[count++] = treatment;
            }
        }
        if (count > 0) {
            for (i = 0; i < 9; i++) {
                pick = old_newrand() % count;
                treatment = list[pick];
                if (allhosps[patient->owner].rooms[treatment].unknown1C != 0) {
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

// FUNCTION: 0x0003e668
void record_person_as_patient(Thing *patient)
{
    if (!(patient->flags2 & 0x40)) {
        reputation_affected(patient->illness, 0x16, 0, patient->owner, 0);
        patient->flags2 |= 0x40;
    }
}

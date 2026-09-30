#include "hosp.h"

struct PossibleDiag {
    u8 count;
    u8 ids[9];
};

extern s16 dying_place[];
extern PossibleDiag possible_diagnoses[];
extern s16 diag_register[][8];

extern "C" {

extern char D_001303c8[];
extern char **D_001602a4;

void fn_000c93bf(char *dst, char *src);

}

// FUNCTION: 0x0003c360
void done_treatment(Thing *e, Thing *w)
{
    if (w->room != 0 && w->room->type == 6) {
        return;
    }
    affect_diagnosis(e, w);
    add_treatment_to_history(w);
    setup_next_treatment(e, w);
}

// FUNCTION: 0x0003c3b8
void affect_diagnosis(Thing *e, Thing *w)
{
    s32 total;
    s16 bonus;

    bonus = 0;
    if (w->unknown171 >= 0x24 || w->unknown171 == 1) {
        total = w->unknown146 - w->unknown144;
        total += calc_ability(e);
        if (w->unknown171 >= 0x24) {
            bonus = diag_register[w->style][w->unknown171 - 0x24];
        }
        total += bonus;
        if (w->unknown146 < total) {
            w->unknown146 = total;
        }
        if (bonus < 0) {
            change_health(w, -5);
        }
    }
    e->unknown126++;
    if (e->unknown126 < 30) {
        e->unknown126 = 30;
    } else if (e->unknown126 > 1000) {
        e->unknown126 = 1000;
    }
}

// FUNCTION: 0x0003c4ec
void setup_next_treatment(Thing *e, Thing *w)
{
    u32 pct;
    s32 pay;

    pay = 1;
    w->unknown124 = 0;
    w->unknown126 = 0;
    reputation_affected(w->unknown171, 10, 0, w->owner, 0);
    switch (expertise[w->unknown171].unknown8) {
    case 0:
        pay_for_treatment(w, 2);
        w->unknown171 = 0;
        break;
    case 1:
        pay_for_treatment(w, 2);
        w->unknown171 = 1;
        pay = shall_peep_pay(w);
        if (pay) {
            pay_for_treatment(w, 3);
        }
        break;
    case 2:
        pay_for_treatment(w, 2);
        w->unknown171 = 0;
        effect_illness(w, w->style, 1);
        if (e > thing) {
            e->unknown81 += 5;
        }
        if (w->unknown174) {
            clean_contagious_peep(w);
        }
        w->flags &= ~0x10;
        reset_icon_value(w, 0x1b);
        if (w->flags & 0x800) {
            set_icon_value(w, 0x16);
            change_health(w, -1000);
            play_sound(10, 0xae, -1, -1);
        } else {
            set_icon_value(w, 0x17);
            change_health(w, 1000);
            w->flags |= 0x2000;
            play_sound(10, 0xaf, -1, -1);
        }
        break;
    case 4:
        pay_for_treatment(w, 2);
        diagnose_patient(e, w);
        pay = shall_peep_pay(w);
        if (pay) {
            pay_for_treatment(w, 3);
        }
        break;
    case 5:
        pay_for_treatment(w, 2);
        if (w->unknown172[0]) {
            w->unknown171 = w->unknown172[0];
            w->unknown172[0] = 0;
        } else {
            w->unknown171 = 1;
        }
        pay = shall_peep_pay(w);
        if (pay) {
            pay_for_treatment(w, 3);
        }
        break;
    case 3:
    default:
        break;
    }
    if (pay == 0) {
        w->unknown171 = 0;
        pay_for_treatment(w, 2);
    }
    if (w->flags & 0x400) {
        pct = w->unknown146 * 100 / gbv.ffc;
        if (allhosps[w->owner].f219 <= pct && allhosps[w->owner].rooms[w->style].unknown1D == 1 && pct > 0) {
            best_guess_treatment(w);
        } else if (allhosps[w->owner].f21b >= pct) {
            w->flags32 = 0;
            w->unknown171 = 0;
            allhosps[w->owner].rooms[w->style].unknown24++;
            w->unknown118 |= 8;
        }
    }
    next_treatment_now_setup(w, 1);
}

// FUNCTION: 0x0003c8d8
void next_treatment_now_setup(Thing *w, u8 in_room)
{
    if (w->flags & 0x400) {
        if (check_playercall(8, 0, w->owner) <= 5) {
            w->flags |= 0x10;
            set_icon_value(w, 0x1b);
            w->state_b = 0x44;
            w->timer = othervar.f82;
            w->unknown91 = make_a_playercall(8, w - thing, 0, w->owner);
            if (w->owner == player) {
                advisor.f128 |= 4;
            }
        } else {
            w->unknown171 = 0;
            w->state_b = 0x26;
            allhosps[w->owner].rooms[w->style].unknown24++;
            w->unknown118 |= 8;
        }
    } else if (w->flags & 0x10) {
        if (check_playercall(4, (u16)w->style, w->owner) <= 5) {
            w->state_b = 0x44;
            w->timer = othervar.f82;
            w->unknown91 = make_a_playercall(4, w - thing, 0, w->owner);
            if (w->owner == player) {
                advisor.f128 |= 4;
            }
        } else {
            w->unknown171 = 0;
            w->state_b = 0x26;
            allhosps[w->owner].rooms[w->style].unknown24++;
            w->unknown118 |= 0x10;
        }
    } else {
        w->state_b = 0x26;
    }
    if (w->room != 0) {
        if (in_room) {
            if (set_complex(w, 0x5a, w->room->unknown6)) {
                process_person_complex(w);
            }
        } else {
            if (set_complex(w, 0xf0, 0)) {
                process_person_complex(w);
            }
        }
    } else {
        w->kind = w->state_b;
        w->state_b = 0;
    }
}

// FUNCTION: 0x0003cb5c
void change_health(Thing *w, s16 amount)
{
    s32 health;

    health = w->unknown130;
    if (health <= 20) {
        set_icon_value(w, 5);
    } else if (health <= 40) {
        set_icon_value(w, 4);
    } else if (health <= 60) {
        set_icon_value(w, 3);
    } else if (health <= 80) {
        set_icon_value(w, 2);
    } else if (health <= 100) {
        reset_icon_value(w, w->unknown24);
        set_icon_value(w, 1);
    } else if (w->unknown24 >= 1 && w->unknown24 <= 5) {
        reset_icon_value(w, w->unknown24);
    }
    if (health > 0) {
        if (w->room != 0) {
            w->unknown132 += amount;
        } else {
            amount += w->unknown132;
            w->unknown132 = 0;
            health += amount;
            if (health < 0) {
                health = 0;
            } else if (health > 1000) {
                health = 1000;
            }
            w->unknown130 = health;
            add_health_to_history(w);
            if (w->unknown130 <= 0) {
                person_dies(w);
            }
        }
    }
}

// FUNCTION: 0x0003ccc8
void person_dies(Thing *w)
{
    u16 i;
    u16 pos;
    u16 tile;
    Map *cell;

    reset_icon_value(w, 5);
    leave_queue(w);
    tile = (u16)w->x.tile + ((u16)w->y.tile << 7);
    for (i = 1; i <= dying_place[0]; i++) {
        pos = tile + dying_place[i];
        if (map[pos].first == 0 && can_i_get_to(w, pos) > 0) {
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
    w->state_b = 0x32;
    w->unknown103 = 0;
    w->unknown61 = 0;
    w->unknown63 = 0;
    w->unknown75w = 0;
    if (set_complex(w, 0x6e, pos)) {
        process_person_complex(w);
    } else {
        w->kind = 0x32;
    }
    allhosps[w->owner].rooms[w->style].unknown20++;
    allhosps[w->owner].f1eb++;
    if (w->owner == player) {
        advisor.f128 |= 0x80;
    }
    reputation_affected(w->style, 3, 0, w->owner, 0);
    record_person_as_patient(w);
    if (w->owner == player) {
        mycontrol->unknown10++;
    }
    w->unknown118 |= 4;
    if (w->unknown91) {
        get_playercall(8, w->unknown91, w->owner);
        get_playercall(4, w->unknown91, w->owner);
        w->unknown91 = 0;
    }
}

// FUNCTION: 0x0003cf5c
void person_dead(Thing *w)
{
    Map *right;
    Map *down;
    u16 tile;

    if (w->unknown103 < 0x1e) {
        tile = (s16)((u16)w->x.tile + (w->y.tile << 7));
        right = &map[tile + 1];
        down = &map[tile + 0x80];
        if ((old_newrand() & 1) == 0) {
            if (right->first == 0 && right->unknown4 == 0) {
                w->unknown17 = 0;
            } else {
                w->unknown17 = 1;
            }
        } else {
            if (down->first == 0 && down->unknown3 == 0) {
                w->unknown17 = 1;
            } else {
                w->unknown17 = 0;
            }
        }
        w->state_b = 0x32;
        w->depth = -8;
        if (anim_starts[w->subtype].unknownA2 == 0x4c1) {
            w->unknown103 = (old_newrand() & 1) + 0x2f;
        } else {
            w->unknown103 = 0x2f;
        }
        if (w->subtype == 0xe) {
            w->subtype = w->unknown170;
            if (set_complex(w, 0x280, 0)) {
                process_person_complex(w);
            }
        } else {
            if (set_complex(w, 0x140, 0)) {
                process_person_complex(w);
            }
        }
    } else if (w->unknown103 == 0x2f) {
        if (w->unknown29 > 5) {
            w->unknown29 = 1;
        }
        w->state_b = 0x34;
        w->height = 0;
        w->unknown18 = 8;
        if (set_complex(w, 0x1cc, 0)) {
            process_person_complex(w);
        }
    } else if (w->unknown103 == 0x30) {
        if (summon_the_reaper(w)) {
            effect_illness(w, w->style, 1);
            w->unknown103 = 0x63;
            w->state_b = 0x32;
        } else {
            w->unknown103 = 0x2f;
        }
    }
}

// FUNCTION: 0x0003d188
void add_health_to_history(Thing *w)
{
    s32 diff;
    u32 i;
    s16 last;

    last = 0;
    for (i = 0; i < 10; i++) {
        if (w->unknown158[i] == 0) {
            break;
        }
        last = w->unknown158[i] * 10;
    }
    if (last - w->unknown130 < 0) {
        diff = -(last - w->unknown130);
    } else {
        diff = last - w->unknown130;
    }
    if (diff >= 50) {
        if (i >= 10) {
            for (i = 1; i < 9; i++) {
                w->unknown158[i] = w->unknown158[i + 1];
            }
        }
        w->unknown158[i] = w->unknown130 / 10;
    }
}

// FUNCTION: 0x0003d28c
void diagnose_patient(Thing *e, Thing *w)
{
    u16 illness;
    s32 ok;
    s16 diag;
    u16 r;
    s32 guess;

    guess = 1;
    diag = w->unknown146;
    if (diag >= gbv.ffc) {
        guess = 0;
        illness = (u16)w->style;
        if (w->unknown169 != illness) {
            e->unknown81 += 5;
            announce_new_illness(illness, (u16)w->owner);
            reputation_affected(illness, 0x15, 0, w->owner, 0);
            record_person_as_patient(w);
            w->unknown169 = illness;
            allhosps[w->owner].f1ef++;
        }
        if (w->unknown174) {
            alert_epidemic(w->style, w->owner);
        }
        w->unknown118 |= 2;
        ok = 1;
        if (diag < allhosps[w->owner].f217) {
            find_possible_diagnosis(w);
            if (w->unknown171 == 0) {
                w->flags |= 0x400;
                reputation_affected(w->style, 7, 0, w->owner, 0);
            } else {
                ok = 0;
            }
        }
        if (ok) {
            set_icon_value(e, 0x15);
            if (allhosps[w->owner].rooms[illness].unknown1C == 0) {
                w->flags |= 0x90;
                w->unknown171 = w->style;
                set_icon_value(w, 0x1b);
            } else {
                reputation_affected(w->style, 4, 0, w->owner, 0);
                w->unknown171 = w->style;
            }
        }
    } else if (diag <= gbv.ffe) {
        r = old_newrand() & 1;
        if (r == 0) {
            if (allhosps[w->owner].rooms[0x2c].unknown1C == 1) {
                w->unknown171 = 0x2c;
            } else {
                r = 1;
            }
        }
        if (r == 1) {
            change_health(w, -(w->unknown130 - 3));
        }
        reputation_affected(w->style, 5, 0, w->owner, 0);
        allhosps[w->owner].fc98++;
        guess = 0;
    }
    if (guess) {
        find_possible_diagnosis(w);
        if (w->unknown171 == 0) {
            set_icon_value(e, 0x18);
            w->flags |= 0x400;
            reputation_affected(w->style, 7, 0, w->owner, 0);
        } else {
            set_icon_value(e, 0x14);
        }
    }
    w->unknown172[0] = 0;
    if (expertise[w->unknown171].unknown5[2] & 1) {
        w->unknown172[0] = w->unknown171;
        w->unknown171 = 0x2a;
    }
}

// FUNCTION: 0x0003d658
s32 calc_ability(Thing *e)
{
    s32 total;
    s32 luck;
    s32 attitude;
    s32 skill;
    s32 madness;
    s32 room;

    attitude = 0;
    skill = 0;
    madness = 0;
    room = 0;
    luck = 250 - old_normal_rand(0, 500);
    if (e->unknown124 & 0x80) {
        attitude = 200;
    } else if (e->unknown124 & 8) {
        attitude = -100;
    }
    if (e->f78 < gbv.fe6) {
        skill = 100;
    } else if (e->f78 >= gbv.fe8) {
        skill = -100;
    } else if (e->f78 >= gbv.fea) {
        skill = -200;
    }
    if (e->subtype == 1 && e->unknown148[2]) {
        madness = -400;
    }
    room = calc_room_value(e->room);
    total = e->unknown126;
    total += luck;
    total += attitude;
    total += skill;
    total += madness;
    total += room;
    {
        char buf[200];

        sprintf(buf, "Diag Ability %d      Staff Ability %3d    Random %3d    Skill %3d    Tired %3d   QuackDoc %3d    RoomAdd %3d", total, e->unknown126, luck, attitude, skill, madness, room);
    }
    return total;
}

// FUNCTION: 0x0003d7c0
void add_treatment_to_history(Thing *w)
{
    u16 i;

    if (w->unknown171 != 0) {
        for (i = 0; i < 10; i++) {
            if (w->unknown148[i] == 0) {
                w->unknown148[i] = w->unknown171;
                break;
            }
        }
        if (i >= 10) {
            for (i = 0; i < 9; i++) {
                w->unknown148[i] = w->unknown148[i + 1];
            }
            w->unknown148[9] = w->unknown171;
        }
    }
}

// FUNCTION: 0x0003d89c
u8 treatment_already_done(Thing *w, u16 treatment)
{
    u16 i;
    u8 found;

    found = 0;
    for (i = 0; i < 10; i++) {
        if (w->unknown148[i] == 0) {
            break;
        }
        if (w->unknown148[i] == treatment) {
            found = 1;
            break;
        }
    }
    return found;
}

// FUNCTION: 0x0003d91c
void person_go_for_treatment(Thing *w)
{
    RoomInfo *room;
    Expertise *exp;
    u16 action;
    u16 pos;
    u16 treatment;

    treatment = (u16)w->unknown171;
    if (shall_peep_pay(w) == 0) {
        treatment = 0;
    }
    exp = &expertise[treatment];
    if (w->flags & 0x40) {
        if (listhead[w->owner].rooms[exp->category] == 0) {
            treatment = 0;
        }
    } else if (allhosps[w->owner].rooms[treatment].unknown1C == 0) {
        reputation_affected(treatment, 2, 0, w->owner, 0);
        treatment = 0;
    }
    if (treatment != 0 && listhead[w->owner].rooms[exp->category] == 0) {
        w->flags |= 0x80;
        w->timer = othervar.f82;
        set_complex(w, 0xf0, 0);
        w->state_b = 0x44;
        process_person_complex(w);
        return;
    }
    switch (treatment) {
    case 0: {
        u16 frame;

        action = 20;
        pos = get_room_info(1, 0x65, w->owner)->unknown4;
        frame = check_frame(anim_starts[w->subtype].idle, w) + (u16)w->unknown17;
        w->sprite = start[frame].frame;
        w->palette = start[frame].flags > 0 ? (u8)1 : (u8)0;
        set_complex(w, action, pos);
        process_person_complex(w);
        break;
    }
    case 0x2a:
        send_person_to_bed(w);
        break;
    case 0x2c:
        send_person_for_research(w);
        break;
    default: {
        if (allhosps[w->owner].rooms[treatment].unknown1C == 1) {
            room = get_expertise_dest(treatment, w);
        } else {
            room = &room_info[listhead[w->owner].rooms[exp->category]];
        }
        room->unknown32++;
        pos = room->unknown4;
        page_staff_i_need(w, room);
        w->unknown89 = room - room_info;
        {
            u16 frame;

            frame = check_frame(anim_starts[w->subtype].idle, w) + (u16)w->unknown17;
            w->sprite = start[frame].frame;
            w->palette = start[frame].flags > 0 ? (u8)1 : (u8)0;
        }
        w->kind = 0x24;
        fn_0004195c(w, pos);
        break;
    }
    }
}

// FUNCTION: 0x0003dcfc
s16 pre_administer_drugs(Thing *w)
{
    HospitalDrug *drug;
    AllHosps *hosp;
    u8 illness;

    hosp = &allhosps[w->owner];
    illness = w->style;
    drug = &hosp->rooms[illness].drug;
    if (drug->effectiveness == 0) {
        return 0;
    }
    return 1;
}

// FUNCTION: 0x0003dd7c
void administer_drugs(Thing *e, Thing *w)
{
    HospitalDrug *drug;
    AllHosps *hosp;
    s32 effect;
    u8 illness;

    w->unknown124 = 0;
    w->unknown126 = 0;
    add_treatment_to_history(w);
    hosp = &allhosps[w->owner];
    illness = w->style;
    drug = &hosp->rooms[illness].drug;
    effect = drug->effectiveness;
    if (w->flags & 0x800) {
        effect = 0;
    }
    if (old_normal_rand(0, 99) < effect) {
        pay_for_treatment(w, 2);
        w->unknown171 = 0;
        effect_illness(w, w->style, 1);
        e->unknown81 += 5;
        w->flags &= ~0x10;
        reset_icon_value(w, 0x1b);
        w->flags |= 0x2000;
        change_health(w, 1000);
        play_sound(10, 0xaf, -1, -1);
        if (w->unknown174) {
            clean_contagious_peep(w);
        }
    } else {
        pay_for_treatment(w, 2);
        if ((w->flags & 0x40) || expertise[w->style].unknown10[1]) {
            change_health(w, -1000);
            play_sound(10, 0xae, -1, -1);
        } else {
            if (w->unknown130 <= 1000 - effect * 10) {
                play_sound(10, 0xae, -1, -1);
            }
            change_health(w, -(1000 - effect * 10));
        }
        w->unknown171 = 1;
        pay_for_treatment(w, 3);
    }
    w->state_b = 0x26;
    if (set_complex(w, 0x5a, e->room->unknown6)) {
        process_person_complex(w);
    }
    change_balance(0x16, -drug->cost, 0, (u16)e->owner);
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
u16 call_no_diagnosis(Thing *w, char **strs)
{
    char buf2[0x7d0];
    char buf1[0xc8];
    u16 a;
    u16 b;
    s16 pct;
    u16 height;
    u16 sep;

    sep = *(u16 *)D_001303c8;
    if (w <= thing) {
        return 0;
    }
    height = lbFontPtr ? lbFontPtr[34].height : 0;
    pct = w->unknown146 * 100 / gbv.ffc;
    if (pct < 0) {
        pct = 0;
    } else if (pct > 99) {
        pct = 99;
    }
    sprintf(buf1, strs[2], pct, D_001602a4[w->style]);
    if (allhosps[w->owner].rooms[w->style].unknown1C != 1 || allhosps[w->owner].rooms[w->style].unknown1D == 0) {
        available_buttons &= ~2;
    }
    a = 0;
    b = 0;
    sprintf(buf2, strs[0]);
    fn_000c93bf(buf2, (char *)&sep);
    fn_000c93bf(buf2, strs[1]);
    fn_000c93bf(buf2, (char *)&sep);
    if (allhosps[w->owner].rooms[w->style].unknown1C == 1 && allhosps[w->owner].rooms[w->style].unknown1D == 1) {
        fn_000c93bf(buf2, buf1);
    }
    LbTextDraw(a, b, buf2);
    return 0;
}

// FUNCTION: 0x0003e2c4
void process_no_diagnosis_buttons(u8 n)
{
    switch (n) {
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
void best_guess_treatment(Thing *w)
{
    u8 pct;

    w->flags &= ~0x400;
    w->flags |= 0x1000;
    w->unknown169 = w->unknown171 = w->style;
    w->unknown172[0] = 0;
    if (expertise[w->unknown171].unknown5[2] & 1) {
        w->unknown172[0] = w->unknown171;
        w->unknown171 = 0x2a;
    }
    if (allhosps[w->owner].rooms[w->style].unknown1C == 0) {
        w->flags |= 0x10;
        set_icon_value(w, 0x1b);
    }
    pct = w->unknown146 * 100 / gbv.ffc;
    if (pct < 0) {
        pct = 0;
    } else if (pct > 99) {
        pct = 99;
    }
    if (pct < old_newrand() % 100) {
        w->flags |= 0x800;
    }
}

// FUNCTION: 0x0003e468
u16 find_possible_diagnosis(Thing *w)
{
    u16 list[9];
    u16 t;
    u16 r;
    PossibleDiag *diag;
    u16 n;
    u16 j;
    u16 i;
    u16 result;

    result = 0;
    diag = &possible_diagnoses[expertise[w->style].unknown5[1]];
    w->unknown171 = 0;
    if (diag->count) {
        for (i = 0, n = 0; i < diag->count; i++) {
            t = (u16)diag->ids[i];
            if (allhosps[w->owner].rooms[t].unknown1C == 1) {
                list[n++] = t;
            }
        }
        if (n > 0) {
            for (i = 0; i < 9; i++) {
                r = old_newrand() % n;
                t = list[r];
                if (allhosps[w->owner].rooms[t].unknown1C != 0) {
                    if (treatment_already_done(w, t) == 0) {
                        result = t;
                        break;
                    }
                    for (j = r; j < n - 1; j++) {
                        list[j] = list[j + 1];
                    }
                    --n;
                    if (n == 0) {
                        break;
                    }
                }
            }
            w->unknown171 = result;
        }
    }
    return result;
}

// FUNCTION: 0x0003e668
void record_person_as_patient(Thing *w)
{
    if (!(w->unknown118 & 0x40)) {
        reputation_affected(w->style, 0x16, 0, w->owner, 0);
        w->unknown118 |= 0x40;
    }
}

#include "hosp.h"

extern "C" {
extern u16 D_001553f9;
extern u16 D_00155432;
extern u16 D_0012dda6;
}

u8 is_person_in_vaccine_queue(Thing *w);
void insert_person_in_vaccination_queue(Thing *w);

// NONMATCHING: register choice
// FUNCTION: 0x000986a0
u8 is_person_highlighted_for_vaccination(u8 flag)
{
    Thing *w;

    w = &thing[D_001553f9];
    flag = 0;
    if (w > thing && w->renderer == 2 && w->owner == player && (s8)w->unknown174 > 0
        && w->style == myhosp->f20b) {
        if (flag || is_person_in_vaccine_queue(w) == 0) {
            D_00155432 = D_001553f9;
            return 1;
        }
    }
    D_00155432 = 0;
    return 0;
}

// FUNCTION: 0x00098770
void request_vaccination(Thing *w)
{
    insert_person_in_vaccination_queue(w);
    w->flags |= 1;
    set_icon_value(w, 0xe);
    allhosps[w->owner].vaccine_requests++;
}

// FUNCTION: 0x000987c8
void cancel_vaccination_request(Thing *w)
{
    if (is_person_in_vaccine_queue(w) && !(w->flags & 2))
        allhosps[w->owner].vaccine_requests--;
    remove_person_from_vaccination_queue(w);
    w->flags &= ~3;
    reset_icon_value(w, 0xe);
    reset_icon_value(w, 0xf);
}

// NONMATCHING: register choice
// FUNCTION: 0x00098844
void insert_person_in_vaccination_queue(Thing *w)
{
    Thing *p;

    w->vaccine_next = 0;
    if (allhosps[w->owner].vaccine_queue) {
        p = &thing[allhosps[w->owner].vaccine_queue];
        while (p->vaccine_next)
            p = &thing[p->vaccine_next];
        p->vaccine_next = w - thing;
    } else {
        allhosps[w->owner].vaccine_queue = w - thing;
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x00098950
u8 remove_person_from_vaccination_queue(Thing *w)
{
    Thing *p;

    if ((u16)(w - thing) == allhosps[w->owner].vaccine_queue) {
        allhosps[w->owner].vaccine_queue = w->vaccine_next;
        w->vaccine_next = 0;
        return 1;
    }
    for (p = &thing[allhosps[w->owner].vaccine_queue]; p > thing; p = &thing[p->vaccine_next]) {
        if ((u16)(w - thing) == p->vaccine_next) {
            p->vaccine_next = w->vaccine_next;
            w->vaccine_next = 0;
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x00098a98
u8 is_person_in_vaccine_queue(Thing *w)
{
    Thing *p;

    for (p = &thing[allhosps[w->owner].vaccine_queue]; p > thing; p = &thing[p->vaccine_next]) {
        if (p == w)
            return 1;
    }
    return 0;
}

// NONMATCHING: register choice; set_complex argument loaded with and ebx,0xffff
// FUNCTION: 0x00098b2c
u8 should_nurse_vaccinate(Thing *w)
{
    Thing *p;

    if (allhosps[w->owner].vaccine_requests == 0)
        return 0;
    for (p = &thing[allhosps[w->owner].vaccine_queue]; p > thing; p = &thing[p->vaccine_next]) {
        if ((p->flags & 1) && !(p->flags & 2) && !p->room) {
            u16 pos = can_i_go_near_point(w, p->x.tile + (p->y.tile << 7));
            if (pos && !(map[pos].unknown7 & 0x10)) {
                if (w->room)
                    set_complex(w, 0x136, w->room->unknown6);
                set_complex(w, 0xbe, pos);
                w->state_b = 0x40;
                w->state_a = 0x40;
                fn_0003e724(w);
                w->unknown73 = p - thing;
                p->flags |= 2;
                reset_icon_value(p, 0xe);
                set_icon_value(p, 0xf);
                allhosps[p->owner].vaccine_requests--;
                return 1;
            }
        }
    }
    return 0;
}

// NONMATCHING: register choice
// FUNCTION: 0x00098d0c
void nurse_vaccinates(Thing *w)
{
    Thing *p;

    p = &thing[w->unknown73];
    w->unknown73 = 0;
    if (!is_person_in_vaccine_queue(p)) {
        w->kind = 0x28;
    } else if (am_i_near_point(w, p->x.tile + (p->y.tile << 7))) {
        cancel_vaccination_request(p);
        w->kind = 0x28;
        reset_icon_value(p, 0xf);
        set_icon_value(p, 0x10);
        if ((s8)p->unknown174 > 0)
            p->unknown174 = 0xff;
        change_balance(0x1d, -D_0012dda6, 0, p->owner);
    } else {
        u16 pos = can_i_go_near_point(w, p->x.tile + (p->y.tile << 7));
        if (pos && !(map[pos].unknown7 & 0x10)) {
            set_complex(w, 0xbe, pos);
            w->state_b = 0x40;
            w->state_a = 0x40;
            fn_0003e724(w);
            w->unknown73 = p - thing;
        } else {
            w->kind = 0x28;
            p->flags &= ~2;
            allhosps[p->owner].vaccine_requests++;
        }
    }
}

// FUNCTION: 0x00098eb4
u8 could_nurse_vaccinate(Thing *w)
{
    u16 from;
    Thing *p;

    if (allhosps[w->owner].vaccine_requests == 0)
        return 0;
    if (w->room)
        from = w->room->unknown4;
    else
        from = w->x.tile + (w->y.tile << 7);
    for (p = &thing[allhosps[w->owner].vaccine_queue]; p > thing; p = &thing[p->vaccine_next]) {
        if (!(p->flags & 1) || (p->flags & 2) || p->room)
            continue;
        u16 pos = p->x.tile + (p->y.tile << 7);
        if (!(map[pos].unknown7 & 0x10) && check_points_connected(pos, from))
            return 1;
    }
    return 0;
}

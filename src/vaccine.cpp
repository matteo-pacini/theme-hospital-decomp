#include "hosp.h"

extern "C" {
}

u8 is_person_in_vaccine_queue(Thing *w);
void insert_person_in_vaccination_queue(Thing *w);

char fingchk3[20] = { 0xad, 0xa9, 0xdd, 0xc9, 0x91, 0xad, 0xa9, 0x99 };

// FUNCTION: 0x000986a0
u8 is_person_highlighted_for_vaccination(u8 flag)
{
    Thing *person;

    person = &thing[othervar.f41];
    flag = 0;
    if (person > thing && person->renderer == 2 && person->owner == player && (s8)person->unknown174 > 0
        && myhosp->f20b == person->style) {
        if (flag || is_person_in_vaccine_queue(person) == 0) {
            othervar.f7a = othervar.f41;
            return 1;
        }
    }
    othervar.f7a = 0;
    return 0;
}

// FUNCTION: 0x00098770
void request_vaccination(Thing *person)
{
    insert_person_in_vaccination_queue(person);
    person->flags |= 1;
    set_icon_value(person, 0xe);
    allhosps[person->owner].vaccine_requests++;
}

// FUNCTION: 0x000987c8
void cancel_vaccination_request(Thing *person)
{
    if (is_person_in_vaccine_queue(person) && !(person->flags & 2))
        allhosps[person->owner].vaccine_requests--;
    remove_person_from_vaccination_queue(person);
    person->flags &= ~3;
    reset_icon_value(person, 0xe);
    reset_icon_value(person, 0xf);
}

// FUNCTION: 0x00098844
void insert_person_in_vaccination_queue(Thing *person)
{
    Thing *last;

    person->vaccine_next = 0;
    if (allhosps[person->owner].vaccine_queue) {
        last = &thing[allhosps[person->owner].vaccine_queue];
        while (last->vaccine_next)
            last = &thing[last->vaccine_next];
        last->vaccine_next = person - thing;
    } else {
        allhosps[person->owner].vaccine_queue = person - thing;
    }
}

// FUNCTION: 0x00098950
u8 remove_person_from_vaccination_queue(Thing *person)
{
    Thing *queued;

    if ((u16)(person - thing) == allhosps[person->owner].vaccine_queue) {
        allhosps[person->owner].vaccine_queue = person->vaccine_next;
        person->vaccine_next = 0;
        return 1;
    }
    for (queued = &thing[allhosps[person->owner].vaccine_queue]; queued > thing; queued = &thing[queued->vaccine_next]) {
        if ((u16)(person - thing) == queued->vaccine_next) {
            queued->vaccine_next = person->vaccine_next;
            person->vaccine_next = 0;
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x00098a98
u8 is_person_in_vaccine_queue(Thing *person)
{
    Thing *queued;

    for (queued = &thing[allhosps[person->owner].vaccine_queue]; queued > thing; queued = &thing[queued->vaccine_next]) {
        if (queued == person)
            return 1;
    }
    return 0;
}

// FUNCTION: 0x00098b2c
u8 should_nurse_vaccinate(Thing *nurse)
{
    Thing *patient;

    if (allhosps[nurse->owner].vaccine_requests == 0)
        return 0;
    for (patient = &thing[allhosps[nurse->owner].vaccine_queue]; patient > thing; patient = &thing[patient->vaccine_next]) {
        if ((patient->flags & 1) && !(patient->flags & 2) && !patient->room) {
            u16 pos = can_i_go_near_point(nurse, patient->x.tile + (patient->y.tile << 7));
            if (pos && !(map[pos].unknown7 & 0x10)) {
                if (nurse->room)
                    set_complex(nurse, 0x136, nurse->room->unknown6);
                set_complex(nurse, 0xbe, pos);
                nurse->state_b = 0x40;
                nurse->state_a = 0x40;
                fn_0003e724(nurse);
                nurse->unknown73 = patient - thing;
                patient->flags |= 2;
                reset_icon_value(patient, 0xe);
                set_icon_value(patient, 0xf);
                allhosps[patient->owner].vaccine_requests--;
                return 1;
            }
        }
    }
    return 0;
}

// FUNCTION: 0x00098d0c
void nurse_vaccinates(Thing *nurse)
{
    Thing *patient;

    patient = &thing[nurse->unknown73];
    nurse->unknown73 = 0;
    if (!is_person_in_vaccine_queue(patient)) {
        nurse->kind = 0x28;
    } else if (am_i_near_point(nurse, patient->x.tile + (patient->y.tile << 7))) {
        cancel_vaccination_request(patient);
        nurse->kind = 0x28;
        reset_icon_value(patient, 0xf);
        set_icon_value(patient, 0x10);
        if ((s8)patient->unknown174 > 0)
            patient->unknown174 = 0xff;
        change_balance(0x1d, -gbv.f126, 0, patient->owner);
    } else {
        u16 pos = can_i_go_near_point(nurse, patient->x.tile + (patient->y.tile << 7));
        if (pos && !(map[pos].unknown7 & 0x10)) {
            set_complex(nurse, 0xbe, pos);
            nurse->state_b = 0x40;
            nurse->state_a = 0x40;
            fn_0003e724(nurse);
            nurse->unknown73 = patient - thing;
        } else {
            nurse->kind = 0x28;
            patient->flags &= ~2;
            allhosps[patient->owner].vaccine_requests++;
        }
    }
}

// FUNCTION: 0x00098eb4
u8 could_nurse_vaccinate(Thing *nurse)
{
    u16 from;
    Thing *patient;

    if (allhosps[nurse->owner].vaccine_requests == 0)
        return 0;
    if (nurse->room)
        from = nurse->room->unknown4;
    else
        from = nurse->x.tile + (nurse->y.tile << 7);
    for (patient = &thing[allhosps[nurse->owner].vaccine_queue]; patient > thing; patient = &thing[patient->vaccine_next]) {
        if (!(patient->flags & 1) || (patient->flags & 2) || patient->room)
            continue;
        u16 pos = patient->x.tile + (patient->y.tile << 7);
        if (!(map[pos].unknown7 & 0x10) && check_points_connected(pos, from))
            return 1;
    }
    return 0;
}

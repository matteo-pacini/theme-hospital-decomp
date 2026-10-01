#include "hosp.h"

extern "C" {
}

u8 is_person_in_vaccine_queue(Thing *person);
void insert_person_in_vaccination_queue(Thing *person);

char fingchk3[20] = { 0xad, 0xa9, 0xdd, 0xc9, 0x91, 0xad, 0xa9, 0x99 };

// Returns 1 if the patient under the mouse (othervar.pointed_thing) is this player's,
// contagious with the epidemic's illness (myhosp->epidemic_illness) and not yet queued
// for a vaccination, recording them in othervar.vaccine_target (0 otherwise). The flag
// parameter is ignored (overwritten with 0).
// FUNCTION: 0x000986a0
u8 is_person_highlighted_for_vaccination(u8 flag)
{
    Thing *person;

    person = &thing[othervar.pointed_thing];
    flag = 0;
    if (person > thing && person->type == THING_PATIENT && person->owner == player && (s8)person->contagion > 0
        && myhosp->epidemic_illness == person->illness) {
        if (flag || is_person_in_vaccine_queue(person) == 0) {
            othervar.vaccine_target = othervar.pointed_thing;
            return 1;
        }
    }
    othervar.vaccine_target = 0;
    return 0;
}

// Queues person for a vaccination (PF_VACCINE_REQUESTED) and counts the
// request.
// FUNCTION: 0x00098770
void request_vaccination(Thing *person)
{
    insert_person_in_vaccination_queue(person);
    person->flags |= PF_VACCINE_REQUESTED;
    set_icon_value(person, ICON_VACCINE_REQUESTED);
    allhosps[person->owner].vaccine_requests++;
}

// Drops person's vaccination request or nurse visit, clearing the flags,
// icons and queue entry.
// FUNCTION: 0x000987c8
void cancel_vaccination_request(Thing *person)
{
    if (is_person_in_vaccine_queue(person) && !(person->flags & PF_VACCINE_NURSE_SENT))
        allhosps[person->owner].vaccine_requests--;
    remove_person_from_vaccination_queue(person);
    person->flags &= ~(PF_VACCINE_REQUESTED | PF_VACCINE_NURSE_SENT);
    reset_icon_value(person, ICON_VACCINE_REQUESTED);
    reset_icon_value(person, ICON_VACCINE_NURSE_SENT);
}

// Appends person to their hospital's vaccination queue.
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

// Takes person out of their hospital's vaccination queue. Returns 1 if they
// were in it.
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

// Returns 1 if person is in their hospital's vaccination queue.
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

// Sends nurse to the first queued patient outside rooms she can reach that
// no nurse is on the way to (PF_VACCINE_NURSE_SENT). Returns 1 if she went.
// FUNCTION: 0x00098b2c
u8 should_nurse_vaccinate(Thing *nurse)
{
    Thing *patient;

    if (allhosps[nurse->owner].vaccine_requests == 0)
        return 0;
    for (patient = &thing[allhosps[nurse->owner].vaccine_queue]; patient > thing; patient = &thing[patient->vaccine_next]) {
        if ((patient->flags & PF_VACCINE_REQUESTED) && !(patient->flags & PF_VACCINE_NURSE_SENT) && !patient->room) {
            u16 pos = can_i_go_near_point(nurse, patient->x.tile + (patient->y.tile << 7));
            if (pos && !(map[pos].owner_flags & MAP_OUTDOOR)) {
                if (nurse->room)
                    set_complex(nurse, 0x136, nurse->room->door_inside_tile);
                set_complex(nurse, 0xbe, pos);
                nurse->next_state = SS_NURSE_VACCINATES;
                nurse->resume_state = SS_NURSE_VACCINATES;
                process_staff_complex(nurse);
                nurse->slave = patient - thing;
                patient->flags |= PF_VACCINE_NURSE_SENT;
                reset_icon_value(patient, ICON_VACCINE_REQUESTED);
                set_icon_value(patient, ICON_VACCINE_NURSE_SENT);
                allhosps[patient->owner].vaccine_requests--;
                return 1;
            }
        }
    }
    return 0;
}

// Staff state SS_NURSE_VACCINATES: next to the patient, vaccinates them
// (they stop being contagious, at gbv.vac_cost's cost); otherwise follows them,
// or gives up and puts the request back.
// FUNCTION: 0x00098d0c
void nurse_vaccinates(Thing *nurse)
{
    Thing *patient;

    patient = &thing[nurse->slave];
    nurse->slave = 0;
    if (!is_person_in_vaccine_queue(patient)) {
        nurse->state = SS_IN_CORRIDOR;
    } else if (am_i_near_point(nurse, patient->x.tile + (patient->y.tile << 7))) {
        cancel_vaccination_request(patient);
        nurse->state = SS_IN_CORRIDOR;
        reset_icon_value(patient, ICON_VACCINE_NURSE_SENT);
        set_icon_value(patient, ICON_VACCINATED);
        if ((s8)patient->contagion > 0)
            patient->contagion = 0xff;
        change_balance(TX_VACCINATION, -gbv.vac_cost, 0, patient->owner);
    } else {
        u16 pos = can_i_go_near_point(nurse, patient->x.tile + (patient->y.tile << 7));
        if (pos && !(map[pos].owner_flags & MAP_OUTDOOR)) {
            set_complex(nurse, 0xbe, pos);
            nurse->next_state = SS_NURSE_VACCINATES;
            nurse->resume_state = SS_NURSE_VACCINATES;
            process_staff_complex(nurse);
            nurse->slave = patient - thing;
        } else {
            nurse->state = SS_IN_CORRIDOR;
            patient->flags &= ~PF_VACCINE_NURSE_SENT;
            allhosps[patient->owner].vaccine_requests++;
        }
    }
}

// Returns 1 if a queued patient outside rooms, with no nurse on the way, can
// be reached from nurse's room (or tile).
// FUNCTION: 0x00098eb4
u8 could_nurse_vaccinate(Thing *nurse)
{
    u16 from;
    Thing *patient;

    if (allhosps[nurse->owner].vaccine_requests == 0)
        return 0;
    if (nurse->room)
        from = nurse->room->door_tile;
    else
        from = nurse->x.tile + (nurse->y.tile << 7);
    for (patient = &thing[allhosps[nurse->owner].vaccine_queue]; patient > thing; patient = &thing[patient->vaccine_next]) {
        if (!(patient->flags & PF_VACCINE_REQUESTED) || (patient->flags & PF_VACCINE_NURSE_SENT) || patient->room)
            continue;
        u16 pos = patient->x.tile + (patient->y.tile << 7);
        if (!(map[pos].owner_flags & MAP_OUTDOOR) && check_points_connected(pos, from))
            return 1;
    }
    return 0;
}

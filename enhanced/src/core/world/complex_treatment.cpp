// Where a patient goes for treatment: choosing its room, the ward's bed and
// the wait for the player's decision (src/patient.cpp).

#include "core/world/complex_internal.h"

#include "core/world/game.h"
#include "core/world/icons.h"
#include "core/world/nav.h"
#include "core/world/reception.h"
#include "core/world/reputation.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/trace.h"

#include <cstdlib>

namespace th
{

namespace
{

// A ward, and its beds.
constexpr uint8_t kRoomWard = 9;
constexpr uint8_t kObjBed = 8;

// get_expertise_dest (0x49e88): of hospital `owner`'s rooms with a door for
// treatment `type`, the one with queue space scoring least by its queue
// (gbv.q_points), distance (gbv.dist_points) and staff (gbv.no_staff_points
// unless a member of staff is in it), else the last such room; 0 if the
// hospital cannot give the treatment.
uint16_t get_expertise_dest(const World &world, uint16_t type, const Thing &patient)
{
    const Balance::Gbv &gbv = settings_of(world).gbv;
    uint8_t kind = kExpertiseCategory[type];
    uint16_t best = 0;
    uint16_t fallback = 0;
    uint32_t best_score = 9999;
    if (patient.owner != 0 || world.cure_available[type] != 1 || !first_room(world, patient.owner, kind))
        return 0;
    for (uint16_t r = 1; r < kRoomCount; r++) {
        const RoomInfo &room = world.rooms[r];
        if (room.owner != patient.owner || room.type != kind || room.door_tile == 0)
            continue;
        fallback = r;
        if (!spare_queue_capacity(world, 0, true, r, patient.owner))
            continue;
        uint32_t score = room.queue_length * gbv.q_points;
        score += uint32_t((std::abs(patient.x.tile - room.door_tile % kMapSize) +
                           std::abs(patient.y.tile - room.door_tile / kMapSize)) *
                          gbv.dist_points);
        score += gbv.no_staff_points;
        for (const Thing &t : world.things) {
            if (t.type == kThingStaff && t.room == r) {
                score -= gbv.no_staff_points;
                break;
            }
        }
        if (score < best_score) {
            best_score = score;
            best = r;
        }
    }
    return best ? best : fallback;
}

// send_person_to_bed (0x16eb0): to the first ward of its hospital with a
// bed no one uses, else the best ward for it (get_expertise_dest), counted
// on its way to its queue, a nurse paged there; it will stay in bed
// 300-599 turns (drawn from world.seed first).
void send_person_to_bed(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    p.timer = int16_t(old_newrand(world.seed) % 300 + 300);
    uint16_t ward = 0;
    for (uint16_t room = 1; room < kRoomCount && !ward; room++) {
        if (world.rooms[room].owner != p.owner || world.rooms[room].type != kRoomWard)
            continue;
        for (uint16_t bed = world.rooms[room].first_object; bed; bed = world.things[bed].room_obj_next) {
            if (world.things[bed].subtype == kObjBed && world.things[bed].user == 0) {
                ward = room;
                break;
            }
        }
    }
    if (ward) {
        page_member_of_staff(world, kSkillNurse, ward, true, 1, p.owner);
        p.dest_room = ward;
        set_destination_coords(world, id, world.rooms[ward].door_tile);
        play_patient_anim(r.anims, p, kAnimWalk);
        p.state = kPsGotoQueue;
        world.rooms[ward].queue_incoming++;
        return;
    }
    ward = get_expertise_dest(world, kIllDiagWard, p);
    world.rooms[ward].queue_incoming++;
    page_member_of_staff(world, kSkillNurse, ward, true, 1, p.owner);
    p.dest_room = ward;
    play_patient_anim(r.anims, p, kAnimWalk);
    p.state = kPsGotoQueue;
    set_destination_coords(world, id, world.rooms[ward].door_tile);
}

// The end of go_for_treatment and send_person_for_research: patient `id` is
// counted on its way to the queue of `room_id` (paging the staff the treatment
// needs) and walks to its door.
void go_to_queue(const Run &r, uint16_t id, uint16_t room_id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    RoomInfo &room = world.rooms[room_id];
    room.queue_incoming++;
    page_staff_i_need(world, id, room_id);
    p.dest_room = room_id;
    play_patient_anim(r.anims, p, kAnimWalk);
    p.state = kPsGotoQueue;
    set_destination_coords(world, id, room.door_tile);
}

// send_person_for_research (0x6db78): patient `id` goes for its autopsy to the
// research room get_expertise_dest picks.
void send_person_for_research(const Run &r, uint16_t id)
{
    uint16_t room_id = get_expertise_dest(r.world, kIllAutopsy, r.world.things[id]);
    trace_event("send_for_research %u room=%u", unsigned(id), unsigned(room_id));
    go_to_queue(r, id, room_id);
}

} // namespace

inline namespace complex_detail
{

// person_go_for_treatment (0x3d91c): the patient pays or not
// (shall_peep_pay); without the treatment (or paying for it) it goes home
// through the first entrance; with no room for it, it waits for a decision;
// else it walks to the queue of the room for it (get_expertise_dest),
// counted on its way there, paging the staff the treatment needs. A
// treatment the hospital cannot give goes to reputation_affected
// (kRepGoForTreatment: no points, but its price moves). A ward's diagnosis
// goes to bed (send_person_to_bed); an autopsy to the research room
// (send_person_for_research).
void go_for_treatment(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    trace_event("go_for_treatment %u treatment=%u", unsigned(id), unsigned(p.treatment));
    uint16_t treatment = p.treatment;
    if (!shall_peep_pay(world, r.anims, id))
        treatment = 0;
    uint8_t category = kExpertiseCategory[treatment];
    if (p.person_flags & kPfEmergency) {
        if (!first_room(world, p.owner, category)) {
            trace_event("emergency_leaves %u", unsigned(id));
            treatment = 0;
        }
    } else if (p.owner != 0 || world.cure_available[treatment] == 0) {
        if (p.owner == 0)
            reputation_affected(world, uint8_t(treatment), kRepGoForTreatment);
        treatment = 0;
    }
    category = kExpertiseCategory[treatment];
    if (treatment != 0 && !first_room(world, p.owner, category)) {
        p.person_flags |= kPfWaitingForRoom;
        p.timer = int16_t(r.game_turn);
        set_complex(world, id, kComplexAbandon, 0);
        p.next_state = kPsWaitsForDecision;
        process(r, id);
        return;
    }
    switch (treatment) {
    case 0:
        play_patient_anim(r.anims, p, kAnimWalk);
        set_complex(world, id, kComplexLeave,
                    world.rooms[get_room_info(world, kRoomEntrance, 0x65, p.owner)].door_tile);
        process(r, id);
        break;
    case kIllDiagWard:
        send_person_to_bed(r, id);
        break;
    case kIllAutopsy:
        send_person_for_research(r, id);
        break;
    default:
        go_to_queue(r, id,
                    world.cure_available[treatment] == 1 ? get_expertise_dest(world, treatment, p)
                                                         : first_room(world, p.owner, category));
        break;
    }
}
} // namespace complex_detail

namespace
{

// allhosps[owner].rooms[illness].turned_away++, kept for hospital 0.
void count_turned_away(World &world, const Thing &p)
{
    if (world.hospital && p.owner == 0)
        world.hospital->illnesses[p.illness % kIllnessCount].turned_away++;
}

// person_waits_for_decision for a diagnosis room, `waited` turns on: goes
// for one once the hospital has one, or home turned away after 1500 turns.
// Returns whether it was decided.
bool waits_for_decision_diagnosis(const Run &r, uint16_t id, uint16_t waited)
{
    World &world = r.world;
    Thing &p = world.things[id];
    bool done = uint8_t(find_possible_diagnosis(world, id)) != 0;
    if (!done && waited < 1500)
        return false;
    p.person_flags &= ~kPfAwaitingDecision;
    reset_icon_value(p, kIconAwaitingDecision);
    if (!(p.person_flags & kPfWaitingForRoom)) {
        get_playercall(world, 8, p.playercall, p.owner);
        p.playercall = 0;
    }
    if (done) {
        p.state = kPsGoForTreatment;
    } else {
        go_home(r, id);
        count_turned_away(world, p);
        p.person_flags |= kPfNoDiagnosis;
    }
    p.person_flags &= ~(kPfNeedsDiagnosisRoom | kPfWaitingForRoom);
    return true;
}

// person_waits_for_decision for a room for its treatment, `waited` turns
// on: goes for it once the hospital can give it and has its room, or home
// after 1500 turns. Returns whether it was decided.
bool waits_for_decision_treatment(const Run &r, uint16_t id, uint16_t waited)
{
    World &world = r.world;
    Thing &p = world.things[id];
    uint8_t treatment = p.next_treatment ? p.next_treatment : p.treatment;
    bool done = p.owner == 0 && world.cure_available[treatment] == 1 &&
                first_room(world, p.owner, kExpertiseCategory[treatment]);
    if (!done && waited < 1500)
        return false;
    p.person_flags &= ~kPfAwaitingDecision;
    reset_icon_value(p, kIconAwaitingDecision);
    if (!(p.person_flags & kPfWaitingForRoom)) {
        go_home(r, id);
        p.person_flags |= kPfNoTreatment;
        return true;
    }
    p.person_flags &= ~kPfWaitingForRoom;
    if (!done) {
        go_home(r, id);
        count_turned_away(world, p);
        p.person_flags |= kPfNoTreatment;
    } else {
        get_playercall(world, 4, p.playercall, p.owner);
        p.state = kPsGoForTreatment;
        p.playercall = 0;
    }
    return true;
}

} // namespace

inline namespace complex_detail
{

// person_waits_for_decision (0x17e30): a patient waiting for a diagnosis
// room goes once the hospital has one for it (find_possible_diagnosis,
// which draws from world.seed), one waiting for a room for its treatment
// once the hospital can give it and has its room; 1500 turns after it began
// waiting, it goes home, turned away. Its player call is answered then. The player's answers (EV_CHOICE_RESULT) are
// apply_call_event's. Meanwhile it stands, moving off a blueprint or a crowded cell.
void waits_for_decision(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (!animate(r, p) || --p.countdown > 0)
        return;
    auto waited = uint16_t(uint16_t(r.game_turn) - uint16_t(p.timer));
    bool decided = p.person_flags & kPfNeedsDiagnosisRoom ? waits_for_decision_diagnosis(r, id, waited)
                                                          : waits_for_decision_treatment(r, id, waited);
    if (decided)
        return;
    if (off_blueprint(world, id))
        return;
    stand(r, p);
    if (find_standing_place(r, id))
        return;
    p.countdown = int16_t(0x100 / p.speed);
}
} // namespace complex_detail

} // namespace th

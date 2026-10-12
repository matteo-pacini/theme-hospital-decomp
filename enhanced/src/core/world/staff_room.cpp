// Breaks and training: staff resting in the staff room, doctors sent back to
// research or training and a consultant's lectures (src/staff.cpp,
// src/training.cpp, src/research.cpp, src/nurse.cpp).

#include "core/world/staff.h"

#include "core/world/game.h"
#include "core/world/research.h"
#include "core/world/rooms.h"
#include "core/world/staff_complex.h"
#include "core/world/staff_internal.h"
#include "core/world/trace.h"
#include "core/world/training.h"

namespace th
{

namespace
{

// A staff room's sofa, pool table and video game.
constexpr uint8_t kObjSofa = 19;
constexpr uint8_t kObjPoolTable = 10;
constexpr uint8_t kObjVideoGame = 57;

// The game's staff_abilities (src/tables.cpp), by staff kind: [0] plays the
// video game, [1] plays pool.
constexpr std::array<std::array<uint8_t, 2>, 5> kStaffAbilities = {{{1, 0}, {1, 1}, {0, 1}, {0, 0}, {0, 0}}};

// The complex taking a doctor to a research room's door and in.
constexpr uint16_t kComplexToResearch = 0x96;

// The complex taking a doctor to a training room's door and in.
constexpr uint16_t kComplexToTraining = 0x96;
} // namespace

inline namespace staff_detail
{

// take_a_break (0x2adfc): the member of staff goes to the nearest staff room
// in use (get_nearest_room_info) for a rest, SS_IN_STAFF_ROOM there: a
// receptionist stops what she does first (interrupt_staff); out of their
// room by its door, then to the staff room's door and in. False, doing
// nothing, if there is no staff room.
bool take_a_break(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &s = world.things[id];
    uint16_t room = get_nearest_room_info(world, s, kRoomStaffRoom);
    if (!room)
        return false;
    trace_event("take_a_break %u room=%u tired=%d", unsigned(id), unsigned(room), int(s.tiredness));
    if (s.subtype == kStaffReceptionist)
        interrupt_staff_in(c, id);
    staff_exit_room_check(world, s);
    s.next_state = kStaffInStaffRoom;
    if (s.room) {
        set_complex(world, id, kComplexLeaveRoom, world.rooms[s.room].door_inside_tile);
        process(c, id);
        s.resume_state = kStaffEvacuate;
        set_complex(world, id, kComplexToStaffRoom, world.rooms[room].door_tile);
    } else {
        set_complex(world, id, kComplexToStaffRoom, world.rooms[room].door_tile);
        s.resume_state = kStaffEvacuate;
        process(c, id);
    }
    s.dest_room = room;
    return true;
}

} // namespace staff_detail

namespace
{

// is_staff_stepping_on_toes (0x2ce9c): whether another member of staff
// comes after them in their cell's list of things.
bool is_staff_stepping_on_toes(const World &world, const Thing &s)
{
    for (int16_t i = s.cell_next; i > 0; i = world.things[size_t(i)].cell_next)
        if (world.things[size_t(i)].type == kThingStaff)
            return true;
    return false;
}
} // namespace

inline namespace staff_detail
{

// overlapping_staff_check (0x2cf14): on every 47th turn, or standing on the
// cell inside their room's door, on another member of staff's cell, or on
// an object's cell without using an object, the member of staff walks to
// a random cell of the room (row drawn first) they can get to, back in
// `state` there. Returns whether they set off.
bool overlapping_staff_check(const Ctx &c, uint16_t id, uint8_t state)
{
    World &world = c.world;
    Thing &s = world.things[id];
    const RoomInfo &room = world.rooms[s.room];
    uint16_t tile = cell_of(s);
    const Thing &obj = world.things[s.object];
    bool uses_object = s.object && (obj.type == kThingObject || obj.type == kThingDoor) && obj.user == id;
    if (!(c.game_turn % 47 == 0 || tile == room.door_inside_tile || is_staff_stepping_on_toes(world, s) ||
          (!uses_object && (world.map[tile].flags & 7))))
        return false;
    auto row = int16_t(old_newrand(world.seed) % uint32_t(room.height));
    auto column = int16_t(old_newrand(world.seed) % uint32_t(room.width));
    auto dest = uint16_t(room.pos + ((row << 7) + column));
    if (tile == dest || !can_i_get_to(world, s, dest))
        return false;
    s.resume_state = state;
    s.next_state = state;
    set_complex(world, id, kComplexWalkTo, dest);
    process(c, id);
    return true;
}

} // namespace staff_detail

namespace
{

// nurse_check_for_work (0x6c6f4): the nurse heads for a ward in use of her
// hospital drawn at random (get_room_info_rand), SS_NURSE_IN_WARD there.
// False if there is none.
bool nurse_check_for_work(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &nurse = world.things[id];
    uint16_t room = get_room_info_rand(world, kRoomWard, 100, nurse.owner);
    if (!room)
        return false;
    set_complex(world, id, kComplexToRoom, world.rooms[room].door_tile);
    nurse.substate = 0;
    nurse.next_state = kStaffNurseInWard;
    nurse.resume_state = kStaffInCorridor;
    nurse.dest_room = room;
    process(c, id);
    return true;
}

// The member of staff in a staff room plays `complex` (the sofa, pool or
// the video game), back in SS_IN_STAFF_ROOM after it.
void staff_room_pastime(const Ctx &c, uint16_t id, uint16_t complex)
{
    Thing &s = c.world.things[id];
    s.resume_state = kStaffInStaffRoom;
    s.next_state = kStaffInStaffRoom;
    set_complex(c.world, id, complex, 0);
    process(c, id);
}

// staff_in_staff_room's pastimes: by a draw of ten, pool, the video game
// (the staff who cannot play it then doing nothing more) or, not on the
// sofa already, the sofa. Returns whether they took one or stay put.
bool staff_in_staff_room_pastime(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    const Thing &s = world.things[id];
    uint32_t roll = old_newrand(world.seed, "staff_in_staff_room") % 10;
    const std::array<uint8_t, 2> &can = kStaffAbilities[s.subtype % kStaffAbilities.size()];
    if (find_object(world, cell_of(s), kObjPoolTable, true, true, s.owner) && s.complex_action != kComplexPool &&
        roll == 1 && can[1]) {
        trace_event("staff_room %u pastime=pool", unsigned(id));
        staff_room_pastime(c, id, kComplexPool);
        return true;
    }
    if (find_object(world, cell_of(s), kObjVideoGame, true, true, s.owner) && s.complex_action != kComplexVideoGame &&
        roll == 2) {
        if (can[0]) {
            trace_event("staff_room %u pastime=video", unsigned(id));
            staff_room_pastime(c, id, kComplexVideoGame);
        }
        return true;
    }
    if (s.complex_action != kComplexSofa && find_object(world, cell_of(s), kObjSofa, true, true, s.owner)) {
        trace_event("staff_room %u pastime=sofa", unsigned(id));
        staff_room_pastime(c, id, kComplexSofa);
        return true;
    }
    return false;
}

// send_doctor_to_research (0x6dedc): the doctor goes to research, in
// another research room if in one (find_rand_research_room), else in the
// emptiest (emptiest_research_room): out of their room by its door, then to
// the research room's door and in, SS_DOCTOR_RESEARCHES there. False, doing
// nothing, if there is none to go to.
bool send_doctor_to_research(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t room = d.room != 0 && world.rooms[d.room].type == kRoomResearch
                        ? find_rand_research_room(world, d.owner, d.room)
                        : emptiest_research_room(world, d.owner);
    if (!room)
        return false;
    trace_event("send_to_research %u room=%u", unsigned(id), unsigned(room));
    staff_exit_room_check(world, d);
    if (d.room)
        set_complex(world, id, kComplexLeaveRoom, world.rooms[d.room].door_inside_tile);
    set_complex(world, id, kComplexToResearch, world.rooms[room].door_tile);
    d.next_state = kStaffDoctorResearches;
    d.resume_state = kStaffEvacuate;
    d.dest_room = room;
    process(c, id);
    return true;
}

// send_doctor_to_training (0x4303c): sends the doctor to a training room
// other than their current one: a consultant to one without a consultant
// (find_training_room_no_consultant), to teach; another doctor to the one
// with the most free lecture chairs (find_emptiest_training_room). Out of
// their room by its door, then to the training room's door and in, in the
// state for their grade. False, doing nothing, if there is none.
bool send_doctor_to_training(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t room;
    uint8_t state;
    if (d.skills & kSkillConsultant) {
        room = find_training_room_no_consultant(world, d.owner, d.room);
        state = kStaffConsultantTrains;
    } else {
        room = find_emptiest_training_room(world, d.owner, d.room);
        state = kStaffDoctorIsTrained;
    }
    if (!room)
        return false;
    trace_event("send_to_training %u room=%u", unsigned(id), unsigned(room));
    staff_exit_room_check(world, d);
    if (d.room)
        set_complex(world, id, kComplexLeaveRoom, world.rooms[d.room].door_inside_tile);
    d.next_state = state;
    d.resume_state = kStaffEvacuate;
    set_complex(world, id, kComplexToTraining, world.rooms[room].door_tile);
    d.dest_room = room;
    process(c, id);
    return true;
}

// check_training_command (0x42ff0): if the doctor has been ordered to train
// (ORD_TRAIN) and a training room is available (send_doctor_to_training),
// sends them there and clears the order. Returns whether they were sent.
bool check_training_command(const Ctx &c, uint16_t id)
{
    Thing &d = c.world.things[id];
    if ((d.orders & kOrderTrain) && send_doctor_to_training(c, id)) {
        d.orders &= 0xf0;
        return true;
    }
    return false;
}
} // namespace

inline namespace staff_detail
{

// doctor_is_trained (0x42a10): SS_DOCTOR_IS_TRAINED (a student in the
// training room): a consultant switches to teaching; otherwise leaves for a
// rest, new orders or an evacuation as needed, else sits on a free lecture
// chair or walks to a random spot. Marks them to return here after a rest.
void doctor_is_trained(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    d.orders &= uint8_t(~kOrderTrain);
    if (d.skills & kSkillConsultant) {
        d.state = kStaffConsultantTrains;
        return;
    }
    if (shall_i_resign(c, id))
        return;
    d.orders |= kOrderReturnToTraining;
    if (world.rooms[d.room].being_edited || world.rooms[d.room].state == kRoomEdited) {
        evacuate_staff(c, id);
        return;
    }
    if (check_training_command(c, id))
        return;
    if (d.orders & kOrderLeaveRoom) {
        d.orders &= 0xf0;
        evacuate_staff(c, id);
        return;
    }
    if (((d.orders & kOrderRest) || d.tiredness > policy_of(world).rest_threshold) && take_a_break(c, id))
        return;
    if (find_object(world, cell_of(d), kObjLectureChair, true, true, d.owner) &&
        d.complex_action != kComplexLectureChair) {
        d.next_state = kStaffDoctorIsTrained;
        d.resume_state = kStaffDoctorIsTrained;
        set_complex(world, id, kComplexLectureChair, 0);
        process(c, id);
        return;
    }
    if (d.complex_action == kComplexLectureChair)
        return;
    const RoomInfo &room = world.rooms[d.room];
    uint8_t x = uint8_t(old_newrand(world.seed) % uint32_t(room.width) + room.pos % 128);
    uint8_t y = uint8_t(old_newrand(world.seed) % uint32_t(room.height) + room.pos / 128);
    if (world.map[(y << 7) | x].first == 0 && can_i_get_to(world, d, uint16_t(x + (y << 7)))) {
        d.next_state = kStaffDoctorIsTrained;
        d.resume_state = kStaffDoctorIsTrained;
        set_complex(world, id, kComplexWalkTo, uint16_t(x + (y << 7)));
        process(c, id);
    }
}

} // namespace staff_detail

namespace
{

// consultant_trains' bookcase (2) and skeleton (1) lessons: by a draw of a
// thousand under 100, teaching from a free one near the consultant.
// Returns whether they took one.
bool consultant_trains_teach(const Ctx &c, uint16_t id, uint8_t object, uint16_t complex, uint8_t kind)
{
    World &world = c.world;
    Thing &d = world.things[id];
    if (!(old_newrand(world.seed, "consultant_trains") % 1000 < 100 &&
          find_object(world, cell_of(d), object, true, true, d.owner)))
        return false;
    training_done(world, d, kind);
    d.next_state = kStaffConsultantTrains;
    d.resume_state = kStaffConsultantTrains;
    set_complex(world, id, complex, 0);
    process(c, id);
    return true;
}

// consultant_trains' checks before it works: resigning, the room being
// edited, new orders, the room mutex and a break. Returns whether the
// consultant did something other than teach.
bool consultant_trains_busy(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    if (shall_i_resign(c, id))
        return true;
    if (world.rooms[d.room].being_edited || world.rooms[d.room].state == kRoomEdited) {
        evacuate_staff(c, id);
        return true;
    }
    if (check_training_command(c, id))
        return true;
    if (d.orders & kOrderLeaveRoom) {
        d.orders &= 0xf0;
        evacuate_staff(c, id);
        return true;
    }
    if (!check_room_mutex(world, id)) {
        evacuate_staff(c, id);
        return true;
    }
    d.orders |= kOrderReturnToTraining;
    if (((d.orders & kOrderRest) || d.tiredness > policy_of(world).rest_threshold) && take_a_break(c, id))
        return true;
    return false;
}
} // namespace

inline namespace staff_detail
{

// consultant_trains (0x42c90): SS_CONSULTANT_TRAINS: the consultant holds
// the room, starts the projector, and gives lectures (training_done kind 0),
// or now and then teaches from the bookcase (2) or skeleton (1); leaves for
// a rest, new orders or an evacuation as needed.
void consultant_trains(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    d.orders &= uint8_t(~kOrderTrain);
    if (consultant_trains_busy(c, id))
        return;
    uint16_t projector = find_object(world, cell_of(d), kObjProjector, false, false, d.owner);
    if (!projector)
        return;
    if (!world.things[projector].lecture_started) {
        world.things[projector].lecture_started = 1;
        set_complex(world, id, kComplexStartProjector, 0);
        d.next_state = kStaffConsultantTrains;
        process(c, id);
        return;
    }
    if (consultant_trains_teach(c, id, kObjBookcase, kComplexTrainingBookcase, 2))
        return;
    if (consultant_trains_teach(c, id, kObjSkeleton, kComplexSkeleton, 1))
        return;
    if (d.complex_action != kComplexToProjector && d.complex_action != kComplexLecture) {
        d.next_state = kStaffConsultantTrains;
        d.resume_state = kStaffEvacuate;
        set_complex(world, id, kComplexToProjector, 0);
        process(c, id);
        return;
    }
    training_done(world, d, 0);
    d.complex_pc = 0;
    d.complex_queued = 0;
    d.complex_queued2 = 0;
    d.complex_resume = 0;
    d.next_state = kStaffConsultantTrains;
    set_complex(world, id, kComplexLecture, 0);
    process(c, id);
}

} // namespace staff_detail

namespace
{

// staff_in_staff_room once rested: discontent halved, a crazy doctor cured,
// the others back to work out of `room` (a doctor who was researching or
// training back to it: send_doctor_to_research, send_doctor_to_training),
// their speed again by `settings`. Returns false for a handyman or
// receptionist, sent out (SS_EVACUATE) with nothing more, and for a doctor
// gone back to research or training.
bool staff_in_staff_room_rested(const Ctx &c, uint16_t id, const Balance &settings, const RoomInfo &room)
{
    World &world = c.world;
    Thing &s = world.things[id];
    trace_event("rested %u discontent=%u crazy=%u", unsigned(id), unsigned(s.discontent), unsigned(s.crazy));
    s.discontent = uint16_t(int16_t(s.discontent) / 2);
    if (s.subtype == kStaffDoctor && s.crazy) {
        if (s.layers[5] == 3)
            s.layers[5] = 1;
        else if (s.layers[5] == 4)
            s.layers[5] = 2;
        s.crazy = 0;
    } else if (s.subtype == kStaffDoctor) {
        if ((s.orders & kOrderReturnToResearch) && send_doctor_to_research(c, id))
            return false;
        if ((s.orders & kOrderReturnToTraining) && send_doctor_to_training(c, id))
            return false;
        if (!obey_pager(c, id)) {
            set_complex(world, id, kComplexLeaveRoom, room.door_inside_tile);
            s.next_state = kStaffInCorridor;
            process(c, id);
        }
    } else if (s.subtype == kStaffNurse) {
        set_complex(world, id, kComplexLeaveRoom, room.door_inside_tile);
        if (!nurse_check_for_work(c, id)) {
            s.next_state = kStaffInCorridor;
            process(c, id);
        }
    } else if (s.subtype == kStaffHandyman || s.subtype == kStaffReceptionist) {
        s.state = kStaffEvacuate;
        return false;
    }
    calculate_speed(world, s, settings);
    return true;
}
} // namespace

inline namespace staff_detail
{

// staff_in_staff_room (0x2af04): at each end of their animation, once their
// countdown runs out (every 256 / speed of them), the member of staff rests
// (rest_staff: gbv.rest_sofa on the sofa, else gbv.rest_standing), their
// ordered rest done; ordered out of the room (kOrderLeaveRoom, the low
// orders cleared) or with the room being edited, they leave
// (evacuate_staff). Else, by a draw of ten, one time in ten they play pool
// (doctors and handymen) and one in ten the video game (doctors and
// nurses; the others then do nothing more) on a free one they are not
// already playing; else, not on the sofa already, they sit on a free one;
// else they move off a crowded cell (overlapping_staff_check); then, at
// speed 0x20, once no longer tired
// (tiredness 0 or less) their discontent halves and a crazy doctor is cured
// and stays, another doctor goes back to research if they came from it
// (kOrderReturnToResearch: send_doctor_to_research), else answers a page
// (obey_pager) or goes back to the corridor, a nurse goes to a ward
// (nurse_check_for_work) or the corridor, and a handyman or receptionist
// goes out (SS_EVACUATE). The order to go back to training sends them to the training room (send_doctor_to_training).
void staff_in_staff_room(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &s = world.things[id];
    if (c.game_turn % s.anim_speed != 0)
        return;
    s.sprite = c.anims.frames[s.sprite].next;
    if (!(c.anims.frames[s.sprite].flags & 1) || --s.countdown > 0)
        return;
    static const Balance statics;
    const Balance &settings = world.settings ? *world.settings : statics;
    const Balance::Gbv &gbv = settings.gbv;
    rest_staff(world, s, int16_t(s.complex_action == kComplexSofa ? gbv.rest_sofa : gbv.rest_standing));
    s.orders &= uint8_t(~kOrderRest);
    if (s.orders & kOrderLeaveRoom) {
        s.orders &= 0xf0;
        evacuate_staff(c, id);
        return;
    }
    const RoomInfo &room = world.rooms[s.room];
    if (room.being_edited || room.state == kRoomEdited) {
        evacuate_staff(c, id);
        return;
    }
    if (staff_in_staff_room_pastime(c, id))
        return;
    if (overlapping_staff_check(c, id, kStaffInStaffRoom))
        return;
    s.speed = 0x20;
    if (s.tiredness <= 0 && !staff_in_staff_room_rested(c, id, settings, room))
        return;
    s.countdown = int16_t(0x100 / s.speed);
}

} // namespace staff_detail

} // namespace th

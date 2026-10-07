#include "core/world/training.h"

#include "core/data/balance.h"
#include "core/world/research.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/trace.h"
#include "core/world/world.h"

#include <algorithm>

namespace th
{

namespace
{

// The three skills a consultant can teach.
constexpr uint16_t kTrainingSkills = kSkillSurgeon | kSkillPsychiatrist | kSkillResearcher;

// The advisor announcement kinds and values of a skill gained
// (src/advisor.cpp, ANN_PROMOTED).
constexpr uint8_t kAnnPromoted = 4;
constexpr uint32_t kPromotedSurgeon = 5;
constexpr uint32_t kPromotedPsychiatrist = 6;
constexpr uint32_t kPromotedResearcher = 8;

// TRAINING_OBJECT: the object `student` is using, if it is an object or door
// whose user is the student, else 0.
uint16_t training_object(const World &world, uint16_t id)
{
    const Thing &s = world.things[id];
    if (!s.object)
        return 0;
    const Thing &obj = world.things[s.object];
    if ((obj.type != kThingObject && obj.type != kThingDoor) || obj.user != id)
        return 0;
    return s.object;
}

// Whether `id` is a seated student: a doctor without the consultant skill on
// a lecture chair.
bool is_seated_student(const World &world, uint16_t id)
{
    const Thing &s = world.things[id];
    if (s.type != kThingStaff || s.subtype != kStaffDoctor || (s.skills & kSkillConsultant))
        return false;
    uint16_t obj = training_object(world, id);
    return obj != 0 && world.things[obj].subtype == kObjLectureChair;
}

uint32_t count_seated_students(const World &world, uint16_t room)
{
    uint32_t n = 0;
    for (uint16_t s = world.rooms[room].first_person; s; s = world.things[s].room_next)
        if (is_seated_student(world, s))
            n++;
    return n;
}

// What one lesson gives: the ability limit the consultant drew, their
// trainable skills, the lesson's base value and the seated students (a
// quarter of them plus one).
struct Lesson {
    uint32_t limit;
    uint16_t skills;
    uint32_t base;
    uint32_t students;
};

// A student gains `points` towards one skill, getting it past `threshold`,
// announced for the consultant's player.
void gain_surgeon(World &world, uint16_t id, uint32_t points, uint16_t threshold)
{
    Thing &s = world.things[id];
    s.surgeon_training = uint8_t(s.surgeon_training + points);
    if (s.surgeon_training > threshold) {
        trace_event("promoted %u skill=%u", unsigned(id), unsigned(kSkillSurgeon));
        s.skills |= kSkillSurgeon;
        if (s.owner == 0)
            announce_this(world, kAnnPromoted, kPromotedSurgeon);
        set_expertise_staff(world, id);
    }
}

void gain_psychiatrist(World &world, uint16_t id, uint32_t points, uint16_t threshold)
{
    Thing &s = world.things[id];
    s.psych_training = uint8_t(s.psych_training + points);
    if (s.psych_training > threshold) {
        trace_event("promoted %u skill=%u", unsigned(id), unsigned(kSkillPsychiatrist));
        s.skills |= kSkillPsychiatrist;
        if (s.owner == 0)
            announce_this(world, kAnnPromoted, kPromotedPsychiatrist);
        set_expertise_staff(world, id);
    }
}

void gain_researcher(World &world, uint16_t id, uint32_t points, uint16_t threshold)
{
    Thing &s = world.things[id];
    s.research_training = uint8_t(s.research_training + points);
    if (s.research_training > threshold) {
        trace_event("promoted %u skill=%u", unsigned(id), unsigned(kSkillResearcher));
        s.skills |= kSkillResearcher;
        if (s.owner == 0)
            announce_this(world, kAnnPromoted, kPromotedResearcher);
        set_expertise_staff(world, id);
    }
}

// One student's lesson: the skills they may learn (the consultant's they
// lack), counted into `count`, then, if their ability is below the lesson's
// limit, the ability gain and the training points towards each skill. The
// gain divides by a quarter of the seated students plus one and by `count`.
void teach_student(World &world, uint16_t id, const Lesson &lesson, uint8_t &count)
{
    Thing &student = world.things[id];
    uint32_t learn = lesson.skills & ~uint32_t(student.skills & kTrainingSkills);
    if (learn & kTrainingSkills) {
        if (learn & kSkillSurgeon)
            count++;
        else if (learn & kSkillPsychiatrist)
            count++;
        else if (learn & kSkillResearcher)
            count++;
    }
    if (student.ability >= lesson.limit)
        return;
    static const Balance statics;
    const Balance::Gbv &gbv = world.settings ? world.settings->gbv : statics.gbv;
    uint32_t divisor = lesson.students * count;
    uint32_t add = uint32_t(gbv.training_rate) / divisor;
    student.ability = std::clamp<uint16_t>(uint16_t(student.ability + (add > 0 ? add : 1)), 30, 1000);
    trace_event("train_ability %u ability=%u", unsigned(id), unsigned(student.ability));
    uint32_t points = lesson.base / divisor;
    if (points == 0)
        points = 1;
    if (learn & kSkillSurgeon)
        gain_surgeon(world, id, points, gbv.ability_threshold[0]);
    if (learn & kSkillPsychiatrist)
        gain_psychiatrist(world, id, points, gbv.ability_threshold[1]);
    if (learn & kSkillResearcher)
        gain_researcher(world, id, points, gbv.ability_threshold[2]);
}

} // namespace

uint16_t find_training_room_no_consultant(const World &world, uint8_t owner, uint16_t exclude)
{
    for (uint16_t room = 1; room < world.rooms.size(); room++) {
        const RoomInfo &r = world.rooms[room];
        if (r.type != kRoomTraining || r.owner != owner || room == exclude)
            continue;
        bool free = true;
        for (uint16_t o = r.first_person; o; o = world.things[o].room_next) {
            const Thing &occupant = world.things[o];
            if (occupant.type == kThingStaff && occupant.subtype == kStaffDoctor &&
                (occupant.skills & kSkillConsultant)) {
                free = false;
                break;
            }
        }
        if (free)
            return room;
    }
    return 0;
}

uint16_t find_emptiest_training_room(const World &world, uint8_t owner, uint16_t exclude)
{
    int32_t best = -10000;
    uint16_t found = 0;
    for (uint16_t room = 1; room < world.rooms.size(); room++) {
        const RoomInfo &r = world.rooms[room];
        if (r.type != kRoomTraining || r.owner != owner || room == exclude)
            continue;
        int32_t space = 0;
        for (uint16_t o = r.first_object; o; o = world.things[o].room_obj_next)
            if (world.things[o].subtype == kObjLectureChair)
                space++;
        for (uint16_t p = r.first_person; p; p = world.things[p].room_next) {
            const Thing &t = world.things[p];
            if (t.type == kThingStaff && t.subtype == kStaffDoctor && !(t.skills & kSkillConsultant))
                space--;
        }
        if (space > best) {
            best = space;
            found = room;
        }
    }
    return found;
}

void training_done(World &world, const Thing &consultant, uint8_t kind)
{
    static const Balance statics;
    const Balance::Gbv &gbv = world.settings ? world.settings->gbv : statics.gbv;
    uint16_t id = uint16_t(&consultant - world.things.data());
    trace_event("training %u kind=%u", unsigned(id), unsigned(kind));
    uint32_t base = gbv.training_value[kind];
    uint16_t skills = uint16_t(consultant.skills & kTrainingSkills);
    uint32_t limit = old_newrand(world.seed, "training_done") % consultant.ability;
    uint32_t students = (count_seated_students(world, consultant.room) >> 2) + 1;
    Lesson lesson{limit, skills, base, students};
    uint8_t count = 1;
    for (uint16_t s = world.rooms[consultant.room].first_person; s; s = world.things[s].room_next)
        if (is_seated_student(world, s))
            teach_student(world, s, lesson, count);
}

} // namespace th

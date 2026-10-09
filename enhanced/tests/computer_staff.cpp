// The computer players' hiring and building (src/computer.cpp: computer_employ,
// computer_build, computer_place_object) and the expertise they set.

#include "common.h"

#include "core/ui/hire_staff.h"
#include "core/world/build.h"
#include "core/world/computer.h"
#include "core/world/events.h"
#include "core/world/object_tables.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"

#include <cstdint>

namespace
{

constexpr uint8_t kRival = 1;
// OBJ_BENCH, OBJ_RECEPTION_DESK and OBJ_CHAIR, and the GP's office.
constexpr uint8_t kBench = 4;
constexpr uint8_t kDesk = 11;
constexpr uint8_t kGpOffice = 7;
// Illness 1: general practice.
constexpr size_t kGp = 1;

// The first applicant of staff kind `kind` after thing `after`, 0 if none.
uint16_t applicant(const th::World &w, uint8_t kind, uint16_t after = 0)
{
    for (size_t id = size_t(after) + 1; id < w.things.size(); id++)
        if (w.things[id].type == th::kThingStaff && w.things[id].state == th::kStaffApplicant &&
            w.things[id].subtype == kind)
            return uint16_t(id);
    return 0;
}

// Hiring pays 1.2 times the salary, counts the hire in every statistic and
// frees the applicant's thing and its place in the job market.
void check_hire(th::Game &g)
{
    using namespace th;
    World &w = g.world;
    RivalHospital &r = w.rivals[kRival];
    uint16_t id = applicant(w, kStaffReceptionist);
    CHECK(id != 0);
    Thing c = w.things[id];
    int32_t pay = c.salary * 12 / 10;
    int32_t before = r.balance;
    uint16_t applicants = g.staff_market.applicants[kStaffReceptionist];
    uint16_t total = g.staff_market.applicant_count;
    CHECK(computer_employ(w, kStaffReceptionist, 0, kRival));
    CHECK(r.balance == before - pay && r.wage_bill == c.salary);
    CHECK(r.staff_count == 1 && r.staff_by_kind[kStaffReceptionist] == 1 && r.surgeon_count == 0);
    CHECK(r.staff_ability_sum[kStaffReceptionist] == int16_t(c.ability));
    for (int i = 0; i < 9; i++)
        CHECK(r.staff_by_skill[size_t(i)] == ((c.skills >> i) & 1));
    CHECK(w.things[id].type == kThingFree && w.things[id].state == 0 && w.things[id].subtype == 0);
    CHECK(g.staff_market.applicants[kStaffReceptionist] == applicants - 1 &&
          g.staff_market.applicant_count == total - 1);
    // Nothing of the player's moved.
    CHECK(w.rivals[2].staff_count == 0 && w.rivals[2].balance == before);
}

// The first applicant of the kind decides: one the balance cannot pay for
// is not passed over for a cheaper one, and a held one is passed over.
void check_hire_refusals(th::Game &g)
{
    using namespace th;
    World &w = g.world;
    RivalHospital &r = w.rivals[kRival];
    uint16_t id = applicant(w, kStaffDoctor);
    CHECK(id != 0);
    int32_t pay = w.things[id].salary * 12 / 10;
    r.balance = pay - 1;
    CHECK(!computer_employ(w, kStaffDoctor, 0, kRival));
    CHECK(r.balance == pay - 1 && r.staff_count == 1 && w.things[id].type == kThingStaff);
    r.balance = 0;
    CHECK(!computer_employ(w, kStaffDoctor, 0, kRival));
    // Exactly the pay is enough, but nothing is left to hire with after.
    r.balance = pay;
    CHECK(computer_employ(w, kStaffDoctor, 0, kRival));
    CHECK(r.balance == 0 && r.staff_count == 2);

    // Skills not all there, and no applicant of a kind none was made for.
    r.balance = 1000000;
    uint16_t next = applicant(w, kStaffDoctor);
    CHECK(next != 0);
    CHECK(!computer_employ(w, kStaffDoctor, uint16_t(~w.things[next].skills & 0x1ff), kRival));
    CHECK(!computer_employ(w, 7, 0, kRival));
    // The held applicant is not taken, the next one is.
    w.held_staff = next;
    uint16_t other = applicant(w, kStaffDoctor, next);
    CHECK(computer_employ(w, kStaffDoctor, 0, kRival));
    CHECK(w.things[next].type == kThingStaff);
    CHECK(other == 0 || w.things[other].type == kThingFree);
    w.held_staff = 0;
}

// Buying pays the rival's own price and counts the object by its category;
// a desk is also a reception.
void check_place_object(th::Game &g)
{
    using namespace th;
    World &w = g.world;
    RivalHospital &r = w.rivals[kRival];
    r.balance = 100000;
    int32_t bench = r.object_stats[kBench].cost;
    r.object_stats[kBench].cost = 111;
    CHECK(computer_place_object(w, kBench, false, kRival));
    CHECK(r.balance == 100000 - 111 && r.amenity_count == 1 && r.object_count == 1);
    CHECK(r.objects[kBench] == 1 && r.object_stats[kBench].count == 1 && r.object_value == 0);
    r.object_stats[kBench].cost = bench;
    CHECK(computer_place_object(w, kDesk, false, kRival));
    CHECK(r.rooms[kRoomReception] == 1 && r.objects[kDesk] == 1 && r.object_count == 2);
    // A machine adds to the value of the objects.
    uint8_t machine = 0;
    for (size_t k = 1; k < size_t(kRivalObjects) && !machine; k++)
        if (kObjects[k].category == 1)
            machine = uint8_t(k);
    CHECK(machine != 0);
    CHECK(computer_place_object(w, machine, false, kRival));
    CHECK(r.machine_count == 1 && r.object_value == r.object_stats[machine].cost);

    // Not affordable: nothing changes, unless it is free.
    r.balance = r.object_stats[machine].cost - 1;
    int32_t value = r.object_value;
    CHECK(!computer_place_object(w, machine, false, kRival));
    CHECK(r.balance == r.object_stats[machine].cost - 1 && r.object_value == value && r.machine_count == 1);
    r.balance = 0;
    CHECK(!computer_place_object(w, kBench, false, kRival));
    r.balance = -50;
    CHECK(computer_place_object(w, machine, true, kRival));
    CHECK(r.balance == -50 && r.machine_count == 2 && r.object_value == value + r.object_stats[machine].cost);
}

// Building pays the room and each compulsory object the balance covers, and
// the GP gives treatments once its doctor is hired.
void check_build(th::Game &g)
{
    using namespace th;
    World &w = g.world;
    const Balance &b = g.settings();
    RivalHospital &r = w.rivals[kRival];
    const RoomTypeInfo &gp = kRoomTypeInfo[kGpOffice];
    int32_t cost = b.rooms[kGpOffice].cost;
    int32_t objects = 0;
    for (int i = 0; i < gp.compulsory_count; i++)
        objects += r.object_stats[gp.objects[size_t(i)]].cost;
    r.staff_by_skill = {};
    r.balance = cost + objects + 7;
    CHECK(computer_build(w, b, kGpOffice, kRival));
    CHECK(r.balance == 7 && r.rooms_built == 1 && r.rooms[kGpOffice] == 1);
    for (int i = 0; i < gp.compulsory_count; i++)
        CHECK(r.objects[gp.objects[size_t(i)]] >= 1);
    CHECK(r.cure_available[kGp] == 0);
    r.balance = 100000;
    CHECK(computer_employ(w, kStaffDoctor, 0, kRival));
    CHECK(r.cure_available[kGp] == 1);

    // Too dear: nothing is paid or counted. Just the room's cost: the room is
    // built and its objects are skipped, each one the balance cannot cover.
    r.balance = cost - 1;
    CHECK(!computer_build(w, b, kGpOffice, kRival));
    CHECK(r.balance == cost - 1 && r.rooms_built == 1 && r.rooms[kGpOffice] == 1);
    r.balance = cost;
    uint16_t desks = r.objects[gp.objects[0]];
    CHECK(computer_build(w, b, kGpOffice, kRival));
    CHECK(r.balance == cost - cost && r.rooms_built == 2 && r.rooms[kGpOffice] == 2);
    CHECK(r.objects[gp.objects[0]] == desks);
}

// The twin of set_expertise_room: no object needed, surgery needs two
// surgeons, a research room's treatments need skills, a ward is needed first.
void check_expertise_room()
{
    using namespace th;
    for (size_t i = 1; i < 0x2f; i++) {
        const ExpertiseInfo &exp = kExpertiseData[i];
        RivalHospital r;
        if (exp.needs_ward)
            continue;
        if (exp.staff_mask == 0) {
            set_expertise_room(r, exp.category);
            CHECK(r.cure_available[i] == (exp.category == 28 ? 0 : 1));
            continue;
        }
        set_expertise_room(r, exp.category);
        CHECK(r.cure_available[i] == 0);
        for (size_t j = 0; j < 9; j++)
            r.staff_by_skill[j] = (exp.staff_mask >> j) & 1;
        r.surgeon_count = 1;
        set_expertise_room(r, exp.category);
        CHECK(r.cure_available[i] == ((exp.staff_mask & kSkillSurgeon) ? 0 : 1));
        r.surgeon_count = 2;
        set_expertise_room(r, exp.category);
        CHECK(r.cure_available[i] == 1);
    }
}

// The twin of set_expertise_staff: needs the room counted, and the object if
// the treatment uses one.
void check_expertise_staff()
{
    using namespace th;
    Thing doctor;
    doctor.skills = kExpertiseData[kGp].staff_mask;
    RivalHospital r;
    set_expertise_staff(r, doctor);
    CHECK(r.cure_available[kGp] == 0);
    r.rooms[kExpertiseData[kGp].category] = 1;
    set_expertise_staff(r, doctor);
    CHECK(r.cure_available[kGp] == 0);
    r.objects[kExpertiseData[kGp].kind] = 1;
    set_expertise_staff(r, doctor);
    CHECK(r.cure_available[kGp] == 1);
}

// The ward treatments (surgery): a computer player gets them when the ward's
// own treatment becomes available, whether the theatre and the surgeons came
// before it (set_expertise_ward) or a surgeon is hired with the theatre
// built (set_expertise_staff waits for the ward).
void check_expertise_ward()
{
    using namespace th;
    constexpr size_t kSurgery = 14;
    const ExpertiseInfo &surgery = kExpertiseData[kSurgery];
    const ExpertiseInfo &ward = kExpertiseData[kIllDiagWard];
    CHECK(surgery.needs_ward && (surgery.staff_mask & kSkillSurgeon));

    RivalHospital early;
    early.rooms[surgery.category] = 1;
    early.surgeon_count = 2;
    for (size_t j = 0; j < 9; j++)
        early.staff_by_skill[j] = (surgery.staff_mask >> j) & 1;
    set_expertise_room(early, surgery.category);
    CHECK(early.cure_available[kSurgery] == 0);
    early.rooms[ward.category] = 1;
    early.staff_by_skill[2] = 1;
    set_expertise_room(early, ward.category);
    CHECK(early.cure_available[kIllDiagWard] == 1 && early.cure_available[kSurgery] == 1);

    // Under two surgeons the ward does not bring surgery.
    RivalHospital one;
    one.rooms[surgery.category] = 1;
    one.surgeon_count = 1;
    for (size_t j = 0; j < 9; j++)
        one.staff_by_skill[j] = (surgery.staff_mask >> j) & 1;
    one.rooms[ward.category] = 1;
    one.staff_by_skill[2] = 1;
    set_expertise_room(one, ward.category);
    CHECK(one.cure_available[kIllDiagWard] == 1 && one.cure_available[kSurgery] == 0);

    Thing surgeon;
    surgeon.skills = surgery.staff_mask;
    RivalHospital hired;
    hired.rooms[surgery.category] = 1;
    hired.objects[surgery.kind] = 1;
    hired.surgeon_count = 2;
    set_expertise_staff(hired, surgeon);
    CHECK(hired.cure_available[kSurgery] == 0);
    hired.cure_available[kIllDiagWard] = 1;
    set_expertise_staff(hired, surgeon);
    CHECK(hired.cure_available[kSurgery] == 1);
}

// A pending loan action (type 4) runs computer_loan with its argument and
// returns to deciding; the argument -1 "repays" by borrowing (docs/BUGS.md).
void check_loan_action(th::Game &g)
{
    using namespace th;
    for (size_t i = 2; i < g.world.rivals.size(); i++)
        g.world.rivals[i].next_action_turn = 0xffffffffu;
    RivalHospital &r = g.world.rivals[kRival];
    r.next_action_turn = 0;
    r.action = {4, -1, 0};
    r.action_from_note = 0;
    r.balance = 10000;
    r.loan = 5000;
    computer_action(g);
    CHECK(r.balance == 15000 && r.loan == 10000 && r.action.type == 0);
    CHECK(r.next_action_turn > 0);
}

// An applicant a computer player takes while the hire window shows it: the
// window's next draw picks another one (req2: state != SS_APPLICANT), and a
// late EV_MARK_EMPLOYING on the freed id only sets the state of a free slot
// (req2_mark_employing is unchecked), which allocation still treats as free.
void check_taken_applicant(th::Game &g)
{
    using namespace th;
    World &w = g.world;
    uint16_t shown = applicant(w, kStaffDoctor);
    CHECK(shown != 0);
    w.rivals[kRival].balance = 1000000;
    uint16_t first = alter_applicant(w, true, 0, kStaffDoctor);
    CHECK(first == shown);
    CHECK(computer_employ(w, kStaffDoctor, 0, kRival));
    CHECK(w.things[shown].state != kStaffApplicant);
    uint16_t again = alter_applicant(w, true, 0, kStaffDoctor);
    CHECK(again != shown && (again == 0 || w.things[again].state == kStaffApplicant));
    Event mark;
    mark.type = EV_MARK_EMPLOYING;
    mark.value = kStaffDoctor;
    mark.arg = shown;
    CHECK(apply_staff_event(g, mark));
    CHECK(w.things[shown].type == kThingFree);
    CHECK(applicant(w, kStaffDoctor, shown - 1) != shown);
}

} // namespace

void test_computer_staff(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    th::Game &g = *f.game;
    g.world.rivals[1].balance = g.world.rivals[2].balance = 1000000;
    check_hire(g);
    check_hire_refusals(g);
    check_place_object(g);
    check_build(g);
    check_expertise_room();
    check_expertise_staff();
    check_expertise_ward();
    check_taken_applicant(g);
    check_loan_action(g);
}

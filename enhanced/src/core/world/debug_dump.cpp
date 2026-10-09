#include "core/world/debug_dump.h"

#include "core/world/arrivals.h"
#include "core/world/build.h"
#include "core/world/complex.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/queue.h"
#include "core/world/staff.h"
#include "core/world/trace.h"
#include "core/world/world.h"

#include <cstdint>
#include <format>
#include <span>
#include <utility>
#include <vector>

namespace th
{

namespace
{

// `name=value`, numbers widened so 8-bit fields print as numbers.
template <typename T> void field(std::string &out, const char *name, T value)
{
    if constexpr (sizeof(T) == 1)
        out += std::format(" {}={}", name, int(value));
    else
        out += std::format(" {}={}", name, value);
}

template <typename Array> void list(std::string &out, const char *name, const Array &values)
{
    out += std::format(" {}=", name);
    for (size_t i = 0; i < values.size(); i++)
        out += std::format("{}{}", i ? "," : "", +values[i]);
}

#define F(s, name) field(out, #name, s.name)

// Names of game values, after include/enums.h's.
struct StateName {
    uint8_t value;
    const char *name;
};

// Thing.type (THING_).
constexpr StateName kThingTypes[] = {
    {0x00, "free"},   {0x02, "patient"},   {0x04, "staff"},  {0x06, "object"},     {0x08, "door"},
    {0x0a, "reaper"}, {0x0c, "rat"},       {0x0e, "debris"}, {0x10, "helicopter"}, {0x12, "numfx"},
    {0x14, "mayor"},  {0x16, "inspector"}, {0x18, "shadow"},
};

// Thing.subtype of a member of staff (STAFF_).
constexpr StateName kStaffKinds[] = {
    {0, "nurse"}, {1, "doctor"}, {2, "handyman"}, {3, "receptionist"}, {4, "surgeon"},
};

// Thing.subtype of a patient: only the alien kind is named (PK_ALIEN).
constexpr StateName kPatientKinds[] = {{0x0e, "alien"}};

// Thing.subtype of an object or door (OBJ_).
constexpr StateName kObjectKinds[] = {
    {1, "desk"},
    {2, "cabinet"},
    {3, "door"},
    {4, "bench"},
    {5, "table"},
    {6, "chair"},
    {7, "drinks_machine"},
    {8, "bed"},
    {9, "inflator"},
    {10, "pool_table"},
    {11, "reception_desk"},
    {12, "table2"},
    {13, "cardio"},
    {14, "scanner"},
    {15, "console"},
    {16, "screen"},
    {17, "litter_bomb"},
    {18, "couch"},
    {19, "sofa"},
    {20, "crash_trolley"},
    {21, "tv"},
    {22, "ultrascanner"},
    {23, "dna_fixer"},
    {24, "cast_remover"},
    {25, "hair_restorer"},
    {26, "slicer"},
    {27, "xray"},
    {28, "radiation_shield"},
    {29, "xray_viewer"},
    {30, "operating_table"},
    {31, "lamp"},
    {32, "sink"},
    {33, "sink2"},
    {34, "sink3"},
    {35, "surgeon_screen"},
    {36, "lecture_chair"},
    {37, "projector"},
    {38, "bed2"},
    {39, "pharmacy_cabinet"},
    {40, "computer"},
    {41, "atom_analyser"},
    {42, "blood_machine"},
    {43, "fire_extinguisher"},
    {44, "radiator"},
    {45, "plant"},
    {46, "electrolyser"},
    {47, "jelly_moulder"},
    {48, "gates_of_hell"},
    {49, "bed3"},
    {50, "bin"},
    {51, "toilet"},
    {52, "swing_door"},
    {53, "swing_door2"},
    {54, "shower"},
    {55, "auto_autopsy"},
    {56, "bookcase"},
    {57, "video_game"},
    {58, "entrance_left_door"},
    {59, "entrance_right_door"},
    {60, "skeleton"},
    {61, "comfy_chair"},
};

// Patient and staff states, after the routines process_things runs for them
// (PS_ and SS_).

constexpr StateName kPatientStates[] = {
    {0x04, "waits_for_busy"},
    {0x06, "goto_destination"},
    {0x08, "return_to_queue_area"},
    {0x0c, "go_home"},
    {0x0e, "gone"},
    {0x10, "play_complex_anim"},
    {0x12, "arrives_at_door"},
    {0x14, "in_queue"},
    {0x16, "goto_object"},
    {0x18, "entertains_themself"},
    {0x1a, "wait_for_complex_slave"},
    {0x20, "slave_ready"},
    {0x22, "two_peeps_interact"},
    {0x24, "goto_queue"},
    {0x26, "go_for_treatment"},
    {0x28, "merged"},
    {0x2a, "complex_retry"},
    {0x2c, "in_bed"},
    {0x2e, "fills"},
    {0x30, "energises"},
    {0x32, "dead"},
    {0x34, "flys"},
    {0x36, "in_bogs"},
    {0x38, "leave_toilets"},
    {0x3a, "showanim"},
    {0x3c, "stuck"},
    {0x40, "bp_goto"},
    {0x42, "at_hell_gate"},
    {0x44, "waits_for_decision"},
    {0x46, "arrived_from_call"},
};

constexpr StateName kStaffStates[] = {
    {0x06, "doctor_in_office"},
    {0x08, "doctor_works_at_desk"},
    {0x0a, "receptionist_works"},
    {0x0c, "receptionist_finds_reception"},
    {0x0e, "play_complex_anim"},
    {0x10, "surgeon_waits"},
    {0x12, "goto_object"},
    {0x14, "doctor_waits_for_patient"},
    {0x18, "assistant_ready"},
    {0x1a, "two_peeps_interact"},
    {0x1c, "doctor_examines"},
    {0x20, "recept_waits_for_patient"},
    {0x22, "recept_admits_patient"},
    {0x24, "doctor_in_machine_room"},
    {0x26, "wait_for_complex_slave"},
    {0x28, "in_corridor"},
    {0x2c, "complex_retry"},
    {0x2e, "applicant"},
    {0x30, "in_staff_room"},
    {0x32, "nurse_in_ward"},
    {0x34, "wanders"},
    {0x36, "evacuate"},
    {0x38, "back_to_applicants"},
    {0x3c, "handyman_works"},
    {0x3e, "doctor_researches"},
    {0x40, "nurse_vaccinates"},
    {0x42, "doctor_in_theatre"},
    {0x44, "held"},
    {0x46, "doctor_is_trained"},
    {0x48, "consultant_trains"},
    {0x4a, "receptionist_setup"},
    {0x4c, "bp_goto"},
    {0x4e, "stuck"},
    {0x50, "play_aniseq"},
};

// Builds a thing's InspectSections: F, L and pos on it add a field to the
// last section.
class Inspection
{
  public:
    explicit Inspection(std::vector<InspectSection> &sections) : sections_(sections) {}
    void section(std::string title) { sections_.push_back({std::move(title), {}}); }
    void add(InspectField f) { sections_.back().fields.push_back(std::move(f)); }
    void number(const char *name, int64_t value, std::string note = {})
    {
        add({name, std::to_string(value), value, InspectKind::Number, std::move(note)});
    }
    void flag(const char *name, bool value) { add({name, value ? "true" : "false", value, InspectKind::Flag, {}}); }
    void text(const char *name, std::string value) { add({name, std::move(value), 0, InspectKind::Text, {}}); }

  private:
    std::vector<InspectSection> &sections_;
};

template <typename T> void field(Inspection &out, const char *name, T value)
{
    out.number(name, int64_t(value));
}

template <typename Array> void list(Inspection &out, const char *name, const Array &values)
{
    std::string text;
    for (size_t i = 0; i < values.size(); i++)
        text += std::format("{}{}", i ? "," : "", +values[i]);
    out.text(name, std::move(text));
}

void pos(Inspection &out, const char *name, MapPos x, MapPos y)
{
    out.text(name, std::format("{}.{:02x},{}.{:02x}", x.tile, x.frac, y.tile, y.frac));
}

// `value`'s name in `names`, empty when it has none.
std::string named(std::span<const StateName> names, uint8_t value)
{
    for (const StateName &n : names)
        if (n.value == value)
            return n.name;
    return {};
}

// The state's name for a patient or member of staff, empty when it has none.
std::string state_name(const Thing &t, uint8_t value)
{
    if (t.type == kThingPatient)
        return named(kPatientStates, value);
    if (t.type == kThingStaff)
        return named(kStaffStates, value);
    return {};
}

// The kind Thing.subtype names for the thing's type, empty when unnamed.
std::string subtype_name(const Thing &t)
{
    if (t.type == kThingPatient)
        return named(kPatientKinds, t.subtype);
    if (t.type == kThingStaff)
        return named(kStaffKinds, t.subtype);
    if (t.type == kThingObject || t.type == kThingDoor)
        return named(kObjectKinds, t.subtype);
    return {};
}

void state_field(Inspection &out, const char *name, const Thing &t, uint8_t value)
{
    out.number(name, value, state_name(t, value));
}

// RoomInfo.type of a staff room and of the toilets (ROOM_STAFF_ROOM,
// ROOM_TOILETS).
constexpr uint8_t kRoomStaffRoom = 25;
constexpr uint8_t kRoomToilets = 29;

// A member of staff's break: "resting" in a staff room (SS_IN_STAFF_ROOM),
// "going" to one (take_a_break's next state), else "none".
const char *break_state(const World &w, const Thing &t)
{
    if (t.state == kStaffInStaffRoom || (t.room && w.rooms[t.room].type == kRoomStaffRoom))
        return "resting";
    if (t.next_state == kStaffInStaffRoom)
        return "going";
    return "none";
}

// A patient's visit to the toilets: "going" (PS_IN_BOGS its next state), "in"
// them (PS_IN_BOGS, or in their room), "leaving" (PS_LEAVE_TOILETS), else
// "none".
const char *toilet_state(const World &w, const Thing &t)
{
    if (t.state == kPsLeaveToilets)
        return "leaving";
    if (t.state == kPsInBogs || (t.room && w.rooms[t.room].type == kRoomToilets))
        return "in";
    if (t.next_state == kPsInBogs)
        return "going";
    return "none";
}
#define L(s, name) list(out, #name, s.name)

void dump_thing_object(Inspection &out, const Thing &t)
{
    out.section("object");
    L(t, layers);
    pos(out, "use", t.use_x, t.use_y);
    L(t, use_pos);
    F(t, use_style);
    F(t, room_id);
    F(t, pair);
    F(t, room_obj_prev);
    F(t, room_obj_next);
    F(t, type_obj_prev);
    F(t, type_obj_next);
    F(t, plant_health);
    F(t, in_use);
    F(t, user);
    F(t, door_partner);
    F(t, door_main);
    F(t, door_cell_flag);
    F(t, times_used);
    F(t, strength);
    L(t, smoke);
    F(t, overlay_frame);
    F(t, aux_sprite);
    F(t, aux_flip);
    F(t, split_height);
    F(t, aux_draw_flags);
    F(t, aux_param);
    F(t, remove_after_anim);
    F(t, lecture_started);
    F(t, rat_dx);
    F(t, rat_dy);
    F(t, rat_ticks);
}

void dump_thing_helicopter(Inspection &out, const Thing &t)
{
    out.section(t.type == kThingHelicopter ? "helicopter" : "shadow");
    F(t, altitude);
    F(t, shadow);
    F(t, caster);
}

void dump_thing_debris(Inspection &out, const Thing &t)
{
    out.section("debris");
    F(t, altitude);
    F(t, vel_x);
    F(t, vel_y);
    F(t, vel_z);
    F(t, bang_room);
}

void dump_thing_reaper(Inspection &out, const Thing &t)
{
    out.section("reaper");
    F(t, slave);
    F(t, gate);
    F(t, timer);
    F(t, reaper_next_state);
    F(t, next_sprite);
    F(t, next_flip);
}

// dump_thing's action sequence and queue lines of a person.
void dump_thing_sequence(Inspection &out, const Thing &t)
{
    out.section("sequence");
    F(t, complex_pc);
    F(t, complex_action);
    F(t, complex_resume);
    F(t, complex_queued);
    F(t, complex_queued2);
    out.text("queued_arg", std::format("{},{}", t.queued_arg_x, t.queued_arg_y));
    out.text("queued2_arg", std::format("{},{}", t.queued2_arg_x, t.queued2_arg_y));
    F(t, complex_repeat);
    state_field(out, "next_state", t, t.next_state);
    state_field(out, "resume_state", t, t.resume_state);
    state_field(out, "saved_state", t, t.saved_state);
    out.text("saved_dest", std::format("{},{}", t.saved_dest_x, t.saved_dest_y));
    F(t, saved_depth);
    F(t, saved_sprite);
    F(t, saved_flip);
    F(t, timer);
    F(t, object);
    F(t, exit_tile);
    F(t, room);
    F(t, room_next);
    F(t, room_prev);
    F(t, room_id);
    F(t, in_use);
    F(t, user);
    F(t, seq_index);
    F(t, interact_repeat);
    F(t, interact_anim);

    out.section("queue");
    F(t, queue_prev);
    F(t, queue_next);
    F(t, queue_room);
    F(t, dest_room);
    F(t, playercall);
    F(t, call_retries);
    F(t, waiting_time);
    F(t, impatience);
}

void dump_thing_patient(Inspection &out, const World &w, const Thing &t)
{
    out.section("patient");
    F(t, illness);
    F(t, known_illness);
    F(t, treatment);
    F(t, next_treatment);
    L(t, treatments);
    F(t, diagnosis);
    F(t, diagnosis_handicap);
    F(t, orig_subtype);
    F(t, insurance);
    F(t, contagion);
    F(t, vaccine_next);
    F(t, cash);
    F(t, fee);
    F(t, substate);
    F(t, slave);
    out.section("needs");
    F(t, health);
    F(t, health_delta);
    L(t, health_history);
    F(t, happiness);
    F(t, warmth);
    F(t, refreshment);
    F(t, bladder);
    F(t, nausea);
    F(t, litter_urge);
    F(t, modify_time);
    out.text("person_flags", std::format("{:#x}", t.person_flags));
    out.flag("emergency", (t.person_flags & kPfEmergency) != 0);
    out.section("toilets");
    out.text("visit", toilet_state(w, t));
}

void dump_thing_staff(Inspection &out, const World &w, const Thing &t)
{
    out.section("staff");
    F(t, substate);
    F(t, skills);
    F(t, ability);
    F(t, salary);
    F(t, wage_demand);
    F(t, tiredness);
    F(t, discontent);
    F(t, resign_count);
    F(t, crazy);
    F(t, surgeon_training);
    F(t, psych_training);
    F(t, research_training);
    out.text("hire_expiry", std::format("{}", applicant_expiry(t)));
    F(t, hire_time);
    F(t, work_delay);
    F(t, female);
    F(t, initial);
    F(t, surname_a);
    F(t, surname_b);
    F(t, face_head);
    F(t, face_body);
    F(t, face_legs);
    F(t, ref_general);
    F(t, ref_good);
    F(t, ref_bad);
    F(t, slave);
    F(t, happiness);
    out.text("person_flags", std::format("{:#x}", t.person_flags));
    out.section("rest");
    out.text("break", break_state(w, t));
    F(t, tiredness);
    out.text("orders", std::format("{:#x}", t.orders));
    F(t, resign_time);
    F(t, warmth);
    F(t, modify_time);
    out.section("work");
    F(t, prio_litter);
    F(t, prio_plants);
    F(t, prio_repair);
    F(t, work_target);
    F(t, work_target2);
    F(t, litter_count);
    F(t, plant_count);
    F(t, repair_count);
}

const char *dump_thing_kind(const Thing &t)
{
    switch (t.type) {
    case kThingPatient:
        return "patient";
    case kThingStaff:
        return t.state == kStaffApplicant ? "applicant" : "staff";
    case kThingObject:
        return "object";
    case kThingReaper:
        return "reaper";
    case kThingRat:
        return "rat";
    case kThingHelicopter:
        return "helicopter";
    case kThingShadow:
        return "shadow";
    case kThingDebris:
        return "debris";
    case kThingNumfx:
        return "numfx";
    default:
        return "thing";
    }
}

// dump_thing's fields every thing has.
void dump_thing_common(Inspection &out, const Thing &t)
{
    out.number("type", t.type, named(kThingTypes, t.type));
    out.number("subtype", t.subtype, subtype_name(t));
    state_field(out, "state", t, t.state);
    pos(out, "at", t.x, t.y);
    F(t, facing);
    F(t, flip);
    F(t, sprite);
    F(t, anim_speed);
    F(t, countdown);
    F(t, speed);
    F(t, depth);
    F(t, owner);
    F(t, cell_prev);
    F(t, cell_next);
    out.text("dest", std::format("{},{}", t.dest_x, t.dest_y));
    std::string path = std::format("{},{}->{},{}:", t.path.x0, t.path.y0, t.path.x1, t.path.y1);
    for (int i = 0; i < t.path.count && i < int(t.path.steps.size()); i++)
        path += std::format("{}{},{}", i ? ";" : "", t.path.steps[size_t(i)].x, t.path.steps[size_t(i)].y);
    out.text("path", std::move(path));
    out.section("icon");
    F(t, icon_id);
    F(t, icon_frame);
    F(t, icon_timer);
}

// dump_thing's fields by the thing's type.
void dump_thing_body(Inspection &out, const World &w, const Thing &t)
{
    if (t.type == kThingObject) {
        dump_thing_object(out, t);
        return;
    }
    if (t.type == kThingReaper) {
        dump_thing_reaper(out, t);
        return;
    }
    if (t.type == kThingHelicopter || t.type == kThingShadow) {
        dump_thing_helicopter(out, t);
        return;
    }
    if (t.type == kThingDebris) {
        dump_thing_debris(out, t);
        return;
    }
    if (t.type == kThingNumfx) {
        out.section("numfx");
        out.number("numfx_type", t.numfx_type);
        F(t, numfx_amount);
        return;
    }
    if (t.type == kThingRat) {
        out.section("rat");
        F(t, rat_dx);
        F(t, rat_dy);
        F(t, rat_ticks);
        return;
    }
    if (t.type == kThingInspector) {
        out.section("inspector");
        F(t, inspect_room);
        F(t, timer);
        F(t, goto_state);
        return;
    }
    if (t.type != kThingPatient && t.type != kThingStaff)
        return;
    dump_thing_sequence(out, t);
    if (t.type == kThingPatient)
        dump_thing_patient(out, w, t);
    else
        dump_thing_staff(out, w, t);
}

void dump_thing(Inspection &out, const World &w, const Thing &t, int id)
{
    out.section(std::format("{} {}", dump_thing_kind(t), id));
    dump_thing_common(out, t);
    dump_thing_body(out, w, t);
}

void dump_room(std::string &out, const RoomInfo &r, int id)
{
    out += std::format("room {}", id);
    F(r, type);
    F(r, state);
    out += std::format(" at={},{} size={}x{}", r.pos % kMapSize, r.pos / kMapSize, r.width, r.height);
    F(r, owner);
    F(r, door_tile);
    F(r, door_inside_tile);
    F(r, queue_head);
    F(r, queue_length);
    F(r, queue_incoming);
    F(r, queue_capacity);
    F(r, patients_seen);
    F(r, first_object);
    F(r, created_turn);
    F(r, radiators);
    F(r, windows);
    F(r, paid_for);
    F(r, being_edited);
    L(r, staff);
    F(r, staff_free);
    F(r, first_person);
    out += '\n';
}

void dump_hospital(std::string &out, const HospitalMarket &h, int id)
{
    out += std::format("hospital {}", id);
    F(h, open);
    F(h, hospital_reputation);
    F(h, hospital_rep_points);
    F(h, hospital_rep_events);
    F(h, visitors);
    F(h, month_visitors);
    F(h, diagnosed);
    F(h, price_level);
    L(h, reputation);
    L(h, price);
    L(h, price_scale);
    out += '\n';
}

void dump_rival_ai(std::string &out, const RivalHospital &r)
{
    F(r, player_type);
    F(r, ai_index);
    F(r, strategy);
    F(r, strategy_step);
    F(r, recheck_step);
    F(r, next_action_turn);
    F(r, action.type);
    F(r, action.arg);
    F(r, action.count);
    F(r, action_from_note);
    out += " notes=";
    for (const ComputerStrategy &n : r.notes)
        out += std::format("{}/{}/{};", n.type, n.arg, n.count);
}

void dump_rival_money(std::string &out, const RivalHospital &r)
{
    F(r, balance);
    F(r, loan);
    F(r, max_loan);
    F(r, wage_bill);
    F(r, hospital_value);
    F(r, object_value);
    F(r, year_start_balance);
    F(r, year_start_visitors);
    F(r, score);
    F(r, start_score);
    F(r, market_share);
    F(r, win_progress);
    F(r, area);
}

void dump_rival_counts(std::string &out, const RivalHospital &r)
{
    F(r, patient_load);
    F(r, rooms_built);
    F(r, cures);
    F(r, deaths);
    F(r, staff_count);
    F(r, surgeon_count);
    F(r, extinguisher_count);
    F(r, radiator_count);
    F(r, amenity_count);
    F(r, leisure_count);
    F(r, object_count);
    F(r, machine_count);
    F(r, heating_level);
    L(r, staff_by_kind);
    L(r, staff_ability_sum);
    L(r, staff_by_skill);
    L(r, rooms);
    L(r, objects);
}

void dump_rival_treatments(std::string &out, const RivalHospital &r)
{
    L(r, cure_available);
    L(r, discovered);
    L(r, drug_effectiveness);
    L(r, drug_cost);
    out += " object_stats=";
    for (const RivalObjectStats &s : r.object_stats)
        out += std::format("{}/{},", s.cost, s.count);
}

void dump_rival_buckets(std::string &out, const RivalHospital &r, int id)
{
    for (size_t m = 0; m < r.buckets.size(); m++) {
        const ComputerBucket &b = r.buckets[m];
        out += std::format("rival {} bucket {}", id, m);
        F(b, income);
        F(b, rep_points);
        F(b, rep_events);
        F(b, cures);
        F(b, deaths);
        F(b, patients_done);
        L(b, room_rep_points);
        L(b, room_rep_events);
        out += '\n';
    }
}

void dump_rival(std::string &out, const RivalHospital &r, int id)
{
    out += std::format("rival {} name={}", id, r.name.data());
    dump_rival_ai(out, r);
    dump_rival_money(out, r);
    dump_rival_counts(out, r);
    dump_rival_treatments(out, r);
    out += '\n';
    dump_rival_buckets(out, r, id);
}

void dump_game_hospitals(std::string &out, const Game &game)
{
    for (size_t i = 0; i < game.arrivals.hospitals.size(); i++)
        dump_hospital(out, game.arrivals.hospitals[i], int(i));
    for (size_t i = 1; i < game.world.rivals.size(); i++)
        dump_rival(out, game.world.rivals[i], int(i));
}

void dump_game_progress(std::string &out, const Game &game)
{
    const YearEnd &p = game.progress;
    out += "progress:";
    F(p, score);
    F(p, start_score);
    F(p, win_progress);
    F(p, lose_progress);
    F(p, best_win_group);
    F(p, market_share);
    F(p, year_start_visitors);
    F(p, year_start_cures);
    F(p, year_start_deaths);
    F(p, year_start_staff);
    F(p, prev_year_deaths);
    F(p, rat_shots);
    F(p, rats_killed);
    F(p, level_won);
    F(p, win_declined);
    F(p, winlose_result);
    F(p, next_level_offer);
    F(p, bonus_level);
    F(p, level_after_bonus);
    F(p, year_end_type);
    out += '\n';
    out += "awards:";
    for (size_t i = 0; i < p.awards.size(); i++)
        if (p.awards[i].text_index)
            out += std::format(" {}={{id={} dir={} value={} kind={} text={}}}", i, p.awards[i].id,
                               int(p.awards[i].direction), p.awards[i].value, p.awards[i].reward_kind,
                               p.awards[i].text_index);
    out += "\ntrophies:";
    for (size_t i = 0; i < p.trophies.size(); i++)
        if (p.trophies[i].text_index)
            out += std::format(" {}={{id={} value={} kind={} text={}}}", i, p.trophies[i].id, p.trophies[i].value,
                               p.trophies[i].reward_kind, p.trophies[i].text_index);
    out += '\n';
    out += "criteria:";
    for (size_t i = 0; i < p.win_criteria.size(); i++)
        out += std::format(" {}={{c={} g={} met={} now={} pct={}}}", i, p.win_criteria[i].criterion,
                           p.win_criteria[i].group, p.win_criteria[i].met, p.win_criteria[i].current,
                           p.win_criteria[i].progress);
    out += '\n';
}

void dump_game_finance(std::string &out, const Game &game)
{
    const Finance &f = game.finance;
    out += "finance:";
    F(f, wages_this_month);
    F(f, income_this_month);
    F(f, expenditure_this_month);
    F(f, loan);
    F(f, max_loan);
    F(f, heating_level);
    F(f, heating_price);
    F(f, hospital_value);
    F(f, object_value);
    F(f, area);
    F(f, cures);
    F(f, deaths);
    F(f, cures_this_month);
    F(f, deaths_this_month);
    out += '\n';
    for (size_t i = 0; i < game.hospital.illnesses.size(); i++) {
        const IllnessFigures &ill = game.hospital.illnesses[i];
        out += std::format("illness {}", i);
        F(ill, discovered);
        F(ill, income);
        F(ill, cured);
        F(ill, deaths);
        F(ill, turned_away);
        F(ill, drug_effectiveness);
        F(ill, drug_cost);
        out += '\n';
    }
    for (int i = 0; i < f.entry_count && i < int(f.entries.size()); i++) {
        const BankEntry &e = f.entries[size_t(i)];
        out += std::format("statement {} amount={} type={} day={} month={} extra={}\n", i, e.amount, e.type, e.day,
                           e.month, e.extra);
    }
}

// dump_game's epidemic (World::Epidemic).
void dump_game_epidemic(std::string &out, const World &w)
{
    out += "epidemic:";
    F(w.epidemic, illness);
    F(w.epidemic, fine);
    F(w.epidemic, contagious_count);
    F(w.epidemic, timer);
    F(w.epidemic, active);
    F(w.epidemic, over);
    F(w.epidemic, inspected_illness);
    L(w.epidemic, inspection);
    F(w.epidemic, vaccine_queue);
    F(w.epidemic, vaccine_requests);
    F(w.epidemic, cover_up_expired);
    out += '\n';
}

// dump_game's VIP visit (World::Mayor).
void dump_game_mayor(std::string &out, const World &w)
{
    out += "mayor:";
    F(w.mayor, arrival_turn);
    F(w.mayor, thing);
    F(w.mayor, name);
    F(w.mayor, state);
    F(w.mayor, verdict);
    F(w.mayor, postponed);
    F(w.mayor, launch_count);
    out += '\n';
}

// dump_game's panel fax icons (World::popups).
void dump_game_popups(std::string &out, const World &w)
{
    const PanelPopups &p = w.popups;
    out += "popups:";
    L(p, pos);
    F(p, shutter_state);
    F(p, shutter_width);
    F(p, shutter_timer);
    F(p, new_faxes);
    out += '\n';
}

// dump_game_world's rat holes and rat game state.
void dump_game_rats(std::string &out, const World &w)
{
    F(w, rats.hole_count);
    F(w, rats.last_litter_tile);
    F(w, rats.next_rat_layer);
    F(w, rats.near_margin);
    F(w, rats.hit_margin);
    F(w, rats.powerup_shots);
    F(w, rats.fest_level);
    F(w, rats.fest_kills_mark);
    F(w, rats.fest_powerup);
    F(w, rats.fest_rats);
}

void dump_game_world(std::string &out, const World &w)
{
    out += "world:";
    F(w, queue_near_dist);
    F(w, object_near_dist);
    F(w, rand_pct);
    F(w, shake_x);
    F(w, shake_y);
    F(w, patients_left_count);
    F(w, bench_shortage);
    F(w, plants);
    F(w, thirsty_plants);
    F(w, plant_health_sum);
    F(w, advisor_events);
    F(w, announce_stuck);
    F(w, unpaid_treatment);
    F(w, too_expensive);
    F(w, drinks_income);
    F(w, jukebox_income);
    F(w, held_staff);
    F(w, editing_room);
    F(w, held_staff_tile);
    F(w, idle_nurses);
    F(w, next_call_id);
    F(w, litter_count);
    dump_game_rats(out, w);
    F(w, mishaps);
    F(w, machine_exploded);
    F(w, disaster_counter);
    F(w, disaster_kind);
    F(w, disaster_timer);
    F(w, saved_heating);
    F(w, heating_broken);
    L(w, machine_sounds);
    L(w, machine_announcements);
    L(w, epidemic_sounds);
    L(w, treatment_sounds);
    L(w, rat_sounds);
    L(w, cure_available);
    L(w, ins_control);
    L(w, room_of_type);
    out += '\n';
    dump_game_epidemic(out, w);
    dump_game_mayor(out, w);
    for (size_t i = 0; i < w.first_call.size(); i++) {
        out += std::format("first_call {}", i);
        list(out, "by_kind", w.first_call[i]);
        out += '\n';
    }
    dump_game_popups(out, w);
}

// dump_game's statistics pass (World::counts).
void dump_game_counts(std::string &out, const World &w)
{
    const AdCounts &c = w.counts;
    out += "counts:";
    F(c, patients);
    F(c, impatience_sum);
    F(c, waiting_sum);
    L(c, wait_sums);
    L(c, wait_counts);
    F(c, happiness_sum);
    F(c, max_happiness);
    F(c, min_happiness);
    F(c, refreshment_sum);
    F(c, min_refreshment);
    F(c, warmth_sum);
    F(c, max_warmth);
    F(c, min_warmth);
    F(c, bladder_sum);
    F(c, max_bladder);
    L(c, illness_counts);
    F(c, doctors_and_nurses);
    F(c, surgeons);
    F(c, consultants);
    F(c, researchers);
    F(c, psychiatrists);
    L(c, staff_tiredness_by_kind);
    L(c, staff_happiness_by_kind);
    F(c, staff_happiness_sum);
    F(c, staff_tiredness_sum);
    F(c, max_staff_tiredness);
    F(c, staff_warmth_sum);
    L(c, idle_staff);
    F(c, crackups);
    F(c, machines);
    F(c, machine_wear_sum);
    F(c, max_machine_wear);
    F(c, worst_machine);
    F(c, has_patients);
    F(c, last_quake_size);
    out += '\n';
}

void dump_game_disasters(std::string &out, const Disasters &d)
{
    out += "disasters:";
    F(d, next_emergency_turn);
    F(d, emergency_illness);
    F(d, emergency_patients);
    F(d, emergency_win_percent);
    F(d, emergency_bonus);
    F(d, emergency_pending);
    L(d, emergency_used);
    F(d, emergencies_active);
    F(d, popup_call);
    F(d, next_quake_turn);
    F(d, quake_severity);
    F(d, quake_pending);
    L(d, quake_used);
    F(d, quake_timer);
    F(d, quake_phase);
    F(d, quake_strength);
    F(d, quake_step_chance);
    F(d, quake_amplitude);
    F(d, quake_ramp);
    F(d, quake_shake_index);
    F(d, shake_seed);
    out += '\n';
    for (size_t i = 0; i < d.offers.size(); i++) {
        const Disasters::Offer &o = d.offers[i];
        if (o.used)
            out += std::format("emergency_offer {} call={} illness={} count={} bonus={} win_percent={}\n", i, o.call,
                               o.illness, o.count, o.bonus, o.win_percent);
    }
}

// dump_game's emergency taken on (World::Emergency).
void dump_game_emergency(std::string &out, const World &w)
{
    const World::Emergency &e = w.emergency;
    out += "emergency:";
    F(e, state);
    F(e, timer);
    F(e, illness);
    F(e, patients);
    F(e, to_spawn);
    F(e, cured);
    F(e, bonus);
    F(e, win_percent);
    F(e, present);
    F(e, heli);
    F(e, heli_landed);
    F(e, heli_pad_tile);
    out += '\n';
}

// dump_game's research (World::research, Game::hospital's allocation), the
// doctors researching or to go back to it, the object kinds' figures (those
// not all 0) and the advisor's announcements waiting.
void dump_game_research(std::string &out, const Game &game)
{
    const World &w = game.world;
    const Research &r = w.research;
    out += "research:";
    F(r, points);
    F(r, earned);
    F(r, spent);
    F(r, exhausted);
    list(out, "alloc", game.hospital.research_alloc);
    out += std::format(" research_illness={}", game.hospital.research_illness);
    list(out, "cost", r.cost);
    list(out, "illness_research", r.illness_research);
    out += "\nresearch_doctors=";
    const char *sep = "";
    for (size_t i = 1; i < w.things.size(); i++) {
        const Thing &t = w.things[i];
        if (t.type == kThingStaff && t.subtype == kStaffDoctor && (t.skills & kSkillResearcher) &&
            t.state != kStaffApplicant) {
            out += std::format("{}{}:state={}:orders={:#x}", sep, i, t.state, t.orders);
            sep = ",";
        }
    }
    out += '\n';
    for (size_t i = 0; i < r.objects.size(); i++) {
        const ObjectStats &o = r.objects[i];
        if (o.cost || o.available || o.strength || o.equip_available || o.upgrade_level)
            out += std::format("object_stats {} cost={} available={} strength={} equip_available={} upgrade_level={}\n",
                               i, o.cost, o.available, o.strength, o.equip_available, o.upgrade_level);
    }
    for (size_t i = 0; i < w.announce.size(); i++)
        if (w.announce[i][0])
            out += std::format("announce {} values={},{},{},{}\n", i, w.announce[i][0], w.announce[i][1],
                               w.announce[i][2], w.announce[i][3]);
}

// dump_game's pagers, calls and repair pages.
void dump_game_pagers(std::string &out, const World &w)
{
    for (size_t i = 0; i < w.pagers.size(); i++) {
        const World::Pager &pg = w.pagers[i];
        if (pg.mask || pg.room)
            out += std::format("pager {} mask={:#x} room={} claimed_until={} responder={} owner={}\n", i, pg.mask,
                               pg.room, pg.claimed_until, pg.responder, pg.owner);
    }
    for (size_t i = 0; i < w.calls.size(); i++) {
        const World::PlayerCall &c = w.calls[i];
        if (c.type)
            out += std::format("call {} type={} subject={} id={} owner={} timer={} slot={} slide_step={}\n", i, c.type,
                               c.subject, c.id, c.owner, c.timer, c.slot, c.slide_step);
    }
    for (size_t i = 0; i < w.repair_pager.count && i < w.repair_pager.pages.size(); i++) {
        const World::ThingPage &pg = w.repair_pager.pages[i];
        out += std::format("repair_page {} id={} priority={} responder={} claimed_until={}\n", i, pg.id, pg.priority,
                           pg.responder, pg.claimed_until);
    }
    for (size_t i = 0; i < w.plant_pager.count && i < w.plant_pager.pages.size(); i++) {
        const World::ThingPage &pg = w.plant_pager.pages[i];
        out += std::format("plant_page {} id={} priority={} responder={} claimed_until={}\n", i, pg.id, pg.priority,
                           pg.responder, pg.claimed_until);
    }
}

// dump_game's building and placing, and the pagers, calls and repair pages.
void dump_game_building(std::string &out, const World &w)
{
    out += "\n## Building and placing\n";
    const Building &bd = w.building;
    const BuildState &b = bd.build;
    out += std::format("build mouse={} door_invalid={} blueprint={},{}-{},{}", int(bd.mouse), bd.door_invalid, b.x0,
                       b.y0, b.x1, b.y1);
    F(b, room);
    F(b, edit_room);
    F(b, room_type);
    F(b, drag);
    F(b, draw_mode);
    F(b, door_tile);
    F(b, door_type);
    F(b, door_kind);
    F(b, state);
    out += '\n';
    const ObjectPlacing &p = w.placing;
    out += std::format("placing requester={}", int(p.requester));
    F(p, placing);
    F(p, kind);
    F(p, dir);
    F(p, mode);
    F(p, room);
    F(p, tile);
    F(p, ok);
    F(p, room_type);
    F(p, requester_mode);
    F(p, item);
    F(p, arg);
    F(p, furnished);
    L(p, offered);
    L(p, buy_count);
    out += '\n';
    dump_game_pagers(out, w);
}

// dump_game's map cells within `radius` of cell `focus`.
void dump_game_map(std::string &out, const World &w, int focus, int radius)
{
    int fc = focus % kMapSize, fr = focus / kMapSize;
    out += std::format("\n## Map around {},{} (hex: floor wall_n wall_w flags litter_shadow owner_flags)\n", fc, fr);
    for (int r = fr - radius; r <= fr + radius; r++) {
        for (int c = fc - radius; c <= fc + radius; c++) {
            if (r < 0 || r >= kMapSize || c < 0 || c >= kMapSize)
                continue;
            const MapCell &m = w.map[size_t(c + r * kMapSize)];
            out += std::format("cell {},{} {:02x} {:02x} {:02x} {:02x} {:02x} {:02x} first={} room={}\n", c, r,
                               m.floor_block, m.wall_n, m.wall_w, m.flags, m.litter_shadow, m.owner_flags, m.first,
                               room_at(w, uint16_t(c + r * kMapSize)));
        }
    }
}

// dump_game's fax screen (Game::fax_screen) and the palette it brings.
void dump_game_fax(std::string &out, const Game &game)
{
    const Game::FaxScreen &fax = game.fax_screen;
    out += std::format("fax_screen open={} state={:#x} timeout={} backup_open={} backup_timeout={} fade={} since={} "
                       "pointer={} fax_pointer={} fax_key={} fax_palette={}\n",
                       fax.open, fax.state, fax.timeout, fax.backup_open, fax.backup_timeout, fax.fade, fax.since,
                       fax.pointer, game.fax_pointer, game.fax_key, game.world.fax_palette);
}

} // namespace

std::string dump_game(const Game &game, int focus, int radius)
{
    const World &w = game.world;
    std::string out;

    out += "## Game\n";
    out += std::format("date year={} month={} day={} turn={} paused={} open={} opening_countdown={}\n", game.date.year,
                       game.date.month, game.date.day, game.turn_count, game.paused, game.open, game.opening_countdown);
    out += std::format("balance={} seed={:08x} route_planner={}\n", game.balance, w.seed,
                       w.path_finder == PathFinder::AStar ? "A*" : "original");
    dump_game_finance(out, game);
    dump_game_progress(out, game);
    dump_game_hospitals(out, game);
    dump_game_world(out, w);
    dump_game_counts(out, w);
    out += std::format("choice_screen type={} subject={} timeout={}\n", game.choice_screen.type,
                       game.choice_screen.subject, game.choice_screen.timeout);
    dump_game_fax(out, game);
    dump_game_disasters(out, game.disasters);
    dump_game_emergency(out, w);
    dump_game_research(out, game);

    dump_game_building(out, w);

    out += "\n## Rooms\n";
    for (size_t i = 0; i < w.rooms.size(); i++)
        if (w.rooms[i].type != kRoomFree)
            dump_room(out, w.rooms[i], int(i));

    out += "\n## Things (patients, staff, job applicants, objects; at = tile.frac)\n";
    for (size_t i = 1; i < w.things.size(); i++)
        if (w.things[i].type != kThingFree)
            out += dump_thing(w, uint16_t(i));

    if (focus >= 0 && focus < kMapCells)
        dump_game_map(out, w, focus, radius);

    out += "\n## Recent events (turn, event; core/world/trace.h)\n";
    for (const TraceEvent &e : game.events.events)
        out += std::format("{} {}\n", e.turn, e.text);

    out += "\n## Trace (core/world/trace.h)\n";

    out += trace_turn(game);
    return out;
}

std::vector<InspectSection> inspect_thing(const World &world, uint16_t id)
{
    std::vector<InspectSection> sections;
    if (id && id < world.things.size()) {
        Inspection out(sections);
        dump_thing(out, world, world.things[id], id);
    }
    return sections;
}

std::string dump_thing(const World &world, uint16_t id)
{
    std::string out;
    for (const InspectSection &s : inspect_thing(world, id)) {
        out += out.empty() ? s.title : std::format(" {}:", s.title);
        for (const InspectField &f : s.fields)
            out += f.note.empty() ? std::format(" {}={}", f.name, f.value)
                                  : std::format(" {}={}({})", f.name, f.value, f.note);
        out += '\n';
    }
    return out;
}

} // namespace th

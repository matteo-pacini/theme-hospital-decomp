#include "core/world/debug_dump.h"

#include "core/world/arrivals.h"
#include "core/world/build.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/staff.h"
#include "core/world/trace.h"
#include "core/world/world.h"

#include <format>

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

void pos(std::string &out, const char *name, MapPos x, MapPos y)
{
    out += std::format(" {}={}.{:02x},{}.{:02x}", name, x.tile, x.frac, y.tile, y.frac);
}

#define F(s, name) field(out, #name, s.name)
#define L(s, name) list(out, #name, s.name)

void dump_thing(std::string &out, const Thing &t, int id)
{
    const char *kind = t.type == kThingPatient                               ? "patient"
                       : t.type == kThingStaff && t.state == kStaffApplicant ? "applicant"
                       : t.type == kThingStaff                               ? "staff"
                       : t.type == kThingObject                              ? "object"
                                                                             : "thing";
    out += std::format("{} {}", kind, id);
    F(t, type);
    F(t, subtype);
    F(t, state);
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
    out += std::format(" dest={},{}", t.dest_x, t.dest_y);
    out += std::format(" path={},{}->{},{}:", t.path.x0, t.path.y0, t.path.x1, t.path.y1);
    for (int i = 0; i < t.path.count && i < int(t.path.steps.size()); i++)
        out += std::format("{}{},{}", i ? ";" : "", t.path.steps[size_t(i)].x, t.path.steps[size_t(i)].y);
    out += '\n';

    if (t.type == kThingObject) {
        out += " object:";
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
        out += '\n';
        return;
    }
    if (t.type != kThingPatient && t.type != kThingStaff)
        return;

    out += " sequence:";
    F(t, complex_pc);
    F(t, complex_action);
    F(t, complex_resume);
    F(t, complex_queued);
    F(t, complex_queued2);
    out += std::format(" queued_arg={},{} queued2_arg={},{}", t.queued_arg_x, t.queued_arg_y, t.queued2_arg_x,
                       t.queued2_arg_y);
    F(t, complex_repeat);
    F(t, next_state);
    F(t, resume_state);
    F(t, saved_state);
    out += std::format(" saved_dest={},{}", t.saved_dest_x, t.saved_dest_y);
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
    out += '\n';

    out += " queue:";
    F(t, queue_prev);
    F(t, queue_next);
    F(t, queue_room);
    F(t, dest_room);
    F(t, playercall);
    F(t, call_retries);
    F(t, waiting_time);
    F(t, impatience);
    out += '\n';

    if (t.type == kThingPatient) {
        out += " patient:";
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
        F(t, icon_id);
        F(t, cash);
        F(t, fee);
        out += '\n';
        out += " needs:";
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
        out += std::format(" person_flags={:#x}", t.person_flags);
        out += '\n';
        return;
    }

    out += " staff:";
    F(t, substate);
    F(t, skills);
    F(t, ability);
    F(t, salary);
    F(t, wage_demand);
    F(t, tiredness);
    F(t, discontent);
    F(t, resign_count);
    F(t, crazy);
    F(t, hire_expiry);
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
    out += std::format(" person_flags={:#x}", t.person_flags);
    out += '\n';
    out += " work:";
    F(t, prio_litter);
    F(t, prio_plants);
    F(t, prio_repair);
    F(t, work_target);
    F(t, work_target2);
    F(t, litter_count);
    F(t, plant_count);
    F(t, repair_count);
    out += '\n';
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
    L(h, reputation);
    L(h, price);
    L(h, price_scale);
    out += '\n';
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
    F(f, area);
    out += '\n';
    for (int i = 0; i < f.entry_count && i < int(f.entries.size()); i++) {
        const BankEntry &e = f.entries[size_t(i)];
        out += std::format("statement {} amount={} type={} day={} month={} extra={}\n", i, e.amount, e.type, e.day,
                           e.month, e.extra);
    }
    for (size_t i = 0; i < game.arrivals.hospitals.size(); i++)
        dump_hospital(out, game.arrivals.hospitals[i], int(i));
    out += "world:";
    F(w, queue_near_dist);
    F(w, object_near_dist);
    F(w, patients_left_count);
    F(w, bench_shortage);
    F(w, unpaid_treatment);
    F(w, too_expensive);
    F(w, drinks_income);
    F(w, held_staff);
    F(w, held_staff_tile);
    F(w, next_call_id);
    L(w, cure_available);
    L(w, ins_control);
    L(w, room_of_type);
    out += '\n';
    const Disasters &d = game.disasters;
    out += "disasters:";
    F(d, next_emergency_turn);
    F(d, emergency_illness);
    F(d, emergency_patients);
    F(d, emergency_win_percent);
    F(d, emergency_bonus);
    F(d, emergency_pending);
    L(d, emergency_used);
    F(d, emergencies_active);
    F(d, next_quake_turn);
    F(d, quake_severity);
    F(d, quake_pending);
    L(d, quake_used);
    out += '\n';
    for (size_t i = 0; i < d.offers.size(); i++) {
        const Disasters::Offer &o = d.offers[i];
        if (o.used)
            out += std::format("emergency_offer {} call={} illness={} count={} bonus={} win_percent={}\n", i, o.call,
                               o.illness, o.count, o.bonus, o.win_percent);
    }

    out += "\n## Building and placing\n";
    const Building &bd = w.building;
    const BuildState &b = bd.build;
    out += std::format("build mouse={} door_invalid={} blueprint={},{}-{},{}", int(bd.mouse), bd.door_invalid, b.x0,
                       b.y0, b.x1, b.y1);
    F(b, room);
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
    L(p, offered);
    L(p, buy_count);
    out += '\n';
    for (size_t i = 0; i < w.pagers.size(); i++) {
        const World::Pager &pg = w.pagers[i];
        if (pg.mask || pg.room)
            out += std::format("pager {} mask={:#x} room={} claimed_until={} responder={} owner={}\n", i, pg.mask,
                               pg.room, pg.claimed_until, pg.responder, pg.owner);
    }
    for (size_t i = 0; i < w.calls.size(); i++) {
        const World::PlayerCall &c = w.calls[i];
        if (c.type)
            out += std::format("call {} type={} subject={} id={} owner={} timer={}\n", i, c.type, c.subject, c.id,
                               c.owner, c.timer);
    }
    for (size_t i = 0; i < w.repair_pager.count && i < w.repair_pager.pages.size(); i++) {
        const World::ThingPage &pg = w.repair_pager.pages[i];
        out += std::format("repair_page {} id={} priority={} responder={} claimed_until={}\n", i, pg.id, pg.priority,
                           pg.responder, pg.claimed_until);
    }

    out += "\n## Rooms\n";
    for (size_t i = 0; i < w.rooms.size(); i++)
        if (w.rooms[i].type != kRoomFree)
            dump_room(out, w.rooms[i], int(i));

    out += "\n## Things (patients, staff, job applicants, objects; at = tile.frac)\n";
    for (size_t i = 1; i < w.things.size(); i++)
        if (w.things[i].type != kThingFree)
            dump_thing(out, w.things[i], int(i));

    if (focus >= 0 && focus < kMapCells) {
        int fc = focus % kMapSize, fr = focus / kMapSize;
        out +=
            std::format("\n## Map around {},{} (hex: floor wall_n wall_w flags litter_shadow owner_flags)\n", fc, fr);
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

    out += "\n## Recent events (turn, event; core/world/trace.h)\n";
    for (const TraceEvent &e : game.events.events)
        out += std::format("{} {}\n", e.turn, e.text);

    out += "\n## Trace (core/world/trace.h)\n";

    out += trace_turn(game);
    return out;
}

} // namespace th

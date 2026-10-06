#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <string>

namespace th
{

class Game;

// The events a game records as its turns run (Game::events), for the replay
// check's trace and the debug report.
struct TraceEvent {
    // The turn whose trace entry it belongs to: the turn running, or for a
    // player's command the turn about to run.
    uint32_t turn = 0;
    // "<kind> <args...>", as in the trace without the leading "E ".
    std::string text;
};

struct EventLog {
    bool enabled = true;
    // Also record each draw from World::seed (`rand` events), many a turn.
    bool rand = false;
    // Events kept, the oldest dropped first; 0 keeps them all until cleared.
    size_t limit = 500;
    std::deque<TraceEvent> events;
};

// While an EventScope lives, trace_event and the random draws record into
// `log` under `turn` (nothing if `log` is null or disabled); the scope it
// replaces comes back when it ends. Game::turn and Game::process_turn open
// one, so events outside them (a level's start, tools) are not recorded.
class EventScope
{
  public:
    EventScope(EventLog *log, uint32_t turn);
    ~EventScope();
    EventScope(const EventScope &) = delete;
    EventScope &operator=(const EventScope &) = delete;

  private:
    EventLog *prev_log_;
    uint32_t prev_turn_;
};

// Records the event formatted as printf would ("<kind> <args...>") in the
// current EventScope's log; does nothing, not even the formatting, outside
// one.
#if defined(__GNUC__)
__attribute__((format(printf, 1, 2)))
#endif
void trace_event(const char *format, ...);

// Whether draws are recorded now (a scope's log with EventLog::rand).
bool tracing_rand();
// Records a draw (`rand <function> <seed>`) by the function whose
// std::source_location::function_name() is `function`, reduced to its bare
// name, leaving World::seed at `seed`.
void trace_rand(const char *function, uint32_t seed);

// One turn of the replay check's trace (enhanced/tools/replay): the state a
// game is in after a turn, in the format the instrumented original writes
// (enhanced/tools/replay/instrument.py), so the two compare line by line.
//
// A header line,
//   T <turn> seed=<8 hex> date=<year>/<month 0-11>/<day> balance=<n> patients=<n>
// with the patients counted over every hospital's; then the turn's events in
// the order they happened (all `events` holds; none if null), one a line,
//   E <kind> <args...>
// then one line per hospital,
//   H <0-3> open=<0 or 1> rep=<n> price=<n>
// rep and price the sums of its rooms[].reputation and rooms[].price over
// illnesses 1-45, which weigh where patients go (who_owns_peep);
// then one line per used entry of the things table in index order,
//    <id> type=<n> sub=<n> state=<n> x=<4 hex> y=<4 hex>
// x and y as the game's MapPos.raw (tile in the high byte).
//
// The events, each written where the game's function of the same name (or
// the one named) runs, at its start unless "at its end" (ids are things
// table indices, rooms room_info indices, tiles column + row * 128):
//   cmd type=<EV_*> value=<n> arg=<n>      a player's command (process_turn)
//   rand <function> <seed 8 hex>          a draw from the seed (old_newrand),
//                                          with the function that drew;
//                                          only when asked for
//   balance <TX_*> amount=<n> arg=<n>      change_balance, player 0's only;
//                                          arg 0 unless the type keeps it
//   rep <illness> event=<REP_*> amount=<n> reputation_affected, player 0's
//   peep <id> illness=<n> owner=<n> pos=<tile>  make_a_peep, at its end
//   remove_thing <id> type=<n>             remove_thing: a person leaves
//   start_going <id>                       start_person_going
//   set_complex <id> action=<n> arg=<n>    set_complex
//   join_queue <id> room=<n>               join_queue
//   leave_queue <id> room=<queue_room> dest=<dest_room>  leave_queue
//   record <id>                            record_person_as_patient
//   shall_pay <id> pays=<0 or 1>           shall_peep_pay, at its end
//   checkin <patient> receptionist=<id>    recept_admits_patient admits
//   pay <id> kind=<n> fee=<n> cash=<n>     pay_for_treatment
//   call <caller> id=<n> action=<n> result=<n>  call_person_to_room, at its end
//   go_home <id>                           person_go_home
//   force_cure <patient>                   force_peep_to_be_cured
//   diagnose <patient> staff=<id>          diagnose_patient
//   next_treatment <patient> staff=<id>    setup_next_treatment (staff 0: a
//                                          ward's bed)
//   done_treatment <patient> staff=<id>    done_treatment
//   treat <staff> patient=<id> room=<n>    doctor_waits_for_patient_to_arrive,
//                                          as the patient called has come
//                                          and the room's work starts
//   drugs <patient> staff=<id>             administer_drugs
//   cure <patient> illness=<n>             effect_illness curing
//   dies <patient>                         person_dies
//   dead <patient> substate=<n>            person_dead
//   summon_reaper <patient>                summon_the_reaper
//   page room=<n> mask=<hex> wanted=<n> result=<0 or 1>  page_member_of_staff,
//                                          at its end
//   check_pager <staff> result=<room>      check_pager, at its end, when it
//                                          finds a pager
//   pager_arrival <staff>                  pager_arrival
//   abandon_pager <staff>                  staff_abandon_pager_response
//   clear_pagers room=<n>                  clear_pager_responses
//   playercall <id> type=<n> subject=<n> owner=<n>  make_a_playercall, at its end
//   place_object <id> kind=<n> tile=<n> dir=<n> owner=<n>  place_object, as a
//                                          new object goes on the map
//   remove_object <id> owner=<n>           remove_object
//   place_staff <id> tile=<n>  place_staff_on_map, on entry
//   staff_picked_up <id>                   staff_picked_up
//   sack_staff <id> mode=<n>               sack_staff
//   handyman_values <id> litter=<n> plants=<n> repair=<n>  adjust_handyman_values,
//                                          the priorities as set, before the
//                                          job counts are reset
//   back_to_applicants <id>                process_things, a member of staff
//                                          in SS_BACK_TO_APPLICANTS
//   litter <tile> type=<n> owner=<n>       place_litter
//   unlitter <tile> owner=<n>              remove_litter
//   commit_blueprint / uncommit_blueprint / commit_doors / open_room /
//   delete_blueprint_room                  those of player 0's build tool
//   delete_room <room>                     delete_room
//   reception <room>                       allocate_reception_roominfo, at
//                                          its end
//   take_a_break <staff> room=<n> tired=<n>  take_a_break, once a staff room
//                                          is found (tiredness then)
//   rest_staff <staff> amount=<n> tired=<n>  rest_staff (amount before its
//                                          scaling, tiredness before)
//   staff_room <staff> pastime=<pool, video or sofa>  staff_in_staff_room,
//                                          as it starts one
//   rested <staff> discontent=<n> crazy=<n>  staff_in_staff_room, no longer
//                                          tired (before discontent halves)
//   room_move <id> from=<room> to=<room>   move_person_in_room_list (to: the
//                                          room of the cell, 0 for none)
//   use_object <id> obj=<n> kind=<n>       process_person_complex and
//                                          process_staff_complex, taking
//                                          their object (step action 6)
//   to_toilets <patient> room=<n> bladder=<n>  person_entertains_themself,
//                                          leaving its queue for the toilets
//   in_bogs <patient> substate=<n>         person_in_bogs
//   leave_toilets <patient>                leave_toilets
//   relieved <patient> bladder=<n>         complex_success of the toilet
//                                          (kind 6; bladder before)
//   wets <patient> bladder=<n>             do_reaction, the bladder
//                                          overflowing
//   policy_rest <n>                        process_turn's EV_SET_POLICY of
//                                          the rest threshold (value 4)
// Whoever ports one of these functions writes its event on both sides.
std::string trace_turn(const Game &game, const EventLog *events = nullptr);

} // namespace th

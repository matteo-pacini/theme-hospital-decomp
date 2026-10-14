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
    uint64_t limit = 500;
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
//   crit <0 win or 1 lose> <row 0-9> c=<criterion> now=<current>
//          target=<value> met=<0 or 1>      check_criteria_values, each of
//                                          player 0's rows, at its end
//   score <n>                              calculate_score, player 0's, after
//                                          the growth
//   winlose win group=<n> offer=<n>        check_game_over, player 0's win
//   winlose lose group=<n>                 check_game_over, player 0's lose
//   quarterly progress=<n> offered=<0 or 1>  check_quarterly_game_over,
//                                          player 0's, every call
//   bonus_level <n>                        try_bonus_level, the bonus level it
//                                          grants (0: none)
//   checkin <patient> receptionist=<id>    recept_admits_patient admits
//   pay <id> kind=<n> fee=<n> cash=<n>     pay_for_treatment
//   call <caller> id=<n> action=<n> result=<n>  call_person_to_room, at its end
//   call_person_again <caller> action=<n>  call_person_again, at its start
//   stop_waiting                           stop_waiting_for_patient, at its
//                                          start (a call dropped)
//   nav_failure <person>                   person_nav_failure, at its start
//   staff_nav_failure <staff>              staff_nav_failure, at its start
//   fail_complex <person>                  fail_person_complex, at its start
//   abandon_complex <person>               abandon_complex, at its start
//   return_to_queue_area <person>          return_to_queue_area, at its start
//   waits_for_busy <person>                person_waits_for_busy, at its start
//   go_home <id>                           person_go_home
//   force_cure <patient>                   force_peep_to_be_cured
//   diagnose <patient> staff=<id>          diagnose_patient
//   diagnose_icon <person> icon=<n>        diagnose_patient's set_icon_value:
//                                          the staff member's (ICON_DIAGNOSED,
//                                          ICON_GUESSED, ICON_NO_DIAGNOSIS) or
//                                          the patient's (ICON_AWAITING_DECISION)
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
//   place_cursor ok=<0 or 1> tile=<n> kind=<n> dir=<n>  update_placement, the
//                                          object cursor's placement check
//                                          and where it would go
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
//   rathole <tile> owner=<n> side=<n>      try_create_rathole, a hole added
//   rathole_remove <tile> owner=<n>        remove_rathole
//   rat <id> owner=<n> from=<tile> to=<tile>  make_a_rat, a rat sent out
//   award_staff_happiness sum=<n>          monthly_update_of_award_counts, the
//                                          staff happiness total the month's
//                                          award figure is made from
//   shoot_rat <rat> killed=<0 or 1> powerup=<n> shots=<n> kills=<n>
//                                          process_turn's EV_SHOOT_RAT (shots
//                                          and kills the year's award counts)
//   rat_game_over kills=<n> litter=<n>     rat_fest_control, the rat game
//                                          ending (the run ends this turn)
//   plant_thirsty <plant> priority=<n>     plant_ages, at a wilting stage
//   plant_dead <plant>                     plant_ages, once 40 past dead
//   watered <handyman> plant=<n> health=<n>  handyman_works, a plant watered,
//                                          with the condition it had
//   commit_blueprint / uncommit_blueprint / commit_doors / open_room /
//   delete_blueprint_room                  those of player 0's build tool
//   blueprint_invalid                      commit_blueprint, the room the
//                                          wrong size
//   blueprint_blocked                      commit_blueprint, the blueprint
//                                          cutting rooms off the entrance
//   door_invalid                           temp_place_door, a piece of the
//                                          door cannot go there
//   door_blocked                           place_bp_door, the door cutting
//                                          rooms off the entrance
//   blueprint x0=<n> y0=<n> x1=<n> y1=<n> drag=<n>  modify_blueprint, the
//                                          blueprint before the call
//   bp_goto <person>                       move_peep_from_blueprint, a
//                                          person sent off a blueprint
//                                          (PS_BP_GOTO or SS_BP_GOTO)
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
//   expire_call <id> type=<n> subject=<n>  expire_playercalls, a call
//                                          expiring unanswered (type 2: an
//                                          emergency offer)
//   advice_variant <text> seed=<hex>       find_advice, before it draws the
//                                          variant of advice text 0x6b, 0x7e or
//                                          0x80 from the presentation
//                                          generator (safeseed), with its
//                                          value before the draw; only once a
//                                          script has set it (@safeseed)
//   emergency_offer <call> illness=<n> count=<n> bonus=<n> win=<n>
//                                          setup_emergency_stats: an
//                                          emergency offered (bonus per
//                                          patient, win the percentage)
//   emergency_accept <call> illness=<n> count=<n> bonus=<n> win=<n>
//                                          give_emergency_stats, the offer
//                                          taken on (before its slot is freed)
//   emergency_refuse <call>                process_turn's CH_EMERGENCY_REFUSE
//                                          (the call last taken off the panel)
//   emergency_start                        process_turn's EV_START_EMERGENCY
//   epidemic_declare                       process_turn's CH_EPIDEMIC_DECLARE
//   epidemic_cover_up                      process_turn's CH_EPIDEMIC_COVER_UP
//   vaccine_cancel <person> requests=<n>   cancel_vaccination_request of a
//                                          person with a request or a nurse
//                                          sent, at its start (requests
//                                          before): a vaccination toggled
//                                          off, given, or the person removed
//   heli <id>                              try_to_generate_emergency, the
//                                          helicopter summoned (0: none free)
//   heli_landed <id>                       helicopter_lands, on landing
//   heli_gone <id>                         helicopter_leaves, at the top,
//                                          before it and its shadow go
//   emergency_peep <id>                    create_a_peep, an emergency patient
//                                          made (after its peep event)
//   emergency_timer <timer> state=<n>      twice_daily_admin, the emergency's
//                                          timer counted down (timer after,
//                                          state before)
//   emergency_over cured=<n> patients=<n> bonus=<n>  daily_admin, the
//                                          emergency over (bonus 0: failed)
//   emergency_cure <patient>               effect_illness, an emergency
//                                          patient cured
//   emergency_dies <patient>               modify_person, an emergency
//                                          patient losing its health as time
//                                          is up
//   emergency_leaves <patient>             person_go_for_treatment, an
//                                          emergency patient finding no room
//                                          of its treatment's kind (one that
//                                          will not pay: treatment 0's), so
//                                          going home
//   area <n>                               count_hospital_area for player 0,
//                                          at its end (month_end's and
//                                          calculate_value's)
//   price_level <n>                        calc_pricing_factor for player 0,
//                                          at month_end
//   choice <value> arg=<n> awaiting=<0 or 1>  process_turn's EV_CHOICE_RESULT
//                                          (a fax answered), awaiting 1 if
//                                          thing arg is a patient awaiting
//                                          a decision (PF_AWAITING_DECISION)
//   palette fax=<0 or 1> red=<n> black=<n> dark_blue=<n>  setup_colors, when the
//                                          colours change between the hospital's
//                                          palette and the fax picture's (the
//                                          fax shown, or closed)
//   counts patients=<n> impatience=<n> waiting=<n> happy=<n> refresh=<n>
//          warm=<n> bladder=<n> staff=<n> tired=<sum>,<per kind 0-3>
//          happy=<n> warm=<n> idle=<per kind 0-4> machines=<n>
//          wear=<sum>,<most> worst=<kind> crackups=<n> visitors=<n>
//                                          final_count, on the turns of a
//                                          statistics pass (adcounts: the
//                                          patients' sums, staff = doctors
//                                          and nurses, then the staff's sums;
//                                          visitors: this month's in the
//                                          graphs' history)
//   modify_machine <id> used=<n> strength=<n> smoke=<n>,<n>,<n>
//                                          modify_machine: a machine worn to
//                                          75% or more, as it was
//   machine_warning <id>                   modify_machine, its warning sound
//   machine_smoke <id> smoke=<n>,<n>,<n>   modify_machine, its smoke after
//                                          it, before it may break down
//   machine_disaster <id> kind=<n> room=<n>  machine_disaster
//   explosion tile=<n> anim=<n>            create_explosion
//   blast tile=<n> radius=<n>              blast_effect, the radius drawn
//   room_destroyed <room> mishaps=<n>      machine_disaster, the room
//                                          destroyed, before its queue is
//                                          sent on (mishaps hospital 0's)
//   disaster_roll owner=<n> kind=<n>       generate_disaster, the kind drawn
//                                          (0 none, 1 heating cold, 2 heating
//                                          hot, 3 sickness)
//   disaster_start owner=<n> kind=<n>      generate_disaster, a drawn
//                                          disaster starting
//   disaster_reject owner=<n> kind=<n>     generate_disaster, a drawn
//                                          disaster rejected, so not started
//   disaster_end owner=<n> kind=<n>        generate_disaster, a running
//                                          disaster ending
//   replace_machine <id> used=<n> strength=<n>  replace_machine
//   repair_request <id>                    process_turn's EV_THINGPAGE_REQUEST
//   repair <handyman> machine=<id>         handyman_works, the repair
//                                          started (machine 0: none left,
//                                          as after an explosion)
//   repaired <handyman> machine=<id> room=<n>  handyman_works, the repair
//                                          done, before the room opens
//   research_points <staff> amount=<n> points=<n>  research_contribution,
//                                          hospital 0's, before the points
//                                          are added (amount with the
//                                          researcher's ability, points
//                                          before)
//   research <category> illness=<n> amount=<n>  do_illness_research,
//                                          hospital 0's (the illness as
//                                          given, before a machine's is
//                                          found)
//   researched <object> illness=<n>        item_researched, hospital 0's
//   drug_researched <illness>              drug_researched, hospital 0's
//   drug_improved <illness> effect=<n> cost=<n>  drug_improved, hospital 0's,
//                                          the drug as it ends
//   upgraded <object> level=<n> strength=<n> cost=<n>  item_upgraded,
//                                          hospital 0's, the object as it
//                                          ends
//   available <object>                     auto_available, hospital 0's
//                                          object becoming available
//   category_exhausted <category>          research_category_exhausted,
//                                          hospital 0's
//   send_to_research <doctor> room=<n>     send_doctor_to_research, a room
//                                          found
//   send_for_research <patient> room=<n>   send_person_for_research, the room
//                                          get_expertise_dest picked
//   autopsied <patient> illness=<n> amount=<n> unhappy=<0|1>
//                                          person_autopsied: the patient's
//                                          illness and the points it gives,
//                                          and whether the staff's average
//                                          happiness is under 25, before
//                                          the draw
//   send_to_training <doctor> room=<n>     send_doctor_to_training, a room
//                                          found
//   training <consultant> kind=<n>         training_done, at its start
//   train_ability <student> ability=<n>    training_done, a student's ability
//                                          after its gain
//   promoted <student> skill=<n>           training_done, a student gaining a
//                                          skill
//   appraise <staff> skill=<n>             appraise_staff, a promotion (8 a
//                                          junior becoming a doctor, 128 a
//                                          doctor a consultant)
//   alter_research <category> value=<n>    alter_research (process_turn's
//                                          EV_ALTER_RESEARCH), hospital 0's
//   tutorial <step>                        tutorial (src/tutorial.cpp),
//                                          advisor.tutorial_step, at its
//                                          entry, while the tutorial runs
// The computer hospitals' (src/computer.cpp), owners 1-3, each tagged with
// its owner; "fires" gives the point in the original's source:
//   computer_action <owner> type=<n> arg=<n> count=<n> next=<turn>
//                                          computer_action, for each rival
//                                          that acts, after its pending
//                                          action ran (the action's fields
//                                          and next_action_turn as they
//                                          stand). Fires: after the switch,
//                                          computer.cpp:255, in the loop
//   computer_decision <owner> step=<n> type=<n>  computer_decision, after its
//                                          two draws: the com_strategy index
//                                          it takes and that entry's type.
//                                          Fires: before the switch,
//                                          computer.cpp:282
//   rival_open <owner>                     computer_decision, step type 10,
//                                          as the hospital opens. Fires:
//                                          computer.cpp:308, after
//                                          hospital_open is set, before
//                                          announce_this
//   computer_employ <owner> thing=<id> subtype=<n> balance=<n>
//                                          computer_employ, a hire: the
//                                          applicant's id and kind, the
//                                          balance after the pay, with the
//                                          applicant's Thing still there.
//                                          Fires: computer.cpp:564, after
//                                          inc_staff_stats, the pay, the wage
//                                          bill and the ability sum, before
//                                          set_expertise_staff, insert_skill
//                                          and remove_staff_in_struct (which
//                                          frees the Thing)
//   computer_build <owner> room=<n> balance=<n>  computer_build, a room built:
//                                          the balance after the room and its
//                                          compulsory objects (computer_place
//                                          _object events come first).
//                                          Fires: computer.cpp:600, before
//                                          return 1
//   computer_place_object <owner> obj=<n> balance=<n>  computer_place_object,
//                                          an object bought (or free).
//                                          Fires: computer.cpp:622, before
//                                          return 1
//   computer_loan <owner> amount=<n> balance=<n> loan=<n>  computer_loan,
//                                          after a change was made. Fires: on
//                                          either return 1, computer.cpp:641
//                                          and :646
//   computer_land <owner> area=<n> balance=<n>  computer_buy_land, after a
//                                          purchase. Fires: computer.cpp:1053,
//                                          before return 1
//   computer_note <owner> op=<make, remove or declass> type=<n> arg=<n> count=<n>
//                                          one event per call, as the
//                                          original's calls run:
//                                          make_a_note, the note stored
//                                          (none if all 8 slots are used;
//                                          fires computer.cpp:815);
//                                          remove_a_note, the note found, as
//                                          its type is cleared (none if
//                                          absent; fires computer.cpp:852);
//                                          declass_a_note, the note moved to
//                                          its new slot (fires
//                                          computer.cpp:874). A declass thus
//                                          shows `remove` for the note it
//                                          takes out (its own remove_a_note
//                                          call) and then `declass`;
//                                          find_a_note emits nothing
//   computer_peep <owner> illness=<n> result=<n> load=<n>
//                                          process_computer_peep, at its end:
//                                          the result as the visit ended and
//                                          the hospital's patient load.
//                                          Fires: computer.cpp:496, before
//                                          patients_done++
//   computer_bucket <owner> balance=<n> cures=<n> deaths=<n> rep=<n> load=<n>
//                                          get_computer_bucket, once the
//                                          bucket is merged and the
//                                          reputations evaluated, before it is
//                                          cleared (rep: the hospital's, not
//                                          a treatment's). Fires:
//                                          computer.cpp:528, before the memset
//   computer_month <owner> balance=<n> wage_bill=<n>  computer_month, per
//                                          rival, after its wages left the
//                                          balance (computer.cpp:180); after
//                                          that rival's computer_bucket event
//   computer_year <owner> value=<n> score=<n>  computer_year, per rival, after
//                                          calculate_value and calculate_score
//                                          (computer.cpp:140)
// The `rep`, `balance`, `score`, `crit`, `area` and `price_level` events stay
// player 0's: no rival path emits them.
// Whoever ports one of these functions writes its event on both sides.
std::string trace_turn(const Game &game, const EventLog *events = nullptr);

} // namespace th

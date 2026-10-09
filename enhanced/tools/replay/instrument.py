#!/usr/bin/env python3
"""Instrument a copy of the decompilation's sources for the replay check.

Usage: instrument.py TREE  (a copy of the repository holding src/; never the
repository itself). Edits TREE/src's units in place and prints the paths
(relative to TREE) of those it changed, to be compiled again.

game.cpp: game() writes TRACE.TXT, the trace of core/world/trace.h, once as
the level starts and at the end of every frame that ran a turn, with the
turn's events after its header line; quits after turn TRACE_TURNS
(environment, default 2000); carries out SCRIPT.TXT's commands (lines "turn
type value arg", decimal) between turn - 1 and turn, as th_replay --script
does; closes a help page as soon as one opens (as the player would; help
mode holds the turns), as one does when a room first opens; leaves out
process_mouse, so that the pointer, wherever DOSBox-X leaves it, sends no
commands (th_replay has no mouse); and runs without
the frame limit or the game speed's turn skipping, so that every frame is a
turn.

The events (E lines, listed in core/world/trace.h) are written by the
functions EVENTS names, each at the same point as the engine's port. With
TRACE_RAND=1 (environment) every draw from the seed is an event too: each
call of old_newrand and old_normal_rand outside rand.cpp names the function
it is in (trace_newrand, trace_normal_rand).

start.cpp: setup_insurances picks the first three insurance companies left
(ins_control 1, 2, 3, as this engine), not by the clock.

finance.cpp: daily_admin and twice_daily_admin note the seed after each
call that may draw from it ("# <call> done seed=...", which compare.py
ignores), and leave out the calls TRACE_SKIP names (environment, names
separated by commas), to compare past a function the engine does not port.
"""

import os
import re
import sys

TRACE = r'''
// Replay check instrumentation (build/replay only, not part of the decomp).
extern "C" char *getenv(const char *name);
#ifndef va_start
typedef char *va_list[1];
#define va_start(ap, last) ((ap)[0] = (char *)&(last) + sizeof(last))
#define va_end(ap) ((ap)[0] = 0)
#endif
extern "C" int vsprintf(char *dest, char *format, va_list arg);
static int trace_handle = -1;
static u32 trace_limit = 2000;
static char trace_buf[256];
static u32 trace_turn;

// The events since the last trace_dump, written after its header line.
static char trace_events[0x80000];
static u32 trace_events_len;
static int trace_events_lost;
static int trace_rand_on;
extern "C" const char *trace_rand_caller = "?";

static void trace_line(int len)
{
    LbFileWrite(trace_handle, trace_buf, len);
}

// Adds an event line "E <format...>" (core/world/trace.h).
extern "C" void trace_event(const char *format, ...)
{
    static char text[256];
    va_list args;
    int len, i;

    if (trace_handle == -1)
        return;
    va_start(args, format);
    len = vsprintf(text, (char *)format, args);
    va_end(args);
    if (trace_events_len + len + 3 >= sizeof(trace_events)) {
        trace_events_lost++;
        return;
    }
    trace_events[trace_events_len++] = 'E';
    trace_events[trace_events_len++] = ' ';
    for (i = 0; i < len; i++)
        trace_events[trace_events_len++] = text[i];

    trace_events[trace_events_len++] = '\n';
}

// A draw from the seed (old_newrand), by trace_rand_caller.
extern "C" void trace_rand(u32 value)
{
    if (trace_rand_on)
        trace_event("rand %s %08lx", trace_rand_caller, (unsigned long)value);
    trace_rand_caller = "?";
}

// old_newrand and old_normal_rand as called by `caller`.
extern "C" u32 trace_newrand(const char *caller)
{
    trace_rand_caller = caller;
    return old_newrand();
}

extern "C" u32 trace_normal_rand(const char *caller, u32 low, u32 high)
{
    trace_rand_caller = caller;
    return old_normal_rand(low, high);
}

static void trace_dump(void)
{
    Thing *t;
    int patients = 0;
    int len, h, i;
    long rep, price;

    if (trace_handle == -1)
        return;
    trace_turn = actual_game_turn;
    for (t = thing + 1; t < end_things; t++)
        if (t->type == 2)
            patients++;
    len = sprintf(trace_buf, "T %lu seed=%08lx date=%u/%u/%u balance=%ld patients=%d\n", (unsigned long)actual_game_turn,
            (unsigned long)seed, (unsigned)mycontrol->year, (unsigned)mycontrol->month, (unsigned)mycontrol->day,
            (long)allhosps[0].balance, patients);
    trace_line(len);
    if (trace_events_lost) {
        len = sprintf(trace_buf, "E lost %d\n", trace_events_lost);
        trace_line(len);
    }
    LbFileWrite(trace_handle, trace_events, trace_events_len);
    trace_events_len = 0;
    trace_events_lost = 0;
    for (h = 0; h < 4; h++) {
        rep = price = 0;
        for (i = 1; i < 46; i++) {
            rep += allhosps[h].rooms[i].reputation;
            price += allhosps[h].rooms[i].price;
        }
        len = sprintf(trace_buf, "H %d open=%u rep=%ld price=%ld\n", h, (unsigned)allhosps[h].hospital_open, rep, price);
        trace_line(len);
    }
    for (t = thing + 1; t < end_things; t++) {
        if (t->type == 0)
            continue;
        len = sprintf(trace_buf, " %d type=%u sub=%u state=%u x=%04x y=%04x\n", (int)(t - thing), (unsigned)t->type,
                (unsigned)t->subtype, (unsigned)t->state, (unsigned)t->x.raw, (unsigned)t->y.raw);
        trace_line(len);
    }
    if (actual_game_turn >= trace_limit)
        quit_game = 1;
}

// Marks a point within a turn with the seed there, for finding which call
// drew differently (a comment line: compare.py ignores it).
extern "C" void trace_note(const char *label)
{
    int len;

    if (trace_handle == -1)
        return;
    len = sprintf(trace_buf, "# %s seed=%08lx\n", label, (unsigned long)seed);
    trace_line(len);
}

// Whether TRACE_SKIP (names separated by commas) names the call `name`:
// the instrumented units leave out such calls, to compare past a function
// the engine does not port yet.
extern "C" int trace_skip(const char *name)
{
    const char *list = getenv("TRACE_SKIP");
    const char *p;
    const char *q;

    if (!list)
        return 0;
    for (p = list; *p; p++) {
        if (p != list && p[-1] != ',')
            continue;
        for (q = name; *q && *p == *q; p++, q++) {
        }
        if (*q == 0 && (*p == 0 || *p == ','))
            return 1;
        if (*p == 0)
            break;
    }
    return 0;
}

// The scripted commands (SCRIPT.TXT: lines "turn type value arg", decimal;
// other lines ignored; at most 256 commands in 16 KB), each carried out
// between turn - 1 and turn, as the replay tool does.
static u32 script[256][4];
static int script_count, script_next;
// The last cell a scripted "@cursor" moved to, as update_placement keeps it.
static u32 script_mouse_cell;

void draw_object_cursor(void);

static void script_load(void)
{
    static char text[16384];
    int handle, len, i, field;
    u32 v[4];
    char *p;

    handle = LbFileOpen("SCRIPT.TXT", 2);
    if (handle == -1)
        return;
    len = LbFileRead(handle, text, sizeof(text) - 1);
    LbFileClose(handle);
    if (len < 0)
        return;
    text[len] = 0;
    for (p = text; *p && script_count < 256;) {
        field = 0;
        while (*p == ' ' || *p == '\t')
            p++;
        while (field < 4 && *p >= '0' && *p <= '9') {
            v[field] = 0;
            while (*p >= '0' && *p <= '9')
                v[field] = v[field] * 10 + (*p++ - '0');
            field++;
            while (*p == ' ' || *p == '\t')
                p++;
        }
        if (field >= 1 && p[0] == '@' && p[1] == 'c' && p[2] == 'u' && p[3] == 'r' && p[4] == 's' &&
            p[5] == 'o' && p[6] == 'r') {
            // The port's "@cursor <cell>": the move its update_placement
            // makes, handed to the placement code below.
            p += 7;
            while (*p == ' ' || *p == '\t')
                p++;
            if (*p >= '0' && *p <= '9') {
                u32 cell = 0;
                while (*p >= '0' && *p <= '9')
                    cell = cell * 10 + (*p++ - '0');
                script[script_count][0] = v[0];
                script[script_count][1] = 0x1000;
                script[script_count][2] = cell;
                script[script_count][3] = 0;
                script_count++;
            }
        } else if (field >= 1 && p[0] == '@' && p[1] == 't' && p[2] == 'u' && p[3] == 't' && p[4] == 'o' &&
                   p[5] == 'r' && p[6] == 'i' && p[7] == 'a' && p[8] == 'l') {
            // The port's "@tutorial <0 or 1>": advisor.flags' tutorial bit,
            // set by the preferences screen, and its step's start (area.cpp).
            u32 on = 0;
            p += 9;
            while (*p == ' ' || *p == '\t')
                p++;
            while (*p >= '0' && *p <= '9')
                on = on * 10 + (*p++ - '0');
            script[script_count][0] = v[0];
            script[script_count][1] = 0x1001;
            script[script_count][2] = on ? 1 : 0;
            script[script_count][3] = 0;
            script_count++;
        } else if (field == 4) {
            for (i = 0; i < 4; i++)
                script[script_count][i] = v[i];
            script_count++;
        }
        while (*p && *p != '\n')
            p++;
        if (*p)
            p++;
    }
}

static void script_run(void)
{
    while (script_next < script_count && script[script_next][0] <= actual_game_turn + 1) {
        if (script[script_next][0] == actual_game_turn + 1) {
            // The port's update_placement keeps the cursor's cell between
            // commands; the redraw code would let the parked DOSBox pointer
            // overwrite it, so put it back first.
            othervar.prev_mouse_cell = (u16)script_mouse_cell;
            othervar.mouse_cell = (u16)script_mouse_cell;
            if (script[script_next][1] == 0x1000) {
                // "@cursor <cell>": the cursor move the port's update_placement
                // makes, without the rest of process_mouse.
                script_mouse_cell = script[script_next][2];
                othervar.mouse_cell = (u16)script_mouse_cell;
                if (gamevar.mouse_mode == MM_PLACE_OBJECT)
                    draw_object_cursor();
            } else if (script[script_next][1] == 0x1001) {
                // "@tutorial <0 or 1>": the preferences screen's flag, and
                // the area screen's step start (area.cpp).
                advisor.flags = (advisor.flags & ~1) | (u8)(script[script_next][2] ? 1 : 0);
                advisor.tutorial_step = 0x1d;
            } else {
                player_events[0].type = (u8)script[script_next][1];
                player_events[0].value = (u16)script[script_next][2];
                player_events[0].flags = 0;
                player_events[0].arg = (u16)script[script_next][3];
                // The port's fixed stand-in for the staff cursor's cell: a
                // member of staff held by a scripted command is placeable.
                if (player_events[0].type == EV_MARK_EMPLOYING)
                    gamevar.staff_place_ok = 1;
                trace_event("cmd type=%u value=%u arg=%u", (unsigned)player_events[0].type,
                            (unsigned)player_events[0].value, (unsigned)player_events[0].arg);
                process_turn();
            }
        }
        script_next++;
    }
}

static void trace_open(void)
{
    char *n = getenv("TRACE_TURNS");
    char *r = getenv("TRACE_RAND");

    if (n)
        trace_limit = atoi(n);
    trace_rand_on = r && *r == '1';
    script_load();
    trace_handle = LbFileOpen("TRACE.TXT", 0);
    trace_dump();
    script_run();
}
'''


GAME_EDITS = [
    ("// The game's main loop", TRACE + "\n// The game's main loop"),
    ("""    process_turn();
    while (quit_game == 0) {""", """    process_turn();
    trace_open();
    while (quit_game == 0) {"""),
    # No mouse: in a building mode the pointer's cell would send commands
    # every frame (EV_HOVER_BLUEPRINT, EV_TEMP_PLACE_DOOR at the pointer).
    ("""                process_mouse();\n""", ""),
    # The win or loss is detected in year_end, at the turn, so that its
    # events are in that turn's trace; do_year_end no longer repeats it (its
    # case 4 only restores the screen).
    ("""\n        check_game_over();\n""", "\n"),
    # The win/lose screens wait for a mouse click that headless never gives.
    # The decision is already in the turn (check_game_over in year_end);
    # leave the screen unplayed and end the run at that turn, so its trace is
    # written (do_year_end runs in the same frame, before trace_dump).
    ("""    case 5:
    case 6:
    case 7:
        play_winlose();
        break;""", """    case 5:
    case 6:
    case 7:
        quit_game = 1;
        break;"""),
    # The quarterly win screen (display_mode DM_YEAR_END_WINLOSE) would play
    # the win movie and wait for a mouse answer the headless run cannot give.
    # Leave it unplayed: the decline command (EV_DECLINE_WIN, process_turn)
    # restores display_mode and the game carries on; without one, the run
    # stays here, as the offer's screen does.
    ("""            case DM_YEAR_END_WINLOSE:
                lib_memcpy(&choice_backup, &year_end_backup, 10);
                play_winlose();
                process_turn();
                break;""", """            case DM_YEAR_END_WINLOSE:
                lib_memcpy(&choice_backup, &year_end_backup, 10);
                process_turn();
                break;"""),
    ("""        if (othervar.fade_palette) {""", """        if (gamevar.mouse_mode == MM_HELP)
            finish_composite_help();
        if (actual_game_turn != trace_turn) {
            trace_dump();
            script_run();
        }
        if (othervar.fade_palette) {"""),
    ("""    while ((now = LbTimerClock()) - gamecounts.last_frame_clock < 0x28) {""",
     """    now = LbTimerClock();
    while (0 && (now = LbTimerClock()) - gamecounts.last_frame_clock < 0x28) {"""),
    ("""    if (now - gamecounts.last_turn_clock < 1000 / myplayer.game_speed_s && othervar.turbo_key == 0 &&""",
     """    if (0 && now - gamecounts.last_turn_clock < 1000 / myplayer.game_speed_s && othervar.turbo_key == 0 &&"""),
    ("""done:
    AIL_end_sequence(GetSongHandle());""", """done:
    if (trace_handle != -1) {
        // "# end" marks a run that left the loop by itself (the last turn,
        // or a win or loss); a trace without it was cut short.
        trace_line(sprintf(trace_buf, "# end\\n"));
        LbFileClose(trace_handle);
    }
    AIL_end_sequence(GetSongHandle());"""),
]

START_EDITS = [
    ("""            pick = LbTimerClock() % left;""", """            pick = 0;"""),
]

FINANCE_EDITS = [
    ("void process_stat_panel(void);", """void process_stat_panel(void);
extern "C" void trace_note(const char *label);
extern "C" int trace_skip(const char *name);"""),
    ("""    mycontrol->day++;
    redraw_panel = 1;""", """    mycontrol->day++;
    redraw_panel = 1;
    trace_note("daily_admin");"""),
    ("""        if (epidemic_status(i))""", """        trace_note("twice_daily_admin");
        if (epidemic_status(i))"""),
    # The win or loss is checked here, at the turn (see GAME_EDITS), so that
    # its draw from the seed and events are in the turn's trace.
    ("""    check_awards_won();
    swap_year_end_packets();""", """    check_awards_won();
    check_game_over();
    swap_year_end_packets();"""),
]

# sim.cpp: the year's score, criteria, win/lose and quarterly events.
SIM_EDITS = [
    # The lose branch: keep the criterion group so the event can name it.
    ("""        } else if (check_criteria(1, player)) {
            year_end_type = 7;""", """        } else if ((pct = check_criteria(1, player)) != 0) {
            trace_event("winlose lose group=%d", (int)pct);
            year_end_type = 7;"""),
    # The quarterly check: every call says its progress, win offered or not.
    ("""        if (evaluate_criteria(player) < 100)
            return;""", """        pct = evaluate_criteria(player);
        trace_event("quarterly progress=%ld offered=%d", (long)pct, pct >= 100 ? 1 : 0);
        if (pct < 100)
            return;"""),
]

# The object cursor's placement check, mirroring the port's update_placement
# (core/world/objects.cpp): the same event at the point its recompute ends.
# The "no change" gotos stay silent, as the port's early returns do.
CURSOR_EVENT = (
    'trace_event("place_cursor ok=%u tile=%u kind=%u dir=%u", (unsigned)gamevar.place_ok, '
    '(unsigned)gamevar.place_tile, (unsigned)gamevar.place_kind, (unsigned)gamevar.place_dir);'
)
CURSOR_EDITS = [
    ("""    gamevar.place_ok = can_place_object(tile, kind, dir, player);
    if (!gamevar.place_ok)
        goto draw;""", f"""    gamevar.place_ok = can_place_object(tile, kind, dir, player);
    if (!gamevar.place_ok) {{
        {CURSOR_EVENT}
        goto draw;
    }}"""),
    ("""        gamevar.place_ok = can_place_object(tile2, kind2, dir2, player);
        if (!gamevar.place_ok)
            goto draw;""", f"""        gamevar.place_ok = can_place_object(tile2, kind2, dir2, player);
        if (!gamevar.place_ok) {{
            {CURSOR_EVENT}
            goto draw;
        }}"""),
    ("""        gamevar.place_ok = place_object_nav_check(&room_info[gamevar.place_room], tile, kind, dir, tile2, kind2, dir2);
    }
draw:""", f"""        gamevar.place_ok = place_object_nav_check(&room_info[gamevar.place_room], tile, kind, dir, tile2, kind2, dir2);
    }}
    {CURSOR_EVENT}
draw:"""),
]

# The calls of daily_admin and twice_daily_admin that may draw, with their
# indentation.
FINANCE_CALLS = [
    ("    ", call)
    for call in ["try_to_generate_emergency();", "generate_mayors();", "generate_quake();", "expire_playercalls();",
                 "manage_rats();", "computer_day();", "do_research();", "create_a_peep();", "rat_fest_control();",
                 "computer_action();"]
] + [("        ", "generate_disaster(i);")]

# The events (core/world/trace.h), per unit: (where, function, code), where
#   "entry"  puts code at the start of the function's body;
#   ("before", anchor)  puts it before `anchor` (unique in the function), at
#            the anchor's indentation;
#   "exit"   wraps the function: code runs after it, with its result in r.
ID = "(unsigned)({} - thing)"
EVENTS = {
    "finance.cpp": [
        (("before", "if (wages[i] != 0)"), "month_end", 'if (i == 0) trace_event("area %u", (unsigned)allhosps[0].area);'),
        (("before", "total += allhosps[i].month_visitors;"), "month_end",
         'if (i == 0) trace_event("price_level %d", (int)allhosps[0].price_level);'),
        (("before", "area_value = hosp->area * gbv.land_cost_per_tile;"), "calculate_value",
         'if (owner == 0) trace_event("area %u", (unsigned)hosp->area);'),
        (("before", "if (hosp->emergency_timer_s == 0 && hosp->emergency_state_s > 0)"), "twice_daily_admin",
         'trace_event("emergency_timer %d state=%d", (int)hosp->emergency_timer_s, (int)hosp->emergency_state_s);'),
        (("before", "allhosps[i].emergency_state = 1;"), "daily_admin",
         'trace_event("emergency_over cured=%u patients=%u bonus=%ld", (unsigned)allhosps[i].emergency_cured, '
         '(unsigned)allhosps[i].emergency_patients, (long)(allhosps[i].emergency_cured * 100 / '
         'allhosps[i].emergency_patients >= allhosps[i].emergency_win_pct ? '
         'allhosps[i].emergency_bonus * allhosps[i].emergency_cured : 0));'),
        ("entry", "change_balance",
         'if (owner == 0) trace_event("balance %u amount=%ld arg=%u", (unsigned)type, (long)amount, '
         '(type == TX_INSURANCE || type == TX_TREAT || type == TX_FINAL_TREAT || type == TX_CURE_TREATMENT || '
         'type == TX_ADVANCE) ? (unsigned)arg : 0u);'),
        ("entry", "pay_for_treatment",
         f'trace_event("pay %u kind=%u fee=%ld cash=%ld", {ID.format("patient")}, (unsigned)kind, '
         '(long)patient->fee, (long)patient->cash);'),
    ],
    "sim.cpp": [
        ("entry", "reputation_affected",
         'if (owner == 0) trace_event("rep %u event=%u amount=%ld", (unsigned)treatment, (unsigned)event, '
         '(long)amount);'),
        ("exit", "shall_peep_pay", f'trace_event("shall_pay %u pays=%d", {ID.format("patient")}, r ? 1 : 0);'),
        (("before", "return max;"), "check_criteria_values",
         'if (owner == 0) { int ti; for (ti = 0; ti < 10; ti++) trace_event("crit %u %d c=%u now=%ld target=%ld '
         'met=%u", (unsigned)mode, ti, (unsigned)table[ti].criterion, (long)table[ti].current, '
         '(long)table[ti].value, (unsigned)table[ti].met); }'),
        (("before", "if (hosp->score > 200000000)"), "calculate_score",
         'if (owner == 0) trace_event("score %ld", (long)hosp->score);'),
        (("before", "year_end_type = 5;"), "check_game_over",
         'trace_event("winlose win group=%u offer=%ld", (unsigned)othervar.best_win_group, '
         '(long)othervar.next_level_offer);'),
    ],
    "queue.cpp": [
        ("entry", "join_queue",
         f'trace_event("join_queue %u room=%u", {ID.format("person")}, (unsigned)(room - room_info));'),
        ("entry", "leave_queue",
         f'trace_event("leave_queue %u room=%u dest=%u", {ID.format("person")}, (unsigned)person->queue_room, '
         '(unsigned)person->dest_room);'),
    ],
    "treat.cpp": [
        (("before", "treatment = 0;\n        }\n    } else if"), "person_go_for_treatment",
         f'trace_event("emergency_leaves %u", {ID.format("patient")});'),
        ("entry", "record_person_as_patient", f'trace_event("record %u", {ID.format("patient")});'),
        ("entry", "diagnose_patient",
         f'trace_event("diagnose %u staff=%u", {ID.format("patient")}, {ID.format("staff")});'),
        # A ward's bed gives no member of staff (a null pointer): 0.
        ("entry", "setup_next_treatment",
         f'trace_event("next_treatment %u staff=%u", {ID.format("patient")}, '
         f'staff ? {ID.format("staff")} : 0u);'),
        ("entry", "done_treatment",
         f'trace_event("done_treatment %u staff=%u", {ID.format("patient")}, {ID.format("staff")});'),
        ("entry", "person_go_for_treatment",
         f'trace_event("go_for_treatment %u treatment=%u", {ID.format("patient")}, (unsigned)patient->treatment);'),
        ("entry", "person_dies", f'trace_event("dies %u", {ID.format("patient")});'),
        ("entry", "person_dead",
         f'trace_event("dead %u substate=%u", {ID.format("patient")}, (unsigned)patient->substate);'),
        ("entry", "administer_drugs",
         f'trace_event("drugs %u staff=%u", {ID.format("patient")}, {ID.format("staff")});'),
    ],
    "examine.cpp": [
        (("before", "allhosps[patient->owner].emergency_cured++;"), "effect_illness",
         f'trace_event("emergency_cure %u", {ID.format("patient")});'),
        ("entry", "effect_illness",
         f'if (cured == 1) trace_event("cure %u illness=%u", {ID.format("patient")}, (unsigned)patient->illness);'),
    ],
    # Not an event: a comment line (compare.py ignores it), as th_replay
    # --advisor writes it.
    "advisor.cpp": [
        ("exit", "find_advice",
         'if (r == 1) { static char note[24]; sprintf(note, "advice %u", (unsigned)advice[id].id); '
         'trace_note(note); }'),
    ],
    "reaper.cpp": [
        ("entry", "summon_the_reaper", f'trace_event("summon_reaper %u", {ID.format("victim")});'),
    ],
    "doctor.cpp": [
        (("before", "patient->resume_state = 0;"), "doctor_waits_for_patient_to_arrive",
         f'trace_event("treat %u patient=%u room=%u", {ID.format("doctor")}, {ID.format("patient")}, '
         '(unsigned)(doctor->room - room_info));'),
        ("entry", "stop_waiting_for_patient", 'trace_event("stop_waiting");'),
    ],
    "peepmake.cpp": [
        (("before", "patient->known_illness = patient->illness;"), "create_a_peep",
         f'trace_event("emergency_peep %u", {ID.format("patient")});'),
        # Not a wrapper ("exit"): that would move make_a_peep's frame, and
        # with it the stack slot process_computer_peep's has_drinks reads
        # uninitialised (the same slot as make_a_peep's nchoices).
        (("before", "return peep;"), "make_a_peep",
         'trace_event("peep %u illness=%u owner=%u pos=%u", (unsigned)(peep - thing), (unsigned)illness, '
         '(unsigned)owner, (unsigned)pos);'),
    ],
    "utils.cpp": [
        (("before", "color.salmon = LbPaletteFindColour(current_palette, 0x3f, 0x1e, 0x1e);"), "setup_colors",
         # Only the hospital's palette (red 12) and the fax picture's (243): the
         # year-end screens' palettes are other ones.
         'static int fax_palette = 0; if ((color.red == 12 || color.red == 243) && (color.red == 243) != fax_palette) { '
         'fax_palette = color.red == 243; trace_event("palette fax=%d red=%d black=%d dark_blue=%d", fax_palette, '
         '(int)color.red, (int)color.black, (int)color.dark_blue); }'),
        ("entry", "remove_thing",
         f'trace_event("remove_thing %u type=%u", {ID.format("target")}, (unsigned)target->type);'),
    ],
    "peeps.cpp": [
        (("before", "owner2 = call->owner;"), "expire_playercalls",
         'trace_event("expire_call %u type=%u subject=%u", (unsigned)call->id, (unsigned)call->type, '
         '(unsigned)call->subject);'),
        ("entry", "start_person_going", f'trace_event("start_going %u", {ID.format("person")});'),
        ("entry", "person_go_home",
         f'trace_event("go_home %u state=%u", {ID.format("person")}, (unsigned)person->state);'),
        ("entry", "person_nav_failure",
         f'trace_event("nav_failure %u", {ID.format("person")});'),
        ("entry", "return_to_queue_area",
         f'trace_event("return_to_queue_area %u", {ID.format("person")});'),
        ("entry", "person_waits_for_busy",
         f'trace_event("waits_for_busy %u", {ID.format("person")});'),
        ("entry", "person_arrives_at_door", f'trace_event("at_door %u", {ID.format("person")});'),
        ("entry", "call_person_to_room",
         f'trace_event("call %u id=%u action=%u arg=%u", {ID.format("caller")}, (unsigned)id, (unsigned)action, '
         '(unsigned)arg);'),
        ("entry", "call_person_again",
         'trace_event("call_person_again %u action=%u", (unsigned)(caller - thing), (unsigned)action);'),
        ("exit", "make_a_playercall",
         'if (r) trace_event("playercall %u type=%u subject=%u owner=%u", (unsigned)r, (unsigned)type, '
         '(unsigned)subject, (unsigned)owner);'),
        ("entry", "get_playercall",
         'trace_event("get_playercall %u type=%u owner=%u", (unsigned)id, (unsigned)type, (unsigned)owner);'),
        ("entry", "force_peep_to_be_cured", f'trace_event("force_cure %u", {ID.format("person")});'),
        (("before", "leave_queue(person);"), "person_entertains_themself",
         f'trace_event("to_toilets %u room=%u bladder=%d", {ID.format("person")}, (unsigned)(room - room_info), '
         '(int)person->bladder);'),
        ("entry", "person_in_bogs",
         f'trace_event("in_bogs %u substate=%u", {ID.format("person")}, (unsigned)person->substate);'),
        ("entry", "leave_toilets", f'trace_event("leave_toilets %u", {ID.format("person")});'),
        ("entry", "move_person_in_room_list",
         f'trace_event("room_move %u from=%u to=%u", {ID.format("person")}, '
         'person->room ? (unsigned)(person->room - room_info) : 0u, '
         'which_room(tile) ? (unsigned)(which_room(tile) - room_info) : 0u);'),
    ],
    "environ.cpp": [
        (("before", "change_health(person, -1000);"), "modify_person",
         f'trace_event("emergency_dies %u", {ID.format("person")});'),
        (("before", "reset_icon_value(peep, ICON_TOILET);"), "do_reaction",
         f'trace_event("wets %u bladder=%d", {ID.format("peep")}, (int)peep->bladder);'),
        ("entry", "final_count",
         'if (othervar.every_32_turns) trace_event("counts patients=%u impatience=%u waiting=%ld happy=%d '
         'refresh=%d warm=%d bladder=%d staff=%u tired=%d,%d,%d,%d,%d happy=%u warm=%d idle=%u,%u,%u,%u,%u '
         'machines=%u wear=%ld,%ld worst=%u crackups=%u visitors=%ld", (unsigned)adcounts.patients, '
         '(unsigned)adcounts.impatience_sum, (long)adcounts.waiting_sum, (int)adcounts.happiness_sum, '
         '(int)adcounts.refreshment_sum, (int)adcounts.warmth_sum, (int)adcounts.bladder_sum, '
         '(unsigned)adcounts.doctors_and_nurses, (int)adcounts.staff_tiredness_sum, '
         '(int)adcounts.staff_tiredness_by_kind[0], (int)adcounts.staff_tiredness_by_kind[1], '
         '(int)adcounts.staff_tiredness_by_kind[2], (int)adcounts.staff_tiredness_by_kind[3], '
         '(unsigned)adcounts.staff_happiness_sum, (int)adcounts.staff_warmth_sum, (unsigned)adcounts.idle_staff[0], '
         '(unsigned)adcounts.idle_staff[1], (unsigned)adcounts.idle_staff[2], (unsigned)adcounts.idle_staff[3], '
         '(unsigned)((u8 *)&adcounts)[0x9d], (unsigned)adcounts.machines, (long)adcounts.machine_wear_sum, '
         '(long)adcounts.max_machine_wear, (unsigned)adcounts.worst_machine, (unsigned)adcounts.crackups, '
         '(long)mycontrol->history[4][0][11]);'),
        (("before", "employee->skills &= ~8;"), "appraise_staff",
         'trace_event("appraise %u skill=%u", (unsigned)(employee - thing), 8);'),
        (("before", "employee->skills |= SKILL_CONSULTANT;"), "appraise_staff",
         'trace_event("appraise %u skill=%u", (unsigned)(employee - thing), 128);'),
    ],
    "io.cpp": [
        # The win/lose screen's decline button, carried as EV_DECLINE_WIN (0x8f)
        # by the replay: the offered win is refused and the game carries on.
        (("before", "case EV_NONE:\n            break;"), "process_turn",
         'case 0x8f:\n            gamevar.win_declined_count++;\n            othervar.win_declined = 1;\n'
         '            othervar.bonus_level = 0;\n            display_mode = DM_GAME;\n            break;'),
        (("before", "if (statvar.num_players > 1) {\n                    restore_popup(2);"), "process_turn",
         'trace_event("emergency_refuse %u", (unsigned)popup_store[player][2].id);'),
        (("before", "allhosps[i].emergency_state = 10;"), "process_turn", 'trace_event("emergency_start");'),
        (("before", "u8 quiet = 0;"), "process_turn",
         'trace_event("choice %u arg=%u awaiting=%u", (unsigned)player_events[i].value, '
         '(unsigned)player_events[i].arg, (unsigned)(thing[player_events[i].arg].type == THING_PATIENT && '
         '(thing[player_events[i].arg].flags0 & PF_AWAITING_DECISION) ? 1 : 0));'),
        (("before", "allhosps[i].rest_threshold = player_events[i].arg;"), "process_turn",
         'trace_event("policy_rest %u", (unsigned)player_events[i].arg);'),
        (("before", "thingpage_request(&allhosps[i].repair_pager, player_events[i].value, 0xff);"), "process_turn",
         'trace_event("repair_request %u", (unsigned)player_events[i].value);'),
        (("before", "count_and_flag_contagious_peeps(allhosps[i].epidemic_illness, 2, i);"), "process_turn",
         'trace_event("epidemic_declare");'),
        (("before", "count_and_flag_contagious_peeps(allhosps[i].epidemic_illness, 1, i);"), "process_turn",
         'trace_event("epidemic_cover_up");'),
        (("before", "if (player_events[i].argb[1]) {"), "process_turn",
         'trace_event("shoot_rat %u killed=%u powerup=%u shots=%u kills=%u", (unsigned)(rat - thing), '
         '(unsigned)player_events[i].argb[1], (unsigned)player_events[i].argb[0], '
         '(unsigned)award_counts.rat_shots, (unsigned)award_counts.rats_killed);'),
    ],
    "req4.cpp": [
        (("before", "handyman->litter_count = 0;"), "adjust_handyman_values",
         'trace_event("handyman_values %u litter=%u plants=%u repair=%u", (unsigned)id, '
         '(unsigned)handyman->prio_litter, (unsigned)handyman->prio_plants, (unsigned)handyman->prio_repair);'),
    ],
    "complex.cpp": [
        ("entry", "set_complex",
         f'trace_event("set_complex %u action=%u arg=%u", {ID.format("w")}, (unsigned)action, (unsigned)arg);'),
        ("entry", "fail_person_complex", f'trace_event("fail_complex %u", {ID.format("person")});'),
        ("entry", "abandon_complex", f'trace_event("abandon_complex %u", {ID.format("w")});'),
        (("before", "obj->user = employee - thing;"), "process_staff_complex",
         f'trace_event("use_object %u obj=%u kind=%u", {ID.format("employee")}, (unsigned)employee->object, '
         '(unsigned)obj->subtype);'),
        (("before", "obj->user = person - thing;"), "process_person_complex",
         f'trace_event("use_object %u obj=%u kind=%u", {ID.format("person")}, (unsigned)person->object, '
         '(unsigned)obj->subtype);'),
        (("before", "reset_icon_value(w, ICON_TOILET);"), "complex_success",
         f'trace_event("relieved %u bladder=%d", {ID.format("w")}, (int)w->bladder);'),
    ],
    "recept.cpp": [
        ("entry", "recept_admits_patient",
         'if (receptionist->slave) trace_event("checkin %u receptionist=%u", (unsigned)receptionist->slave, '
         f'{ID.format("receptionist")});'),
    ],
    "skills.cpp": [
        ("entry", "page_member_of_staff",
         'trace_event("page room=%u mask=%x wanted=%d", (unsigned)room_id, (unsigned)skill_mask, (int)wanted);'),
        ("exit", "check_pager",
         f'if (r) trace_event("check_pager %u room=%u", {ID.format("staff")}, (unsigned)r);'),
        ("entry", "pager_arrival", f'trace_event("pager_arrival %u", {ID.format("staff")});'),
        ("entry", "clear_pager_responses", 'trace_event("clear_pagers room=%u", (unsigned)room_id);'),
        ("entry", "staff_abandon_pager_response", f'trace_event("abandon_pager %u", {ID.format("staff")});'),
    ],
    "objects.cpp": [
        (("before", "place_existing_object(obj, tile, dir);\n        return obj - thing;\n    }\n    return 0;"),
         "place_object",
         'trace_event("place_object %u kind=%u tile=%u dir=%u owner=%u", (unsigned)(obj - thing), (unsigned)kind, '
         '(unsigned)tile, (unsigned)dir, (unsigned)owner);'),
        ("entry", "remove_object",
         f'trace_event("remove_object %u owner=%u", {ID.format("obj")}, (unsigned)owner);'),
        ("entry", "modify_machine",
         f'trace_event("modify_machine %u used=%ld strength=%ld smoke=%u,%u,%u", {ID.format("machine")}, '
         '(long)machine->times_used, (long)machine->strength, (unsigned)machine->smoke[0], '
         '(unsigned)machine->smoke[1], (unsigned)machine->smoke[2]);'),
        (("before", "play_sound(SNDSRC_GENERAL, SFX_MACHWARN, -1, -1);"), "modify_machine",
         f'trace_event("machine_warning %u", {ID.format("machine")});'),
        (("before", "if (machine->times_used >= machine->strength) {"), "modify_machine",
         f'trace_event("machine_smoke %u smoke=%u,%u,%u", {ID.format("machine")}, (unsigned)machine->smoke[0], '
         '(unsigned)machine->smoke[1], (unsigned)machine->smoke[2]);'),
        ("entry", "replace_machine",
         f'trace_event("replace_machine %u used=%ld strength=%ld", {ID.format("machine")}, '
         '(long)machine->times_used, (long)machine->strength);'),
    ],
    "staff.cpp": [
        ("entry", "place_staff_on_map", 'trace_event("place_staff %u tile=%u", (unsigned)id, (unsigned)tile);'),
        (("before", "if (staff->subtype == STAFF_RECEPTIONIST) {"), "take_a_break",
         f'trace_event("take_a_break %u room=%u tired=%d", {ID.format("staff")}, (unsigned)(room - room_info), '
         '(int)staff->tiredness);'),
        ("entry", "rest_staff",
         f'trace_event("rest_staff %u amount=%d tired=%d", {ID.format("staff")}, (int)amount, (int)staff->tiredness);'),
        (("before", "set_complex(staff, 0x2ee, 0);"), "staff_in_staff_room",
         f'trace_event("staff_room %u pastime=pool", {ID.format("staff")});'),
        (("before", "set_complex(staff, 0x2f8, 0);"), "staff_in_staff_room",
         f'trace_event("staff_room %u pastime=video", {ID.format("staff")});'),
        (("before", "set_complex(staff, 0x2da, 0);"), "staff_in_staff_room",
         f'trace_event("staff_room %u pastime=sofa", {ID.format("staff")});'),
        (("before", "staff->discontent = (s16)staff->discontent / 2;"), "staff_in_staff_room",
         f'trace_event("rested %u discontent=%u crazy=%u", {ID.format("staff")}, (unsigned)staff->discontent, '
         '(unsigned)staff->crazy);'),
        ("entry", "staff_picked_up", f'trace_event("staff_picked_up %u", {ID.format("staff")});'),
        ("entry", "sack_staff", f'trace_event("sack_staff %u mode=%u", {ID.format("staff")}, (unsigned)mode);'),
        ("entry", "staff_nav_failure",
         f'trace_event("staff_nav_failure %u", {ID.format("staff")});'),
    ],
    "things.cpp": [
        (("before", "remove_node(cur);\n                cur->state = SS_APPLICANT;"), "process_things",
         f'trace_event("back_to_applicants %u", {ID.format("cur")});'),
    ],
    "handyman.cpp": [
        ("entry", "place_litter",
         'trace_event("litter %u type=%u owner=%u", (unsigned)tile, (unsigned)type, (unsigned)(u8)owner);'),
        ("entry", "remove_litter", 'trace_event("unlitter %u owner=%u", (unsigned)tile, (unsigned)owner);'),
        (("before", "seq = objects[thing[handyman->object].subtype].repair_aniseq;"), "handyman_works",
         f'trace_event("repair %u machine=%u", {ID.format("handyman")}, (unsigned)handyman->object);'),
        (("before", "handyman->room->state = 0;"), "handyman_works",
         f'trace_event("repaired %u machine=%u room=%u", {ID.format("handyman")}, (unsigned)handyman->object, '
         '(unsigned)(handyman->room - room_info));'),
        (("before", "NRAdcounts.thirsty_plants++;"), "plant_ages",
         f'trace_event("plant_thirsty %u priority=%u", {ID.format("plant")}, (unsigned)stage);'),
        (("before", "NRAdcounts.thirsty_plants += 2;"), "plant_ages",
         f'trace_event("plant_dead %u", {ID.format("plant")});'),
        (("before", "plant->state = OS_PLANT_AGES;"), "handyman_works",
         f'trace_event("watered %u plant=%u health=%d", {ID.format("handyman")}, {ID.format("plant")}, '
         '(int)plant->plant_health);'),
    ],
    "epidemic.cpp": [
        ("entry", "spread_epidemic", f'trace_event("epidemic %u", {ID.format("carrier")});'),
    ],
    "disaster.cpp": [
        ("entry", "machine_disaster",
         f'trace_event("machine_disaster %u kind=%u room=%u", {ID.format("machine")}, (unsigned)machine->subtype, '
         '(unsigned)machine->room_id);'),
        (("before", "restart_entire_queue(room);"), "machine_disaster",
         'trace_event("room_destroyed %u mishaps=%u", (unsigned)(room - room_info), (unsigned)allhosps[0].mishaps);'),
        ("entry", "create_explosion", 'trace_event("explosion tile=%u anim=%u", (unsigned)tile, (unsigned)anim);'),
        (("before", "cx = tile % 128;"), "blast_effect",
         'trace_event("blast tile=%u radius=%d", (unsigned)tile, (int)radius);'),
        # generate_disaster: the kind drawn, the disasters started, rejected
        # and ended, at the points the port traces them (core/world/disaster.cpp).
        (("before", "            switch (hosp->disaster_kind) {"), "generate_disaster",
         'trace_event("disaster_roll owner=%u kind=%u", (unsigned)owner, (unsigned)hosp->disaster_kind);'),
        (("before", "advisor.forced_advice_id = 0xb;"), "generate_disaster",
         'trace_event("disaster_start owner=%u kind=%u", (unsigned)owner, (unsigned)hosp->disaster_kind);'),
        (("before", "advisor.forced_advice_id = 0xa;"), "generate_disaster",
         'trace_event("disaster_start owner=%u kind=%u", (unsigned)owner, (unsigned)hosp->disaster_kind);'),
        (("before", "advisor.forced_advice_id = 0xd;"), "generate_disaster",
         'trace_event("disaster_start owner=%u kind=%u", (unsigned)owner, (unsigned)hosp->disaster_kind);'),
        (("before", "hosp->disaster_kind = 0;\n                }\n                break;\n            case 2:"),
         "generate_disaster",
         'trace_event("disaster_reject owner=%u kind=%u", (unsigned)owner, (unsigned)hosp->disaster_kind);'),
        (("before", "hosp->disaster_kind = 0;\n                }\n                break;\n            case 3:"),
         "generate_disaster",
         'trace_event("disaster_reject owner=%u kind=%u", (unsigned)owner, (unsigned)hosp->disaster_kind);'),
        (("before", "hosp->disaster_kind = 0;\n                }\n                break;\n            }\n        }\n"
                    "        break;\n    case 1:"),
         "generate_disaster",
         'trace_event("disaster_reject owner=%u kind=%u", (unsigned)owner, (unsigned)hosp->disaster_kind);'),
        (("before", "hosp->disaster_kind = 0;\n            hosp->heating_level = hosp->saved_heating;"),
         "generate_disaster",
         'trace_event("disaster_end owner=%u kind=%u", (unsigned)owner, (unsigned)hosp->disaster_kind);'),
        (("before", "hosp->disaster_kind = 0;\n            advisor.forced_advice_id = 0xe;"), "generate_disaster",
         'trace_event("disaster_end owner=%u kind=%u", (unsigned)owner, (unsigned)hosp->disaster_kind);'),
    ],
    "blueprnt.cpp": [
        ("entry", "commit_blueprint", 'if (owner == 0) trace_event("commit_blueprint");'),
        (("before", "advisor.forced_advice_id = 4;"), "commit_blueprint",
         'if (owner == 0) trace_event("blueprint_invalid");'),
        (("before", "advisor.forced_advice_id = 5;"), "commit_blueprint",
         'if (owner == 0) trace_event("blueprint_blocked");'),
        ("entry", "modify_blueprint",
         'if (owner == 0) trace_event("blueprint x0=%u y0=%u x1=%u y1=%u drag=%u", '
         '(unsigned)allhosps[owner].build.x0, (unsigned)allhosps[owner].build.y0, '
         '(unsigned)allhosps[owner].build.x1, (unsigned)allhosps[owner].build.y1, '
         '(unsigned)allhosps[owner].build.drag);'),
        ("entry", "uncommit_blueprint", 'if (owner == 0) trace_event("uncommit_blueprint");'),
        ("entry", "commit_doors", 'if (owner == 0) trace_event("commit_doors");'),
        ("entry", "delete_blueprint_room", 'if (owner == 0) trace_event("delete_blueprint_room");'),
        (("before", "return 1;"), "move_peep_from_blueprint",
         'trace_event("bp_goto %u", (unsigned)(peep - thing));'),
        ("exit", "allocate_reception_roominfo",
         'if (r) trace_event("reception %u", (unsigned)(r - room_info));'),
    ],
    "emerg.cpp": [
        ("entry", "setup_emergency_stats",
         'trace_event("emergency_offer %u illness=%u count=%u bonus=%u win=%u", (unsigned)call, '
         '(unsigned)gamevar.emergency_illness, (unsigned)gamevar.emergency_patients, '
         '(unsigned)gamevar.emergency_bonus, (unsigned)gamevar.emergency_win_percent);'),
        (("before", "gamevar.emergency_calls[i].used = 0;"), "give_emergency_stats",
         'trace_event("emergency_accept %u illness=%u count=%u bonus=%u win=%u", (unsigned)call, '
         '(unsigned)hosp->emergency_illness, (unsigned)hosp->emergency_patients, (unsigned)hosp->emergency_bonus, '
         '(unsigned)hosp->emergency_win_pct);'),
        (("before", "if (hosp->emergency_heli) {"), "try_to_generate_emergency",
         'trace_event("heli %u", (unsigned)hosp->emergency_heli);'),
        (("before", "allhosps[heli->owner].heli_landed = 1;"), "helicopter_lands",
         f'trace_event("heli_landed %u", {ID.format("heli")});'),
        (("before", "hosp->heli_landed = 0;"), "helicopter_leaves",
         f'trace_event("heli_gone %u", {ID.format("heli")});'),
    ],
    "build.cpp": [
        ("entry", "open_room", 'if (owner == 0) trace_event("open_room");'),
        ("entry", "delete_room", 'trace_event("delete_room %u", (unsigned)(room - room_info));'),
        (("before", "othervar.door_invalid[owner] = 1;"), "temp_place_door",
         'if (owner == 0) trace_event("door_invalid");'),
        (("before", "advisor.forced_advice_id = 6;"), "place_bp_door",
         'if (owner == 0) trace_event("door_blocked");'),
    ],
    "research.cpp": [
        (("before", "allhosps[researcher->owner].research_points += amount;"), "research_contribution",
         f'if (researcher->owner == 0) trace_event("research_points %u amount=%d points=%ld", '
         f'{ID.format("researcher")}, (int)amount, (long)allhosps[0].research_points);'),
        ("entry", "do_illness_research",
         'if (owner == 0) trace_event("research %u illness=%u amount=%lu", (unsigned)category, (unsigned)illness, '
         '(unsigned long)amount);'),
        ("entry", "item_researched",
         'if (owner == 0) trace_event("researched %u illness=%u", (unsigned)item, (unsigned)illness);'),
        ("entry", "drug_researched", 'if (owner == 0) trace_event("drug_researched %u", (unsigned)illness);'),
        (("before", "if (owner == player && done)"), "drug_improved",
         'if (owner == 0) trace_event("drug_improved %u effect=%u cost=%u", (unsigned)illness, '
         '(unsigned)hosp->rooms[illness].drug.effectiveness, (unsigned)hosp->rooms[illness].drug.cost);'),
        (("before", "if (owner == player)\n        announce_this(ANN_OBJECT_IMPROVED"), "item_upgraded",
         'if (owner == 0) trace_event("upgraded %u level=%u strength=%u cost=%ld", (unsigned)item, '
         '(unsigned)hosp->equipment[item].upgrade_level[0], (unsigned)hosp->object_stats[item].strength, '
         '(long)hosp->object_stats[item].cost);'),
        (("before", "allhosps[owner].equipment[item].available = 1;"), "auto_available",
         'if (owner == 0) trace_event("available %u", (unsigned)item);'),
        ("entry", "research_category_exhausted",
         'if (owner == 0) trace_event("category_exhausted %u", (unsigned)category);'),
        (("before", "staff_exit_room_check(doctor);"), "send_doctor_to_research",
         f'trace_event("send_to_research %u room=%u", {ID.format("doctor")}, (unsigned)id);'),
        (("before", "room->queue_incoming++;"), "send_person_for_research",
         f'trace_event("send_for_research %u room=%u", {ID.format("patient")}, (unsigned)(room - room_info));'),
        (("before", "do_illness_research(patient->owner, patient->illness,"), "person_autopsied",
         f'trace_event("autopsied %u illness=%u amount=%u unhappy=%d", {ID.format("patient")}, '
         '(unsigned)patient->illness, (unsigned)(gbv.autopsy_rsch_percent * expertise[patient->illness].cost / 100), '
         '(int)((staff_happiness << 8) / hosp->staff_count < 0x1900));'),
    ],
    "rschscrn.cpp": [
        ("entry", "alter_research",
         'if (owner == 0) trace_event("alter_research %u value=%u", (unsigned)cat, (unsigned)value);'),
    ],
    "training.cpp": [
        (("before", "staff_exit_room_check(doctor);"), "send_doctor_to_training",
         'trace_event("send_to_training %u room=%u", (unsigned)(doctor - thing), (unsigned)room);'),
        ("entry", "training_done",
         'trace_event("training %u kind=%u", (unsigned)(consultant - thing), (unsigned)kind);'),
        (("before", "points = base / (students * count);"), "training_done",
         'trace_event("train_ability %u ability=%u", (unsigned)(student - thing), (unsigned)student->ability);'),
        (("before", "student->skills |= SKILL_SURGEON;"), "training_done",
         'trace_event("promoted %u skill=%u", (unsigned)(student - thing), 32);'),
        (("before", "student->skills |= SKILL_PSYCHIATRIST;"), "training_done",
         'trace_event("promoted %u skill=%u", (unsigned)(student - thing), 64);'),
        (("before", "student->skills |= SKILL_RESEARCHER;"), "training_done",
         'trace_event("promoted %u skill=%u", (unsigned)(student - thing), 256);'),
    ],
    "rats.cpp": [
        (("before", "    if (side == 0) {"), "try_create_rathole",
         'trace_event("rathole %u owner=%u side=%u", (unsigned)tile, (unsigned)owner, (unsigned)side);'),
        ("entry", "remove_rathole",
         'trace_event("rathole_remove %u owner=%u", (unsigned)allhosps[owner].ratholes[idx].tile, (unsigned)owner);'),
        (("before", "insert_node(rat, rat->x, rat->y);"), "make_a_rat",
         'trace_event("rat %u owner=%u from=%u to=%u", (unsigned)(rat - thing), (unsigned)owner, '
         '(unsigned)allhosps[owner].ratholes[from].tile, (unsigned)allhosps[owner].ratholes[to].tile);'),
        (("before", "    }\n}"), "try_bonus_level",
         'trace_event("bonus_level %u", (unsigned)othervar.bonus_level);'),
    ],
    "tutorial.cpp": [
        ("entry", "tutorial", 'trace_event("tutorial %u", (unsigned)advisor.tutorial_step);'),
    ],
    # The computer hospitals (owners 1-3), each event tagged with its owner.
    "computer.cpp": [
        (("before", "    }\n}"), "computer_action",
         'trace_event("computer_action %u type=%u arg=%d count=%d next=%lu", (unsigned)i, (unsigned)hosp->action.type, '
         '(int)hosp->action.arg, (int)hosp->action.count, (unsigned long)hosp->next_action_turn);'),
        (("before", "action = com_strategy[step].type;"), "computer_decision",
         'trace_event("computer_decision %u step=%d type=%u", (unsigned)owner, step, '
         '(unsigned)com_strategy[step].type);'),
        (("before", "announce_this(ANN_RIVAL_OPENED, owner);"), "computer_decision",
         'trace_event("rival_open %u", (unsigned)owner);'),
        (("before", "candidate->owner = owner;"), "computer_employ",
         'trace_event("computer_employ %u thing=%u subtype=%u balance=%ld", (unsigned)owner, '
         '(unsigned)(candidate - thing), (unsigned)candidate->subtype, (long)hosp->balance);'),
        (("before", "return 1;"), "computer_build",
         'trace_event("computer_build %u room=%u balance=%ld", (unsigned)owner, (unsigned)type, '
         '(long)hosp->balance);'),
        (("before", "return 1;"), "computer_place_object",
         'trace_event("computer_place_object %u obj=%u balance=%ld", (unsigned)owner, (unsigned)obj, '
         '(long)hosp->balance);'),
        ("exit", "computer_loan",
         'if (r) trace_event("computer_loan %u amount=%d balance=%ld loan=%ld", (unsigned)owner, (int)amount, '
         '(long)allhosps[owner].balance, (long)allhosps[owner].loan);'),
        (("before", "return 1;"), "computer_buy_land",
         'trace_event("computer_land %u area=%u balance=%ld", (unsigned)owner, (unsigned)hosp->area, '
         '(long)hosp->balance);'),
        (("before", "return 1;"), "make_a_note",
         'trace_event("computer_note %u op=make type=%u arg=%d count=%d", (unsigned)owner, '
         '(unsigned)hosp->notes[i].type, (int)hosp->notes[i].arg, (int)hosp->notes[i].count);'),
        (("before", "hosp->notes[i].type = 0;"), "remove_a_note",
         'trace_event("computer_note %u op=remove type=%u arg=%d count=%d", (unsigned)owner, '
         '(unsigned)hosp->notes[i].type, (int)hosp->notes[i].arg, (int)hosp->notes[i].count);'),
        (("before", "break;"), "declass_a_note",
         'trace_event("computer_note %u op=declass type=%u arg=%d count=%d", (unsigned)owner, '
         '(unsigned)hosp->notes[i].type, (int)hosp->notes[i].arg, (int)hosp->notes[i].count);'),
        (("before", "bucket->patients_done++;"), "process_computer_peep",
         'trace_event("computer_peep %u illness=%d result=%d load=%ld", (unsigned)owner, illness, result, '
         '(long)hosp->patient_load);'),
        (("before", "memset(bucket, 0, 400);"), "get_computer_bucket",
         'trace_event("computer_bucket %u balance=%ld cures=%ld deaths=%ld rep=%ld load=%ld", (unsigned)owner, '
         '(long)hosp->balance, (long)hosp->cures, (long)hosp->deaths, (long)hosp->reputation, '
         '(long)hosp->patient_load);'),
        (("before", "            }\n        }\n    }\n}"), "computer_month",
         'trace_event("computer_month %u balance=%ld wage_bill=%ld", (unsigned)i, (long)hosp->balance, '
         '(long)hosp->wage_bill);'),
        (("before", "            }\n        }\n    }\n}"), "computer_year",
         'trace_event("computer_year %u value=%ld score=%ld", (unsigned)i, (long)hosp->hospital_value, '
         '(long)hosp->score);'),
    ],
}

DECLS = '''// Replay check instrumentation (instrument.py).
extern "C" void trace_event(const char *format, ...);
extern "C" void trace_note(const char *label);
extern "C" unsigned long trace_newrand(const char *caller);
extern "C" unsigned long trace_normal_rand(const char *caller, unsigned long low, unsigned long high);
'''


def definition(s, name):
    """(start of the signature, start of the body's first line, end of the
    body's closing brace line, the signature) of function `name` in `s`."""
    m = re.search(r"^(?![ \t#/])([^;{}\n]*?)\b" + name + r"\(([^;{}]*?)\)[ \t]*\n\{\n", s, re.M)
    assert m, f"no definition of {name}"
    end = s.index("\n}\n", m.end()) + 3
    return m.start(), m.end(), end, m


def entry(s, name, code):
    _, body, _, _ = definition(s, name)
    return s[:body] + "    " + code + "\n" + s[body:]


def before(s, name, anchor, code):
    _, body, end, _ = definition(s, name)
    at = s.index(anchor, body, end)
    assert s.find(anchor, at + 1, end) == -1, f"{name}: anchor not unique"
    line = s.rindex("\n", 0, at) + 1
    indent = s[line:at]
    return s[:line] + indent + code + "\n" + s[line:]


def exit_(s, name, code):
    start, _, end, m = definition(s, name)
    ret, params = m.group(1), m.group(2)
    names = [] if params.strip() == "void" else [re.findall(r"\w+", p)[-1] for p in params.split(",")]
    inner = name + "_untraced"
    s = s[:start] + s[start:end].replace(name + "(", inner + "(", 1) + s[end:]
    end += len(inner) - len(name)
    wrapper = (f"\n{ret}{name}({params})\n{{\n    {ret.strip()} r = {inner}({', '.join(names)});\n"
               f"    {code}\n    return r;\n}}\n")
    return s[:end] + wrapper + s[end:]


def tag_rand(s):
    """Calls of old_newrand and old_normal_rand in function bodies name
    their function (trace_newrand, trace_normal_rand)."""
    out = []
    name = None
    lines = s.split("\n")
    for i, line in enumerate(lines):
        if line == "{":
            sig = ""
            for prev in reversed(lines[:i]):
                if not prev.strip() or prev.rstrip().endswith((";", "}")) or prev.startswith(("//", "#")):
                    break
                sig = prev + " " + sig
            m = re.search(r"(\w+)\s*\(", sig)
            name = m.group(1) if m else None
        elif line == "}":
            name = None
        elif name:
            line = line.replace("old_newrand()", f'trace_newrand("{name}")')
            line = line.replace("old_normal_rand(", f'trace_normal_rand("{name}", ')
        out.append(line)
    return "\n".join(out)


def instrument(tree):
    src = os.path.join(tree, "src")
    changed = []
    for unit in sorted(os.listdir(src)):
        path = os.path.join(src, unit)
        if not unit.endswith(".cpp") or not os.path.isfile(path):
            continue
        s = orig = open(path).read()

        def rep(old, new):
            nonlocal s
            assert old in s, f"{path}: not found: {old}"
            s = s.replace(old, new, 1)

        # First, before the instrumentation's own draws and renamed functions.
        if unit != "rand.cpp":
            s = tag_rand(s)
        if unit == "game.cpp":
            for old, new in GAME_EDITS:
                rep(old, new)

        elif unit == "start.cpp":
            for old, new in START_EDITS:
                rep(old, new)
        elif unit == "finance.cpp":
            for old, new in FINANCE_EDITS:
                rep(old, new)
            for indent, call in FINANCE_CALLS:
                fn = call.split("(")[0]
                rep(f"{indent}{call}\n",
                    f'{indent}if (!trace_skip("{fn}"))\n{indent}    {call}\n{indent}trace_note("{fn} done");\n')
        elif unit == "sim.cpp":
            for old, new in SIM_EDITS:
                rep(old, new)
        elif unit == "cursor.cpp":
            for old, new in CURSOR_EDITS:
                rep(old, new)
        for where, name, code in EVENTS.get(unit, []):
            if where == "entry":
                s = entry(s, name, code)
            elif where == "exit":
                s = exit_(s, name, code)
            else:
                s = before(s, name, where[1], code)
        if unit == "rand.cpp":
            s = entry(s, "old_normal_rand", "const char *trace_caller = trace_rand_caller;")
            rep("    sum += old_newrand() % range;", "    trace_rand_caller = trace_caller;\n    sum += old_newrand() % range;")
            rep("    seed = ror13(seed);\n    return seed;", "    seed = ror13(seed);\n    trace_rand(seed);\n    return seed;")
            s = 'extern "C" const char *trace_rand_caller;\nextern "C" void trace_rand(unsigned long value);\n' + s
        if s != orig:

            open(path, "w").write(DECLS + s)
            changed.append(os.path.join("src", unit))
    for unit in EVENTS:
        assert os.path.join("src", unit) in changed, f"{unit}: missing"
    print("\n".join(changed))


if __name__ == "__main__":
    if len(sys.argv) != 2 or os.path.abspath(sys.argv[1]) == os.path.dirname(
            os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))):
        sys.exit(__doc__.strip())
    instrument(sys.argv[1])

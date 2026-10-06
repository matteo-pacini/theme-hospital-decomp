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
        if (field == 4) {
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
            player_events[0].type = (u8)script[script_next][1];
            player_events[0].value = (u16)script[script_next][2];
            player_events[0].flags = 0;
            player_events[0].arg = (u16)script[script_next][3];
            trace_event("cmd type=%u value=%u arg=%u", (unsigned)player_events[0].type,
                        (unsigned)player_events[0].value, (unsigned)player_events[0].arg);
            process_turn();
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
    if (trace_handle != -1)
        LbFileClose(trace_handle);
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
    ],
    "queue.cpp": [
        ("entry", "join_queue",
         f'trace_event("join_queue %u room=%u", {ID.format("person")}, (unsigned)(room - room_info));'),
        ("entry", "leave_queue",
         f'trace_event("leave_queue %u room=%u dest=%u", {ID.format("person")}, (unsigned)person->queue_room, '
         '(unsigned)person->dest_room);'),
    ],
    "treat.cpp": [
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
        ("entry", "effect_illness",
         f'if (cured == 1) trace_event("cure %u illness=%u", {ID.format("patient")}, (unsigned)patient->illness);'),
    ],
    "reaper.cpp": [
        ("entry", "summon_the_reaper", f'trace_event("summon_reaper %u", {ID.format("victim")});'),
    ],
    "doctor.cpp": [
        (("before", "patient->resume_state = 0;"), "doctor_waits_for_patient_to_arrive",
         f'trace_event("treat %u patient=%u room=%u", {ID.format("doctor")}, {ID.format("patient")}, '
         '(unsigned)(doctor->room - room_info));'),
    ],
    "peepmake.cpp": [
        ("exit", "make_a_peep",
         'if (r) trace_event("peep %u illness=%u owner=%u pos=%u", (unsigned)(r - thing), (unsigned)illness, '
         '(unsigned)owner, (unsigned)pos);'),
    ],
    "utils.cpp": [
        ("entry", "remove_thing",
         f'trace_event("remove_thing %u type=%u", {ID.format("target")}, (unsigned)target->type);'),
    ],
    "peeps.cpp": [
        ("entry", "start_person_going", f'trace_event("start_going %u", {ID.format("person")});'),
        ("entry", "person_go_home",
         f'trace_event("go_home %u state=%u", {ID.format("person")}, (unsigned)person->state);'),
        ("entry", "person_arrives_at_door", f'trace_event("at_door %u", {ID.format("person")});'),
        ("entry", "call_person_to_room",
         f'trace_event("call %u id=%u action=%u arg=%u", {ID.format("caller")}, (unsigned)id, (unsigned)action, '
         '(unsigned)arg);'),
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
        (("before", "reset_icon_value(peep, ICON_TOILET);"), "do_reaction",
         f'trace_event("wets %u bladder=%d", {ID.format("peep")}, (int)peep->bladder);'),
    ],
    "io.cpp": [
        (("before", "allhosps[i].rest_threshold = player_events[i].arg;"), "process_turn",
         'trace_event("policy_rest %u", (unsigned)player_events[i].arg);'),
    ],
    "req4.cpp": [
        (("before", "handyman->litter_count = 0;"), "adjust_handyman_values",
         'trace_event("handyman_values %u litter=%u plants=%u repair=%u", (unsigned)id, '
         '(unsigned)handyman->prio_litter, (unsigned)handyman->prio_plants, (unsigned)handyman->prio_repair);'),
    ],
    "complex.cpp": [
        ("entry", "set_complex",
         f'trace_event("set_complex %u action=%u arg=%u", {ID.format("w")}, (unsigned)action, (unsigned)arg);'),
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
    ],
    "things.cpp": [
        (("before", "remove_node(cur);\n                cur->state = SS_APPLICANT;"), "process_things",
         f'trace_event("back_to_applicants %u", {ID.format("cur")});'),
    ],
    "handyman.cpp": [
        ("entry", "place_litter",
         'trace_event("litter %u type=%u owner=%u", (unsigned)tile, (unsigned)type, (unsigned)(u8)owner);'),
        ("entry", "remove_litter", 'trace_event("unlitter %u owner=%u", (unsigned)tile, (unsigned)owner);'),
    ],
    "epidemic.cpp": [
        ("entry", "spread_epidemic", f'trace_event("epidemic %u", {ID.format("carrier")});'),
    ],
    "blueprnt.cpp": [
        ("entry", "commit_blueprint", 'if (owner == 0) trace_event("commit_blueprint");'),
        ("entry", "uncommit_blueprint", 'if (owner == 0) trace_event("uncommit_blueprint");'),
        ("entry", "commit_doors", 'if (owner == 0) trace_event("commit_doors");'),
        ("entry", "delete_blueprint_room", 'if (owner == 0) trace_event("delete_blueprint_room");'),
        ("exit", "allocate_reception_roominfo",
         'if (r) trace_event("reception %u", (unsigned)(r - room_info));'),
    ],
    "build.cpp": [
        ("entry", "open_room", 'if (owner == 0) trace_event("open_room");'),
        ("entry", "delete_room", 'trace_event("delete_room %u", (unsigned)(room - room_info));'),
    ],
}

DECLS = '''// Replay check instrumentation (instrument.py).
extern "C" void trace_event(const char *format, ...);
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

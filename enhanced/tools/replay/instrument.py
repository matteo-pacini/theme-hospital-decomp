#!/usr/bin/env python3
"""Instrument copies of the decompilation's units for the replay check.

Usage: instrument.py FILE...  (copies of src/game.cpp and src/finance.cpp,
edited in place; never the repository's own src/)

game.cpp: game() writes TRACE.TXT, the trace of core/world/trace.h, once as
the level starts and at the end of every frame that ran a turn; quits after
turn TRACE_TURNS (environment, default 2000); carries out SCRIPT.TXT's
commands (lines "turn type value arg", decimal) between turn - 1 and turn,
as th_replay --script does; and runs without the frame limit or the game
speed's turn skipping, so that every frame is a turn.

finance.cpp: daily_admin and twice_daily_admin note the seed after each
call that may draw from it ("# <call> done seed=...", which compare.py
ignores), and leave out the calls TRACE_SKIP names (environment, names
separated by commas), to compare past a function the engine does not port.
"""

import os
import sys

TRACE = r'''
// Replay check instrumentation (build/replay only, not part of the decomp).
extern "C" char *getenv(const char *name);
static int trace_handle = -1;
static u32 trace_limit = 2000;
static char trace_buf[256];
static u32 trace_turn;

static void trace_line(int len)
{
    LbFileWrite(trace_handle, trace_buf, len);
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
// other lines ignored), each carried out between turn - 1 and turn, as the
// replay tool does.
static u32 script[64][4];
static int script_count, script_next;

static void script_load(void)
{
    static char text[4096];
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
    for (p = text; *p && script_count < 64;) {
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
            process_turn();
        }
        script_next++;
    }
}

static void trace_open(void)
{
    char *n = getenv("TRACE_TURNS");

    if (n)
        trace_limit = atoi(n);
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
    ("""        if (othervar.fade_palette) {""", """        if (actual_game_turn != trace_turn) {
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


def instrument(path):
    s = open(path).read()

    def rep(old, new):
        nonlocal s
        assert old in s, f"{path}: not found: {old}"
        s = s.replace(old, new, 1)

    name = os.path.basename(path)
    if name == "game.cpp":
        for old, new in GAME_EDITS:
            rep(old, new)
    elif name == "finance.cpp":
        for old, new in FINANCE_EDITS:
            rep(old, new)
        for indent, call in FINANCE_CALLS:
            fn = call.split("(")[0]
            rep(f"{indent}{call}\n",
                f'{indent}if (!trace_skip("{fn}"))\n{indent}    {call}\n{indent}trace_note("{fn} done");\n')
    else:
        sys.exit(f"{path}: not a unit instrument.py knows")
    open(path, "w").write(s)


if __name__ == "__main__":
    for p in sys.argv[1:]:
        instrument(p)

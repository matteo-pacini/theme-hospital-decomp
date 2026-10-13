// th_replay: runs a level headless, from its start or from a snapshot, with
// only scripted input, and writes the replay check's trace
// (core/world/trace.h) of every turn with its events, to compare (compare.py)
// with the instrumented original's (orig_trace.sh); at the end it can dump
// the game's state and render the view.
//
// Usage: th_replay [--data DIR] [--level N] [-bID] [--load SNAPSHOT] [--turns [+]N] [--script FILE] [--astar] [--rand]
//                  [--skip NAME[,NAME]] [--advisor]
//                  [--out FILE] [--dump FILE [--focus CELL]] [--render OUT.png|OUT.ppm [--camera X,Y] [--size WxH]]
//                  [--snapshot FILE]
//   --data    the game data folder (default: the development folder)
//   --level   the level to start (default 1); with --load, the snapshot's
//   -bID      the balance files' prefix (default FULL), as the game's -b: levels/<ID>00.sam
//             and levels/<ID><level>.sam; --level also takes "N -bID"
//   --load    start from a snapshot (core/world/snapshot.h; a debug report's
//             snapshot.bin) instead of the level's start
//   --turns   the last turn to run, as the game counts them from 1, or with
//             +, the turns to run from the start or the snapshot (default 2000)
//   --script  the player's input: lines "turn type value arg" (decimal; the
//             game's EV_* numbers), each command carried out between turn - 1
//             and turn, and the other lines of Game::command_log (a debug
//             report's commands.txt); with --load, turns count from the
//             snapshot's (turn 1 the first run after it); "#" lines and others
//             are ignored, but a command log of another build is refused
//   --astar   plan routes with A* rather than the game's wall following
//             (from the level's start; a snapshot or command log keeps its own)
//   --rand    also trace every draw from the random seed (`rand` events)
//   --skip    calls left out, comma-separated, as the instrumented original's
//             SKIP (orig_trace.sh): computer_action (default none)
//   --advisor also run the advisor (core/world/advisor.h) in each turn,
//             after its start, as the game's frame does, and note each advice
//             it gives in the trace as a comment line "# advice <situation>"
//             (compare.py ignores it; the instrumented original writes the
//             same, followed by the seed)
//   --out     where to write the trace (default: standard output, unless
//             --dump or --render is given)
//   --dump    write dump_game (core/world/debug_dump.h) at the end, with the
//             map around cell --focus (column + row * 128)
//   --snapshot write the game's snapshot (core/world/snapshot.h) at the end, to
//             continue with --load (in this build or one of the same snapshot id)
//   --render  draw the hospital view at the end, with shadows, as a PNG or (by
//             the extension) a PPM: --size pixels (default 1280x720), the
//             view's top-left at map pixel --camera (a debug report's camera
//             line; default centred on the level's starting cell)

#include "core/data/balance.h"
#include "core/data/gamedata.h"
#include "core/render/iso.h"
#include "core/render/shadows.h"
#include "core/render/surface.h"
#include "core/render/view.h"
#include "core/ui/charts.h"
#include "core/world/debug_dump.h"
#include "core/world/game.h"
#include "core/world/savegame.h"
#include "core/world/snapshot.h"
#include "core/world/staff.h"
#include "core/world/trace.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace
{

int usage()
{
    std::fprintf(stderr, "usage: th_replay [--data DIR] [--level N] [--load SNAPSHOT] [--turns [+]N] [--script FILE] "
                         "[--astar] [--rand] [--skip NAMES] [--advisor] [--out FILE] [--dump FILE [--focus CELL]] "
                         "[--render OUT.png|OUT.ppm "
                         "[--camera X,Y] [--size WxH]] [--save SAVE] [--snapshot FILE]\n");
    return 2;
}

// A script line: what Game::replay_input carries out before turn `turn`.
struct ScriptLine {
    uint32_t turn = 0;
    std::string input;
};

// Reads `path`'s commands into `script`, and a "# Score N" line into
// `carried_score` (the score the last level left; the original's side reads
// the same line).
bool read_script(const char *path, std::vector<ScriptLine> &script, int32_t &carried_score, std::string &error)
{
    std::ifstream in(path);
    if (!in) {
        error = std::string(path) + ": " + std::strerror(errno);
        return false;
    }
    std::string line;
    bool other_build = false, raw_bytes = false;
    while (std::getline(in, line)) {
        if (line.starts_with("#")) {
            if (line.starts_with("# Score "))
                carried_score = int32_t(std::strtol(line.c_str() + 8, nullptr, 10));
            // Game::command_log's first line names the build.
            if (size_t at = line.find(", build "); at != std::string::npos)
                other_build |= std::strtoull(line.c_str() + at + 8, nullptr, 16) != th::snapshot_build_id();
            continue;
        }
        std::istringstream fields(line);
        uint32_t turn;
        std::string first;
        if (!(fields >> turn >> first))
            continue;
        if (first[0] == '@' || std::isdigit(static_cast<unsigned char>(first[0]))) {
            raw_bytes |= first == "@placing" || first == "@build" || first == "@hospital";
            // After the turn: "113 1 0 0" has its command's "1" in the turn too.
            size_t turn_end = line.find_first_not_of("0123456789", line.find_first_not_of(" \t"));
            script.push_back({turn, line.substr(line.find(first, turn_end))});
        }
    }
    if (other_build && raw_bytes) {
        error = std::string(path) + ": a command log of another build (its @placing, @build and @hospital lines "
                                    "are that build's bytes)";
        return false;
    }
    return true;
}

uint32_t crc32(const uint8_t *p, size_t n, uint32_t crc = 0)
{
    crc = ~crc;
    for (size_t i = 0; i < n; i++) {
        crc ^= p[i];
        for (int k = 0; k < 8; k++)
            crc = (crc >> 1) ^ (0xedb88320u & (0u - (crc & 1)));
    }
    return ~crc;
}

void put32(std::vector<uint8_t> &out, uint32_t v)
{
    for (int s = 24; s >= 0; s -= 8)
        out.push_back(uint8_t(v >> s));
}

// `rgb` (width * height * 3 bytes) as a PNG, its image data in stored
// (uncompressed) deflate blocks.
std::vector<uint8_t> encode_png(const std::vector<uint8_t> &rgb, int width, int height)
{
    std::vector<uint8_t> raw;
    for (int y = 0; y < height; y++) {
        raw.push_back(0); // no filter
        raw.insert(raw.end(), rgb.begin() + y * width * 3, rgb.begin() + (y + 1) * width * 3);
    }
    std::vector<uint8_t> z = {0x78, 0x01};
    for (size_t at = 0; at < raw.size() || at == 0; at += 0xffff) {
        size_t n = std::min<size_t>(0xffff, raw.size() - at);
        z.push_back(at + n == raw.size());
        z.insert(z.end(), {uint8_t(n), uint8_t(n >> 8), uint8_t(~n), uint8_t(~n >> 8)});
        z.insert(z.end(), raw.begin() + long(at), raw.begin() + long(at + n));
    }
    uint32_t a = 1, b = 0;
    for (uint8_t c : raw) {
        a = (a + c) % 65521;
        b = (b + a) % 65521;
    }
    put32(z, b << 16 | a);

    std::vector<uint8_t> png = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1a, '\n'};
    auto chunk = [&](const char *type, const std::vector<uint8_t> &data) {
        put32(png, uint32_t(data.size()));
        size_t start = png.size();
        png.insert(png.end(), type, type + 4);
        png.insert(png.end(), data.begin(), data.end());
        put32(png, crc32(png.data() + start, png.size() - start));
    };
    std::vector<uint8_t> header;
    put32(header, uint32_t(width));
    put32(header, uint32_t(height));
    header.insert(header.end(), {8, 2, 0, 0, 0}); // 8-bit RGB
    chunk("IHDR", header);
    chunk("IDAT", z);
    chunk("IEND", {});
    return png;
}

// Draws the view of `game` as the app does (with shadows) and writes it to
// `path`, a PNG or, ending in .ppm, a binary PPM.
bool render(const th::Game &game, const th::GameData &data, const std::string &path, th::Point camera, int width,
            int height)
{
    th::ShadowSprites shadows(data.blocks(), data.ghost());
    th::Surface view, scratch;
    view.resize(width, height);
    view.fill(0);
    scratch.resize(width, height);
    scratch.fill(th::kMaskColour);
    th::draw_world({view, {0, 0, width, height}, camera, game.turn_count}, game.world, data, &shadows, scratch);
    std::vector<uint8_t> rgb;
    rgb.reserve(size_t(width) * size_t(height) * 3);
    for (size_t i = 0; i < size_t(width) * size_t(height); i++) {
        uint32_t c = data.palette()[view.data()[i]];
        rgb.insert(rgb.end(), {uint8_t(c >> 16), uint8_t(c >> 8), uint8_t(c)});
    }
    std::ofstream out(path, std::ios::binary);
    if (path.ends_with(".ppm")) {
        out << "P6\n" << width << ' ' << height << "\n255\n";
        out.write(reinterpret_cast<const char *>(rgb.data()), std::streamsize(rgb.size()));
    } else {
        std::vector<uint8_t> png = encode_png(rgb, width, height);
        out.write(reinterpret_cast<const char *>(png.data()), std::streamsize(png.size()));
    }
    return bool(out);
}

// The command line's options.
struct Options {
#ifdef TH_DEV_DATA_DIR
    std::string data_dir = TH_DEV_DATA_DIR;
#else
    std::string data_dir = ".";
#endif
    int level_number = 1;
    bool level_given = false;
    std::string balance_id{th::kDefaultBalanceId};
    std::string turns_arg = "2000";
    bool astar = false;
    bool rand = false;
    bool advisor = false;
    const char *out_path = nullptr;
    const char *script_path = nullptr;
    const char *load_path = nullptr;
    const char *dump_path = nullptr;
    const char *render_path = nullptr;
    const char *save_path = nullptr;
    const char *snapshot_path = nullptr;
    const char *skip = "";
    int focus = -1;
    bool camera_given = false;
    th::Point camera{};
    int width = 1280, height = 720;
};

// Takes `value` for `flag` when it is one of the options whose value is a
// path or kept as text; false for any other flag.
bool take_text_option(Options &o, const char *flag, const char *value)
{
    if (!std::strcmp(flag, "--data"))
        o.data_dir = value;
    else if (!std::strcmp(flag, "--turns"))
        o.turns_arg = value;
    else if (!std::strcmp(flag, "--script"))
        o.script_path = value;
    else if (!std::strcmp(flag, "--out"))
        o.out_path = value;
    else if (!std::strcmp(flag, "--load"))
        o.load_path = value;
    else if (!std::strcmp(flag, "--dump"))
        o.dump_path = value;
    else if (!std::strcmp(flag, "--render"))
        o.render_path = value;
    else if (!std::strcmp(flag, "--save"))
        o.save_path = value;
    else if (!std::strcmp(flag, "--snapshot"))
        o.snapshot_path = value;
    else if (!std::strcmp(flag, "--skip"))
        o.skip = value;
    else
        return false;
    return true;
}

// Reads --level's value: the level number, then optionally the balance id as
// the game's command line gives it ("20 -bNET", a script's `# Balance`).
bool parse_level_arg(Options &o, const char *value)
{
    char *end = nullptr;
    o.level_number = int(std::strtol(value, &end, 10));
    if (end == value)
        return false;
    while (*end == ' ')
        end++;
    if (!std::strncmp(end, "-b", 2) && end[2])
        o.balance_id = end + 2;
    else if (*end)
        return false;
    return true;
}

// Takes `arg` when it is an option without a value (--astar, --rand,
// --advisor, -b<id>); false for any other.
bool take_flag(Options &o, const char *arg)
{
    if (!std::strcmp(arg, "--astar"))
        o.astar = true;
    else if (!std::strcmp(arg, "--rand"))
        o.rand = true;
    else if (!std::strcmp(arg, "--advisor"))
        o.advisor = true;
    else if (!std::strncmp(arg, "-b", 2) && arg[2])
        o.balance_id = arg + 2;
    else
        return false;
    return true;
}

// Reads the command line into `o`; false when it is not a valid one.
bool parse_args(int argc, char **argv, Options &o)
{
    for (int i = 1; i < argc; i++) {
        bool has_value = i + 1 < argc;
        if (has_value && take_text_option(o, argv[i], argv[i + 1])) {
            i++;
        } else if (!std::strcmp(argv[i], "--level") && has_value) {
            if (!parse_level_arg(o, argv[++i]))
                return false;
            o.level_given = true;
        } else if (!std::strcmp(argv[i], "--focus") && has_value) {
            o.focus = std::atoi(argv[++i]);
        } else if (!std::strcmp(argv[i], "--camera") && has_value) {
            if (std::sscanf(argv[++i], "%d,%d", &o.camera.x, &o.camera.y) != 2)
                return false;
            o.camera_given = true;
        } else if (!std::strcmp(argv[i], "--size") && has_value) {
            if (std::sscanf(argv[++i], "%dx%d", &o.width, &o.height) != 2 || o.width <= 0 || o.height <= 0)
                return false;
        } else if (!take_flag(o, argv[i])) {
            return false;
        }
    }
    return true;
}

// Loads the game from the snapshot --load, moving the script's turns to
// count from it. Reports what went wrong on standard error.
bool load_from_snapshot(th::Game &game, Options &o, const th::GameData &data, std::vector<ScriptLine> &script)
{
    std::string error;
    std::ifstream in(o.load_path, std::ios::binary);
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (!in && !in.eof()) {
        std::perror(o.load_path);
        return false;
    }
    if (!th::load_snapshot(game, bytes, data, &error)) {
        std::fprintf(stderr, "%s: %s\n", o.load_path, error.c_str());
        return false;
    }
    if (o.level_given && o.level_number != game.level_number()) {
        std::fprintf(stderr, "%s is of level %d, not %d\n", o.load_path, game.level_number(), o.level_number);
        return false;
    }
    o.level_number = game.level_number();
    for (ScriptLine &line : script)
        line.turn += game.turn_count;
    return true;
}

// Starts the game at level --level's start. Reports what went wrong on
// standard error.
bool start_level(th::Game &game, const Options &o, const th::GameData &data, int32_t carried_score)
{
    std::string error;
    auto level = data.load_level(o.level_number, &error);
    th::Balance settings;
    if (!level || !th::load_balance(data.dir(), o.level_number, o.balance_id, settings, &error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return false;
    }
    // As the app's begin_level and load_level: the game's start with no
    // tutorial and the wall shadows; the insurers' clock draw does not
    // touch the seed.
    game.staff_market.ref_counts = th::reference_counts(data.text());
    game.start(*level, o.level_number, settings, data.anims(), data.person_complexes(), data.staff_complexes());
    game.carry_score(carried_score);
    th::add_static_shadows(game.world.map);
    th::start_advisor(game.advisor, false);
    th::start_hospital_settings(game.hospital, game.settings());
    th::setup_insurances(game.world, 0);
    game.world.path_finder = o.astar ? th::PathFinder::AStar : th::PathFinder::Original;
    return true;
}

// display_advisor as the game's frame runs it, between the turn's start and
// its things: each advice given is noted in `out` (when not null) as a
// comment line. `last` and `pending` are the commands carried out last and
// at this turn, for the tutorial's steps.
void run_advisor(th::Game &game, const th::GameData &data, std::FILE *out, const th::Event &last,
                 const th::Event &pending, bool cursor_moved)
{
    th::AdvisorInputs in;
    in.game_turn = game.turn_count;
    in.level = game.hospital_level();
    in.month = game.date.month;
    in.open = game.open;
    in.balance = game.balance;
    in.reputation = game.reputation();
    in.finance = &game.finance;
    in.settings = &game.settings();
    in.world = &game.world;
    in.visitors = game.arrivals.hospitals[0].visitors;
    in.quake_phase = game.disasters.quake_phase;
    in.win_progress = game.progress.win_average;
    in.lose_progress = game.progress.lose_progress;
    in.text = &data.text();
    in.last = last;
    in.pending = pending;
    // The tutorial's placement steps read the pointer's cell, which a script
    // does not move; a held member of staff is always placeable here.
    in.staff_place_ok = true;
    // The instrumented original draws the object cursor, setting or clearing
    // the advisor's bad placement flag, only where a script moves it (@cursor;
    // instrument.py removes process_mouse).
    if (cursor_moved && game.world.placing.placing)
        in.object_place_ok = game.world.placing.ok;
    std::array<uint8_t, th::kAdviceCount> given{};
    for (size_t i = 0; i < given.size(); i++)
        given[i] = game.advisor.advice[i].times_given;
    // The tutorial's step events belong to this turn, as the game's frame
    // writes its turn's events.
    th::EventScope scope(&game.events, game.turn_count);
    th::advisor_frame(game.advisor, in, data.anims(), game.safeseed);
    for (size_t i = 0; i < given.size(); i++)
        if (out && game.advisor.advice[i].times_given != given[i])
            std::fprintf(out, "# advice %u\n", unsigned(game.advisor.advice[i].id));
}

// The command a script line gives ("<type> <value> <arg>"), or EV_NONE for a
// line that is not one (an @ setting or a byte change).
th::Event script_command(const std::string &line)
{
    unsigned type = 0, value = 0, arg = 0;
    if (line.empty() || line[0] == '@' || std::sscanf(line.c_str(), "%u %u %u", &type, &value, &arg) != 3)
        return {};
    return {uint8_t(type), uint16_t(value), 0, uint16_t(arg)};
}

// Carries out the script's commands due before turn `turn`, in order, and
// leaves the last command's event in `command` (the one the original's
// process_turn leaves in old_packet). False, reported on standard error,
// when one cannot be carried out.
bool apply_script_commands(th::Game &game, const Options &o, const std::vector<ScriptLine> &script, size_t &next,
                           uint32_t turn, th::Event &command)
{
    std::string error;
    for (; next < script.size() && script[next].turn <= turn; next++) {
        if (script[next].turn != turn)
            continue;
        if (th::Event e = script_command(script[next].input); e.type)
            command = e;
        if (!game.replay_input(script[next].input, &error)) {
            std::fprintf(stderr, "%s: %s\n", o.script_path, error.c_str());
            return false;
        }
    }
    return true;
}

// Whether the script moves the object cursor (@cursor) at turn `turn`.
bool script_moves_cursor(const std::vector<ScriptLine> &script, uint32_t turn)
{
    for (const ScriptLine &line : script)
        if (line.turn == turn && line.input.rfind("@cursor", 0) == 0)
            return true;
    return false;
}

// Runs turns up to `last_turn`, carrying out the script's commands between
// them and tracing each to `out` (when not null). False, reported on
// standard error, when a command cannot be carried out.
bool run_turns(th::Game &game, const Options &o, const std::vector<ScriptLine> &script, uint32_t last_turn,
               std::FILE *out, const th::GameData &data)
{
    size_t next = 0;
    while (game.turn_count < last_turn) {
        th::Event command{};
        if (!apply_script_commands(game, o, script, next, game.turn_count + 1, command))
            return false;
        if (game.paused) {
            std::fprintf(stderr, "the script leaves the game paused before turn %u\n", game.turn_count + 1);
            break;
        }
        // The tutorial's steps run whether or not --advisor asked for the
        // advisor, as the game runs them every frame.
        bool with_advisor = o.advisor || (game.advisor.flags & th::kAdvisorTutorial);
        if (!with_advisor) {
            game.turn();
        } else if (game.start_turn()) {
            // The original carries a scripted command out at the end of the
            // frame before the one it is for, so the advisor of turn N sees
            // it as the last command, with none pending.
            // A win or loss decided this turn leaves the game screen, so the
            // next frame has no display_advisor; nor has a frame with a fax up
            // (display_mode DM_CHOICE).
            if (game.progress.winlose_result == 0 && !game.progress.rat_game_over && !game.fax_screen.open)
                run_advisor(game, data, out, command, {}, script_moves_cursor(script, game.turn_count));
            game.run_things();
        }
        if (out)
            std::fputs(th::trace_turn(game, &game.events).c_str(), out);
        game.events.events.clear();
        // The original ends the run at the turn the win or loss is decided
        // (its year-end screens wait for input the replay cannot give) or the
        // rat game ends (it goes to the area screen), so the port stops here
        // too, at the same turn.
        if (game.progress.winlose_result != 0 || game.progress.rat_game_over)
            break;
    }
    return true;
}

// Writes the snapshot of `game` to `path`.
bool write_snapshot(const th::Game &game, const char *path)
{
    std::vector<uint8_t> bytes;
    if (!th::save_snapshot(game, bytes))
        return false;
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char *>(bytes.data()), std::streamsize(bytes.size()));
    return bool(out);
}

// Writes --dump, --snapshot, --save and --render at the end of the run.
// Returns the process exit code.
int write_results(const th::Game &game, Options &o, const th::GameData &data)
{
    if (o.dump_path && !(std::ofstream(o.dump_path) << th::dump_game(game, o.focus))) {
        std::perror(o.dump_path);
        return 1;
    }
    if (o.snapshot_path && !write_snapshot(game, o.snapshot_path)) {
        std::fprintf(stderr, "%s: cannot write the snapshot\n", o.snapshot_path);
        return 1;
    }
    if (o.save_path) {
        std::string error;
        th::SaveInfo info;
        info.balance_id = o.balance_id;
        auto level = data.load_level(o.level_number, &error);
        int start = level ? level->camera_start[0] : 0;
        th::Point centre = th::cell_to_world(start % th::kMapSize, start / th::kMapSize);
        info.view_centre_x = centre.x;
        info.view_centre_y = centre.y;
        if (!th::write_save_slot(o.save_path, game, info, &error)) {
            std::fprintf(stderr, "%s: %s\n", o.save_path, error.c_str());
            return 1;
        }
    }
    if (!o.render_path)
        return 0;
    if (!o.camera_given) {
        std::string error;
        auto level = data.load_level(o.level_number, &error);
        int start = level ? level->camera_start[0] : 0;
        th::Point centre = th::cell_to_world(start % th::kMapSize, start / th::kMapSize);
        o.camera = {centre.x - o.width / 2, centre.y - o.height / 2};
    }
    if (!render(game, data, o.render_path, o.camera, o.width, o.height)) {
        std::perror(o.render_path);
        return 1;
    }
    return 0;
}

} // namespace

int main(int argc, char **argv)
{
    Options o;
    if (!parse_args(argc, argv, o))
        return usage();

    std::string error;
    std::vector<ScriptLine> script;
    int32_t carried_score = 0;
    if (o.script_path && !read_script(o.script_path, script, carried_score, error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }

    th::GameData data;
    if (!data.load(o.data_dir, &error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }

    auto game = std::make_unique<th::Game>();
    if (o.load_path ? !load_from_snapshot(*game, o, data, script) : !start_level(*game, o, data, carried_score))
        return 1;
    if (!th::parse_skipped_calls(o.skip, game->skipped_calls)) {
        std::fprintf(stderr, "--skip: only computer_action is known\n");
        return usage();
    }
    game->events.limit = 0;
    game->events.rand = o.rand;
    game->events.events.clear();

    uint32_t last_turn = uint32_t(std::atol(o.turns_arg.c_str() + (o.turns_arg[0] == '+')));
    if (o.turns_arg[0] == '+')
        last_turn += game->turn_count;

    std::FILE *out = o.out_path ? std::fopen(o.out_path, "w") : o.dump_path || o.render_path ? nullptr : stdout;
    if (o.out_path && !out) {
        std::perror(o.out_path);
        return 1;
    }
    // The original's trace starts with the state as the level starts, then
    // one entry at the end of each turn; every frame is a turn, as it runs
    // with no frame limit.
    if (out)
        std::fputs(th::trace_turn(*game).c_str(), out);
    bool ran = run_turns(*game, o, script, last_turn, out, data);
    if (o.out_path)
        std::fclose(out);
    if (!ran)
        return 1;
    return write_results(*game, o, data);
}

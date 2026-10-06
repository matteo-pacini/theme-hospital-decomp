// th_replay: runs a level headless, from its start or from a snapshot, with
// only scripted input, and writes the replay check's trace
// (core/world/trace.h) of every turn with its events, to compare (compare.py)
// with the instrumented original's (orig_trace.sh); at the end it can dump
// the game's state and render the view.
//
// Usage: th_replay [--data DIR] [--level N] [--load SNAPSHOT] [--turns [+]N] [--script FILE] [--astar] [--rand]
//                  [--out FILE] [--dump FILE [--focus CELL]] [--render OUT.png|OUT.ppm [--camera X,Y] [--size WxH]]
//   --data    the game data folder (default: the development folder)
//   --level   the level to start (default 1); with --load, the snapshot's
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
//   --out     where to write the trace (default: standard output, unless
//             --dump or --render is given)
//   --dump    write dump_game (core/world/debug_dump.h) at the end, with the
//             map around cell --focus (column + row * 128)
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
                         "[--astar] [--rand] [--out FILE] [--dump FILE [--focus CELL]] [--render OUT.png|OUT.ppm "
                         "[--camera X,Y] [--size WxH]]\n");
    return 2;
}

// A script line: what Game::replay_input carries out before turn `turn`.
struct ScriptLine {
    uint32_t turn = 0;
    std::string input;
};

bool read_script(const char *path, std::vector<ScriptLine> &script, std::string &error)
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
    th::draw_world(view, {0, 0, width, height}, camera, game.world, data, &shadows, scratch);
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

} // namespace

int main(int argc, char **argv)
{
#ifdef TH_DEV_DATA_DIR
    std::string data_dir = TH_DEV_DATA_DIR;
#else
    std::string data_dir = ".";
#endif
    int level_number = 1;
    bool level_given = false;
    std::string turns_arg = "2000";
    bool astar = false;
    bool rand = false;
    const char *out_path = nullptr;
    const char *script_path = nullptr;
    const char *load_path = nullptr;
    const char *dump_path = nullptr;
    const char *render_path = nullptr;
    int focus = -1;
    bool camera_given = false;
    th::Point camera{};
    int width = 1280, height = 720;
    for (int i = 1; i < argc; i++) {
        bool has_value = i + 1 < argc;
        if (!std::strcmp(argv[i], "--data") && has_value) {
            data_dir = argv[++i];
        } else if (!std::strcmp(argv[i], "--level") && has_value) {
            level_number = std::atoi(argv[++i]);
            level_given = true;
        } else if (!std::strcmp(argv[i], "--turns") && has_value) {
            turns_arg = argv[++i];
        } else if (!std::strcmp(argv[i], "--script") && has_value) {
            script_path = argv[++i];
        } else if (!std::strcmp(argv[i], "--out") && has_value) {
            out_path = argv[++i];
        } else if (!std::strcmp(argv[i], "--load") && has_value) {
            load_path = argv[++i];
        } else if (!std::strcmp(argv[i], "--dump") && has_value) {
            dump_path = argv[++i];
        } else if (!std::strcmp(argv[i], "--focus") && has_value) {
            focus = std::atoi(argv[++i]);
        } else if (!std::strcmp(argv[i], "--render") && has_value) {
            render_path = argv[++i];
        } else if (!std::strcmp(argv[i], "--camera") && has_value) {
            if (std::sscanf(argv[++i], "%d,%d", &camera.x, &camera.y) != 2)
                return usage();
            camera_given = true;
        } else if (!std::strcmp(argv[i], "--size") && has_value) {
            if (std::sscanf(argv[++i], "%dx%d", &width, &height) != 2 || width <= 0 || height <= 0)
                return usage();
        } else if (!std::strcmp(argv[i], "--astar")) {
            astar = true;
        } else if (!std::strcmp(argv[i], "--rand")) {
            rand = true;
        } else {
            return usage();
        }
    }

    std::string error;
    std::vector<ScriptLine> script;
    if (script_path && !read_script(script_path, script, error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }

    th::GameData data;
    if (!data.load(data_dir, &error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }

    auto game = std::make_unique<th::Game>();
    if (load_path) {
        std::ifstream in(load_path, std::ios::binary);
        std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        if (!in && !in.eof()) {
            std::perror(load_path);
            return 1;
        }
        if (!th::load_snapshot(*game, bytes, data, &error)) {
            std::fprintf(stderr, "%s: %s\n", load_path, error.c_str());
            return 1;
        }
        if (level_given && level_number != game->level_number()) {
            std::fprintf(stderr, "%s is of level %d, not %d\n", load_path, game->level_number(), level_number);
            return 1;
        }
        level_number = game->level_number();
        for (ScriptLine &line : script)
            line.turn += game->turn_count;
    } else {
        auto level = data.load_level(level_number, &error);
        th::Balance settings;
        if (!level || !th::load_balance(data.dir(), level_number, th::kDefaultBalanceId, settings, &error)) {
            std::fprintf(stderr, "%s\n", error.c_str());
            return 1;
        }
        // As the app's begin_level and load_level: the game's start with no
        // tutorial and the wall shadows; the insurers' clock draw does not
        // touch the seed.
        game->staff_market.ref_counts = th::reference_counts(data.text());
        game->start(*level, level_number, settings, data.anims(), data.person_complexes(), data.staff_complexes());
        th::add_static_shadows(game->world.map);
        th::start_advisor(game->advisor, false);
        th::start_hospital_settings(game->hospital, game->settings());
        th::setup_insurances(game->world, 0);
        game->world.path_finder = astar ? th::PathFinder::AStar : th::PathFinder::Original;
    }
    game->events.limit = 0;
    game->events.rand = rand;
    game->events.events.clear();

    uint32_t last_turn = uint32_t(std::atol(turns_arg.c_str() + (turns_arg[0] == '+')));
    if (turns_arg[0] == '+')
        last_turn += game->turn_count;

    std::FILE *out = out_path ? std::fopen(out_path, "w") : dump_path || render_path ? nullptr : stdout;
    if (out_path && !out) {
        std::perror(out_path);
        return 1;
    }
    // The original's trace starts with the state as the level starts, then
    // one entry at the end of each turn; every frame is a turn, as it runs
    // with no frame limit.
    if (out)
        std::fputs(th::trace_turn(*game).c_str(), out);
    size_t next = 0;
    while (game->turn_count < last_turn) {
        for (; next < script.size() && script[next].turn <= game->turn_count + 1; next++) {
            if (script[next].turn == game->turn_count + 1 && !game->replay_input(script[next].input, &error)) {
                std::fprintf(stderr, "%s: %s\n", script_path, error.c_str());
                return 1;
            }
        }
        if (game->paused) {
            std::fprintf(stderr, "the script leaves the game paused before turn %u\n", game->turn_count + 1);
            break;
        }
        game->turn();
        if (out)
            std::fputs(th::trace_turn(*game, &game->events).c_str(), out);
        game->events.events.clear();
    }
    if (out && out != stdout)
        std::fclose(out);

    if (dump_path && !(std::ofstream(dump_path) << th::dump_game(*game, focus))) {
        std::perror(dump_path);
        return 1;
    }
    if (render_path) {
        if (!camera_given) {
            auto level = data.load_level(level_number, &error);
            int start = level ? level->camera_start[0] : 0;
            th::Point centre = th::cell_to_world(start % th::kMapSize, start / th::kMapSize);
            camera = {centre.x - width / 2, centre.y - height / 2};
        }
        if (!render(*game, data, render_path, camera, width, height)) {
            std::perror(render_path);
            return 1;
        }
    }
    return 0;
}

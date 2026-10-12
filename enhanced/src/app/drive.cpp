#include "app/drive.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <format>
#include <fstream>
#include <sstream>
#include <thread>
#include <utility>

#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

namespace th
{

namespace
{

// The frames `turns N` waits at most for its turns, so a paused game cannot
// hang a script.
constexpr int kTurnWaitFrames = 100000;
// A click or key press is held this many frames, then this many pass after
// the release, as a player's would: the panel's buttons act on a press held
// over frames.
constexpr int kHoldFrames = 4;
constexpr int kReleaseFrames = 2;

std::vector<std::string> split_words(const std::string &line)
{
    std::istringstream in(line);
    std::vector<std::string> words;
    for (std::string w; in >> w;)
        words.push_back(w);
    return words;
}

// The SDL button a word names, 1 for left by default.
int button_named(const std::vector<std::string> &words, size_t at)
{
    return at < words.size() && words[at] == "right" ? 3 : 1;
}

bool number(const std::vector<std::string> &words, size_t at, int &out)
{
    if (at >= words.size())
        return false;
    try {
        size_t used = 0;
        out = std::stoi(words[at], &used);
        return used == words[at].size();
    } catch (...) {
        return false;
    }
}

// The text after the command word.
std::string rest_of(const std::string &line, const std::string &word)
{
    size_t at = line.find(word);
    if (at == std::string::npos)
        return {};
    std::string rest = line.substr(at + word.size());
    size_t first = rest.find_first_not_of(" \t");
    return first == std::string::npos ? std::string() : rest.substr(first);
}

// "Shift+Ctrl+NAME" -> the DriveMod bits and the bare key NAME.
uint16_t key_mods(std::string word, std::string &name)
{
    uint16_t mods = 0;
    for (size_t plus; (plus = word.find('+')) != std::string::npos;) {
        std::string mod = word.substr(0, plus);
        if (mod == "Shift")
            mods |= kDriveModShift;
        else if (mod == "Ctrl")
            mods |= kDriveModCtrl;
        else if (mod == "Alt")
            mods |= kDriveModAlt;
        else
            break;
        word.erase(0, plus + 1);
    }
    name = std::move(word);
    return mods;
}

bool read_file(const std::filesystem::path &path, std::string &out)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        return false;
    std::ostringstream text;
    text << file.rdbuf();
    out = text.str();
    return true;
}

// Whether the two files hold the same bytes.
bool same_file(const std::filesystem::path &a, const std::filesystem::path &b)
{
    std::ifstream fa(a, std::ios::binary), fb(b, std::ios::binary);
    if (!fa || !fb)
        return false;
    return std::equal(std::istreambuf_iterator<char>(fa), std::istreambuf_iterator<char>(),
                      std::istreambuf_iterator<char>(fb), std::istreambuf_iterator<char>());
}

// Takes the first free slot of `dir`, writing `info` into it; the open file
// keeps the lock. -1 if all are held, -2 if the files cannot be made.
int try_slots(const std::filesystem::path &dir, int max, const std::string &info)
{
    for (int i = 0; i < max; i++) {
        std::string path = (dir / std::format("slot-{}", i)).string();
        int fd = open(path.c_str(), O_RDWR | O_CREAT | O_CLOEXEC, 0644);
        if (fd < 0)
            return -2;
        if (flock(fd, LOCK_EX | LOCK_NB) == 0) {
            std::time_t now = std::time(nullptr);
            char since[16];
            std::strftime(since, sizeof since, "%H:%M:%S", std::localtime(&now));
            std::string line = std::format("pid {} since {} {}\n", getpid(), since, info);
            if (ftruncate(fd, 0) != 0 || write(fd, line.data(), line.size()) < 0)
                std::fprintf(stderr, "drive: cannot note the holder in %s\n", path.c_str());
            return fd;
        }
        close(fd);
    }
    return -1;
}

} // namespace

bool hold_slot(const std::filesystem::path &dir, int max, const std::string &info)
{
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    bool told = false;
    for (;;) {
        int fd = try_slots(dir, max, info);
        if (fd >= 0)
            return true;
        if (fd == -2)
            return false;
        if (!told)
            std::fprintf(stderr, "drive: waiting for a free slot in %s (%d in use)\n", dir.string().c_str(), max);
        told = true;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

Driver::Driver(std::filesystem::path out, bool control) : out_(std::move(out)), control_(control)
{
    std::error_code ec;
    std::filesystem::create_directories(out_, ec);
}

void Driver::add_line(std::string line)
{
    std::lock_guard lock(mutex_);
    input_.push_back(std::move(line));
}

void Driver::end_input()
{
    std::lock_guard lock(mutex_);
    input_ended_ = true;
}

bool Driver::finished() const
{
    if (quit_)
        return true;
    std::lock_guard lock(mutex_);
    return input_ended_ && input_.empty() && pending_.empty() && wait_frames_ == 0 && until_turn_ < 0;
}

void Driver::reply(std::string text, bool failed)
{
    if (control_) {
        std::printf("%s\n", text.c_str());
        std::fflush(stdout);
    } else if (failed) {
        std::fprintf(stderr, "drive: %s\n", text.c_str());
    }
}

void Driver::defer(std::string text, bool failed)
{
    flush();
    deferred_ = std::move(text);
    deferred_failed_ = failed;
}

void Driver::flush()
{
    if (deferred_.empty())
        return;
    reply(std::exchange(deferred_, {}), deferred_failed_);
}

bool Driver::next_command(Command &out)
{
    if (!pending_.empty()) {
        out = std::move(pending_.front());
        pending_.pop_front();
        return true;
    }
    // The command before has had all its frames: its answer is due.
    flush();
    std::lock_guard lock(mutex_);
    while (!input_.empty()) {
        std::string line = std::move(input_.front());
        input_.pop_front();
        if (size_t hash = line.find('#'); hash != std::string::npos)
            line.resize(hash);
        std::vector<std::string> words = split_words(line);
        if (!words.empty()) {
            out = {line, std::move(words)};
            return true;
        }
    }
    return false;
}

bool Driver::frame(DriveHost &host)
{
    if (wait_frames_ > 0) {
        wait_frames_--;
        return true;
    }
    if (until_turn_ >= 0) {
        if (host.drive_turn() < until_turn_ && --turn_wait_frames_ > 0)
            return true;
        bool reached = host.drive_turn() >= until_turn_;
        if (!reached)
            failures_++;
        reply(reached ? std::string("ok turns")
                      : std::format("fail turns: stopped at turn {}, waiting for {}", host.drive_turn(), until_turn_),
              !reached);
        until_turn_ = -1;
    }
    Command c;
    while (!quit_ && next_command(c)) {
        int frames = run(host, c);
        if (frames > 0) {
            wait_frames_ = frames - 1;
            return true;
        }
        if (until_turn_ >= 0)
            return true;
    }
    return false;
}

int Driver::run(DriveHost &host, const Command &c)
{
    const std::string &verb = c.words[0];
    int n = 0;
    if (verb == "wait") {
        if (!number(c.words, 1, n) || n < 1)
            n = 1;
        defer("ok " + c.line);
        return n;
    }
    if (verb == "turns") {
        if (!number(c.words, 1, n) || n < 1)
            n = 1;
        until_turn_ = host.drive_turn() + n;
        turn_wait_frames_ = kTurnWaitFrames;
        return 0;
    }
    if (verb == "quit") {
        quit_ = true;
        reply("ok quit");
        return 0;
    }
    if (verb == "move" || verb == "cell" || verb == "thing" || verb == "down" || verb == "up" || verb[0] == '_')
        return run_pointer(host, c);
    if (verb == "click" || verb == "click-cell" || verb == "click-thing" || verb == "key" || verb == "keydown" ||
        verb == "keyup")
        return run_click(host, c);
    if (verb == "demand" || verb == "win" || verb == "lose" || verb == "type" || verb == "wheel")
        return run_hook(host, c);
    return run_output(host, c);
}

// The commands that call a hook of the host: demand, win, lose, type and
// wheel.
int Driver::run_hook(DriveHost &host, const Command &c)
{
    const std::string &verb = c.words[0];
    bool ok = false;
    if (verb == "demand") {
        int id = 0;
        ok = number(c.words, 1, id) && host.drive_staff_demand(uint16_t(id));
    } else if (verb == "win") {
        ok = host.drive_win_level();
    } else if (verb == "lose") {
        ok = host.drive_lose_level();
    } else if (verb == "wheel") {
        int notches = 0;
        ok = number(c.words, 1, notches) && host.drive_wheel(notches);
    } else {
        ok = host.drive_text(rest_of(c.line, verb));
    }
    if (!ok)
        failures_++;
    reply((ok ? "ok " : "fail ") + c.line, !ok);
    return 0;
}

// The pointer commands: moves, and pressing or releasing a button or key.
int Driver::run_pointer(DriveHost &host, const Command &c)
{
    const std::string &verb = c.words[0];
    int a = 0, b = 0;
    bool ok = true;
    if (verb == "move")
        ok = number(c.words, 1, a) && number(c.words, 2, b) && host.drive_move_screen(a, b);
    else if (verb == "cell")
        ok = number(c.words, 1, a) && number(c.words, 2, b) && host.drive_move_cell(a, b);
    else if (verb == "thing")
        ok = number(c.words, 1, a) && host.drive_move_thing(uint16_t(a));
    else if (verb == "_keyup")
        ok = host.drive_key(c.words.at(1), false, 0);
    else
        host.drive_button(button_named(c.words, 1), verb == "down" || verb == "_down");
    if (verb == "_down")
        return kHoldFrames;
    if (verb[0] == '_')
        return kReleaseFrames;
    if (!ok)
        failures_++;
    reply((ok ? "ok " : "fail ") + c.line, !ok);
    return 0;
}

// keydown and keyup: a key pressed or released on its own, to hold it.
int Driver::run_key_hold(DriveHost &host, const Command &c)
{
    std::string name;
    uint16_t mods = c.words.size() > 1 ? key_mods(c.words[1], name) : 0;
    bool ok = !name.empty() && host.drive_key(name, c.words[0] == "keydown", mods);
    if (!ok)
        failures_++;
    defer((ok ? "ok " : "fail ") + c.line, !ok);
    return kReleaseFrames;
}

// The commands that press then release: a click (after moving) or a key.
int Driver::run_click(DriveHost &host, const Command &c)
{
    const std::string &verb = c.words[0];
    if (verb == "keydown" || verb == "keyup")
        return run_key_hold(host, c);
    int a = 0, b = 0;
    bool ok = false;
    size_t button_at = 3;
    if (verb == "key") {
        std::string name;
        uint16_t mods = 0;
        if (c.words.size() > 1) {
            mods = key_mods(c.words[1], name);
            ok = !name.empty() && host.drive_key(name, true, mods);
        }
        if (ok)
            pending_.push_front({"_keyup " + name, {"_keyup", name}});
    } else {
        if (verb == "click")
            ok = number(c.words, 1, a) && number(c.words, 2, b) && host.drive_move_screen(a, b);
        else if (verb == "click-cell")
            ok = number(c.words, 1, a) && number(c.words, 2, b) && host.drive_move_cell(a, b);
        else {
            ok = number(c.words, 1, a) && host.drive_move_thing(uint16_t(a));
            button_at = 2;
        }
        // A frame over the target before the press, as a moving pointer
        // gives: the interface acts on a press over what it last hovered.
        std::string button = button_named(c.words, button_at) == 3 ? "right" : "left";
        if (ok) {
            pending_.push_front({"_up " + button, {"_up", button}});
            pending_.push_front({"_down " + button, {"_down", button}});
        }
    }
    if (!ok) {
        failures_++;
        reply("fail " + c.line, true);
        return 0;
    }
    defer("ok " + c.line);
    return verb == "key" ? kHoldFrames : 1;
}

// The expect-report commands: the last report's note or screenshot.
bool Driver::run_report(DriveHost &host, const Command &c, const std::string &text)
{
    const std::string &verb = c.words[0];
    bool note = verb == "expect-report" || verb == "expect-report-none";
    bool shot = verb == "expect-report-shot";
    if (!note && !shot)
        return false;
    std::string dir = host.drive_report_dir();
    bool ok = false;
    std::string body;
    if (shot)
        ok = !text.empty() && !dir.empty() &&
             same_file(std::filesystem::path(dir) / "screenshot.png", out_ / (text + ".png"));
    else if (verb == "expect-report-none")
        ok = dir.empty();
    else
        ok = !dir.empty() && read_file(std::filesystem::path(dir) / "note.txt", body) &&
             body.find(text) != std::string::npos;
    if (!ok)
        failures_++;
    reply((ok ? "ok " : "fail ") + c.line, !ok);
    return true;
}

// Screenshots, state dumps, expectations and messages.
int Driver::run_output(DriveHost &host, const Command &c)
{
    const std::string &verb = c.words[0];
    std::string text = rest_of(c.line, verb);
    if ((verb == "screenshot" || verb == "screenshot-ui") && !text.empty()) {
        std::filesystem::path path = out_ / (text + ".png");
        if (verb == "screenshot")
            host.drive_screenshot(path);
        else
            host.drive_screenshot_ui(path);
        defer(path.string());
        return 1;
    }
    if (verb == "dump" && !text.empty()) {
        std::filesystem::path path = out_ / (text + ".txt");
        std::ofstream(path) << host.drive_state();
        reply(path.string());
        return 0;
    }
    if (run_report(host, c, text))
        return 0;
    if ((verb == "expect" || verb == "expect-not") && !text.empty()) {
        bool found = host.drive_state().find(text) != std::string::npos;
        bool ok = found == (verb == "expect");
        if (!ok)
            failures_++;
        reply((ok ? "ok " : "fail ") + c.line, !ok);
        return 0;
    }
    if (verb == "echo") {
        reply(text);
        if (!control_)
            std::printf("%s\n", text.c_str());
        return 0;
    }
    failures_++;
    reply("fail unknown command: " + c.line, true);
    return 0;
}

} // namespace th

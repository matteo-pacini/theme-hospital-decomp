#include "app/drive.h"

#include "common.h"

#include <filesystem>
#include <format>
#include <string>
#include <vector>

namespace
{

// A host that records what the driver asks, frame by frame.
class FakeHost : public th::DriveHost
{
  public:
    int frame = 0;
    int64_t turn = 0;
    std::vector<std::string> calls;
    std::string state = "screen: hospital\ninterface: requester 9,\n";

    bool drive_move_screen(int x, int y) override
    {
        calls.push_back(std::format("{} move {},{}", frame, x, y));
        return x >= 0 && y >= 0 && x < 640 && y < 480;
    }
    bool drive_move_cell(int col, int row) override
    {
        calls.push_back(std::format("{} cell {},{}", frame, col, row));
        return true;
    }
    bool drive_move_thing(uint16_t id) override
    {
        calls.push_back(std::format("{} thing {}", frame, id));
        return id != 0;
    }
    void drive_button(int button, bool down) override
    {
        calls.push_back(std::format("{} button {} {}", frame, button, down ? "down" : "up"));
    }
    bool drive_key(std::string_view name, bool down) override
    {
        calls.push_back(std::format("{} key {} {}", frame, name, down ? "down" : "up"));
        return name != "Nonsense";
    }
    void drive_screenshot(const std::filesystem::path &) override { calls.push_back(std::format("{} shot", frame)); }
    std::string drive_state() const override { return state; }
    int64_t drive_turn() const override { return turn; }
    bool drive_staff_demand(uint16_t id) override { return id != 0; }
};

// Runs the driver to its end, one frame per call that asks for one; the
// game runs a turn every other frame.
int run_to_end(th::Driver &d, FakeHost &host)
{
    for (int guard = 0; guard < 1000 && !d.finished(); guard++) {
        if (d.frame(host)) {
            host.frame++;
            host.turn += host.frame % 2;
        }
    }
    return host.frame;
}

} // namespace

// Driver (app/drive.h): a click hovers a frame, holds the button 4 frames
// and lets 2 pass after the release; waits, turns, keys and expectations.
void test_drive_script()
{
    std::filesystem::path out = std::filesystem::temp_directory_path() / "th_drive_test";
    th::Driver d(out, false);
    for (const char *line : {"wait 2", "click 10 20  # a comment", "key Escape", "turns 3", "expect requester 9,",
                             "expect-not requester 5,", "expect nothing like this", "click 700 10", "key Nonsense",
                             "screenshot s", "quit", "wait 50"})
        d.add_line(line);
    d.end_input();
    FakeHost host;
    int frames = run_to_end(d, host);
    std::vector<std::string> want = {"2 move 10,20",     "3 button 1 down", "7 button 1 up",        "9 key Escape down",
                                     "13 key Escape up", "21 move 700,10",  "21 key Nonsense down", "21 shot"};
    CHECK(host.calls == want);
    // The failed expectation, the click off the screen and the unknown key;
    // quit stops before the last wait.
    CHECK(d.failures() == 3);
    CHECK(frames == 22);
    CHECK(d.finished());
}

// The debug report (F10 quick report, the F12 tool's save) and the state text
// it and the driven runs' `expect` read.

#include "app/app.h"
#include "app/web.h"
#include "core/data/gamedata.h"
#include "core/world/debug_dump.h"
#include "th_version.h"

#include <algorithm>
#include <ctime>
#include <format>
#include <fstream>

namespace th
{

std::filesystem::path App::report_base() const
{
    const char *dir = std::getenv("TH_DEBUG_REPORT_DIR");
    if (dir && *dir)
        return dir;
    return search_.from_dev_folder || user_paths_.reports_dir.empty() ? std::filesystem::path("debug-reports")
                                                                      : user_paths_.reports_dir;
}

void App::capture_debug_report(DebugCapture &capture)
{
    capture.shot = SDL_RenderReadPixels(renderer_, nullptr);
    capture.shot_error = capture.shot ? std::string() : SDL_GetError();
    capture.state = debug_state();
    capture.snapshot.clear();
    capture.commands.clear();
    capture.has_snapshot = false;
    capture.snapshot_error.clear();
    if (map_ && !in_startup_) {
        map_->game.safeseed = sounds_.safeseed;
        if (save_snapshot(map_->game, capture.snapshot))
            capture.has_snapshot = true;
        else
            capture.snapshot_error = ", without the snapshot";
        for (const std::string &line : map_->game.command_log)
            capture.commands += line + '\n';
    }
}

DebugReportResult App::write_debug_report(const std::string &note, DebugCapture &capture)
{
    namespace fs = std::filesystem;
    DebugReportResult result;
    char stamp[32];
    std::time_t now = std::time(nullptr);
    std::strftime(stamp, sizeof stamp, "%Y%m%d-%H%M%S", std::localtime(&now));
    fs::path base = report_base();
    fs::path dir = base / stamp;
    for (int n = 2; fs::exists(dir); n++)
        dir = base / std::format("{}-{}", stamp, n);
    std::error_code ec;
    fs::create_directories(dir, ec);
    if (ec) {
        result.message = std::format("Could not create {}: {}", dir.string(), ec.message());
        return result;
    }
    bool shot_ok = capture.shot && SDL_SavePNG(capture.shot, (dir / "screenshot.png").string().c_str());
    std::ofstream(dir / "note.txt") << note << '\n';
    std::ofstream(dir / "state.txt") << capture.state;
    if (capture.has_snapshot)
        std::ofstream(dir / "snapshot.bin", std::ios::binary)
            .write(reinterpret_cast<const char *>(capture.snapshot.data()), std::streamsize(capture.snapshot.size()));
    if (map_ && !in_startup_)
        std::ofstream(dir / "commands.txt") << capture.commands;
    result.dir = dir;
    web::download_report(dir);
    web::flush_storage();
    result.message = std::format("Saved to {}", fs::absolute(dir).string());
    result.message += capture.snapshot_error;
    if (!shot_ok)
        result.message += ", without the screenshot: " + (capture.shot ? SDL_GetError() : capture.shot_error);
    return result;
}

void App::save_debug_report()
{
    DebugCapture capture;
    capture_debug_report(capture);
    DebugReportResult result = write_debug_report(tools_.report.note.data(), capture);
    if (capture.shot)
        SDL_DestroySurface(capture.shot);
    tools_.report.last = result.message;
    std::fprintf(stderr, "debug report: %s\n", tools_.report.last.c_str());
}

// F10 pauses and arms the capture; Shift+F10 writes it at once, with no box.
void App::quick_report_key(SDL_Keymod mods)
{
    if (quick_.open || quick_.armed)
        return;
    quick_.note.fill(0);
    quick_.immediate = (mods & SDL_KMOD_SHIFT) != 0;
    quick_.have_pause = map_ && !in_startup_;
    quick_.restore_paused = quick_.have_pause && map_->game.paused;
    if (quick_.have_pause)
        map_->game.paused = true;
    quick_.armed = true;
}

void App::quick_report_step()
{
    if (quick_.armed) {
        quick_.armed = false;
        capture_debug_report(quick_.capture);
        if (quick_.immediate) {
            quick_.immediate = false;
            finish_quick_report("");
        } else {
            quick_.open = true;
            quick_.focus = true;
        }
    }
    if (!quick_.open)
        return;
    QuickNoteAction action = draw_quick_report_note(quick_.note, quick_.focus);
    if (action == QuickNoteAction::Save) {
        quick_.open = false;
        finish_quick_report(quick_.note.data());
    } else if (action == QuickNoteAction::Discard) {
        quick_.open = false;
        discard_quick_report();
    }
}

void App::draw_quick_report()
{
    quick_report_step();
    if (ticks_ms() >= quick_.toast_until)
        quick_.toast.clear();
    if (!quick_.toast.empty())
        draw_toast(quick_.toast);
}

void App::finish_quick_report(const std::string &note)
{
    DebugReportResult result = write_debug_report(note, quick_.capture);
    if (!result.dir.empty()) {
        quick_.last_report = result.dir.string();
        quick_.toast = "Report saved: " + result.dir.string();
        quick_.toast_until = ticks_ms() + 3000;
    }
    tools_.report.last = result.message;
    std::fprintf(stderr, "debug report: %s\n", result.message.c_str());
    if (quick_.capture.shot) {
        SDL_DestroySurface(quick_.capture.shot);
        quick_.capture.shot = nullptr;
    }
    restore_quick_pause();
}

void App::discard_quick_report()
{
    if (quick_.capture.shot) {
        SDL_DestroySurface(quick_.capture.shot);
        quick_.capture.shot = nullptr;
    }
    restore_quick_pause();
}

void App::restore_quick_pause()
{
    if (quick_.have_pause)
        map_->game.paused = quick_.restore_paused;
    quick_.have_pause = false;
}

std::string App::screen_name() const
{
    if (in_startup_)
        return std::format("start-up screen {} (movie {})", int(startup_.screen()), startup_.movie());
    if (!map_)
        return "no level";
    if (year_end_showing_)
        return std::format("year end {} (phase {})", int(year_end_.phase()), year_end_phase_);
    if (winlose_showing_)
        return std::format("win/lose (movie {}{})", winlose_movie_index_, winlose_.has_backdrop() ? ", backdrop" : "");
    if (fax_.open)
        return "fax";
    if (charts_.open)
        return std::format("chart {}", charts_.chart);
    if (town_.is_open())
        return "town map";
    return "hospital";
}

// The start-up screens' lines of the state text.
std::string App::debug_state_startup() const
{
    std::string s = std::format("startup screen: {}\n", int(startup_.screen()));
    if (startup_.screen() == StartupScreen::About) {
        s += std::format("title box: {}\n", startup_.about_box_shown() ? "shown" : "hidden");
        const bool open = startup_.changelog_open();
        const bool end = open && startup_.changelog_scroll() == startup_.changelog_scroll_max();
        s += std::format("changelog: {}, scroll {} of {}{}\n", open ? "open" : "closed", startup_.changelog_scroll(),
                         startup_.changelog_scroll_max(), end ? " (end)" : "");
    }
    s += std::format("continue save: {}\n", save_slot_exists(save_path(kContinueSlot)) ? "yes" : "no");
    return s;
}

std::string App::debug_state() const
{
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(window_, &w, &h);
    float mx, my;
    SDL_GetMouseState(&mx, &my);
    std::string s = "# Theme Hospital Enhanced debug report\n";
    s += std::format("version: {}\n", kVersion);
    s += std::format("data: {}\n", data_ ? data_->dir().string() : "");
    s += std::format("window: {}x{} px, zoom {}, UI scale {}\n", w, h, zoom(), ui_scale());
    s += std::format("screen: {}", screen_name());
    s += std::format("\nfade: {}\nscreen shots: {}", fader_.state(), screen_shots_);
    s += std::format("\nmouse: {},{} (window points), map cell {}", mx, my, map_cell_);
    if (map_cell_ >= 0)
        s += std::format(" (column {}, row {})", map_cell_ % kMapSize, map_cell_ / kMapSize);
    s += std::format("\ninterface: requester {}, modal {}, menu open {}\n", int(interface_.requester()),
                     interface_.modal(), menu_.active);
    s += std::format("menu bar: count {}, row {}, open {}, selected {}, mouse {},{} (game)\n", menu_.count,
                     menu_.line_height, menu_.open, menu_.selected_option, menu_.mouse_x, menu_.mouse_y);
    // The help page shown, if any, and the last tooltip drawn (on one line).
    const HelpBox &help = help_.box();
    bool page = help.style == 2 && !help.msg.empty();
    std::string tooltip = help_.drawn_text();
    std::replace(tooltip.begin(), tooltip.end(), '\n', ' ');
    s += std::format("help: page {}{}, last tooltip \"{}\"\n", page ? "open" : "closed",
                     page ? std::format(" (room type {})", help.sub) : "", tooltip);
    // The query windows (Requester::Patient, Staff, Handyman, Machine) and
    // what they show.
    s += std::format("query window: patient {} (thing {}, {}, toggle delay {}), staff {} (thing {}), handyman {}, "
                     "machine {} (thing {}); pointer {}, staff list on {}\n",
                     interface_.is_open(int(Requester::Patient)), info_.patient,
                     info_.show_graph ? "health graph" : "treatments", info_.toggle_delay,
                     interface_.is_open(int(Requester::Staff)), info_.staff,
                     interface_.is_open(int(Requester::Handyman)), interface_.is_open(int(Requester::Machine)),
                     info_.machine, pointer_sprite_, charts_.slist_staff);
    s += std::format("quick report: {}, last report: {}\n", quick_.open ? "open" : "closed",
                     quick_.last_report.empty() ? "(none)" : quick_.last_report);
    if (in_startup_)
        s += debug_state_startup();
    if (!data_) {
        s += std::format("setup: game folder {}, {}, notice \"{}\", error \"{}\"\n",
                         game_dir_exists_ ? (game_dir_incomplete_ ? "incomplete" : "complete") : "absent",
                         job_.busy() ? "job running" : "idle", setup_notice_, data_error_);
    }
    if (!map_ || in_startup_)
        return s;
    const Game &g = map_->game;
    s += debug_state_hospital(g);
    s += "\n";
    s += debug_state_calls(g);
    s += "\n";
    s += dump_game(g, map_cell_);
    return s;
}

std::string App::debug_state_hospital(const Game &g) const
{
    std::string s;
    s += std::format("level: {} ({}), tutorial {}\n", level_, balance_id_, tutorial_);
    std::string slots;
    for (int slot = 1; slot <= kMenuSlots; slot++)
        if (save_slots_[size_t(slot)])
            slots += std::format(" {}", slot);
    s += std::format("save slots:{}\n", slots);
    s += std::format("camera: {},{}\n", camera_.x, camera_.y);
    s += std::format("speed: {} turns/s, paused {}\n", int(speed_), g.paused);
    s += std::format(
        "options: autosave {}, shadows {}, sound {} volume {}, music {} volume {}, announcements {} volume {}\n",
        details_.autosave != 0, shadows_, sounds_.sfx_on(), sounds_.sfx_volume(), music_.music_on(), music_.volume(),
        sounds_.ambients_on(), sounds_.ambient_volume());
    s += std::format("route planner: {}\n", g.world.path_finder == PathFinder::AStar ? "A*" : "original");
    s += std::format("panel info: kind {}, thing {}, shown {}\n", panel_info_, panel_thing_,
                     panel_info_drawn_ != 0 ? "true" : "false");
    s += std::format("panel message: \"{}\" ({} frames)\n", panel_message_, panel_message_time_);
    s += std::format("placing: object {}, staff {}, build mouse {}\n", g.world.placing.placing, hire_.placing(),
                     int(g.world.building.mouse));
    s += std::format("flashing: pickup thing {}, queue thing {}\n", build_ui_.pickup_thing, queue_.hilight);
    s += std::format("date: year {} month {} day {}, turn {}\n", g.date.year, g.date.month, g.date.day, g.turn_count);
    s += std::format("open: {}, opening countdown {}\n", g.open, g.opening_countdown);
    s += std::format("tutorial flash: counter {}, drawn {}\n", interface_.flash().counter, interface_.flash().drawn);
    std::string message = g.advisor.text;
    std::ranges::replace(message, '\n', ' ');
    s += std::format("advisor: tutorial {}, step {:#x}, highlight {:#x}, message \"{}\"\n",
                     (g.advisor.flags & kAdvisorTutorial) != 0, g.advisor.tutorial_step, g.advisor.tutorial_highlight,
                     message);
    s += std::format("advisor enabled: {}\n", g.advisor.enabled);
    s += std::format("balance: {}, reputation {}\n", g.balance, g.arrivals.hospitals[0].hospital_reputation);
    s += std::format("last command: type={} value={} flags={} arg={}\n", last_event_.type, last_event_.value,
                     last_event_.flags, last_event_.arg);
    s += std::format("casebook: illness {}, price scale {}; staff list: filter {}, page {}, member {}\n",
                     charts_.selected_illness, g.arrivals.hospitals[0].price_scale[charts_.selected_illness],
                     charts_.slist_filter, charts_.slist_page, charts_.value);
    s += "sounds played:";
    for (int i = 0; i < kSfxCount; i++)
        if (uint32_t n = sounds_.played_counts()[size_t(i)])
            s += std::format(" {:#04x}={};", i, n);
    return s;
}

std::string App::debug_state_calls(const Game &g) const
{
    // The panel's fax icons as last drawn, and the call each stands for.
    std::string s =
        std::format("fax icons: hovered kind {}, emergency flash {}\n", popups_.hovered, popups_.emergency_frame);
    auto icon = [&](const char *where, const PopupIcons::Icon &i) {
        uint16_t head = g.world.first_call[0][i.kind];
        const World::PlayerCall &c = g.world.calls[head];
        s += std::format("  {} kind {} call {} (id {}) subject {} timer {} slot {} slide_step {} at {},{}\n", where,
                         i.kind, head, c.id, c.subject, c.timer, c.slot, c.slide_step, i.x, i.y);
    };
    for (uint8_t i = 0; i < popups_.rising_count; i++)
        icon("rising", popups_.rising[i]);
    for (uint8_t i = 0; i < popups_.level_count; i++)
        icon("level", popups_.level[i]);
    if (!fax_.open)
        return s + "fax: closed\n";
    // The open fax: its call, answers and the button under the mouse.
    const UiMouse &m = fax_.pending_mouse;
    std::vector<Rect> boxes = fax_answer_buttons(fax_);
    int hovered = -1;
    for (size_t i = 0; i < boxes.size(); i++)
        if (m.x >= boxes[i].x && m.x < boxes[i].x + boxes[i].w && m.y >= boxes[i].y && m.y < boxes[i].y + boxes[i].h)
            hovered = int(i);
    s += std::format("fax: kind {} subject {} timeout {} state {:#x}, enabled {:#x}, hovered answer {}, dialled {}\n",
                     fax_.choice.type, fax_.choice.subject, fax_.choice.timeout, fax_.state, fax_.available, hovered,
                     fax_.dialled_count);
    for (size_t i = 0; i < fax_.buttons.size(); i++)
        s += std::format("  answer {}: {}\n", i, fax_.buttons[i]);
    return s;
}

} // namespace th

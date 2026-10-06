#pragma once

#include "core/data/palette.h"
#include "core/data/sprites.h"
#include "core/data/text.h"
#include "core/render/surface.h"
#include "core/ui/layer.h"
#include "core/world/events.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace th
{

class Game;
struct Balance;
struct World;
enum class PanelAction;

// The full-screen charts, numbered as the game's chart_type (EV_OPEN_SCREEN's
// value, the keys F1 to F8 and the Charts menu's items).
enum ChartType : int {
    kChartStatement = 0, // bank statement (bank_statement, src/finance.cpp)
    kChartCasebook = 1,  // display_drugs, src/drugs.cpp
    kChartPolicy = 2,    // display_policy, src/policy.cpp
    kChartResearch = 3,  // display_research, src/rschscrn.cpp
    kChartGraphs = 4,    // draw_graphs, src/graph.cpp
    kChartStaff = 5,     // display_slist, src/slist.cpp
    kChartBank = 6,      // display_manager, src/manager.cpp
    kChartStatus = 7,    // display_peep_overview, src/overscr.cpp
};
inline constexpr int kChartCount = 8;

// The illnesses and treatments (the game's ILL_*, expertise[] entries).
inline constexpr int kIllnessCount = 47;

// The rest of what the casebook shows for one illness or treatment (the
// game's allhosps[].rooms[] entry): its price, price_scale and reputation
// are HospitalMarket's (core/world/arrivals.h), its cure_available
// World::cure_available.
struct IllnessFigures {
    // The hospital knows it (expertise Known).
    uint8_t discovered = 0;
    int32_t income = 0;
    uint16_t cured = 0;
    uint16_t deaths = 0;
    uint16_t turned_away = 0;
    // Drug cures: effectiveness in percent, and the cost of a dose
    // (administer_drugs).
    uint16_t drug_effectiveness = 0;
    int32_t drug_cost = 0;
};

// The player's hospital settings the charts show and change through
// commands, which Game does not hold elsewhere: the AllHosps and gamevar
// fields of policy, research and the casebook, and the insurers' claims.
// Set it up with start_hospital_settings when a level starts and change it
// with apply_chart_event.
struct HospitalSettings {
    // Policy (EV_SET_POLICY): send patients home below this chance of a
    // cure, guess a cure above it (0-100, send_home <= guess_cure); diagnose
    // up to diag_threshold (1000-2000); staff rest at rest_threshold
    // tiredness (0-1000); staff may leave rooms (the pager option).
    int send_home_threshold = 10;
    int guess_cure_threshold = 90;
    int diag_threshold = 1000;
    int rest_threshold = 0;
    bool staff_leave_rooms = true;
    // Percent of research per category (cure, diagnosis, drugs,
    // improvements, specialisation); -1 closed.
    std::array<int8_t, 5> research_alloc{20, 20, 20, 20, 20};
    // The illness research concentrates on (gamevar.research_illness), 0 for
    // none.
    uint8_t research_illness = 0;
    std::array<IllnessFigures, kIllnessCount> illnesses{};
    // allhosps.insurance_claims: fees owed by each of the level's three
    // insurance companies (World::ins_control[0-2]), by the month they pay.
    std::array<std::array<int32_t, 12>, 3> insurance_claims{};
};

// init_player's part (src/start.cpp) for the player: default policy (rest
// at gbv.tired), research split evenly, each illness's knowledge (expertise
// Known) and drug effectiveness and cost (setup_drug: gbv.start_rating and
// start_cost).
void start_hospital_settings(HospitalSettings &settings, const Balance &balance);

// setup_insurances (0x90f70), single player: the level's three insurance
// companies (World::ins_control[0-2]), each drawn from the eight left by the
// game's timer, for which `clock` stands.
void setup_insurances(World &world, uint32_t clock);

// The game's side of the charts' commands for the player, as process_turn
// (src/io.cpp): EV_OPEN_SCREEN (the hospital valued for the bank manager and
// the status screen, every hospital's prices updated: update_cure_costs),
// EV_TAKE_LOAN and EV_REPAY_LOAN ($5000 and a statement line; repaying needs
// $5000 in the bank), EV_SET_POLICY, EV_ALTER_RESEARCH,
// EV_CONCENTRATE_RESEARCH, EV_PRICE_UP and EV_PRICE_DOWN (hospital 0's
// price_scale and price, in Game::arrivals), EV_INCREASE_STAFF_SALARIES and
// EV_PAY_STAFF_BONUS (paid only if the balance covers it). EV_OPEN_SCREEN
// first puts a picked-up member of staff back (put_back_held_staff). Not
// ported: the discontent a raise clears. Returns whether `event` was one of these.
bool apply_chart_event(Game &game, HospitalSettings &settings, const Event &event);

// The keys the charts read: Escape and Space pressed since the last frame
// (lbInkey), the arrows held (lbKeyOn).
struct ChartKeys {
    bool escape = false;
    bool space = false;
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
};

// What a frame of the charts asks of the caller.
struct ChartsResult {
    // The player's command, for Game::event (none if type is EV_NONE).
    Event event;
    // The sound effects played (play_sound; SFX_* of include/enums.h), in
    // order. The research screen's bubbling (SFX_RESEARCH on its own
    // source, stopped when it closes) is among them, once.
    std::vector<uint8_t> sounds;
    // The bank manager's town map hotspot was clicked (key T: display_mode
    // DM_TOWN_MAP).
    bool town_map = false;
    // With event EV_OPEN_YES_NO (the staff list's sack button,
    // open_yes_no_requester): the commands the yes/no requester is to send
    // for yes and for no, and the question, a string of
    // TextSection::YesNo.
    Event yes;
    Event no;
    int question = 0;
};

// The charts' state and art: the game's display_mode DM_CHARTS with
// chart_type and each screen's state. Each chart is drawn into `surface`
// (640 x 480) with its own palette, from its QDATA picture and sprites as
// load_ingame_pic shows it; the palette fades and the help texts are not
// drawn. The game keeps running while a chart is up (display_mode_control
// runs turns in DM_CHARTS); the panel, menus, requesters and map are not
// shown and take no input.
struct Charts {
    // A chart is up; `chart` says which.
    bool open = false;
    int chart = kChartStatement;
    // The hospitals' names (allhosps[i].name): [0] the player's, which the
    // caller sets; the rivals' are chosen when a level starts
    // (setup_computer_players), which is not ported.
    std::array<std::string, 4> hospital_names;

    Surface surface;
    Palette palette{};

    // What follows is the charts' own state.
    struct Picture {
        Surface pixels;
        Palette palette{};
        std::array<uint8_t, 768> vga{};
        SpriteSheet sprites;
    };
    std::array<Picture, kChartCount> pictures;
    std::array<Font, kFontSlots> fonts;
    std::vector<uint8_t> faces;
    const LanguageText *text = nullptr;
    Palette game_palette{};

    // The shown chart's Menu state and value (bank.state, manager.value, ...).
    int state = 0;
    int value = 0;
    // Game frames are run every 40 ms; input waits for the next one.
    double frame_time = 0;
    UiMouse pending_mouse;
    ChartKeys pending_keys;
    // othervar.clock_ms, advanced 40 a game frame; the price and research
    // buttons repeat by it.
    uint32_t clock_ms = 0;
    int repeat_step = 0x100;
    uint32_t repeat_time = 0;
    // The presentation's random generator (saferand), for the bank
    // manager's face.
    uint32_t safeseed = 0x546b2;
    // The palette shown before the chart: colours the game looks up as the
    // chart opens are found in it.
    Palette previous_palette{};
    // Casebook.
    uint8_t selected_illness = 1; // general practice
    int drug_pct_display_time = 0;
    // Graphs: which lines are drawn and the scale (0 months, 1 years, 2
    // four-year spans).
    std::array<uint8_t, 8> graph_visible{1, 1, 1, 1, 1, 1, 1, 1};
    int graph_period = 0;
    // Staff list: the kinds shown (bits: 1 doctors, 2 nurses, 4 handymen, 8
    // receptionists), the page and the row under the mouse.
    int slist_filter = 1;
    int slist_page = 0;
    int slist_cursor = 0;
    // othervar.slist_staff: the member of staff the list opens on, as the
    // staff window's face sets it (kept after, as in the game); 0 for none.
    uint16_t slist_staff = 0;
    // Bank manager: its face animations' frames, the graph's line colours
    // and the sprites its insurance buttons are drawn pressed with.
    std::array<int, 3> face_frame{};
    uint8_t line_colour = 0;
    uint8_t line_shadow_colour = 0;
    std::array<int, 3> insurance_button_sprites{3, 3, 3};
    // Status: each pie slice's pull-out.
    std::array<int, 4> pie_pull{};
};

// Loads every chart's picture, palette and sprites (hiresscreens,
// screenpalettes and hiresscreen_sprites_*, src/files.cpp), their fonts and
// the faces file from `dir`. `text` is kept by reference; `game_palette` is
// the in-game palette. On failure says why in `error`.
bool load_charts(Charts &charts, const std::filesystem::path &dir, const LanguageText &text,
                 const Palette &game_palette, std::string *error = nullptr);

// The interface's side of a command, as process_turn for the local player:
// EV_OPEN_SCREEN opens chart `value` (0-7) at its first state, from any
// chart or the game; EV_RETURN_FROM_SCREEN, EV_CLOSE_SCREEN,
// EV_RETURN_TO_THING_WINDOW and EV_OPEN_YES_NO (whose requester the game
// shows over the hospital) close it. Run it before Game::process_turn,
// which clears the command.
void chart_command(Charts &charts, const Event &event);

// Runs the open chart for `seconds` of time with this mouse (in the
// surface's coordinates) and keys: a game frame per 40 ms passed, at most
// one per call; presses, releases and keys wait for it. Draws the chart
// into `surface`. Nothing if no chart is open.
ChartsResult chart_frame(Charts &charts, const UiMouse &mouse, const ChartKeys &keys, double seconds, const Game &game,
                         const HospitalSettings &settings);

// The command opening chart `chart` (0-7) from a key F1-F8, the Charts menu or
// the panel: EV_OPEN_SCREEN; the research screen is refused (with
// SFX_WRONG2) without a research room. The keys do nothing while the game is
// paused (process_keys): the caller checks that.
Event open_chart_command(int chart, const Game &game, bool *refused = nullptr);

// The chart a panel button opens (BankManager, BankStatement,
// StaffManagement, Casebook, Research, Status, Charts, Policy), -1 for
// others.
int chart_for_panel_action(PanelAction action);

} // namespace th

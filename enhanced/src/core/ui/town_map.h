#pragma once

#include "core/data/level.h"
#include "core/data/palette.h"
#include "core/data/sprites.h"
#include "core/data/text.h"
#include "core/render/surface.h"
#include "core/ui/layer.h"
#include "core/world/events.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace th
{

class Game;
class HelpSystem;
struct HelpContext;
struct PanelState;
struct Thing;

// Plot numbers a level may use (for_sale and land_amount hold 200).
inline constexpr int kPlotCount = 200;

// The level's land as src/town.cpp keeps it: the level file, whose tiles a
// bought plot is copied from, with the plots already owned taken out of its
// plot map (recreate_map), and who owns each plot (for_sale).
struct TownLand {
    Level level;
    int level_number = 0;
    // Per plot, the owner's bit (1 << player), 0x10 for the public land
    // (plot 0); 0 while it is for sale.
    std::array<uint8_t, kPlotCount> for_sale{};
};

// recreate_map(0)'s part for the land when level `number` starts: the
// public land and the plots of the hospitals that play (1 to the level's
// player count) are owned and leave the plot map; the rest is for sale.
void start_town_land(TownLand &land, const Level &level, int number);

// calculate_land_amount for `plot`: its cells still for sale.
int land_amount(const TownLand &land, int plot);

// can_buy_plot (0x63d6c): `plot` if player `player` may buy it, that is if
// the level has no neighbour rule for it (adjacent_plots, src/tables.cpp)
// or it borders a plot the player owns; 0 otherwise; 1 for plot 0.
int can_buy_plot(const TownLand &land, int plot, int player = 0);

// buy_land (0x64db0) for the local player, as EV_BUY_LAND carries it out:
// the plot's cells are copied from the level with the objects it places,
// become the player's, and leave the plot map; the partition walls are
// taken down and put up again, the grass relaid, the cost (cells times
// gbv.land_cost_per_tile) paid as kTxBuyLand and the static shadows redone.
// Rooms' walls are not recoloured (recolour_all_rooms: no rooms are built
// yet). As in the game, a copied cell's list of things is replaced by the
// object the level puts there, or emptied.
void buy_land(Game &game, TownLand &land, int plot);

// The town map's commands for the local player (process_turn,
// src/io.cpp): EV_BUY_LAND (buy_land) and EV_SET_HEATING (the heating
// level becomes value; radiators do not break yet). Returns whether `event`
// was one of them.
bool apply_town_event(Game &game, TownLand &land, const Event &event);

// What a frame of the town map asks of the caller.
struct TownMapResult {
    // The map closed (town.state 1, return_from_screen): back to the game.
    bool closed = false;
    // A command for Game::event (EV_BUY_LAND, EV_SET_HEATING, or the
    // bank manager's EV_OPEN_SCREEN 6), EV_NONE for none.
    Event event;
    // Sound effects to play (SFX_* numbers), in order.
    std::vector<uint8_t> sounds;
};

// A frame's input to the town map: the mouse (640 x 480) and whether Escape
// was pressed.
struct TownMapInput {
    UiMouse mouse;
    bool escape = false;
};

// The town map (display_mode DM_TOWN_MAP; src/town.cpp, draw_town_select
// and its keys and mouse): full screen in its own picture and palette
// (QDATA/TOWN01V, sprites TOWN02V), 640 x 480. It shows the hospital's
// land, walls and doors, its people, plants, fire extinguishers, machines
// and radiators (each overlay switched by a button; a right click shows
// only that one), the plots for sale and, for the cell under the mouse, its
// plot, owner, size and price; the counts of each overlay, the heating
// level with its two buttons and monthly cost, and the balance, rolling as
// the panel's. A left click on a plot the player may buy and afford buys
// it; a right click on the map centres the view there and closes the map;
// the balance opens the bank manager. The game keeps running behind it.
// The help tooltips and the network auction are not ported; the app
// fades the screen in and out (ScreenFader).
class TownMap
{
  public:
    // Loads the picture, palette and sprites, fonts 24-26 of the cache and
    // the in-game blend table (QDATA/GHOST66). `text` is the language's,
    // kept by reference. On failure says why in `error`.
    bool load(const std::filesystem::path &dir, const LanguageText &text, std::string *error = nullptr);

    // display_mode DM_TOWN_MAP with town.state 0: the next frame sets the
    // map up. The caller cancels what the player is building or placing
    // (setup_abort_all_requesters), as that frame does.
    void open() { state_ = State::Opening; }
    // display_mode leaving DM_TOWN_MAP for another screen (the bank manager
    // its balance opens): the map stops until open() is called again.
    void close() { state_ = State::Closed; }
    bool is_open() const { return state_ != State::Closed; }

    // One game frame of the town map, before Game::process_turn: draws it
    // into surface() and handles `input`.
    // `hospital_name` is the player's hospital's (allhosps[0].name).
    // `panel` holds the balance shown and its roll, which the panel shares.
    // A right click on the map calls `go_to(column, row)`, which centres the
    // view on that cell and returns true, or leaves it and returns false if
    // it would pass the scroll limits (the map then stays open).
    // When set_help was called, the map's help tooltip is drawn over it while
    // shown.
    TownMapResult frame(const TownMapInput &input, const Game &game, const TownLand &land, PanelState &panel,
                        std::string_view hospital_name, const std::function<bool(int column, int row)> &go_to);

    // The help system and context the map's tooltip is drawn from (the caller
    // owns both; the context's mouse is input.mouse).
    void set_help(HelpSystem &system, HelpContext &context)
    {
        help_ = &system;
        help_ctx_ = &context;
    }

    const Surface &surface() const { return surface_; }
    const Palette &palette() const { return palette_; }
    // town.value: the overlays shown, bit 0 people, 1 plants, 2 fire
    // extinguishers, 3 machines, 4 radiators. All five when first opened.
    unsigned overlays() const { return value_; }

  private:
    enum class State { Closed, Opening, Shown, Closing };
    struct Colours {
        uint8_t white = 0, dark_blue = 0, green = 0, orange = 0, light_red = 0, red = 0, cyan = 0, purple = 0, grey = 0,
                blue = 0, pale_green = 0;
    };

    void draw_overhead_walls(const Game &game);
    void display_overhead_map(const Game &game, const TownLand &land, const UiMouse &mouse,
                              std::string_view hospital_name, PanelState &panel);
    void draw_map(const Game &game, const TownLand &land);
    void draw_map_cell(const Game &game, const TownLand &land, int x, int y);
    void draw_things(const Game &game);
    // Object `t`'s mark on the map at (px, py), if its overlay is on.
    void draw_object_on_map(const Thing &t, int px, int py, int heating);
    // The panel's counts of patients and objects and the heating bar.
    void draw_map_counts(const Game &game);
    void draw_plot_under_mouse(const Game &game, const TownLand &land, const UiMouse &mouse,
                               std::string_view hospital_name);
    void draw_money(PanelState &panel, int balance);
    bool process_panel(const UiMouse &mouse, const Game &game, TownMapResult &result);
    // process_town_button: panel button `i` released, the right button if
    // `right`.
    void process_town_button(size_t i, bool right, int heating, TownMapResult &result);
    // The map's setup as it opens.
    void open_map(const TownLand &land);
    // process_mouse_town_select's left click: buys the plot under the mouse
    // if it is for sale and affordable.
    void buy_plot(const UiMouse &m, const Game &game, const TownLand &land, TownMapResult &result);
    void box(int x, int y, int w, int h, uint8_t colour);
    void text(const Font &font, int x, int y, std::string_view s);

    const LanguageText *text_ = nullptr;
    HelpSystem *help_ = nullptr;
    HelpContext *help_ctx_ = nullptr;
    Surface picture_;
    Surface background_; // bscreen: the picture with the walls drawn at setup
    Surface surface_;
    Palette palette_{};
    std::array<uint8_t, 768> vga_{};
    SpriteSheet sprites_;
    std::vector<uint8_t> ghost_;
    Font detail_font_, count_font_, money_font_; // cache slots 0x18, 0x19, 0x1a
    Colours colours_;

    State state_ = State::Closed;
    unsigned value_ = 0;
    int heat_icon_time_ = 0;
    bool dirty_ = false;
    std::array<uint16_t, kPlotCount> land_amount_{};
};

} // namespace th

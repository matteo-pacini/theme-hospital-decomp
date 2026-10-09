// Theme Hospital Enhanced: draws a level's map from the game's own data at
// any window size, with the bottom panel centred at native size and ImGui
// developer tools (F12).

#include "app/audio/mixer.h"
#include "app/datadir.h"
#include "app/drive.h"
#include "app/settings.h"
#include "app/ui/tools.h"
#include "core/audio/music.h"
#include "core/audio/sounds.h"
#include "core/data/fs.h"
#include "core/data/gamedata.h"
#include "core/render/blit.h"
#include "core/render/iso.h"
#include "core/render/panel.h"
#include "core/render/shadows.h"
#include "core/render/surface.h"
#include "core/render/view.h"
#include "core/ui/build_room.h"
#include "core/ui/buy_objects.h"
#include "core/ui/charts.h"
#include "core/ui/fax.h"
#include "core/ui/helpmsg.h"
#include "core/ui/hire_staff.h"
#include "core/ui/info_windows.h"
#include "core/ui/jukebox.h"
#include "core/ui/layer.h"
#include "core/ui/menubar.h"
#include "core/ui/panel_info.h"
#include "core/ui/popups.h"
#include "core/ui/queue_window.h"
#include "core/ui/staff_demand.h"
#include "core/ui/startup.h"
#include "core/ui/town_map.h"
#include "core/ui/windows.h"
#include "core/ui/winlose.h"
#include "core/ui/yearend.h"
#include "core/world/build.h"
#include "core/world/debug_dump.h"
#include "core/world/game.h"
#include "core/world/icons.h"
#include "core/world/rooms.h"
#include "core/world/savegame.h"
#include "core/world/snapshot.h"
#include "core/world/vaccine.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <utility>

namespace th
{
namespace
{

// The view never shows less than the original's 640x480; automatic zoom
// picks the largest integer scale that keeps at least this many lines.
constexpr int kMinViewHeight = 480;
constexpr int kEdgeScrollMargin = 8; // pixels from the window edge
// The scroll keys' bits (move_windows' scroll.up/down/left/right) and the
// velocity they add each frame (x moves twice as fast as y, as the game's
// 2:1 tiles do).
constexpr uint8_t kScrollUp = 1, kScrollDown = 2, kScrollLeft = 4, kScrollRight = 8;
constexpr int32_t kScrollAccelX = 0x80000, kScrollAccelY = 0x40000;
constexpr int32_t kScrollDecay = 0xcd; // the velocity's per-frame decay, * 0xcd >> 8
// move_windows runs once a game frame, which limit_framerate caps at 40 ms.
constexpr double kGameFrame = 0.04;

// The scroll bit an arrow or numpad key asks for, or 0 (src/keyboard.cpp's
// KC_UP/KC_NUMPAD8 and so on).
uint8_t scroll_key_bit(SDL_Keycode key)
{
    switch (key) {
    case SDLK_UP:
    case SDLK_KP_8:
        return kScrollUp;
    case SDLK_DOWN:
    case SDLK_KP_2:
        return kScrollDown;
    case SDLK_LEFT:
    case SDLK_KP_4:
        return kScrollLeft;
    case SDLK_RIGHT:
    case SDLK_KP_6:
        return kScrollRight;
    default:
        return 0;
    }
}
// The in-game pointers drawn: the default one and those hovering on the map
// shows (core/ui/info_windows.h), all with their hotspot at the top-left
// corner.
constexpr uint8_t kPointersDrawn[] = {kPointerDefault, kPointerStaff,     kPointerMachine,        kPointerPatient,
                                      kPointerQueue,   kPointerVaccinate, kPointerVaccinateTarget};

// The folder chosen in the native dialog, handed over from its callback,
// which SDL may call on another thread.
struct FolderPick {
    std::mutex mutex;
    std::optional<std::string> chosen;
};

void SDLCALL on_folder_chosen(void *userdata, const char *const *files, int)
{
    if (!files || !files[0])
        return; // error or cancelled
    auto *pick = static_cast<FolderPick *>(userdata);
    std::lock_guard lock(pick->mutex);
    pick->chosen = files[0];
}

// A level as it starts: its map and objects, the shadowed blocks, and the
// scratch surface the view is drawn with.
struct LevelView {
    Game game;
    std::unique_ptr<ShadowSprites> shadows;
    Surface scratch;
};

// A debug report's contents, held in memory between the capture (at F10 or
// the F12 button) and the write (when the note box is answered).
struct DebugCapture {
    SDL_Surface *shot = nullptr;
    std::string shot_error;
    std::string state;
    std::vector<uint8_t> snapshot;
    bool has_snapshot = false;
    std::string snapshot_error;
    std::string commands;
};

// The F10 quick report: the capture held while its note box is up.
struct QuickReport {
    bool armed = false;     // F10 pressed: capture at the frame's start
    bool open = false;      // the note box is up
    bool immediate = false; // Shift+F10: write at once, with no box
    bool focus = false;     // focus the note box's text field
    bool have_pause = false;
    bool restore_paused = false;
    DebugCapture capture;
    std::array<char, 4096> note{};
    std::string last_report; // the folder of the last report written
    std::string toast;
    uint32_t toast_until = 0;
};

// What write_debug_report did.
struct DebugReportResult {
    std::string message;
    std::filesystem::path dir;
};

// Starts level `number` with its settings (the balance set `balance_id`:
// "EASY", "FULL" or "HARD"), or returns nothing and says why.
std::unique_ptr<LevelView> begin_level(const GameData &data, const Level &level, int number,
                                       std::string_view balance_id, std::string *error)
{
    Balance settings;
    if (!load_balance(data.dir(), number, balance_id, settings, error))
        return nullptr;
    auto view = std::make_unique<LevelView>();
    view->game.staff_market.ref_counts = reference_counts(data.text());
    view->game.start(level, number, settings, data.anims(), data.person_complexes(), data.staff_complexes());
    view->shadows = std::make_unique<ShadowSprites>(data.blocks(), data.ghost());
    add_static_shadows(view->game.world.map);
    return view;
}

// Draws the hospital view into `surface`.
void draw_view(Surface &surface, const GameData &data, LevelView *level, Point camera, bool shadows,
               const DrawnThings &drawn = {})
{
    surface.fill(0);
    if (level) {
        if (level->scratch.width() != surface.width() || level->scratch.height() != surface.height()) {
            level->scratch.resize(surface.width(), surface.height());
            level->scratch.fill(kMaskColour);
        }
        draw_world({surface, {0, 0, surface.width(), surface.height()}, camera}, level->game.world, data,
                   shadows ? level->shadows.get() : nullptr, level->scratch, drawn);
    }
}

// Whether the hospital has a receptionist (the announcer's pages wait for
// one).
bool has_receptionist(const World &world)
{
    for (const Thing &t : world.things)
        if (t.type == kThingStaff && t.subtype == kStaffReceptionist)
            return true;
    return false;
}

// Where `src` goes in an `out_w` x `out_h` window, as large as fits whole and
// centred.
SDL_FRect fit_rect(const Surface &src, int out_w, int out_h)
{
    int sw = std::max(src.width(), 1), sh = std::max(src.height(), 1);
    float fit = std::min(float(out_w) / float(sw), float(out_h) / float(sh));
    float dw = sw * fit, dh = sh * fit;
    return {(out_w - dw) / 2, (out_h - dh) / 2, dw, dh};
}

// A static texture of `width` x `height` 0xAARRGGBB pixels, scaled without
// smoothing.
SDL_Texture *argb_texture(SDL_Renderer *renderer, const std::vector<uint32_t> &pixels, int width, int height)
{
    SDL_Texture *t = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, height);
    if (!t)
        return nullptr;
    SDL_UpdateTexture(t, nullptr, pixels.data(), width * 4);
    SDL_SetTextureScaleMode(t, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    return t;
}

// The row `y` of `img`'s 32-bit pixels.
uint32_t *surface_row(SDL_Surface *img, int y)
{
    return reinterpret_cast<uint32_t *>(static_cast<uint8_t *>(img->pixels) + y * img->pitch);
}

// screenshot's view: `surface` in the game's palette over the whole of `img`.
void screenshot_view(SDL_Surface *img, const Surface &surface, const GameData &data)
{
    const uint8_t *src = surface.data();
    for (int y = 0; y < img->h; y++) {
        uint32_t *row = surface_row(img, y);
        for (int x = 0; x < img->w; x++)
            row[x] = data.palette()[*src++];
    }
}

// screenshot's panel: at native size with the level's starting figures, at
// the bottom centre of `img`, as the window draws it.
void screenshot_panel(SDL_Surface *img, const GameData &data, const Game &g)
{
    int width = img->w, height = img->h;
    PanelFigures figures{g.balance, g.date.day, g.date.month, g.reputation(), g.opening_countdown, g.open_countdown};
    int pw = 0, ph = 0;
    std::vector<uint32_t> panel = panel_to_argb(data, &figures, pw, ph);
    for (int y = 0; y < ph && height - ph + y >= 0; y++) {
        uint32_t *row = surface_row(img, height - ph + y);
        for (int x = 0; x < pw; x++) {
            int dx = (width - pw) / 2 + x;
            if (dx >= 0 && dx < width && panel[size_t(y * pw + x)])
                row[dx] = panel[size_t(y * pw + x)];
        }
    }
}

// --screenshot: renders level `level_number` at `width` x `height` with the view
// centred on the player's start, without opening a window, and saves it as
// a BMP. Returns the process exit code.
int screenshot(const std::filesystem::path &data_dir, int level_number, int width, int height, const std::string &out,
               std::optional<int> language)
{
    GameData data;
    std::string error;
    if (!data.load(data_dir, &error, language)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }
    auto level = data.load_level(level_number, &error);
    if (!level) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }
    auto map = begin_level(data, *level, level_number, kDefaultBalanceId, &error);
    if (!map) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }
    int start = level->camera_start[0];
    Point centre = cell_to_world(start % kMapSize, start / kMapSize);
    Surface surface;
    surface.resize(width, height);
    draw_view(surface, data, map.get(), {centre.x - width / 2, centre.y - height / 2}, true);

    SDL_Surface *img = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_XRGB8888);
    if (!img) {
        std::fprintf(stderr, "SDL_CreateSurface: %s\n", SDL_GetError());
        return 1;
    }
    screenshot_view(img, surface, data);
    screenshot_panel(img, data, map->game);
    bool ok = SDL_SaveBMP(img, out.c_str());
    if (!ok)
        std::fprintf(stderr, "SDL_SaveBMP: %s\n", SDL_GetError());
    SDL_DestroySurface(img);
    return ok ? 0 : 1;
}

// The game speed a Speed menu item's value picks.
Speed menu_speed(int value)
{
    return value <= 10   ? Speed::Slowest
           : value <= 20 ? Speed::Slower
           : value <= 30 ? Speed::Normal
           : value <= 40 ? Speed::Max
                         : Speed::AndThenSome;
}

// saferand (src/rand.cpp), for the year-end presentation's letter variants.
uint16_t saferand(uint32_t &seed)
{
    seed = seed * 0x24a1 + 0x24df;
    seed = (seed >> 13) | (seed << 19);
    return uint16_t(seed);
}

// A key pressed in the start-up screens, which read keys as lbInkey.
StartupKey startup_key(SDL_Keycode key)
{
    StartupKey k;
    if (key >= SDLK_A && key <= SDLK_Z) {
        k.code = StartupKey::Letter;
        k.letter = char('A' + (key - SDLK_A));
    } else if (key == SDLK_ESCAPE) {
        k.code = StartupKey::Escape;
    } else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
        k.code = StartupKey::Return;
    } else if (key == SDLK_BACKSPACE || key == SDLK_KP_PERIOD) {
        k.code = StartupKey::Backspace;
    } else if (key == SDLK_SPACE) {
        k.code = StartupKey::Space;
    }
    return k;
}

class App : public DriveHost
{
  public:
    bool init(int argc, char **argv);
    void run();
    void shutdown();
    // 1 when a drive script's expectations failed, else 0.
    int exit_code() const { return driver_ && driver_->failures() > 0 ? 1 : 0; }

  private:
    // --drive SCRIPT, --control, --out DIR, --size WxH, --visible.
    void parse_drive_args(int argc, char **argv);
    // One of TH_DRIVE_SLOTS (default 6) slots shared by every driven run on
    // the machine, under the repository's build/slots/drive.
    void hold_drive_slot();
    bool create_window(bool driving);
    void start_driver();
    // Milliseconds since start: the real clock, or the driven one, which
    // moves a fixed step per frame so driven runs repeat exactly.
    uint32_t ticks_ms() const;
    // Window points of map pixel `p` in the hospital view, false off the
    // window.
    bool drive_warp_world(Point p);
    bool drive_move_screen(int x, int y) override;
    bool drive_move_cell(int col, int row) override;
    bool drive_move_thing(uint16_t id) override;
    void drive_button(int button, bool down) override;
    bool drive_key(std::string_view name, bool down, uint16_t mods = 0) override;
    bool drive_text(std::string_view text) override;
    void drive_screenshot(const std::filesystem::path &path) override;
    void drive_screenshot_ui(const std::filesystem::path &path) override;
    std::string drive_state() const override;
    std::string drive_report_dir() const override;
    int64_t drive_turn() const override;
    bool drive_staff_demand(uint16_t id) override;

    bool try_load_data(const std::filesystem::path &dir);
    void load_level(int number);
    // Takes over `view` as the level in play and resets the interface round
    // it. `fresh` starts the advisor, the hospital settings and the insurers
    // as a level's start does; a loaded save has them already.
    void start_view(std::unique_ptr<LevelView> view, const Level &level, bool fresh);
    // The save slot path for the player's name and the current save folder.
    std::filesystem::path save_path(int slot) const;
    // What a save of the game in play continues with: the balance id, the
    // tutorial and the view's camera (the level is the game's).
    SaveInfo current_save() const;
    // Marks which File > Save/Load slots hold a save (check_save_games).
    void refresh_save_slots();
    // Whether the game may be saved now (check_okay_to_save): in the normal
    // mouse mode with nothing being built, placed or edited.
    bool can_save(const Game &game) const;
    // Saves the game to `slot` (save_game) or loads `slot` over a level
    // (load_game); both show the panel message the original shows (0x18
    // saved, 0x19 cannot save, 0x1c load failed). perform_load returns
    // whether it loaded.
    void perform_save(int slot);
    bool perform_load(int slot);
    // display_panel_special_info: the misc_text message `id` on the panel's
    // information bar for `frames` frames (the timer is one a frame).
    void show_panel_message(uint8_t id, int frames);
    void draw_panel_special_info();
    void draw_panel_bar(const Game &game);
    // Saves the Continue slot when the game returns to the main menu
    // (quit_game 1: save_game(0x63)).
    void save_continue_slot();
    void handle_event(const SDL_Event &e);
    void key_down(const SDL_KeyboardEvent &key);
    void key_up(const SDL_KeyboardEvent &key);
    void game_key(const SDL_KeyboardEvent &key);
    // Alt+S quick saves and Alt+L quick loads debug_save_slot; true when
    // `key` was one of them (keyboard.cpp's KC_S and KC_L).
    bool quick_save_load_key(const SDL_KeyboardEvent &key);
    // The speed keys 1-5: true when `key` is one and the speed was set.
    bool game_speed_key(const SDL_KeyboardEvent &key);
    void open_chart_key(int chart);
    void mouse_down(const SDL_MouseButtonEvent &button);
    void inspect_click(uint8_t button);
    void mouse_up(const SDL_MouseButtonEvent &button);
    // The mouse in a layer drawn at (ox, oy) at `scale`.
    UiMouse ui_mouse(int ox, int oy, int scale) const;
    void menu_step(int out_w, int scale);
    void perform(const MenuEvent &event);
    // File > Save and File > Load (pulldown_save and pulldown_load),
    // perform's, apart for their length.
    void perform_save_load(const MenuEvent &event);
    void play_menu_sound(MenuSound sound);
    void open_menu_screen(int chart);
    void update(float dt);
    void run_turns(float dt);
    // move_windows (src/cakewin.cpp): moves the view by the scroll direction
    // `dir` (the arrow keys' bits, 1 up, 2 down, 4 left, 8 right, plus the
    // window edge), accelerating while held and stopping at the scroll limit.
    void scroll_step(uint8_t dir);
    // The scroll direction this frame: the held keys, the pointer's window
    // edge replacing them (process_mouse's flags), an open pulldown ignoring
    // both (io.cpp), and opposite keys cancelling (process_game_keys).
    uint8_t scroll_direction();
    void render();
    // Reads the render target back and saves it as a PNG (a driven run's
    // screenshot).
    void save_drive_screenshot(const std::filesystem::path &path);
    // The frame of what takes the whole window, if anything: the start-up
    // screens, the fax, a chart or the town map. False when it is the
    // hospital's.
    bool screen_step(int out_w, int out_h);
    void hospital_step(int out_w, int out_h);
    void resize_view(int vw, int vh);
    void view_sounds(int vw, int vh);
    void draw_view_overlays(int vw, int vh);
    void present_view(int vw, int vh, int z);
    int zoom() const;
    int ui_scale() const;
    void make_ui_textures();
    // Frees the textures, cursors and surfaces the app owns.
    void destroy_textures();
    void interface_step(int out_w, int out_h, int scale);
    bool interface_escape(Game *game);
    InfoWindowResult interface_requesters(Game &game, const InterfaceEvents &events, const UiMouse &mouse);
    // The jukebox, hire staff and staff demand requesters' clicks, split from
    // interface_requesters.
    void interface_jukebox_input(Game &game, const InterfaceEvents &events, const UiMouse &mouse);
    void interface_hire_input(Game &game, const InterfaceEvents &events);
    void interface_staff_demand_input(Game &game, const InterfaceEvents &events);
    void interface_yes_no(Game *game, const InterfaceEvents &events);
    void interface_panel_action(Game &game, PanelAction action, bool placing);
    // The panel's edit room button (panel_l_button): toggles the edit-room
    // mouse mode. Returns whether it refused.
    bool panel_edit_room(Game &game);
    void interface_map_mouse(Game &game, const UiMouse &mouse, bool on_map, bool placing);
    // The map's right button this frame (process_right_button): the edit-room
    // modes, picking up or cancelling. Returns the command, EV_NONE for none.
    Event interface_map_right(Game &game, bool placing);
    void interface_map_hover(Game &game, const UiMouse &mouse, bool on_map, bool placing);
    // The watch button's highlight sound (draw_emergency's othervar.watch_hovered):
    // played once on entering it, when the button is drawn (epidemic, or the
    // opening countdown past the tutorial flag).
    void interface_watch_hover(Game &game, const UiMouse &mouse);
    // The watch's buttons over the panel (process_left_button): the epidemic's
    // vaccination button and the opening countdown's open-now button. True
    // when one took the left click.
    bool interface_watch_buttons(Game &game, const UiMouse &mouse);
    // The pointer and left click of interface_map_hover for what it is over.
    void interface_map_hover_click(Game &game, const UiMouse &mouse, const MapHover &hover);
    // interface_map_hover's near rat: the near pointer and a miss on a click;
    // true if it handled it.
    bool interface_near_rat(Game &game, const UiMouse &mouse, const MapHover &hover);
    // After interface_map_hover while the queue window is open: its panel
    // information, the drag pointer and the door hotspot's scroll (req7_overdraw).
    void queue_window_after_frame(Game &game);
    void interface_advisor(Game &game, const UiMouse &mouse, bool input, bool paused);
    void interface_command(Game &game, const InfoWindowResult &info);
    void interface_draw(Game *game, int ox, int oy, int scale);
    // The help tooltip of the screen open this frame (src/helpmsg.cpp): the
    // group depends on the front requester.
    void interface_help();
    // The text font (cache slots 0 and 0x2d), the requester frame art and the
    // blend table the help system draws with.
    void load_help_art(const std::filesystem::path &dir);
    // compute_help_text for help group `group` at (x, y), with a context of
    // the in-game interfaces.
    void help_screen(int group, int x, int y);
    // Fills help_ctx_ for the current frame (the game, the data and the mouse
    // the last interface frame saw) and sets the help clock.
    void help_context();
    // Points the full-screen screens at help_ and help_ctx_ (once, after the
    // data and help art are loaded).
    void wire_screen_help();
    // build.cpp's show_help_page(0, type): a room type's page the first time
    // one is built.
    void interface_room_help(Game &game);
    // Draws the prepared help box onto the interface layer.
    void help_draw();
    // Whether a framed help page is open.
    bool help_page_open() const { return help_.box().style == 2 && !help_.box().msg.empty(); }
    void draw_info_view(Game &game);
    void ask_quit();
    void ask_restart();
    void draw_pointer(int scale, uint8_t sprite = kPointerDefault);
    void startup_step(int out_w, int out_h);
    // Carries out what the start-up screens chose (startup_step's result).
    void startup_result(const StartupResult &r);
    // The debug report tool's save: the window as drawn so far, the state
    // (debug_state) and the note, in a new folder under debug-reports/.
    void save_debug_report();
    // The F10 quick report: pause at the press, capture the frame and state
    // into memory, and either write at once (Shift+F10) or show the note box.
    void quick_report_key(SDL_Keymod mods);
    void quick_report_step();
    // The note box and the save confirmation of the F10 quick report.
    void draw_quick_report();
    void capture_debug_report(DebugCapture &capture);
    // Writes a held capture to a new debug-reports/<time>/ folder.
    DebugReportResult write_debug_report(const std::string &note, DebugCapture &capture);
    void finish_quick_report(const std::string &note);
    void discard_quick_report();
    void restore_quick_pause();
    // Where a report goes: debug-reports/ under the folder the game was
    // started from, or TH_DEBUG_REPORT_DIR when set (the drive tests).
    std::filesystem::path report_base() const;
    std::string debug_state() const;
    // What fills the window this frame, for debug_state.
    std::string screen_name() const;
    // debug_state's hospital lines: the level, saves, camera, panel, date,
    // money, the last command and the sounds played.
    std::string debug_state_hospital(const Game &g) const;
    // debug_state's fax icons and open fax.
    std::string debug_state_calls(const Game &g) const;
    void open_town_map();
    void town_step(int out_w, int out_h);
    void charts_step(int out_w, int out_h);
    void fax_step(int out_w, int out_h);
    // The year-end screens (the hall of fame, the league tables and awards)
    // and the win/lose sequence.
    void check_year_end();
    void open_year_end();
    void year_end_step(int out_w, int out_h);
    void open_winlose();
    int lose_line_index(int anim);
    void winlose_step(int out_w, int out_h);
    // Plays the win/lose movie (index -1 when none): its frames and audio, as
    // startup_step plays a start-up movie.
    void open_winlose_movie(int index);
    void advance_winlose_movie();
    void load_ui_art(const std::filesystem::path &dir);
    // The sounds and announcements of what the game did since the last
    // frame: new faxes, the emergency's helicopter sent, its clock.
    void game_sounds(Game &game);
    // Carries out the frame's command (Game::event): the interface's part,
    // then the game's (process_turn).
    void run_command(Game &game);
    // The mouse in `src` drawn at `at` (from fit_rect).
    UiMouse fitted_mouse(const SDL_FRect &at, const Surface &src) const;
    // Draws `src` in `palette` into `dst` of the window through `tex`.
    void draw_fitted(const Surface &src, const Palette &palette, SDL_Texture *&tex, const SDL_FRect &dst);

    SDL_Window *window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;
    SDL_Texture *screen_ = nullptr;
    int screen_w_ = 0, screen_h_ = 0;
    // The panel and the pointer, drawn at the UI scale over the view.
    // The game's interface (panel and requesters), drawn in its 640 x 480
    // canvas.
    UiLayer ui_;
    SDL_Texture *ui_tex_ = nullptr;
    PanelState panel_state_;
    Interface interface_;
    HospScores hosp_scores_;
    RequesterArt requester_art_;
    // Furnishing and hiring (milestone 4 steps 2 and 3).
    // Sound effects: the bank, the game's rules for playing them and the
    // device they are mixed to.
    SoundBank bank_;
    SoundPlayer sounds_;
    MusicPlayer music_; // the songs of SOUND/MIDI, played on an emulated OPL3
    Mixer mixer_;
    std::vector<FrameSound> frame_sounds_;
    ObjectUi object_ui_;
    HireStaff hire_;
    HireStaffArt hire_art_;
    StaffDemand demand_;
    // The panel information bar's kind and the thing under the pointer
    // (othervar.panel_info and panel_thing).
    uint8_t panel_info_ = 0;
    uint16_t panel_thing_ = 0;
    // The bar's kind as the last frame drew it, 0 when the raised panel
    // covered it (draw_panel_info's return).
    uint8_t panel_info_drawn_ = 0;
    // othervar.panel_info_timer: frames the bar stays once the pointer leaves.
    int panel_info_timer_ = 0;
    // othervar.watch_hovered: the pointer is over the watch button, so the
    // highlight sound plays once on entering it.
    bool watch_hovered_ = false;
    // othervar.panel_message and panel_message_time: the special message on
    // the panel's information bar (display_panel_special_info).
    std::string panel_message_;
    int panel_message_time_ = 0;
    BuildUi build_ui_;
    Jukebox jukebox_;
    // The help system (tooltips and room help pages), its text font (cache
    // slot 0) and the mouse the last interface frame saw.
    HelpSystem help_;
    Font help_font_;
    Font help_menu_font_;
    UiMouse help_mouse_;
    // The help context the full-screen screens share, filled by help_context.
    HelpContext help_ctx_;
    // Which room types have shown their help page, and the last room built.
    std::array<bool, kRoomTypes> room_help_seen_{};
    uint8_t last_built_room_ = 0;
    // The town map: while open it takes the window from the view, the
    // interface and the menu bar; the game keeps running behind it.
    TownMap town_;
    SDL_Texture *town_tex_ = nullptr;
    // The full-screen charts (F1-F8, the Charts menu, the panel), shown in
    // place of everything else while open, the game running behind them.
    Charts charts_;
    SDL_Texture *charts_tex_ = nullptr;
    // The fax (the choice screen), shown in place of everything else while
    // open, the game running behind it; and the fax icons above the panel.
    Fax fax_;
    SDL_Texture *fax_tex_ = nullptr;
    SDL_Texture *year_end_tex_ = nullptr;
    SDL_Texture *winlose_tex_ = nullptr;
    PopupIcons popups_;
    // The year-end screens (the hall of fame, the league tables and awards)
    // and the win/lose sequence: shown in place of everything else, the game
    // paused behind them.
    YearEndScreen year_end_;
    WinLoseScreen winlose_;
    // The emergency's helicopter at the last frame, for its announcement.
    uint16_t heli_seen_ = 0;
    // The thing under the pointer as the view was drawn this frame.
    PointedThing pointed_;
    // othervar.near_rat: the pointer is in a rat's near box but not its hit
    // box. Reset before each draw, set by draw_world.
    bool near_rat_ = false;
    // The start-up screens (intro, menus, the level's area movie), shown
    // until they start a level; skipped with --level.
    Startup startup_;
    std::vector<StartupKey> startup_keys_; // keys pressed since the last frame
    SDL_Texture *startup_tex_ = nullptr;
    SDL_AudioStream *movie_stream_ = nullptr;
    // The win/lose movie: its frames (into winlose_movie_surface_) and audio,
    // played before the letter as startup_step plays a start-up movie.
    SmackerVideo winlose_movie_;
    int winlose_movie_index_ = -1;
    unsigned winlose_movie_flags_ = 0;
    double winlose_movie_time_ = 0;
    Surface winlose_movie_surface_;
    Palette winlose_movie_palette_{};
    std::vector<int16_t> winlose_movie_audio_;
    std::string balance_id_{kDefaultBalanceId};
    Event last_event_; // the command carried out the frame before (the tutorial reads it)
    // The commands the yes/no requester sends for yes and for no when it
    // asks before one (othervar.yes_type and no_type); EV_NONE otherwise.
    Event yes_event_;
    Event no_event_;
    // The slot File > Save is overwriting, waiting for the yes/no answer
    // (pulldown_save's EV_CLOSE_SAVE_YES_NO); 0 for none.
    int pending_save_slot_ = 0;
    // The slot Alt+S and Alt+L use (debug.save_slot).
    int debug_save_slot_ = 0;
    // The pointers of kPointersDrawn, by number.
    struct PointerTexture {
        SDL_Texture *texture = nullptr;
        int w = 0, h = 0;
    };
    std::array<PointerTexture, kPointerQueue + 1> pointers_{};
    // The system crosshair, shown over the game while the inspector picks.
    SDL_Cursor *crosshair_ = nullptr;
    bool crosshair_shown_ = false;
    // The patient, staff and handyman windows, and the view of their person
    // drawn into them (its surface and draw_world's scratch).
    InfoWindows info_;
    Surface info_view_;
    Surface info_scratch_;
    // The queue window (src/req7.cpp).
    QueueWindow queue_;

    Settings settings_;
    DataSearch search_;
    std::string data_error_;
    std::filesystem::path hiscore_path_;
    // The folder the saved games live in, and which File > Save/Load slots
    // hold one ([0] is slot 1).
    std::filesystem::path save_dir_;
    std::array<bool, kMenuSlots + 1> save_slots_{};
    FolderPick pick_;
    std::unique_ptr<GameData> data_;

    std::optional<int> language_; // --language: LANG-n.DAT instead of HOSPITAL.CFG's
    std::unique_ptr<LevelView> map_;

    Surface surface_;
    Point camera_;                                         // map pixel position of the view's top-left
    uint16_t scroll_min_ = 0, scroll_max_ = kMapCells - 1; // the level's scroll area, as cells
    // The scroll keys held (move_windows' scroll.*), and the view's velocity
    // in 1/65536 pixel a frame with the fraction carried between frames.
    uint8_t scroll_held_ = 0;
    int32_t scroll_vel_x_ = 0, scroll_vel_y_ = 0;
    int32_t scroll_frac_x_ = 0, scroll_frac_y_ = 0;
    double scroll_frame_time_ = 0;      // real time accrued towards the next move_windows frame
    float scroll_x_ = 0, scroll_y_ = 0; // the mouse drag's sub-pixel pixels, carried between frames
    double turn_clock_ = 0;             // turns due but not yet run
    ToolsState tools_;
    QuickReport quick_;
    // The game's menu bar, drawn into its own layer across the top of the
    // window at the UI scale.
    MenuBar menu_;
    UiLayer menu_layer_;
    SDL_Texture *menu_tex_ = nullptr;

    int level_ = 1;
    int year_end_phase_ = 0; // 0 the hall, 1 the charts
    int last_year_ = 0;      // the game's year the last frame saw
    int winlose_pick_ = 0;   // the letter variant drawn
    // The map cell under the pointer this frame (column + row * 128), -1
    // when off the map.
    int map_cell_ = -1;
    int movie_playing_ = -1;  // the movie movie_stream_ plays
    float frame_seconds_ = 0; // the last frame's length
    int zoom_ = 0;            // the view's scale: 0 automatic, else the integer scale
    Speed speed_ = Speed::Normal;
    // The pointer shown over the hospital (othervar.pointer_sprite).
    uint8_t pointer_sprite_ = kPointerDefault;
    // The epidemic's vaccination tool chosen (mouse mode MM_VACCINATE): a
    // click on the watch's vaccination button toggles it, a click on a
    // highlighted patient queues or unqueues them.
    bool vaccinate_ = false;
    bool open_town_ = false; // the T key asked for the town map this frame
    bool year_end_showing_ = false;
    bool winlose_showing_ = false;
    bool restart_level_ = false;
    bool winlose_to_menu_ = false; // the losing letter asked for the main menu
    bool chart_space_ = false;     // Space pressed for the charts this frame
    bool in_startup_ = false;      // the start-up screens are shown
    bool level_from_cli_ = false;
    bool tutorial_ = false;    // the advisor's tutorial, chosen in the start-up screens
    bool escape_ = false;      // Escape pressed this frame
    bool asking_quit_ = false; // the yes/no requester asks whether to quit
    bool dragging_ = false;
    bool shadows_ = true; // the game's shadows option
    // The mouse buttons as the game's interface sees them; the pressed and
    // released flags last one frame.
    struct {
        bool left = false, right = false;
        bool left_pressed = false, right_pressed = false;
        bool left_released = false, right_released = false;
    } buttons_;
    bool running_ = true;
    bool imgui_ready_ = false;

    // Driven runs (drive.h): the script or control input, its options, the
    // driven clock and the screenshot asked for this frame.
    std::unique_ptr<Driver> driver_;
    std::filesystem::path drive_script_;
    std::filesystem::path drive_out_ = "drive-out";
    bool drive_control_ = false;
    bool drive_visible_ = false;
    int drive_w_ = 1280, drive_h_ = 720;
    uint64_t drive_ns_ = 0;
    std::filesystem::path drive_shot_;
    std::filesystem::path drive_shot_ui_;
    // The text of the `type` commands waiting to be polled: SDL keeps the
    // pointer, so the strings outlive SDL_PushEvent until the next poll.
    std::deque<std::string> drive_texts_;
    // The full-screen screen drawn last (draw_fitted): where, its size and
    // the frame, for driven clicks in its 640 x 480 coordinates.
    struct {
        SDL_FRect at{};
        int w = 0, h = 0;
        uint64_t frame = 0;
    } fit_;
    uint64_t frame_ = 0;
};

bool App::init(int argc, char **argv)
{
    std::optional<std::filesystem::path> data_arg;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--data" && i + 1 < argc)
            data_arg = argv[++i];
        else if (arg == "--level" && i + 1 < argc) {
            level_ = std::atoi(argv[++i]);
            level_from_cli_ = true;
        } else if (arg == "--language" && i + 1 < argc)
            language_ = std::atoi(argv[++i]);
    }
    parse_drive_args(argc, argv);
    bool driving = !drive_script_.empty() || drive_control_;
    if (driving)
        hold_drive_slot();
    if (driving && !drive_visible_) {
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen");
        SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");
    }

    if (!create_window(driving))
        return false;

    // A driven run does not depend on, or change, the player's settings.
    if (driving)
        start_driver();
    else
        settings_.load();
    search_ = find_data_dir(data_arg, settings_);
    if (search_.found)
        try_load_data(*search_.found);
    return true;
}

// SDL, the window and its renderer, and ImGui; a driven run's window is the
// --size it asked for, at a scale of 1.
bool App::create_window(bool driving)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return false;
    }
    float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    if (scale <= 0 || driving)
        scale = 1.0f;
    int win_w = driving ? drive_w_ : 1280, win_h = driving ? drive_h_ : 720;
    window_ = SDL_CreateWindow("Theme Hospital Enhanced", int(float(win_w) * scale), int(float(win_h) * scale),
                               driving ? 0 : SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!window_) {
        std::fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return false;
    }
    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (!renderer_) {
        std::fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        return false;
    }
    // A driven run's frames go as fast as they can: its clock is its own.
    if (!driving)
        SDL_SetRenderVSync(renderer_, 1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGui::GetStyle().ScaleAllSizes(scale);
    ImGui::GetStyle().FontScaleDpi = scale;
    ImGui_ImplSDL3_InitForSDLRenderer(window_, renderer_);
    ImGui_ImplSDLRenderer3_Init(renderer_);
    imgui_ready_ = true;
    return true;
}

bool App::try_load_data(const std::filesystem::path &dir)
{
    auto data = std::make_unique<GameData>();
    std::string error;
    if (!data->load(dir, &error, language_)) {
        data_error_ = error;
        return false;
    }
    data_ = std::move(data);
    load_ui_art(dir);
    data_error_.clear();
    // The saved games: in the user's preferences folder, or under a driven
    // run's output folder, which is removed between runs, so tests start
    // with no saves.
    if (driver_)
        save_dir_ = drive_out_ / "save";
    else
        save_dir_ = settings_.path().empty() ? std::filesystem::path("save") : settings_.path().parent_path() / "save";
    startup_.set_save_dir(save_dir_);
    // The start-up screens choose the level, unless --level did.
    if (!level_from_cli_ && startup_.load(dir, data_->text(), data_->language(), &error))
        in_startup_ = true;
    else
        load_level(level_);
    return true;
}

void App::load_ui_art(const std::filesystem::path &dir)
{
    std::string error;
    auto report = [&](bool ok) {
        if (!ok)
            std::fprintf(stderr, "%s\n", error.c_str());
    };
    report(menu_.load(dir, data_->text(), &error));
    report(requester_art_.load(dir, &error));
    report(hire_art_.load(dir, &error));
    report(town_.load(dir, data_->text(), &error));
    report(load_charts(charts_, dir, data_->text(), data_->palette(), &error));
    report(load_fax(fax_, dir, data_->text(), &error));
    report(year_end_.load(dir, data_->text(), data_->palette(), &error));
    report(winlose_.load(dir, data_->text(), data_->palette(), &error));
    hiscore_path_ = dir / "save" / "hiscore.dat";
    init_object_ui(object_ui_, data_->palette());
    report(bank_.load(dir, data_->language(), &error));
    sounds_.set_bank(&bank_);
    if (!music_.load(dir, &error))
        std::fprintf(stderr, "no music: %s\n", error.c_str());
    if (!mixer_.is_open() && !mixer_.open(&error))
        std::fprintf(stderr, "no sound: %s\n", error.c_str());
    help_.setup_page_indices(data_->text());
    load_help_art(dir);
    wire_screen_help();
    make_ui_textures();
}

void App::load_level(int number)
{
    std::string error;
    auto level = data_->load_level(number, &error);
    if (!level) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return;
    }
    auto map = begin_level(*data_, *level, number, balance_id_, &error);
    if (!map) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return;
    }
    start_view(std::move(map), *level, true);
}

// Takes over `view` as the level in play and resets the interface round it.
// A loaded save has the advisor, the hospital settings and the insurers in
// the snapshot already, so `fresh` skips their level-start setup. `level` is
// the level file, for its camera start and scroll area.
void App::start_view(std::unique_ptr<LevelView> view, const Level &level, bool fresh)
{
    level_ = view->game.level_number();
    map_ = std::move(view);
    close_irequesters(interface_);
    info_ = {};
    if (fresh) {
        start_advisor(map_->game.advisor, tutorial_);
        start_hospital_settings(map_->game.hospital, map_->game.settings());
        setup_insurances(map_->game.world, ticks_ms());
    }
    charts_.hospital_names[0] = startup_.player_name();
    charts_.open = false;
    fax_.open = false;
    popups_ = {};
    heli_seen_ = 0;
    town_.close();
    year_end_.close();
    year_end_showing_ = false;
    winlose_.close();
    winlose_showing_ = false;
    winlose_movie_index_ = -1;
    // A message from the screen before this game (e.g. a load failure) is not
    // shown on the new level.
    panel_message_.clear();
    panel_message_time_ = 0;
    last_year_ = map_->game.date.year;
    panel_state_.shown_balance = map_->game.balance;
    sounds_.safeseed = kLevelStartSeed;
    // A level starts on a song picked by the clock, as the game does.
    music_.start_level(ticks_ms());
    sounds_.init_ambients();
    // Centre the view on the player's starting cell, as load_level does.
    int start = level.camera_start[0];
    scroll_min_ = level.scroll_min;
    scroll_max_ = level.scroll_max;
    Point centre = cell_to_world(start % kMapSize, start / kMapSize);
    camera_ = {centre.x - surface_.width() / 2, centre.y - surface_.height() / 2};
    refresh_save_slots();
}

std::filesystem::path App::save_path(int slot) const
{
    return save_slot_path(save_dir_, startup_.player_name(), slot);
}

SaveInfo App::current_save() const
{
    SaveInfo info;
    info.balance_id = balance_id_;
    info.tutorial = tutorial_;
    // The view's centre, not its top-left: it is restored the same whatever
    // the view surface's size, including when it is 0x0 on a first load.
    info.view_centre_x = camera_.x + surface_.width() / 2;
    info.view_centre_y = camera_.y + surface_.height() / 2;
    return info;
}

// check_save_games (src/menu.cpp): which File > Save/Load slots hold a save,
// from the files in the save folder.
void App::refresh_save_slots()
{
    save_slots_.fill(false);
    for (int slot = 1; slot <= kMenuSlots; slot++)
        save_slots_[size_t(slot)] = save_slot_exists(save_path(slot));
}

// check_okay_to_save (src/io.cpp): saving needs the normal mouse mode, with
// nothing being placed, built or edited and no modal requester up.
bool App::can_save(const Game &game) const
{
    if (interface_.modal() || hire_.placing() || game.world.placing.placing)
        return false;
    return game.world.building.mouse == BuildMouse::None && !build_tool_busy(game);
}

// The panel messages the save and load show (misc_text indices, io.cpp):
// saved, cannot save and load failed.
constexpr uint8_t kPanelMessageSaved = 0x18;
constexpr uint8_t kPanelMessageCannotSave = 0x19;
constexpr uint8_t kPanelMessageLoadFailed = 0x1c;

// display_panel_special_info: the message text and its timer in frames.
void App::show_panel_message(uint8_t id, int frames)
{
    if (!data_)
        return;
    panel_message_ = std::string(data_->text().at(TextSection::Misc, id));
    panel_message_time_ = frames;
}

// save_game (src/io.cpp): the whole game to the slot's file.
void App::perform_save(int slot)
{
    if (!map_ || !data_)
        return;
    if (!can_save(map_->game)) {
        show_panel_message(kPanelMessageCannotSave, 0x14);
        return;
    }
    std::string error;
    if (!write_save_slot(save_path(slot), map_->game, current_save(), &error)) {
        std::fprintf(stderr, "save slot %d: %s\n", slot, error.c_str());
        show_panel_message(kPanelMessageCannotSave, 0x14);
        return;
    }
    show_panel_message(kPanelMessageSaved, 3);
    refresh_save_slots();
}

// load_game (src/io.cpp): the save at the slot's file, over a fresh level;
// false, with the load-failed message, when it cannot be loaded.
bool App::perform_load(int slot)
{
    if (!data_)
        return false;
    std::string error;
    auto view = std::make_unique<LevelView>();
    SaveInfo info;
    if (!read_save_slot(save_path(slot), view->game, *data_, &info, &error)) {
        std::fprintf(stderr, "load slot %d: %s\n", slot, error.c_str());
        show_panel_message(kPanelMessageLoadFailed, 0x46);
        return false;
    }
    auto level = data_->load_level(info.level, &error);
    if (!level) {
        std::fprintf(stderr, "%s\n", error.c_str());
        show_panel_message(kPanelMessageLoadFailed, 0x46);
        return false;
    }
    view->shadows = std::make_unique<ShadowSprites>(data_->blocks(), data_->ghost());
    // The settings and the view the save continues with (window and
    // scroll_y, src/data.cpp:22-23, restored by load_game io.cpp:2067-2093).
    if (!info.balance_id.empty())
        balance_id_ = info.balance_id;
    tutorial_ = info.tutorial;
    start_view(std::move(view), *level, false);
    // The view centre over the surface's centre: with a 0x0 surface (a first
    // load from the start-up screens) this leaves the centre, and resize_view
    // moves the camera to it once the view has its size.
    camera_ = {info.view_centre_x - surface_.width() / 2, info.view_centre_y - surface_.height() / 2};
    return true;
}

// save_game(0x63) when the game returns to the main menu.
void App::save_continue_slot()
{
    if (!map_ || !data_ || !can_save(map_->game))
        return;
    std::string error;
    if (!write_save_slot(save_path(kContinueSlot), map_->game, current_save(), &error))
        std::fprintf(stderr, "continue save: %s\n", error.c_str());
    else
        show_panel_message(kPanelMessageSaved, 3);
}

int App::ui_scale() const
{
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(window_, &w, &h);
    if (tools_.ui_scale > 0)
        return tools_.ui_scale;
    return std::max(1, h / kMinViewHeight);
}

// Runs and draws the game's interface for the frame: the requester windows
// and the panel, in the 640 x 480 canvas at the window's bottom centre.
// The windows go first: the front one takes the mouse, a modal one (the
// yes/no requester) everything.
void App::interface_step(int out_w, int out_h, int scale)
{
    int ox = (out_w - kUiWidth * scale) / 2, oy = out_h - kUiHeight * scale;
    UiMouse mouse = ui_mouse(ox, oy, scale);
    Game *game = map_ ? &map_->game : nullptr;
    InfoWindowResult info;
    help_mouse_ = mouse;

    // A framed help page holds the screen: its close button, Escape or Space
    // closes it, and the screen behind is not read.
    if (help_page_open()) {
        if (help_.update_close_hover(mouse.x, mouse.y))
            sounds_.play_sound(kSourceGeneral, kSfxHighlight5);
        bool close = std::exchange(escape_, false) || (mouse.left_pressed && help_.box().close_hover);
        if (close) {
            help_.finish_composite_help();
            sounds_.play_sound(kSourceGeneral, kSfxYesX);
        }
        interface_draw(game, ox, oy, scale);
        return;
    }

    bool escape = interface_escape(game);
    if (game) {
        sync_object_requesters(interface_, *game);
        sync_build_requester(interface_, *game);
    }
    InterfaceEvents events = interface_.frame(mouse, escape);
    if (game)
        info = interface_requesters(*game, events, mouse);
    interface_yes_no(game, events);
    bool paused = game && game->paused;
    bool input = !events.mouse_on_window && !interface_.modal() && !menu_.active;
    PanelAction action = panel_frame(panel_state_, mouse, *data_, paused, input);
    if (game)
        popup_icons_frame(popups_, game->world, mouse, *data_, frame_seconds_);
    if (game) {
        bool placing = game->world.placing.placing || hire_.placing();
        interface_panel_action(*game, action, placing);
        // The map: what is being placed follows the pointer.
        bool on_map = input && !paused && mouse.y < panel_top(*data_);
        interface_map_mouse(*game, mouse, on_map, placing);
        interface_map_hover(*game, mouse, on_map, placing);
        if (interface_.requester() == Requester::Queue)
            queue_window_after_frame(*game);
        interface_advisor(*game, mouse, input, paused);
        interface_command(*game, info);
        interface_room_help(*game);
        interface_help();
    }
    interface_draw(game, ox, oy, scale);
}

// Escape: with a modal requester up, the requester's; else it drops what is
// being hired, bought or placed (setup_abort_all_requesters). Returns
// whether the requesters still get it.
bool App::interface_escape(Game *game)
{
    bool escape = std::exchange(escape_, false);
    if (!escape || !game || interface_.modal())
        return escape;
    if (hire_.placing() || interface_.requester() == Requester::HireStaff ||
        interface_.requester() == Requester::Jukebox) {
        game->event = hire_staff_cancel(hire_);
        return false;
    }
    if (interface_.requester() == Requester::Queue) {
        // setup_abort_all_requesters closes the queue window (EV_CANCEL_BUILD).
        game->event = {EV_CLOSE_QUEUE_WINDOW, 0, 0, 0};
        return false;
    }
    if (object_escape(*game))
        return false;
    if (interface_.window_count() > 0 && interface_.requester() == Requester::None) {
        // The patient, staff or handyman window: EV_CANCEL_BUILD closes it.
        game->event = hire_staff_cancel(hire_);
        return false;
    }
    return true;
}

// The requesters' input for the frame: the jukebox, buying objects,
// building, hiring and the patient, staff and handyman windows, whose
// result it returns.
InfoWindowResult App::interface_requesters(Game &game, const InterfaceEvents &events, const UiMouse &mouse)
{
    interface_jukebox_input(game, events, mouse);
    const bool *keys = SDL_GetKeyboardState(nullptr);
    bool shift = keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];
    bool ctrl = keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL];
    object_requesters_input(interface_, object_ui_, game, events, mouse, {shift, ctrl});
    build_requesters_input(interface_, build_ui_, game, events, mouse);
    queue_window_input(queue_, interface_, game, {*data_, requester_art_, map_cell_, events, mouse});
    interface_hire_input(game, events);
    interface_staff_demand_input(game, events);
    InfoWindowResult info = info_window_frame(info_, interface_, events, mouse,
                                              {game.world, data_->text(), {scroll_min_, scroll_max_}, game.balance});
    for (uint8_t sfx : info.sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    if (info.event.type)
        game.event = info.event;
    if (info.slist_staff)
        charts_.slist_staff = info.slist_staff;
    if (info.centre_on) {
        // process_tracking_windows' othervar.center_on_slot.
        Point p = info_centre(game.world, info.centre_on);
        if (!beyond_scroll_limit(p, scroll_min_, scroll_max_))
            camera_ = {p.x - surface_.width() / 2, p.y - surface_.height() / 2};
    }
    return info;
}

void App::interface_jukebox_input(Game &game, const InterfaceEvents &events, const UiMouse &mouse)
{
    if (interface_.requester() != Requester::Jukebox)
        return;
    JukeboxResult jr = jukebox_frame(jukebox_, interface_, {events, mouse}, requester_art_, music_, game.paused);
    for (uint8_t sfx : jr.sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    if (jr.close)
        game.event = hire_staff_cancel(hire_);
}

void App::interface_hire_input(Game &game, const InterfaceEvents &events)
{
    std::vector<uint8_t> hire_sounds;
    Event hire_event = hire_staff_click(hire_, events, game, &hire_sounds);
    for (uint8_t sfx : hire_sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    if (hire_event.type)
        game.event = hire_event;
}

void App::interface_staff_demand_input(Game &game, const InterfaceEvents &events)
{
    if (interface_.requester() != Requester::StaffDemand)
        return;
    bool accept = false;
    if (Event e = staff_demand_click(demand_, events, accept); e.type) {
        sounds_.play_sound(kSourceGeneral, accept ? kSfxBonus2 : kSfxSack);
        game.event = e;
    }
}

// The yes/no requester's answer: its sound, then what it was asked for.
void App::interface_yes_no(Game *game, const InterfaceEvents &events)
{
    if (events.yes_no == YesNoAnswer::None)
        return;
    // The No button plays SFX_NO4 (req13); Escape and a right click outside
    // the window answer No without a sound (req13_cancel_check).
    if (events.yes_no == YesNoAnswer::Yes)
        sounds_.play_sound(kSourceGeneral, kSfxYesX);
    else if (!events.yes_no_cancel)
        sounds_.play_sound(kSourceGeneral, kSfxNo4);
    if (events.yes_no == YesNoAnswer::Yes && asking_quit_) {
        // quit_game 1: close the requester, then save the Continue slot
        // (game.cpp's close_irequesters before save_game(0x63)) and return to
        // the main menu; a run started with --level has no menu to return to.
        asking_quit_ = false;
        yes_event_ = no_event_ = {};
        interface_.close_yes_no(false);
        if (!level_from_cli_ && data_) {
            save_continue_slot();
            map_.reset();
            in_startup_ = true;
            startup_.return_to_main_menu();
        } else {
            running_ = false;
        }
        return;
    }
    int save_slot = std::exchange(pending_save_slot_, 0);
    if (events.yes_no == YesNoAnswer::Yes && save_slot > 0) {
        // EV_CLOSE_SAVE_YES_NO closes the requester before the save
        // (check_save_games follows).
        interface_.close_yes_no(false);
        perform_save(save_slot);
    }
    if (game) {
        Event e = events.yes_no == YesNoAnswer::Yes ? yes_event_ : no_event_;
        if (e.type)
            game->event = e;
    }
    asking_quit_ = false;
    yes_event_ = no_event_ = {};
}

// panel_l_button's room editing on/off (MM_EDIT_ROOM): from the normal mode
// it starts editing, from the edit mode it stops; refused otherwise.
bool App::panel_edit_room(Game &game)
{
    BuildMouse mode = game.world.building.mouse;
    if ((mode != BuildMouse::None && mode != BuildMouse::EditRoom) || interface_.requester() != Requester::None)
        return true;
    game.world.building.mouse = mode == BuildMouse::None ? BuildMouse::EditRoom : BuildMouse::None;
    game.world.building.build.edit_room = 0;
    game.world.building.build.draw_mode = 0;
    return false;
}

// Carries out the panel button pressed this frame, with its sound; `placing`
// says whether an object or a member of staff is being placed.
void App::interface_panel_action(Game &game, PanelAction action, bool placing)
{
    if (action == PanelAction::None)
        return;
    // panel_l_button and panel_r_button refuse every panel click while an
    // edited room empties (MM_EDIT_ROOM_WAIT).
    if (game.world.building.mouse == BuildMouse::EditRoomWait) {
        sounds_.play_sound(kSourceGeneral, kSfxWrong2);
        return;
    }
    // The other panel buttons lead to requesters and screens not ported yet.
    bool refused = action == PanelAction::Refused;
    if (action == PanelAction::FurnishCorridor) {
        refused = hire_.placing() || build_tool_busy(game);
        if (!refused)
            furnish_corridor(game);
    } else if (action == PanelAction::HireStaff) {
        refused = placing || build_tool_busy(game);
        if (!refused)
            game.event = {EV_OPEN_HIRE_STAFF};
    } else if (action == PanelAction::BuildRoom) {
        refused = hire_.placing() || build_tool_busy(game);
        if (!refused)
            open_build_room(game);
    } else if (action == PanelAction::EditRoom) {
        refused = panel_edit_room(game);
    } else if (action == PanelAction::TownMap) {
        // Refused while the edit room requester, or the buy objects one
        // other than for the corridor, is open.
        refused = interface_.requester() == Requester::EditRoom ||
                  (interface_.requester() == Requester::BuyObjects && game.world.placing.arg != 1);
        if (!refused)
            open_town_map();
    } else if (int chart = chart_for_panel_action(action); chart >= 0) {
        Event e = open_chart_command(chart, game, &refused);
        if (!refused)
            game.event = e;
    }
    sounds_.play_sound(kSourceGeneral, refused ? kSfxWrong2 : kSfxSelectX);
}

// The mouse on the map (`on_map`): what is being placed follows the
// pointer, and the right button drops what is held or picks up staff.
void App::interface_map_mouse(Game &game, const UiMouse &mouse, bool on_map, bool placing)
{
    int cell = on_map ? map_cell_ : -1;
    placement_mouse(object_ui_, game, cell, mouse, on_map);
    build_mouse(build_ui_, game, cell, pointed_.thing, mouse, on_map);
    if (hire_.placing() && on_map && cell >= 0) {
        update_staff_cursor(hire_, game.world, cell);
        if (mouse.left_pressed) {
            if (Event e = place_staff_click(hire_, game.world, cell); e.type)
                game.event = e;
        } else if (mouse.right_pressed) {
            game.event = hire_staff_cancel(hire_);
        }
    } else if (mouse.right_pressed && on_map && !advisor_hit(game.advisor, mouse.x, mouse.y) &&
               (interface_.requester() == Requester::None || interface_.requester() == Requester::EditRoom)) {
        if (Event e = interface_map_right(game, placing); e.type)
            game.event = e;
    }
}

// process_right_button on the map: the edit-room modes, picking up an object
// or member of staff, or cancelling. Returns the command, EV_NONE for none.
Event App::interface_map_right(Game &game, bool placing)
{
    BuildMouse mode = game.world.building.mouse;
    if (mode == BuildMouse::EditRoomWait)
        return {EV_CANCEL_EDIT_ROOM, 0, 0, uint16_t(mode)};
    if (mode == BuildMouse::EditRoom || mode == BuildMouse::PickUpObject) {
        object_escape(game);
        return {};
    }
    if (mode != BuildMouse::None || placing || build_tool_busy(game))
        return {};
    if (game.world.building.build.room)
        return check_object_right_click_in_room_edit(game.world, pointed_.thing);
    if (interface_.window_count() > 0)
        return hire_staff_cancel(hire_);
    if (Event e = pick_up_object_right_click(game.world, pointed_.thing); e.type)
        return e;
    if (Event e = pick_up_staff_click(game.world, pointed_.thing); e.type) {
        sounds_.play_sound(kSourceGeneral, kSfxPickup);
        return e;
    }
    return {};
}

// The watch's buttons over the panel (process_left_button): a left click on
// the epidemic's vaccination button toggles the MM_VACCINATE pointer; on the
// opening countdown's it opens the hospital at once (EV_OPEN_HOSPITAL) with
// its fanfare. The game uses watch[14] for both hotspots. True when one took
// the click.
bool App::interface_watch_buttons(Game &game, const UiMouse &mouse)
{
    if (!mouse.left_pressed)
        return false;
    Sprite button = data_->watch().at(14);
    if (mouse.x < kPanelWatchButtonX || mouse.x >= kPanelWatchButtonX + button.width || mouse.y < kPanelWatchButtonY ||
        mouse.y >= kPanelWatchButtonY + button.height)
        return false;
    if (game.world.epidemic.timer > 0) {
        // MM_VACCINATE, with SFX_SELECTX (mouse.cpp:317).
        vaccinate_ = !vaccinate_;
        sounds_.play_sound(kSourceGeneral, kSfxSelectX);
        return true;
    }
    if (game.opening_countdown > 0) {
        game.event = {EV_OPEN_HOSPITAL};
        sounds_.play_sound(kSourceGeneral, kSfxFanfare);
        return true;
    }
    return false;
}

// The watch button's highlight (draw_emergency's othervar.watch_hovered): its
// sound plays once on entering it, when the button is drawn.
void App::interface_watch_hover(Game &game, const UiMouse &mouse)
{
    // The button is drawn only in the epidemic branch or, past an emergency
    // and with the tutorial flag clear, in the opening-countdown branch
    // (draw_emergency, panel.cpp:593).
    bool epidemic = game.world.epidemic.timer > 0;
    bool countdown =
        game.opening_countdown > 0 && !(game.advisor.flags & kAdvisorTutorial) && game.open_countdown >= 13;
    if (game.world.emergency.timer > 0 || (!epidemic && !countdown)) {
        watch_hovered_ = false;
        return;
    }
    Sprite button = data_->watch().at(14);
    bool over_watch = mouse.x >= kPanelWatchButtonX && mouse.x < kPanelWatchButtonX + button.width &&
                      mouse.y >= kPanelWatchButtonY && mouse.y < kPanelWatchButtonY + button.height;
    if (over_watch && !watch_hovered_)
        sounds_.play_sound(kSourceGeneral, kSfxHighlight5);
    watch_hovered_ = over_watch;
}

// process_mouse's MM_NORMAL: what the pointer is over, and a left click
// there (process_left_button) opens its window.
void App::interface_map_hover(Game &game, const UiMouse &mouse, bool on_map, bool placing)
{
    // The bar stays 100 frames after the pointer leaves a thing
    // (othervar.panel_info_timer).
    if (panel_info_timer_ && --panel_info_timer_ == 0)
        panel_info_ = 0;
    interface_watch_hover(game, mouse);
    if (!on_map || placing || build_tool_busy(game) || game.world.building.mouse != BuildMouse::None ||
        interface_.requester() != Requester::None || advisor_hit(game.advisor, mouse.x, mouse.y)) {
        pointer_sprite_ = kPointerDefault;
        return;
    }
    if (popups_.hovered) {
        // Hover action 7: a fax icon, which a click answers.
        pointer_sprite_ = kPointerDefault;
        if (mouse.left_pressed) {
            close_irequesters(interface_);
            game.event = popup_icon_click(popups_);
        }
        return;
    }
    // The watch's buttons handled the click; a left click on a contagious
    // patient queues them for vaccination (EV_TOGGLE_VACCINATION).
    if (interface_watch_buttons(game, mouse))
        return;
    const World::Epidemic &epidemic = game.world.epidemic;
    MapHover hover = map_hover(game.world, pointed_.thing);
    if (hover.hit) {
        panel_info_timer_ = 100;
        panel_info_ = hover.info;
        panel_thing_ = hover.thing;
    }
    // process_mouse's MM_NORMAL: a rat near the pointer but not aimed at
    // (othervar.near_rat) shows the near pointer, and a click fires a miss.
    if (interface_near_rat(game, mouse, hover))
        return;
    // During a cover-up, hovering a contagious patient shows the vaccination
    // pointer, as process_mouse's MM_NORMAL does; a click on one queues them.
    uint16_t target = 0;
    if (epidemic.active && hover.hit && is_person_highlighted_for_vaccination(game.world, hover.thing, target)) {
        pointer_sprite_ = kPointerVaccinateTarget;
        if (mouse.left_pressed)
            game.event = {EV_TOGGLE_VACCINATION, target, 0, 0};
        return;
    }
    if (vaccinate_ && epidemic.timer > 0) {
        bool highlighted = is_person_highlighted_for_vaccination(game.world, pointed_.thing, target);
        pointer_sprite_ = highlighted ? kPointerVaccinateTarget : kPointerVaccinate;
        if (mouse.left_pressed && highlighted)
            game.event = {EV_TOGGLE_VACCINATION, target, 0, 0};
        return;
    }
    if (epidemic.timer == 0)
        vaccinate_ = false;
    interface_map_hover_click(game, mouse, hover);
}

// interface_map_hover's rat near the pointer but not aimed at: the near
// pointer and, on a click, EV_SHOOT_RAT at nothing (a miss), as
// process_mouse's MM_NORMAL. False if no rat is near or one is aimed at.
bool App::interface_near_rat(Game &game, const UiMouse &mouse, const MapHover &hover)
{
    if (!near_rat_)
        return false;
    if (hover.hit && hover.thing < game.world.things.size() && game.world.things[hover.thing].type == kThingRat)
        return false;
    pointer_sprite_ = kPointerRatNear;
    if (mouse.left_pressed)
        game.event = map_left_click(game.world, hover, interface_, map_cell_, true);
    return true;
}

// interface_map_hover's pointer and left click for the thing `hover` is over:
// its pointer, its status icon and, on a click, map_left_click's command.
void App::interface_map_hover_click(Game &game, const UiMouse &mouse, const MapHover &hover)
{
    if (hover.pointer != kPointerKeep)
        pointer_sprite_ = hover.pointer;
    // peep_icon (src/mouse.cpp): passing the mouse over a person announces
    // their pending status icon.
    if (hover.hit && hover.thing && hover.thing < game.world.things.size()) {
        Thing &h = game.world.things[hover.thing];
        if ((h.type == kThingPatient || h.type == kThingStaff) && h.owner == 0)
            peep_icon(game.world, h);
    }
    if (mouse.left_pressed)
        if (Event e = map_left_click(game.world, hover, interface_, map_cell_); e.type)
            game.event = e;
}

// After interface_map_hover while the queue window is open: req7_overdraw's
// panel information and MM_MOVE_QUEUE_PERSON pointer, and the door hotspot's
// scroll to the room.
void App::queue_window_after_frame(Game &game)
{
    panel_info_ = queue_.panel_info;
    panel_thing_ = queue_.panel_thing;
    if (game.world.building.mouse == BuildMouse::MoveQueuePerson)
        pointer_sprite_ = kPointerMoveQueuePerson;
    if (queue_.goto_cell >= 0) {
        Point p = cell_to_world(queue_.goto_cell % kMapSize, queue_.goto_cell / kMapSize);
        if (!beyond_scroll_limit(p, scroll_min_, scroll_max_))
            camera_ = {p.x - surface_.width() / 2, p.y - surface_.height() / 2};
    }
}

// The advisor: a right click on him or his bubble dismisses him; then his
// frame, which reads the command about to be carried out.
void App::interface_advisor(Game &game, const UiMouse &mouse, bool input, bool paused)
{
    if (mouse.right_pressed && input && mouse.y < panel_top(*data_) && advisor_hit(game.advisor, mouse.x, mouse.y))
        game.advisor.dismiss = true;
    AdvisorInputs in;
    in.game_turn = game.turn_count;
    in.paused = paused;
    in.requester_open = interface_.requester() != Requester::None && interface_.requester() != Requester::EditRoom;
    in.level = game.level_number();
    in.month = game.date.month;
    in.open = game.open;
    in.balance = game.balance;
    in.reputation = game.reputation();
    in.finance = &game.finance;
    in.settings = &game.settings();
    in.world = &game.world;
    in.visitors = game.arrivals.hospitals[0].visitors;
    in.text = &data_->text();
    uint8_t ambient = 0;
    in.ambient = &ambient;
    in.pending = game.event;
    in.last = last_event_;
    in.staff_place_ok = hire_.place_ok;
    if (game.world.placing.placing)
        in.object_place_ok = game.world.placing.ok;
    advisor_frame(game.advisor, in, game.anims(), sounds_.safeseed);
    if (ambient)
        sounds_.play_ambient(ambient, has_receptionist(game.world), paused);
    last_event_ = game.event;
}

// The command, as process_turn: the interface's part, then the game's; then
// what the windows asked for besides (`info`).
void App::interface_command(Game &game, const InfoWindowResult &info)
{
    run_command(game);
    if (info.event.type == EV_OPEN_YES_NO) {
        std::string q = info.question_text.empty() ? std::string(data_->text().at(TextSection::YesNo, info.question))
                                                   : info.question_text;
        interface_.open_yes_no(q);
        yes_event_ = info.yes;
        no_event_ = info.no;
    }
    if (info.casebook) {
        // req3 opens the casebook itself, with no command; EV_OPEN_SCREEN is
        // refused while an edited room empties (io.cpp:824).
        if (game.world.building.mouse == BuildMouse::EditRoomWait) {
            sounds_.play_sound(kSourceGeneral, kSfxWrong2);
        } else {
            charts_.selected_illness = info.casebook;
            chart_command(charts_, {EV_OPEN_SCREEN, uint16_t(kChartCasebook)});
        }
    }
}

// Draws the panel, the advisor and the requesters into the interface's
// canvas, and the canvas at (ox, oy) of the window at `scale`.
void App::interface_draw(Game *game, int ox, int oy, int scale)
{
    ui_.clear();
    PanelFigures figures;
    if (map_) {
        const Game &g = map_->game;
        figures = {g.balance, g.date.day, g.date.month, g.reputation(), g.opening_countdown, g.open_countdown};
        figures.emergency_timer = g.world.emergency.timer;
        figures.emergency_state = g.world.emergency.state;
        figures.epidemic_timer = g.world.epidemic.timer;
        figures.epidemic_illness = g.world.epidemic.illness != 0;
        // The watch is not shown while the tutorial holds the countdown.
        if (g.advisor.flags & kAdvisorTutorial)
            figures.opening_countdown = 0;
    }
    draw_panel_ui(ui_, panel_state_, map_ ? &figures : nullptr, *data_);
    if (map_ && game)
        draw_panel_bar(*game);
    if (map_) {
        draw_popup_icons(ui_, popups_, *data_, panel_state_, &figures);
        draw_emergency(ui_, figures, *data_);
    }
    if (game) {
        draw_advisor(ui_, game->advisor, *data_, data_->font(0x1d));
        advance_balance_roll(panel_state_, game->balance);
        draw_info_view(*game);
    }
    draw_interface(ui_, interface_, requester_art_);
    if (game)
        draw_info_windows(ui_, info_, interface_, {requester_art_, hire_art_, data_->text()}, game->world,
                          object_ui_.red);
    if (game)
        draw_queue_window(ui_, queue_, interface_, game->world, requester_art_, *data_);
    if (interface_.requester() == Requester::Jukebox)
        draw_jukebox(ui_, interface_, requester_art_, music_);
    if (game) {
        draw_object_requesters(ui_, interface_, object_ui_, *game, requester_art_, *data_);
        draw_build_requester(ui_, interface_, build_ui_, *game, requester_art_, *data_);
        draw_hire_staff(ui_, hire_, interface_, {requester_art_, hire_art_, data_->text()}, game->world);
        draw_staff_demand(ui_, demand_, interface_, {requester_art_, hire_art_, data_->text()}, *data_, game->world);
    }
    help_draw();
    std::vector<uint32_t> pixels = ui_.to_argb(data_->palette());
    SDL_UpdateTexture(ui_tex_, nullptr, pixels.data(), kUiWidth * 4);
    SDL_FRect dst = {float(ox), float(oy), float(kUiWidth * scale), float(kUiHeight * scale)};
    SDL_RenderTexture(renderer_, ui_tex_, nullptr, &dst);
}

// The panel's information bar: the special message while one is up, else
// the thing under the pointer.
void App::draw_panel_bar(const Game &game)
{
    if (panel_message_.empty()) {
        panel_info_drawn_ = draw_panel_info({ui_, panel_top(*data_), requester_art_.font(), panel_state_.raised},
                                            panel_info_, panel_thing_, game, *data_);
    } else {
        panel_info_drawn_ = 0;
        draw_panel_special_info();
    }
}

// display_panel_special_info: the saved/cannot-save/load-failed message on
// the panel's information bar, for its remaining frames. The pointer over the
// panel clears it at once, unless the game is paused (panel.cpp:1558-1560).
void App::draw_panel_special_info()
{
    if (map_ && !map_->game.paused && help_mouse_.y >= panel_top(*data_)) {
        panel_message_.clear();
        panel_message_time_ = 0;
        return;
    }
    draw_panel_message({ui_, panel_top(*data_), requester_art_.font()}, panel_message_, *data_);
    if (panel_message_time_ > 0 && --panel_message_time_ == 0)
        panel_message_.clear();
}

// The help tooltip of the screen open this frame: the original computes the
// front requester's group in its overdraw (draw_main_window), then the
// panel's group 3 in draw_all_windows, every hospital frame.
void App::interface_help()
{
    if (interface_.modal()) {
        help_screen(8, 0, 0); // req13, the yes/no requester
    } else if (uint8_t group = requester_help_group(interface_.requester())) {
        const Window *w = nullptr;
        Requester r = interface_.requester();
        if (r == Requester::BuildRoom || r == Requester::HireStaff || r == Requester::EditRoom || r == Requester::Queue)
            w = &interface_.window(int(r));
        help_screen(group, w ? w->x : 0, w ? w->y : 0);
    }
    if (queue_.help_group)
        help_screen(queue_.help_group, queue_.help_x, queue_.help_y);
    help_screen(3, 0, 0); // the panel, after the requester
}

void App::interface_room_help(Game &game)
{
    uint8_t built = game.world.building.last_built_room;
    if (built == last_built_room_)
        return;
    last_built_room_ = built;
    if (built == 0 || built >= int(room_help_seen_.size()) || room_help_seen_[built])
        return;
    room_help_seen_[built] = true;
    // open_room closes the edit room requester before the page; the page
    // then holds the screen, so the next frame's sync would come too late.
    sync_object_requesters(interface_, game);
    help_.show_help_page(data_->text(), built);
}

void App::help_screen(int group, int x, int y)
{
    if (!data_)
        return;
    help_context();
    help_.compute_help_text(help_ctx_, x, y, uint8_t(group));
}

void App::load_help_art(const std::filesystem::path &dir)
{
    std::string error;
    if (!load_font(dir, 0, help_font_, &error))
        std::fprintf(stderr, "%s\n", error.c_str());
    if (load_font(dir, 0x2d, help_menu_font_, &error))
        help_.set_main_menu_font(help_menu_font_);
    else
        std::fprintf(stderr, "%s\n", error.c_str());
    help_.set_art(help_font_, menu_.sprites);
    help_.set_ghost(data_->ghost().data());
}

void App::help_context()
{
    help_ctx_ = HelpContext{};
    if (map_) {
        help_ctx_.game = &map_->game;
        help_ctx_.world = &map_->game.world;
        help_ctx_.placing = &map_->game.world.placing;
    }
    help_ctx_.data = data_.get();
    help_ctx_.text = &data_->text();
    help_ctx_.ui = &interface_;
    help_ctx_.hire = &hire_;
    help_ctx_.info = &info_;
    help_ctx_.charts = &charts_;
    help_ctx_.object_ui = &object_ui_;
    help_ctx_.build = &build_ui_;
    help_ctx_.mouse = &help_mouse_;
    help_ctx_.pulldv = &menu_.sprites;
    help_ctx_.help = &help_.box();
    help_.set_clock(ticks_ms());
}

void App::wire_screen_help()
{
    town_.set_help(help_, help_ctx_);
    charts_.set_help(help_, help_ctx_);
    startup_.set_help(help_, help_ctx_);
}

void App::help_draw()
{
    if (!data_ || help_.box().msg.empty())
        return;
    // mouse_mode_help_allowed: a tooltip only in the normal mode, not while
    // something is placed or built; the advisor's tutorial overrides. A
    // framed page is MM_HELP, which is allowed.
    bool page = help_.box().style == 2;
    bool allowed = page;
    bool tutorial = false;
    if (map_) {
        const Game &g = map_->game;
        tutorial = (g.advisor.flags & kAdvisorTutorial) != 0;
        allowed = allowed || (!g.world.placing.placing && !build_tool_busy(g) && !hire_.placing() &&
                              interface_.requester() != Requester::EditRoom);
    }
    if (!(allowed || tutorial)) {
        // The tooltip was not computed in a mode that may show it; drop it so
        // it cannot reappear once the mode is allowed again.
        help_.box().msg.clear();
        return;
    }
    help_.draw(ui_.pixels(), data_->palette(), allowed, tutorial);
    const HelpBox &b = help_.box();
    ui_.mark({b.x, b.y, b.w, b.h});
    // The framed page's shadow is a ghost blend in the original, but the
    // interface layer is transparent, so it is drawn translucent instead.
    for (Rect r : help_.page_shadow_rects())
        ui_.shadow(r, HelpSystem::kPageShadowAlpha);
}

// The view window over the query window showing a person, under its art.
void App::draw_info_view(Game &game)
{
    Rect view_box;
    Point view_camera;
    if (!info_view(info_, interface_, game.world, view_box, view_camera))
        return;
    if (info_view_.width() != view_box.w || info_view_.height() != view_box.h) {
        info_view_.resize(view_box.w, view_box.h);
        info_scratch_.resize(view_box.w, view_box.h);
        info_scratch_.fill(kMaskColour);
    }
    info_view_.fill(0);
    draw_world({info_view_, {0, 0, view_box.w, view_box.h}, view_camera}, game.world, *data_,
               shadows_ ? map_->shadows.get() : nullptr, info_scratch_);
    Surface &px = ui_.pixels();
    for (int y = 0; y < view_box.h; y++)
        for (int x = 0; x < view_box.w; x++)
            if (view_box.x + x >= 0 && view_box.x + x < px.width() && view_box.y + y >= 0 &&
                view_box.y + y < px.height())
                px.row(view_box.y + y)[view_box.x + x] = info_view_.row(y)[x];
    ui_.mark(view_box);
}

// Asks "really quit?" with the yes/no requester, as the game does.
void App::ask_quit()
{
    if (interface_.modal() || !data_)
        return;
    interface_.open_yes_no(data_->text().at(TextSection::YesNo, 0));
    asking_quit_ = true;
}

// File > Restart: ask_restart_level's yes/no requester.
void App::ask_restart()
{
    if (!map_ || interface_.modal())
        return;
    interface_.open_yes_no(data_->text().at(TextSection::YesNo, 6));
    yes_event_ = {EV_RESTART_LEVEL};
    no_event_ = {};
    asking_quit_ = false;
}

// The game's own pointer `sprite`, in place of the system one over the
// game; its hotspot is its top-left corner.
void App::draw_pointer(int scale, uint8_t sprite)
{
    float mx, my;
    SDL_GetMouseState(&mx, &my);
    float density = SDL_GetWindowPixelDensity(window_);
    const PointerTexture &pt =
        pointers_[sprite < pointers_.size() && pointers_[sprite].texture ? sprite : kPointerDefault];
    bool over_game = SDL_GetMouseFocus() == window_ && !ImGui::GetIO().WantCaptureMouse;
    bool crosshair = tools_.inspect.picking && over_game;
    if (crosshair && !crosshair_)
        crosshair_ = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
    if (crosshair != crosshair_shown_) {
        SDL_SetCursor(crosshair ? crosshair_ : SDL_GetDefaultCursor());
        crosshair_shown_ = crosshair;
    }
    if (crosshair) {
        SDL_ShowCursor();
    } else if (pt.texture && over_game) {
        SDL_HideCursor();
        SDL_FRect p = {mx * density, my * density, float(pt.w * scale), float(pt.h * scale)};
        SDL_RenderTexture(renderer_, pt.texture, nullptr, &p);
    } else {
        SDL_ShowCursor();
    }
}

UiMouse App::fitted_mouse(const SDL_FRect &at, const Surface &src) const
{
    float mx, my;
    SDL_GetMouseState(&mx, &my);
    float density = SDL_GetWindowPixelDensity(window_);
    float fit = at.w / float(std::max(src.width(), 1));
    UiMouse mouse = ui_mouse(0, 0, 1);
    mouse.x = int((mx * density - at.x) / fit);
    mouse.y = int((my * density - at.y) / fit);
    return mouse;
}

void App::draw_fitted(const Surface &src, const Palette &palette, SDL_Texture *&tex, const SDL_FRect &dst)
{
    int sw = std::max(src.width(), 1), sh = std::max(src.height(), 1);
    if (!tex || tex->w != sw || tex->h != sh) {
        if (tex)
            SDL_DestroyTexture(tex);
        tex = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, sw, sh);
        SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
    }
    void *pixels;
    int pitch;
    if (tex && src.width() > 0 && SDL_LockTexture(tex, nullptr, &pixels, &pitch)) {
        const uint8_t *p = src.data();
        for (int y = 0; y < sh; y++) {
            auto *row = reinterpret_cast<uint32_t *>(static_cast<uint8_t *>(pixels) + y * pitch);
            for (int x = 0; x < sw; x++)
                row[x] = palette[*p++];
        }
        SDL_UnlockTexture(tex);
        SDL_RenderTexture(renderer_, tex, nullptr, &dst);
        fit_.at = dst;
        fit_.w = sw;
        fit_.h = sh;
        fit_.frame = frame_;
    }
}

// Opens the town map, first dropping what is being hired, bought or placed
// (setup_abort_all_requesters), as the game does.
void App::open_town_map()
{
    if (!map_ || town_.is_open())
        return;
    Game &game = map_->game;
    if (hire_.placing() || interface_.requester() == Requester::HireStaff ||
        interface_.requester() == Requester::Jukebox)
        game.event = hire_staff_cancel(hire_);
    else
        object_escape(game);
    town_.open();
}

// One frame of the town map, in place of the view and the interface, drawn
// aspect-fit in the window; then the turn's command, as interface_step
// carries it out.
void App::town_step(int out_w, int out_h)
{
    Game &game = map_->game;
    const Surface &src = town_.surface();
    SDL_FRect at = fit_rect(src, out_w, out_h);
    UiMouse mouse = fitted_mouse(at, src);
    help_mouse_ = mouse;
    help_context();
    bool escape = escape_;
    escape_ = false;
    auto go_to = [this](int column, int row) {
        Point centre = cell_to_world(column, row);
        if (beyond_scroll_limit(centre, scroll_min_, scroll_max_))
            return false;
        camera_ = {centre.x - surface_.width() / 2, centre.y - surface_.height() / 2};
        return true;
    };
    TownMapResult r = town_.frame({mouse, escape}, game, game.land, panel_state_, startup_.player_name(), go_to);
    for (uint8_t sfx : r.sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    if (r.event.type)
        game.event = r.event;
    if (r.closed || r.event.type == EV_OPEN_SCREEN)
        town_.close();
    last_event_ = game.event;
    run_command(game);
    draw_fitted(src, town_.palette(), town_tex_, at);
}

void App::run_command(Game &game)
{
    // EV_OPEN_SCREEN is refused while an edited room empties (io.cpp:824).
    if (game.event.type == EV_OPEN_SCREEN && game.world.building.mouse == BuildMouse::EditRoomWait) {
        sounds_.play_sound(kSourceGeneral, kSfxWrong2);
        game.event = {};
    }
    bool restart = game.event.type == EV_RESTART_LEVEL;
    // Answering a member of staff's resignation call opens the staff demand
    // requester once the command has removed the call (io.cpp's
    // EV_ANSWER_PLAYERCALL).
    uint16_t demand_staff = 0;
    if (game.event.type == EV_ANSWER_PLAYERCALL && game.event.value == 1)
        demand_staff = playercall_subject(game.world, 1, game.event.arg, 0);
    // The demand's answers close its requester (process_turn's
    // close_crequester and close_irequesters).
    bool demand_answer = game.event.type == EV_REFUSE_STAFF_DEMAND || game.event.type == EV_RAISE_STAFF_SALARY ||
                         game.event.type == EV_GIVE_STAFF_BONUS;
    hire_staff_event(hire_, interface_, game.world, game.event);
    jukebox_event(jukebox_, interface_, game.event);
    chart_command(charts_, game.event);
    // EV_SACK_STAFF and EV_CANCEL_SACK close the yes/no requester that asked.
    if (game.event.type == EV_SACK_STAFF || game.event.type == EV_CANCEL_SACK)
        interface_.close_yes_no(true);
    if (game.event.type == EV_SACK_STAFF)
        sounds_.play_sound(kSourceGeneral, kSfxSack);
    // EV_CANCEL_SACK plays SFX_NO4 (io.cpp:1841), on top of the No button's.
    else if (game.event.type == EV_CANCEL_SACK)
        sounds_.play_sound(kSourceGeneral, kSfxNo4);
    info_window_event(info_, interface_, game.world, game.event);
    queue_window_event(queue_, interface_, game.world, game.event);
    fax_command(fax_, game.event);
    game.process_turn();
    if (demand_answer) {
        interface_.close_requester(Requester::StaffDemand);
        close_irequesters(interface_);
    }
    // compute_whether_user_not_busy (user_idle): the demand opens only with
    // no window open and nothing being placed.
    if (demand_staff && !interface_.modal() && !game.world.placing.placing && !build_tool_busy(game) &&
        !hire_.placing()) {
        sounds_.play_sound(kSourceGeneral, kSfxStaffDemand);
        open_staff_resign_requester(demand_, interface_, game.world, demand_staff, sounds_.safeseed);
    }
    // display_choice_screen, from the command's answer to a call.
    if (game.choice_screen.type) {
        close_irequesters(interface_);
        display_choice_screen(fax_, game.choice_screen);
    }
    hire_staff_after_turn(hire_, game.world);
    for (uint8_t sfx : game.world.building.sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    game.world.building.sounds.clear();
    game_sounds(game);
    if (restart)
        restart_level_ = true;
}

void App::game_sounds(Game &game)
{
    // add_panel_popup's SFX_NEWFAX.
    constexpr uint8_t kSfxNewFax = 0xdd;
    // research_category_exhausted's SFX_RESEARCH_EXHAUSTED.
    constexpr uint8_t kSfxResearchExhausted = 0xdb;
    for (; game.world.popups.new_faxes; game.world.popups.new_faxes--)
        sounds_.play_sound(kSourceGeneral, kSfxNewFax);
    for (; game.world.research.exhausted; game.world.research.exhausted--)
        sounds_.play_sound(kSourceGeneral, kSfxResearchExhausted);
    // summon_vehicle's announcement of the emergency's illness, as its
    // helicopter is sent.
    const World::Emergency &e = game.world.emergency;
    if (e.heli && e.heli != heli_seen_ && e.illness < 0x24)
        sounds_.play_ambient(kAnnounceEmergency + e.illness, has_receptionist(game.world), game.paused);
    heli_seen_ = e.heli;
}

// One frame of the fax, in place of the view and the interface, drawn
// aspect-fit in the window; then the turn's command.
void App::fax_step(int out_w, int out_h)
{
    Game &game = map_->game;
    SDL_FRect at = fit_rect(fax_.surface, out_w, out_h);
    UiMouse mouse = fitted_mouse(at, fax_.surface);
    FaxKeys keys;
    keys.escape = std::exchange(escape_, false) || std::exchange(chart_space_, false);
    FaxResult r = fax_frame(fax_, mouse, keys, frame_seconds_, game, sounds_.safeseed);
    for (uint8_t sfx : r.sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    if (r.event.type)
        game.event = r.event;
    last_event_ = game.event;
    run_command(game);
    draw_fitted(fax_.surface, fax_.palette, fax_tex_, at);
}

// One frame of the open chart, in place of the view and the interface,
// drawn aspect-fit in the window; then the turn's command.
void App::charts_step(int out_w, int out_h)
{
    Game &game = map_->game;
    SDL_FRect at = fit_rect(charts_.surface, out_w, out_h);
    UiMouse mouse = fitted_mouse(at, charts_.surface);
    help_mouse_ = mouse;
    help_context();
    const bool *held = SDL_GetKeyboardState(nullptr);
    ChartKeys keys;
    keys.escape = std::exchange(escape_, false);
    keys.space = std::exchange(chart_space_, false);
    keys.up = held[SDL_SCANCODE_UP];
    keys.down = held[SDL_SCANCODE_DOWN];
    keys.left = held[SDL_SCANCODE_LEFT];
    keys.right = held[SDL_SCANCODE_RIGHT];
    ChartsResult r = chart_frame(charts_, mouse, keys, frame_seconds_, game, game.hospital);
    for (uint8_t sfx : r.sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    // The research screen's bubbling loops on the crowd source until the
    // screen closes (rschscrn.cpp:39/50).
    if (r.research_sound)
        sounds_.play_sound(kSourceCrowd, kSfxResearch);
    if (r.research_stop)
        sounds_.stop_sample(kSourceCrowd, kSfxResearch);
    if (r.event.type)
        game.event = r.event;
    last_event_ = game.event;
    run_command(game);
    if (r.event.type == EV_OPEN_YES_NO) {
        // EV_OPEN_YES_NO: the chart closed, the requester over the hospital.
        interface_.open_yes_no(data_->text().at(TextSection::YesNo, r.question));
        yes_event_ = r.yes;
        no_event_ = r.no;
    }
    draw_fitted(charts_.surface, charts_.palette, charts_tex_, at);
    if (r.town_map) {
        charts_.open = false;
        open_town_map();
    }
}

// Opens the year-end screens when the year moves on (year_end), or the
// win/lose sequence when a level ends (check_game_over /
// check_quarterly_game_over). The original's start_year_end pauses the game
// for the screens.
void App::check_year_end()
{
    if (!map_ || year_end_showing_ || winlose_showing_)
        return;
    Game &game = map_->game;
    bool year_moved = game.date.year != last_year_;
    last_year_ = game.date.year;
    if (year_moved)
        open_year_end();
    else if (game.progress.winlose_result != 0)
        open_winlose();
}

void App::open_year_end()
{
    Game &game = map_->game;
    hosp_scores_ = default_hosp_scores();
    load_highscores(hiscore_path_, hosp_scores_);
    bool showing_fame = insert_hall_details(hosp_scores_, startup_.player_name(), game.progress.score);
    save_highscores(hiscore_path_, hosp_scores_);
    year_end_.open(hosp_scores_, showing_fame);
    year_end_phase_ = 0;
    year_end_showing_ = true;
    game.paused = true;
}

// One frame of the year-end screens, drawn aspect-fit in the window; the
// game is paused behind them.
void App::year_end_step(int out_w, int out_h)
{
    Game &game = map_->game;
    const Surface &src = year_end_.surface();
    SDL_FRect at = fit_rect(src, out_w, out_h);
    UiMouse mouse = fitted_mouse(at, src);
    YearEndKeys keys;
    keys.escape = std::exchange(escape_, false);
    keys.space = std::exchange(chart_space_, false);
    YearEndResult r;
    if (year_end_phase_ == 0) {
        r = year_end_.hall_frame(mouse, keys, frame_seconds_, startup_.player_name(), game.progress.score);
        if (r.hall_done) {
            year_end_phase_ = 1;
            year_end_.set_phase(1);
        }
    } else {
        YearEndView view;
        view.year = game.date.year;
        view.level = game.level_number();
        view.awards = &game.progress.awards;
        view.trophies = &game.progress.trophies;
        view.chart = &game.progress.chart;
        view.hospital_names = &charts_.hospital_names;
        r = year_end_.charts_frame(mouse, keys, frame_seconds_, view);
        if (r.charts_done) {
            year_end_.close();
            year_end_showing_ = false;
            game.paused = false;
        }
    }
    for (uint8_t sfx : r.sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    draw_fitted(src, year_end_.palette(), year_end_tex_, at);
}

// The losing-line index `anim`'s group carries (the original's nested scans
// of win_lose), drawn from the presentation's safeseed.
int App::lose_line_index(int anim)
{
    const LanguageText &t = data_->text();
    int count = int(t.size(TextSection::WinLose));
    int tc = 0;
    for (int n = anim; n != 0; n--)
        while (tc < count && t.at(TextSection::WinLose, tc++) != ".") {
        }
    int len = 0;
    while (tc + len < count && t.at(TextSection::WinLose, tc + len) != ".")
        len++;
    if (len <= 0)
        return std::min(tc, std::max(count - 1, 0));
    return tc + saferand(sounds_.safeseed) % len;
}

// Opens the win or lose sequence: the movie, then the letter.
void App::open_winlose()
{
    Game &game = map_->game;
    bool win = game.progress.winlose_result == 1;
    int movie = kMovieWinLevel;
    if (win) {
        if (game.level_number() >= 12)
            winlose_pick_ = saferand(sounds_.safeseed) % 4 + 8;
        else {
            winlose_pick_ = saferand(sounds_.safeseed) % 4;
            if (game.progress.win_declined % 2 == 1)
                winlose_pick_ += 4;
        }
    } else {
        int anim = saferand(sounds_.safeseed) % 6;
        if (anim == 2)
            anim = 1; // ANIM_LOSE3 plays ANIM_LOSE2
        movie = anim;
        winlose_pick_ = lose_line_index(anim);
        // insert_hall_details: the losing score enters the hall of shame.
        hosp_scores_ = default_hosp_scores();
        load_highscores(hiscore_path_, hosp_scores_);
        insert_hall_details(hosp_scores_, startup_.player_name(), game.progress.score);
        save_highscores(hiscore_path_, hosp_scores_);
    }
    winlose_.open(win, game.level_number());
    winlose_showing_ = true;
    winlose_to_menu_ = !win;
    game.progress.winlose_result = 0;
    game.paused = true;
    open_winlose_movie(movie);
}

// Starts the win/lose movie `index` (-1: none), as play_movie does for the
// start-up screens.
void App::open_winlose_movie(int index)
{
    winlose_movie_index_ = -1;
    if (movie_stream_) {
        SDL_DestroyAudioStream(movie_stream_);
        movie_stream_ = nullptr;
    }
    movie_playing_ = -1;
    if (index < 0)
        return;
    const MovieInfo &info = movie_info(index);
    auto path = resolve_ci(data_->dir(), info.file);
    if (!path || !winlose_movie_.open(*path) || !winlose_movie_.next_frame())
        return;
    winlose_movie_index_ = index;
    movie_playing_ = index;
    winlose_movie_flags_ = info.flags;
    winlose_movie_time_ = 0;
    winlose_movie_surface_.resize(kUiWidth, kUiHeight);
    winlose_movie_surface_.fill(0);
    copy_frame_to_screen(winlose_movie_, winlose_movie_surface_, winlose_movie_flags_);
    winlose_movie_palette_ = winlose_movie_.palette();
    winlose_movie_audio_.clear();
    SmackerAudioFormat format = winlose_movie_.audio_format(0);
    if (!(winlose_movie_flags_ & kSmkNoSound) && format.present()) {
        SDL_AudioSpec spec{SDL_AUDIO_S16, format.channels, format.rate};
        movie_stream_ = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
        if (movie_stream_)
            SDL_ResumeAudioStreamDevice(movie_stream_);
        winlose_movie_audio_.insert(winlose_movie_audio_.end(), winlose_movie_.audio(0).begin(),
                                    winlose_movie_.audio(0).end());
    }
}

void App::advance_winlose_movie()
{
    if (winlose_movie_index_ < 0)
        return;
    bool ended = false;
    winlose_movie_time_ += frame_seconds_ * 1e6;
    while (!ended && winlose_movie_time_ >= double(winlose_movie_.frame_duration_us())) {
        winlose_movie_time_ -= double(winlose_movie_.frame_duration_us());
        if (!winlose_movie_.next_frame()) {
            ended = true;
            break;
        }
        copy_frame_to_screen(winlose_movie_, winlose_movie_surface_, winlose_movie_flags_);
        winlose_movie_palette_ = winlose_movie_.palette();
        if (!(winlose_movie_flags_ & kSmkNoSound))
            winlose_movie_audio_.insert(winlose_movie_audio_.end(), winlose_movie_.audio(0).begin(),
                                        winlose_movie_.audio(0).end());
    }
    if (ended) {
        winlose_movie_index_ = -1;
        movie_playing_ = -1;
        winlose_movie_audio_.clear();
        if (movie_stream_) {
            SDL_DestroyAudioStream(movie_stream_);
            movie_stream_ = nullptr;
        }
    }
}

// One frame of the win or lose sequence: its movie, then its letter. Accepting
// a win starts the next level; dismissing the losing letter returns to the
// main menu, as the original does.
void App::winlose_step(int out_w, int out_h)
{
    Game &game = map_->game;
    if (winlose_movie_index_ >= 0) {
        advance_winlose_movie();
        if (movie_stream_ && !winlose_movie_audio_.empty()) {
            SDL_PutAudioStreamData(movie_stream_, winlose_movie_audio_.data(),
                                   int(winlose_movie_audio_.size() * sizeof(int16_t)));
            winlose_movie_audio_.clear();
        }
        SDL_FRect mat = fit_rect(winlose_movie_surface_, out_w, out_h);
        draw_fitted(winlose_movie_surface_, winlose_movie_palette_, winlose_tex_, mat);
        return;
    }
    const Surface &src = winlose_.surface();
    SDL_FRect at = fit_rect(src, out_w, out_h);
    UiMouse mouse = fitted_mouse(at, src);
    WinLoseKeys keys;
    keys.escape = std::exchange(escape_, false);
    keys.space = std::exchange(chart_space_, false);
    WinLoseView view;
    view.win = game.progress.level_won != 0;
    view.level = game.level_number();
    view.score = game.progress.score;
    view.next_level_offer = game.progress.next_level_offer;
    view.pick = winlose_pick_;
    view.lose_font = 0x13;
    view.hospital_name = startup_.player_name();
    WinLoseResult r = winlose_.frame(mouse, keys, frame_seconds_, view);
    for (uint8_t sfx : r.sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    if (r.closed) {
        winlose_.close();
        winlose_showing_ = false;
        if (r.next_level > 0) {
            load_level(r.next_level);
            help_.set_page(-1, -1, level_briefing(data_->text(), r.next_level));
        } else if (winlose_to_menu_ && !level_from_cli_) {
            // DM_MAIN_MENU: back to the main menu (the game is unloaded).
            save_continue_slot();
            map_.reset();
            in_startup_ = true;
            startup_.return_to_main_menu();
        } else {
            game.paused = false;
        }
        winlose_to_menu_ = false;
    }
    draw_fitted(src, winlose_.palette(), winlose_tex_, at);
}

// One frame of the start-up screens: drawn aspect-fit in the window, with
// the movies' sound on a stream of its own. Starts the level they choose.
void App::startup_step(int out_w, int out_h)
{
    const Surface &src = startup_.surface();
    SDL_FRect at = fit_rect(src, out_w, out_h);
    UiMouse mouse = fitted_mouse(at, src);
    help_mouse_ = mouse;
    help_context();
    StartupResult r = startup_.frame(mouse, startup_keys_, frame_seconds_);
    startup_keys_.clear();
    for (uint8_t sfx : startup_.take_sounds())
        sounds_.play_sound(kSourceGeneral, sfx);

    // The movie's sound: a stream of the movie's format, emptied when the
    // movie changes.
    SmackerAudioFormat format = startup_.movie_audio_format();
    if (startup_.movie() != movie_playing_ || !format.present()) {
        if (movie_stream_)
            SDL_DestroyAudioStream(movie_stream_);
        movie_stream_ = nullptr;
        movie_playing_ = startup_.movie();
        if (format.present()) {
            SDL_AudioSpec spec{SDL_AUDIO_S16, format.channels, format.rate};
            movie_stream_ = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
            if (movie_stream_)
                SDL_ResumeAudioStreamDevice(movie_stream_);
        }
    }
    std::vector<int16_t> pcm;
    startup_.take_movie_audio(pcm);
    if (movie_stream_ && !pcm.empty())
        SDL_PutAudioStreamData(movie_stream_, pcm.data(), int(pcm.size() * sizeof(int16_t)));

    draw_fitted(src, startup_.palette(), startup_tex_, at);

    startup_result(r);
}

// What the start-up screens chose: quit, or start a level, or load a save
// (the main menu's Continue, or the load game screen's chosen slot).
void App::startup_result(const StartupResult &r)
{
    if (r.kind == StartupResult::Kind::Running)
        return;
    if (movie_stream_)
        SDL_DestroyAudioStream(movie_stream_);
    movie_stream_ = nullptr;
    if (r.kind == StartupResult::Kind::Quit) {
        running_ = false;
    } else if (r.kind == StartupResult::Kind::StartLevel) {
        in_startup_ = false;
        balance_id_ = r.balance_id;
        tutorial_ = r.tutorial;
        load_level(r.level);
    } else if (r.kind == StartupResult::Kind::LoadGame) {
        if (perform_load(r.slot)) {
            in_startup_ = false;
        } else {
            // loadscr state 1 on failure returns to the load game screen
            // (state 4); the main menu's Continue (mainmenu.cpp
            // MMS_CONTINUE_FAILED) returns to the main menu. The screens must
            // stay up: no level was loaded.
            in_startup_ = true;
            startup_.load_failed(r.slot != kContinueSlot);
        }
    }
}

std::filesystem::path App::report_base() const
{
    const char *dir = std::getenv("TH_DEBUG_REPORT_DIR");
    return dir && *dir ? std::filesystem::path(dir) : std::filesystem::path("debug-reports");
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
        return std::format("win/lose (movie {})", winlose_movie_index_);
    if (fax_.open)
        return "fax";
    if (charts_.open)
        return std::format("chart {}", charts_.chart);
    if (town_.is_open())
        return "town map";
    return "hospital";
}

std::string App::debug_state() const
{
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(window_, &w, &h);
    float mx, my;
    SDL_GetMouseState(&mx, &my);
    std::string s = "# Theme Hospital Enhanced debug report\n";
    s += std::format("data: {}\n", data_ ? data_->dir().string() : "");
    s += std::format("window: {}x{} px, zoom {}, UI scale {}\n", w, h, zoom(), ui_scale());
    s += std::format("screen: {}", screen_name());
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
        s += std::format("startup screen: {}\n", int(startup_.screen()));
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
    s += std::format("route planner: {}\n", g.world.path_finder == PathFinder::AStar ? "A*" : "original");
    s += std::format("panel info: kind {}, thing {}, shown {}\n", panel_info_, panel_thing_,
                     panel_info_drawn_ != 0 ? "true" : "false");
    s += std::format("panel message: \"{}\" ({} frames)\n", panel_message_, panel_message_time_);
    s += std::format("placing: object {}, staff {}, build mouse {}\n", g.world.placing.placing, hire_.placing(),
                     int(g.world.building.mouse));
    s += std::format("date: year {} month {} day {}, turn {}\n", g.date.year, g.date.month, g.date.day, g.turn_count);
    s += std::format("open: {}, opening countdown {}\n", g.open, g.opening_countdown);
    s += std::format("balance: {}, reputation {}\n", g.balance, g.arrivals.hospitals[0].hospital_reputation);
    s += std::format("last command: type={} value={} flags={} arg={}\n", last_event_.type, last_event_.value,
                     last_event_.flags, last_event_.arg);
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

void App::make_ui_textures()
{
    for (PointerTexture &pt : pointers_)
        if (pt.texture)
            SDL_DestroyTexture(std::exchange(pt.texture, nullptr));
    if (!ui_tex_) {
        ui_.resize(kUiWidth, kUiHeight);
        ui_tex_ =
            SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, kUiWidth, kUiHeight);
        SDL_SetTextureScaleMode(ui_tex_, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(ui_tex_, SDL_BLENDMODE_BLEND);
    }
    for (uint8_t id : kPointersDrawn) {
        Sprite s = data_->pointers().at(id);
        PointerTexture &pt = pointers_[id];
        pt.w = s.width;
        pt.h = s.height;
        std::vector<uint32_t> pixels(size_t(pt.w) * size_t(pt.h));
        sprite_to_argb(s, data_->palette(), pixels.data(), pt.w * 4);
        pt.texture = argb_texture(renderer_, pixels, pt.w, pt.h);
    }
}

int App::zoom() const
{
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(window_, &w, &h);
    if (zoom_ > 0)
        return zoom_;
    return std::max(1, h / kMinViewHeight);
}

UiMouse App::ui_mouse(int ox, int oy, int scale) const
{
    float mx, my;
    SDL_GetMouseState(&mx, &my);
    float density = SDL_GetWindowPixelDensity(window_);
    UiMouse m;
    m.x = (int(mx * density) - ox) / scale;
    m.y = (int(my * density) - oy) / scale;
    m.left_held = buttons_.left;
    m.right_held = buttons_.right;
    m.left_pressed = buttons_.left_pressed;
    m.right_pressed = buttons_.right_pressed;
    m.left_released = buttons_.left_released;
    m.right_released = buttons_.right_released;
    return m;
}

// Runs and draws the menu bar for the frame: it spans the window's width
// (at least the game's 640) across the top.
void App::menu_step(int out_w, int scale)
{
    int width = std::max(kUiWidth, out_w / scale);
    int ox = (out_w - width * scale) / 2;
    MenuContext context;
    context.shadows = shadows_;
    context.sound = sounds_.sfx_on();
    context.sound_volume = sounds_.volume_item();
    context.music = music_.music_on();
    context.music_volume = music_.volume_item();
    context.announcements = sounds_.ambients_on();
    context.announcement_volume = sounds_.ambient_volume_item();
    context.speed = speed_ == Speed::Slowest  ? 0
                    : speed_ == Speed::Slower ? 1
                    : speed_ == Speed::Normal ? 2
                    : speed_ == Speed::Max    ? 3
                                              : 4;
    context.paused = map_ && map_->game.paused;
    context.disabled = interface_.modal() || (map_ && map_->game.world.placing.menu_disabled);
    for (int i = 0; i < int(context.save_slots.size()); i++)
        context.save_slots[size_t(i)] = save_slots_[size_t(i + 1)];
    perform(menu_frame(menu_, ui_mouse(ox, 0, scale), context));

    if (menu_layer_.width() != width)
        menu_layer_.resize(width, kUiHeight);
    menu_layer_.clear();
    draw_menu_bar(menu_layer_, menu_);
    std::vector<uint32_t> pixels = menu_layer_.to_argb(data_->palette());
    if (menu_tex_ && menu_tex_->w != width) {
        SDL_DestroyTexture(menu_tex_);
        menu_tex_ = nullptr;
    }
    if (!menu_tex_) {
        menu_tex_ =
            SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, kUiHeight);
        SDL_SetTextureScaleMode(menu_tex_, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(menu_tex_, SDL_BLENDMODE_BLEND);
    }
    SDL_UpdateTexture(menu_tex_, nullptr, pixels.data(), width * 4);
    SDL_FRect dst = {float(ox), 0, float(width * scale), float(kUiHeight * scale)};
    SDL_RenderTexture(renderer_, menu_tex_, nullptr, &dst);
}

// Carries out what the menu bar asked for; what does not exist yet is
// ignored.
void App::perform(const MenuEvent &event)
{
    play_menu_sound(event.sound);
    switch (event.action) {
    case MenuAction::Speed:
        speed_ = menu_speed(event.value);
        break;
    case MenuAction::ToggleShadows:
        shadows_ = !shadows_;
        break;
    case MenuAction::ToggleSound:
        sounds_.toggle_sound();
        break;
    case MenuAction::SoundVolume:
        sounds_.set_volume_item(event.value);
        break;
    case MenuAction::ToggleMusic:
        music_.toggle_music();
        break;
    case MenuAction::MusicVolume:
        music_.set_volume_item(event.value);
        break;
    case MenuAction::ToggleAnnouncements:
        sounds_.toggle_ambient();
        break;
    case MenuAction::AnnouncementVolume:
        sounds_.set_ambient_volume_item(event.value);
        break;
    case MenuAction::Quit:
        ask_quit();
        break;
    case MenuAction::Screen:
        open_menu_screen(event.value);
        break;
    case MenuAction::Jukebox:
        if (map_)
            map_->game.event = {EV_OPEN_JUKEBOX};
        break;
    case MenuAction::Restart:
        ask_restart();
        break;
    case MenuAction::Briefing:
        // Charts > Briefing: show_help_page(1, level), refused while an
        // edited room empties (menu.cpp:487).
        if (map_ && map_->game.world.building.mouse == BuildMouse::EditRoomWait)
            sounds_.play_sound(kSourceGeneral, kSfxWrong2);
        else if (map_)
            help_.set_page(-1, -1, level_briefing(data_->text(), level_));
        break;
    case MenuAction::Save:
    case MenuAction::Load:
        perform_save_load(event);
        break;
    default:
        break;
    }
}

// pulldown_save and pulldown_load: a slot already used asks before it is
// overwritten (yesno_text[3], EV_SAVE_GAME on yes); a load goes to the slot
// (EV_LOAD_GAME).
void App::perform_save_load(const MenuEvent &event)
{
    if (event.value < 1 || event.value > kMenuSlots)
        return;
    if (event.action == MenuAction::Save) {
        if (!map_)
            return;
        if (save_slots_[size_t(event.value)]) {
            interface_.open_yes_no(data_->text().at(TextSection::YesNo, 3));
            pending_save_slot_ = event.value;
            asking_quit_ = false;
        } else {
            perform_save(event.value);
        }
    } else {
        perform_load(event.value);
    }
}

void App::play_menu_sound(MenuSound sound)
{
    switch (sound) {
    case MenuSound::Highlight:
        sounds_.play_sound(kSourceGeneral, kSfxHighlight5);
        break;
    case MenuSound::Select:
        sounds_.play_sound(kSourceGeneral, kSfxSelectX);
        break;
    case MenuSound::Wrong:
        sounds_.play_sound(kSourceGeneral, kSfxWrong2);
        break;
    case MenuSound::None:
        break;
    }
}

// The Charts menu's screen `chart`, refused with a sound when it cannot open.
void App::open_menu_screen(int chart)
{
    if (!map_)
        return;
    bool refused = false;
    Event e = open_chart_command(chart, map_->game, &refused);
    if (refused)
        sounds_.play_sound(kSourceGeneral, kSfxWrong2);
    else
        map_->game.event = e;
}

void App::handle_event(const SDL_Event &e)
{
    ImGui_ImplSDL3_ProcessEvent(&e);
    const ImGuiIO &io = ImGui::GetIO();
    switch (e.type) {
    case SDL_EVENT_QUIT:
        running_ = false;
        break;
    case SDL_EVENT_KEY_DOWN:
        if (!io.WantCaptureKeyboard && !quick_.open)
            key_down(e.key);
        break;
    case SDL_EVENT_KEY_UP:
        key_up(e.key);
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (!io.WantCaptureMouse && !quick_.open)
            mouse_down(e.button);
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (!quick_.open)
            mouse_up(e.button);
        break;
    case SDL_EVENT_MOUSE_MOTION:
        if (dragging_ && data_ && !quick_.open) {
            float density = SDL_GetWindowPixelDensity(window_);
            scroll_x_ -= e.motion.xrel * density / float(zoom());
            scroll_y_ -= e.motion.yrel * density / float(zoom());
        }
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        if (!io.WantCaptureMouse && data_ && !quick_.open) {
            // render() keeps the view centred when the surface changes size.
            zoom_ = std::clamp(zoom() + (e.wheel.y > 0 ? 1 : e.wheel.y < 0 ? -1 : 0), 1, 4);
        }
        break;
    default:
        break;
    }
}

// A key pressed, outside the developer tools.
void App::key_down(const SDL_KeyboardEvent &key)
{
    if (key.key == SDLK_F10 && data_) {
        quick_report_key(SDL_Keymod(key.mod));
        return;
    }
    if (key.key == SDLK_F12 && data_ && in_startup_) {
        tools_.open = !tools_.open;
        return;
    }
    if (in_startup_) {
        startup_keys_.push_back(startup_key(key.key));
        return;
    }
    if (key.key == SDLK_F12) {
        tools_.open = !tools_.open;
    } else if (!data_) {
        if (key.key == SDLK_ESCAPE)
            running_ = false;
    } else if (key.key == SDLK_ESCAPE && !(key.mod & SDL_KMOD_SHIFT)) {
        // Closes the front requester (the game's Escape).
        escape_ = true;
    } else if (interface_.modal() || town_.is_open()) {
        // A modal requester takes the keys; the town map only Escape.
    } else {
        if (uint8_t bit = scroll_key_bit(key.key))
            scroll_held_ |= bit;
        game_key(key);
    }
}

// A key released: the scroll keys stop (move_windows clears its scroll.*
// every frame; the held set is what keeps them on).
void App::key_up(const SDL_KeyboardEvent &key)
{
    scroll_held_ &= uint8_t(~scroll_key_bit(key.key));
}

// The game's speed keys 1-5 (process_game_keys, src/keyboard.cpp): key 1 is
// ignored with Shift, its cheat. True when `key` is one and set the speed.
bool App::game_speed_key(const SDL_KeyboardEvent &key)
{
    if (key.key < SDLK_1 || key.key > SDLK_5)
        return false;
    if (key.key == SDLK_1 && (key.mod & SDL_KMOD_SHIFT))
        return false;
    speed_ = menu_speed(10 * int(key.key - SDLK_1 + 1));
    return true;
}

// Alt+S quick saves and Alt+L quick loads debug_save_slot (keyboard.cpp's
// KC_S and KC_L with MOD_ALT).
bool App::quick_save_load_key(const SDL_KeyboardEvent &key)
{
    if (!map_ || !(key.mod & SDL_KMOD_ALT) || (key.key != SDLK_S && key.key != SDLK_L))
        return false;
    if (key.key == SDLK_S)
        perform_save(debug_save_slot_);
    else
        perform_load(debug_save_slot_);
    return true;
}

// A key pressed over the hospital or a chart, with no modal requester up.
void App::game_key(const SDL_KeyboardEvent &key)
{
    if (fax_.open) {
        // The fax reads Escape and Space alone.
        chart_space_ = chart_space_ || key.key == SDLK_SPACE;
        return;
    }
    if (year_end_showing_ || winlose_showing_) {
        // The year-end screens and the letters read Escape and Space alone.
        chart_space_ = chart_space_ || key.key == SDLK_SPACE;
        return;
    }
    if (map_ && game_speed_key(key))
        return;
    if (quick_save_load_key(key))
        return;
    if (key.key >= SDLK_F1 && key.key <= SDLK_F8 && map_) {
        // The charts, from the game or another chart; not while paused.
        if (!map_->game.paused)
            open_chart_key(int(key.key - SDLK_F1));
    } else if (charts_.open) {
        // The charts take Escape, Space and the arrows only.
        chart_space_ = chart_space_ || key.key == SDLK_SPACE;
    } else if ((key.key == SDLK_ESCAPE && (key.mod & SDL_KMOD_SHIFT)) ||
               ((key.key == SDLK_Q || key.key == SDLK_X) &&
                (key.mod & (SDL_KMOD_CTRL | SDL_KMOD_ALT | SDL_KMOD_SHIFT)))) {
        // The game's quit keys ask first.
        ask_quit();
    } else if (key.key == SDLK_P && map_) {
        map_->game.paused = !map_->game.paused;
    } else if (key.key == SDLK_T && map_) {
        open_town_ = true;
    } else if (key.key == SDLK_J && !(key.mod & SDL_KMOD_SHIFT) && map_ && !map_->game.paused) {
        map_->game.event = {EV_OPEN_JUKEBOX};
    }
}

// F1-F8: the chart `chart`, refused with a sound when it cannot open.
void App::open_chart_key(int chart)
{
    bool refused = false;
    Event ev = open_chart_command(chart, map_->game, &refused);
    if (refused)
        sounds_.play_sound(kSourceGeneral, kSfxWrong2);
    else
        map_->game.event = ev;
}

// The inspector's pick: a left click on a person or object inspects it, a
// right click gives up; the game sees neither.
void App::inspect_click(uint8_t button)
{
    InspectTool &inspect = tools_.inspect;
    if (button == SDL_BUTTON_RIGHT)
        inspect.picking = false;
    if (button != SDL_BUTTON_LEFT || !pointed_.thing)
        return;
    inspect.thing = pointed_.thing;
    inspect.picking = false;
}

void App::mouse_down(const SDL_MouseButtonEvent &button)
{
    if (tools_.inspect.picking && map_) {
        inspect_click(button.button);
        return;
    }
    if (button.button == SDL_BUTTON_LEFT)
        buttons_.left = buttons_.left_pressed = true;
    if (button.button == SDL_BUTTON_RIGHT)
        buttons_.right = buttons_.right_pressed = true;
    // An open menu, a modal requester or the town map takes the clicks.
    if (menu_.active || interface_.modal() || town_.is_open() || charts_.open || fax_.open || year_end_showing_ ||
        winlose_showing_)
        return;
    // While something is being placed, its clicks are the interface's.
    if (map_ && (map_->game.world.placing.placing || hire_.placing()))
        return;
    if (button.button == SDL_BUTTON_MIDDLE || button.button == SDL_BUTTON_RIGHT)
        dragging_ = true;
}

void App::mouse_up(const SDL_MouseButtonEvent &button)
{
    if (button.button == SDL_BUTTON_LEFT && buttons_.left) {
        buttons_.left = false;
        buttons_.left_released = true;
    }
    if (button.button == SDL_BUTTON_RIGHT && buttons_.right) {
        buttons_.right = false;
        buttons_.right_released = true;
    }
    if (button.button == SDL_BUTTON_MIDDLE || button.button == SDL_BUTTON_RIGHT)
        dragging_ = false;
}

void App::update(float dt)
{
    if (!data_ || in_startup_)
        return;
    if (quick_.open)
        return;
    if (restart_level_) {
        restart_level_ = false;
        load_level(level_);
        return;
    }
    if (map_ && !map_->game.paused)
        run_turns(dt);
    if (map_ && !year_end_showing_ && !winlose_showing_)
        check_year_end();
    if (year_end_showing_ || winlose_showing_ || town_.is_open() || charts_.open || fax_.open)
        return;
    // move_windows runs once a game frame, not once a rendered frame, so the
    // scroll speed follows the game's 40 ms frame, not the display's refresh.
    scroll_frame_time_ += double(dt);
    if (scroll_frame_time_ < kGameFrame)
        return;
    scroll_frame_time_ = std::min(scroll_frame_time_ - kGameFrame, kGameFrame);
    scroll_step(scroll_direction());
}

// The scroll direction this frame: the held keys, the pointer's window edge
// replacing them, an open pulldown ignoring both, and opposite keys
// cancelling, as keyboard.cpp, mouse.cpp and io.cpp set move_windows' flags.
uint8_t App::scroll_direction()
{
    const ImGuiIO &io = ImGui::GetIO();
    uint8_t dir = io.WantCaptureKeyboard ? 0 : scroll_held_;
    if ((dir & kScrollUp) && (dir & kScrollDown))
        dir &= uint8_t(~(kScrollUp | kScrollDown));
    if ((dir & kScrollLeft) && (dir & kScrollRight))
        dir &= uint8_t(~(kScrollLeft | kScrollRight));
    if (!io.WantCaptureMouse && !dragging_ && SDL_GetMouseFocus() == window_) {
        uint8_t edge = 0;
        float mx, my;
        SDL_GetMouseState(&mx, &my);
        int w, h;
        SDL_GetWindowSize(window_, &w, &h);
        if (mx < kEdgeScrollMargin)
            edge |= kScrollLeft;
        else if (mx >= w - kEdgeScrollMargin)
            edge |= kScrollRight;
        if (my < kEdgeScrollMargin)
            edge |= kScrollUp;
        else if (my >= h - kEdgeScrollMargin)
            edge |= kScrollDown;
        if (edge)
            dir = edge;
    }
    return menu_.open ? 0 : dir;
}

// The turns due after `dt` seconds at the game's speed.
void App::run_turns(float dt)
{
    // The staff demand requester is modal (display_mode DM_MODAL): the game's
    // turns pause while it is open.
    if (map_ && interface_.requester() == Requester::StaffDemand)
        return;
    // At most a few frames' worth of turns after a stall.
    turn_clock_ = std::min(turn_clock_ + dt * double(int(speed_)), 8.0);
    for (; turn_clock_ >= 1; turn_clock_ -= 1)
        map_->game.turn();
}

// move_windows (src/cakewin.cpp): the scroll direction `dir` accelerates the
// view's velocity while held (and stops it dead when none is), the velocity
// moves the view by its whole pixels with the fraction carried, clamped to
// one tile a frame, stopped at the scroll limit, and then decays.
void App::scroll_step(uint8_t dir)
{
    dir &= 0xf;
    if (!dir) {
        scroll_vel_x_ = 0;
        scroll_vel_y_ = 0;
    } else {
        if (dir & kScrollLeft)
            scroll_vel_x_ -= kScrollAccelX;
        if (dir & kScrollRight)
            scroll_vel_x_ += kScrollAccelX;
        if (dir & kScrollUp)
            scroll_vel_y_ -= kScrollAccelY;
        if (dir & kScrollDown)
            scroll_vel_y_ += kScrollAccelY;
    }
    int32_t dx = scroll_vel_x_ >> 16;
    int32_t dy = scroll_vel_y_ >> 16;
    scroll_frac_x_ += scroll_vel_x_ - (dx << 16);
    scroll_frac_y_ += scroll_vel_y_ - (dy << 16);
    int32_t carry_x = scroll_frac_x_ >> 16;
    int32_t carry_y = scroll_frac_y_ >> 16;
    scroll_frac_x_ -= carry_x << 16;
    scroll_frac_y_ -= carry_y << 16;
    dx += carry_x;
    dy += carry_y;
    // The mouse drag's own pixels ride along with the velocity's.
    int32_t drag_x = int32_t(scroll_x_), drag_y = int32_t(scroll_y_);
    scroll_x_ -= float(drag_x);
    scroll_y_ -= float(drag_y);
    dx += drag_x;
    dy += drag_y;
    // The game refuses, dead, a scroll that would take the view's centre out
    // of the level's scroll area: both axes stop and the velocity is lost.
    Point centre = {camera_.x + surface_.width() / 2, camera_.y + surface_.height() / 2};
    if (beyond_scroll_limit({centre.x + dx, centre.y + dy}, scroll_min_, scroll_max_)) {
        scroll_vel_x_ = 0;
        scroll_vel_y_ = 0;
        scroll_frac_x_ = 0;
        scroll_frac_y_ = 0;
        dx = 0;
        dy = 0;
    }
    dx = std::clamp(dx, -kTileWidth, kTileWidth);
    dy = std::clamp(dy, -kTileHeight, kTileHeight);
    camera_.x += dx;
    camera_.y += dy;
    scroll_vel_x_ = (scroll_vel_x_ * kScrollDecay) >> 8;
    scroll_vel_y_ = (scroll_vel_y_ * kScrollDecay) >> 8;
}

bool App::screen_step(int out_w, int out_h)
{
    if (in_startup_) {
        startup_step(out_w, out_h);
    } else if (map_ && year_end_showing_) {
        year_end_step(out_w, out_h);
    } else if (map_ && winlose_showing_) {
        winlose_step(out_w, out_h);
    } else if (map_ && fax_.open) {
        fax_step(out_w, out_h);
    } else if (map_ && charts_.open) {
        charts_step(out_w, out_h);
    } else if (map_ && (town_.is_open() || std::exchange(open_town_, false))) {
        if (!town_.is_open())
            open_town_map();
        town_step(out_w, out_h);
    } else {
        return false;
    }
    mixer_.update(sounds_, &music_);
    draw_pointer(ui_scale());
    return true;
}

void App::render()
{
    frame_++;
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    int out_w = 0, out_h = 0;
    SDL_GetCurrentRenderOutputSize(renderer_, &out_w, &out_h);
    SDL_SetRenderScale(renderer_, 1.0f, 1.0f);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);

    if (!data_) {
        MissingDataAction action = draw_missing_data_screen(search_, data_error_);
        if (action == MissingDataAction::Quit)
            running_ = false;
        else if (action == MissingDataAction::Choose)
            SDL_ShowOpenFolderDialog(on_folder_chosen, &pick_, window_, nullptr, false);
    } else if (!screen_step(out_w, out_h)) {
        hospital_step(out_w, out_h);
    }

    if (data_)
        draw_tools(tools_, map_ && !in_startup_ ? &map_->game.world : nullptr);
    // Read back before ImGui draws, so the screenshot shows the game alone.
    if (std::exchange(tools_.report.save, false))
        save_debug_report();
    draw_quick_report();
    if (!drive_shot_.empty())
        save_drive_screenshot(std::exchange(drive_shot_, {}));

    ImGui::Render();
    const ImGuiIO &io = ImGui::GetIO();
    SDL_SetRenderScale(renderer_, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer_);
    if (!drive_shot_ui_.empty())
        save_drive_screenshot(std::exchange(drive_shot_ui_, {}));
    SDL_RenderPresent(renderer_);
    buttons_.left_pressed = buttons_.right_pressed = false;
    buttons_.left_released = buttons_.right_released = false;
}

void App::save_drive_screenshot(const std::filesystem::path &path)
{
    SDL_Surface *shot = SDL_RenderReadPixels(renderer_, nullptr);
    if (!shot || !SDL_SavePNG(shot, path.string().c_str()))
        std::fprintf(stderr, "drive: screenshot %s: %s\n", path.string().c_str(), SDL_GetError());
    if (shot)
        SDL_DestroySurface(shot);
}

// One frame of the hospital: the view at its zoom with what is being placed
// over it, its sounds, then the interface, the menu bar and the pointer at
// the UI scale.
void App::hospital_step(int out_w, int out_h)
{
    int z = zoom();
    int vw = std::max(out_w / z, 1), vh = std::max(out_h / z, 1);
    resize_view(vw, vh);

    float cmx, cmy;
    SDL_GetMouseState(&cmx, &cmy);
    float cdensity = SDL_GetWindowPixelDensity(window_);
    pointed_.at = {int(cmx * cdensity) / z, int(cmy * cdensity) / z};
    near_rat_ = false;
    frame_sounds_.clear();
    // The earthquake's shake offset (shake_screen) moved the original view.
    Point shaken = {camera_.x + map_->game.world.shake_x, camera_.y + map_->game.world.shake_y};
    draw_view(surface_, *data_, map_.get(), shaken, shadows_, {&frame_sounds_, &pointed_, &near_rat_});
    if (map_)
        view_sounds(vw, vh);
    mixer_.update(sounds_, &music_);

    // What is being placed, drawn over the view as the game does.
    Point under = world_to_cell({camera_.x + pointed_.at.x, camera_.y + pointed_.at.y});
    bool inside = under.x >= 0 && under.x < kMapSize && under.y >= 0 && under.y < kMapSize;
    map_cell_ = inside ? under.x + under.y * kMapSize : -1;
    if (map_)
        draw_view_overlays(vw, vh);
    present_view(vw, vh, z);

    // The interface and the pointer keep their size whatever the view's
    // zoom.
    int s = ui_scale();
    interface_step(out_w, out_h, s);
    menu_step(out_w, s);

    draw_pointer(s, menu_.active || interface_.modal() ? kPointerDefault : pointer_sprite_);

    if (tools_.open && map_)
        draw_nav_overlay(tools_.nav, map_->game.world, camera_, float(z) / ImGui::GetIO().DisplayFramebufferScale.x);
}

// Makes the view's surface and texture `vw` x `vh`.
void App::resize_view(int vw, int vh)
{
    if (vw != surface_.width() || vh != surface_.height()) {
        // Keep the same map point at the view's centre across resizes.
        Point centre = {camera_.x + surface_.width() / 2, camera_.y + surface_.height() / 2};
        surface_.resize(vw, vh);
        camera_ = {centre.x - vw / 2, centre.y - vh / 2};
    }
    if (!screen_ || screen_w_ != vw || screen_h_ != vh) {
        if (screen_)
            SDL_DestroyTexture(screen_);
        screen_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, vw, vh);
        SDL_SetTextureScaleMode(screen_, SDL_SCALEMODE_NEAREST);
        screen_w_ = vw;
        screen_h_ = vh;
    }
}

// The sounds of what was drawn, positioned on the `vw` x `vh` view.
void App::view_sounds(int vw, int vh)
{
    sounds_.frames_drawn(frame_sounds_, vw);
    sounds_.manage_sound(map_->game.world, camera_, vw, vh);
    const World::Emergency &emergency = map_->game.world.emergency;
    sounds_.manage_background_sounds(map_->game.paused,
                                     emergency.timer > 0 ? std::optional<int>(emergency.timer) : std::nullopt);
    // twice_daily_admin stops the clock as the emergency's time runs out.
    if (emergency.timer <= 0 && sounds_.playing(kSourceClock, 0xa9))
        sounds_.stop_sample(kSourceClock, 0xa9);
    // The worn machines' warnings and announcements, and explosions.
    World &world = map_->game.world;
    for (uint8_t sfx : world.machine_sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    for (uint8_t announcement : world.machine_announcements)
        sounds_.play_ambient(announcement, has_receptionist(world), map_->game.paused);
    world.machine_sounds.clear();
    world.machine_announcements.clear();
    for (uint8_t sfx : world.epidemic_sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    world.epidemic_sounds.clear();
    for (uint8_t sfx : world.treatment_sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    world.treatment_sounds.clear();
    for (uint8_t sfx : world.rat_sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    world.rat_sounds.clear();
    // The music moves on to the next song, and the announcer speaks, as
    // the game does each frame; only the announcer while a modal
    // requester is up.
    if (!interface_.modal())
        music_.play_next(map_->game.paused);
    sounds_.do_ambients(map_->game.paused, false, has_receptionist(map_->game.world));
}

// What is being placed (an object, a room's blueprint, a member of staff),
// over the `vw` x `vh` view.
void App::draw_view_overlays(int vw, int vh)
{
    Rect view{0, 0, vw, vh};
    draw_placement(surface_, view, camera_, map_->game.world.placing, object_ui_, *data_);
    draw_build_overlay({surface_, view, camera_}, map_->game, build_ui_, object_ui_, *data_);
    if (hire_.placing() && map_cell_ >= 0)
        draw_held_staff({surface_, view, camera_}, hire_, map_->game.world, map_cell_, *data_, map_->scratch);
}

// The `vw` x `vh` view in the window at zoom `z`.
void App::present_view(int vw, int vh, int z)
{
    void *pixels;
    int pitch;
    if (!screen_ || !SDL_LockTexture(screen_, nullptr, &pixels, &pitch))
        return;
    const Palette &pal = data_->palette();
    const uint8_t *src = surface_.data();
    for (int y = 0; y < vh; y++) {
        auto *row = reinterpret_cast<uint32_t *>(static_cast<uint8_t *>(pixels) + y * pitch);
        for (int x = 0; x < vw; x++)
            row[x] = pal[src[x]];
        src += vw;
    }
    SDL_UnlockTexture(screen_);
    SDL_FRect dst = {0, 0, float(vw * z), float(vh * z)};
    SDL_RenderTexture(renderer_, screen_, nullptr, &dst);
}

void App::run()
{
    // A driven run starts with the pointer at the window's centre, not at the
    // window's (0,0) edge, so the camera starts where the level puts it and
    // does not edge-scroll until a script moves the pointer there.
    if (driver_) {
        int w = 0, h = 0;
        SDL_GetWindowSize(window_, &w, &h);
        SDL_WarpMouseInWindow(window_, float(w) / 2.0f, float(h) / 2.0f);
    }
    uint64_t last = SDL_GetTicksNS();
    while (running_) {
        // A driven run moves on only as its commands ask for frames.
        if (driver_) {
            if (driver_->finished()) {
                driver_->flush();
                break;
            }
            if (!driver_->frame(*this)) {
                SDL_Delay(5);
                continue;
            }
        }
        SDL_Event e;
        while (SDL_PollEvent(&e))
            handle_event(e);
        // The `type` text events are polled; SDL no longer needs the strings.
        drive_texts_.clear();

        std::optional<std::string> chosen;
        {
            std::lock_guard lock(pick_.mutex);
            chosen.swap(pick_.chosen);
        }
        if (chosen) {
            std::vector<std::string> missing = missing_data_files(*chosen);
            if (!missing.empty()) {
                data_error_ = *chosen + " does not hold the game data (missing " + missing.front() + ")";
            } else if (try_load_data(*chosen)) {
                settings_.set("data_dir", *chosen);
                settings_.save();
            }
        }

        uint64_t now = SDL_GetTicksNS();
        float dt = float(now - last) / 1e9f;
        last = now;
        if (driver_) {
            dt = 1.0f / 60.0f;
            drive_ns_ += 16666667;
        }
        frame_seconds_ = std::min(dt, 0.1f);
        update(frame_seconds_);
        render();
    }
}

void App::parse_drive_args(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--drive" && i + 1 < argc)
            drive_script_ = argv[++i];
        else if (arg == "--out" && i + 1 < argc)
            drive_out_ = argv[++i];
        else if (arg == "--size" && i + 1 < argc)
            std::sscanf(argv[++i], "%dx%d", &drive_w_, &drive_h_);
        else if (arg == "--control")
            drive_control_ = true;
        else if (arg == "--visible")
            drive_visible_ = true;
    }
}

void App::hold_drive_slot()
{
    const char *env = std::getenv("TH_DRIVE_SLOTS");
    int max = env ? std::max(1, std::atoi(env)) : 6;
    // The executable sits in build/<tree>/ of the repository.
    const char *base = SDL_GetBasePath();
    std::filesystem::path dir = std::filesystem::path(base ? base : "").parent_path().parent_path() / "slots" / "drive";
    std::string info =
        drive_control_ ? std::string("th_enhanced --control") : "th_enhanced --drive " + drive_script_.string();
    if (!hold_slot(dir, max, info))
        std::fprintf(stderr, "drive: no slot files in %s; running without a slot\n", dir.string().c_str());
}

void App::start_driver()
{
    // English unless --language says otherwise, whatever HOSPITAL.CFG holds,
    // so that expectations on text hold everywhere.
    if (!language_)
        language_ = kEnglish;
    driver_ = std::make_unique<Driver>(drive_out_, drive_control_);
    if (!drive_control_) {
        std::ifstream in(drive_script_);
        if (!in)
            std::fprintf(stderr, "drive: cannot read %s\n", drive_script_.string().c_str());
        for (std::string line; std::getline(in, line);)
            driver_->add_line(line);
        driver_->end_input();
        return;
    }
    // Control commands come one per line on stdin, as the controller sends
    // them; the app waits between them.
    std::thread([driver = driver_.get()] {
        for (std::string line; std::getline(std::cin, line);)
            driver->add_line(line);
        driver->end_input();
    }).detach();
}

uint32_t App::ticks_ms() const
{
    return driver_ ? uint32_t(drive_ns_ / 1000000) : uint32_t(SDL_GetTicks());
}

bool App::drive_warp_world(Point p)
{
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(window_, &w, &h);
    int z = zoom();
    float px = float((p.x - camera_.x) * z), py = float((p.y - camera_.y) * z);
    if (px < 0 || py < 0 || px >= float(w) || py >= float(h))
        return false;
    float density = SDL_GetWindowPixelDensity(window_);
    SDL_WarpMouseInWindow(window_, px / density, py / density);
    return true;
}

bool App::drive_move_screen(int x, int y)
{
    if (x < 0 || y < 0 || x >= kUiWidth || y >= kUiHeight)
        return false;
    float px = 0, py = 0;
    if (fit_.frame == frame_ && fit_.w > 0 && fit_.h > 0) {
        px = fit_.at.x + (float(x) + 0.5f) * fit_.at.w / float(fit_.w);
        py = fit_.at.y + (float(y) + 0.5f) * fit_.at.h / float(fit_.h);
    } else {
        int w = 0, h = 0;
        SDL_GetWindowSizeInPixels(window_, &w, &h);
        int scale = ui_scale();
        px = float((w - kUiWidth * scale) / 2 + x * scale + scale / 2);
        py = float(h - kUiHeight * scale + y * scale + scale / 2);
    }
    float density = SDL_GetWindowPixelDensity(window_);
    SDL_WarpMouseInWindow(window_, px / density, py / density);
    return true;
}

bool App::drive_move_cell(int col, int row)
{
    if (!map_ || in_startup_ || col < 0 || row < 0 || col >= kMapSize || row >= kMapSize)
        return false;
    Point p = cell_to_world(col, row);
    return drive_warp_world({p.x, p.y + kTileHeight / 2});
}

bool App::drive_move_thing(uint16_t id)
{
    if (!map_ || in_startup_ || id == 0 || id >= map_->game.world.things.size())
        return false;
    const Thing &t = map_->game.world.things[id];
    if (t.type == kThingFree)
        return false;
    // Above the thing's feet, onto its sprite.
    Point p = thing_to_world(t);
    return drive_warp_world({p.x, p.y - kTileHeight / 2});
}

void App::drive_button(int button, bool down)
{
    SDL_Event e{};
    e.type = down ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
    e.button.windowID = SDL_GetWindowID(window_);
    e.button.button = uint8_t(button);
    e.button.down = down;
    e.button.clicks = 1;
    SDL_GetMouseState(&e.button.x, &e.button.y);
    SDL_PushEvent(&e);
}

bool App::drive_key(std::string_view name, bool down, uint16_t mods)
{
    SDL_Keycode key = SDL_GetKeyFromName(std::string(name).c_str());
    if (key == SDLK_UNKNOWN)
        return false;
    SDL_Keymod sdl_mods = SDL_KMOD_NONE;
    if (mods & kDriveModShift)
        sdl_mods |= SDL_KMOD_SHIFT;
    if (mods & kDriveModCtrl)
        sdl_mods |= SDL_KMOD_CTRL;
    if (mods & kDriveModAlt)
        sdl_mods |= SDL_KMOD_ALT;
    SDL_Event e{};
    e.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    e.key.windowID = SDL_GetWindowID(window_);
    e.key.key = key;
    e.key.scancode = SDL_GetScancodeFromKey(key, nullptr);
    e.key.mod = sdl_mods;
    e.key.down = down;
    SDL_PushEvent(&e);
    return true;
}

bool App::drive_text(std::string_view text)
{
    drive_texts_.emplace_back(text);
    SDL_Event e{};
    e.type = SDL_EVENT_TEXT_INPUT;
    e.text.windowID = SDL_GetWindowID(window_);
    e.text.text = drive_texts_.back().c_str();
    SDL_PushEvent(&e);
    return true;
}

void App::drive_screenshot(const std::filesystem::path &path)
{
    drive_shot_ = path;
}

void App::drive_screenshot_ui(const std::filesystem::path &path)
{
    drive_shot_ui_ = path;
}

std::string App::drive_state() const
{
    return debug_state();
}

std::string App::drive_report_dir() const
{
    return quick_.last_report;
}

int64_t App::drive_turn() const
{
    return map_ && !in_startup_ ? int64_t(map_->game.turn_count) : -1;
}

bool App::drive_staff_demand(uint16_t id)
{
    if (!map_ || in_startup_ || id == 0 || id >= map_->game.world.things.size())
        return false;
    open_staff_resign_requester(demand_, interface_, map_->game.world, id, sounds_.safeseed);
    return true;
}

void App::destroy_textures()
{
    if (quick_.capture.shot)
        SDL_DestroySurface(quick_.capture.shot);
    if (startup_tex_)
        SDL_DestroyTexture(startup_tex_);
    if (town_tex_)
        SDL_DestroyTexture(town_tex_);
    if (charts_tex_)
        SDL_DestroyTexture(charts_tex_);
    if (fax_tex_)
        SDL_DestroyTexture(fax_tex_);
    if (year_end_tex_)
        SDL_DestroyTexture(year_end_tex_);
    if (winlose_tex_)
        SDL_DestroyTexture(winlose_tex_);
    if (ui_tex_)
        SDL_DestroyTexture(ui_tex_);
    for (PointerTexture &pt : pointers_)
        if (pt.texture)
            SDL_DestroyTexture(pt.texture);
    if (crosshair_)
        SDL_DestroyCursor(crosshair_);
    if (screen_)
        SDL_DestroyTexture(screen_);
}

void App::shutdown()
{
    mixer_.close();
    if (movie_stream_)
        SDL_DestroyAudioStream(movie_stream_);
    destroy_textures();
    if (imgui_ready_) {
        ImGui_ImplSDLRenderer3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }
    if (renderer_)
        SDL_DestroyRenderer(renderer_);
    if (window_)
        SDL_DestroyWindow(window_);
    SDL_Quit();
}

} // namespace
} // namespace th

int main(int argc, char **argv)
{
    // --screenshot OUT.bmp [--size WxH] [--level N] [--language N] --data DIR
    std::string shot, data_dir;
    int level = 1, width = 1280, height = 720;
    std::optional<int> language;
    for (int i = 1; i + 1 < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--screenshot")
            shot = argv[++i];
        else if (arg == "--data")
            data_dir = argv[++i];
        else if (arg == "--level")
            level = std::atoi(argv[++i]);
        else if (arg == "--language")
            language = std::atoi(argv[++i]);
        else if (arg == "--size")
            std::sscanf(argv[++i], "%dx%d", &width, &height);
    }
    if (!shot.empty()) {
        if (data_dir.empty()) {
            std::fprintf(stderr, "--screenshot needs --data\n");
            return 2;
        }
        return th::screenshot(data_dir, level, width, height, shot, language);
    }

    th::App app;
    if (app.init(argc, argv))
        app.run();
    app.shutdown();
    return app.exit_code();
}

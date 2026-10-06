// Theme Hospital Enhanced: draws a level's map from the game's own data at
// any window size, with the bottom panel centred at native size and ImGui
// developer tools (F12).

#include "app/audio/mixer.h"
#include "app/datadir.h"
#include "app/settings.h"
#include "app/ui/tools.h"
#include "core/audio/music.h"
#include "core/audio/sounds.h"
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
#include "core/ui/hire_staff.h"
#include "core/ui/jukebox.h"
#include "core/ui/layer.h"
#include "core/ui/menubar.h"
#include "core/ui/startup.h"
#include "core/ui/town_map.h"
#include "core/ui/windows.h"
#include "core/world/debug_dump.h"
#include "core/world/game.h"
#include "core/world/snapshot.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>

namespace th
{
namespace
{

// The view never shows less than the original's 640x480; automatic zoom
// picks the largest integer scale that keeps at least this many lines.
constexpr int kMinViewHeight = 480;
constexpr int kEdgeScrollMargin = 8;   // pixels from the window edge
constexpr float kScrollSpeed = 900.0f; // view pixels per second
// The in-game pointer (PTR_DEFAULT), hotspot at its top-left corner.
constexpr size_t kPointerDefault = 1;

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
               std::vector<FrameSound> *sounds = nullptr, PointedThing *pointed = nullptr)
{
    surface.fill(0);
    if (level) {
        if (level->scratch.width() != surface.width() || level->scratch.height() != surface.height()) {
            level->scratch.resize(surface.width(), surface.height());
            level->scratch.fill(kMaskColour);
        }
        draw_world(surface, {0, 0, surface.width(), surface.height()}, camera, level->game.world, data,
                   shadows ? level->shadows.get() : nullptr, level->scratch, sounds, pointed);
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
    const uint8_t *src = surface.data();
    for (int y = 0; y < height; y++) {
        auto *row = reinterpret_cast<uint32_t *>(static_cast<uint8_t *>(img->pixels) + y * img->pitch);
        for (int x = 0; x < width; x++)
            row[x] = data.palette()[*src++];
    }
    // The panel at native size with the level's starting figures, as the
    // window draws it.
    const Game &g = map->game;
    PanelFigures figures{g.balance, g.date.day, g.date.month, g.reputation(), g.opening_countdown, g.open_countdown};
    int pw = 0, ph = 0;
    std::vector<uint32_t> panel = panel_to_argb(data, &figures, pw, ph);
    for (int y = 0; y < ph && height - ph + y >= 0; y++) {
        auto *row = reinterpret_cast<uint32_t *>(static_cast<uint8_t *>(img->pixels) + (height - ph + y) * img->pitch);
        for (int x = 0; x < pw; x++) {
            int dx = (width - pw) / 2 + x;
            if (dx >= 0 && dx < width && panel[size_t(y * pw + x)])
                row[dx] = panel[size_t(y * pw + x)];
        }
    }
    bool ok = SDL_SaveBMP(img, out.c_str());
    if (!ok)
        std::fprintf(stderr, "SDL_SaveBMP: %s\n", SDL_GetError());
    SDL_DestroySurface(img);
    return ok ? 0 : 1;
}

class App
{
  public:
    bool init(int argc, char **argv);
    void run();
    void shutdown();

  private:
    bool try_load_data(const std::filesystem::path &dir);
    void load_level(int number);
    void handle_event(const SDL_Event &e);
    void nav_click(uint8_t button, float x, float y);
    // The mouse in a layer drawn at (ox, oy) at `scale`.
    UiMouse ui_mouse(int ox, int oy, int scale) const;
    void menu_step(int out_w, int scale);
    void perform(const MenuEvent &event);
    void update(float dt);
    void render();
    int zoom() const;
    int ui_scale() const;
    void make_ui_textures();
    void interface_step(int out_w, int out_h, int scale);
    void ask_quit();
    void draw_pointer(int scale);
    void startup_step(int out_w, int out_h);
    // The debug report tool's save: the window as drawn so far, the state
    // (debug_state) and the note, in a new folder under debug-reports/.
    void save_debug_report();
    std::string debug_state() const;
    void open_town_map();
    void town_step(int out_w, int out_h);
    void charts_step(int out_w, int out_h);
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
    BuildUi build_ui_;
    Jukebox jukebox_;
    // The town map: while open it takes the window from the view, the
    // interface and the menu bar; the game keeps running behind it.
    TownMap town_;
    bool open_town_ = false; // the T key asked for it this frame
    SDL_Texture *town_tex_ = nullptr;
    // The full-screen charts (F1-F8, the Charts menu, the panel), shown in
    // place of everything else while open, the game running behind them.
    Charts charts_;
    bool chart_space_ = false; // Space pressed for the charts this frame
    SDL_Texture *charts_tex_ = nullptr;
    // The map cell under the pointer this frame (column + row * 128), -1
    // when off the map.
    int map_cell_ = -1;
    // The thing under the pointer as the view was drawn this frame.
    PointedThing pointed_;
    // The start-up screens (intro, menus, the level's area movie), shown
    // until they start a level; skipped with --level.
    Startup startup_;
    bool in_startup_ = false;
    bool level_from_cli_ = false;
    std::vector<StartupKey> startup_keys_; // keys pressed since the last frame
    SDL_Texture *startup_tex_ = nullptr;
    SDL_AudioStream *movie_stream_ = nullptr;
    int movie_playing_ = -1; // the movie movie_stream_ plays
    std::string balance_id_{kDefaultBalanceId};
    bool tutorial_ = false;    // the advisor's tutorial, chosen in the start-up screens
    Event last_event_;         // the command carried out the frame before (the tutorial reads it)
    float frame_seconds_ = 0;  // the last frame's length
    bool escape_ = false;      // Escape pressed this frame
    bool asking_quit_ = false; // the yes/no requester asks whether to quit
    // The commands the yes/no requester sends for yes and for no when it
    // asks before one (othervar.yes_type and no_type); EV_NONE otherwise.
    Event yes_event_;
    Event no_event_;
    SDL_Texture *pointer_ = nullptr;
    int pointer_w_ = 0, pointer_h_ = 0;

    Settings settings_;
    DataSearch search_;
    std::string data_error_;
    FolderPick pick_;
    std::unique_ptr<GameData> data_;

    int level_ = 1;
    std::optional<int> language_; // --language: LANG-n.DAT instead of HOSPITAL.CFG's
    std::unique_ptr<LevelView> map_;

    Surface surface_;
    Point camera_;                                         // map pixel position of the view's top-left
    uint16_t scroll_min_ = 0, scroll_max_ = kMapCells - 1; // the level's scroll area, as cells
    float scroll_x_ = 0, scroll_y_ = 0;                    // sub-pixel scroll carried between frames
    bool dragging_ = false;
    int zoom_ = 0;        // the view's scale: 0 automatic, else the integer scale
    bool shadows_ = true; // the game's shadows option
    Speed speed_ = Speed::Normal;
    double turn_clock_ = 0; // turns due but not yet run
    ToolsState tools_;
    // The game's menu bar, drawn into its own layer across the top of the
    // window at the UI scale.
    MenuBar menu_;
    UiLayer menu_layer_;
    SDL_Texture *menu_tex_ = nullptr;
    // The mouse buttons as the game's interface sees them; the pressed and
    // released flags last one frame.
    struct {
        bool left = false, right = false;
        bool left_pressed = false, right_pressed = false;
        bool left_released = false, right_released = false;
    } buttons_;
    bool running_ = true;
    bool imgui_ready_ = false;
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

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return false;
    }
    float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    if (scale <= 0)
        scale = 1.0f;
    window_ = SDL_CreateWindow("Theme Hospital Enhanced", int(1280 * scale), int(720 * scale),
                               SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!window_) {
        std::fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return false;
    }
    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (!renderer_) {
        std::fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        return false;
    }
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

    settings_.load();
    search_ = find_data_dir(data_arg, settings_);
    if (search_.found)
        try_load_data(*search_.found);
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
    if (!menu_.load(dir, data_->text(), &error))
        std::fprintf(stderr, "%s\n", error.c_str());
    if (!requester_art_.load(dir, &error))
        std::fprintf(stderr, "%s\n", error.c_str());
    if (!hire_art_.load(dir, &error))
        std::fprintf(stderr, "%s\n", error.c_str());
    if (!town_.load(dir, data_->text(), &error))
        std::fprintf(stderr, "%s\n", error.c_str());
    if (!load_charts(charts_, dir, data_->text(), data_->palette(), &error))
        std::fprintf(stderr, "%s\n", error.c_str());
    init_object_ui(object_ui_, data_->palette());
    if (!bank_.load(dir, data_->language(), &error))
        std::fprintf(stderr, "%s\n", error.c_str());
    sounds_.set_bank(&bank_);
    if (!music_.load(dir, &error))
        std::fprintf(stderr, "no music: %s\n", error.c_str());
    if (!mixer_.is_open() && !mixer_.open(&error))
        std::fprintf(stderr, "no sound: %s\n", error.c_str());
    make_ui_textures();
    data_error_.clear();
    // The start-up screens choose the level, unless --level did.
    if (!level_from_cli_ && startup_.load(dir, data_->text(), data_->language(), &error))
        in_startup_ = true;
    else
        load_level(level_);
    return true;
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
    level_ = number;
    map_ = std::move(map);
    start_advisor(map_->game.advisor, tutorial_);
    start_hospital_settings(map_->game.hospital, map_->game.settings());
    setup_insurances(map_->game.world, uint32_t(SDL_GetTicks()));
    charts_.hospital_names[0] = startup_.player_name();
    charts_.open = false;
    town_.close();
    panel_state_.shown_balance = map_->game.balance;
    sounds_.safeseed = kLevelStartSeed;
    // A level starts on a song picked by the clock, as the game does.
    music_.start_level(uint32_t(SDL_GetTicks()));
    sounds_.init_ambients();
    // Centre the view on the player's starting cell, as load_level does.
    int start = level->camera_start[0];
    scroll_min_ = level->scroll_min;
    scroll_max_ = level->scroll_max;
    Point centre = cell_to_world(start % kMapSize, start / kMapSize);
    camera_ = {centre.x - surface_.width() / 2, centre.y - surface_.height() / 2};
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

    // Escape: with a modal requester up, the requester's; else it drops
    // what is being hired, bought or placed (setup_abort_all_requesters).
    bool escape = escape_;
    escape_ = false;
    if (escape && game && !interface_.modal()) {
        if (hire_.placing() || interface_.requester() == Requester::HireStaff ||
            interface_.requester() == Requester::Jukebox) {
            game->event = hire_staff_cancel(hire_);
            escape = false;
        } else if (object_escape(*game)) {
            escape = false;
        }
    }
    if (game) {
        sync_object_requesters(interface_, *game);
        sync_build_requester(interface_, *game);
    }
    InterfaceEvents events = interface_.frame(mouse, escape);
    if (game && interface_.requester() == Requester::Jukebox) {
        JukeboxResult jr = jukebox_frame(jukebox_, interface_, events, requester_art_, music_, mouse, game->paused);
        for (uint8_t sfx : jr.sounds)
            sounds_.play_sound(kSourceGeneral, sfx);
        if (jr.close)
            game->event = hire_staff_cancel(hire_);
    }
    if (game) {
        const bool *keys = SDL_GetKeyboardState(nullptr);
        bool shift = keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];
        bool ctrl = keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL];
        object_requesters_input(interface_, object_ui_, *game, events, mouse, shift, ctrl);
        build_requesters_input(interface_, build_ui_, *game, events, mouse);
        if (Event e = hire_staff_click(hire_, events, *game); e.type)
            game->event = e;
    }
    if (events.yes_no != YesNoAnswer::None)
        sounds_.play_sound(kSourceGeneral, events.yes_no == YesNoAnswer::Yes ? kSfxYesX : kSfxNo4);
    if (events.yes_no == YesNoAnswer::Yes && asking_quit_)
        running_ = false;
    if (game && events.yes_no != YesNoAnswer::None) {
        Event e = events.yes_no == YesNoAnswer::Yes ? yes_event_ : no_event_;
        if (e.type)
            game->event = e;
    }
    if (events.yes_no != YesNoAnswer::None) {
        asking_quit_ = false;
        yes_event_ = no_event_ = {};
    }
    bool paused = game && game->paused;
    bool input = !events.mouse_on_window && !interface_.modal() && !menu_.active;
    PanelAction action = panel_frame(panel_state_, mouse, *data_, paused, input);
    if (game) {
        bool placing = game->world.placing.placing || hire_.placing();
        // The other panel buttons lead to requesters and screens not ported yet.
        bool refused = action == PanelAction::Refused;
        if (action == PanelAction::FurnishCorridor) {
            refused = hire_.placing() || build_tool_busy(*game);
            if (!refused)
                furnish_corridor(*game);
        } else if (action == PanelAction::HireStaff) {
            refused = placing || build_tool_busy(*game);
            if (!refused)
                game->event = {EV_OPEN_HIRE_STAFF};
        } else if (action == PanelAction::BuildRoom) {
            refused = hire_.placing() || build_tool_busy(*game);
            if (!refused)
                open_build_room(*game);
        } else if (action == PanelAction::TownMap) {
            open_town_map();
        } else if (int chart = chart_for_panel_action(action); chart >= 0) {
            Event e = open_chart_command(chart, *game, &refused);
            if (!refused)
                game->event = e;
        }
        if (action != PanelAction::None)
            sounds_.play_sound(kSourceGeneral, refused ? kSfxWrong2 : kSfxSelectX);

        // The map: what is being placed follows the pointer.
        bool on_map = input && !paused && mouse.y < panel_top(*data_);
        int cell = on_map ? map_cell_ : -1;
        placement_mouse(object_ui_, *game, cell, mouse, on_map);
        build_mouse(build_ui_, *game, cell, mouse, on_map);
        if (hire_.placing() && on_map && cell >= 0) {
            update_staff_cursor(hire_, game->world, cell);
            if (mouse.left_pressed) {
                if (Event e = place_staff_click(hire_, game->world, cell); e.type)
                    game->event = e;
            } else if (mouse.right_pressed) {
                game->event = hire_staff_cancel(hire_);
            }
        } else if (mouse.right_pressed && on_map && !placing && !build_tool_busy(*game) &&
                   interface_.window_count() == 0 && game->world.building.build.room == 0 &&
                   !advisor_hit(game->advisor, mouse.x, mouse.y)) {
            // process_right_button in the normal mouse mode, no requester
            // open and no room being edited: picks up the player's member of
            // staff under the pointer.
            if (Event e = pick_up_staff_click(game->world, pointed_.thing); e.type) {
                game->event = e;
                sounds_.play_sound(kSourceGeneral, kSfxPickup);
            }
        }

        // The advisor: a right click on him or his bubble dismisses him; then
        // his frame, which reads the command about to be carried out.
        if (mouse.right_pressed && input && mouse.y < panel_top(*data_) && advisor_hit(game->advisor, mouse.x, mouse.y))
            game->advisor.dismiss = true;
        AdvisorInputs in;
        in.game_turn = game->turn_count;
        in.paused = paused;
        in.requester_open = interface_.requester() != Requester::None && interface_.requester() != Requester::EditRoom;
        in.level = game->level_number();
        in.month = game->date.month;
        in.open = game->open;
        in.balance = game->balance;
        in.reputation = game->reputation();
        in.finance = &game->finance;
        in.settings = &game->settings();
        in.world = &game->world;
        in.text = &data_->text();
        in.pending = game->event;
        in.last = last_event_;
        in.staff_place_ok = hire_.place_ok;
        if (game->world.placing.placing)
            in.object_place_ok = game->world.placing.ok;
        advisor_frame(game->advisor, in, game->anims(), sounds_.safeseed);
        last_event_ = game->event;

        // The command, as process_turn: the interface's part, then the game's.
        run_command(*game);
    }

    ui_.clear();
    PanelFigures figures;
    if (map_) {
        const Game &g = map_->game;
        figures = {g.balance, g.date.day, g.date.month, g.reputation(), g.opening_countdown, g.open_countdown};
        // The watch is not shown while the tutorial holds the countdown.
        if (g.advisor.flags & kAdvisorTutorial)
            figures.opening_countdown = 0;
    }
    draw_panel_ui(ui_, panel_state_, map_ ? &figures : nullptr, *data_);
    if (game) {
        draw_advisor(ui_, game->advisor, *data_, data_->font(0x1d));
        advance_balance_roll(panel_state_, game->balance);
    }
    draw_interface(ui_, interface_, requester_art_);
    if (interface_.requester() == Requester::Jukebox)
        draw_jukebox(ui_, interface_, requester_art_, music_);
    if (game) {
        draw_object_requesters(ui_, interface_, object_ui_, *game, requester_art_, *data_);
        draw_build_requester(ui_, interface_, build_ui_, *game, requester_art_, *data_);
        draw_hire_staff(ui_, hire_, interface_, requester_art_, hire_art_, game->world, data_->text());
    }
    std::vector<uint32_t> pixels = ui_.to_argb(data_->palette());
    SDL_UpdateTexture(ui_tex_, nullptr, pixels.data(), kUiWidth * 4);
    SDL_FRect dst = {float(ox), float(oy), float(kUiWidth * scale), float(kUiHeight * scale)};
    SDL_RenderTexture(renderer_, ui_tex_, nullptr, &dst);
}

// Asks "really quit?" with the yes/no requester, as the game does.
void App::ask_quit()
{
    if (interface_.modal() || !data_)
        return;
    interface_.open_yes_no(data_->text().at(TextSection::YesNo, 0));
    asking_quit_ = true;
}

// The game's own pointer, in place of the system one over the game; its
// hotspot is its top-left corner.
void App::draw_pointer(int scale)
{
    float mx, my;
    SDL_GetMouseState(&mx, &my);
    float density = SDL_GetWindowPixelDensity(window_);
    if (pointer_ && SDL_GetMouseFocus() == window_ && !ImGui::GetIO().WantCaptureMouse) {
        SDL_HideCursor();
        SDL_FRect p = {mx * density, my * density, float(pointer_w_ * scale), float(pointer_h_ * scale)};
        SDL_RenderTexture(renderer_, pointer_, nullptr, &p);
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
    bool escape = escape_;
    escape_ = false;
    auto go_to = [this](int column, int row) {
        Point centre = cell_to_world(column, row);
        if (beyond_scroll_limit(centre, scroll_min_, scroll_max_))
            return false;
        camera_ = {centre.x - surface_.width() / 2, centre.y - surface_.height() / 2};
        return true;
    };
    TownMapResult r = town_.frame(mouse, escape, game, game.land, panel_state_, startup_.player_name(), go_to);
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
    hire_staff_event(hire_, interface_, game.world, game.event);
    jukebox_event(jukebox_, interface_, game.event);
    chart_command(charts_, game.event);
    // EV_SACK_STAFF and EV_CANCEL_SACK close the yes/no requester that asked.
    if (game.event.type == EV_SACK_STAFF || game.event.type == EV_CANCEL_SACK)
        interface_.close_yes_no(true);
    if (game.event.type == EV_SACK_STAFF)
        sounds_.play_sound(kSourceGeneral, kSfxSack);
    game.process_turn();
    hire_staff_after_turn(hire_, game.world);
    for (uint8_t sfx : game.world.building.sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    game.world.building.sounds.clear();
}

// One frame of the open chart, in place of the view and the interface,
// drawn aspect-fit in the window; then the turn's command.
void App::charts_step(int out_w, int out_h)
{
    Game &game = map_->game;
    SDL_FRect at = fit_rect(charts_.surface, out_w, out_h);
    UiMouse mouse = fitted_mouse(at, charts_.surface);
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

// One frame of the start-up screens: drawn aspect-fit in the window, with
// the movies' sound on a stream of its own. Starts the level they choose.
void App::startup_step(int out_w, int out_h)
{
    const Surface &src = startup_.surface();
    SDL_FRect at = fit_rect(src, out_w, out_h);
    UiMouse mouse = fitted_mouse(at, src);
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

    if (r.kind == StartupResult::Kind::Quit) {
        running_ = false;
    } else if (r.kind == StartupResult::Kind::StartLevel) {
        if (movie_stream_)
            SDL_DestroyAudioStream(movie_stream_);
        movie_stream_ = nullptr;
        in_startup_ = false;
        balance_id_ = r.balance_id;
        tutorial_ = r.tutorial;
        load_level(r.level);
    }
}

void App::save_debug_report()
{
    namespace fs = std::filesystem;
    char stamp[32];
    std::time_t now = std::time(nullptr);
    std::strftime(stamp, sizeof stamp, "%Y%m%d-%H%M%S", std::localtime(&now));
    fs::path dir = fs::path("debug-reports") / stamp;
    for (int n = 2; fs::exists(dir); n++)
        dir = fs::path("debug-reports") / std::format("{}-{}", stamp, n);
    std::error_code ec;
    fs::create_directories(dir, ec);
    if (ec) {
        tools_.report.last = std::format("Could not create {}: {}", dir.string(), ec.message());
        return;
    }
    SDL_Surface *shot = SDL_RenderReadPixels(renderer_, nullptr);
    bool shot_ok = shot && SDL_SavePNG(shot, (dir / "screenshot.png").string().c_str());
    std::string shot_error = shot_ok ? "" : SDL_GetError();
    if (shot)
        SDL_DestroySurface(shot);
    std::ofstream(dir / "note.txt") << tools_.report.note.data() << '\n';
    std::ofstream(dir / "state.txt") << debug_state();
    std::string snapshot_error;
    if (map_ && !in_startup_) {
        std::vector<uint8_t> snapshot;
        if (save_snapshot(map_->game, snapshot))
            std::ofstream(dir / "snapshot.bin", std::ios::binary)
                .write(reinterpret_cast<const char *>(snapshot.data()), std::streamsize(snapshot.size()));
        else
            snapshot_error = ", without the snapshot";
        std::ofstream commands(dir / "commands.txt");
        for (const std::string &line : map_->game.command_log)
            commands << line << '\n';
    }
    tools_.report.last = std::format("Saved to {}", fs::absolute(dir).string());
    tools_.report.last += snapshot_error;
    if (!shot_ok)
        tools_.report.last += ", without the screenshot: " + shot_error;
    std::fprintf(stderr, "debug report: %s\n", tools_.report.last.c_str());
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
    const char *screen = in_startup_       ? "start-up screens"
                         : !map_           ? "no level"
                         : charts_.open    ? "chart"
                         : town_.is_open() ? "town map"
                                           : "hospital";
    s += std::format("screen: {}", screen);
    if (charts_.open)
        s += std::format(" {}", charts_.chart);
    s += std::format("\nmouse: {},{} (window points), map cell {}", mx, my, map_cell_);
    if (map_cell_ >= 0)
        s += std::format(" (column {}, row {})", map_cell_ % kMapSize, map_cell_ / kMapSize);
    s += std::format("\ninterface: requester {}, modal {}, menu open {}\n", int(interface_.requester()),
                     interface_.modal(), menu_.active);
    if (!map_ || in_startup_)
        return s;
    const Game &g = map_->game;
    s += std::format("level: {} ({}), tutorial {}\n", level_, balance_id_, tutorial_);
    s += std::format("camera: {},{}\n", camera_.x, camera_.y);
    s += std::format("speed: {} turns/s, paused {}\n", int(speed_), g.paused);
    s += std::format("route planner: {}\n", g.world.path_finder == PathFinder::AStar ? "A*" : "original");
    s += std::format("placing: object {}, staff {}, build mouse {}\n", g.world.placing.placing, hire_.placing(),
                     int(g.world.building.mouse));
    s += std::format("date: year {} month {} day {}, turn {}\n", g.date.year, g.date.month, g.date.day, g.turn_count);
    s += std::format("open: {}, opening countdown {}\n", g.open, g.opening_countdown);
    s += std::format("balance: {}, reputation {}\n", g.balance, g.arrivals.hospitals[0].hospital_reputation);
    s += std::format("last command: type={} value={} flags={} arg={}\n", last_event_.type, last_event_.value,
                     last_event_.flags, last_event_.arg);
    s += "\n";
    s += dump_game(g, map_cell_);
    return s;
}

void App::make_ui_textures()
{
    if (pointer_)
        SDL_DestroyTexture(pointer_);
    if (!ui_tex_) {
        ui_.resize(kUiWidth, kUiHeight);
        ui_tex_ =
            SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, kUiWidth, kUiHeight);
        SDL_SetTextureScaleMode(ui_tex_, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(ui_tex_, SDL_BLENDMODE_BLEND);
    }
    Sprite s = data_->pointers().at(kPointerDefault);
    pointer_w_ = s.width;
    pointer_h_ = s.height;
    std::vector<uint32_t> pointer(size_t(pointer_w_) * size_t(pointer_h_));
    sprite_to_argb(s, data_->palette(), pointer.data(), pointer_w_ * 4);
    pointer_ = argb_texture(renderer_, pointer, pointer_w_, pointer_h_);
}

int App::zoom() const
{
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(window_, &w, &h);
    if (zoom_ > 0)
        return zoom_;
    return std::max(1, h / kMinViewHeight);
}

// The path finding tool's clicks on the view (window coordinates): left
// places a walker, selects the person on the cell or sends the selected one
// there; right deselects.
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
    switch (event.sound) {
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
    switch (event.action) {
    case MenuAction::Speed:
        speed_ = event.value <= 10   ? Speed::Slowest
                 : event.value <= 20 ? Speed::Slower
                 : event.value <= 30 ? Speed::Normal
                 : event.value <= 40 ? Speed::Max
                                     : Speed::AndThenSome;
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
        if (map_) {
            bool refused = false;
            Event e = open_chart_command(event.value, map_->game, &refused);
            if (refused)
                sounds_.play_sound(kSourceGeneral, kSfxWrong2);
            else
                map_->game.event = e;
        }
        break;
    case MenuAction::Jukebox:
        if (map_)
            map_->game.event = {EV_OPEN_JUKEBOX};
        break;
    default:
        break;
    }
}

void App::nav_click(uint8_t button, float x, float y)
{
    NavTool &nav = tools_.nav;
    Game &game = map_->game;
    if (button == SDL_BUTTON_RIGHT) {
        nav.selected = 0;
        return;
    }
    if (button != SDL_BUTTON_LEFT)
        return;
    float density = SDL_GetWindowPixelDensity(window_);
    int z = zoom();
    Point cell = world_to_cell({camera_.x + int(x * density) / z, camera_.y + int(y * density) / z});
    if (cell.x < 0 || cell.x >= kMapSize || cell.y < 0 || cell.y >= kMapSize)
        return;
    int tile = cell.x + cell.y * kMapSize;
    if (nav.placing) {
        nav.placing = false;
        nav.selected = game.add_walker(tile);
    } else if (uint16_t id = person_at(game.world, cell.x, cell.y)) {
        nav.selected = id;
    } else if (nav.selected) {
        game.send_walker(nav.selected, tile);
    }
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
        if (io.WantCaptureKeyboard)
            break;
        if (e.key.key == SDLK_F12 && data_ && in_startup_) {
            tools_.open = !tools_.open;
            break;
        }
        if (in_startup_) {
            // The start-up screens read keys as lbInkey.
            StartupKey k;
            SDL_Keycode key = e.key.key;
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
            startup_keys_.push_back(k);
            break;
        }
        if (e.key.key == SDLK_F12) {
            tools_.open = !tools_.open;
        } else if (!data_) {
            if (e.key.key == SDLK_ESCAPE)
                running_ = false;
        } else if (e.key.key == SDLK_ESCAPE && !(e.key.mod & SDL_KMOD_SHIFT)) {
            // Closes the front requester (the game's Escape).
            escape_ = true;
        } else if (interface_.modal() || town_.is_open()) {
            // A modal requester takes the keys; the town map only Escape.
        } else if (e.key.key >= SDLK_F1 && e.key.key <= SDLK_F8 && map_) {
            // The charts, from the game or another chart; not while paused.
            if (!map_->game.paused) {
                bool refused = false;
                Event ev = open_chart_command(int(e.key.key - SDLK_F1), map_->game, &refused);
                if (refused)
                    sounds_.play_sound(kSourceGeneral, kSfxWrong2);
                else
                    map_->game.event = ev;
            }
        } else if (charts_.open) {
            // The charts take Escape, Space and the arrows only.
            chart_space_ = chart_space_ || e.key.key == SDLK_SPACE;
        } else if ((e.key.key == SDLK_ESCAPE && (e.key.mod & SDL_KMOD_SHIFT)) ||
                   ((e.key.key == SDLK_Q || e.key.key == SDLK_X) &&
                    (e.key.mod & (SDL_KMOD_CTRL | SDL_KMOD_ALT | SDL_KMOD_SHIFT)))) {
            // The game's quit keys ask first.
            ask_quit();
        } else if (e.key.key == SDLK_P && map_) {
            map_->game.paused = !map_->game.paused;
        } else if (e.key.key == SDLK_T && map_) {
            open_town_ = true;
        } else if (e.key.key == SDLK_J && !(e.key.mod & SDL_KMOD_SHIFT) && map_ && !map_->game.paused) {
            map_->game.event = {EV_OPEN_JUKEBOX};
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (io.WantCaptureMouse)
            break;
        if (e.button.button == SDL_BUTTON_LEFT)
            buttons_.left = buttons_.left_pressed = true;
        if (e.button.button == SDL_BUTTON_RIGHT)
            buttons_.right = buttons_.right_pressed = true;
        // An open menu, a modal requester or the town map takes the clicks.
        if (menu_.active || interface_.modal() || town_.is_open() || charts_.open)
            break;
        // While something is being placed, its clicks are the interface's.
        if (map_ && (map_->game.world.placing.placing || hire_.placing()))
            break;
        if (e.button.button == SDL_BUTTON_MIDDLE || e.button.button == SDL_BUTTON_RIGHT)
            dragging_ = true;
        if (tools_.open && map_)
            nav_click(e.button.button, e.button.x, e.button.y);
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (e.button.button == SDL_BUTTON_LEFT && buttons_.left) {
            buttons_.left = false;
            buttons_.left_released = true;
        }
        if (e.button.button == SDL_BUTTON_RIGHT && buttons_.right) {
            buttons_.right = false;
            buttons_.right_released = true;
        }
        if (e.button.button == SDL_BUTTON_MIDDLE || e.button.button == SDL_BUTTON_RIGHT)
            dragging_ = false;
        break;
    case SDL_EVENT_MOUSE_MOTION:
        if (dragging_ && data_) {
            float density = SDL_GetWindowPixelDensity(window_);
            scroll_x_ -= e.motion.xrel * density / float(zoom());
            scroll_y_ -= e.motion.yrel * density / float(zoom());
        }
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        if (!io.WantCaptureMouse && data_) {
            // render() keeps the view centred when the surface changes size.
            zoom_ = std::clamp(zoom() + (e.wheel.y > 0 ? 1 : e.wheel.y < 0 ? -1 : 0), 1, 4);
        }
        break;
    default:
        break;
    }
}

void App::update(float dt)
{
    if (!data_ || in_startup_)
        return;
    if (map_ && !map_->game.paused) {
        // At most a few frames' worth of turns after a stall.
        turn_clock_ = std::min(turn_clock_ + dt * double(int(speed_)), 8.0);
        for (; turn_clock_ >= 1; turn_clock_ -= 1)
            map_->game.turn();
    }
    if (town_.is_open() || charts_.open)
        return;
    const ImGuiIO &io = ImGui::GetIO();
    if (!io.WantCaptureKeyboard) {
        const bool *keys = SDL_GetKeyboardState(nullptr);
        float step = kScrollSpeed * dt / float(zoom());
        if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A])
            scroll_x_ -= step;
        if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D])
            scroll_x_ += step;
        if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W])
            scroll_y_ -= step;
        if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S])
            scroll_y_ += step;
    }
    if (!io.WantCaptureMouse && !dragging_ && SDL_GetMouseFocus() == window_) {
        float mx, my;
        SDL_GetMouseState(&mx, &my);
        int w, h;
        SDL_GetWindowSize(window_, &w, &h);
        float step = kScrollSpeed * dt / float(zoom());
        if (mx < kEdgeScrollMargin)
            scroll_x_ -= step;
        else if (mx >= w - kEdgeScrollMargin)
            scroll_x_ += step;
        if (my < kEdgeScrollMargin)
            scroll_y_ -= step;
        else if (my >= h - kEdgeScrollMargin)
            scroll_y_ += step;
    }
    int dx = int(scroll_x_), dy = int(scroll_y_);
    scroll_x_ -= float(dx);
    scroll_y_ -= float(dy);
    // The game refuses a scroll that would take the view's centre out of the
    // level's scroll area. Trying each axis on its own lets the view slide
    // along the area's diagonal edges, where the game stops dead.
    Point centre = {camera_.x + surface_.width() / 2, camera_.y + surface_.height() / 2};
    if (!beyond_scroll_limit({centre.x + dx, centre.y}, scroll_min_, scroll_max_))
        camera_.x += dx;
    if (!beyond_scroll_limit({camera_.x + surface_.width() / 2, centre.y + dy}, scroll_min_, scroll_max_))
        camera_.y += dy;
}

void App::render()
{
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
    } else if (in_startup_) {
        startup_step(out_w, out_h);
        mixer_.update(sounds_, &music_);
        draw_pointer(ui_scale());
    } else if (map_ && charts_.open) {
        charts_step(out_w, out_h);
        mixer_.update(sounds_, &music_);
        draw_pointer(ui_scale());
    } else if (map_ && (town_.is_open() || std::exchange(open_town_, false))) {
        if (!town_.is_open())
            open_town_map();
        town_step(out_w, out_h);
        mixer_.update(sounds_, &music_);
        draw_pointer(ui_scale());
    } else {
        int z = zoom();
        int vw = std::max(out_w / z, 1), vh = std::max(out_h / z, 1);
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

        float cmx, cmy;
        SDL_GetMouseState(&cmx, &cmy);
        float cdensity = SDL_GetWindowPixelDensity(window_);
        pointed_.at = {int(cmx * cdensity) / z, int(cmy * cdensity) / z};
        frame_sounds_.clear();
        draw_view(surface_, *data_, map_.get(), camera_, shadows_, &frame_sounds_, &pointed_);
        if (map_) {
            // The sounds of what was drawn, positioned on the view.
            sounds_.frames_drawn(frame_sounds_, vw);
            sounds_.manage_sound(map_->game.world, camera_, vw, vh);
            sounds_.manage_background_sounds(map_->game.paused);
            // The music moves on to the next song, and the announcer speaks, as
            // the game does each frame; only the announcer while a modal
            // requester is up.
            if (!interface_.modal())
                music_.play_next(map_->game.paused);
            sounds_.do_ambients(map_->game.paused, false, has_receptionist(map_->game.world));
        }
        mixer_.update(sounds_, &music_);

        // What is being placed, drawn over the view as the game does.
        Point under = world_to_cell({camera_.x + pointed_.at.x, camera_.y + pointed_.at.y});
        bool inside = under.x >= 0 && under.x < kMapSize && under.y >= 0 && under.y < kMapSize;
        map_cell_ = inside ? under.x + under.y * kMapSize : -1;
        if (map_) {
            Rect view{0, 0, vw, vh};
            draw_placement(surface_, view, camera_, map_->game.world.placing, object_ui_, *data_);
            draw_build_overlay(surface_, view, camera_, map_->game, build_ui_, object_ui_, *data_);
            if (hire_.placing() && map_cell_ >= 0)
                draw_held_staff(surface_, view, camera_, hire_, map_->game.world, map_cell_, *data_, map_->scratch);
        }

        void *pixels;
        int pitch;
        if (screen_ && SDL_LockTexture(screen_, nullptr, &pixels, &pitch)) {
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

        // The interface and the pointer keep their size whatever the view's
        // zoom.
        int s = ui_scale();
        interface_step(out_w, out_h, s);
        menu_step(out_w, s);

        draw_pointer(s);

        if (tools_.open && map_)
            draw_nav_overlay(tools_.nav, map_->game.world, camera_,
                             float(z) / ImGui::GetIO().DisplayFramebufferScale.x);
    }

    if (data_)
        draw_tools(tools_, map_ && !in_startup_ ? &map_->game.world : nullptr);
    // Read back before ImGui draws, so the screenshot shows the game alone.
    if (std::exchange(tools_.report.save, false))
        save_debug_report();

    ImGui::Render();
    const ImGuiIO &io = ImGui::GetIO();
    SDL_SetRenderScale(renderer_, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer_);
    SDL_RenderPresent(renderer_);
    buttons_.left_pressed = buttons_.right_pressed = false;
    buttons_.left_released = buttons_.right_released = false;
}

void App::run()
{
    uint64_t last = SDL_GetTicksNS();
    while (running_) {
        SDL_Event e;
        while (SDL_PollEvent(&e))
            handle_event(e);

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
        frame_seconds_ = std::min(dt, 0.1f);
        update(frame_seconds_);
        render();
    }
}

void App::shutdown()
{
    mixer_.close();
    if (movie_stream_)
        SDL_DestroyAudioStream(movie_stream_);
    if (startup_tex_)
        SDL_DestroyTexture(startup_tex_);
    if (town_tex_)
        SDL_DestroyTexture(town_tex_);
    if (charts_tex_)
        SDL_DestroyTexture(charts_tex_);
    if (ui_tex_)
        SDL_DestroyTexture(ui_tex_);
    if (pointer_)
        SDL_DestroyTexture(pointer_);
    if (screen_)
        SDL_DestroyTexture(screen_);
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
    return 0;
}

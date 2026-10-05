// Theme Hospital Enhanced: draws a level's map from the game's own data at
// any window size, with the bottom panel centred at native size and ImGui
// developer tools (F1).

#include "app/datadir.h"
#include "app/settings.h"
#include "app/ui/tools.h"
#include "core/data/gamedata.h"
#include "core/render/blit.h"
#include "core/render/iso.h"
#include "core/render/panel.h"
#include "core/render/shadows.h"
#include "core/render/surface.h"
#include "core/render/view.h"
#include "core/ui/layer.h"
#include "core/ui/menubar.h"
#include "core/ui/windows.h"
#include "core/world/game.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <algorithm>
#include <cstdio>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

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

// Starts level `number` with its settings (the default FULL set), or
// returns nothing and says why.
std::unique_ptr<LevelView> begin_level(const GameData &data, const Level &level, int number, std::string *error)
{
    Balance settings;
    if (!load_balance(data.dir(), number, kDefaultBalanceId, settings, error))
        return nullptr;
    auto view = std::make_unique<LevelView>();
    view->game.start(level, number, settings, data.anims(), data.person_complexes());
    // The shadowed blocks are made from the map as recreate_map leaves it,
    // before add_static_shadows.
    view->shadows = std::make_unique<ShadowSprites>(data.blocks(), data.ghost(), view->game.world.map.data());
    add_static_shadows(view->game.world.map);
    return view;
}

// Draws the hospital view into `surface`.
void draw_view(Surface &surface, const GameData &data, LevelView *level, Point camera, bool shadows)
{
    surface.fill(0);
    if (level) {
        if (level->scratch.width() != surface.width() || level->scratch.height() != surface.height()) {
            level->scratch.resize(surface.width(), surface.height());
            level->scratch.fill(kMaskColour);
        }
        draw_world(surface, {0, 0, surface.width(), surface.height()}, camera, level->game.world, data,
                   shadows ? level->shadows.get() : nullptr, level->scratch);
    }
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
    auto map = begin_level(data, *level, level_number, &error);
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
    PanelFigures figures{g.balance, g.date.day, g.date.month, g.reputation, g.opening_countdown, g.open_countdown};
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
    bool escape_ = false;      // Escape pressed this frame
    bool asking_quit_ = false; // the yes/no requester asks whether to quit
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
        else if (arg == "--level" && i + 1 < argc)
            level_ = std::atoi(argv[++i]);
        else if (arg == "--language" && i + 1 < argc)
            language_ = std::atoi(argv[++i]);
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
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
    make_ui_textures();
    data_error_.clear();
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
    auto map = begin_level(*data_, *level, number, &error);
    if (!map) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return;
    }
    level_ = number;
    map_ = std::move(map);
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
    InterfaceEvents events = interface_.frame(mouse, escape_);
    escape_ = false;
    if (events.yes_no == YesNoAnswer::Yes && asking_quit_)
        running_ = false;
    if (events.yes_no != YesNoAnswer::None)
        asking_quit_ = false;
    bool paused = map_ && map_->game.paused;
    bool input = !events.mouse_on_window && !interface_.modal() && !menu_.active;
    // The panel's buttons lead to requesters and screens not ported yet.
    panel_frame(panel_state_, mouse, *data_, paused, input);

    ui_.clear();
    PanelFigures figures;
    if (map_) {
        const Game &g = map_->game;
        figures = {g.balance, g.date.day, g.date.month, g.reputation, g.opening_countdown, g.open_countdown};
    }
    draw_panel_ui(ui_, panel_state_, map_ ? &figures : nullptr, *data_);
    draw_interface(ui_, interface_, requester_art_);
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
    context.speed = speed_ == Speed::Slowest  ? 0
                    : speed_ == Speed::Slower ? 1
                    : speed_ == Speed::Normal ? 2
                    : speed_ == Speed::Max    ? 3
                                              : 4;
    context.paused = map_ && map_->game.paused;
    context.disabled = interface_.modal();
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
    case MenuAction::Quit:
        ask_quit();
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
        if (e.key.key == SDLK_F1) {
            tools_.open = !tools_.open;
        } else if (!data_) {
            if (e.key.key == SDLK_ESCAPE)
                running_ = false;
        } else if (e.key.key == SDLK_ESCAPE && !(e.key.mod & SDL_KMOD_SHIFT)) {
            // Closes the front requester (the game's Escape).
            escape_ = true;
        } else if (interface_.modal()) {
            // A modal requester takes the keys.
        } else if ((e.key.key == SDLK_ESCAPE && (e.key.mod & SDL_KMOD_SHIFT)) ||
                   ((e.key.key == SDLK_Q || e.key.key == SDLK_X) &&
                    (e.key.mod & (SDL_KMOD_CTRL | SDL_KMOD_ALT | SDL_KMOD_SHIFT)))) {
            // The game's quit keys ask first.
            ask_quit();
        } else if (e.key.key == SDLK_P && map_) {
            map_->game.paused = !map_->game.paused;
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (io.WantCaptureMouse)
            break;
        if (e.button.button == SDL_BUTTON_LEFT)
            buttons_.left = buttons_.left_pressed = true;
        if (e.button.button == SDL_BUTTON_RIGHT)
            buttons_.right = buttons_.right_pressed = true;
        // An open menu or a modal requester takes the clicks.
        if (menu_.active || interface_.modal())
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
    if (!data_)
        return;
    if (map_ && !map_->game.paused) {
        // At most a few frames' worth of turns after a stall.
        turn_clock_ = std::min(turn_clock_ + dt * double(int(speed_)), 8.0);
        for (; turn_clock_ >= 1; turn_clock_ -= 1)
            map_->game.turn();
    }
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

        draw_view(surface_, *data_, map_.get(), camera_, shadows_);

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

        // The game's own pointer replaces the system one over the view; its
        // hotspot is its top-left corner.
        float pmx, pmy;
        SDL_GetMouseState(&pmx, &pmy);
        float pdensity = SDL_GetWindowPixelDensity(window_);
        if (pointer_ && SDL_GetMouseFocus() == window_ && !ImGui::GetIO().WantCaptureMouse) {
            SDL_HideCursor();
            SDL_FRect p = {pmx * pdensity, pmy * pdensity, float(pointer_w_ * s), float(pointer_h_ * s)};
            SDL_RenderTexture(renderer_, pointer_, nullptr, &p);
        } else {
            SDL_ShowCursor();
        }

        draw_tools(tools_, map_ ? &map_->game.world : nullptr);
        if (tools_.open && map_)
            draw_nav_overlay(tools_.nav, map_->game.world, camera_,
                             float(z) / ImGui::GetIO().DisplayFramebufferScale.x);
    }

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
        update(std::min(dt, 0.1f));
        render();
    }
}

void App::shutdown()
{
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

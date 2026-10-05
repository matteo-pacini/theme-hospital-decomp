// Theme Hospital Enhanced: draws a level's map from the game's own data at
// any window size, with the bottom panel centred at native size and ImGui
// developer tools (F1).

#include "app/datadir.h"
#include "app/settings.h"
#include "app/ui/tools.h"
#include "core/data/fs.h"
#include "core/data/gamedata.h"
#include "core/render/iso.h"
#include "core/render/panel.h"
#include "core/render/surface.h"

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

// Draws one frame of the game view: the map, then the panel over it.
void draw_view(Surface &surface, const GameData &data, const std::array<MapCell, kMapCells> *map, Point camera)
{
    surface.fill(0);
    if (map)
        draw_map(surface, {0, 0, surface.width(), surface.height()}, camera, map->data(), data.blocks());
    draw_panel(surface, data.panel());
}

// --screenshot: renders level `level_number` at `width` x `height` with the view
// centred on the player's start, without opening a window, and saves it as
// a BMP. Returns the process exit code.
int screenshot(const std::filesystem::path &data_dir, int level_number, int width, int height, const std::string &out)
{
    GameData data;
    std::string error;
    if (!data.load(data_dir, &error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }
    auto level = data.load_level(level_number, &error);
    if (!level) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }
    auto map = std::make_unique<std::array<MapCell, kMapCells>>(build_start_map(*level));
    int start = level->camera_start[0];
    Point centre = cell_to_world(start % kMapSize, start / kMapSize);
    Surface surface;
    surface.resize(width, height);
    draw_view(surface, data, map.get(), {centre.x - width / 2, centre.y - height / 2});

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
    void update(float dt);
    void render(float frame_ms);
    int zoom() const;

    SDL_Window *window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;
    SDL_Texture *screen_ = nullptr;
    int screen_w_ = 0, screen_h_ = 0;

    Settings settings_;
    DataSearch search_;
    std::string data_error_;
    FolderPick pick_;
    std::unique_ptr<GameData> data_;

    std::vector<int> levels_;
    std::unique_ptr<std::array<MapCell, kMapCells>> map_;
    std::string level_error_;

    Surface surface_;
    Point camera_;                      // map pixel position of the view's top-left
    float scroll_x_ = 0, scroll_y_ = 0; // sub-pixel scroll carried between frames
    bool dragging_ = false;
    ToolsState tools_;
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
            tools_.level = std::atoi(argv[++i]);
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
    if (!data->load(dir, &error)) {
        data_error_ = error;
        return false;
    }
    data_ = std::move(data);
    data_error_.clear();
    levels_.clear();
    for (int n = 0; n < 120; n++)
        if (resolve_ci(dir, "levels/level.l" + std::to_string(n)))
            levels_.push_back(n);
    load_level(tools_.level);
    return true;
}

void App::load_level(int number)
{
    std::string error;
    auto level = data_->load_level(number, &error);
    if (!level) {
        level_error_ = error;
        return;
    }
    level_error_.clear();
    tools_.level = number;
    map_ = std::make_unique<std::array<MapCell, kMapCells>>(build_start_map(*level));
    // Centre the view on the player's starting cell, as load_level does.
    int start = level->camera_start[0];
    Point centre = cell_to_world(start % kMapSize, start / kMapSize);
    camera_ = {centre.x - surface_.width() / 2, centre.y - surface_.height() / 2};
}

int App::zoom() const
{
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(window_, &w, &h);
    if (tools_.zoom > 0)
        return tools_.zoom;
    return std::max(1, h / kMinViewHeight);
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
        if (e.key.key == SDLK_F1)
            tools_.open = !tools_.open;
        else if (e.key.key == SDLK_ESCAPE && !data_)
            running_ = false;
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (!io.WantCaptureMouse && (e.button.button == SDL_BUTTON_MIDDLE || e.button.button == SDL_BUTTON_RIGHT))
            dragging_ = true;
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
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
            tools_.zoom = std::clamp(zoom() + (e.wheel.y > 0 ? 1 : e.wheel.y < 0 ? -1 : 0), 1, 4);
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
    camera_.x += dx;
    camera_.y += dy;
    scroll_x_ -= float(dx);
    scroll_y_ -= float(dy);

    // Keep the view's centre over the map's diamond bounding box.
    int cx = std::clamp(camera_.x + surface_.width() / 2, -kMapSize * kTileWidth / 2, kMapSize * kTileWidth / 2);
    int cy = std::clamp(camera_.y + surface_.height() / 2, 0, kMapSize * kTileHeight);
    camera_ = {cx - surface_.width() / 2, cy - surface_.height() / 2};

    if (tools_.level_requested) {
        tools_.level_requested = false;
        load_level(tools_.level);
    }
}

void App::render(float frame_ms)
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

        draw_view(surface_, *data_, map_.get(), camera_);

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

        float mx, my;
        SDL_GetMouseState(&mx, &my);
        float density = SDL_GetWindowPixelDensity(window_);
        Point cell = world_to_cell({camera_.x + int(mx * density) / z, camera_.y + int(my * density) / z});
        ViewInfo info{out_w, out_h, vw, vh, z, camera_.x, camera_.y, cell.x, cell.y, frame_ms, levels_, level_error_};
        draw_tools(tools_, *data_, info, renderer_);
    }

    ImGui::Render();
    const ImGuiIO &io = ImGui::GetIO();
    SDL_SetRenderScale(renderer_, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer_);
    SDL_RenderPresent(renderer_);
}

void App::run()
{
    uint64_t last = SDL_GetTicksNS();
    float frame_ms = 0;
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
        frame_ms = frame_ms * 0.9f + dt * 1000.0f * 0.1f;
        update(std::min(dt, 0.1f));
        render(frame_ms);
    }
}

void App::shutdown()
{
    if (tools_.preview)
        SDL_DestroyTexture(tools_.preview);
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
    // --screenshot OUT.bmp [--size WxH] [--level N] --data DIR
    std::string shot, data_dir;
    int level = 1, width = 1280, height = 720;
    for (int i = 1; i + 1 < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--screenshot")
            shot = argv[++i];
        else if (arg == "--data")
            data_dir = argv[++i];
        else if (arg == "--level")
            level = std::atoi(argv[++i]);
        else if (arg == "--size")
            std::sscanf(argv[++i], "%dx%d", &width, &height);
    }
    if (!shot.empty()) {
        if (data_dir.empty()) {
            std::fprintf(stderr, "--screenshot needs --data\n");
            return 2;
        }
        return th::screenshot(data_dir, level, width, height, shot);
    }

    th::App app;
    if (app.init(argc, argv))
        app.run();
    app.shutdown();
    return 0;
}

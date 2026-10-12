// Theme Hospital Enhanced: draws a level's map from the game's own data at
// any window size, with the bottom panel centred at native size and ImGui
// developer tools (F12).

#define SDL_MAIN_USE_CALLBACKS 1

#include "app/app.h"
#include "app/app_internal.h"
#include "app/web.h"
#include "core/data/gamedata.h"
#include "core/render/shadows.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstdio>
#include <cstdlib>
#include <optional>
#include <string>

namespace th
{
namespace
{

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

} // namespace
} // namespace th

namespace
{

SDL_AppResult app_result(const th::App &app)
{
    return app.exit_code() ? SDL_APP_FAILURE : SDL_APP_SUCCESS;
}

} // namespace

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
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
            std::exit(2);
        }
        // exit() because the callbacks' results give only 0 and 1.
        std::exit(th::screenshot(data_dir, level, width, height, shot, language));
    }

    th::App *app = new th::App;
    *appstate = app;
    if (!app->init(argc, argv))
        return app_result(*app);
    app->begin();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    static_cast<th::App *>(appstate)->on_event(*event);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    th::App &app = *static_cast<th::App *>(appstate);
    return app.iterate() ? SDL_APP_CONTINUE : app_result(app);
}

void SDL_AppQuit(void *appstate, SDL_AppResult)
{
    th::App *app = static_cast<th::App *>(appstate);
    if (!app)
        return;
    app->shutdown();
    delete app;
    th::web::show_closed();
}

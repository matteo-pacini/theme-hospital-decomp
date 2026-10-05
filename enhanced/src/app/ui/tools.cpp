#include "app/ui/tools.h"

#include "core/data/gamedata.h"
#include "core/render/blit.h"
#include "core/render/frame.h"

#include <SDL3/SDL.h>
#include <imgui.h>

#include <algorithm>
#include <cstring>

namespace th
{

MissingDataAction draw_missing_data_screen(const DataSearch &search, const std::string &error)
{
    MissingDataAction action = MissingDataAction::None;
    const ImGuiViewport *vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::Begin("Theme Hospital data needed", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

    ImGui::TextUnformatted("Theme Hospital data needed");
    ImGui::Separator();
    ImGui::TextWrapped("This program contains no game files. It needs the data from your own copy of Theme Hospital "
                       "(for example GOG's release): the folder that holds DATA, LEVELS and QDATA.");
    ImGui::Spacing();
    if (ImGui::Button("Choose the game folder..."))
        action = MissingDataAction::Choose;
    ImGui::SameLine();
    if (ImGui::Button("Quit"))
        action = MissingDataAction::Quit;
    if (!error.empty()) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.4f, 1.0f), "%s", error.c_str());
    }

    ImGui::Spacing();
    ImGui::TextUnformatted("Places looked at:");
    for (const DataCandidate &c : search.tried) {
        ImGui::BulletText("%s: %s", c.source.c_str(), c.path.string().c_str());
        if (!c.missing.empty()) {
            ImGui::Indent();
            std::string missing = "missing ";
            for (size_t i = 0; i < c.missing.size(); i++)
                missing += (i ? ", " : "") + c.missing[i];
            ImGui::TextDisabled("%s", missing.c_str());
            ImGui::Unindent();
        }
    }
    ImGui::Spacing();
    ImGui::TextDisabled("You can also pass the folder on the command line: --data <folder>");
    ImGui::End();
    return action;
}

namespace
{

// Decodes `sprite` into `state.preview` with transparent runs see-through.
void update_preview(ToolsState &state, const Sprite &sprite, const Palette &palette, SDL_Renderer *renderer)
{
    if (!state.preview) {
        state.preview = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 256);
        if (!state.preview)
            return;
        SDL_SetTextureScaleMode(state.preview, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(state.preview, SDL_BLENDMODE_BLEND);
    }
    void *pixels;
    int pitch;
    if (!SDL_LockTexture(state.preview, nullptr, &pixels, &pitch))
        return;
    for (int y = 0; y < 256; y++)
        std::memset(static_cast<uint8_t *>(pixels) + y * pitch, 0, 256 * 4);
    sprite_to_argb(sprite, palette, static_cast<uint32_t *>(pixels), pitch);
    SDL_UnlockTexture(state.preview);
}

} // namespace

// Draws the browser's current frame, centred low in a 256x256 texture, and
// advances it at `fps`.
static void draw_anim_browser(ToolsState &state, const GameData &data, SDL_Renderer *renderer, float dt)
{
    const AnimData &anims = data.anims();
    int last = std::max(int(anims.starts.size()) - 1, 0);
    if (ImGui::InputInt("Animation", &state.anim))
        state.anim_frame = -1;
    state.anim = std::clamp(state.anim, 0, last);
    ImGui::Checkbox("Play", &state.anim_playing);
    if (anims.starts.empty())
        return;
    const AnimStart &start = anims.starts[size_t(state.anim)];
    if (state.anim_frame < 0 || size_t(state.anim_frame) >= anims.frames.size())
        state.anim_frame = int(start.frame);
    // Objects advance a frame every 2 turns (anim_speed); about 10 a second
    // at normal speed.
    constexpr double kFrameSeconds = 0.1;
    if (state.anim_playing) {
        state.anim_clock += dt;
        while (state.anim_clock >= kFrameSeconds) {
            state.anim_clock -= kFrameSeconds;
            state.anim_frame = anims.frames[size_t(state.anim_frame)].next;
        }
    } else if (ImGui::SameLine(); ImGui::Button("Step")) {
        state.anim_frame = anims.frames[size_t(state.anim_frame)].next;
    }
    const AnimFrame &f = anims.frames[size_t(state.anim_frame)];
    ImGui::Text("start %d: frame %d%s, box %dx%d, sound %d, next %d%s", state.anim, state.anim_frame,
                start.flip ? " (mirrored)" : "", f.width, f.height, f.sound, f.next, (f.flags & 1) ? ", last" : "");

    if (!state.anim_preview) {
        state.anim_preview =
            SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 256);
        if (!state.anim_preview)
            return;
        SDL_SetTextureScaleMode(state.anim_preview, SDL_SCALEMODE_NEAREST);
    }
    Surface surface;
    surface.resize(256, 256);
    surface.fill(0);
    draw_frame(surface, 128, 192, uint32_t(state.anim_frame), start.flip != 0, 0, data);
    void *pixels;
    int pitch;
    if (SDL_LockTexture(state.anim_preview, nullptr, &pixels, &pitch)) {
        const uint8_t *src = surface.data();
        for (int y = 0; y < 256; y++) {
            auto *row = reinterpret_cast<uint32_t *>(static_cast<uint8_t *>(pixels) + y * pitch);
            for (int x = 0; x < 256; x++)
                row[x] = data.palette()[*src++];
        }
        SDL_UnlockTexture(state.anim_preview);
    }
    ImGui::Image(ImTextureRef(ImTextureID(intptr_t(state.anim_preview))), ImVec2(512, 512));
}

void draw_tools(ToolsState &state, const GameData &data, const ViewInfo &info, SDL_Renderer *renderer)
{
    if (!state.open)
        return;
    ImGui::SetNextWindowSize(ImVec2(360, 0), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Tools (F1)", &state.open)) {
        ImGui::End();
        return;
    }

    ImGui::Text("%.2f ms/frame (%.0f fps)", info.frame_ms, info.frame_ms > 0 ? 1000.0f / info.frame_ms : 0.0f);
    ImGui::Text("Window %dx%d px, view %dx%d at %dx", info.window_w, info.window_h, info.view_w, info.view_h,
                info.zoom);
    ImGui::Text("Camera %d,%d  cell %d,%d", info.camera_x, info.camera_y, info.cell_col, info.cell_row);
    ImGui::TextDisabled("Data: %s", data.dir().string().c_str());

    if (ImGui::CollapsingHeader("View", ImGuiTreeNodeFlags_DefaultOpen)) {
        const char *zooms[] = {"Automatic", "1x", "2x", "3x", "4x"};
        ImGui::Combo("Zoom", &state.zoom, zooms, IM_ARRAYSIZE(zooms));
        ImGui::Combo("UI scale", &state.ui_scale, zooms, IM_ARRAYSIZE(zooms));
        ImGui::Checkbox("Shadows", &state.shadows);
        if (!info.levels.empty()) {
            std::string current = "level.l" + std::to_string(state.level);
            if (ImGui::BeginCombo("Level", current.c_str())) {
                for (int n : info.levels) {
                    std::string label = "level.l" + std::to_string(n);
                    if (ImGui::Selectable(label.c_str(), n == state.level)) {
                        state.level = n;
                        state.level_requested = true;
                    }
                }
                ImGui::EndCombo();
            }
        }
        if (!info.level_error.empty())
            ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.4f, 1.0f), "%s", info.level_error.c_str());
        ImGui::TextDisabled("Scroll: arrows/WASD, screen edges, middle or right drag. Zoom: wheel (the view only; the "
                            "panel and pointer follow the UI scale).");
    }

    if (ImGui::CollapsingHeader("Palette")) {
        ImDrawList *dl = ImGui::GetWindowDrawList();
        ImVec2 origin = ImGui::GetCursorScreenPos();
        const float cell = 14.0f;
        for (int i = 0; i < 256; i++) {
            uint32_t c = data.palette()[i];
            ImVec2 a(origin.x + (i % 16) * cell, origin.y + (i / 16) * cell);
            dl->AddRectFilled(a, ImVec2(a.x + cell - 1, a.y + cell - 1),
                              IM_COL32((c >> 16) & 0xff, (c >> 8) & 0xff, c & 0xff, 0xff));
        }
        ImGui::Dummy(ImVec2(16 * cell, 16 * cell));
        ImVec2 mouse = ImGui::GetIO().MousePos;
        int col = int((mouse.x - origin.x) / cell), row = int((mouse.y - origin.y) / cell);
        if (col >= 0 && col < 16 && row >= 0 && row < 16 && ImGui::IsWindowHovered())
            ImGui::SetTooltip("Index %d: #%06X", row * 16 + col, data.palette()[row * 16 + col] & 0xffffff);
    }

    if (ImGui::CollapsingHeader("Animations"))
        draw_anim_browser(state, data, renderer, ImGui::GetIO().DeltaTime);

    if (ImGui::CollapsingHeader("Sprites")) {
        ImGui::RadioButton("Blocks (VBLK-0)", &state.sheet, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Panel (PANEL02V)", &state.sheet, 1);
        const SpriteSheet &sheet = state.sheet == 0 ? data.blocks() : data.panel();
        int last = std::max(int(sheet.size()) - 1, 0);
        ImGui::SliderInt("Sprite", &state.sprite, 0, last);
        state.sprite = std::clamp(state.sprite, 0, last);
        Sprite s = sheet.at(size_t(state.sprite));
        ImGui::Text("%d x %d", s.width, s.height);
        update_preview(state, s, data.palette(), renderer);
        if (state.preview && !s.empty()) {
            float scale = 2.0f;
            ImGui::Image(ImTextureRef(ImTextureID(intptr_t(state.preview))), ImVec2(s.width * scale, s.height * scale),
                         ImVec2(0, 0), ImVec2(s.width / 256.0f, s.height / 256.0f));
        }
    }
    ImGui::End();
}

} // namespace th

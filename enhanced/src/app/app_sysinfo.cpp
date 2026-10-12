// The debug report's system.txt: the build, the machine, the display, the
// renderer and the settings that change what is drawn or heard. Nothing
// personal goes in (no host or user name).

#include "app/app.h"
#include "app/sysinfo.h"
#include "app/web.h"
#include "th_build_info.h"
#include "th_version.h"

#include <cstring>
#include <format>

namespace th
{

namespace
{

std::string sdl_version_text(int version)
{
    return std::format("{}.{}.{}", SDL_VERSIONNUM_MAJOR(version), SDL_VERSIONNUM_MINOR(version),
                       SDL_VERSIONNUM_MICRO(version));
}

std::string text_or_unknown(const char *text)
{
    return text && *text ? text : "unknown";
}

std::string build_section()
{
    std::string s = "[build]\n";
    s += std::format("version: {}\n", kVersion);
    s += std::format("commit: {}\n", kGitCommit);
    s += std::format("type: {}\n", kReleaseBuild ? "release" : "development");
    s += std::format("platform: {}, {}\n", SDL_GetPlatform(), sysinfo::architecture());
    s += std::format("compiler: {}\n", kCompiler);
    s +=
        std::format("SDL: compiled {}, linked {}\n", sdl_version_text(SDL_VERSION), sdl_version_text(SDL_GetVersion()));
    return s;
}

std::string machine_section()
{
    std::string s = "[machine]\n";
    if (web::kEnabled)
        s += std::format("user agent: {}\n", web::user_agent());
    else
        s += std::format("os: {}\n", sysinfo::os_description());
    s += std::format("cpu: {}\n", sysinfo::cpu_model());
    s += std::format("logical cores: {}\n", SDL_GetNumLogicalCPUCores());
    s += std::format("memory: {}\n", sysinfo::format_mib(SDL_GetSystemRAM()));
    return s;
}

std::string display_section(SDL_Window *window)
{
    std::string s = "[display]\n";
    s += std::format("video driver: {}\n", text_or_unknown(SDL_GetCurrentVideoDriver()));
    const SDL_DisplayID shown = SDL_GetDisplayForWindow(window);
    int count = 0;
    SDL_DisplayID *displays = SDL_GetDisplays(&count);
    for (int i = 0; displays && i < count; i++) {
        const SDL_DisplayMode *mode = SDL_GetCurrentDisplayMode(displays[i]);
        if (!mode)
            continue;
        s += std::format("display {}{}: {}x{} at {:.2f} Hz, pixel density {:.2f}, content scale {:.2f}\n", i,
                         displays[i] == shown ? " (window)" : "", mode->w, mode->h, double(mode->refresh_rate),
                         double(mode->pixel_density), double(SDL_GetDisplayContentScale(displays[i])));
    }
    SDL_free(displays);
    s += std::format("window mode: {}\n",
                     (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) ? "fullscreen" : "windowed");
    s += std::format("audio driver: {}\n", text_or_unknown(SDL_GetCurrentAudioDriver()));
    s += std::format("audio device: {}\n", text_or_unknown(SDL_GetAudioDeviceName(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK)));
    return s;
}

// The OpenGL strings of the renderer's own context, which SDL leaves
// current after creating it.
std::string opengl_strings()
{
    using GlGetString = const unsigned char *(*)(unsigned);
    constexpr unsigned kVendor = 0x1F00, kRenderer = 0x1F01, kVersion = 0x1F02;
    SDL_FunctionPointer proc = SDL_GL_GetProcAddress("glGetString");
    if (!proc)
        return {};
    GlGetString get = reinterpret_cast<GlGetString>(proc);
    auto text = [&](unsigned name) {
        const unsigned char *s = get(name);
        return s ? std::string(reinterpret_cast<const char *>(s)) : std::string("unknown");
    };
    return std::format("GPU: {}\nGPU vendor: {}\nGPU driver: OpenGL {}\n", text(kRenderer), text(kVendor),
                       text(kVersion));
}

std::string gpu_device_strings(SDL_GPUDevice *device)
{
    SDL_PropertiesID props = SDL_GetGPUDeviceProperties(device);
    auto text = [&](const char *name) { return text_or_unknown(SDL_GetStringProperty(props, name, "")); };
    return std::format("GPU: {}\nGPU driver: {} {} ({})\n", text(SDL_PROP_GPU_DEVICE_NAME_STRING),
                       text(SDL_PROP_GPU_DEVICE_DRIVER_NAME_STRING), text(SDL_PROP_GPU_DEVICE_DRIVER_VERSION_STRING),
                       text(SDL_PROP_GPU_DEVICE_DRIVER_INFO_STRING));
}

std::string renderer_section(SDL_Renderer *renderer)
{
    std::string s = "[renderer]\n";
    const char *name = SDL_GetRendererName(renderer);
    s += std::format("renderer: {}\n", text_or_unknown(name));
    SDL_PropertiesID props = SDL_GetRendererProperties(renderer);
    s += std::format("vsync: {}\n", SDL_GetNumberProperty(props, SDL_PROP_RENDERER_VSYNC_NUMBER, 0));
    s += std::format("max texture size: {}\n",
                     SDL_GetNumberProperty(props, SDL_PROP_RENDERER_MAX_TEXTURE_SIZE_NUMBER, 0));
    int w = 0, h = 0;
    if (SDL_GetCurrentRenderOutputSize(renderer, &w, &h))
        s += std::format("output size: {}x{}\n", w, h);
    // Direct3D and Metal give only the renderer's name: the adapter would
    // come from IDXGIDevice (D3D11) or an Objective-C MTLDevice (Metal).
    if (web::kEnabled) {
        std::string gpu = web::webgl_renderer();
        s += std::format("GPU: {}\n", gpu.empty() ? "unknown" : gpu);
    } else if (SDL_GPUDevice *device = SDL_GetGPURendererDevice(renderer)) {
        s += gpu_device_strings(device);
    } else if (name && std::strncmp(name, "opengl", 6) == 0) {
        s += opengl_strings();
    }
    return s;
}

} // namespace

std::string App::system_text() const
{
    std::string s = "# Theme Hospital Enhanced system report\n";
    s += build_section();
    s += machine_section();
    s += display_section(window_);
    s += renderer_section(renderer_);
    s += "[settings]\n";
    s += std::format("upscale: {}\n",
                     upscale_.on() ? std::format("{}:{}", upscale_.scaler->id, upscale_.factor) : "off");
    s += std::format("language: {}\n", data_ ? std::to_string(data_->language()) : "unknown");
    s += std::format("sound: {} volume {}, music: {} volume {}, announcements: {} volume {}\n", sounds_.sfx_on(),
                     sounds_.sfx_volume(), music_.music_on(), music_.volume(), sounds_.ambients_on(),
                     sounds_.ambient_volume());
    return s;
}

} // namespace th

#include "app/web.h"

#include "core/data/gog_install.h"

#include <emscripten.h>

#include <string>
#include <system_error>

namespace th::web
{

namespace
{

// The one pending pick: web/bridge.js answers through th_web_picked.
PickCallback g_callback = nullptr;
void *g_userdata = nullptr;

} // namespace

} // namespace th::web

extern "C" {

// The manifest's paths, upper case, one per line: the only files a folder
// pick needs to bring into memory.
EMSCRIPTEN_KEEPALIVE const char *th_web_manifest()
{
    static std::string list;
    if (list.empty())
        for (const th::ManifestFile &file : th::manifest_files()) {
            list += file.path;
            list += '\n';
        }
    return list.c_str();
}

// The picker's answer: the path it copied to, or null when cancelled.
EMSCRIPTEN_KEEPALIVE void th_web_picked(const char *path)
{
    th::web::PickCallback callback = th::web::g_callback;
    void *userdata = th::web::g_userdata;
    th::web::g_callback = nullptr;
    if (!callback)
        return;
    const char *files[2] = {path, nullptr};
    callback(userdata, files, 0);
}

} // extern "C"

namespace th::web
{

EM_JS(void, js_pick, (int folder), { Module.thPick(folder != 0); });
EM_JS(void, js_flush, (), { Module.thFlush(); });
EM_JS(void, js_persist, (), { Module.thRequestPersist(); });
EM_JS(void, js_download_tar, (const char *dir), { Module.thDownloadTar(UTF8ToString(dir)); });
EM_JS(void, js_download_file, (const char *path), { Module.thDownloadFile(UTF8ToString(path)); });
EM_JS(void, js_closed, (), { Module.thClosed(); });

void show_picker(bool folder, PickCallback callback, void *userdata)
{
    if (g_callback)
        return;
    g_callback = callback;
    g_userdata = userdata;
    js_pick(folder ? 1 : 0);
}

void discard_picked()
{
    std::error_code ec;
    std::filesystem::remove_all("/pick", ec);
}

void flush_storage()
{
    js_flush();
}

void request_persistent_storage()
{
    js_persist();
}

void download_report(const std::filesystem::path &dir)
{
    js_download_tar(dir.c_str());
}

void download_file(const std::filesystem::path &path)
{
    js_download_file(path.c_str());
}

void show_closed()
{
    js_closed();
}

std::string user_agent()
{
    return emscripten_run_script_string("navigator.userAgent");
}

std::string webgl_renderer()
{
    return emscripten_run_script_string(
        "(function () {"
        "  var gl = document.createElement('canvas').getContext('webgl2') ||"
        "           document.createElement('canvas').getContext('webgl');"
        "  if (!gl) return '';"
        "  var info = gl.getExtension('WEBGL_debug_renderer_info');"
        "  return String(gl.getParameter(info ? info.UNMASKED_RENDERER_WEBGL : gl.RENDERER));"
        "})()");
}

} // namespace th::web

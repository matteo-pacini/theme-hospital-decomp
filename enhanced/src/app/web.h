#pragma once

#include <filesystem>

namespace th::web
{

// What a picker calls back with: SDL's dialog callback (SDL_DialogFileCallback)
// so that the native and the browser pickers share one callback. `files` is a
// null-terminated list, empty when cancelled.
using PickCallback = void (*)(void *userdata, const char *const *files, int filter);

#ifdef __EMSCRIPTEN__

constexpr bool kEnabled = true;

// Opens the browser's file input (a game folder, or GOG's installer), copies
// what was chosen to memory and calls `callback` with that path. Must run in
// reaction to a click.
void show_picker(bool folder, PickCallback callback, void *userdata);

// Removes what show_picker copied.
void discard_picked();

// Writes /persist back to the browser's IndexedDB (asynchronously; calls
// that arrive meanwhile are merged).
void flush_storage();

// Asks the browser not to evict the origin's storage.
void request_persistent_storage();

// Downloads the report folder `dir` as one tar file.
void download_report(const std::filesystem::path &dir);

// Downloads the file `path`, under its own name.
void download_file(const std::filesystem::path &path);

// Tells the page the program has quit.
void show_closed();

#else

constexpr bool kEnabled = false;

inline void show_picker(bool, PickCallback, void *)
{
}
inline void discard_picked()
{
}
inline void flush_storage()
{
}
inline void request_persistent_storage()
{
}
inline void download_report(const std::filesystem::path &)
{
}
inline void download_file(const std::filesystem::path &)
{
}
inline void show_closed()
{
}

#endif

} // namespace th::web

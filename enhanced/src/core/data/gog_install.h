#pragma once

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <span>
#include <string>
#include <string_view>

namespace th
{

// One game file of GOG's v3 (28027) installer that the port needs, as the
// installer lays it out: whole zlib chunks, `chunk_count` of them from
// `first_chunk` on.
struct ManifestFile {
    // As in the install, upper case, components separated by '/'.
    std::string_view path;
    uint64_t size;
    // 64 lower-case hex digits.
    std::string_view sha256;
    uint32_t first_chunk;
    uint32_t chunk_count;
};

// The files an install holds, sorted by path.
std::span<const ManifestFile> manifest_files();

// The manifest's version, written to an install's install.ok. A different
// version means the install is not complete for this build.
uint32_t manifest_version();

// How an install reports and is stopped. Both are optional.
struct InstallOptions {
    // Called as work proceeds with the bytes done and the total, whose unit
    // is the same for the whole call.
    std::function<void(uint64_t done, uint64_t total)> progress;
    // Set from another thread to stop: the call removes what it made and
    // returns with `cancelled` set.
    const std::atomic<bool> *cancel = nullptr;
};

struct InstallResult {
    bool ok = false;
    bool cancelled = false;
    // On failure: the file and what was wrong with it (missing, size,
    // checksum), or what could not be done.
    std::string error;
};

// Writes the manifest's files from GOG's installer (checked for its size and
// sha256 first) into `dest`, whatever `dest` held being replaced only once
// every file is written and checked. The work is done in `dest`.partial,
// renamed to `dest` at the end with install.ok in it, written last.
InstallResult install_from_installer(const std::filesystem::path &installer, const std::filesystem::path &dest,
                                     const InstallOptions &options = {});

// The same from a game folder: only the manifest's files are taken, found
// case-insensitively; other files are ignored.
InstallResult install_from_folder(const std::filesystem::path &folder, const std::filesystem::path &dest,
                                  const InstallOptions &options = {});

// Re-hashes every manifest file of the install in `dest`.
InstallResult verify_install(const std::filesystem::path &dest, const InstallOptions &options = {});

// A quick check, hashing nothing: install.ok holds the current manifest
// version and every manifest file exists with the right size.
bool is_installed(const std::filesystem::path &dest);

} // namespace th

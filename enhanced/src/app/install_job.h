#pragma once

#include "core/data/gog_install.h"
#include "core/render/upscale/scaler.h"

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <thread>

namespace th
{

enum class InstallKind {
    Installer, // GOG's installer file
    Folder,    // a game folder
    Verify,    // re-hash an install
};

// What a dropped or chosen path is: a directory is a game folder, anything
// else the installer.
InstallKind install_kind_of(const std::filesystem::path &path);

// What the install thread builds once the game is installed: the upscaled
// sprites for `config` into `cache_dir` (nothing when the config is off).
struct UpscaleBuild {
    UpscaleConfig config;
    std::filesystem::path cache_dir;
};

// One install or verification on a worker thread, driven from the UI thread.
class InstallJob
{
  public:
    InstallJob() = default;
    InstallJob(const InstallJob &) = delete;
    InstallJob &operator=(const InstallJob &) = delete;
    // Cancels a running job and waits for it.
    ~InstallJob();

    // Starts a job unless one is running (or its result not yet taken);
    // returns whether it started. `source` is unused for Verify. Installs
    // and verifies `dest`. After a successful install the thread builds
    // `upscale`, its progress then being the job's.
    bool start(InstallKind kind, const std::filesystem::path &source, const std::filesystem::path &dest,
               const UpscaleBuild &upscale = {});

    // Asks a running job to stop; take_result then reports `cancelled`.
    void cancel() { cancel_ = true; }

    // True from start until take_result returned the result.
    bool busy() const { return thread_.joinable(); }
    InstallKind kind() const { return kind_; }
    // Whether the job is past the install, building the upscaled sprites.
    bool upscaling() const { return upscaling_; }
    // Why the upscaled sprites could not be built after a good install; empty
    // otherwise. Valid once take_result returned the result.
    const std::string &upscale_error() const { return upscale_error_; }
    // The last progress reported: the bytes done and the total (0 before the
    // first report).
    uint64_t done() const { return done_; }
    uint64_t total() const { return total_; }

    // The result once the job has finished, joining its thread; empty while
    // it runs or when there is no job.
    std::optional<InstallResult> take_result();

  private:
    // Builds the upscaled sprites of the game just installed in `dest`.
    void build_upscale(const std::filesystem::path &dest, const UpscaleBuild &upscale);

    std::thread thread_;
    InstallKind kind_ = InstallKind::Installer;
    std::atomic<bool> cancel_{false};
    std::atomic<bool> finished_{false};
    std::atomic<bool> upscaling_{false};
    std::string upscale_error_;
    std::atomic<uint64_t> done_{0};
    std::atomic<uint64_t> total_{0};
    // Written by the thread before it sets finished_.
    InstallResult result_;
};

} // namespace th

#include "app/install_job.h"

#include "core/render/upscale/cache.h"

#include <system_error>

namespace th
{

InstallKind install_kind_of(const std::filesystem::path &path)
{
    std::error_code ec;
    return std::filesystem::is_directory(path, ec) ? InstallKind::Folder : InstallKind::Installer;
}

InstallJob::~InstallJob()
{
    cancel_ = true;
    if (thread_.joinable())
        thread_.join();
}

bool InstallJob::start(InstallKind kind, const std::filesystem::path &source, const std::filesystem::path &dest,
                       const UpscaleBuild &upscale)
{
    if (thread_.joinable())
        return false;
    kind_ = kind;
    cancel_ = false;
    finished_ = false;
    done_ = 0;
    total_ = 0;
    result_ = {};
    upscaling_ = false;
    upscale_error_.clear();
    thread_ = std::thread([this, kind, source, dest, upscale] {
        InstallOptions options;
        options.cancel = &cancel_;
        options.progress = [this](uint64_t done, uint64_t total) {
            total_ = total;
            done_ = done;
        };
        switch (kind) {
        case InstallKind::Installer:
            result_ = install_from_installer(source, dest, options);
            break;
        case InstallKind::Folder:
            result_ = install_from_folder(source, dest, options);
            break;
        case InstallKind::Verify:
            result_ = verify_install(dest, options);
            break;
        }
        if (result_.ok && kind != InstallKind::Verify && upscale.config.on())
            build_upscale(dest, upscale);
        finished_ = true;
    });
    return true;
}

// Builds the upscaled sprites of the game just installed in `dest`. A failure
// does not undo the install: the game builds them again when it starts.
void InstallJob::build_upscale(const std::filesystem::path &dest, const UpscaleBuild &upscale)
{
    upscaling_ = true;
    done_ = 0;
    total_ = 0;
    th::MapHires hires;
    std::string error;
    auto progress = [this](size_t done, size_t total) {
        total_ = total;
        done_ = done;
        return !cancel_;
    };
    UpscaleConfig config = upscale.config;
    if (!upscaled_map_sheets(dest, upscale.cache_dir, config, progress, hires, &error))
        upscale_error_ = cancel_ ? "Upscaling cancelled." : error;
}

std::optional<InstallResult> InstallJob::take_result()
{
    if (!thread_.joinable() || !finished_)
        return std::nullopt;
    thread_.join();
    return std::move(result_);
}

} // namespace th

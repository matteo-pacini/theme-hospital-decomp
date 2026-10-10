#include "app/install_job.h"

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

bool InstallJob::start(InstallKind kind, const std::filesystem::path &source, const std::filesystem::path &dest)
{
    if (thread_.joinable())
        return false;
    kind_ = kind;
    cancel_ = false;
    finished_ = false;
    done_ = 0;
    total_ = 0;
    result_ = {};
    thread_ = std::thread([this, kind, source, dest] {
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
        finished_ = true;
    });
    return true;
}

std::optional<InstallResult> InstallJob::take_result()
{
    if (!thread_.joinable() || !finished_)
        return std::nullopt;
    thread_.join();
    return std::move(result_);
}

} // namespace th

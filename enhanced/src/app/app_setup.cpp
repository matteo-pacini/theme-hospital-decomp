// The setup screen: the install and verification jobs, the folder pickers and
// the setup command-line arguments.

#include "app/app.h"
#include "app/web.h"

#include <utility>

namespace th
{

namespace
{

void SDLCALL on_folder_chosen(void *userdata, const char *const *files, int)
{
    if (!files || !files[0])
        return; // error or cancelled
    auto *pick = static_cast<FolderPick *>(userdata);
    std::lock_guard lock(pick->mutex);
    pick->chosen = files[0];
}

std::optional<std::string> take_pick(FolderPick &pick)
{
    std::lock_guard lock(pick.mutex);
    return std::exchange(pick.chosen, std::nullopt);
}

} // namespace

// The setup screen, with what its buttons ask for.
void App::setup_step()
{
    static const SDL_DialogFileFilter kInstallerFilter = {"GOG's installer", "exe"};
    switch (draw_setup_screen(setup_view())) {
    case SetupAction::Quit:
        running_ = false;
        break;
    case SetupAction::InstallFromInstaller:
        if (web::kEnabled)
            web::show_picker(false, on_folder_chosen, &installer_pick_);
        else
            SDL_ShowOpenFileDialog(on_folder_chosen, &installer_pick_, window_, &kInstallerFilter, 1, nullptr, false);
        break;
    case SetupAction::InstallFromFolder:
        if (web::kEnabled)
            web::show_picker(true, on_folder_chosen, &pick_);
        else
            SDL_ShowOpenFolderDialog(on_folder_chosen, &pick_, window_, nullptr, false);
        break;
    case SetupAction::Verify:
        start_setup_job(InstallKind::Verify, {});
        break;
    case SetupAction::Cancel:
        job_.cancel();
        break;
    case SetupAction::None:
        break;
    }
}

std::optional<std::filesystem::path> App::parse_setup_args(int argc, char **argv)
{
    std::optional<std::filesystem::path> install_from;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--user-data")
            user_data_ = true;
        else if (arg == "--install-from" && i + 1 < argc)
            install_from = argv[++i];
        else if (arg == "--drop" && i + 1 < argc)
            drop_arg_ = argv[++i];
        else if (arg == "--upscale" && i + 1 < argc)
            upscale_arg_ = argv[++i];
    }
    return install_from;
}

// --drop: the event a file dropped on the window sends.
void App::queue_drop_arg()
{
    if (drop_arg_.empty())
        return;
    SDL_Event e{};
    e.type = SDL_EVENT_DROP_FILE;
    e.drop.windowID = SDL_GetWindowID(window_);
    e.drop.data = drop_arg_.c_str();
    SDL_PushEvent(&e);
}

bool App::install_from_command_line(const std::filesystem::path &from)
{
    if (user_paths_.game_dir.empty()) {
        std::fprintf(stderr, "--install-from: no per-user folder for the game files (no home folder)\n");
        return false;
    }
    std::fprintf(stderr, "Installing the game files from %s into %s\n", from.string().c_str(),
                 user_paths_.game_dir.string().c_str());
    uint64_t tenth = 0;
    InstallOptions options;
    options.progress = [&tenth](uint64_t done, uint64_t total) {
        uint64_t now = total ? done * 10 / total : 0;
        for (; tenth < now; tenth++)
            std::fprintf(stderr, "  %llu%%\n", static_cast<unsigned long long>((tenth + 1) * 10));
    };
    InstallResult result = install_kind_of(from) == InstallKind::Folder
                               ? install_from_folder(from, user_paths_.game_dir, options)
                               : install_from_installer(from, user_paths_.game_dir, options);
    if (!result.ok) {
        std::fprintf(stderr, "--install-from: %s\n", result.error.c_str());
        return false;
    }
    std::fprintf(stderr, "Installed.\n");
    return true;
}

void App::start_setup_job(InstallKind kind, const std::filesystem::path &source)
{
    if (job_.busy())
        return;
    data_error_.clear();
    setup_notice_.clear();
    if (user_paths_.game_dir.empty()) {
        data_error_ = "There is no per-user folder to install the game files into (no home folder).";
        return;
    }
    job_.start(kind, source, user_paths_.game_dir,
               kind == InstallKind::Verify ? UpscaleBuild{} : commit_upscale_choice());
}

UpscaleBuild App::commit_upscale_choice()
{
    init_upscale_choice();
    UpscaleBuild build;
    const std::span<const Scaler> algorithms = scalers();
    if (upscale_choice_.on && size_t(upscale_choice_.algorithm) < algorithms.size())
        build.config = {&algorithms[size_t(upscale_choice_.algorithm)], upscale_choice_.factor};
    build.cache_dir = upscale_cache_dir();
    // The screen's choice is the player's setting from here on, whatever
    // --upscale preselected; a driven run leaves the settings alone.
    if (!driver_) {
        upscale_arg_.clear();
        settings_.set("upscale", upscale_to_string(build.config));
        settings_.save();
    }
    return build;
}

// What the dialogs chose and the worker thread finished since the last frame.
void App::poll_setup()
{
    if (std::optional<std::string> path = take_pick(installer_pick_))
        start_setup_job(InstallKind::Installer, *path);
    if (std::optional<std::string> path = take_pick(pick_))
        start_setup_job(InstallKind::Folder, *path);
    poll_setup_job();
}

void App::poll_setup_job()
{
    std::optional<InstallResult> result = job_.take_result();
    if (!result)
        return;
    if (result->cancelled)
        data_error_ = job_.kind() == InstallKind::Verify ? "Verification cancelled." : "Installation cancelled.";
    else if (!result->ok)
        data_error_ = result->error;
    else if (job_.kind() != InstallKind::Verify) {
        if (!job_.upscale_error().empty())
            setup_notice_ = "The game is installed; its graphics could not be upscaled: " + job_.upscale_error();
        try_load_data(user_paths_.game_dir);
        web::flush_storage();
        web::request_persistent_storage();
    }
    if (job_.kind() != InstallKind::Verify)
        web::discard_picked();
    refresh_game_dir_state();
    if (result->ok && job_.kind() == InstallKind::Verify)
        setup_notice_ = game_dir_incomplete_
                            ? "The game files are correct, but the install was not completed: install again."
                            : "All installed game files are present and correct.";
}

void App::refresh_game_dir_state()
{
    std::error_code ec;
    game_dir_exists_ = !user_paths_.game_dir.empty() && std::filesystem::exists(user_paths_.game_dir, ec);
    game_dir_incomplete_ = game_dir_exists_ && !is_installed(user_paths_.game_dir);
}

SetupView App::setup_view()
{
    init_upscale_choice();
    return {search_,          data_error_,          setup_notice_,    user_paths_.game_dir.string(),
            game_dir_exists_, game_dir_incomplete_, job_.busy(),      job_.kind() == InstallKind::Verify,
            job_.done(),      job_.total(),         &upscale_choice_, job_.upscaling()};
}

} // namespace th

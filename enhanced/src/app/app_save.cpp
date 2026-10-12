// Saved games and the player's details: save slots, the Continue slot, the
// .hdt file and autosave (src/io.cpp).

#include "app/app.h"
#include "app/web.h"
#include "core/data/gamedata.h"
#include "core/render/shadows.h"

#include <algorithm>
#include <utility>

namespace th
{

std::filesystem::path App::save_path(int slot) const
{
    return save_slot_path(save_dir_, player_name(), slot);
}

SaveInfo App::current_save() const
{
    SaveInfo info;
    info.balance_id = balance_id_;
    info.tutorial = tutorial_;
    // The view's centre, not its top-left: it is restored the same whatever
    // the view surface's size, including when it is 0x0 on a first load.
    info.view_centre_x = camera_.x + surface_.width() / 2;
    info.view_centre_y = camera_.y + surface_.height() / 2;
    return info;
}

// check_save_games (src/menu.cpp): which File > Save/Load slots hold a save,
// from the files in the save folder.
void App::refresh_save_slots()
{
    save_slots_.fill(false);
    for (int slot = 1; slot <= kMenuSlots; slot++)
        save_slots_[size_t(slot)] = save_slot_exists(save_path(slot));
}

// check_okay_to_save (src/io.cpp): saving needs the normal mouse mode, with
// nothing being placed, built or edited (no blueprint on the map, no room being
// furnished) and no modal requester up.
bool App::can_save(const Game &game) const
{
    if (interface_.modal() || hire_.placing() || game.world.placing.placing)
        return false;
    return game.world.building.mouse == BuildMouse::None && !build_tool_busy(game) &&
           game.world.building.build.room == 0 && !is_player_building(game.world);
}

// The panel messages the save and load show (misc_text indices, io.cpp):
// saved, cannot save and load failed.
constexpr uint8_t kPanelMessageSaved = 0x18;
constexpr uint8_t kPanelMessageCannotSave = 0x19;
constexpr uint8_t kPanelMessageLoadFailed = 0x1c;

// display_panel_special_info: the message text and its timer in frames.
void App::show_panel_message(uint8_t id, int frames)
{
    if (!data_)
        return;
    panel_message_ = std::string(data_->text().at(TextSection::Misc, id));
    panel_message_time_ = frames;
}

// save_game (src/io.cpp): the whole game to the slot's file.
void App::perform_save(int slot)
{
    if (!map_ || !data_)
        return;
    if (!can_save(map_->game)) {
        show_panel_message(kPanelMessageCannotSave, 0x14);
        return;
    }
    std::string error;
    // The live generator is the sounds'; the snapshot carries the game's.
    map_->game.safeseed = sounds_.safeseed;
    if (!write_save_slot(save_path(slot), map_->game, current_save(), &error)) {
        std::fprintf(stderr, "save slot %d: %s\n", slot, error.c_str());
        show_panel_message(kPanelMessageCannotSave, 0x14);
        return;
    }
    show_panel_message(kPanelMessageSaved, 3);
    refresh_save_slots();
    save_player_details();
}

// load_game (src/io.cpp): the save at the slot's file, over a fresh level;
// false, with the load-failed message, when it cannot be loaded.
bool App::perform_load(int slot)
{
    if (!data_)
        return false;
    std::string error;
    auto view = std::make_unique<LevelView>();
    SaveInfo info;
    if (!read_save_slot(save_path(slot), view->game, *data_, &info, &error)) {
        std::fprintf(stderr, "load slot %d: %s\n", slot, error.c_str());
        show_panel_message(kPanelMessageLoadFailed, 0x46);
        return false;
    }
    auto level = data_->load_level(info.level, &error);
    if (!level) {
        std::fprintf(stderr, "%s\n", error.c_str());
        show_panel_message(kPanelMessageLoadFailed, 0x46);
        return false;
    }
    view->shadows = std::make_unique<ShadowSprites>(data_->blocks(), data_->ghost());
    // The settings and the view the save continues with (window and
    // scroll_y, src/data.cpp:22-23, restored by load_game io.cpp:2067-2093).
    if (!info.balance_id.empty())
        balance_id_ = info.balance_id;
    tutorial_ = info.tutorial;
    start_view(std::move(view), *level, false);
    // The view centre over the surface's centre: with a 0x0 surface (a first
    // load from the start-up screens) this leaves the centre, and resize_view
    // moves the camera to it once the view has its size.
    camera_ = {info.view_centre_x - surface_.width() / 2, info.view_centre_y - surface_.height() / 2};
    return true;
}

// save_game(0x63) when the game returns to the main menu.
void App::save_continue_slot()
{
    if (!map_ || !data_ || !can_save(map_->game))
        return;
    std::string error;
    map_->game.safeseed = sounds_.safeseed;
    if (!write_save_slot(save_path(kContinueSlot), map_->game, current_save(), &error))
        std::fprintf(stderr, "continue save: %s\n", error.c_str());
    else {
        show_panel_message(kPanelMessageSaved, 3);
        save_player_details();
    }
}

std::string App::player_name() const
{
    if (!startup_.player_name().empty() || !data_)
        return startup_.player_name();
    return std::string(data_->text().misc(1));
}

// The options the original keeps in myplayer, put in force: the sound, music
// and announcement switches and volumes, the playlist, shadows and speed.
// Switches and volumes of an audio system that is not there are left alone.
void App::apply_player_details()
{
    const PlayerDetails &d = details_;
    shadows_ = d.shadows_on != 0;
    if (sounds_.sound_ok()) {
        if (sounds_.sfx_on() != (d.sfx_on != 0))
            sounds_.toggle_sound();
        if (sounds_.ambients_on() != (d.ambients_on != 0))
            sounds_.toggle_ambient();
        sounds_.set_volume_item(9 - d.sfx_volume * 10 / 128);
        sounds_.set_ambient_volume_item(9 - d.ambient_volume * 10 / 128);
    }
    if (music_.music_ok()) {
        if (music_.music_on() != (d.music_on != 0))
            music_.toggle_music();
        music_.set_volume_item(9 - d.music_volume * 10 / 128);
        for (size_t i = 0; i < d.track_enabled.size(); i++)
            music_.set_track_enabled(int(i), d.track_enabled[i] != 0);
    }
    switch (d.game_speed) {
    case 1:
        speed_ = Speed::Slowest;
        break;
    case 5:
        speed_ = Speed::Slower;
        break;
    case 0x16:
        speed_ = Speed::Normal;
        break;
    case 0x19:
        speed_ = Speed::Max;
        break;
    case 0x7d00:
        speed_ = Speed::AndThenSome;
        break;
    default:
        break;
    }
}

void App::collect_player_details()
{
    PlayerDetails &d = details_;
    d.shadows_on = shadows_;
    if (sounds_.sound_ok()) {
        d.sfx_on = sounds_.sfx_on();
        d.sfx_volume = uint8_t(sounds_.sfx_volume());
        d.ambients_on = sounds_.ambients_on();
        d.ambient_volume = uint8_t(sounds_.ambient_volume());
    }
    if (music_.music_ok()) {
        d.music_on = music_.music_on();
        d.music_volume = uint8_t(music_.volume());
        for (size_t i = 0; i < d.track_enabled.size(); i++)
            d.track_enabled[i] = music_.track_enabled(int(i));
    }
    constexpr std::array<uint16_t, 5> kGameSpeeds = {1, 5, 0x16, 0x19, 0x7d00};
    size_t choice = speed_ == Speed::Slowest  ? 0
                    : speed_ == Speed::Slower ? 1
                    : speed_ == Speed::Normal ? 2
                    : speed_ == Speed::Max    ? 3
                                              : 4;
    d.game_speed = kGameSpeeds[choice];
    if (data_)
        d.language = uint8_t(data_->language());
    if (map_) {
        const Advisor &a = map_->game.advisor;
        std::ranges::copy(a.given, d.advisor_flags.begin());
        d.advisor_flags[20] = a.enabled;
    }
}

void App::give_advisor_flags(Game &game) const
{
    std::copy_n(details_.advisor_flags.begin(), game.advisor.given.size(), game.advisor.given.begin());
    game.advisor.enabled = details_.advisor_flags[20] != 0;
}

void App::load_player_details()
{
    std::filesystem::path path = player_details_path(save_dir_, player_name());
    DetailsRead read = read_player_details(path, details_);
    if (read == DetailsRead::Failed) {
        std::fprintf(stderr, "cannot read the player details %s\n", path.string().c_str());
        return;
    }
    if (read == DetailsRead::Missing) {
        set_default_player_details(details_);
        // Driven runs start at Normal: their scripts count frames.
        if (driver_ && !drive_default_speed_)
            details_.game_speed = kDrivenGameSpeed;
    }
    apply_player_details();
    if (map_)
        give_advisor_flags(map_->game);
}

void App::save_player_details()
{
    collect_player_details();
    std::filesystem::path path = player_details_path(save_dir_, player_name());
    if (!write_player_details(path, details_, player_name()))
        std::fprintf(stderr, "cannot write the player details %s\n", path.string().c_str());
    web::flush_storage();
}

void App::check_autosave(const Game &game)
{
    if (!details_.autosave || (game.date.day != 28 && !need_autosave_))
        return;
    uint8_t stamp = uint8_t(game.date.month + game.date.day);
    if (stamp == last_autosave_ && !need_autosave_)
        return;
    if (!can_save(game)) {
        need_autosave_ = true;
        return;
    }
    perform_save(kAutosaveSlot);
    last_autosave_ = stamp;
    need_autosave_ = false;
}

} // namespace th

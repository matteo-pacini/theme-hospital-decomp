// Starting a level: begin_level and the App members that take a level in play
// and reset the interface round it.

#include "app/app.h"
#include "app/app_internal.h"
#include "core/data/gamedata.h"
#include "core/render/shadows.h"

#include <utility>

namespace th
{

inline namespace app_detail
{

// Starts level `number` with its settings (the balance set `balance_id`:
// "EASY", "FULL" or "HARD"), or returns nothing and says why. `carried` is
// what the last level left.
std::unique_ptr<LevelView> begin_level(const GameData &data, const Level &level, int number,
                                       std::string_view balance_id, std::string *error, Carried carried)
{
    Balance settings;
    if (!load_balance(data.dir(), number, balance_id, settings, error))
        return nullptr;
    auto view = std::make_unique<LevelView>();
    view->game.staff_market.ref_counts = reference_counts(data.text());
    view->game.start(level, number, settings, data.anims(), data.person_complexes(), data.staff_complexes());
    view->game.carry_score(carried.score);
    if (carried.buckets)
        view->game.carry_buckets(*carried.buckets);
    view->game.carry_force_bonus_level(carried.force_bonus);
    view->shadows = std::make_unique<ShadowSprites>(data.blocks(), data.ghost());
    add_static_shadows(view->game.world.map);
    return view;
}

} // namespace app_detail

// Keeps what the level in progress leaves for the next one: the score, the
// rivals' banks and the forced bonus level.
void App::remember_level_end()
{
    if (!map_)
        return;
    carried_score_ = map_->game.progress.score;
    carried_buckets_ = map_->game.rival_buckets();
    have_carried_buckets_ = true;
    carried_force_bonus_ = map_->game.force_bonus_level;
}

void App::load_level(int number)
{
    // allhosps[player].score outlives the game it was in, as in the original:
    // init_player of the next level keeps it.
    remember_level_end();
    std::string error;
    auto level = data_->load_level(number, &error);
    if (!level) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return;
    }
    auto map = begin_level(*data_, *level, number, balance_id_, &error,
                           {carried_score_, have_carried_buckets_ ? &carried_buckets_ : nullptr, carried_force_bonus_});
    if (!map) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return;
    }
    map->game.progress.level_after_bonus = uint8_t(level_after_bonus_);
    start_view(std::move(map), *level, true);
    // rat_fest_builder's vary_speed(0x1e): the rat game starts at normal speed.
    if (map_->game.world.rats.fest_level)
        speed_ = Speed::Normal;
}

// What setup_new_level clears of the interface (othervar, gamevar) and what
// EV_CANCEL_BUILD closes: nothing of the old level stays open or held.
void App::reset_interface_for_level()
{
    hire_ = {};
    if (interface_.modal())
        interface_.close_yes_no(false);
    hire_staff_event(hire_, interface_, map_->game.world, {EV_CANCEL_BUILD, 0, 0, 0});
    close_irequesters(interface_);
    info_ = {};
    demand_ = {};
    queue_ = {};
    build_ui_ = {};
    panel_info_ = 0;
    panel_thing_ = 0;
    panel_info_timer_ = 0;
    yes_event_ = {};
    no_event_ = {};
    pending_save_slot_ = 0;
}

// Takes over `view` as the level in play and resets the interface round it.
// A loaded save has the advisor, the hospital settings and the insurers in
// the snapshot already, so `fresh` skips their level-start setup. `level` is
// the level file, for its camera start and scroll area.
void App::start_view(std::unique_ptr<LevelView> view, const Level &level, bool fresh)
{
    // The advice given and the advisor's switch outlive the level.
    if (map_)
        collect_player_details();
    give_advisor_flags(view->game);
    level_ = view->game.level_number();
    map_ = std::move(view);
    reset_interface_for_level();
    if (fresh) {
        key_mappos_.fill(0);
        start_advisor(map_->game.advisor, tutorial_);
        start_hospital_settings(map_->game.hospital, map_->game.settings());
        setup_insurances(map_->game.world, ticks_ms());
    }
    charts_.hospital_names[0] = startup_.player_name();
    name_computer_players(map_->game.world.rivals, data_->text());
    for (size_t i = 1; i < charts_.hospital_names.size(); i++)
        charts_.hospital_names[i] = map_->game.world.rivals[i].name.data();
    charts_.open = false;
    fax_.open = false;
    popups_ = {};
    heli_seen_ = 0;
    town_.close();
    year_end_.close();
    year_end_showing_ = false;
    winlose_.close();
    winlose_showing_ = false;
    winlose_movie_index_ = -1;
    // A message from the screen before this game (e.g. a load failure) is not
    // shown on the new level.
    panel_message_.clear();
    panel_message_time_ = 0;
    last_year_ = map_->game.date.year;
    panel_state_.shown_balance = map_->game.balance;
    sounds_.safeseed = map_->game.safeseed;
    // A level starts on a song picked by the clock, as the game does.
    music_.start_level(ticks_ms());
    sounds_.init_ambients();
    // Centre the view on the player's starting cell, as load_level does.
    int start = level.camera_start[0];
    scroll_min_ = level.scroll_min;
    scroll_max_ = level.scroll_max;
    Point centre = cell_to_world(start % kMapSize, start / kMapSize);
    camera_ = {centre.x - surface_.width() / 2, centre.y - surface_.height() / 2};
    refresh_save_slots();
}

} // namespace th

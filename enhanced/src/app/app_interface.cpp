// The in-game interface, part 1: its frame step, the requesters' input and
// the panel and map mouse.

#include "app/app.h"
#include "app/app_internal.h"
#include "core/data/gamedata.h"

#include <utility>

namespace th
{

int App::ui_scale() const
{
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(window_, &w, &h);
    if (tools_.ui_scale > 0)
        return tools_.ui_scale;
    return std::max(1, h / kMinViewHeight);
}

// Runs and draws the game's interface for the frame: the requester windows
// and the panel, in the 640 x 480 canvas at the window's bottom centre.
// The windows go first: the front one takes the mouse, a modal one (the
// yes/no requester) everything.
void App::interface_step(int out_w, int out_h, int scale)
{
    int ox = (out_w - kUiWidth * scale) / 2, oy = out_h - kUiHeight * scale;
    UiMouse mouse = ui_mouse(ox, oy, scale);
    Game *game = map_ ? &map_->game : nullptr;
    InfoWindowResult info;
    help_mouse_ = mouse;

    // A framed help page holds the screen: its close button, Escape or Space
    // closes it, and the screen behind is not read.
    if (help_page_open()) {
        if (help_.update_close_hover(mouse.x, mouse.y))
            sounds_.play_sound(kSourceGeneral, kSfxHighlight5);
        bool close = std::exchange(escape_, false) || (mouse.left_pressed && help_.box().close_hover);
        if (close) {
            help_.finish_composite_help();
            sounds_.play_sound(kSourceGeneral, kSfxYesX);
        }
        interface_draw(game, ox, oy, scale);
        return;
    }

    bool escape = interface_escape(game);
    if (game) {
        sync_object_requesters(interface_, *game);
        sync_build_requester(interface_, *game);
    }
    InterfaceEvents events = interface_.frame(mouse, escape);
    if (game)
        info = interface_requesters(*game, events, mouse);
    interface_yes_no(game, events);
    bool paused = game && game->paused;
    bool input = !events.mouse_on_window && !interface_.modal() && !menu_.active;
    PanelAction action = panel_step(game, mouse, paused, input);
    if (game)
        popup_icons_frame(popups_, game->world, mouse, *data_, frame_seconds_);
    if (game) {
        bool placing = game->world.placing.placing || hire_.placing();
        interface_panel_action(*game, action, placing);
        // The map: what is being placed follows the pointer.
        bool on_map = input && !paused && mouse.y < panel_top_of(*game);
        interface_map_mouse(*game, mouse, on_map, placing);
        interface_map_hover(*game, mouse, on_map, placing);
        if (interface_.requester() == Requester::Queue)
            queue_window_after_frame(*game);
        interface_advisor(*game, mouse, input, paused);
        interface_command(*game, info);
        interface_room_help(*game);
        interface_help();
    }
    interface_draw(game, ox, oy, scale);
}

// One frame of the panel's icons for `mouse`; the rat game's panel has none to
// raise or click.
PanelAction App::panel_step(const Game *game, const UiMouse &mouse, bool paused, bool input)
{
    if (game && game->world.rats.fest_level) {
        panel_state_ = {.shown_balance = panel_state_.shown_balance};
        return PanelAction::None;
    }
    return panel_frame(panel_state_, mouse, *data_, paused, input);
}

int App::panel_top_of(const Game &game) const
{
    return game.world.rats.fest_level ? rat_panel_top(*data_) : panel_top(*data_);
}

// Escape: with a modal requester up, the requester's; else it drops what is
// being hired, bought or placed (setup_abort_all_requesters). Returns
// whether the requesters still get it.
bool App::interface_escape(Game *game)
{
    bool escape = std::exchange(escape_, false);
    if (!escape || !game || interface_.modal())
        return escape;
    if (hire_.placing() || interface_.requester() == Requester::HireStaff ||
        interface_.requester() == Requester::Jukebox) {
        game->event = hire_staff_cancel(hire_);
        return false;
    }
    if (interface_.requester() == Requester::Queue) {
        // setup_abort_all_requesters closes the queue window (EV_CANCEL_BUILD).
        game->event = {EV_CLOSE_QUEUE_WINDOW, 0, 0, 0};
        return false;
    }
    if (object_escape(*game))
        return false;
    if (interface_.window_count() > 0 && interface_.requester() == Requester::None) {
        // The patient, staff or handyman window: EV_CANCEL_BUILD closes it.
        game->event = hire_staff_cancel(hire_);
        return false;
    }
    return true;
}

// The requesters' input for the frame: the jukebox, buying objects,
// building, hiring and the patient, staff and handyman windows, whose
// result it returns.
InfoWindowResult App::interface_requesters(Game &game, const InterfaceEvents &events, const UiMouse &mouse)
{
    interface_jukebox_input(game, events, mouse);
    const bool *keys = SDL_GetKeyboardState(nullptr);
    bool shift = keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];
    bool ctrl = keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL];
    object_requesters_input(interface_, object_ui_, game, events, mouse, {shift, ctrl});
    build_requesters_input(interface_, build_ui_, game, events, mouse);
    queue_window_input(queue_, interface_, game, {*data_, requester_art_, map_cell_, events, mouse});
    interface_hire_input(game, events);
    interface_staff_demand_input(game, events);
    InfoWindowResult info = info_window_frame(info_, interface_, events, mouse,
                                              {game.world, data_->text(), {scroll_min_, scroll_max_}, game.balance});
    for (uint8_t sfx : info.sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    if (info.event.type)
        game.event = info.event;
    if (info.slist_staff)
        charts_.slist_staff = info.slist_staff;
    if (info.centre_on)
        centre_view_on(game.world, info.centre_on);
    return info;
}

// process_tracking_windows' othervar.center_on_slot: the view centres on thing
// `id` unless it is beyond the scroll limit.
void App::centre_view_on(const World &world, uint16_t id)
{
    Point p = info_centre(world, id);
    if (!beyond_scroll_limit(p, scroll_min_, scroll_max_))
        camera_ = {p.x - surface_.width() / 2, p.y - surface_.height() / 2};
}

void App::interface_jukebox_input(Game &game, const InterfaceEvents &events, const UiMouse &mouse)
{
    if (interface_.requester() != Requester::Jukebox)
        return;
    JukeboxResult jr = jukebox_frame(jukebox_, interface_, {events, mouse}, requester_art_, music_, game.paused);
    for (uint8_t sfx : jr.sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    if (jr.close)
        game.event = hire_staff_cancel(hire_);
}

void App::interface_hire_input(Game &game, const InterfaceEvents &events)
{
    std::vector<uint8_t> hire_sounds;
    Event hire_event = hire_staff_click(hire_, events, game, &hire_sounds);
    for (uint8_t sfx : hire_sounds)
        sounds_.play_sound(kSourceGeneral, sfx);
    if (hire_event.type)
        game.event = hire_event;
}

void App::interface_staff_demand_input(Game &game, const InterfaceEvents &events)
{
    if (interface_.requester() != Requester::StaffDemand)
        return;
    bool accept = false;
    if (Event e = staff_demand_click(demand_, events, accept); e.type) {
        sounds_.play_sound(kSourceGeneral, accept ? kSfxBonus2 : kSfxSack);
        game.event = e;
    }
}

// EV_QUIT_GAME (quit_game 1): close the requester, then save the Continue
// slot (game.cpp's close_irequesters before save_game(0x63)) and return to
// the main menu; a run started with --level has no menu to return to.
void App::quit_game()
{
    asking_quit_ = false;
    yes_event_ = no_event_ = {};
    interface_.close_yes_no(false);
    if (!level_from_cli_ && data_) {
        save_continue_slot();
        return_to_main_menu();
    } else {
        running_ = false;
    }
}

// The yes/no requester's answer: its sound, then what it was asked for.
void App::interface_yes_no(Game *game, const InterfaceEvents &events)
{
    if (events.yes_no == YesNoAnswer::None)
        return;
    // The No button plays SFX_NO4 (req13); Escape and a right click outside
    // the window answer No without a sound (req13_cancel_check).
    if (events.yes_no == YesNoAnswer::Yes)
        sounds_.play_sound(kSourceGeneral, kSfxYesX);
    else if (!events.yes_no_cancel)
        sounds_.play_sound(kSourceGeneral, kSfxNo4);
    if (events.yes_no == YesNoAnswer::Yes && asking_quit_) {
        quit_game();
        return;
    }
    int save_slot = std::exchange(pending_save_slot_, 0);
    if (events.yes_no == YesNoAnswer::Yes && save_slot > 0) {
        // EV_CLOSE_SAVE_YES_NO closes the requester before the save
        // (check_save_games follows).
        interface_.close_yes_no(false);
        perform_save(save_slot);
    }
    if (game) {
        Event e = events.yes_no == YesNoAnswer::Yes ? yes_event_ : no_event_;
        if (e.type)
            game->event = e;
    }
    asking_quit_ = false;
    yes_event_ = no_event_ = {};
}

// panel_l_button's room editing on/off (MM_EDIT_ROOM): from the normal mode
// it starts editing, from the edit mode it stops; refused otherwise.
bool App::panel_edit_room(Game &game)
{
    BuildMouse mode = game.world.building.mouse;
    if ((mode != BuildMouse::None && mode != BuildMouse::EditRoom) || interface_.requester() != Requester::None)
        return true;
    game.world.building.mouse = mode == BuildMouse::None ? BuildMouse::EditRoom : BuildMouse::None;
    game.world.building.build.edit_room = 0;
    game.world.building.build.draw_mode = 0;
    return false;
}

// Carries out the panel button pressed this frame, with its sound; `placing`
// says whether an object or a member of staff is being placed.
void App::interface_panel_action(Game &game, PanelAction action, bool placing)
{
    if (action == PanelAction::None)
        return;
    // panel_l_button and panel_r_button refuse every panel click while an
    // edited room empties (MM_EDIT_ROOM_WAIT).
    if (game.world.building.mouse == BuildMouse::EditRoomWait) {
        sounds_.play_sound(kSourceGeneral, kSfxWrong2);
        return;
    }
    bool refused = action == PanelAction::Refused;
    if (action == PanelAction::FurnishCorridor) {
        refused = hire_.placing() || build_tool_busy(game);
        if (!refused)
            furnish_corridor(game);
    } else if (action == PanelAction::HireStaff) {
        refused = placing || build_tool_busy(game);
        if (!refused)
            game.event = {EV_OPEN_HIRE_STAFF};
    } else if (action == PanelAction::BuildRoom) {
        refused = hire_.placing() || build_tool_busy(game);
        if (!refused)
            open_build_room(game);
    } else if (action == PanelAction::EditRoom) {
        refused = panel_edit_room(game);
    } else if (action == PanelAction::TownMap) {
        // Refused while the edit room requester, or the buy objects one
        // other than for the corridor, is open.
        refused = interface_.requester() == Requester::EditRoom ||
                  (interface_.requester() == Requester::BuyObjects && game.world.placing.arg != 1);
        if (!refused)
            open_town_map();
    } else if (int chart = chart_for_panel_action(action); chart >= 0) {
        Event e = open_chart_command(chart, game, &refused);
        if (!refused)
            game.event = e;
    }
    sounds_.play_sound(kSourceGeneral, refused ? kSfxWrong2 : kSfxSelectX);
}

// The mouse on the map (`on_map`): what is being placed follows the
// pointer, and the right button drops what is held or picks up staff.
void App::interface_map_mouse(Game &game, const UiMouse &mouse, bool on_map, bool placing)
{
    int cell = on_map ? map_cell_ : -1;
    placement_mouse(object_ui_, game, cell, mouse, on_map);
    build_mouse(build_ui_, game, cell, pointed_.thing, mouse, on_map);
    if (hire_.placing() && on_map && cell >= 0) {
        update_staff_cursor(hire_, game.world, cell);
        if (mouse.left_pressed) {
            if (Event e = place_staff_click(hire_, game.world, cell); e.type)
                game.event = e;
        } else if (mouse.right_pressed) {
            game.event = hire_staff_cancel(hire_);
        }
    } else if (mouse.right_pressed && on_map && !game.world.rats.fest_level &&
               !advisor_hit(game.advisor, mouse.x, mouse.y) &&
               (interface_.requester() == Requester::None || interface_.requester() == Requester::EditRoom)) {
        if (Event e = interface_map_right(game, mouse, placing); e.type)
            game.event = e;
    }
}

// process_right_button on the map: the watch's patient, the edit-room modes,
// picking up an object or member of staff, or cancelling. Returns the
// command, EV_NONE for none.
Event App::interface_map_right(Game &game, const UiMouse &mouse, bool placing)
{
    BuildMouse mode = game.world.building.mouse;
    if (mode == BuildMouse::EditRoomWait)
        return {EV_CANCEL_EDIT_ROOM, 0, 0, uint16_t(mode)};
    if (mode == BuildMouse::EditRoom || mode == BuildMouse::PickUpObject) {
        object_escape(game);
        return {};
    }
    if (mode != BuildMouse::None || placing || build_tool_busy(game))
        return {};
    if (Event e = interface_watch_right(game, mouse); e.type)
        return e;
    if (game.world.building.build.room)
        return check_object_right_click_in_room_edit(game.world, pointed_.thing);
    if (interface_.window_count() > 0)
        return hire_staff_cancel(hire_);
    if (Event e = pick_up_object_right_click(game.world, pointed_.thing); e.type)
        return e;
    if (Event e = pick_up_staff_click(game.world, pointed_.thing); e.type) {
        sounds_.play_sound(kSourceGeneral, kSfxPickup);
        return e;
    }
    return {};
}

} // namespace th

// The in-game interface, part 2: the map pointer's hover and clicks, the
// advisor and the interface's commands.

#include "app/app.h"
#include "app/app_internal.h"
#include "core/data/gamedata.h"
#include "core/world/icons.h"
#include "core/world/queue.h"
#include "core/world/rats.h"
#include "core/world/vaccine.h"

namespace th
{

namespace
{

// map_hover, but the rat game shows nothing of the people and objects under
// the pointer (process_mouse returns before them).
MapHover game_map_hover(const World &world, uint16_t pointed)
{
    MapHover hover = map_hover(world, pointed);
    if (world.rats.fest_level && !(hover.hit && world.things[hover.thing].type == kThingRat))
        return {};
    return hover;
}

// OBJ_RECEPTION_DESK (include/enums.h).
constexpr uint8_t kObjReceptionDesk = 11;

} // namespace

inline namespace app_detail
{

// Whether the hospital has a receptionist (the announcer's pages wait for
// one).
bool has_receptionist(const World &world)
{
    for (const Thing &t : world.things)
        if (t.type == kThingStaff && t.subtype == kStaffReceptionist)
            return true;
    return false;
}

} // namespace app_detail

// The watch's buttons over the panel (process_left_button): a left click on
// the epidemic's vaccination button toggles the MM_VACCINATE pointer; on the
// opening countdown's it opens the hospital at once (EV_OPEN_HOSPITAL) with
// its fanfare. The game uses watch[14] for both hotspots. True when one took
// the click.
bool App::interface_watch_buttons(Game &game, const UiMouse &mouse)
{
    if (!mouse.left_pressed)
        return false;
    Sprite button = data_->watch().at(14);
    if (mouse.x < kPanelWatchButtonX || mouse.x >= kPanelWatchButtonX + button.width || mouse.y < kPanelWatchButtonY ||
        mouse.y >= kPanelWatchButtonY + button.height)
        return false;
    if (game.world.epidemic.timer > 0) {
        // MM_VACCINATE, with SFX_SELECTX (mouse.cpp:317).
        vaccinate_ = !vaccinate_;
        sounds_.play_sound(kSourceGeneral, kSfxSelectX);
        return true;
    }
    if (game.opening_countdown > 0) {
        game.event = {EV_OPEN_HOSPITAL};
        sounds_.play_sound(kSourceGeneral, kSfxFanfare);
        return true;
    }
    return false;
}

// The watch button's highlight (draw_emergency's othervar.watch_hovered): its
// sound plays once on entering it, when the button is drawn.
void App::interface_watch_hover(Game &game, const UiMouse &mouse)
{
    // The button is drawn only in the epidemic branch or, past an emergency
    // and with the tutorial flag clear, in the opening-countdown branch
    // (draw_emergency, panel.cpp:593).
    bool epidemic = game.world.epidemic.timer > 0;
    bool countdown =
        game.opening_countdown > 0 && !(game.advisor.flags & kAdvisorTutorial) && game.open_countdown >= 13;
    if (game.world.emergency.timer > 0 || (!epidemic && !countdown)) {
        watch_hovered_ = false;
        return;
    }
    Sprite button = data_->watch().at(14);
    bool over_watch = mouse.x >= kPanelWatchButtonX && mouse.x < kPanelWatchButtonX + button.width &&
                      mouse.y >= kPanelWatchButtonY && mouse.y < kPanelWatchButtonY + button.height;
    if (over_watch && !watch_hovered_)
        sounds_.play_sound(kSourceGeneral, kSfxHighlight5);
    watch_hovered_ = over_watch;
}

// process_mouse's MM_NORMAL: what the pointer is over, and a left click
// there (process_left_button) opens its window.
void App::interface_map_hover(Game &game, const UiMouse &mouse, bool on_map, bool placing)
{
    // The bar stays 100 frames after the pointer leaves a thing
    // (othervar.panel_info_timer).
    if (panel_info_timer_ && --panel_info_timer_ == 0)
        panel_info_ = 0;
    interface_watch_hover(game, mouse);
    if (!on_map || placing || build_tool_busy(game) || game.world.building.mouse != BuildMouse::None ||
        interface_.requester() != Requester::None || advisor_hit(game.advisor, mouse.x, mouse.y)) {
        pointer_sprite_ = kPointerDefault;
        return;
    }
    if (popups_.hovered) {
        // Hover action 7: a fax icon, which a click answers.
        pointer_sprite_ = kPointerDefault;
        if (mouse.left_pressed) {
            close_irequesters(interface_);
            game.event = popup_icon_click(popups_);
        }
        return;
    }
    // The watch's buttons handled the click; a left click on a contagious
    // patient queues them for vaccination (EV_TOGGLE_VACCINATION).
    if (interface_watch_buttons(game, mouse))
        return;
    const World::Epidemic &epidemic = game.world.epidemic;
    MapHover hover = game_map_hover(game.world, pointed_.thing);
    if (hover.hit) {
        panel_info_timer_ = 100;
        panel_info_ = hover.info;
        panel_thing_ = hover.thing;
    }
    interface_flag_queue(game, mouse);
    // process_mouse's MM_NORMAL: a rat near the pointer but not aimed at
    // (othervar.near_rat) shows the near pointer, and a click fires a miss.
    if (interface_near_rat(game, mouse, hover))
        return;
    // During a cover-up, hovering a contagious patient shows the vaccination
    // pointer, as process_mouse's MM_NORMAL does; a click on one queues them.
    uint16_t target = 0;
    if (epidemic.active && hover.hit && is_person_highlighted_for_vaccination(game.world, hover.thing, target)) {
        pointer_sprite_ = kPointerVaccinateTarget;
        if (mouse.left_pressed)
            game.event = {EV_TOGGLE_VACCINATION, target, 0, 0};
        return;
    }
    if (vaccinate_ && epidemic.timer > 0) {
        bool highlighted = is_person_highlighted_for_vaccination(game.world, pointed_.thing, target);
        pointer_sprite_ = highlighted ? kPointerVaccinateTarget : kPointerVaccinate;
        if (mouse.left_pressed && highlighted)
            game.event = {EV_TOGGLE_VACCINATION, target, 0, 0};
        return;
    }
    if (epidemic.timer == 0)
        vaccinate_ = false;
    interface_map_hover_click(game, mouse, hover);
}

// flag_all_queue_members from process_no_button (src/mouse.cpp): with no
// mouse button down in MM_NORMAL, a room of the player's under the pointer
// shows the icon on everyone in its queue, as does a reception desk or
// receptionist (the room of the panel's thing); SFX_HLIGHTP2 plays when the
// queue was not flagged on the frame before.
void App::interface_flag_queue(Game &game, const UiMouse &mouse)
{
    World &world = game.world;
    if (mouse.left_held || mouse.right_held || mouse.left_pressed || mouse.right_pressed || vaccinate_ ||
        world.rats.fest_level)
        return;
    uint16_t room = map_cell_ >= 0 ? room_at(world, uint16_t(map_cell_)) : 0;
    if (room && world.rooms[room % kRoomCount].owner != 0)
        room = 0;
    if (!room) {
        const Thing *hovered = pointed_.thing < world.things.size() ? &world.things[pointed_.thing] : nullptr;
        if (pointed_.thing && hovered && hovered->owner == 0 &&
            ((hovered->type == kThingObject && hovered->subtype == kObjReceptionDesk) ||
             (hovered->type == kThingStaff && hovered->subtype == kStaffReceptionist)) &&
            panel_thing_ < world.things.size())
            room = world.things[panel_thing_].room;
    }
    if (flag_all_queue_members(world, game.anims(), room) &&
        (frame_ - 1 != last_flag_frame_ || last_flag_room_ != room))
        sounds_.play_sound(kSourceGeneral, kSfxHighlightP2);
    last_flag_room_ = room;
    last_flag_frame_ = frame_;
}

// interface_map_hover's rat near the pointer but not aimed at: the near
// pointer and, on a click, EV_SHOOT_RAT at nothing (a miss), as
// process_mouse's MM_NORMAL. False if no rat is near or one is aimed at.
bool App::interface_near_rat(Game &game, const UiMouse &mouse, const MapHover &hover)
{
    if (!near_rat_)
        return false;
    if (hover.hit && hover.thing < game.world.things.size() && game.world.things[hover.thing].type == kThingRat)
        return false;
    pointer_sprite_ = kPointerRatNear;
    if (mouse.left_pressed) {
        game.event = map_left_click(game.world, hover, interface_, map_cell_, true);
        arm_rat_shot(game.world, game.event);
    }
    return true;
}

// interface_map_hover's pointer and left click for the thing `hover` is over:
// its pointer, its status icon and, on a click, map_left_click's command.
void App::interface_map_hover_click(Game &game, const UiMouse &mouse, const MapHover &hover)
{
    if (hover.pointer != kPointerKeep)
        pointer_sprite_ = hover.pointer;
    // peep_icon (src/mouse.cpp): passing the mouse over a person announces
    // their pending status icon.
    if (hover.hit && hover.thing && hover.thing < game.world.things.size()) {
        Thing &h = game.world.things[hover.thing];
        if ((h.type == kThingPatient || h.type == kThingStaff) && h.owner == 0)
            peep_icon(game.world, h);
    }
    if (mouse.left_pressed)
        if (Event e = map_left_click(game.world, hover, interface_, map_cell_); e.type) {
            arm_rat_shot(game.world, e);
            game.event = e;
        }
}

// The level a win accepted goes to: the bonus level a good year unlocked,
// which then goes on to `next` (winlose.cpp's accept button).
int App::level_after_win(const Game &game, int next)
{
    if (!game.progress.bonus_level)
        return next;
    level_after_bonus_ = next;
    return game.progress.bonus_level;
}

void App::finish_rat_level()
{
    // rat_fest_control hands level_number level_after_bonus; the area
    // screen takes level 0 as 1.
    int next = std::max(level_after_bonus_, 1);
    load_level(next);
    help_.set_page(-1, -1, level_briefing(data_->text(), next));
}

// After interface_map_hover while the queue window is open: req7_overdraw's
// panel information and MM_MOVE_QUEUE_PERSON pointer, and the door hotspot's
// scroll to the room.
void App::queue_window_after_frame(Game &game)
{
    panel_info_ = queue_.panel_info;
    panel_thing_ = queue_.panel_thing;
    if (game.world.building.mouse == BuildMouse::MoveQueuePerson)
        pointer_sprite_ = kPointerMoveQueuePerson;
    if (queue_.goto_cell >= 0) {
        Point p = cell_to_world(queue_.goto_cell % kMapSize, queue_.goto_cell / kMapSize);
        if (!beyond_scroll_limit(p, scroll_min_, scroll_max_))
            camera_ = {p.x - surface_.width() / 2, p.y - surface_.height() / 2};
    }
}

// The advisor: a right click on him or his bubble dismisses him; then his
// frame, which reads the command about to be carried out.
void App::interface_advisor(Game &game, const UiMouse &mouse, bool input, bool paused)
{
    if (mouse.right_pressed && input && mouse.y < panel_top(*data_) && advisor_hit(game.advisor, mouse.x, mouse.y))
        game.advisor.dismiss = true;
    AdvisorInputs in;
    in.game_turn = game.turn_count;
    in.paused = paused;
    in.requester_open = interface_.requester() != Requester::None && interface_.requester() != Requester::EditRoom;
    in.level = game.hospital_level();
    in.month = game.date.month;
    in.open = game.open;
    in.balance = game.balance;
    in.reputation = game.reputation();
    in.finance = &game.finance;
    in.settings = &game.settings();
    in.world = &game.world;
    in.visitors = game.arrivals.hospitals[0].visitors;
    in.quake_phase = game.disasters.quake_phase;
    in.win_progress = game.progress.win_average;
    in.lose_progress = game.progress.lose_progress;
    in.text = &data_->text();
    uint8_t ambient = 0;
    in.ambient = &ambient;
    int help_page = -1;
    in.help_page = &help_page;
    in.pending = game.event;
    in.last = last_event_;
    in.staff_place_ok = hire_.place_ok;
    if (game.world.placing.placing)
        in.object_place_ok = game.world.placing.ok;
    advisor_frame(game.advisor, in, game.anims(), sounds_.safeseed);
    if (help_page >= 0)
        help_.set_page(-1, -1, level_briefing(data_->text(), help_page));
    if (ambient)
        sounds_.play_ambient(ambient, has_receptionist(game.world), paused);
    last_event_ = game.event;
}

// The command, as process_turn: the interface's part, then the game's; then
// what the windows asked for besides (`info`).
void App::interface_command(Game &game, const InfoWindowResult &info)
{
    run_command(game);
    if (info.event.type == EV_OPEN_YES_NO) {
        std::string q = info.question_text.empty() ? std::string(data_->text().at(TextSection::YesNo, info.question))
                                                   : info.question_text;
        interface_.open_yes_no(q);
        yes_event_ = info.yes;
        no_event_ = info.no;
    }
    if (info.casebook) {
        // req3 opens the casebook itself, with no command; EV_OPEN_SCREEN is
        // refused while an edited room empties (io.cpp:824).
        if (game.world.building.mouse == BuildMouse::EditRoomWait) {
            sounds_.play_sound(kSourceGeneral, kSfxWrong2);
        } else {
            charts_.selected_illness = info.casebook;
            chart_command(charts_, {EV_OPEN_SCREEN, uint16_t(kChartCasebook)});
        }
    }
}

} // namespace th

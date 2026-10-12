// The info windows and the panel's popup icons (core/ui/info_windows.h,
// core/ui/popups.h).

#include "common.h"

#include "core/audio/sounds.h"
#include "core/data/gamedata.h"
#include "core/data/level.h"
#include "core/data/sprites.h"
#include "core/render/panel.h"
#include "core/render/surface.h"
#include "core/ui/charts.h"
#include "core/ui/hire_staff.h"
#include "core/ui/info_windows.h"
#include "core/ui/layer.h"
#include "core/ui/popups.h"
#include "core/ui/queue_window.h"
#include "core/ui/town_map.h"
#include "core/ui/windows.h"
#include "core/world/epidemic.h"
#include "core/world/events.h"
#include "core/world/finance.h"
#include "core/world/mayor.h"
#include "core/world/popups.h"
#include "core/world/rooms.h"
#include "core/world/savegame.h"
#include "core/world/snapshot.h"
#include "core/world/staff.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace
{

// Presses and releases the left button on button `button` (counted from 1)
// of window `window`; the frame whose callback ran, or the release's.
th::InterfaceEvents click_button(th::Interface &ui, int window, int button)
{
    const th::Window &win = ui.window(window);
    const th::WindowButton &b = win.buttons[size_t(button - 1)];
    th::UiMouse m;
    m.x = win.x + b.x + b.width / 2;
    m.y = win.y + b.y + b.height / 2;
    m.left_pressed = m.left_held = true;
    th::InterfaceEvents ev = ui.frame(m);
    if (ev.triggered_button >= 0)
        return ev;
    m.left_pressed = m.left_held = false;
    m.left_released = true;
    return ui.frame(m);
}

// The mouse at (x, y) of window `window`.
th::UiMouse window_mouse(const th::Interface &ui, int window, int x, int y)
{
    th::UiMouse m;
    m.x = ui.window(window).x + x;
    m.y = ui.window(window).y + y;
    return m;
}

constexpr int kPatient = int(th::Requester::Patient), kStaff = int(th::Requester::Staff),
              kHandyman = int(th::Requester::Handyman);

// The info windows on level 1 with a doctor, a handyman and two patients.
struct InfoFixture {
    QueueFixture f;
    th::RequesterArt art;
    th::HireStaffArt hire_art;
    th::Interface ui;
    th::InfoWindows info;
    th::UiLayer layer;
    th::ScrollArea scroll = {};
    uint16_t doctor = 0, handyman = 0, patient = 0, patient2 = 0;

    // Whether the level started and all four could be put on the map.
    bool start(const char *dir)
    {
        std::string error;
        CHECK(f.start(dir) && art.load(dir, &error) && hire_art.load(dir, &error));
        if (!f.level)
            return false;
        start_rooms(f);
        th::Game &g = *f.game;
        const int start = f.level->camera_start[0];
        scroll = {f.level->scroll_min, f.level->scroll_max};
        doctor = hire_near(f, th::kStaffDoctor, start);
        handyman = hire_near(f, th::kStaffHandyman, start + 2 * th::kMapSize);
        patient = g.add_walker(start + 4 * th::kMapSize);
        patient2 = g.add_walker(start + 5 * th::kMapSize);
        CHECK(doctor && handyman && patient && patient2);
        layer.resize(th::kUiWidth, th::kUiHeight);
        return doctor && handyman && patient && patient2;
    }

    th::World &world() { return f.game->world; }

    th::InfoWindowResult frame(const th::InterfaceEvents &ev, const th::UiMouse &m = {})
    {
        return th::info_window_frame(info, ui, ev, m, {world(), f.gd.text(), scroll, f.game->balance});
    }

    void draw_windows() { th::draw_info_windows(layer, info, ui, {art, hire_art, f.gd.text()}, world(), 0xc8); }

    uint8_t pixel(int window, int x, int y)
    {
        return layer.pixels().row(ui.window(window).y + y)[ui.window(window).x + x];
    }
};

// Hovering gives the person and their pointer, nothing off them; a click
// opens the window, closing another, and draws it undiagnosed: the casebook
// and guess cure buttons covered, the graph drawn.
void check_patient_window_opens(InfoFixture &t)
{
    th::World &w = t.world();
    th::MapHover h = th::map_hover(w, t.patient);
    CHECK(h.hit && h.thing == t.patient && h.pointer == th::kPointerPatient);
    h = th::map_hover(w, t.doctor);
    CHECK(h.hit && h.thing == t.doctor && h.pointer == th::kPointerStaff);
    CHECK(!th::map_hover(w, 0).hit && th::map_hover(w, 0).pointer == th::kPointerDefault);

    t.ui.open_window(kStaff);
    th::Event e = th::map_left_click(w, th::map_hover(w, t.patient), t.ui);
    CHECK(e.type == th::EV_OPEN_THING_WINDOW && e.value == t.patient && t.ui.window_count() == 0);
    th::info_window_event(t.info, t.ui, w, e);
    CHECK(t.ui.is_open(kPatient) && t.ui.window_count() == 1 && t.info.patient == t.patient && t.info.show_graph);
    CHECK(t.ui.requester() == th::Requester::None);

    t.layer.clear();
    th::draw_interface(t.layer, t.ui, t.art);
    t.draw_windows();
    CHECK(t.pixel(kPatient, 167 + 10, 164 + 10) == 0x46);
    CHECK(t.pixel(kPatient, 167 + 10, 115 + 10) == 0x46);
    int graph = 0;
    for (int y = 28; y <= 101; y++)
        for (int x = 99; x <= 198; x++)
            graph += t.pixel(kPatient, x, y) == 0xc8;
    CHECK(graph > 50);
}

// Send home; guess cure only once diagnosed, with a treatment, its illness
// not the one known.
void check_patient_buttons(InfoFixture &t)
{
    th::InterfaceEvents ev = click_button(t.ui, kPatient, 9);
    CHECK(ev.triggered_window == kPatient && ev.triggered_button == 9);
    th::InfoWindowResult r = t.frame(ev);
    CHECK(r.event.type == th::EV_PERSON_GO_HOME && r.event.value == t.patient);
    CHECK(r.sounds.size() == 1 && r.sounds[0] == th::kSfxSack);
    r = t.frame(click_button(t.ui, kPatient, 11));
    CHECK(r.event.type == th::EV_NONE);
    th::Thing &p = t.world().things[t.patient];
    p.diagnosis = 1000;
    p.known_illness = 1;
    p.illness = p.treatment = 5;
    r = t.frame(click_button(t.ui, kPatient, 11));
    CHECK(r.event.type == th::EV_FORCE_CURE && r.event.value == t.patient);
    r = t.frame(click_button(t.ui, kPatient, 8));
    CHECK(r.event.type == th::EV_NONE && r.casebook == 5);
}

// The treatments in place of the graph, then the next patient; then the
// window closed.
void check_patient_treatments_and_next(InfoFixture &t)
{
    th::World &w = t.world();
    th::UiMouse m = window_mouse(t.ui, kPatient, 78 + 10, 25 + 10);
    m.left_pressed = m.left_held = true;
    t.frame({}, m);
    CHECK(!t.info.show_graph && t.info.toggle_delay == 5);
    t.frame({}, m);
    CHECK(!t.info.show_graph);
    t.layer.clear();
    t.draw_windows();
    CHECK(t.pixel(kPatient, 78 + 2, 25 + 2) == 0xa9);
    m = window_mouse(t.ui, kPatient, 44 + 10, 211 + 10);
    m.left_pressed = m.left_held = true;
    th::InfoWindowResult r = t.frame({}, m);
    CHECK(r.centre_on == t.patient);
    m.left_pressed = m.left_held = false;
    m.right_pressed = m.right_held = true;
    r = t.frame({}, m);
    CHECK(t.info.patient == th::find_next_thing(w, t.patient, th::kThingPatient, -1, -1, t.scroll));
    CHECK(t.info.patient != t.patient && r.sounds.size() == 1);
    r = t.frame({}, m);
    CHECK(r.sounds.empty());

    r = t.frame(click_button(t.ui, kPatient, 6));
    CHECK(r.event.type == th::EV_CLOSE_QUERY && r.event.value == 1);
    th::info_window_event(t.info, t.ui, w, r.event);
    CHECK(t.ui.window_count() == 0);
}

// The staff window drawn: the face's border; the qualifications covered for
// all but doctors.
void check_staff_window_drawn(InfoFixture &t)
{
    th::World &w = t.world();
    t.layer.clear();
    th::draw_interface(t.layer, t.ui, t.art);
    t.draw_windows();
    CHECK(t.pixel(kStaff, 47, 134 - 1) == 0x9c);
    auto covered = [&] {
        int n = 0;
        for (int y = 141; y < 141 + 45; y++)
            for (int x = 41; x < 41 + 176; x++)
                n += t.pixel(kStaff, x, y) == 0x7a;
        return n;
    };
    CHECK(covered() < 176 * 45);
    w.things[t.doctor].subtype = th::kStaffNurse;
    t.draw_windows();
    CHECK(covered() == 176 * 45);
    w.things[t.doctor].subtype = th::kStaffDoctor;
}

// The staff window: sacking asks first; no opens the window again; the
// staff list and picking up from its buttons.
void check_staff_window(InfoFixture &t)
{
    th::World &w = t.world();
    th::info_window_event(t.info, t.ui, w, th::map_left_click(w, th::map_hover(w, t.doctor), t.ui));
    CHECK(t.ui.is_open(kStaff) && t.info.staff == t.doctor);
    check_staff_window_drawn(t);
    th::InfoWindowResult r = t.frame(click_button(t.ui, kStaff, 16));
    CHECK(r.event.type == th::EV_OPEN_YES_NO && r.question == 5 && t.ui.window_count() == 0);
    CHECK(r.yes.type == th::EV_SACK_STAFF && r.yes.value == t.doctor && r.yes.arg == 0);
    CHECK(r.no.type == th::EV_CANCEL_SACK && r.no.value == t.doctor);
    th::info_window_event(t.info, t.ui, w, r.no);
    CHECK(t.ui.is_open(kStaff) && t.info.staff == t.doctor);
    th::UiMouse m = window_mouse(t.ui, kStaff, 47 + 10, 53 + 10);
    m.left_pressed = m.left_held = true;
    r = t.frame({}, m);
    CHECK(r.event.type == th::EV_OPEN_SCREEN && r.event.value == 5 && r.slist_staff == t.doctor);
    r = t.frame(click_button(t.ui, kStaff, 17));
    CHECK(r.event.type == th::EV_PICK_UP_STAFF && r.event.value == t.doctor);
    th::info_window_event(t.info, t.ui, w, r.event);
    CHECK(t.ui.window_count() == 0);
}

// The handyman's window: his priorities' buttons act while held.
void check_handyman_window(InfoFixture &t)
{
    th::info_window_event(t.info, t.ui, t.world(), {th::EV_OPEN_THING_WINDOW, t.handyman});
    CHECK(t.ui.is_open(kHandyman) && !t.ui.is_open(kStaff) && t.info.staff == t.handyman);
    for (int button : {29, 30, 31}) {
        th::InterfaceEvents ev = click_button(t.ui, kHandyman, button);
        CHECK(ev.triggered_button == button);
        th::InfoWindowResult r = t.frame(ev);
        CHECK(r.event.type == th::EV_ADJUST_HANDYMAN_VALUES && r.event.value == t.handyman && r.event.arg == button);
        t.frame({});
    }
    t.layer.clear();
    t.draw_windows();
    CHECK(t.pixel(kHandyman, 55, 202) != 0);
    th::InfoWindowResult r = t.frame(click_button(t.ui, kHandyman, 26));
    CHECK(r.event.type == th::EV_CLOSE_QUERY && r.event.value == 2);
}

// The window closes once its person is gone; another player's people open
// nothing.
void check_window_follows_person(InfoFixture &t)
{
    th::World &w = t.world();
    th::info_window_event(t.info, t.ui, w, {th::EV_OPEN_THING_WINDOW, t.patient2});
    CHECK(t.ui.is_open(kPatient));
    t.frame({});
    CHECK(t.ui.is_open(kPatient));
    th::Rect box;
    th::Point camera;
    CHECK(th::info_view(t.info, t.ui, w, box, camera) && box.w == 77 && box.h == 77);
    w.things[t.patient2].type = th::kThingFree;
    t.frame({});
    CHECK(t.ui.window_count() == 0 && !th::info_view(t.info, t.ui, w, box, camera));
    w.things[t.patient2].type = th::kThingPatient;

    w.things[t.patient2].owner = 1;
    CHECK(th::map_left_click(w, th::map_hover(w, t.patient2), t.ui).type == th::EV_NONE);
    th::info_window_event(t.info, t.ui, w, {th::EV_OPEN_THING_WINDOW, t.patient2});
    CHECK(t.ui.window_count() == 0);
}

// Hovering a rat gives the aim pointer; a click fires at it (a hit). With
// nothing under the pointer but a rat near (othervar.near_rat), a click
// fires a miss; without near_rat it does nothing (src/mouse.cpp MM_NORMAL,
// src/io.cpp EV_SHOOT_RAT).
void check_rat_click(InfoFixture &t)
{
    th::World &w = t.world();
    uint16_t rat = 0;
    for (uint16_t id = 1; id < w.things.size() && !rat; id++)
        if (w.things[id].type == th::kThingFree)
            rat = id;
    CHECK(rat != 0);
    th::Thing &r = w.things[rat];
    r.type = th::kThingRat;
    r.owner = 0;
    th::MapHover h = th::map_hover(w, rat);
    CHECK(h.hit && h.thing == rat && h.pointer == th::kPointerRatAim);
    th::Event e = th::map_left_click(w, h, t.ui);
    CHECK(e.type == th::EV_SHOOT_RAT && e.value == rat && e.arg == 0x0100);
    e = th::map_left_click(w, th::MapHover{}, t.ui, -1, true);
    CHECK(e.type == th::EV_SHOOT_RAT && e.value == 0 && e.arg == 0);
    CHECK(th::map_left_click(w, th::MapHover{}, t.ui, -1, false).type == th::EV_NONE);
    r.type = th::kThingFree;
}

} // namespace

// The patient, staff and handyman windows (src/req3.cpp, src/req4.cpp):
// what hovering and clicking a person on the map gives, the windows the
// commands open and close, their buttons' commands from clicks where the
// art puts them, their own clicks (the treatments, the next person, the
// staff list), closing once their person is gone, and their drawing.
void test_info_windows(const char *dir)
{
    auto t = std::make_unique<InfoFixture>();
    if (!t->start(dir))
        return;
    check_patient_window_opens(*t);
    check_patient_buttons(*t);
    check_patient_treatments_and_next(*t);
    check_staff_window(*t);
    check_handyman_window(*t);
    check_window_follows_person(*t);
    check_rat_click(*t);
}

namespace
{

// Game frames of the fax icons, the mouse at `m`, until `done` or `limit`
// frames; the frames run.
int popup_frames(th::PopupIcons &icons, th::World &w, const th::GameData &gd, const th::UiMouse &m, int limit,
                 const std::function<bool()> &done)
{
    for (int i = 0; i < limit; i++) {
        if (done())
            return i;
        th::popup_icons_frame(icons, w, m, gd, 0.04);
    }
    return limit;
}

bool popups_settled(const th::World &w)
{
    for (size_t kind = 1; kind < th::kPopupIcons.size(); kind++)
        if (uint16_t head = w.first_call[0][kind]; head && w.calls[head].slide_step)
            return false;
    return true;
}

// The kinds of the icons level with their slots, left to right.
std::vector<int> level_kinds(const th::PopupIcons &icons)
{
    std::vector<std::pair<int, int>> by_x;
    for (uint8_t i = 0; i < icons.level_count; i++)
        by_x.emplace_back(icons.level[i].x, icons.level[i].kind);
    std::sort(by_x.begin(), by_x.end());
    std::vector<int> kinds;
    for (auto [x, kind] : by_x)
        kinds.push_back(kind);
    return kinds;
}

// The icons settled, a game frame on: those level with their slots.
void popups_settle(th::PopupIcons &icons, th::World &w, const th::GameData &gd)
{
    th::UiMouse away;
    popup_frames(icons, w, gd, away, 200, [&] { return popups_settled(w); });
    th::popup_icons_frame(icons, w, away, gd, 0.04);
}

// Three calls of three kinds and a second of the first kind: the three
// rise and slide into their slots, the fourth has none.
void check_popups_slide(th::PopupIcons &icons, th::World &w, const th::GameData &gd)
{
    uint16_t first = th::make_a_playercall(w, 4, 10, 0, 0);
    th::make_a_playercall(w, 7, 22, 0xf, 0);
    th::make_a_playercall(w, 8, 11, 0, 0);
    uint16_t second = th::make_a_playercall(w, 4, 12, 0, 0);
    CHECK(first == 1 && second == 4 && w.popups.new_faxes == 4);
    CHECK(w.first_call[0][4] == 1 && w.first_call[0][7] == 2 && w.first_call[0][8] == 3);
    CHECK(w.calls[1].slot == 0 && w.calls[2].slot == 1 && w.calls[3].slot == 2 && w.calls[1].slide_step == 1);
    CHECK(w.popups.shutter_state == 2);
    th::UiMouse away;
    th::popup_icons_frame(icons, w, away, gd, 0.04);
    CHECK(w.popups.pos[3] == 0 && icons.rising_count == 3 && icons.level_count == 0);
    CHECK(icons.rising[0].y == 29 && icons.rising[0].x == th::kPopupSlotX[0] + 174 && w.calls[1].slide_step == 1);
    int frames = popup_frames(icons, w, gd, away, 200, [&] { return popups_settled(w); });
    CHECK(frames > 13 && frames < 100);
    th::popup_icons_frame(icons, w, away, gd, 0.04);
    CHECK(icons.rising_count == 0 && level_kinds(icons) == std::vector<int>({4, 7, 8}));
    for (uint8_t i = 0; i < icons.level_count; i++)
        CHECK(icons.level[i].x == th::kPopupSlotX[w.calls[w.first_call[0][icons.level[i].kind]].slot]);
}

// The mouse over the second icon: it answers the new illness's call; just
// above the icons, nothing.
void check_popups_hover(th::PopupIcons &icons, th::World &w, const th::GameData &gd)
{
    th::UiMouse over;
    over.x = th::kPopupSlotX[1] + 2;
    over.y = icons.level[0].hover_y + 2;
    th::popup_icons_frame(icons, w, over, gd, 0.0);
    CHECK(icons.hovered == 7);
    th::Event click = th::popup_icon_click(icons);
    CHECK(click.type == th::EV_ANSWER_PLAYERCALL && click.value == 7 && click.arg == 0);
    over.y = icons.level[0].hover_y - 1;
    th::popup_icons_frame(icons, w, over, gd, 0.0);
    CHECK(icons.hovered == 0 && th::popup_icon_click(icons).type == th::EV_NONE);
}

// Drawn in their window above the panel's left end, which is redrawn.
void check_popups_drawn(const th::PopupIcons &icons, const th::GameData &gd)
{
    th::UiLayer layer;
    layer.resize(th::kUiWidth, th::kUiHeight);
    layer.clear();
    th::draw_popup_icons(layer, icons, gd, {}, nullptr);
    std::vector<uint32_t> argb = layer.to_argb(gd.palette());
    int top = th::kUiHeight, right = 0;
    for (int y = 0; y < th::panel_top(gd); y++)
        for (int x = 0; x < th::kUiWidth; x++)
            if (argb[size_t(y * th::kUiWidth + x)]) {
                top = std::min(top, y);
                right = std::max(right, x);
            }
    CHECK(top == icons.level[0].hover_y + th::kPopupWindowDy);
    CHECK(right == th::kPopupSlotX[1] + gd.panel().at(th::kPopupIcons[4]).width - 1);
    CHECK(argb[size_t((th::kUiHeight - 1) * th::kUiWidth + 10)] != 0);
}

} // namespace

// The panel's fax icons: a call of each kind takes the next slot and rises
// from behind the panel, held until the message bar's shutter has closed,
// then slides left into its slot; a second call of a kind shows no icon of
// its own. The icon under the mouse answers its kind's first call. As
// calls are answered or expire the icons close up, the next call of a kind
// taking over its icon.
void test_panel_popups(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    th::World &w = f.game->world;
    th::PopupIcons icons;
    check_popups_slide(icons, w, f.gd);
    check_popups_hover(icons, w, f.gd);
    // The first call answered: the second of its kind takes its icon, at the
    // end, the others closing up.
    CHECK(th::get_playercall(w, 4, 1, 0) == 10 && w.first_call[0][4] == 4);
    popups_settle(icons, w, f.gd);
    CHECK(level_kinds(icons) == std::vector<int>({7, 8, 4}));
    // The new illness's call expires after 15 days.
    for (int day = 0; day < 15; day++)
        th::expire_playercalls(w);
    CHECK(w.first_call[0][7] == 0);
    popups_settle(icons, w, f.gd);
    CHECK(level_kinds(icons) == std::vector<int>({8, 4}));
    check_popups_drawn(icons, f.gd);
}

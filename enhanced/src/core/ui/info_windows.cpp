#include "core/ui/info_windows.h"

#include "core/audio/sounds.h"
#include "core/render/text.h"
#include "core/ui/charts.h"
#include "core/ui/hire_staff.h"
#include "core/world/build.h"
#include "core/world/object_tables.h"
#include "core/world/queue.h"
#include "core/world/research.h"
#include "core/world/staff.h"
#include "core/world/world.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace th
{

namespace
{

constexpr int kPatientWindow = int(Requester::Patient);
constexpr int kStaffWindow = int(Requester::Staff);
constexpr int kHandymanWindow = int(Requester::Handyman);
constexpr int kMachineWindow = int(Requester::Machine);

// hires_requester_buttons (src/reqbut.cpp), counted from 1 in each window's
// list: [0xd] to [0x11] the patient window's close, casebook, send home,
// queue and guess cure; [0x26], [0x2a] and [0x2b] the handyman window's
// close, sack and pick up ([0x27] to [0x29], his priorities, are in
// core/world/staff.h); [0x2c] to [0x2e] the staff window's close, sack and
// pick up.
constexpr int kPatientClose = 6;
constexpr int kPatientCasebook = 8;
constexpr int kPatientHome = 9;
constexpr int kPatientQueue = 10;
constexpr int kPatientCure = 11;
constexpr int kHandymanClose = 26;
constexpr int kHandymanSack = 27;
constexpr int kHandymanPickUp = 28;
constexpr int kStaffClose = 15;
constexpr int kStaffSack = 16;
constexpr int kStaffPickUp = 17;
constexpr int kMachineClose = 12;
constexpr int kMachineHandyman = 13;
constexpr int kMachineReplace = 14;

// The yes/no requester's question asking to replace a machine
// (yesno_text[2], its arguments the object's name then its cost).
constexpr int kReplaceQuestion = 2;

// The yes/no requester's question asking to sack (yesno_text[5]).
constexpr int kSackQuestion = 5;
constexpr uint8_t kSfxCamClick = 0xbb;

// The game's static gbv.diagnosis_complete (src/array.cpp).
constexpr int kDiagnosisComplete = 1000;

// Object kinds (OBJ_*) and staff orders (ORD_*) the mouse tests.
constexpr uint8_t kObjReceptionDesk = 11;
constexpr uint8_t kObjSwingDoor = 52;
constexpr uint8_t kObjSwingDoor2 = 53;
constexpr uint8_t kOrderResign = 0x20;
// objects[].flags bit 1: hovering shows the panel's object information.
constexpr uint8_t kObjectFlagHoverInfo = 2;

// HiresBoxes and HiresCoords (src/coords.cpp), relative to the window, as
// req3_overdraw reads them (ReqLayout): the boxes covering the casebook,
// queue, guess cure and send home buttons; the patient's view (clicked) and
// the treatments' box; the health graph's left, bottom, width and height;
// the happiness, thirst and warmth gauges and their length.
constexpr Rect kCasebookBox = {167, 164, 50, 52};
constexpr Rect kQueueBox = {183, 77, 31, 38};
constexpr Rect kCureBox = {167, 115, 48, 49};
constexpr Rect kHomeBox = {145, 214, 70, 70};
constexpr Rect kPatientViewBox = {44, 211, 98, 83};
constexpr Rect kDiagBox = {78, 25, 96, 85};
constexpr Rect kGraph = {99, 101, 99, 73};
constexpr Point kPatientHappinessAt = {99, 128};
constexpr Point kRefreshmentAt = {99, 156};
constexpr Point kWarmthAt = {99, 185};
constexpr int kPatientGaugeLen = 41;

// The same, as req4_overdraw reads them: the name, the face's bottom left,
// the wage (a handyman's lower), the happiness, tiredness and ability
// gauges and their lengths; the qualifications (HiresBoxes[0x1a-0x1f]:
// junior, doctor, consultant, surgeon, psychiatrist, researcher) and the
// box covering them for all but doctors; the views (clicked) of a member of
// staff and of a handyman; the face (clicked for the staff list); the
// handyman's priority gauges (litter, plants, repairs) and their length.
constexpr Point kNameAt = {54, 29};
constexpr Point kFaceAt = {47, 134};
constexpr Point kWageAt = {147, 201};
constexpr Point kHandymanWageAt = {147, 228};
constexpr Point kHappinessAt = {151, 58};
constexpr Point kTirednessAt = {151, 90};
constexpr Point kAbilityAt = {151, 122};
constexpr int kStaffGaugeLen = 41;
constexpr int kAbilityGaugeLen = 40;
constexpr Point kSkillAt[6] = {{51, 175}, {73, 175}, {103, 175}, {156, 150}, {167, 161}, {190, 159}};
constexpr Rect kSkillCover = {41, 141, 176, 45};
constexpr Rect kStaffViewBox = {14, 206, 98, 83};
constexpr Rect kHandymanViewBox = {15, 234, 96, 82};
constexpr Rect kFaceBox = {47, 53, 70, 80};
constexpr int kPriorityX[3] = {55, 111, 167};
constexpr int kPriorityY = 202;
constexpr int kPriorityLen = 40;

// HiresCoords and HiresBoxes (src/coords.cpp), relative to the window, as
// req5_overdraw reads them: the machine's name and the room's patients seen,
// its strength bar (HiresBoxes[0x53]) and the view (clicked) of the machine
// (HiresBoxes[0x54]).
constexpr Point kMachineNameAt = {68, 29};
constexpr Point kMachineCountAt = {102, 62};
constexpr Rect kMachineUsageBar = {97, 100, 39, 0};
constexpr Rect kMachineViewBox = {59, 22, 120, 20};

// hires_requester_sprites[4]: the segment req5_draw_machine_usage_bar
// repeats.
constexpr int kUsageBarSprite = 5;

// HiresBoxes read as Rect (x0, y0, x1, y1) for each window's view of its
// person (EV_OPEN_THING_WINDOW's open_new_window): [0x20] staff, [0x21]
// handyman, [0x22] patient.
constexpr Rect kStaffView = {28, 212, 105 - 28, 289 - 212};
constexpr Rect kHandymanView = {28, 239, 105 - 28, 316 - 239};
constexpr Rect kPatientView = {57, 216, 134 - 57, 293 - 216};

// Sprites of the windows' sheets (hires_requester_sprites, src/reqbut.cpp):
// [0] happiness, [1] tiredness, [2] ability, [3] the handyman's priorities,
// [10] a qualification's tick, [11-13] surgeon, psychiatrist and
// researcher, [0x10] thirst, [0x11] warmth.
constexpr int kHappinessSprite = 1;
constexpr int kTirednessSprite = 2;
constexpr int kAbilitySprite = 3;
constexpr int kPrioritySprite = 4;
constexpr int kTickSprite = 0x15b;
constexpr int kSurgeonSprite = 0x158;
constexpr int kPsychiatristSprite = 0x159;
constexpr int kResearcherSprite = 0x15a;
constexpr int kRefreshmentSprite = 0x15f;
constexpr int kWarmthSprite = 0x15d;

// The colours LbDrawBox fills with: unavailable buttons, the treatments'
// background and the qualifications' cover.
constexpr uint8_t kGreyColour = 0x46;
constexpr uint8_t kDiagColour = 0xa9;
constexpr uint8_t kCoverColour = 0x7a;

// The info bar's "cured" (info_bar_text[8]).
constexpr int kCuredText = 8;

// How far above a person's feet the views centred on them centre: about
// half a person's height, so that they are seen whole. The game's own
// offsets (process_tracking_windows' track_offset_x and _y) are not ported.
constexpr int kTrackLift = 24;

bool over(int mx, int my, Point at, Rect box)
{
    return mx >= at.x + box.x && mx < at.x + box.x + box.w && my >= at.y + box.y && my < at.y + box.y + box.h;
}

// A language string with its arguments formatted in, as the game's sprintf.
template <typename... Args> std::string format(std::string_view fmt, Args... args)
{
    std::string f(fmt);
    char buf[200];
    std::snprintf(buf, sizeof buf, f.c_str(), args...);
    return buf;
}

// is_thing_paged (src/req5.cpp): whether thing `id` is among the pager's
// requests.
bool is_thing_paged(const World &world, uint16_t id)
{
    const World::ThingPager &pager = world.repair_pager;
    for (int i = pager.count - 1; i >= 0; i--)
        if (pager.pages[size_t(i)].id == id)
            return true;
    return false;
}

const Thing *thing_at(const World &world, uint16_t id)
{
    return id < world.things.size() ? &world.things[id] : nullptr;
}

// The person thing `id`'s window shows, if of type `type`.
const Thing *shown(const World &world, uint16_t id, uint8_t type)
{
    const Thing *t = thing_at(world, id);
    return t && id && t->type == type ? t : nullptr;
}

// PARTNER (src/mouse.cpp): the object or door person `t` (thing `id`) is
// using, 0 for none.
uint16_t partner(const World &world, const Thing &t, uint16_t id)
{
    const Thing *o = thing_at(world, t.object);
    if (!t.object || !o || (o->type != kThingObject && o->type != kThingDoor) || o->user != id)
        return 0;
    return t.object;
}

// How draw_gauge fills a gauge: the value clamped to `min`-`max`, as a
// part of `range`, from the left (`up`), else the rest.
struct GaugeScale {
    long max;
    long min;
    long range;
    bool up;
};

// draw_gauge (0x73fbc): `value` on `scale` over `len` pixels from `at`,
// drawn by repeating `spr`.
void draw_gauge(UiLayer &layer, long value, Point at, int len, const Sprite &spr, GaugeScale scale)
{
    value = std::clamp(value, scale.min, scale.max);
    long filled = scale.up ? len * value / scale.range : len - len * value / scale.range;
    for (long i = 0; i < filled; i++)
        layer.draw_sprite(at.x + int(i), at.y, spr);
}

// LbDrawLine: from (x1, y1) to (x2, y2) inclusive.
void draw_line(UiLayer &layer, int x1, int y1, int x2, int y2, uint8_t colour)
{
    Surface &px = layer.pixels();
    int dx = std::abs(x2 - x1), dy = -std::abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1, sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;
    for (;;) {
        if (x1 >= 0 && y1 >= 0 && x1 < px.width() && y1 < px.height()) {
            px.row(y1)[x1] = colour;
            layer.mark({x1, y1, 1, 1});
        }
        if (x1 == x2 && y1 == y2)
            break;
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x1 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y1 += sy;
        }
    }
}

// draw_health_graph (0x73704): health_history[1-9] as a line graph.
void draw_health_graph(UiLayer &layer, const Thing &p, Point at, uint8_t colour)
{
    int step = kGraph.w / 10;
    for (int i = 1, point = 0; i < 9; i++, point++) {
        int h1 = kGraph.h * (p.health_history[size_t(i)] * 10) / 1000;
        int h2 = p.health_history[size_t(i) + 1] * 10 * kGraph.h / 1000;
        h1 = std::clamp(h1, 1, kGraph.h);
        h2 = std::clamp(h2, 1, kGraph.h);
        draw_line(layer, at.x + kGraph.x + point * step, at.y + kGraph.y - h1, at.x + kGraph.x + (point + 1) * step,
                  at.y + kGraph.y - h2, colour);
    }
}

// draw_diagnosis (0x738e8): the treatments given, then "cured", a line each
// from a line down.
void draw_diagnosis(UiLayer &layer, const Thing &p, Point at, const RequesterArt &art, const LanguageText &text)
{
    Rect box = {at.x + kDiagBox.x, at.y + kDiagBox.y, kDiagBox.w, kDiagBox.h};
    layer.fill(box, kDiagColour);
    const Font &font = art.list_font();
    const int line = font.glyph('A').height;
    const TextWindow whole = {0, 0, kUiWidth};
    int dy = 0;
    for (uint8_t t : p.treatments) {
        if (!t)
            break;
        dy += line;
        layer.draw_text(font, box.x, box.y + dy, text.at(TextSection::Illness, t), {whole, box});
    }
    if (p.person_flags & kPfCured) {
        dy += line;
        layer.draw_text(font, box.x, box.y + dy, text.at(TextSection::InfoBar, kCuredText), {whole, box});
    }
}

void draw_patient_window(UiLayer &layer, const InfoWindows &info, const Window &win, const WindowArt &window_art,
                         const World &world, uint8_t red)
{
    const RequesterArt &art = window_art.art;
    const Thing *pp = shown(world, info.patient, kThingPatient);
    if (!pp)
        return;
    const Thing &p = *pp;
    const Point at = {win.x, win.y};
    auto cover = [&](Rect r) { layer.fill({at.x + r.x, at.y + r.y, r.w, r.h}, kGreyColour); };
    bool diagnosed = p.diagnosis >= kDiagnosisComplete;
    if (!diagnosed) {
        cover(kCasebookBox);
        cover(kCureBox);
    }
    if (!p.queue_room)
        cover(kQueueBox);
    if (p.treatment == 0 || (p.person_flags & (kPfGuessed | kPfFedUp)) || p.illness == p.known_illness || !diagnosed) {
        cover(kCureBox);
        if (p.person_flags & (kPfCured | kPfSentHome))
            cover(kHomeBox);
    }
    const SpriteSheet &sheet = art.sheet(kPatientWindow);
    draw_gauge(layer, int16_t(p.happiness), {at.x + kPatientHappinessAt.x, at.y + kPatientHappinessAt.y},
               kPatientGaugeLen, sheet.at(kHappinessSprite), {100, 0, 100, true});
    draw_gauge(layer, p.refreshment, {at.x + kRefreshmentAt.x, at.y + kRefreshmentAt.y}, kPatientGaugeLen,
               sheet.at(kRefreshmentSprite), {100, 0, 100, true});
    draw_gauge(layer, int16_t(p.warmth), {at.x + kWarmthAt.x, at.y + kWarmthAt.y}, kPatientGaugeLen,
               sheet.at(kWarmthSprite), {100, 0, 100, true});
    if (info.show_graph)
        draw_health_graph(layer, p, at, red);
    else
        draw_diagnosis(layer, p, at, art, window_art.text);
}

void draw_staff_window(UiLayer &layer, const InfoWindows &info, const Window &win, int window,
                       const WindowArt &window_art, const World &world)
{
    const RequesterArt &art = window_art.art;
    const LanguageText &text = window_art.text;
    const Thing *sp = shown(world, info.staff, kThingStaff);
    if (!sp)
        return;
    const Thing &s = *sp;
    const Point at = {win.x, win.y};
    const SpriteSheet &sheet = art.sheet(window);
    const Rect screen = {0, 0, kUiWidth, kUiHeight};
    const TextWindow whole = {0, 0, kUiWidth};
    if (s.subtype == kStaffHandyman) {
        layer.draw_text(art.font(), at.x + kHandymanWageAt.x, at.y + kHandymanWageAt.y, std::to_string(s.salary),
                        {whole, screen});
        // draw_staff_handyman_bars.
        const uint8_t prio[3] = {s.prio_litter, s.prio_plants, s.prio_repair};
        for (size_t i = 0; i < 3; i++)
            draw_gauge(layer, prio[i], {at.x + kPriorityX[i], at.y + kPriorityY}, kPriorityLen,
                       sheet.at(kPrioritySprite), {100, 0, 100, true});
    } else {
        layer.draw_text(art.font(), at.x + kWageAt.x, at.y + kWageAt.y, std::to_string(s.salary), {whole, screen});
        const bool shows[6] = {
            (s.skills & kSkillJunior) != 0,
            (s.skills & kSkillDoctor) && !(s.skills & kSkillConsultant) && !(s.skills & kSkillJunior),
            (s.skills & kSkillConsultant) != 0,
            (s.skills & kSkillSurgeon) != 0,
            (s.skills & kSkillPsychiatrist) != 0,
            (s.skills & kSkillResearcher) != 0,
        };
        const int sprites[6] = {kTickSprite,    kTickSprite,         kTickSprite,
                                kSurgeonSprite, kPsychiatristSprite, kResearcherSprite};
        for (size_t i = 0; i < 6; i++)
            if (shows[i])
                layer.draw_sprite(at.x + kSkillAt[i].x, at.y + kSkillAt[i].y, sheet.at(size_t(sprites[i])));
        if (s.subtype != kStaffDoctor && s.subtype != kStaffSurgeon)
            layer.fill({at.x + kSkillCover.x, at.y + kSkillCover.y, kSkillCover.w, kSkillCover.h}, kCoverColour);
    }
    draw_face(layer, window_art.hire_art.faces(), s, at.x + kFaceAt.x, at.y + kFaceAt.y);
    draw_gauge(layer, int16_t(s.happiness), {at.x + kHappinessAt.x, at.y + kHappinessAt.y}, kStaffGaugeLen,
               sheet.at(kHappinessSprite), {100, 0, 100, true});
    draw_gauge(layer, s.tiredness, {at.x + kTirednessAt.x, at.y + kTirednessAt.y}, kStaffGaugeLen,
               sheet.at(kTirednessSprite), {1000, 0, 1000, false});
    draw_gauge(layer, s.ability, {at.x + kAbilityAt.x, at.y + kAbilityAt.y}, kAbilityGaugeLen, sheet.at(kAbilitySprite),
               {1000, 30, 970, true});
    std::string name = std::string(1, char(s.initial)) + ". " +
                       std::string(text.at(TextSection::StaffNames1, s.surname_a)) +
                       std::string(text.at(TextSection::StaffNames2, s.surname_b));
    layer.draw_text(art.font(), at.x + kNameAt.x, at.y + kNameAt.y, name, {whole, screen});
}

// The window open for a person: the patient's, the staff's or the
// handyman's; -1 for none.
int open_info_window(const Interface &ui)
{
    for (int w : {kPatientWindow, kStaffWindow, kHandymanWindow, kMachineWindow})
        if (ui.is_open(w))
            return w;
    return -1;
}

// The window EV_OPEN_THING_WINDOW (and its kin) open for thing `id`: -1 if
// none (not the player's, or not a person or machine).
int window_for(const World &world, uint16_t id)
{
    const Thing *t = thing_at(world, id);
    if (!t || t->owner != 0)
        return -1;
    if (t->type == kThingPatient)
        return kPatientWindow;
    if (t->type == kThingStaff)
        return t->subtype == kStaffHandyman ? kHandymanWindow : kStaffWindow;
    if (t->type == kThingObject && kObjectIsMachine[t->subtype % kObjectKinds])
        return kMachineWindow;
    return -1;
}

void open_for(InfoWindows &info, Interface &ui, int window, uint16_t id)
{
    close_irequesters(ui);
    if (window == kPatientWindow) {
        info.patient = id;
        info.show_graph = true;
        info.toggle_delay = 0;
    } else if (window == kMachineWindow) {
        info.machine = id;
    } else {
        info.staff = id;
    }
    ui.open_window(window);
}

// req3 (0x72d60): the patient window's buttons.
void patient_click(const Thing &p, uint16_t id, int button, InfoWindowResult &r)
{
    bool diagnosed = p.diagnosis >= kDiagnosisComplete;
    switch (button) {
    case kPatientClose:
        r.event = {EV_CLOSE_QUERY, 1};
        r.sounds.push_back(kSfxNo4);
        break;
    case kPatientCasebook:
        if (diagnosed) {
            r.casebook = p.illness;
            r.sounds.push_back(kSfxSelectX);
        }
        break;
    case kPatientHome:
        if (!(p.person_flags & (kPfCured | kPfSentHome)) && p.health > 0) {
            r.event = {EV_PERSON_GO_HOME, id};
            r.sounds.push_back(kSfxSack);
        }
        break;
    case kPatientQueue:
        if (p.queue_room) {
            r.event = {EV_OPEN_QUEUE_WINDOW, p.queue_room, 0, 0};
            r.sounds.push_back(kSfxSelectX);
        }
        break;
    case kPatientCure:
        if (p.treatment != 0 && !(p.person_flags & (kPfGuessed | kPfFedUp)) && p.illness != p.known_illness &&
            diagnosed && p.health > 0) {
            r.event = {EV_FORCE_CURE, id};
            r.sounds.push_back(kSfxSelectX);
        }
        break;
    default:
        break;
    }
}

// req4 (0x86c10): the staff and handyman windows' buttons.
void staff_click(const Thing &s, uint16_t id, int button, InfoWindowResult &r, Interface &ui)
{
    const bool handyman = s.subtype == kStaffHandyman;
    const int close = handyman ? kHandymanClose : kStaffClose;
    const int sack = handyman ? kHandymanSack : kStaffSack;
    const int pick_up = handyman ? kHandymanPickUp : kStaffPickUp;
    if (button == close) {
        r.event = {EV_CLOSE_QUERY, 2};
        r.sounds.push_back(kSfxNo4);
    } else if (button == sack) {
        // close_query(2), then open_yes_no_requester.
        close_irequesters(ui);
        r.sounds.push_back(kSfxSelectX);
        r.event = {EV_OPEN_YES_NO};
        r.question = kSackQuestion;
        r.yes = {EV_SACK_STAFF, id, 0, 0};
        r.no = {EV_CANCEL_SACK, id, 0, 0};
    } else if (button == pick_up) {
        r.event = {EV_PICK_UP_STAFF, id};
        r.sounds.push_back(kSfxPickup);
    } else if (handyman && (button == kHandymanLitterButton || button == kHandymanPlantsButton ||
                            button == kHandymanRepairButton)) {
        r.event = {EV_ADJUST_HANDYMAN_VALUES, id, 0, uint16_t(button)};
        // adjust_handyman_values' sound, for a priority it will raise.
        uint8_t prio = button == kHandymanLitterButton   ? s.prio_litter
                       : button == kHandymanPlantsButton ? s.prio_plants
                                                         : s.prio_repair;
        if (prio < 94)
            r.sounds.push_back(kSfxSelectX);
    }
}

// map_hover's test of object `hovered` (thing `id`), or of the other part
// of a two-part object: a machine, an object that shows information, or no
// hit.
MapHover object_hover(const World &world, uint16_t id, const Thing *hovered)
{
    const Thing *pair = thing_at(world, hovered->pair);
    if (hovered->pair && pair && kObjects[pair->subtype % kObjectKinds].next_part != -1) {
        id = hovered->pair;
        hovered = pair;
    }
    if (kObjectIsMachine[hovered->subtype % kObjectKinds])
        return {true, id, kPointerMachine, 3};
    if (kObjectFlags[hovered->subtype % kObjectKinds] & kObjectFlagHoverInfo)
        return {true, id, kPointerKeep, 6};
    return {};
}

// map_hover's last test of the resolved thing `id`: a patient or member of
// staff, the mayor or inspector (panel line 5), or a door of a room with a
// queue (the queue pointer).
MapHover map_hover_thing(const World &world, uint16_t id, const Thing &hovered)
{
    if (hovered.type == kThingPatient)
        return {true, id, kPointerPatient, 2};
    if (hovered.type == kThingStaff)
        return {true, id, kPointerStaff, 1};
    if (hovered.type == kThingMayor || hovered.type == kThingInspector)
        return {true, id, kPointerKeep, 5};
    if (hovered.type == kThingRat)
        return {true, id, kPointerRatAim, 6};
    if (hovered.type == kThingDoor && kRoomHasQueue[world.rooms[hovered.room_id % kRoomCount].type % kRoomTypes])
        return {true, id, kPointerQueue, 4};
    return {};
}

} // namespace

MapHover map_hover(const World &world, uint16_t pointed)
{
    MapHover h;
    const Thing *hovered = thing_at(world, pointed);
    if (!pointed || !hovered)
        return h;
    uint16_t id = pointed;
    if (hovered->type == kThingPatient || hovered->type == kThingStaff) {
        if (uint16_t used = partner(world, *hovered, pointed)) {
            if (world.things[used].subtype == kObjReceptionDesk)
                return {true, pointed, kPointerQueue, 4};
            if (kObjectIsMachine[world.things[used].subtype % kObjectKinds]) {
                id = used;
                hovered = &world.things[used];
            }
        }
    }
    if (hovered->type == kThingObject) {
        if (MapHover object = object_hover(world, id, hovered); object.hit)
            return object;
    }
    id = pointed;
    hovered = &world.things[pointed];
    if (hovered->type == kThingObject && (hovered->subtype == kObjSwingDoor || hovered->subtype == kObjSwingDoor2) &&
        hovered->door_main) {
        id = hovered->door_main;
        hovered = thing_at(world, id);
        if (!hovered)
            return h;
    }
    MapHover tail = map_hover_thing(world, id, *hovered);
    return tail.hit ? tail : h;
}

// map_left_click on a member of staff: a receptionist at her desk opens the
// queue window; otherwise close the query windows and open theirs (or answer
// their call).
Event map_left_click_staff(const World &world, const Thing &t, const MapHover &hover, Interface &ui)
{
    if (t.subtype == kStaffReceptionist) {
        uint16_t desk = partner(world, t, hover.thing);
        if (desk && world.things[desk].subtype == kObjReceptionDesk && t.room)
            return {EV_OPEN_QUEUE_WINDOW, t.room, 0, 0};
    }
    close_irequesters(ui);
    if ((t.orders & kOrderResign) && t.playercall)
        return {EV_ANSWER_PLAYERCALL, 1, 0, t.playercall};
    return {EV_OPEN_THING_WINDOW, hover.thing};
}

// map_left_click on a patient: close the query windows and open theirs, or
// answer their call.
Event map_left_click_patient(const Thing &t, const MapHover &hover, Interface &ui)
{
    close_irequesters(ui);
    if ((t.person_flags & kPfNeedsDiagnosisRoom) && t.playercall)
        return {EV_ANSWER_PLAYERCALL, 8, 0, t.playercall};
    if ((t.person_flags & kPfAwaitingDecision) && t.playercall)
        return {EV_ANSWER_PLAYERCALL, 4, 0, t.playercall};
    return {EV_OPEN_THING_WINDOW, hover.thing};
}

// map_left_click on an object: a machine opens its window, a reception desk
// the queue window of its receptionist's room.
Event map_left_click_object(const World &world, const Thing &t, const MapHover &hover, Interface &ui)
{
    if (kObjectIsMachine[t.subtype % kObjectKinds]) {
        close_irequesters(ui);
        return {EV_OPEN_THING_WINDOW, hover.thing};
    }
    if (t.subtype == kObjReceptionDesk) {
        const Thing *user = t.user < world.things.size() ? &world.things[t.user] : nullptr;
        if (user && user->type == kThingStaff && user->room)
            return {EV_OPEN_QUEUE_WINDOW, user->room, 0, 0};
    }
    return {};
}

// map_left_click on a door of a queued room: open its queue window and
// highlight the room (src/mouse.cpp's compute_room_pointed).
Event map_left_click_door(World &world, const Thing &t, int cell)
{
    if (!kRoomHasQueue[world.rooms[t.room_id % kRoomCount].type % kRoomTypes])
        return {};
    uint16_t scratch = 0;
    compute_room_pointed(world, cell, 0, scratch);
    return {EV_OPEN_QUEUE_WINDOW, t.room_id, 0, uint16_t((t.y.tile << 7) + t.x.tile)};
}

Event map_left_click(World &world, const MapHover &hover, Interface &ui, int cell, bool near_rat)
{
    const Thing *t = thing_at(world, hover.thing);
    if (!hover.hit || !t || t->owner != 0)
        // A rat near the pointer but not aimed at: a shot at nothing (a
        // miss), as process_mouse's MM_NORMAL.
        return !hover.hit && near_rat ? Event{EV_SHOOT_RAT, 0, 0, 0} : Event{};
    if (t->type == kThingStaff)
        return map_left_click_staff(world, *t, hover, ui);
    if (t->type == kThingPatient)
        return map_left_click_patient(*t, hover, ui);
    if (t->type == kThingObject)
        return map_left_click_object(world, *t, hover, ui);
    if (t->type == kThingRat)
        // The player shoots the rat aimed at (argb[1] 1: a hit); the rat
        // game's weapon is argb[0], 0 outside it.
        return {EV_SHOOT_RAT, hover.thing, 0, 0x0100};
    if (t->type == kThingDoor)
        return map_left_click_door(world, *t, cell);
    return {};
}

void info_window_event(InfoWindows &info, Interface &ui, const World &world, const Event &event)
{
    switch (event.type) {
    case EV_OPEN_THING_WINDOW:
        if (int w = window_for(world, event.value); w >= 0)
            open_for(info, ui, w, event.value);
        break;
    case EV_CLOSE_QUERY:
    case EV_PICK_UP_STAFF:
        close_irequesters(ui);
        break;
    case EV_CANCEL_SACK:
        if (const Thing *t = thing_at(world, event.value); t && t->owner == 0)
            open_for(info, ui, t->subtype == kStaffHandyman ? kHandymanWindow : kStaffWindow, event.value);
        break;
    case EV_RETURN_TO_THING_WINDOW:
        if (int w = window_for(world, event.arg); w >= 0 && ui.requester() == Requester::None)
            open_for(info, ui, w, event.arg);
        break;
    default:
        break;
    }
}

uint16_t find_next_thing(const World &world, uint16_t from, uint8_t type, int sub1, int sub2, ScrollArea scroll)
{
    const size_t n = world.things.size();
    auto matches = [&](size_t i) {
        const Thing &t = world.things[i];
        if (t.type != type || t.owner != 0)
            return false;
        if (sub1 >= 0 && t.subtype != sub1 && t.subtype != sub2)
            return false;
        if (!beyond_scroll_limit({t.x.tile, t.y.tile}, scroll.min_cell, scroll.max_cell))
            return false;
        return sub1 >= 0 || type != kThingObject || kObjectIsMachine[t.subtype % kObjectKinds];
    };
    size_t i = size_t(from) + 1 < n ? size_t(from) + 1 : 0;
    for (; i < n; i++)
        if (matches(i))
            return uint16_t(i);
    for (i = 0; i < n; i++)
        if (matches(i))
            return uint16_t(i);
    return uint16_t(n);
}

Point thing_position(const World &world, uint16_t id)
{
    const Thing &t = world.things[id];
    Point p = cell_to_world(t.x.tile, t.y.tile);
    return {p.x + ((int(t.x.frac) - int(t.y.frac)) >> 3), p.y + ((int(t.x.frac) + int(t.y.frac)) >> 4)};
}

Point info_centre(const World &world, uint16_t id)
{
    Point p = thing_position(world, id);
    return {p.x, p.y - kTrackLift};
}

namespace
{

// What req3_overdraw's and req4_overdraw's clicks test: the pointer, the
// window's position, its view's box, and the scroll limits.
struct InfoClick {
    const UiMouse &mouse;
    Point at;
    Rect view;
    bool left;
    bool right_click;
    ScrollArea scroll;
};

// req3_overdraw's clicks: centring on the patient, the diagnosis graph's
// toggle, and the next patient.
void patient_window_clicks(InfoWindows &info, InfoWindowResult &r, const World &world, uint16_t id,
                           const InfoClick &click)
{
    const UiMouse &mouse = click.mouse;
    if (info.toggle_delay)
        info.toggle_delay--;
    if (click.left && over(mouse.x, mouse.y, click.at, click.view)) {
        r.centre_on = id;
    } else if (click.left && over(mouse.x, mouse.y, click.at, kDiagBox) && !info.toggle_delay) {
        info.show_graph = !info.show_graph;
        info.toggle_delay = 5;
    }
    if (click.right_click && over(mouse.x, mouse.y, click.at, click.view)) {
        uint16_t next = find_next_thing(world, id, kThingPatient, -1, -1, click.scroll);
        if (next < world.things.size() && next != id) {
            info.patient = next;
            r.sounds.push_back(kSfxCamClick);
        }
    }
}

// req4_overdraw's clicks: centring on the member of staff, their face (the
// staff list), and the next one of their kind.
void staff_window_clicks(InfoWindows &info, InfoWindowResult &r, const World &world, uint16_t id,
                         const InfoClick &click)
{
    const UiMouse &mouse = click.mouse;
    const Thing &t = world.things[id];
    if (click.left && over(mouse.x, mouse.y, click.at, click.view))
        r.centre_on = id;
    if (click.left && over(mouse.x, mouse.y, click.at, kFaceBox)) {
        r.slist_staff = id;
        r.event = {EV_OPEN_SCREEN, uint16_t(kChartStaff)};
        r.sounds.push_back(kSfxSelectX);
    }
    if (click.right_click && over(mouse.x, mouse.y, click.at, click.view)) {
        uint16_t next = t.subtype == kStaffDoctor || t.subtype == kStaffSurgeon
                            ? find_next_thing(world, id, kThingStaff, kStaffDoctor, kStaffSurgeon, click.scroll)
                            : find_next_thing(world, id, kThingStaff, t.subtype, -1, click.scroll);
        if (next < world.things.size())
            info.staff = next;
        r.sounds.push_back(kSfxCamClick);
    }
}

// req5 (0x88260): the machine window's buttons.
void machine_click(const Thing &m, uint16_t id, int button, InfoWindowResult &r, const InfoWindowContext &context)
{
    switch (button) {
    case kMachineClose:
        r.event = {EV_CLOSE_QUERY, 3, 0, 0};
        r.sounds.push_back(kSfxNo4);
        break;
    case kMachineHandyman:
        r.event = {EV_THINGPAGE_REQUEST, id, 0, 0};
        r.sounds.push_back(kSfxSelectX);
        break;
    case kMachineReplace: {
        int cost = object_cost(context.world, m.subtype);
        if (context.balance >= cost && context.balance > 0) {
            r.event = {EV_OPEN_YES_NO, 0, 0, 0};
            r.question_text = format(context.text.at(TextSection::YesNo, kReplaceQuestion),
                                     std::string(context.text.at(TextSection::ObjectNames, m.subtype)).c_str(), cost);
            r.yes = {EV_REPLACE_MACHINE, id, 0, 0};
            r.no = {EV_CLOSE_YES_NO, 0, 0, 0};
            r.sounds.push_back(kSfxSelectX);
        }
        break;
    }
    default:
        break;
    }
}

// req5_overdraw's usage bar: `filled` of `total` of `box`'s width, the
// segment sprite repeated from the left.
void draw_machine_usage_bar(UiLayer &layer, Point at, int filled, int total, Rect box, const Sprite &sprite)
{
    int step = (total << 16) / box.w;
    if (step <= 0)
        return;
    for (int i = (filled << 16) / step - 1; i >= 0; i--)
        layer.draw_sprite(at.x + box.x + i, at.y + box.y, sprite);
}

// req5_overdraw: the machine's name (in brackets while a handyman is paged),
// the room's patient count and the strength left as a bar.
void draw_machine_window(UiLayer &layer, const InfoWindows &info, const Window &win, const WindowArt &window_art,
                         const World &world)
{
    const Thing *mp = thing_at(world, info.machine);
    if (!mp || mp->type != kThingObject)
        return;
    const Thing &m = *mp;
    const RequesterArt &art = window_art.art;
    const LanguageText &text = window_art.text;
    const Point at = {win.x, win.y};
    const Rect screen = {0, 0, kUiWidth, kUiHeight};
    const TextWindow whole = {0, 0, kUiWidth};
    std::string name(text.at(TextSection::ObjectNames, m.subtype));
    if (is_thing_paged(world, info.machine))
        name = "(" + name + ")";
    layer.draw_text(art.font(), at.x + kMachineNameAt.x, at.y + kMachineNameAt.y, name, {whole, screen});
    if (m.room_id) {
        const RoomInfo &room = world.rooms[m.room_id % kRoomCount];
        layer.draw_text(art.font(), at.x + kMachineCountAt.x, at.y + kMachineCountAt.y,
                        std::to_string(room.patients_seen), {whole, screen});
    }
    int32_t max = object_strength(world, m.subtype);
    int pct = max ? int((max - m.times_used) * 100 / max) : 0;
    draw_machine_usage_bar(layer, at, pct, 100, kMachineUsageBar, art.sheet(kMachineWindow).at(kUsageBarSprite));
}

// req5_overdraw's clicks: centring on the machine and the next machine.
void machine_window_clicks(InfoWindows &info, InfoWindowResult &r, const World &world, uint16_t id,
                           const InfoClick &click)
{
    const UiMouse &mouse = click.mouse;
    if (click.left && over(mouse.x, mouse.y, click.at, click.view))
        r.centre_on = id;
    if (click.right_click && over(mouse.x, mouse.y, click.at, click.view)) {
        // SFX_CAMCLICK always, even when the next machine is the shown one
        // (req5.cpp:130).
        uint16_t next = find_next_thing(world, id, kThingObject, -1, -1, click.scroll);
        if (next < world.things.size()) {
            info.machine = next;
            r.centre_on = next;
        }
        r.sounds.push_back(kSfxCamClick);
    }
}

// The window open and the thing it shows.
struct InfoWindowShow {
    int window;
    uint16_t id;
    const Thing *thing;
};

// The window's button, req3, req4 or req5 as it is.
void info_window_button(Interface &ui, const InterfaceEvents &events, const InfoWindowShow &show, InfoWindowResult &r,
                        const InfoWindowContext &context)
{
    if (events.triggered_window != show.window)
        return;
    if (show.window == kPatientWindow)
        patient_click(*show.thing, show.id, events.triggered_button, r);
    else if (show.window == kMachineWindow)
        machine_click(*show.thing, show.id, events.triggered_button, r, context);
    else
        staff_click(*show.thing, show.id, events.triggered_button, r, ui);
}

// req3_overdraw's, req4_overdraw's and req5_overdraw's clicks.
void info_window_click(InfoWindows &info, Interface &ui, const InfoWindowShow &show, const UiMouse &mouse,
                       InfoWindowResult &r, const InfoWindowContext &context)
{
    const Window &win = ui.window(show.window);
    const Point at = {win.x, win.y};
    const bool left = mouse.left_held || mouse.left_pressed;
    const bool right = mouse.right_held || mouse.right_pressed;
    const bool right_click = right && !info.right_was_down;
    const Rect view = show.window == kPatientWindow    ? kPatientViewBox
                      : show.window == kMachineWindow  ? kMachineViewBox
                      : show.window == kHandymanWindow ? kHandymanViewBox
                                                       : kStaffViewBox;
    const InfoClick click = {mouse, at, view, left, right_click, context.scroll};
    if (show.window == kPatientWindow)
        patient_window_clicks(info, r, context.world, show.id, click);
    else if (show.window == kMachineWindow)
        machine_window_clicks(info, r, context.world, show.id, click);
    else
        staff_window_clicks(info, r, context.world, show.id, click);
    info.right_was_down = right;
}

} // namespace

InfoWindowResult info_window_frame(InfoWindows &info, Interface &ui, const InterfaceEvents &events,
                                   const UiMouse &mouse, const InfoWindowContext &context)
{
    InfoWindowResult r;
    const int window = open_info_window(ui);
    if (window < 0)
        return r;
    const bool patient = window == kPatientWindow;
    const bool machine = window == kMachineWindow;
    const uint16_t id = patient ? info.patient : machine ? info.machine : info.staff;
    const Thing *t =
        machine ? thing_at(context.world, id) : shown(context.world, id, patient ? kThingPatient : kThingStaff);
    if (!t || t->owner != 0 || (machine && t->type != kThingObject)) {
        close_irequesters(ui);
        return r;
    }
    const InfoWindowShow show = {window, id, t};
    info_window_button(ui, events, show, r, context);
    if (!ui.is_open(window))
        return r;
    info_window_click(info, ui, show, mouse, r, context);

    // process_tracking_windows: the machine window has no view and is not
    // tracked.
    if (machine)
        return r;
    const uint16_t tracked = patient ? info.patient : info.staff;
    const Thing *tt = shown(context.world, tracked, patient ? kThingPatient : kThingStaff);
    if (!tt || beyond_scroll_limit(thing_position(context.world, tracked), context.scroll.min_cell,
                                   context.scroll.max_cell, 10))
        close_irequesters(ui);
    return r;
}

bool info_view(const InfoWindows &info, const Interface &ui, const World &world, Rect &box, Point &camera)
{
    const int window = open_info_window(ui);
    if (window < 0 || window == kMachineWindow)
        return false;
    const bool patient = window == kPatientWindow;
    const uint16_t id = patient ? info.patient : info.staff;
    if (!shown(world, id, patient ? kThingPatient : kThingStaff))
        return false;
    const Window &win = ui.window(window);
    const Rect r = patient ? kPatientView : window == kHandymanWindow ? kHandymanView : kStaffView;
    box = {win.x + r.x, win.y + r.y, r.w, r.h};
    Point c = info_centre(world, id);
    camera = {c.x - r.w / 2, c.y - r.h / 2};
    return true;
}

void draw_info_windows(UiLayer &layer, const InfoWindows &info, const Interface &ui, const WindowArt &window_art,
                       const World &world, uint8_t red)
{
    const int window = open_info_window(ui);
    if (window == kPatientWindow)
        draw_patient_window(layer, info, ui.window(window), window_art, world, red);
    else if (window == kMachineWindow)
        draw_machine_window(layer, info, ui.window(window), window_art, world);
    else if (window >= 0)
        draw_staff_window(layer, info, ui.window(window), window, window_art, world);
}

} // namespace th

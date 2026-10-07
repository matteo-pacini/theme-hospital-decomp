#include "core/ui/staff_demand.h"

#include "core/data/gamedata.h"
#include "core/render/frame.h"
#include "core/world/staff.h"
#include "core/world/world.h"

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <string>

namespace th
{

namespace
{

// HiresCoords (src/coords.cpp, read as IconScreen/Req2Layout by req12): the
// demand requester's pieces, relative to the window (coords[0] the ability
// gauge's position, [2] the name, [3] the face's bottom left, [4] the wage,
// [91-96] the qualifications) and its boxes (HiresBoxes, read as Req2Layout:
// [23] the view window, [24] the message, [87-88] the covers over the
// doctor-only parts, [86] the gauge's length).
constexpr Point kGaugeAt = {44, 232};
constexpr Point kNameAt = {23, 24};
constexpr Point kFaceAt = {98, 127};
constexpr Point kWageAt = {60, 181};
constexpr Point kSkillAt[6] = {{99, 232}, {121, 232}, {151, 232}, {63, 138}, {80, 144}, {107, 143}};
constexpr Rect kViewBox = {18, 47, 71, 81};
constexpr Rect kMsgBox = {208, 26, 135, 150};
constexpr Rect kCover2 = {15, 132, 157, 38};
constexpr Rect kCover3 = {91, 199, 81, 42};
constexpr int kGaugeWidth = 41;

// requester_sprites (src/reqbut.cpp): the ability gauge (requester_sprites[3]
// = 4) and the qualification tick and the surgeon, psychiatrist and
// researcher badges.
constexpr int kGaugeSprite = 4;
constexpr int kTickSprite = 0x128;
constexpr int kSurgeonSprite = 0x124;
constexpr int kPsychiatristSprite = 0x125;
constexpr int kResearcherSprite = 0x126;

constexpr uint8_t kBoxColour = 0x77;

// The interface's button numbers of req12's demand-granted and refused
// buttons (requester_buttons[0x15] and [0x16]).
constexpr int kGrantButton = 19;
constexpr int kRefuseButton = 20;

// saferand and safe_normal_rand (src/rand.cpp), from the safe seed.
uint16_t safe_rand(uint32_t &seed)
{
    seed = std::rotr(seed * 0x24a1u + 0x24dfu, 13);
    return uint16_t(seed);
}

uint32_t safe_normal_rand(uint32_t &seed, uint32_t low, uint32_t high)
{
    int32_t range = int32_t((high - low) / 2 + 1);
    if (range <= 0)
        range = 1;
    uint32_t sum = safe_rand(seed) % uint32_t(range);
    sum += safe_rand(seed) % uint32_t(range);
    return sum + low;
}

} // namespace

void open_staff_resign_requester(StaffDemand &demand, Interface &ui, const World &world, uint16_t staff,
                                 uint32_t &safeseed)
{
    static const Balance statics;
    const Balance::Gbv &gbv = world.settings ? world.settings->gbv : statics.gbv;
    const Thing &s = world.things[staff];
    demand.staff = staff;
    if (s.resign_count > 5) {
        demand.kind = 2;
        demand.amount = 0;
    } else if (s.wage_demand > s.salary) {
        demand.kind = 7;
        demand.amount = s.wage_demand * 10 / 100 + (s.wage_demand - s.salary);
    } else if (s.tiredness > gbv.very_tired && safe_rand(safeseed) % 2) {
        uint32_t i = safe_normal_rand(safeseed, 10, 30);
        demand.kind = safe_rand(safeseed) % 2 == 0 ? 3 : 4;
        demand.amount = s.salary * int32_t(i) / 100;
    } else if (int16_t(s.happiness) > gbv.very_tired) {
        uint32_t i = safe_normal_rand(safeseed, 10, 30);
        demand.kind = safe_rand(safeseed) % 2 == 0 ? 5 : 6;
        demand.amount = s.salary * int32_t(i) / 100;
    } else {
        uint32_t i = safe_normal_rand(safeseed, 10, 30);
        demand.kind = 8;
        demand.amount = s.salary * int32_t(i) / 100;
    }
    close_irequesters(ui);
    ui.open_requester(Requester::StaffDemand);
}

Event staff_demand_click(const StaffDemand &demand, const InterfaceEvents &events, bool &accept)
{
    accept = false;
    if (events.triggered_window != int(Requester::StaffDemand))
        return {};
    if (events.triggered_button == kRefuseButton)
        return {EV_REFUSE_STAFF_DEMAND, demand.staff, 0, 0};
    if (events.triggered_button != kGrantButton)
        return {};
    accept = true;
    switch (demand.kind) {
    case 2:
        return {EV_REFUSE_STAFF_DEMAND, demand.staff, 0, 0};
    case 3:
    case 5:
    case 8:
        return {EV_GIVE_STAFF_BONUS, demand.staff, 0, uint16_t(demand.amount)};
    case 4:
    case 6:
    case 7:
        return {EV_RAISE_STAFF_SALARY, demand.staff, 0, uint16_t(demand.amount)};
    default:
        return {};
    }
}

namespace
{

// The view window: the member of staff's current frame, centred in its box
// (the game draws the map there).
void demand_view(UiLayer &layer, const Window &win, const Thing &s, const GameData &data)
{
    Rect extent;
    FrameOptions opts;
    opts.layers = s.layers.data();
    opts.extent = &extent;
    Point at = {win.x + kViewBox.x + kViewBox.w / 2, win.y + kViewBox.y + kViewBox.h};
    Rect box = draw_frame(layer.pixels(), at, s.sprite, s.flip != 0, data, opts);
    if (box.w)
        layer.mark({box.x, box.y, box.w, box.h});
}

// The member of staff's name, wage, face, qualifications, the covers over the
// doctor-only parts and the ability gauge.
void demand_details(UiLayer &layer, const Window &win, const SpriteSheet &sheet, const WindowArt &art, const Thing &s)
{
    const Font &font = art.art.font();
    const LanguageText &text = art.text;
    const int x0 = win.x, y0 = win.y;
    const Rect screen = {0, 0, kUiWidth, kUiHeight};
    const TextWindow whole = {0, 0, kUiWidth};
    std::string name = std::string(1, char(s.initial)) + ". " +
                       std::string(text.at(TextSection::StaffNames1, s.surname_a)) +
                       std::string(text.at(TextSection::StaffNames2, s.surname_b));
    layer.draw_text(font, x0 + kNameAt.x, y0 + kNameAt.y, name, {whole, screen});
    layer.draw_text(font, x0 + kWageAt.x, y0 + kWageAt.y, "$" + std::to_string(s.salary), {whole, screen});
    draw_face(layer, art.hire_art.faces(), s, x0 + kFaceAt.x, y0 + kFaceAt.y);

    // The qualifications, at coords[91+i], one slot per kind whether shown or
    // not.
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
            layer.draw_sprite(x0 + kSkillAt[i].x, y0 + kSkillAt[i].y, sheet.at(size_t(sprites[i])));
    // draw_gauge: ability clamped to 30-1000, of a range of 970, from the
    // gauge's position.
    long value = std::clamp<long>(s.ability, 30, 1000);
    long filled = long(kGaugeWidth) * value / 970;
    Sprite gauge = sheet.at(kGaugeSprite);
    for (long i = 0; i < filled; i++)
        layer.draw_sprite(x0 + kGaugeAt.x + int(i), y0 + kGaugeAt.y, gauge);
    // Then the covers over the doctor-only parts, drawn after the gauge.
    if (s.subtype != kStaffDoctor && s.subtype != kStaffSurgeon) {
        layer.fill({x0 + kCover2.x, y0 + kCover2.y, kCover2.w, kCover2.h}, kBoxColour);
        layer.fill({x0 + kCover3.x, y0 + kCover3.y, kCover3.w, kCover3.h}, kBoxColour);
    }
}

// The request's text, the language's misc_text[kind] with its amounts.
void demand_message(UiLayer &layer, const Window &win, const WindowArt &art, const StaffDemand &demand, const Thing &s)
{
    const std::string fmt(art.text.misc(demand.kind));
    char buf[200];
    if (demand.kind == 2)
        std::snprintf(buf, sizeof buf, "%s", fmt.c_str());
    else if (demand.kind == 4 || demand.kind == 6)
        std::snprintf(buf, sizeof buf, fmt.c_str(), int(demand.amount), int(demand.amount) + int(s.salary));
    else
        std::snprintf(buf, sizeof buf, fmt.c_str(), int(demand.amount));
    Rect msg = {win.x + kMsgBox.x, win.y + kMsgBox.y, kMsgBox.w, kMsgBox.h};
    layer.draw_text(art.art.font(), 0, 0, buf, {{msg.x, msg.y, msg.w}, msg, kTextWrap});
}

} // namespace

void draw_staff_demand(UiLayer &layer, const StaffDemand &demand, const Interface &ui, const WindowArt &art,
                       const GameData &data, const World &world)
{
    if (ui.requester() != Requester::StaffDemand || demand.staff >= world.things.size())
        return;
    const Window &win = ui.window(int(Requester::StaffDemand));
    const Thing &s = world.things[demand.staff];
    demand_view(layer, win, s, data);
    demand_details(layer, win, art.art.sheet(int(Requester::StaffDemand)), art, s);
    demand_message(layer, win, art, demand, s);
}

} // namespace th

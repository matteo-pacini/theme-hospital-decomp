#include "core/ui/panel_info.h"

#include "core/data/gamedata.h"
#include "core/render/panel.h"
#include "core/render/text.h"
#include "core/ui/layer.h"
#include "core/world/build.h"
#include "core/world/game.h"
#include "core/world/queue.h"
#include "core/world/reaper.h"
#include "core/world/staff.h"
#include "core/world/world.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <utility>

namespace th
{

namespace
{

// HiresCoords and HiresBoxes (src/coords.cpp), as PanelLayout: the info
// bar's text x, first line y and line height, and its percentage bar, all
// relative to panel icon 7's top-left (the game's panel->x/y).
constexpr int kInfoTextX = 10;
constexpr int kInfoTextY = 10;
constexpr int kInfoLineHeight = 10;
constexpr Rect kInfoBar = {150, 0, 100, 10};

// The bar's line width (draw_panel_info_line: the screen less panel->x less
// the margin on each side; panel->x is 0 in the hires mode).
constexpr int kInfoWidth = kUiWidth - kInfoTextX * 2;

// LbPaletteFindColour: the nearest palette colour by squared distance, in six
// bits, ties to the first.
uint8_t find_colour(const Palette &palette, int r, int g, int b)
{
    auto at = [&](int i, int k) { return int(((palette[size_t(i)] >> (16 - 8 * k)) & 0xff) >> 2); };
    auto dist = [&](int i) {
        int dr = r - at(i, 0), dg = g - at(i, 1), db = b - at(i, 2);
        return dr * dr + dg * dg + db * db;
    };
    int best = 0;
    for (int i = 1; i < 256; i++)
        if (dist(i) < dist(best))
            best = i;
    return uint8_t(best);
}

// The info bar being drawn: its layer, font, text, palette, the origin of
// its text window (panel icon 7's top-left, the game's panel->x/y) and the
// line reached.
struct InfoBar {
    UiLayer &layer;
    const Font &font;
    const LanguageText &text;
    const Palette &palette;
    Point origin{};
    int line = 0;
};

// The language's info_bar_text[index] with `args` formatted in.
template <typename... Args> std::string info_format(const InfoBar &b, int index, Args... args)
{
    std::string f(b.text.at(TextSection::InfoBar, index));
    char buf[200];
    std::snprintf(buf, sizeof buf, f.c_str(), args...);
    return buf;
}

void info_at(const InfoBar &b, std::string_view s, int x, int y)
{
    b.layer.draw_text(b.font, b.origin.x + x, b.origin.y + y, s, {{0, 0, kUiWidth}, {0, 0, kUiWidth, kUiHeight}});
}

// draw_panel_info_line: `s` on the bar's current line, split over two when it
// does not fit.
void info_line(InfoBar &b, std::string_view s, int lines)
{
    int y = kInfoTextY + b.line * kInfoLineHeight;
    if (text_width(b.font, s) < kInfoWidth) {
        if (b.line + 1 <= lines) {
            info_at(b, s, kInfoTextX, y);
            b.line++;
        }
        return;
    }
    if (b.line + 2 > lines)
        return;
    // Split at the last space whose prefix fits the bar.
    size_t split = std::string_view::npos;
    for (size_t i = s.find(' '); i != std::string_view::npos; i = s.find(' ', i + 1))
        if (text_width(b.font, s.substr(0, i)) < kInfoWidth)
            split = i;
    if (split == std::string_view::npos) {
        info_at(b, s, kInfoTextX, y);
        b.line++;
        return;
    }
    info_at(b, s.substr(0, split), kInfoTextX, y);
    b.line++;
    info_at(b, s.substr(split + 1), kInfoTextX, kInfoTextY + b.line * kInfoLineHeight);
    b.line++;
}

// draw_panel_percent_bar: the caption on the third line and the bar filled to
// `value` per cent.
void info_percent(InfoBar &b, int value, std::string_view caption)
{
    uint8_t white = find_colour(b.palette, 0x3f, 0x3f, 0x3f);
    uint8_t red = find_colour(b.palette, 0x3f, 0, 0);
    uint8_t black = find_colour(b.palette, 0, 0, 0);
    info_at(b, caption, kInfoTextX, kInfoTextY + kInfoLineHeight * 2);
    value = std::clamp(value, 0, 100);
    int x = b.origin.x + kInfoBar.x;
    int y = b.origin.y + kInfoTextY + kInfoLineHeight * 2 + kInfoBar.y;
    int filled = (kInfoBar.w - 2) * value / 100;
    b.layer.fill({x, y, kInfoBar.w, kInfoBar.h}, white);
    b.layer.fill({x + 1, y + 1, filled, kInfoBar.h - 2}, red);
    b.layer.fill({x + 1 + filled, y + 1, kInfoBar.w - 2 - filled, kInfoBar.h - 2}, black);
}

std::string room_name(const World &world, const GameData &data, uint16_t room)
{
    if (room == 0 || room >= world.rooms.size())
        return {};
    const RoomInfo &r = world.rooms[room];
    return std::string(data.text().at(TextSection::RoomNames, kRoomTypeInfo[r.type % kRoomTypes].name));
}

// The info_bar_text index of the patient's overriding status (dying, no
// diagnosis, no treatment, too expensive), 0 for none.
int person_special(const Thing &p)
{
    if (p.person_flags & kPfDying)
        return 4;
    if (p.person_flags & kPfNoDiagnosis)
        return 15;
    if (p.person_flags & kPfNoTreatment)
        return 16;
    if (p.person_flags & kPfTooExpensive)
        return 19;
    return 0;
}

// The patient's decision or waiting status and the rooms they queue for or
// head to (one line each in the hires mode).
void person_rooms(InfoBar &b, const World &world, const GameData &data, const Thing &p)
{
    const int lines = 1;
    if (p.person_flags & kPfAwaitingDecision) {
        info_line(b, b.text.at(TextSection::InfoBar, 5), lines);
        return;
    }
    if (p.person_flags & kPfWaitingForRoom) {
        info_line(b, b.text.at(TextSection::InfoBar, (p.person_flags & kPfNeedsDiagnosisRoom) ? 17 : 18), lines);
        return;
    }
    if (p.queue_room)
        info_line(b, info_format(b, 6, room_name(world, data, p.queue_room).c_str()), lines);
    if (p.dest_room && p.dest_room != p.queue_room)
        info_line(b, info_format(b, 7, room_name(world, data, p.dest_room).c_str()), lines);
}

// The patient's outcome status (cured, sent home, fed up, referred,
// contagious).
void person_status(InfoBar &b, const Thing &p)
{
    static constexpr std::pair<uint32_t, int> kStatus[] = {{kPfCured, 8},     {kPfSentHomeByInspector, 24},
                                                           {kPfFedUp, 9},     {kPfSentHome, 10},
                                                           {kPfReferred, 11}, {kPfContagious, 25}};
    for (const auto &[flag, index] : kStatus)
        if (p.person_flags & flag) {
            info_line(b, b.text.at(TextSection::InfoBar, index), 1);
            return;
        }
}

// The patient's illness: an emergency's, the guessed or diagnosed one, or
// the diagnosis progress.
void person_diagnosis(InfoBar &b, const Thing &p, const GameData &data)
{
    if (b.line >= 3 || p.illness == 0)
        return;
    if (p.person_flags & kPfEmergency) {
        b.line = 2;
        info_line(b, info_format(b, 22, data.text().at(TextSection::Illness, p.illness).data()), 3);
    } else if (p.person_flags & (kPfGuessed | kPfDiagnosed)) {
        b.line = 2;
        const char *name = data.text().at(TextSection::Illness, p.known_illness).data();
        info_line(b, info_format(b, (p.person_flags & kPfDiagnosed) ? 12 : 13, name), 3);
    } else {
        // gbv.diagnosis_complete: a patient's diagnosis is complete at 1000
        // (rooms.cpp's kDiagnosisComplete).
        info_percent(b, int(p.diagnosis) * 100 / 1000, b.text.at(TextSection::InfoBar, 14));
    }
}

// display_panel_person_info: the patient's status, the room they queue for or
// head to, and the diagnosis or its progress.
void person_info(InfoBar &b, const World &world, const GameData &data, uint16_t thing)
{
    const Thing &p = world.things[thing];
    if (int special = person_special(p)) {
        info_line(b, b.text.at(TextSection::InfoBar, special), 3);
        return;
    }
    person_rooms(b, world, data, p);
    person_status(b, p);
    person_diagnosis(b, p, data);
}

// The member of staff's name or, for a qualified doctor or surgeon, their
// skills joined.
std::string staff_name(const GameData &data, const Thing &s)
{
    if ((s.subtype != kStaffDoctor && s.subtype != kStaffSurgeon) ||
        !(s.skills & (kSkillSurgeon | kSkillPsychiatrist | kSkillConsultant | kSkillResearcher)))
        return std::string(data.text().at(TextSection::StaffNames, s.subtype));
    static constexpr std::pair<uint16_t, int> kSkill[] = {
        {kSkillConsultant, 7}, {kSkillResearcher, 8}, {kSkillSurgeon, 5}, {kSkillPsychiatrist, 6}};
    std::string joined;
    int count = 0;
    for (const auto &[mask, name] : kSkill) {
        if (!(s.skills & mask))
            continue;
        if (count == 3)
            joined += " " + std::string(data.text().at(TextSection::InfoBar, 26));
        else
            joined += (count ? " " : "") + std::string(data.text().at(TextSection::SkillNames, name));
        count++;
    }
    return joined;
}

// The member of staff's tiredness bar, with the policy's rest threshold as a
// white line.
void staff_tiredness(InfoBar &b, const Game &game, const Thing &s)
{
    int pct = std::clamp(game.hospital.rest_threshold * 100 / 1000, 0, 100);
    uint8_t white = find_colour(b.palette, 0x3f, 0x3f, 0x3f);
    int x = b.origin.x + kInfoBar.x + 1 + pct * (kInfoBar.w - 2) / 100;
    int y0 = b.origin.y + kInfoTextY + kInfoLineHeight * 2 + 1;
    int y1 = b.origin.y + kInfoTextY + kInfoLineHeight * 2 + kInfoBar.h - 1 + kInfoBar.y;
    if (x >= 0 && x < kUiWidth)
        for (int y = y0; y <= y1; y++)
            if (y >= 0 && y < kUiHeight) {
                b.layer.pixels().row(y)[x] = white;
                b.layer.mark({x, y, 1, 1});
            }
    info_percent(b, s.tiredness * 100 / 1000, b.text.at(TextSection::InfoBar, 28));
}

// display_panel_staff_info: the member of staff's kind and skills and their
// tiredness against the policy's rest threshold (the heading line is
// mode-13 only).
void staff_info(InfoBar &b, const World &world, const Game &game, const GameData &data, uint16_t thing)
{
    const Thing &s = world.things[thing];
    if (s.owner == 0x63)
        return;
    info_line(b, staff_name(data, s), 2);
    staff_tiredness(b, game, s);
}

// display_panel_object_info: a machine's name and its uses and strength.
void object_info(InfoBar &b, const World &world, const GameData &data, uint16_t thing)
{
    const Thing &o = world.things[thing];
    if (o.type != kThingObject)
        return;
    info_line(b, data.text().at(TextSection::ObjectNames, o.subtype), 3);
    b.line = 2;
    info_line(b, info_format(b, 31, int(o.times_used)) + "/" + std::to_string(o.strength), 3);
}

// display_panel_queue_info: the room under the pointer and its queue.
void queue_info(InfoBar &b, const World &world, const GameData &data, uint16_t thing)
{
    const Thing &s = world.things[thing];
    uint16_t room = s.type == kThingStaff ? s.room : s.room_id;
    if (room == 0 || room >= world.rooms.size())
        return;
    const RoomInfo &r = world.rooms[room];
    info_line(b, room_name(world, data, room), 3);
    b.line = 2;
    info_line(b, info_format(b, 32, int(r.queue_length)), 3);
}

// display_panel_usage_info: the machine under the pointer and its uses.
void usage_info(InfoBar &b, const World &world, const GameData &data, uint16_t thing)
{
    const Thing &m = world.things[thing];
    info_line(b, data.text().at(TextSection::ObjectNames, m.subtype), 3);
    b.line = 2;
    info_line(b, info_format(b, 31, int(m.times_used)), 3);
}

// display_panel_mayor_info: the VIP or the health inspector under the
// pointer.
void mayor_info(InfoBar &b, const World &world, const GameData &data, uint16_t thing)
{
    const Thing &m = world.things[thing];
    if (m.type == kThingMayor) {
        info_line(b, b.text.at(TextSection::InfoBar, 20), 3);
        info_line(b, data.text().at(TextSection::MayorNames, world.mayor.name), 3);
    } else if (m.type == kThingInspector) {
        info_line(b, b.text.at(TextSection::InfoBar, 21), 3);
    }
}

} // namespace

void draw_panel_info(const PanelInfoView &view, uint8_t info, uint16_t thing, const Game &game, const GameData &data)
{
    if (!info || thing == 0 || thing >= game.world.things.size())
        return;
    // The text window's origin is panel icon 7's top-left (the game's
    // panel->x/y); the panel is not raised while the pointer is on the map.
    InfoBar b{view.layer, view.font, data.text(), data.palette(), panel_icon_at(data, false, 7), 0};
    switch (info) {
    case 1:
        staff_info(b, game.world, game, data, thing);
        break;
    case 2:
        person_info(b, game.world, data, thing);
        break;
    case 3:
        object_info(b, game.world, data, thing);
        break;
    case 4:
        queue_info(b, game.world, data, thing);
        break;
    case 5:
        mayor_info(b, game.world, data, thing);
        break;
    case 6:
        usage_info(b, game.world, data, thing);
        break;
    default:
        break;
    }
}

} // namespace th

#include "core/ui/hire_staff.h"

#include "core/data/gamedata.h"
#include "core/render/blit.h"
#include "core/render/frame.h"
#include "core/render/text.h"
#include "core/render/view.h"
#include "core/world/game.h"
#include "core/world/staff.h"

#include <algorithm>
#include <cstdlib>
#include <string>

namespace th
{

namespace
{

constexpr int kHireWindow = int(Requester::HireStaff);

// hires_requester_buttons[2-9] (src/reqbut.cpp): the hire staff
// requester's buttons, counted from 1 in its list.
constexpr int kDoctorButton = 0x26;
constexpr int kNurseButton = 0x27;
constexpr int kHandymanButton = 0x28;
constexpr int kReceptionistButton = 0x29;
constexpr int kPreviousButton = 0x2a;
constexpr int kHireButton = 0x2b;
constexpr int kCloseButton = 0x2c;
constexpr int kNextButton = 0x2d;

// Sprites of the requester's sheet (REQ11V) the overdraw uses: the kind
// buttons lit (hires_requester_sprites[0x35] and [0x31], and the literal
// 0x10b and 0x10d), the gauge (requester_sprites[2]) and the
// qualifications' ticks (requester_sprites[6-9]).
constexpr int kDoctorLit = 0x109;
constexpr int kNurseLit = 0x10b;
constexpr int kHandymanLit = 0x10d;
constexpr int kReceptionistLit = 0x10f;
constexpr int kGaugeSprite = 3;
constexpr int kTickSprite = 0x128;
constexpr int kSurgeonSprite = 0x124;
constexpr int kPsychiatristSprite = 0x125;
constexpr int kResearcherSprite = 0x126;

// HiresCoords (src/coords.cpp), relative to the window: [78-81] the
// applicant counts (doctors, nurses, handymen, receptionists), [82] the
// name, [83] the face's bottom left, [84] the wage, [85-90] the
// qualifications (junior, doctor, consultant, surgeon, psychiatrist,
// researcher).
constexpr Point kCountAt[4] = {{29, 57}, {29, 136}, {29, 215}, {29, 294}};
constexpr Point kNameAt = {81, 24};
constexpr Point kFaceAt = {157, 128};
constexpr Point kWageAt = {118, 181};
constexpr Point kSkillAt[6] = {{81, 82}, {103, 82}, {133, 82}, {122, 138}, {139, 144}, {166, 143}};

// HiresBoxes (src/coords.cpp, read as Req2Layout), relative to the window:
// [59] and [60] cover the doctor-only parts, [61] blanks the applicant
// while no kind is chosen, [65] the reference's text window, [86] the
// ability gauge (x, y, length).
constexpr Rect kCover0 = {73, 47, 79, 43};
constexpr Rect kCover1 = {73, 132, 157, 40};
constexpr Rect kBlankBox = {70, 13, 161, 261};
constexpr Rect kReferenceBox = {79, 207, 146, 59};
constexpr Rect kGauge = {87, 106, 41, 0};

// The colours the requester boxes are filled with (LbDrawBox).
constexpr uint8_t kBoxColour = 0x77;
constexpr uint8_t kFaceBorderColour = 0x9c;

// The faces file's sizes (HiresCoords[162-166]): width, part heights and
// the border round a face.
constexpr int kFaceW = 65;
constexpr int kHeadH = 29;
constexpr int kBodyH = 24;
constexpr int kLegsH = 22;
constexpr int kFaceH = kHeadH + kBodyH + kLegsH;
constexpr int kFaceBorder = 3;
constexpr int kFaceParts = 18;

// MM_PLACE_STAFF, the mouse mode EV_CANCEL_BUILD carries while placing.
constexpr uint16_t kMousePlaceStaff = 3;

// The requesters EV_CANCEL_BUILD closes before close_irequesters.
constexpr Requester kCancelCloses[] = {Requester::Staff,       Requester::HireStaff, Requester::Jukebox,
                                       Requester::Queue,       Requester::BuildRoom, Requester::BuyObjects,
                                       Requester::StaffDemand, Requester::EditRoom};
// LbSqrL: the integer square root, rounded down.
uint32_t isqrt(uint32_t n)
{
    uint32_t r = 0;
    for (uint32_t bit = 1u << 30; bit; bit >>= 2) {
        if (n >= r + bit) {
            n -= r + bit;
            r = (r >> 1) + bit;
        } else {
            r >>= 1;
        }
    }
    return r;
}

// The candidates of `list` whose `key` is the smallest, in `list`'s order.
template <typename Key> std::vector<int> smallest(const std::vector<int> &list, Key &&key)
{
    int best = 999999;
    for (int i : list)
        best = std::min(best, key(i));
    std::vector<int> out;
    for (int i : list)
        if (key(i) == best)
            out.push_back(i);
    return out;
}

// LbPaletteFindColour (0xc960c) on a palette of 6-bit channels.
uint8_t find_colour(const std::array<std::array<int, 3>, 256> &pal, int r, int g, int b)
{
    auto sq = [](int v) { return v * v; };
    auto dist = [&](int i) {
        return sq(r - pal[size_t(i)][0]) + sq(g - pal[size_t(i)][1]) + sq(b - pal[size_t(i)][2]);
    };
    auto manhattan = [&](int i) {
        return std::abs(r - pal[size_t(i)][0]) + std::abs(g - pal[size_t(i)][1]) + std::abs(b - pal[size_t(i)][2]);
    };
    auto darkness = [&](int i) {
        const auto &p = pal[size_t(i)];
        return p[1] * 2 * p[1] + p[0] * 2 * p[0] + p[2] * p[2];
    };
    std::vector<int> list(256);
    for (int i = 0; i < 256; i++)
        list[size_t(i)] = i;
    list = smallest(list, dist);
    // An exact match is the first one, whatever the ties.
    if (list.size() == 1 || dist(list[0]) == 0)
        return uint8_t(list[0]);
    list = smallest(list, manhattan);
    if (list.size() == 1)
        return uint8_t(list[0]);
    return uint8_t(smallest(list, darkness)[0]);
}

} // namespace

void draw_face(UiLayer &layer, const std::vector<uint8_t> &faces, const Thing &staff, int x, int y)
{
    Surface &px = layer.pixels();
    const size_t heads = size_t(kFaceW) * kHeadH * kFaceParts;
    const size_t bodies = size_t(kFaceW) * kBodyH * kFaceParts;
    const struct {
        size_t offset;
        int rows;
    } parts[3] = {{size_t(staff.face_head) * kFaceW * kHeadH, kHeadH},
                  {heads + size_t(staff.face_body) * kFaceW * kBodyH, kBodyH},
                  {heads + bodies + size_t(staff.face_legs) * kFaceW * kLegsH, kLegsH}};
    int top = y - kFaceH - kFaceBorder;
    int row = top;
    for (const auto &part : parts) {
        for (int r = 0; r < part.rows; r++, row++) {
            size_t src = part.offset + size_t(r) * kFaceW;
            if (src + kFaceW > faces.size() || row < 0 || row >= px.height())
                continue;
            for (int c = 0; c < kFaceW; c++) {
                int dx = x + kFaceBorder + c;
                if (dx >= 0 && dx < px.width())
                    px.row(row)[dx] = faces[src + size_t(c)];
            }
        }
    }
    layer.mark({x + kFaceBorder, top, kFaceW, kFaceH});
    int outer_top = y - kFaceH - kFaceBorder * 2;
    int outer_w = kFaceBorder * 2 + kFaceW;
    int outer_h = kFaceH + kFaceBorder * 2;
    layer.fill({x, outer_top, outer_w, kFaceBorder}, kFaceBorderColour);
    layer.fill({x, outer_top, kFaceBorder, outer_h}, kFaceBorderColour);
    layer.fill({x, y - kFaceBorder, outer_w, kFaceBorder}, kFaceBorderColour);
    layer.fill({x + kFaceW + kFaceBorder, outer_top, kFaceBorder, outer_h}, kFaceBorderColour);
}

bool HireStaffArt::load(const std::filesystem::path &dir, std::string *error)
{
    auto faces = load_data_file(dir, "qdata/face01v.dat", error);
    if (!faces)
        return false;
    faces_ = std::move(*faces);
    return load_font(dir, 13, count_font_, error);
}

uint16_t alter_applicant(const World &world, bool forward, uint16_t cur, uint16_t kind)
{
    auto applicant = [&](size_t i) {
        const Thing &t = world.things[i];
        return t.type == kThingStaff && t.subtype == kind && t.state == kStaffApplicant;
    };
    size_t end = world.things.size();
    if (cur == 0) {
        for (size_t i = 1; i < end; i++)
            if (applicant(i))
                return uint16_t(i);
        return 0;
    }
    if (cur >= end)
        return 0;
    if (forward) {
        for (size_t i = size_t(cur) + 1; i < end; i++)
            if (applicant(i))
                return uint16_t(i);
    } else {
        for (size_t i = size_t(cur) - 1; i >= 1; i--)
            if (applicant(i))
                return uint16_t(i);
    }
    return cur;
}

Event hire_staff_click(const HireStaff &hire, const InterfaceEvents &events, const Game &game)
{
    Event e;
    if (events.triggered_window != kHireWindow)
        return e;
    const World &world = game.world;
    switch (events.triggered_button) {
    case kDoctorButton:
        return {EV_HIRE_SELECT_KIND, kStaffDoctor};
    case kNurseButton:
        return {EV_HIRE_SELECT_KIND, kStaffNurse};
    case kReceptionistButton:
        return {EV_HIRE_SELECT_KIND, kStaffReceptionist};
    case kHandymanButton:
        return {EV_HIRE_SELECT_KIND, kStaffHandyman};
    case kHireButton:
        if (hire.shown && hire.mode == 2 && hire.shown < world.things.size()) {
            int32_t salary = world.things[hire.shown].salary;
            if (game.balance >= salary && game.balance > 0)
                e = {EV_MARK_EMPLOYING, hire.kind, 0, hire.shown};
        }
        return e;
    case kPreviousButton:
    case kNextButton:
        if (hire.shown) {
            bool forward = events.triggered_button == kNextButton;
            uint16_t id = alter_applicant(world, forward, hire.shown, hire.kind);
            if (id && id != hire.shown)
                e = {EV_HIRE_NEXT_APPLICANT, uint16_t(forward)};
        }
        return e;
    case kCloseButton:
        return hire_staff_cancel(hire);
    default:
        return e;
    }
}

Event hire_staff_cancel(const HireStaff &hire)
{
    return {EV_CANCEL_BUILD, hire.held, 0, uint16_t(hire.placing() ? kMousePlaceStaff : 0)};
}

void hire_staff_event(HireStaff &hire, Interface &ui, const World &world, const Event &event)
{
    switch (event.type) {
    case EV_OPEN_HIRE_STAFF:
        // cancel_top_level_requesters(REQ_HIRE_STAFF); the buy objects
        // requester's case is left to its own step.
        if (ui.requester() == Requester::BuildRoom)
            ui.close_requester(Requester::BuildRoom);
        if (ui.requester() == Requester::None) {
            // req2_reset.
            hire.mode = 1;
            hire.kind = 0;
            close_irequesters(ui);
            ui.open_requester(Requester::HireStaff);
        }
        break;
    case EV_HIRE_SELECT_KIND:
        hire.mode = 2;
        hire.kind = event.value;
        hire.shown = alter_applicant(world, true, 0, hire.kind);
        break;
    case EV_HIRE_NEXT_APPLICANT:
        hire.shown = alter_applicant(world, event.value != 0, hire.shown, hire.kind);
        break;
    case EV_MARK_EMPLOYING:
        if (event.arg < world.things.size() && world.things[event.arg].state != kStaffHeld) {
            ui.close_requester(ui.requester());
            hire.held_kind = event.value;
            hire.held = event.arg;
            hire.cursor_cell = -1;
        }
        break;
    case EV_CANCEL_BUILD:
        for (Requester r : kCancelCloses)
            ui.close_requester(r);
        close_irequesters(ui);
        hire.held = 0;
        hire.held_kind = 0;
        break;
    default:
        break;
    }
}

void hire_staff_after_turn(HireStaff &hire, const World &world)
{
    if (hire.held && (hire.held >= world.things.size() || world.things[hire.held].state != kStaffHeld)) {
        hire.held = 0;
        hire.held_kind = 0;
    }
    // staff_picked_up's part on the player's machine.
    uint16_t picked = world.held_staff;
    if (!hire.held && picked && picked < world.things.size() && world.things[picked].state == kStaffHeld) {
        hire.held = picked;
        hire.held_kind = world.things[picked].subtype;
        hire.cursor_cell = -1;
    }
}

bool update_staff_cursor(HireStaff &hire, const World &world, int cell)
{
    if (cell != hire.cursor_cell) {
        hire.place_ok = staff_place_ok(world, cell, hire.held_kind);
        hire.cursor_cell = cell;
    }
    return hire.place_ok;
}

Event pick_up_staff_click(const World &world, uint16_t pointed)
{
    if (pointed == 0 || pointed >= world.things.size())
        return {};
    const Thing &t = world.things[pointed];
    if (t.type != kThingStaff || t.owner != 0)
        return {};
    return {EV_PICK_UP_STAFF, pointed};
}

Event place_staff_click(HireStaff &hire, const World &world, int cell)
{
    if (!hire.placing() || !update_staff_cursor(hire, world, cell))
        return {};
    return {EV_PLACE_STAFF, uint16_t(cell), 0, hire.held};
}

namespace
{

// The lit button of the kind of staff whose applicant is shown.
void draw_hire_staff_kind(UiLayer &layer, const Window &win, const SpriteSheet &sheet, int kind)
{
    int button = 0, sprite = 0;
    switch (kind) {
    case kStaffDoctor:
        button = kDoctorButton, sprite = kDoctorLit;
        break;
    case kStaffNurse:
        button = kNurseButton, sprite = kNurseLit;
        break;
    case kStaffReceptionist:
        button = kReceptionistButton, sprite = kReceptionistLit;
        break;
    case kStaffHandyman:
        button = kHandymanButton, sprite = kHandymanLit;
        break;
    }
    if (button) {
        const WindowButton &b = win.buttons[size_t(button - 1)];
        layer.draw_sprite(win.x + b.x, win.y + b.y, sheet.at(size_t(sprite)));
    }
}

// The number of applicants of each kind.
void draw_hire_staff_counts(UiLayer &layer, const Window &win, const HireStaffArt &hire_art, const World &world)
{
    const Rect screen = {0, 0, kUiWidth, kUiHeight};
    const TextWindow whole = {0, 0, kUiWidth};
    const uint8_t count_kinds[4] = {kStaffDoctor, kStaffNurse, kStaffHandyman, kStaffReceptionist};
    int counts[4] = {};
    for (const Thing &t : world.things)
        if (t.type == kThingStaff && t.state == kStaffApplicant)
            for (int i = 0; i < 4; i++)
                counts[i] += t.subtype == count_kinds[i];
    for (int i = 0; i < 4; i++)
        layer.draw_text(hire_art.count_font(), win.x + kCountAt[i].x, win.y + kCountAt[i].y, std::to_string(counts[i]),
                        {whole, screen});
}

// Applicant `s`'s qualifications, the cover over the doctors' ones for other
// staff, and the ability gauge.
void draw_hire_staff_skills(UiLayer &layer, const Window &win, const SpriteSheet &sheet, const Thing &s)
{
    const int x0 = win.x, y0 = win.y;
    const bool shows[6] = {
        (s.skills & kSkillJunior) && !(s.skills & kSkillConsultant),
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
    if (s.subtype != kStaffDoctor) {
        // draw_req2_coverup.
        layer.fill({x0 + kCover0.x, y0 + kCover0.y, kCover0.w, kCover0.h}, kBoxColour);
        layer.fill({x0 + kCover1.x, y0 + kCover1.y, kCover1.w, kCover1.h}, kBoxColour);
    }
    // draw_gauge (0x73fbc): value clamped to 30-1000, of a range of 970. The
    // game adds the qualifications loop's counter (5) to its x.
    long value = std::clamp<long>(s.ability, 30, 1000);
    long filled = kGauge.w * value / 970;
    Sprite gauge = sheet.at(kGaugeSprite);
    for (long i = 0; i < filled; i++)
        layer.draw_sprite(x0 + kGauge.x + 5 + int(i), y0 + kGauge.y, gauge);
}

// Applicant `s`'s details: name, wage, face, qualifications and reference.
void draw_hire_staff_applicant(UiLayer &layer, const Window &win, const WindowArt &window_art, const Thing &s)
{
    const RequesterArt &art = window_art.art;
    const LanguageText &text = window_art.text;
    const int x0 = win.x, y0 = win.y;
    const Rect screen = {0, 0, kUiWidth, kUiHeight};
    const TextWindow whole = {0, 0, kUiWidth};
    std::string name = std::string(1, char(s.initial)) + ". " +
                       std::string(text.at(TextSection::StaffNames1, s.surname_a)) +
                       std::string(text.at(TextSection::StaffNames2, s.surname_b));
    layer.draw_text(art.font(), x0 + kNameAt.x, y0 + kNameAt.y, name, {whole, screen});
    layer.draw_text(art.font(), x0 + kWageAt.x, y0 + kWageAt.y, "$" + std::to_string(s.salary), {whole, screen});
    draw_face(layer, window_art.hire_art.faces(), s, x0 + kFaceAt.x, y0 + kFaceAt.y);
    draw_hire_staff_skills(layer, win, art.sheet(kHireWindow), s);

    // In 640 x 480 the reference is the general text followed by the good or
    // the bad one.
    std::string reference(text.at(TextSection::RefGeneral, s.ref_general));
    reference += s.ability > 500 ? text.at(TextSection::RefGood, s.ref_good) : text.at(TextSection::RefBad, s.ref_bad);
    Rect box = {x0 + kReferenceBox.x, y0 + kReferenceBox.y, kReferenceBox.w, kReferenceBox.h};
    layer.draw_text(art.font(), 0, 0, reference, {{box.x, box.y, box.w}, box, kTextWrap});
}

} // namespace

void draw_hire_staff(UiLayer &layer, HireStaff &hire, const Interface &ui, const WindowArt &window_art,
                     const World &world)
{
    bool open = false;
    for (int i = 0; i < ui.window_count(); i++)
        open |= ui.open_window_at(i) == kHireWindow;
    if (!open)
        return;
    if (hire.shown >= world.things.size())
        hire.shown = 0;
    if (hire.shown && world.things[hire.shown].state != kStaffApplicant)
        hire.shown = alter_applicant(world, true, 0, hire.kind);

    const Window &win = ui.window(kHireWindow);
    if (hire.shown && hire.mode == 2)
        draw_hire_staff_kind(layer, win, window_art.art.sheet(kHireWindow), hire.kind);
    draw_hire_staff_counts(layer, win, window_art.hire_art, world);
    if (hire.mode != 2 || !hire.shown) {
        layer.fill({win.x + kBlankBox.x, win.y + kBlankBox.y, kBlankBox.w, kBlankBox.h}, kBoxColour);
        return;
    }
    draw_hire_staff_applicant(layer, win, window_art, world.things[hire.shown]);
}

std::array<uint8_t, 256> grey_remap(const Palette &palette)
{
    std::array<std::array<int, 3>, 256> pal{};
    for (size_t i = 0; i < 256; i++)
        pal[i] = {int(palette[i] >> 18 & 0x3f), int(palette[i] >> 10 & 0x3f), int(palette[i] >> 2 & 0x3f)};
    std::array<uint8_t, 256> out{};
    for (size_t i = 0; i < 255; i++) {
        uint32_t r = uint32_t(pal[i][0]), g = uint32_t(pal[i][1]), b = uint32_t(pal[i][2]);
        uint32_t grey = isqrt(r * r + g * g);
        grey = isqrt(grey * grey + b * b);
        grey = (grey * 0x3f + 0x36) / 0x6d;
        out[i] = find_colour(pal, int(uint8_t(grey)), int(uint8_t(grey)), int(uint8_t(grey)));
    }
    return out;
}

namespace
{

// The greying remap and the see-through blending table.
struct GreyRemap {
    std::array<uint8_t, 256> grey;
    const uint8_t *ghost;
};

// Copies the pixels `scratch` holds within `r` (clipped to `view`) to `dst`
// through the grey remap, blended as `flags` asks, and clears them from
// `scratch`.
void blend_grey_element(Surface &dst, Surface &scratch, Rect r, Rect view, unsigned flags, const GreyRemap &remap)
{
    const std::array<uint8_t, 256> &grey = remap.grey;
    const uint8_t *ghost = remap.ghost;
    int x0 = std::max(r.x, view.x), y0 = std::max(r.y, view.y);
    int x1 = std::min(r.x + r.w, view.x + view.w), y1 = std::min(r.y + r.h, view.y + view.h);
    for (int py = y0; py < y1; py++) {
        uint8_t *src = scratch.row(py);
        uint8_t *out = dst.row(py);
        for (int px = x0; px < x1; px++) {
            if (src[px] == kMaskColour)
                continue;
            uint8_t c = grey[src[px]];
            if (flags & kDrawGhost)
                out[px] = ghost[c << 8 | out[px]];
            else if (flags & kDrawGhostUnder)
                out[px] = ghost[out[px] << 8 | c];
            else
                out[px] = c;
            src[px] = kMaskColour;
        }
    }
}

// Whether `held`'s layers show element `e`.
bool element_shown(const AnimElement &e, const Thing &held)
{
    if (!(e.flags & 0xffb0))
        return true;
    int layer = (e.flags >> 4) & 0x1f;
    return layer < kLayerCount && held.layers[size_t(layer)] == e.flags >> 9;
}

// Greyed (LbSpriteDrawRemap), element by element as draw_frame draws them:
// each sprite is drawn apart, then its pixels remapped before any
// see-through blending with the view (assumed to be LbSpriteDrawRemap's
// order).
void draw_held_staff_greyed(const ViewTarget &target, Point at, AnimStart start, const Thing &held,
                            const GameData &data, Surface &scratch)
{
    const AnimData &anims = data.anims();
    const Rect view = target.view;
    const GreyRemap remap = {grey_remap(data.palette()), data.ghost().data()};
    if (start.frame >= anims.frames.size())
        return;
    const AnimFrame &f = anims.frames[start.frame];
    const bool flip = start.flip != 0;
    scratch.set_clip(view);
    for (size_t i = f.list; i < anims.lists.size() && anims.lists[i] != kListEnd; i++) {
        if (anims.lists[i] >= anims.elements.size())
            continue;
        const AnimElement &e = anims.elements[anims.lists[i]];
        if (!element_shown(e, held) || e.sprite == 0)
            continue;
        Sprite s = data.sprites().at(e.sprite / 6);
        int ex = e.x - kElementBiasX;
        unsigned flags = e.flags & 0xf;
        if (flip) {
            flags ^= kDrawFlipX;
            ex = -ex - s.width;
        }
        Rect r = {at.x + ex, at.y + e.y - kElementBiasY, s.width, s.height};
        draw_sprite(scratch, r.x, r.y, s, flags & (kDrawFlipX | kDrawFlipY));
        blend_grey_element(target.dst, scratch, r, view, flags, remap);
    }
    scratch.reset_clip();
}

} // namespace

void draw_held_staff(const ViewTarget &target, const HireStaff &hire, const World &world, int cell,
                     const GameData &data, Surface &scratch)
{
    Surface &dst = target.dst;
    const Rect view = target.view;
    const Point camera = target.camera;
    if (!hire.placing() || hire.held >= world.things.size() || cell < 0 || cell >= kMapCells)
        return;
    const Thing &held = world.things[hire.held];
    const AnimData &anims = data.anims();
    size_t start = size_t(staff_held_anim(uint8_t(hire.held_kind))) + 1;
    if (start >= anims.starts.size())
        return;
    Point p = cell_to_world(cell % kMapSize, cell / kMapSize);
    int x = p.x - camera.x + view.x;
    int y = p.y - camera.y + view.y + kTileHeight / 2;
    uint32_t frame = anims.starts[start].frame;
    bool flip = anims.starts[start].flip != 0;
    if (hire.place_ok) {
        dst.set_clip(view);
        draw_frame(dst, {x, y}, frame, flip, data, {.layers = held.layers.data()});
        dst.reset_clip();
        return;
    }
    draw_held_staff_greyed(target, {x, y}, anims.starts[start], held, data, scratch);
}

} // namespace th

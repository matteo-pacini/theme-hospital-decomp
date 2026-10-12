// The staff list chart of Frame: the list, the selected member of staff
// with their face, skills and training (src/slist.cpp).

#include "core/ui/charts_internal.h"

#include "core/data/balance.h"
#include "core/world/staff.h"

#include <algorithm>

namespace th
{

namespace
{

// The faces file's layout (HiresCoords[162-165]) and draw_face's frame.
constexpr int kFaceW = 65, kHeadH = 29, kBodyH = 24, kLegsH = 22, kFaceH = kHeadH + kBodyH + kLegsH;
constexpr int kFaceBorder = 3, kFaceParts = 18;
constexpr uint8_t kFaceBorderColour = 0x9c;

// The staff of the kinds the staff list's `filter` shows: the player's,
// not applicants (the game's test in get_slist_staff and its kin).
bool slist_shows(const Thing &t, int filter)
{
    if (t.type != kThingStaff || t.owner != 0 || t.state == kStaffApplicant)
        return false;
    return ((t.subtype == kStaffDoctor || t.subtype == kStaffSurgeon) && (filter & 1)) ||
           (t.subtype == kStaffNurse && (filter & 2)) || (t.subtype == kStaffReceptionist && (filter & 8)) ||
           (t.subtype == kStaffHandyman && (filter & 4));
}
} // namespace

inline namespace charts_detail
{

void Frame::display_slist()
{
    switch (c_.state) {
    case 0:
        load_pic();
        c_.state = 1;
        c_.value = 0;
        c_.slist_filter = 1;
        c_.slist_page = 0;
        if (c_.slist_staff && c_.slist_staff < game_.world.things.size())
            open_slist_on(c_.slist_staff);
        draw_slist();
        break;
    case 1:
        copy_screen();
        draw_slist();
        process_slist_panel();
        break;
    case 2:
        r_.event = {EV_RETURN_TO_THING_WINDOW, 0, 0, uint16_t(c_.value)};
        break;
    case 3:
        return_from_screen();
        break;
    }
    slist_keys();
    if (!staff_on_page(c_.slist_page) && c_.slist_page > 0)
        c_.slist_page--;
    rleft_ = left_ = false;
    if (exit_quick_requester())
        c_.state = 3;
}

// The staff list opened on member of staff `id`: their kind's filter
// and the page they are on.
void Frame::open_slist_on(uint16_t id)
{
    switch (thing(id).subtype) {
    case kStaffNurse:
        c_.slist_filter = 2;
        break;
    case kStaffDoctor:
    case kStaffSurgeon:
        c_.slist_filter = 1;
        break;
    case kStaffHandyman:
        c_.slist_filter = 4;
        break;
    case kStaffReceptionist:
        c_.slist_filter = 8;
        break;
    default:
        break;
    }
    for (;;) {
        int prev = c_.value;
        c_.value = get_slist_staff(0);
        if (prev == c_.value) {
            c_.value = 0;
            break;
        }
        if (c_.value == id)
            break;
        if (c_.slist_cursor / 11 > 0 && c_.slist_cursor % 11 == 0 && c_.slist_page < 150 &&
            staff_on_page(c_.slist_page + 1))
            c_.slist_page++;
    }
}

void Frame::draw_slist()
{
    draw_slist_staff(c_.value);
    draw_slist_window(c_.value);
    draw_slist_overall();
    draw_slist_nums();
}

void Frame::slist_filter(int filter)
{
    c_.slist_filter = filter;
    c_.value = 0;
    c_.slist_cursor = 0;
    sound(kSfxSelectX);
}

void Frame::slist_keys()
{
    const uint8_t pad = keys_.pad_pressed;
    if (keys_.right || (pad & ChartKeys::kPadRight)) {
        if (c_.slist_filter != 8)
            slist_filter(c_.slist_filter << 1);
    } else if (keys_.left || (pad & ChartKeys::kPadLeft)) {
        if (c_.slist_filter != 1)
            slist_filter(c_.slist_filter >> 1);
    }
    if (keys_.down || (pad & ChartKeys::kPadDown))
        slist_key_down();
    else if (keys_.up || (pad & ChartKeys::kPadUp))
        slist_key_up();
}

// slist_keys, Down: the next member of staff, onto the next page past
// the last line.
void Frame::slist_key_down()
{
    int prev = c_.value;
    c_.value = get_slist_staff(0);
    if (prev == c_.value)
        return;
    sound(kSfxSelectX);
    if (c_.slist_cursor / 11 > 0 && c_.slist_cursor % 11 == 0) {
        if (c_.slist_page < 150 && staff_on_page(c_.slist_page + 1))
            c_.slist_page++;
        sound(kSfxSelectX);
    }
}

// slist_keys, Up: the member of staff before, back a page past the
// first line.
void Frame::slist_key_up()
{
    int prev = c_.value;
    c_.value = get_slist_staff(1);
    if (prev == c_.value)
        return;
    sound(kSfxSelectX);
    if (c_.slist_cursor / 10 > 0 && c_.slist_cursor % 10 == 0) {
        sound(kSfxSelectX);
        if (c_.slist_page > 0) {
            c_.slist_page--;
            for (int i = 0; i < 10; i++)
                c_.value = get_slist_staff(1);
        }
    }
}

int Frame::get_slist_staff(int dir)
{
    const auto &things = game_.world.things;
    if (dir == 0) {
        for (size_t i = size_t(c_.value) + 1; i < things.size(); i++) {
            if (slist_shows(things[i], c_.slist_filter)) {
                c_.slist_cursor++;
                return int(i);
            }
        }
    } else {
        for (int i = c_.value - 1; i >= 1; i--) {
            if (slist_shows(things[size_t(i)], c_.slist_filter)) {
                c_.slist_cursor--;
                return i;
            }
        }
    }
    return c_.value;
}

bool Frame::staff_on_page(int page) const
{
    int listed = 0;
    const auto &things = game_.world.things;
    for (size_t i = 1; i < things.size(); i++) {
        if (slist_shows(things[i], c_.slist_filter)) {
            if (listed >= page * 10 && listed < (page + 1) * 10)
                return true;
            listed++;
        }
    }
    return false;
}

// draw_gauge (src/req3.cpp): `value` clamped to min-max as a share of
// `range`, filled from the left if `up`, else the rest.
void Frame::draw_gauge(int value, const Rect &box, int max, int min, int range, bool up)
{
    value = std::clamp(value, min, max);
    int filled = up ? box.w * value / range : box.w - box.w * value / range;
    for (int i = 0; i < filled; i++)
        reqspr(box.x + i, box.y, box.h);
}

void Frame::draw_slist_staff(int selected)
{
    int height = line_height(font(1));
    for (int row = 0; row < 3; row++) {
        const Rect &b = kBoxes[row + 61];
        text(1, b, {0, (b.h - height) / 2}, str(TextSection::Slist, row + 2), kTextCentre);
    }
    int row = 0, listed = 0;
    const auto &things = game_.world.things;
    uint8_t red = colours().red;
    for (size_t id = 1; id < things.size() && row < 10; id++) {
        const Thing &s = things[id];
        if (!slist_shows(s, c_.slist_filter))
            continue;
        if (listed >= c_.slist_page * 10 && listed < (c_.slist_page + 1) * 10) {
            draw_slist_staff_line(row, listed, int(id), selected == int(id), red);
            row++;
        }
        listed++;
    }
    if (row < 10) {
        const Rect &b = kBoxes[row + 104];
        draw_box(dst_, b.x, b.y, b.w, (10 - row) * b.h, 0x77);
    }
    int pages = 0;
    while (staff_on_page(pages))
        pages++;
    if (pages > 1) {
        const Rect &b = kBoxes[114];
        uint8_t pos = uint8_t(b.h / (pages - 1) * c_.slist_page);
        reqspr(b.x, pos + b.y, b.w);
    }
}

// draw_slist_staff, the page's `row`th line: member of staff `id`, the
// `listed`th in the list, framed in colour `frame` if selected; the mouse
// highlights and picks it.
void Frame::draw_slist_staff_line(int row, int listed, int id, bool selected, uint8_t frame)
{
    const Thing &s = thing(id);
    const Rect &line = kBoxes[row + 104];
    int f = 1;
    if (over(mx_, my_, line.x, line.y, line.w, line.h)) {
        if (c_.slist_cursor != row + c_.slist_page * 10 + 1)
            sound(kSfxHighlight5);
        f = 2;
        c_.slist_cursor = c_.slist_page * 10 + row + 1;
    }
    if (selected) {
        f = 2;
        int fx = line.x, fy = line.y - 1;
        draw_line(dst_, fx, fy, fx + line.w, fy, frame);
        draw_line(dst_, fx, fy, fx, fy + line.h, frame);
        draw_line(dst_, fx + line.w, fy, fx + line.w, fy + line.h, frame);
        draw_line(dst_, fx, fy + line.h, fx + line.w, fy + line.h, frame);
        draw_slist_button_overlays(s);
    }
    text(f, kCoords[row + 114].x, kCoords[row + 114].y, format("%d", listed + 1));
    std::string name = format("%c. %s%s", s.initial, std::string(str(TextSection::StaffNames1, s.surname_a)).c_str(),
                              std::string(str(TextSection::StaffNames2, s.surname_b)).c_str());
    text(f, kCoords[row + 124].x, kCoords[row + 124].y, name);
    text(f, kCoords[row + 134].x, kCoords[row + 134].y, format("$%d", s.salary));
    draw_gauge(s.happiness, kBoxes[row + 71], 100, 0, 100, true);
    draw_gauge(s.tiredness, kBoxes[row + 81], 1000, 0, 1000, false);
    draw_gauge(s.ability, kBoxes[row + 91], 1000, 30, 970, true);
    if (mleft_ && over(mx_, my_, line.x, line.y, line.w, line.h)) {
        if (id != c_.value)
            sound(kSfxSelectX);
        c_.value = id;
    }
}

// draw_face (src/req2.cpp): the face in its frame, its bottom left at
// (x, y).
void Frame::draw_face(const Thing &s, int x, int y)
{
    const size_t heads = size_t(kFaceW) * kHeadH * kFaceParts;
    const size_t bodies = size_t(kFaceW) * kBodyH * kFaceParts;
    const struct {
        size_t offset;
        int rows;
    } parts[3] = {{size_t(s.face_head) * kFaceW * kHeadH, kHeadH},
                  {heads + size_t(s.face_body) * kFaceW * kBodyH, kBodyH},
                  {heads + bodies + size_t(s.face_legs) * kFaceW * kLegsH, kLegsH}};
    int row = y - kFaceH - kFaceBorder;
    for (const auto &part : parts) {
        for (int r = 0; r < part.rows; r++, row++) {
            size_t src = part.offset + size_t(r) * kFaceW;
            if (src + kFaceW > c_.faces.size())
                continue;
            for (int col = 0; col < kFaceW; col++)
                plot(dst_, x + kFaceBorder + col, row, c_.faces[src + size_t(col)]);
        }
    }
    int top = y - kFaceH - kFaceBorder * 2, w = kFaceBorder * 2 + kFaceW, h = kFaceH + kFaceBorder * 2;
    draw_box(dst_, x, top, w, kFaceBorder, kFaceBorderColour);
    draw_box(dst_, x, top, kFaceBorder, h, kFaceBorderColour);
    draw_box(dst_, x, y - kFaceBorder, w, kFaceBorder, kFaceBorderColour);
    draw_box(dst_, x + kFaceW + kFaceBorder, top, kFaceBorder, h, kFaceBorderColour);
}

// draw_slist_training_bar (src/slist.cpp): `value` of `max` as a row of
// pixels, one per max / width of the bar's box.
void Frame::draw_slist_training_bar(int x, int y, int value, int max)
{
    const int width = kBoxes[115].w;
    for (int i = 0; i < value / (max / width) && i < width; i++)
        reqspr(x + kBoxes[115].x + i, y + kBoxes[115].y, 13);
}

// The staff member's map view in kSlistView is drawn over this by the
// caller (slist_view).
void Frame::draw_slist_window(int id)
{
    if (!id)
        return;
    const Thing &s = thing(id);
    draw_face(s, kSlistFace.x, kSlistFace.y);
    if (s.subtype == kStaffDoctor || s.subtype == kStaffSurgeon) {
        const bool shows[3] = {(s.skills & kSkillJunior) && !(s.skills & kSkillConsultant),
                               (s.skills & kSkillDoctor) && !(s.skills & kSkillConsultant) &&
                                   !(s.skills & kSkillJunior),
                               (s.skills & kSkillConsultant) != 0};
        for (int i = 0; i < 3; i++)
            if (shows[i])
                reqspr(kBoxes[i + 64].x, kBoxes[i + 64].y, kBoxes[i + 64].h);
        // The qualifications: the icon if held, else the faded icon and
        // the training bar once any training is done.
        const struct {
            uint16_t skill;
            int training;
        } skills[3] = {{kSkillSurgeon, s.surgeon_training},
                       {kSkillPsychiatrist, s.psych_training},
                       {kSkillResearcher, s.research_training}};
        for (int i = 0; i < 3; i++) {
            const Rect &b = kBoxes[i + 67];
            if (s.skills & skills[i].skill) {
                reqspr(b.x, b.y, b.h);
            } else if (skills[i].training) {
                reqspr(b.x, b.y, b.h - 3);
                draw_slist_training_bar(b.x, b.y, skills[i].training,
                                        game_.settings().gbv.ability_threshold[size_t(i)]);
            }
        }
    }
    int delay = std::clamp(int(s.work_delay), 0, 10);
    int count = delay * 4200 / 1000;
    for (int j = 0; j < count; j++)
        reqspr(kBoxes[70].x + j, kBoxes[70].y, kBoxes[70].h);
}

void Frame::draw_slist_overall()
{
    int happiness = 0, tiredness = 0, ability = 0, count = 0;
    for (const Thing &s : game_.world.things) {
        if (slist_shows(s, c_.slist_filter)) {
            happiness += s.happiness;
            tiredness += s.tiredness;
            ability += s.ability;
            count++;
        }
    }
    if (!(c_.slist_filter & 1)) {
        // draw_slist_coverup: the doctors' parts.
        for (int b : {116, 117})
            draw_box(dst_, kBoxes[b].x, kBoxes[b].y, kBoxes[b].w, kBoxes[b].h, 0x76);
    }
    if (count) {
        draw_gauge(happiness / count, kBoxes[101], 100, 0, 100, true);
        draw_gauge(tiredness / count, kBoxes[102], 1000, 0, 1000, false);
        draw_gauge(ability / count, kBoxes[103], 1000, 30, 970, true);
    }
}

void Frame::process_slist_panel()
{
    if (rleft_) {
        for (int i = kSlistButtons; kButtons[i].w && kButtons[i].h; i++) {
            const ScreenButton &b = kButtons[i];
            if (over(mx_, my_, b.x, b.y, b.w, b.h)) {
                process_slist_button(i - kSlistButtons);
                reqspr(b.sprite_x, b.sprite_y, b.sprite);
            }
        }
        if (over(mx_, my_, kSlistView.x, kSlistView.y, kSlistView.w, kSlistView.h) && c_.value) {
            c_.state = 2;
            sound(kSfxSelectX);
        }
        if (c_.value)
            draw_slist_button_overlays(thing(c_.value));
    }
    if (mleft_) {
        for (int i = kSlistButtons; kButtons[i].w && kButtons[i].h; i++) {
            const ScreenButton &b = kButtons[i];
            if (over(mx_, my_, b.x, b.y, b.w, b.h)) {
                reqspr(b.sprite_x, b.sprite_y, b.sprite);
                break;
            }
        }
        if (c_.value)
            draw_slist_button_overlays(thing(c_.value));
    }
}

void Frame::process_slist_button(int button)
{
    static constexpr int kFilters[4] = {1, 2, 8, 4};
    switch (button) {
    case 0:
    case 1:
    case 2:
    case 3:
        if (c_.slist_filter != kFilters[button])
            sound(kSfxSelectX);
        c_.slist_filter = kFilters[button];
        c_.value = 0;
        c_.slist_cursor = 0;
        break;
    case 4:
        if (c_.value) {
            sound(kSfxSelectX);
            // open_yes_no_requester.
            r_.event = {EV_OPEN_YES_NO, 0, 0, 0};
            r_.yes = {EV_SACK_STAFF, uint16_t(c_.value), 0, 0};
            r_.no = {EV_CANCEL_SACK, uint16_t(c_.value), 0, 0};
            r_.question = 5;
            c_.value = 0;
        }
        break;
    case 5:
        if (c_.value) {
            r_.event = {EV_INCREASE_STAFF_SALARIES, uint16_t(thing(c_.value).salary / 10), 0, uint16_t(c_.value)};
            sound(kSfxBonusAll2);
        }
        break;
    case 6:
        if (c_.value) {
            r_.event = {EV_PAY_STAFF_BONUS, uint16_t(thing(c_.value).salary / 10), 0, uint16_t(c_.value)};
            sound(kSfxBonus2);
        }
        break;
    case 7:
        c_.state = 3;
        sound(kSfxNo4);
        break;
    case 8:
    case 9: {
        uint8_t sfx = kSfxWrong2;
        int page = button == 8 ? c_.slist_page - 1 : c_.slist_page + 1;
        if (page >= 0 && page <= 150 && staff_on_page(page)) {
            c_.slist_page = page;
            for (int i = 0; i < 10; i++)
                c_.value = get_slist_staff(button == 8 ? 1 : 0);
            sfx = kSfxSelectX;
        }
        sound(sfx);
        break;
    }
    }
}

void Frame::draw_slist_nums()
{
    int tab = c_.slist_filter & 1 ? 0 : c_.slist_filter & 2 ? 1 : c_.slist_filter & 8 ? 2 : 3;
    const ScreenButton &b = kButtons[kSlistButtons + tab];
    reqspr(b.sprite_x, b.sprite_y, b.sprite);
    int counts[4] = {};
    for (const Thing &s : game_.world.things) {
        if (s.type != kThingStaff || s.owner != 0 || s.state == kStaffApplicant)
            continue;
        if (s.subtype == kStaffDoctor || s.subtype == kStaffSurgeon)
            counts[0]++;
        else if (s.subtype == kStaffNurse)
            counts[1]++;
        else if (s.subtype == kStaffHandyman)
            counts[2]++;
        else if (s.subtype == kStaffReceptionist)
            counts[3]++;
    }
    for (int i = 0; i < 4; i++)
        text(0xd, kCoords[i + 144].x, kCoords[i + 144].y, format("%d", counts[i]));
}

void Frame::draw_slist_button_overlays(const Thing &s)
{
    text(1, kCoords[149].x, kCoords[149].y, format("$%d", s.salary / 10 + s.salary));
    text(1, kCoords[150].x, kCoords[150].y, format("$%d", s.salary / 10));
}

} // namespace charts_detail

} // namespace th

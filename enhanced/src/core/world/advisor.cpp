#include "core/world/advisor_internal.h"

#include "core/data/gamedata.h"
#include "core/render/panel.h"
#include "core/world/build.h"
#include "core/world/staff.h"
#include "core/world/world.h"

#include <bit>

namespace th
{

namespace
{

// The game's advice table (advice[], src/advisor.cpp): situation, duration
// and repeat weight.
constexpr std::array<Advice, kAdviceCount> kAdviceTable = {{
    {0, 0, 0},          {0x6a, 0xfa, 0},     {0x1c, 0xfa, 0x1e}, {0x87, 0xfa, 0x1e}, {0x8d, 0xfa, 0x1e},
    {0x8a, 0xfa, 0x1e}, {0x8b, 0xfa, 0x1e},  {0x8c, 0xfa, 0x1e}, {0x15, 0xfa, 0},    {0x4d, 0xfa, 0},
    {0x48, 0xfa, 0x14}, {0x5e, 0xfa, 0x14},  {0x78, 0xfa, 0x14}, {0x3, 0xfa, 0x14},  {0x6f, 0xfa, 0x1e},
    {0x72, 0xfa, 0x1e}, {0x75, 0xfa, 0x14},  {0x4, 0x82, 0},     {0x5, 0x82, 0},     {0xc, 0xfa, 0},
    {0x58, 0xfa, 0x14}, {0x7b, 0xfa, 0},     {0x6, 0xfa, 0x14},  {0x63, 0xfa, 0x14}, {0x7, 0xfa, 0x14},
    {0x8, 0xfa, 0x14},  {0x9, 0xfa, 0x14},   {0xa, 0xfa, 0x14},  {0x14, 0xfa, 0x14}, {0x76, 0xfa, 0x14},
    {0x77, 0xfa, 0x14}, {0xb, 0xfa, 0x14},   {0xc, 0xfa, 0x14},  {0xd, 0xfa, 0x14},  {0xe, 0xfa, 0x14},
    {0xf, 0xfa, 0x14},  {0x10, 0xfa, 0x14},  {0x11, 0xfa, 0x14}, {0x12, 0xfa, 0x14}, {0x13, 0xfa, 0x14},
    {0x86, 0xfa, 0},    {0x7a, 0xfa, 0},     {0x79, 0x15e, 0},   {0x44, 0xfa, 0},    {0x89, 0x15e, 0},
    {0x16, 0x15e, 0},   {0x17, 0xfa, 0},     {0x18, 0xfa, 0},    {0x19, 0xfa, 0},    {0x1a, 0xfa, 0},
    {0x1b, 0xfa, 0},    {0x45, 0xfa, 0x14},  {0x46, 0xfa, 0x14}, {0x88, 0xfa, 0x14}, {0x41, 0xfa, 0x14},
    {0x42, 0xfa, 0x14}, {0x43, 0xfa, 0x14},  {0x64, 0xfa, 0x14}, {0x65, 0xfa, 0x14}, {0x4b, 0xfa, 0x14},
    {0x5c, 0xfa, 0x14}, {0x5a, 0xfa, 0xf},   {0x5b, 0xfa, 0x14}, {0x4a, 0xfa, 0x14}, {0x49, 0xfa, 0x14},
    {0x66, 0x96, 0x14}, {0x54, 0x15e, 0x14}, {0x67, 0x96, 0x14}, {0x55, 0x96, 0x14}, {0x56, 0x96, 0x14},
    {0x50, 0x64, 0x14}, {0x53, 0x15e, 0x14}, {0x4f, 0x96, 0x14}, {0x57, 0x64, 0x14}, {0x4e, 0x15e, 0x14},
    {0x5d, 0x64, 0x14}, {0x47, 0x4b, 0x14},  {0x52, 0x4b, 0x14}, {0x4c, 0x64, 0x14}, {0x51, 0x64, 0x14},
    {0x69, 0x64, 0x1e}, {0x68, 0x64, 0x1e},  {0x6e, 0x4b, 0x14}, {0x6b, 0xfa, 0x1e}, {0, 0, 0},
}};

// advisor_specials (src/tables.cpp): the up states' occasional animations.
constexpr std::array<uint8_t, 2> kAdvisorSpecials = {9, 10};

// The speech bubble's layout in high resolution (HiresCoords 0x93-0x9e,
// src/coords.cpp, through AdvisorLayout in src/advtext.cpp): the wrap width
// is middle pieces * 17 + 42 + 11, less the text's margins of 8 and 4.
constexpr int kMiddleTextWidth = 17;
constexpr int kRightTextWidth = 42;
constexpr int kLeftTextWidth = 11;
constexpr int kTextLeftMargin = 8;
constexpr int kTextRightMargin = 4;

// The rest of the layout (HiresCoords 0x48, 0x9e, 0x93, 0x96-0x98): the
// advisor's x, the bubble's right end, the y of its other pieces and the
// message's y for three, two and one lines.
constexpr int kAdvisorX = 560;
constexpr int kRightPieceX = 515, kRightPieceY = 375;
constexpr int kBubbleY = 390;
constexpr int kTextY3 = 395, kTextY2 = 400, kTextY1 = 403;

// The panel sprites of the bubble: its left end, a middle piece, its right
// end (game_panel[37] to [39]).
constexpr size_t kBubbleLeft = 37, kBubbleMiddle = 38, kBubbleRightEnd = 39;

// statvar.view_height in high resolution: the message is clipped to the
// view above the panel.
constexpr int kViewHeight = 432;

// Right clicks on the bubble (HiresCoords 0x93's y, 0x92) and on him
// (0x90 to 0x91), process_mouse (src/mouse.cpp).
constexpr int kBubbleTop = 390;
constexpr int kBubbleRight = 531, kBubbleBottom = 428;
constexpr int kAdvisorLeft = 531, kAdvisorTop = 336, kAdvisorRight = 619, kAdvisorBottom = 428;

void enter_state(Advisor &a, const AnimData &anims, uint8_t state)
{
    a.anim_state = state;
    uint16_t start = kAdvisorAnims[state];
    a.anim_frame = start < anims.starts.size() ? uint16_t(anims.starts[start].frame) : 0;
}

// advisor_character (src/advtext.cpp) without its drawing.
void advisor_character(Advisor &a, const AdvisorInputs &in, const AnimData &anims, uint32_t &safeseed)
{
    auto saferand = [&] {
        safeseed = std::rotr(safeseed * 0x24a1u + 0x24dfu, 13);
        return uint16_t(safeseed);
    };
    // get_advisor
    if (a.advice_id && a.anim_state == 0)
        enter_state(a, anims, 1);
    uint32_t speed = 0;
    bool advance_at_end = false;
    switch (a.anim_state) {
    case 1:
    case 4:
    case 5:
    case 6:
    case 8:
        speed = 1;
        advance_at_end = true;
        a.state_advanced = false;
        break;
    case 9:
    case 10:
        speed = 4;
        advance_at_end = true;
        break;
    case 2:
    case 3:
    case 7:
        if (a.anim_state == 2)
            a.state_advanced = false;
        if (a.anim_state != 7 && saferand() % 50 == 0)
            enter_state(a, anims, kAdvisorSpecials[saferand() % 2]);
        speed = 2;
        break;
    default:
        break;
    }
    a.drawn = speed != 0;
    if (!speed)
        return;
    a.drawn_frame = a.anim_frame;
    if (in.game_turn % speed == 0 && !in.paused && a.anim_frame < anims.frames.size()) {
        a.anim_frame = anims.frames[a.anim_frame].next;
        if (advance_at_end && a.anim_frame < anims.frames.size() && (anims.frames[a.anim_frame].flags & 1))
            enter_state(a, anims, kNextAdvisorState[a.anim_state]);
    }
}

// format_advisor_text (src/advtext.cpp): wraps the message into a bubble
// `segments` middle pieces wide, if it fits in three lines.
void format_advisor_text(Advisor &a, const Font &font, int segments)
{
    int max_width = segments * kMiddleTextWidth + kRightTextWidth + kLeftTextWidth - kTextLeftMargin - kTextRightMargin;
    // The game indexes the font by character - 31, past its start for
    // control characters; they count nothing here.
    auto width_of = [&](uint8_t c) { return c == ' ' ? font.space_width() : c > ' ' ? font.glyph(c).width : 0; };
    const std::string &text = a.text;
    size_t end = text.size();
    std::string wrapped;
    size_t p = 0, brk = 0, line_start = 0;
    int width = 0;
    int lines = 0;
    do {
        do {
            width += width_of(uint8_t(text[p]));
            char c = text[p];
            if (c == ' ' || c == ',' || c == '.' || c == ';' || c == ':')
                brk = p;
            p++;
        } while (width <= max_width && p != end);
        if (p == end)
            brk = p;
        width = 0;
        lines++;
        wrapped.append(text, line_start, brk - line_start);
        wrapped += '\n';
        if (p != end) {
            p = brk + 1;
            brk = p;
            line_start = p;
        }
    } while (lines < 3 && p != end);
    if (p == end) {
        a.text = wrapped;
        a.line_count = uint8_t(lines);
        a.extra_segments = uint8_t(segments - kAdvisorMinSegments);
    }
}

// advisor_frame's ageing of the advice given: whether it is old enough to
// give way to other advice, and ending it when its time is up.
bool advisor_frame_advice_stale(Advisor &a, const AdvisorInputs &in)
{
    if (!a.advice_id)
        return false;
    Advice &shown = a.advice[size_t(a.advice_id)];
    if (in.requester_open) {
        shown.time = in.game_turn - shown.duration / 4;
        return false;
    }
    bool stale = false;
    // 16 bits, as the game keeps it.
    uint16_t elapsed = uint16_t(in.game_turn - shown.time);
    if (elapsed > 100 || a.dismiss)
        stale = true;
    if (elapsed >= shown.duration || a.dismiss)
        a.advice_id = 0;
    return stale;
}

// advisor_frame's search for the first advice due and fitting the
// situation.
void advisor_frame_find_advice(Advisor &a, const AdvisorInputs &in, uint32_t &safeseed)
{
    for (int i = 1; a.advice[size_t(i)].id; i++) {
        const Advice &advice = a.advice[size_t(i)];
        if (in.game_turn > advice.time &&
            uint32_t(advice.times_given * advice.repeat_weight << 10) / (in.game_turn - advice.time) < 10 &&
            find_advice(a, i, in, safeseed)) {
            a.rewrap = true;
            return;
        }
    }
}

// advisor_frame's bubble, or his going down, while he is up.
void advisor_frame_bubble(Advisor &a, const AnimData &anims)
{
    if (a.advice_id && !a.dismiss) {
        a.bubble = true;
    } else if (--a.time_left <= 0 || a.dismiss) {
        a.dismiss = false;
        a.time_left = -1;
        enter_state(a, anims, kNextAdvisorState[a.anim_state]);
        a.state_advanced = false;
    } else if (!a.state_advanced) {
        enter_state(a, anims, kNextAdvisorState[a.anim_state]);
        a.state_advanced = true;
        a.dismiss = false;
    }
}

} // namespace

StaffCounts count_staff(const World &world)
{
    StaffCounts c;
    for (const Thing &t : world.things) {
        if (t.type != kThingStaff || t.owner != 0 || t.state == kStaffApplicant)
            continue;
        c.total++;
        c.by_kind[t.subtype == kStaffSurgeon ? kStaffDoctor : t.subtype]++;
        if (t.skills & kSkillReceptionist)
            c.receptionists++;
        if (t.skills & kSkillDoctor)
            c.doctors++;
    }
    return c;
}

void start_advisor(Advisor &a, bool tutorial)
{
    // reset_advisor
    a.advice = kAdviceTable;
    a.advice_id = 0;
    a.time_left = 0;
    a.forced_advice_id = 0;
    a.mouse_advice_id = 0;
    a.text.clear();
    a.event_flags = 0;
    // setup_new_level and the area screen.
    a.tutorial_shown = {};
    a.tutorial_step = 0x1d;
    a.tutorial_highlight = 0;
    a.flags = tutorial ? kAdvisorTutorial : 0;
    a.anim_state = 0;
    a.anim_frame = 0;
    a.state_advanced = false;
    a.line_count = 0;
    a.extra_segments = 0;
    a.rewrap = false;
    a.dismiss = false;
    a.drawn = false;
    a.bubble = false;
}

void advisor_frame(Advisor &a, const AdvisorInputs &in, const AnimData &anims, uint32_t &safeseed)
{
    a.drawn = false;
    a.bubble = false;
    // display_advisor does nothing in the rat game.
    if (in.world->rats.fest_level)
        return;
    // What drawing the blueprint and setting the edit room requester leave in
    // the flags, before the frame's advisor.
    a.flags = blueprint_draw_flags(*in.world, a.flags);
    a.flags = in.world->placing.furnished ? uint8_t(a.flags | kAdvisorRoomFurnished)
                                          : uint8_t(a.flags & ~kAdvisorRoomFurnished);
    if (!a.enabled)
        return;
    if (in.object_place_ok) {
        if (*in.object_place_ok)
            a.flags &= ~kAdvisorBadPlacement;
        else
            a.flags |= kAdvisorBadPlacement;
    }
    if (a.flags & kAdvisorTutorial) {
        if (tutorial(a, in) == 0 || a.text.empty())
            return;
        a.advice_id = 1;
        advisor_character(a, in, anims, safeseed);
        a.bubble = kAdvisorUpStates[a.anim_state];
        return;
    }

    if (advisor_frame_advice_stale(a, in) || a.advice_id == 0)
        advisor_frame_find_advice(a, in, safeseed);
    advisor_character(a, in, anims, safeseed);
    if (!kAdvisorUpStates[a.anim_state])
        return;
    advisor_frame_bubble(a, anims);
}

void monthly_update_of_advisor_counts(Advisor &a)
{
    a.advice[14].times_given = 0;
    a.advice[15].times_given = 0;
}

void toggle_advisor(Advisor &a, World &world)
{
    a.enabled = !a.enabled;
    world.announce = {};
    world.announce_stuck = false;
}

void wrap_advisor_text(Advisor &a, const Font &font)
{
    a.no_pieces = kAdvisorMinSegments;
    do {
        if (!a.text.empty())
            format_advisor_text(a, font, a.no_pieces);
        a.no_pieces++;
    } while (a.no_pieces <= kAdvisorMaxSegments && !a.line_count);
    a.rewrap = false;
}

bool advisor_hit(const Advisor &a, int x, int y)
{
    if (!kAdvisorUpStates[a.anim_state])
        return false;
    // The bubble's left edge is compared unsigned, as in the game.
    return (y >= kBubbleTop && uint16_t(a.box_left) <= x && x <= kBubbleRight && y <= kBubbleBottom) ||
           (x >= kAdvisorLeft && y >= kAdvisorTop && x <= kAdvisorRight && y <= kAdvisorBottom);
}

void draw_advisor(UiLayer &layer, Advisor &a, const GameData &data, const Font &font)
{
    if (a.drawn) {
        // draw_a_frame
        const AnimData &anims = data.anims();
        int y = panel_top(data);
        if (a.drawn_frame < anims.frames.size()) {
            for (size_t i = anims.frames[a.drawn_frame].list; i < anims.lists.size() && anims.lists[i] != kListEnd;
                 i++) {
                uint16_t id = anims.lists[i];
                if (id >= anims.elements.size() || anims.elements[id].sprite == 0)
                    continue;
                const AnimElement &e = anims.elements[id];
                layer.draw_sprite(kAdvisorX + e.x - kElementBiasX, y + e.y - kElementBiasY,
                                  data.sprites().at(e.sprite / 6), e.flags & 0xf, data.ghost().data());
            }
        }
    }
    if (!a.bubble || a.text.empty())
        return;
    // advisor_text_box
    if (a.rewrap)
        wrap_advisor_text(a, font);
    // The game leaves y unset for a message not wrapped (no lines).
    int y = a.line_count == 3 ? kTextY3 : a.line_count == 2 ? kTextY2 : kTextY1;
    if (a.no_pieces > kAdvisorMaxSegments)
        return;
    const SpriteSheet &panel = data.panel();
    Sprite left = panel.at(kBubbleLeft);
    Sprite middle = panel.at(kBubbleMiddle);
    Sprite right = panel.at(kBubbleRightEnd);
    int pieces = a.extra_segments + kAdvisorMinSegments;
    layer.draw_sprite(kRightPieceX, kRightPieceY, right);
    layer.draw_sprite(kRightPieceX - left.width - pieces * middle.width, kBubbleY, left);
    for (int i = 0; i < pieces; i++)
        layer.draw_sprite(kRightPieceX - (pieces - i) * middle.width, kBubbleY, middle);
    // As the game has it: one more middle piece and the right end's width.
    a.box_left = int16_t(kRightPieceX - middle.width * (pieces + 1) - right.width);
    layer.draw_text(font, kRightPieceX - pieces * middle.width + kTextLeftMargin - left.width, y, a.text,
                    {{0, 0, kUiWidth}, {0, 0, kUiWidth, kViewHeight}});
}

} // namespace th

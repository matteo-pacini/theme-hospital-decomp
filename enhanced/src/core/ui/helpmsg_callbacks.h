#pragma once

#include <cstdint>
#include <string>

namespace th
{

class Game;
class GameData;
class LanguageText;
class Interface;
class SpriteSheet;
class Font;
struct World;
struct HireStaff;
struct InfoWindows;
struct Charts;
struct ObjectPlacing;
struct ObjectUi;
struct BuildUi;
struct UiMouse;

struct HelpContext;

// A rectangle of a requester's help hotspots (the game's HelpHotspot,
// include/hosp.h), from the hireshelp table. x0 < 0 ends a group.
struct HelpHotspot {
    // 0x0: Left; -1 ends a requester's group, -2 the table.
    int16_t x0 = 0;
    // 0x2: Top.
    int16_t y0 = 0;
    // 0x4: Right.
    int16_t x1 = 0;
    // 0x6: Bottom.
    int16_t y1 = 0;
    // 0x8: Help text index in the low word, requester slot (req_indices
    // index) in the high word.
    int32_t text_and_slot = 0;
    // 0xa: Builds the hotspot's text when set.
    void (*func)(HelpContext &) = nullptr;

    // The help text index (HelpHotspot.text).
    uint16_t text() const { return uint16_t(text_and_slot & 0xffff); }
    // The requester slot (HelpHotspot.req_slot).
    uint16_t req_slot() const { return uint16_t((text_and_slot >> 16) & 0xffff); }
};

// The help box's state and the text it shows (othervar's help fields).
struct HelpBox {
    // othervar.help_msg: the tooltip or help page's text.
    std::string msg;
    // othervar.help_x/y/w/h.
    int x = 0, y = 0, w = 0, h = 0;
    // othervar.help_group and help_style (1 tooltip, 2 framed page).
    uint8_t group = 0, style = 0;
    // othervar.help_close_x/y: the close icon of a framed page.
    int close_x = 0, close_y = 0;
    // othervar.help_hotspot: the current hotspot's index, 0 for none.
    uint16_t hotspot = 0;
    // othervar.help_hover_start and help_shown_time, in clock_ms units.
    uint32_t hover_start = 0, shown_time = 0;
    // othervar.clock_ms: the frame's clock, set by the caller.
    uint32_t clock_ms = 0;
    // othervar.forced_help_hotspot and forced_help_group.
    uint16_t forced_hotspot = 0;
    uint8_t forced_group = 0;
    // othervar.help_page and help_sub.
    uint8_t page = 0, sub = 0;
    // othervar.help_close_hover.
    bool close_hover = false;
    // othervar.screen_pic_loaded: a full-screen picture is behind the box, so
    // it is drawn in white rather than the interface's light grey.
    bool screen_pic_loaded = false;
};

// What a help callback may read, so it does not touch globals. tips-callbacks
// fills the entries it needs; the fields a callback does not use stay null.
struct HelpContext {
    Game *game = nullptr;
    World *world = nullptr;
    const GameData *data = nullptr;
    const LanguageText *text = nullptr;
    Interface *ui = nullptr;
    HireStaff *hire = nullptr;
    InfoWindows *info = nullptr;
    Charts *charts = nullptr;
    ObjectPlacing *placing = nullptr;
    ObjectUi *object_ui = nullptr;
    BuildUi *build = nullptr;
    const UiMouse *mouse = nullptr;
    const SpriteSheet *pulldv = nullptr;
    // The help system's state and the hotspot whose callback is running.
    HelpBox *help = nullptr;
    const HelpHotspot *hotspot = nullptr;
};

// Writes the running hotspot's plain help_text entry into ctx.help->msg, the
// fallback every callback starts from.
void help_default_text(HelpContext &ctx);

// One callback per hireshelp entry that has one; each builds its hotspot's
// text into ctx.help->msg.
void req3_help_if_diagnosed(HelpContext &ctx);
void req3_help_if_curable(HelpContext &ctx);
void req3_help_if_queueing(HelpContext &ctx);
void manager_help_displayed_ins(HelpContext &ctx);
void manager_help_pointed_ins(HelpContext &ctx);
void overscr_help_add_name(HelpContext &ctx);
void overscr_help_win_criteria(HelpContext &ctx);
void panel_rep_help(HelpContext &ctx);
void panel_watch_help(HelpContext &ctx);
void req10_help_room(HelpContext &ctx);
void req11_help_minus(HelpContext &ctx);
void req11_help_object(HelpContext &ctx);
void req11_help_plus(HelpContext &ctx);
void req12_help_if_doctor(HelpContext &ctx);
void req12_help_if_research(HelpContext &ctx);
void req12_help_if_shrink(HelpContext &ctx);
void req12_help_if_surgeon(HelpContext &ctx);
void req2_help_if_doctor(HelpContext &ctx);
void req2_help_if_psycho(HelpContext &ctx);
void req2_help_if_researcher(HelpContext &ctx);
void req2_help_if_selected(HelpContext &ctx);
void req2_help_if_surgeon(HelpContext &ctx);
void req4_help_if_doctor(HelpContext &ctx);
void req4_help_if_research(HelpContext &ctx);
void req4_help_if_shrink(HelpContext &ctx);
void req4_help_if_surgeon(HelpContext &ctx);
void slist_ability_help_if_present(HelpContext &ctx);
void slist_happiness_help_if_present(HelpContext &ctx);
void slist_help_if_doctor(HelpContext &ctx);
void slist_help_if_researcher(HelpContext &ctx);
void slist_help_if_shrink(HelpContext &ctx);
void slist_help_if_surgeon(HelpContext &ctx);
void slist_salary_help_if_present(HelpContext &ctx);
void slist_tiredness_help_if_present(HelpContext &ctx);
void drugs_help_concentrate(HelpContext &ctx);
void drugs_help_cure_type(HelpContext &ctx);
void drugs_help_handled(HelpContext &ctx);

} // namespace th

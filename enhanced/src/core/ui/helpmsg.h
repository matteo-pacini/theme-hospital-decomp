#pragma once

#include "core/data/palette.h"
#include "core/data/sprites.h"
#include "core/render/surface.h"
#include "core/render/text.h"
#include "core/ui/helpmsg_callbacks.h"
#include "core/ui/windows.h"

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace th
{

// The hireshelp table (helpmsg_table.cpp), the hireshelp entry count and, per
// help group, where its hotspots start (setup_help_indices).
extern const HelpHotspot kHiresHelp[];
extern const int kHiresHelpCount;

// The help group the front requester computes (each reqN's compute_help_text):
// the standard groups; 0 for a requester with none.
uint8_t requester_help_group(Requester requester);

// The game's help system (src/helpmsg.cpp): the tooltips of a requester's
// hotspots and the framed help pages. State lives in `box_` (the game's
// othervar help fields); the language's help text and the art are passed in.
class HelpSystem
{
  public:
    // The framed page's drop shadow is drawn as black at this opacity: the
    // ghost table's blend of black into what is under it (draw flag 4) is two
    // thirds of the backdrop, which a black pixel at one third opacity gives.
    static constexpr uint8_t kPageShadowAlpha = 0x55;

    HelpSystem();

    // setup_help_indices: fills help_start with the index each help group's
    // hotspots begin at, groups ended by x0 -1.
    void setup_help_indices();
    // setup_indices_for_composite_help for the ShowHelp0 section: page0_start
    // with the string each room's help page begins at.
    void setup_page_indices(const LanguageText &text);

    HelpBox &box() { return box_; }
    const HelpBox &box() const { return box_; }
    // The clock the timing tests read (othervar.clock_ms).
    void set_clock(uint32_t ms) { box_.clock_ms = ms; }
    // The text font (cache slot 0 in game) and the requester frame art
    // (PULLDV) the boxes are measured and drawn with.
    void set_art(const Font &font, const SpriteSheet &pulldv);
    // The font cache slot 0x2d used instead in the main menu.
    void set_main_menu_font(const Font &font) { main_font_ = &font; }
    // Draw and measure with the main menu font (DM_MAIN_MENU); the caller
    // sets it while the start-up menus draw.
    void set_main_menu(bool on) { main_menu_ = on; }
    // The in-game blend table (GameData::ghost) the page's shadow boxes blend
    // through; without it they are solid.
    void set_ghost(const uint8_t *ghost) { ghost_ = ghost; }

    // The index group's hotspots start at.
    uint16_t help_start(int group) const { return help_start_.at(size_t(group)); }
    uint16_t page0_start(int sub) const { return page0_start_.at(size_t(sub)); }

    // compute_help_text: shows the tooltip of help group `group`'s hotspot
    // under the mouse (relative to x, y), or the forced hotspot.
    void compute_help_text(HelpContext &ctx, int x, int y, uint8_t group);
    // help_hotspot_detected: the mouse resting on `hotspot`: its tooltip
    // appears after 750 ms, or at once within 500 ms of the previous one.
    void help_hotspot_detected(HelpContext &ctx, uint16_t hotspot, uint8_t group, int x, int y);
    // set_help_from_index: shows `hotspot`'s text above its top centre.
    void set_help_from_index(HelpContext &ctx, uint16_t hotspot, int x, int y);
    // set_force_help / clear_force_help: the tooltip of text id `id` shows
    // while help group `group` is displayed.
    void set_force_help(uint8_t group, uint16_t id);
    void clear_force_help();
    // cancel_helpmsg: clears the tooltip and its hover state.
    void cancel();

    // set_help_text mode 1: a tooltip of `text` above x, y.
    void set_tooltip(int x, int y, std::string_view text);
    // set_help_text mode 2: a framed help page of the "."-ended `strs`,
    // centred when x, y is -1, -1.
    void set_page(int x, int y, const std::vector<std::string_view> &strs);
    // show_help_page 0: room `sub`'s help page from the language's ShowHelp0.
    void show_help_page(const LanguageText &text, int sub);
    // finish_composite_help: closes a page. Returns whether one was open.
    bool finish_composite_help();

    // Draws the box and its text into an indexed surface; `palette` gives the
    // white the box is filled with over a full-screen picture. `allowed`
    // mirrors mouse_mode_help_allowed, `tutorial` the advisor's flag: without
    // either nothing is drawn (the game's draw_help_text gate), and a tooltip
    // is cleared after it is drawn unless a framed page is up.
    void draw(Surface &dst, const Palette &palette, bool allowed = true, bool tutorial = false);
    // Sets the framed page's close-button hover from the mouse and returns
    // whether it just became highlighted (its sound, SFX_HLIGHT5).
    bool update_close_hover(int mouse_x, int mouse_y);
    // The text the last draw showed: box().msg is cleared after a tooltip is
    // drawn, as the original clears help_msg, so this keeps what was shown.
    const std::string &drawn_text() const { return drawn_; }
    // The framed page's drop shadow as the two rectangles to its right and
    // below, which the caller draws translucent over what is behind the help
    // layer (a ghost blend has no backdrop there). Empty without a page.
    std::array<Rect, 2> page_shadow_rects() const;

    // copy_and_format: `src` wrapped to a width that grows with its length.
    static std::string copy_and_format(std::string_view src);
    // copy_and_format_composite: the "."-ended `strs` joined into one page,
    // "//" giving a paragraph break, wrapped to three quarters of the screen.
    static std::string copy_and_format_composite(const std::vector<std::string_view> &strs, int screen_width,
                                                 const Font &font);

  private:
    // The help text index `id`'s entry, or empty.
    static std::string_view help_text(const HelpContext &ctx, uint16_t id);
    // The font the box is measured and drawn with (main menu font if set).
    const Font &active_font() const { return main_menu_ && main_font_ ? *main_font_ : *font_; }
    // Places a measured tooltip with its top centre at (cx, cy).
    void place_tooltip(int cx, int cy);
    // Clamps the box to the game's 640 x 480 screen.
    void clamp_to_screen(int screen_w);
    // The framed page's pulldv border, its close button and its black edge.
    void draw_page_frame(Surface &dst, uint8_t black);

    std::array<uint16_t, 0x1c> help_start_{};
    std::array<uint16_t, 31> page0_start_{};
    HelpBox box_;
    std::string drawn_;
    const Font *font_ = nullptr;
    const Font *main_font_ = nullptr;
    const SpriteSheet *pulldv_ = nullptr;
    const uint8_t *ghost_ = nullptr;
    bool main_menu_ = false;
};

} // namespace th

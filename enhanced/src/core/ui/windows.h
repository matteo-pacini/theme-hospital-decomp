#pragma once

#include "core/data/sprites.h"
#include "core/data/text.h"
#include "core/ui/layer.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace th
{

// The game's requesters (REQ_* in include/enums.h). Requester n is window n
// of the window list (kWindowList) and is drawn with sheet QDATA/REQnnV.
enum class Requester : uint8_t {
    Staff = 0,
    Handyman = 1,
    Patient = 2,
    Machine = 3,
    YesNo = 4, // src/req13.cpp
    EditRoom = 5,
    Queue = 6,
    BuildRoom = 9,
    BuyObjects = 0xa,
    HireStaff = 0xb,
    StaffDemand = 0xc,
    Jukebox = 0xd,
    None = 0x63,
};

inline constexpr int kWindowListSize = 14;

// The art the requesters are drawn with, which GameData does not load.
class RequesterArt
{
  public:
    // Loads every 640 x 480 requester sheet (hires_req_filenames,
    // src/files.cpp: QDATA/REQ00V to REQ13V but 07 and 08, reqspr coding)
    // and the fonts of cache slots 1, 2 and 3. On failure says why in `error`.
    bool load(const std::filesystem::path &dir, std::string *error = nullptr);

    // The sheet window `window` is drawn with (open_requester loads it into
    // button_sprite); empty for windows 7 and 8, which have none.
    const SpriteSheet &sheet(int window) const { return sheets_.at(size_t(window)); }
    // Font cache slot 1 (QDATA/FONT01V), the requesters' text.
    const Font &font() const { return font_; }
    // Font cache slot 2 (QDATA/FONT02V), the requesters' highlighted text.
    const Font &highlight_font() const { return highlight_font_; }
    // Font cache slot 3 (QDATA/FONT74V), the patient window's treatments.
    const Font &list_font() const { return list_font_; }

  private:
    std::array<SpriteSheet, kWindowListSize> sheets_;
    Font font_;
    Font highlight_font_;
    Font list_font_;
};

// WindowButton.flags (include/hosp.h). The trigger bits say which mouse
// events press a button while the mouse is over it; the hold bits which show
// it held.
enum WindowButtonFlags : uint32_t {
    kButtonDisabled = 0x1,  // drawn with disabled_sprite, not processed
    kButtonHeld = 0x2,      // state
    kButtonPressed = 0x4,   // state; kept across frames by kButtonToggle
    kButtonHoldOver = 0x8,  // held while the mouse is over it
    kButtonHoldLeft = 0x10, // held while over it with the left button down
    kButtonHoldRight = 0x20,
    kButtonTriggerOver = 0x40,
    kButtonTriggerLeftHeld = 0x80, // every frame the left button is down
    kButtonTriggerRightHeld = 0x100,
    kButtonTriggerLeftRelease = 0x200,
    kButtonTriggerRightRelease = 0x400,
    kButtonInert = 0x800,   // drawn, not processed
    kButtonHidden = 0x1000, // neither drawn nor processed
    kButtonToggle = 0x4000000,
    kButtonTriggerLeftClick = 0x8000000, // on the frame the left button goes down
    kButtonTriggerRightClick = 0x10000000,
    kButtonUndrawn = 0x20000000, // processed, not drawn
};

// Window.flags (include/hosp.h).
enum WindowFlags : uint16_t {
    kWindowNoFrame = 0x1,
    kWindowFrame0 = 0x2,
    kWindowFrame1 = 0x4,
    kWindowDraggable = 0x10,
    kWindowCentreX = 0x20,
    kWindowCentreY = 0x40,
    kWindowModal = 0x600, // on the front window, stops OpenWindow
};

// A button of an open window, as the game's WindowButton.
struct WindowButton {
    int16_t x = 0, y = 0;
    uint16_t width = 0, height = 0;
    uint32_t flags = 0;
    uint8_t callback = 0; // n of its reqN, 0 for none
    int sprite = 0, pressed_sprite = 0, disabled_sprite = 0;
    int16_t disabled_x = 0, disabled_y = 0;
    int16_t pressed_x = 0, pressed_y = 0;
};

// A window of the window list, as the game's Window. Its buttons' states
// last while it is closed, as the game's list does.
struct Window {
    int16_t x = 0, y = 0;
    uint16_t width = 0, height = 0;
    uint16_t flags = 0;
    uint8_t draw = 0; // n of its reqN_overdraw, 0 for none
    std::vector<WindowButton> buttons;
};

enum class YesNoAnswer { None, Yes, No };

// What a frame of the interface did.
struct InterfaceEvents {
    // The mouse is over the front window (mouse_was_on_window): the game
    // then shows the normal pointer and process_mouse ignores the buttons,
    // so the panel and the map must not act on them.
    bool mouse_on_window = false;
    // The yes/no requester's answer this frame. `yes_no_cancel` is set when
    // it came from Escape or a right click outside the window
    // (req13_cancel_check) rather than the No button: the game plays no
    // sound for that path.
    YesNoAnswer yes_no = YesNoAnswer::None;
    bool yes_no_cancel = false;
    // The button whose callback ran (debug.last_button counts from 1 in the
    // window's list), in window `triggered_window`; -1 if none.
    int triggered_window = -1;
    int triggered_button = -1;
    // The left button was down the frame before (OldLeftButton) when the
    // callback ran: a release trigger came from the left button.
    bool old_left = false;
};

// A requester's own input after Interface::frame: what the frame did and
// the mouse, in the interface's coordinates.
struct RequesterInput {
    const InterfaceEvents &events;
    const UiMouse &mouse;
};

// The game's window system (src/winproc.cpp, src/windraw.cpp) and requester
// state (crequester), in 640 x 480 coordinates: at most two windows open,
// the front one taking the mouse.
class Interface
{
  public:
    Interface();

    // OpenWindow: opens window `window` in front, or brings it there.
    // Fails if the front window is modal or two others are open.
    bool open_window(int window);
    // remove_window: closes it, clearing its buttons' states. Returns
    // whether it was open.
    bool close_window(int window);
    // Open windows, front first.
    int window_count() const;
    bool is_open(int window) const { return open_[0] == window || open_[1] == window; }
    int open_window_at(int i) const { return open_[size_t(i)]; }
    const Window &window(int window) const { return list_.at(size_t(window)); }
    // Sets window `window`'s height, as a requester's drawing may (req14_overdraw
    // fits the edit room requester to its list): the mouse is on the window
    // within it.
    void set_window_height(int window, int height) { list_.at(size_t(window)).height = uint16_t(height); }

    // The current requester (crequester.id).
    Requester requester() const { return requester_; }
    // open_requester(id, 1) with crequester.id set to it.
    void open_requester(Requester id);
    // close_crequester: closes window `id` and clears the current requester.
    void close_requester(Requester id);

    // open_yes_no_requester and the EV_OPEN_YES_NO it sends: closes the
    // patient, staff, handyman and machine windows and the current
    // requester (remembered), and opens the yes/no requester asking
    // `question` (in the game, a yesno_text string). While it is open the
    // interface is modal (display_mode DM_MODAL): the caller skips the
    // panel, the map's mouse and the keys, and disables the menu bar
    // (pmenu.disabled).
    void open_yes_no(std::string_view question);
    // close_yes_no_requester: if the yes/no requester is open, closes it
    // and, with `reopen`, reopens the requester it covered. The caller
    // restores the mouse mode it saved.
    void close_yes_no(bool reopen);
    bool modal() const { return modal_; }
    const std::string &yes_no_question() const { return question_; }

    // One frame: ProcessWindows, then in modal mode req13_cancel_check
    // (`escape`: Escape was pressed). A "no" closes the yes/no requester and
    // reopens what it covered, as the EV_CLOSE_YES_NO its callers send; a
    // "yes" leaves it open for the caller to act and close
    // (close_yes_no(false), as EV_QUIT_GAME and EV_DELETE_ROOM do).
    InterfaceEvents frame(const UiMouse &mouse, bool escape = false);

  private:
    void process_window(int window, const UiMouse &mouse, InterfaceEvents &events);
    void clear_buttons(int window);
    // The first of the `count` open windows under the last press comes to
    // the front.
    void raise_pressed_window(int count);
    bool mouse_on(int window, int x, int y) const;
    void callback(int window, int button, InterfaceEvents &events);

    std::array<Window, kWindowListSize> list_;
    std::array<int, 2> open_ = {-1, -1};
    Requester requester_ = Requester::None;
    Requester saved_requester_ = Requester::None;
    bool modal_ = false;
    std::string question_;
    bool mouse_was_on_window_ = false;
    bool old_left_ = false, old_right_ = false;
    int press_x_ = -1, press_y_ = -1;
};

// close_irequesters (0x87b4c): closes the patient, staff, handyman and
// machine windows, whatever the current requester.
void close_irequesters(Interface &ui);

// DrawWindows: the open windows, back to front, each its frame (none for
// any window of the list), its buttons from its requester's sheet, then its
// own drawing (the yes/no requester's question).
void draw_interface(UiLayer &layer, const Interface &ui, const RequesterArt &art);

} // namespace th

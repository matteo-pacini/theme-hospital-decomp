#include "core/ui/windows.h"

#include "core/data/gamedata.h"
#include "core/render/text.h"
#include "core/ui/window_tables.h"

namespace th
{

namespace
{

// hires_req_filenames (src/files.cpp), by window.
constexpr std::array<std::string_view, kWindowListSize> kRequesterFiles = {
    "qdata/req00v",
    "qdata/req01v",
    "qdata/req02v",
    "qdata/req03v",
    "qdata/req04v",
    "qdata/req05v",
    "qdata/req06v",
    "",
    "",
    "qdata/req09v",
    "qdata/req10v",
    "qdata/req11v",
    "qdata/req12v",
    "qdata/req13v",
};

// hires_requester_buttons[0] and [1] (src/reqbut.cpp): the yes/no
// requester's yes and no buttons, counted from 1 in its list.
constexpr int kYesButton = 12;
constexpr int kNoButton = 13;

// The yes/no requester's question box (HiresBoxes[36], src/coords.cpp,
// read by req13_overdraw as Req13Layout), in screen coordinates whatever
// the window's position.
constexpr Rect kYesNoTextBox = {253, 125, 150, 121};

bool over(int mx, int my, int x1, int y1, int x2, int y2)
{
    return mx >= x1 && mx <= x2 && my >= y1 && my <= y2;
}

} // namespace

bool RequesterArt::load(const std::filesystem::path &dir, std::string *error)
{
    for (size_t i = 0; i < kRequesterFiles.size(); i++) {
        if (kRequesterFiles[i].empty())
            continue;
        std::string base(kRequesterFiles[i]);
        auto tab = load_data_file(dir, base + ".tab", error);
        if (!tab)
            return false;
        auto dat = load_data_file(dir, base + ".dat", error);
        if (!dat)
            return false;
        if (!sheets_[i].load(std::move(*tab), std::move(*dat), SpriteFormat::Req)) {
            if (error)
                *error = base + ": bad sprite sheet";
            return false;
        }
    }
    return load_font(dir, 1, font_, error) && load_font(dir, 2, highlight_font_, error) &&
           load_font(dir, 3, list_font_, error);
}

Interface::Interface()
{
    for (size_t i = 0; i < list_.size(); i++) {
        const WindowDef &d = kWindowList[i];
        Window &w = list_[i];
        w.x = d.x;
        w.y = d.y;
        w.width = d.width;
        w.height = d.height;
        w.flags = d.flags;
        w.draw = d.draw;
        for (int b = d.first_button; b >= 0; b = kButtonList[size_t(b)].next) {
            const ButtonDef &bd = kButtonList[size_t(b)];
            w.buttons.push_back({bd.x, bd.y, bd.width, bd.height, bd.flags, bd.callback, bd.sprite, bd.pressed_sprite,
                                 bd.disabled_sprite, bd.disabled_x, bd.disabled_y, bd.pressed_x, bd.pressed_y});
        }
    }
}

int Interface::window_count() const
{
    int n = 0;
    while (n < 2 && open_[size_t(n)] >= 0)
        n++;
    return n;
}

bool Interface::open_window(int window)
{
    mouse_was_on_window_ = false;
    old_left_ = old_right_ = false;
    if (open_[0] >= 0 && (list_[size_t(open_[0])].flags & kWindowModal))
        return false;
    // make_front_window, else insert_window.
    for (size_t i = 0; i < 2; i++) {
        if (open_[i] == window || open_[i] < 0) {
            for (; i > 0; i--)
                open_[i] = open_[i - 1];
            open_[0] = window;
            return true;
        }
    }
    return false;
}

bool Interface::close_window(int window)
{
    for (size_t i = 0; i < 2; i++) {
        if (open_[i] == window) {
            mouse_was_on_window_ = false;
            clear_buttons(window);
            for (; i < 1; i++)
                open_[i] = open_[i + 1];
            open_[i] = -1;
            return true;
        }
    }
    return false;
}

void Interface::open_requester(Requester id)
{
    requester_ = id;
    open_window(int(id));
}

void Interface::close_requester(Requester id)
{
    close_window(int(id));
    requester_ = Requester::None;
}

void Interface::open_yes_no(std::string_view question)
{
    question_ = question;
    modal_ = true;
    // close_irequesters (the windows opened over the view are not ported).
    for (Requester r : {Requester::Patient, Requester::Staff, Requester::Handyman, Requester::Machine})
        close_window(int(r));
    saved_requester_ = requester_;
    if (requester_ != Requester::None)
        close_requester(requester_);
    open_requester(Requester::YesNo);
}

void Interface::close_yes_no(bool reopen)
{
    if (requester_ != Requester::YesNo)
        return;
    modal_ = false;
    close_requester(requester_);
    for (Requester r : {Requester::Patient, Requester::Staff, Requester::Handyman, Requester::Machine})
        close_window(int(r));
    if (reopen && saved_requester_ != Requester::None)
        open_requester(saved_requester_);
}

bool Interface::mouse_on(int window, int x, int y) const
{
    const Window &w = list_[size_t(window)];
    return over(x, y, w.x, w.y, w.x + w.width - 1, w.y + w.height - 1);
}

void Interface::clear_buttons(int window)
{
    for (WindowButton &b : list_[size_t(window)].buttons) {
        if (!(b.flags & kButtonToggle))
            b.flags &= ~uint32_t(kButtonPressed);
        b.flags &= ~uint32_t(kButtonHeld);
    }
}

namespace
{

// The mouse as process_buttons_normal_state tests it.
struct ButtonMouse {
    bool left, right;                 // down, or pressed this frame
    bool left_click, right_click;     // pressed this frame
    bool left_release, right_release; // down last frame, not now
};

// Whether a button with `flags` shows held, `on` whether the mouse is over
// it.
bool button_held(uint32_t flags, bool on, const ButtonMouse &m)
{
    return on && ((flags & kButtonHoldOver) || ((flags & kButtonHoldLeft) && m.left) ||
                  ((flags & kButtonHoldRight) && m.right));
}

// Whether a button with `flags` triggers, `on` whether the mouse is over
// it.
bool button_triggered(uint32_t flags, bool on, const ButtonMouse &m)
{
    return on &&
           ((flags & kButtonTriggerOver) || ((flags & kButtonTriggerLeftHeld) && m.left) ||
            ((flags & kButtonTriggerLeftClick) && m.left_click) ||
            ((flags & kButtonTriggerLeftRelease) && m.left_release) || ((flags & kButtonTriggerRightHeld) && m.right) ||
            ((flags & kButtonTriggerRightClick) && m.right_click) ||
            ((flags & kButtonTriggerRightRelease) && m.right_release));
}

// A button's pressed state lasts only while it triggers, unless it toggles.
void release_unless_toggle(WindowButton &b)
{
    if (!(b.flags & kButtonToggle))
        b.flags &= ~uint32_t(kButtonPressed);
}

} // namespace

// process_window with process_buttons_normal_state. The table has no
// resize, drag, size-cycling, close or slider buttons, so their handling is
// left out, as is the debug key D pressing the first enabled button.
void Interface::process_window(int window, const UiMouse &mouse, InterfaceEvents &events)
{
    Window &win = list_[size_t(window)];
    // A press and release within one frame still counts as held for it, so
    // that the release triggers on the next.
    bool left = mouse.left_held || mouse.left_pressed;
    bool right = mouse.right_held || mouse.right_pressed;
    const ButtonMouse m = {
        left, right, mouse.left_pressed, mouse.right_pressed, old_left_ && !left, old_right_ && !right};
    size_t i = 0;
    for (; i < win.buttons.size(); i++) {
        WindowButton &b = win.buttons[i];
        if (b.flags & (kButtonDisabled | kButtonInert | kButtonHidden))
            continue;
        bool on =
            over(mouse.x, mouse.y, b.x + win.x, b.y + win.y, b.x + win.x + b.width - 1, b.y + win.y + b.height - 1);
        if (button_held(b.flags, on, m))
            b.flags |= kButtonHeld;
        else
            b.flags &= ~uint32_t(kButtonHeld);
        if (button_triggered(b.flags, on, m)) {
            if (b.flags & kButtonToggle)
                b.flags ^= kButtonPressed;
            else
                b.flags |= kButtonPressed;
            if (b.callback)
                callback(window, int(i) + 1, events);
            i++;
            break;
        }
        release_unless_toggle(b);
    }
    for (; i < win.buttons.size(); i++)
        release_unless_toggle(win.buttons[i]);

    old_left_ = left;
    old_right_ = right;
    if (win.flags & kWindowCentreX)
        win.x = int16_t((kUiWidth - win.width) / 2);
    if (win.flags & kWindowCentreY)
        win.y = int16_t((kUiHeight - win.height) / 2);
}

// The buttons' reqN: of the requesters, only the yes/no one's (req13) is
// ported.
void Interface::callback(int window, int button, InterfaceEvents &events)
{
    events.triggered_window = window;
    events.triggered_button = button;
    events.old_left = old_left_;
    if (list_[size_t(window)].buttons[size_t(button - 1)].callback != 13)
        return;
    int n = 1;
    for (const WindowButton &b : list_[size_t(window)].buttons) {
        if (b.flags & (kButtonHeld | kButtonPressed)) {
            if (n == kNoButton)
                events.yes_no = YesNoAnswer::No;
            else if (n == kYesButton)
                events.yes_no = YesNoAnswer::Yes;
        }
        n++;
    }
}

void Interface::raise_pressed_window(int count)
{
    for (int i = 0; i < count; i++) {
        int w = open_[size_t(i)];
        if (!mouse_on(w, press_x_, press_y_))
            continue;
        // make_front_window
        for (; i > 0; i--)
            open_[size_t(i)] = open_[size_t(i - 1)];
        open_[0] = w;
        return;
    }
}

InterfaceEvents Interface::frame(const UiMouse &mouse, bool escape)
{
    InterfaceEvents events;
    if (mouse.left_pressed || mouse.right_pressed) {
        press_x_ = mouse.x;
        press_y_ = mouse.y;
    }

    // ProcessWindows.
    if (open_[0] >= 0) {
        mouse_was_on_window_ = mouse_on(open_[0], mouse.x, mouse.y);
        int count = window_count();
        if (!(list_[size_t(open_[0])].flags & kWindowModal) && (mouse.left_pressed || mouse.right_pressed))
            raise_pressed_window(count);
        for (int i = count - 1; i > 0; i--)
            clear_buttons(open_[size_t(i)]);
        process_window(open_[0], mouse, events);
    }
    events.mouse_on_window = mouse_was_on_window_;

    // req13_cancel_check.
    if (modal_ && requester_ == Requester::YesNo && (escape || (mouse.right_pressed && !mouse_was_on_window_)))
        events.yes_no = YesNoAnswer::No;
    if (events.yes_no == YesNoAnswer::No)
        close_yes_no(true);
    return events;
}

void close_irequesters(Interface &ui)
{
    for (Requester r : {Requester::Patient, Requester::Staff, Requester::Handyman, Requester::Machine})
        ui.close_window(int(r));
}

void draw_interface(UiLayer &layer, const Interface &ui, const RequesterArt &art)
{
    for (int i = ui.window_count() - 1; i >= 0; i--) {
        int id = ui.open_window_at(i);
        const Window &win = ui.window(id);
        // draw_window: frames are drawn from the bord sprites, which the game
        // never loads; every window of the list has kWindowNoFrame.
        const SpriteSheet &sheet = art.sheet(id);
        for (const WindowButton &b : win.buttons) {
            if (b.flags & (kButtonHidden | kButtonUndrawn))
                continue;
            if (b.flags & kButtonDisabled)
                layer.draw_sprite(win.x + b.disabled_x, win.y + b.disabled_y, sheet.at(size_t(b.disabled_sprite)));
            else if ((b.flags & (kButtonHeld | kButtonPressed)) &&
                     (!(b.flags & kButtonHeld) || !(b.flags & kButtonPressed)))
                layer.draw_sprite(win.x + b.pressed_x, win.y + b.pressed_y, sheet.at(size_t(b.pressed_sprite)));
            else
                layer.draw_sprite(win.x + b.x, win.y + b.y, sheet.at(size_t(b.sprite)));
        }
        if (win.draw == 13) {
            // req13_overdraw.
            const Rect &box = kYesNoTextBox;
            layer.draw_text(art.font(), 0, 0, ui.yes_no_question(), {{box.x, box.y, box.w}, box, kTextWrap});
        }
    }
}

} // namespace th

#pragma once

#include "core/data/palette.h"
#include "core/data/sprites.h"
#include "core/data/text.h"
#include "core/render/surface.h"
#include "core/ui/layer.h"
#include "core/world/events.h"
#include "core/world/game.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace th
{

// The fax (the game's choice screen, display_mode DM_CHOICE; src/choice.cpp):
// a call the player answers, shown full screen on the fax machine's picture
// with its message and answer buttons and the machine's keypad. The game
// runs on behind it. Its kinds are Game::choice_screen's (kFax*): the
// emergency offered (accept, refused while an emergency runs, or refuse),
// its result, a new illness, an epidemic found and its result, and a
// patient with no treatment (send home,
// wait, or the research department for an autopsy, once it can be done) or
// no diagnosis (send home, guess the cure, once the illness is known and
// curable, or wait); the VIP's faxes are not ported and show nothing.
struct Fax {
    // A fax is up, display_choice's state (0x5a drawing it, 0x5b up, 0x5c
    // closing), and what it is about (choice.value, choice_spare_data and
    // choice_time_out).
    bool open = false;
    int state = 0;
    Game::ChoiceScreen choice;
    // The answer buttons enabled (available_buttons bits), their text
    // (c_buttons) and the message's strings (c_text).
    uint16_t available = 0xffff;
    std::vector<std::string> buttons;
    std::vector<std::string> strings;
    // The game turn the mouse last moved (choice_actual_game_turn) and where
    // (choice_mouse_position, x + y); a fax that may close itself does so
    // once `timeout` turns pass without it moving.
    uint32_t still_since = 0;
    int mouse_position = 0;
    // The keypad's buttons dialled (fax_no, NumPressed).
    std::array<uint8_t, 6> dialled{};
    uint8_t dialled_count = 0;

    // Game frames run every 40 ms (limit_framerate); input waits for one.
    double frame_time = 0;
    UiMouse pending_mouse;
    bool pending_escape = false;

    // The fax machine's picture (SCR_FAX: fax01v), its palette and sprites
    // (fax02v), the font (cache slot 0x1e) and the language's text.
    Surface picture;
    Palette palette{};
    SpriteSheet sprites;
    Font font;
    const LanguageText *text = nullptr;
    // The screen shown (wscreen) and the picture with the fax drawn on it
    // (bscreen), 640 x 480.
    Surface surface;
    Surface background;
};

// What a frame of the fax asks of the caller.
struct FaxResult {
    // The player's answer, for Game::event (none if type is EV_NONE).
    Event event;
    // The sound effects played (play_sound; SFX_* of include/enums.h).
    std::vector<uint8_t> sounds;
};

// Loads the fax machine's picture, palette, sprites and font from `dir`;
// `text` is kept by reference. On failure says why in `error`.
bool load_fax(Fax &fax, const std::filesystem::path &dir, const LanguageText &text, std::string *error = nullptr);

// display_choice_screen (0xa410c): opens the fax `choice` asks for (none if
// its type is 0), to be drawn by its next frame.
void display_choice_screen(Fax &fax, const Game::ChoiceScreen &choice);

// The interface's side of a command, as process_turn for the local player:
// EV_CHOICE_RESULT (an answer) closes the fax (restore_display_screen).
// Run it before Game::process_turn, which clears the command.
void fax_command(Fax &fax, const Event &event);

// The keys the fax reads: Escape or Space pressed since the last frame.
struct FaxKeys {
    bool escape = false;
};

// display_choice (0xa3158) for `seconds` of time with this mouse (in the
// surface's 640 x 480 coordinates): a game frame per 40 ms passed, at most
// one per call. The first draws the fax into `background` (its message
// composed from `game` and the choice's subject, drawing on `safeseed`,
// the game's saferand, for the emergency's caller) with SFX_FAX_IN; later
// ones show it with the button under a held left button pressed, and a
// left button released on an answer runs its handler (the command, or
// closing the fax) and on the keypad dials. A fax that may close itself
// closes on Escape or Space, the keypad's close button or `timeout` turns
// without the mouse moving. An answer that carries out part of its handler
// in the interface (a VIP's refusals in a row) updates `game`. Nothing if
// no fax is open.
FaxResult fax_frame(Fax &fax, const UiMouse &mouse, const FaxKeys &keys, double seconds, Game &game,
                    uint32_t &safeseed);

// The fax's answer buttons as calculate_buttons lays them out: their
// boxes, right-aligned in the message's box, the last at its bottom.
std::vector<Rect> fax_answer_buttons(const Fax &fax);

// The fax's message as display_choice_text composes it for `game`, and the
// answer buttons it leaves enabled in `available` (from all). Draws on
// `safeseed` for the emergency's caller, as the game does. The message's
// strings are fax.strings.
std::string fax_message(const Fax &fax, const Game &game, uint16_t &available, uint32_t &safeseed);

} // namespace th

#include "core/world/fax_screen.h"

#include "core/world/emergency.h"
#include "core/world/epidemic.h"
#include "core/world/trace.h"

#include <utility>

namespace th
{

namespace
{
constexpr uint8_t kShowing = 0x5a;
constexpr uint8_t kUp = 0x5b;
constexpr uint8_t kClosing = 0x5c;
constexpr uint8_t kFadeHospital = 1;
constexpr uint8_t kFadeFax = 2;

// The 0x5b step: with a timeout, Escape or Space closes the fax, and so do
// `timeout` turns without the pointer moving.
void fax_up(Game &game)
{
    Game::FaxScreen &fax = game.fax_screen;
    if (!fax.timeout)
        return;
    if (std::exchange(game.fax_key, false))
        fax.state = kClosing;
    if (fax.pointer != game.fax_pointer) {
        fax.since = game.turn_count;
        fax.pointer = game.fax_pointer;
    }
    if (game.turn_count - fax.since > uint32_t(fax.timeout))
        fax.state = kClosing;
}
} // namespace

void fax_open(Game &game, int16_t timeout)
{
    Game::FaxScreen &fax = game.fax_screen;
    // save_display_screen.
    fax.backup_open = fax.open;
    fax.backup_timeout = fax.timeout;
    fax.open = true;
    fax.state = kShowing;
    fax.timeout = timeout;
    game.fax_key = false;
}

void fax_restore(Game &game)
{
    Game::FaxScreen &fax = game.fax_screen;
    fax.open = fax.backup_open;
    fax.timeout = fax.backup_timeout;
    fax.backup_open = false;
    if (fax.open)
        fax.state = kShowing;
    else
        fax.fade = kFadeHospital;
}

void fax_return_to_game(Game &game)
{
    game.fax_screen.open = false;
    game.fax_screen.fade = kFadeHospital;
}

void fax_event(Game &game, const Event &event)
{
    if (event.type == EV_START_EMERGENCY) {
        fax_return_to_game(game);
    } else if (event.type == EV_CHOICE_RESULT) {
        // Accepting an emergency starts it through EV_START_EMERGENCY, which
        // closes the fax itself; covering an epidemic up with a nonzero arg is
        // the quiet answer, which leaves the screen alone.
        if (event.value == kChoiceEmergencyAccept) {
            fax_return_to_game(game);
        } else if (event.value != kChoiceEpidemicCoverUp || event.arg == 0) {
            fax_restore(game);
            game.fax_screen.fade = kFadeHospital;
        }
    }
}

void fax_frame(Game &game)
{
    Game::FaxScreen &fax = game.fax_screen;
    if (!fax.open)
        return;
    switch (fax.state) {
    case kShowing:
        fax.since = game.turn_count;
        fax.state = kUp;
        fax.fade = kFadeFax;
        break;
    case kUp:
        fax_up(game);
        break;
    case kClosing:
        fax_restore(game);
        break;
    default:
        break;
    }
}

void fax_fade(Game &game)
{
    Game::FaxScreen &fax = game.fax_screen;
    if (!fax.fade)
        return;
    World &world = game.world;
    bool on = fax.fade == kFadeFax;
    fax.fade = 0;
    if (on == world.fax_palette)
        return;
    world.fax_palette = on;
    trace_event("palette fax=%d red=%d black=%d dark_blue=%d", int(on), int(colour_red(world)), int(kColourBlack),
                int(colour_dark_blue(world)));
}

} // namespace th

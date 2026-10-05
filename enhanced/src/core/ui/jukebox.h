#pragma once

#include "core/ui/layer.h"
#include "core/ui/windows.h"
#include "core/world/events.h"

#include <cstdint>
#include <vector>

namespace th
{

class MusicPlayer;

// The jukebox requester's state (the game's crequester while it is
// REQ_JUKEBOX): crequester.arg, set by a left press anywhere while it is
// open and cleared by the track list click it arms.
struct Jukebox {
    bool armed = false;
};

// What a frame of the jukebox did besides driving the music.
struct JukeboxResult {
    // The close button was clicked: the caller runs
    // setup_abort_all_requesters, as for Escape.
    bool close = false;
    // The sound effects to play (SFX_* numbers), in order.
    std::vector<uint8_t> sounds;
};

// The interface's part of EV_OPEN_JUKEBOX (process_turn, src/io.cpp): with
// no requester open, opens the jukebox, closing the patient, staff,
// handyman and machine windows. Call it with Game::event before
// Game::process_turn; other commands are ignored.
void jukebox_event(Jukebox &jukebox, Interface &ui, const Event &event);

// One frame of the open jukebox, after the interface's frame: req6
// (0x8dee0) for a button clicked (`events`): play or pause, previous and
// next (both ignored while looping), stop, loop and close; then the track
// list of req6_overdraw (0x8e184): once the left button is up after a press,
// a click on a title plays that track, a click on its light takes it out of
// the playlist or back (moving on if it is the one playing). Previous,
// next and the light act on the song as the game does, ending it for
// MusicPlayer::play_next to move on, which does nothing while `paused`.
// The list lies below the window (Interface::window's height), as in the
// game, so the interface does not count the mouse as on it there.
JukeboxResult jukebox_frame(Jukebox &jukebox, const Interface &ui, const InterfaceEvents &events,
                            const RequesterArt &art, MusicPlayer &music, const UiMouse &mouse, bool paused);

// req6_overdraw's drawing, after draw_interface while the jukebox is open:
// the playing track's title, the loop and play lights, the track list with
// each track's light (all off but the current track while looping) and the
// list's bottom edge.
void draw_jukebox(UiLayer &layer, const Interface &ui, const RequesterArt &art, const MusicPlayer &music);

} // namespace th

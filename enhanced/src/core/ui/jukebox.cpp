#include "core/ui/jukebox.h"

#include "core/audio/music.h"
#include "core/audio/sounds.h"

#include <string_view>

namespace th
{

namespace
{

// hires_requester_buttons[0x53] to [0x58] (src/reqbut.cpp): the jukebox's
// buttons, counted from 1 in its window's list.
constexpr int kCloseButton = 0x10;
constexpr int kPlayButton = 0x09;
constexpr int kPreviousButton = 0x0a;
constexpr int kNextButton = 0x0b;
constexpr int kStopButton = 0x0c;
constexpr int kLoopButton = 0x0d;

// hires_requester_sprites[0x1f] to [0x27] (src/reqbut.cpp): REQ13V's
// sprites for the loop light, a list row's left end and middle, a track's
// light on and off, the bottom edge's left, middle and right, and the play
// light.
constexpr size_t kLoopLight = 0x191;
constexpr size_t kRowLeft = 0x192;
constexpr size_t kRowTile = 0x193;
constexpr size_t kTrackOn = 0x194;
constexpr size_t kTrackOff = 0x195;
constexpr size_t kBottomLeft = 0x196;
constexpr size_t kBottomTile = 0x197;
constexpr size_t kBottomRight = 0x198;
constexpr size_t kPlayLight = 0x189;

// HiresCoords (src/coords.cpp), relative to the window: [0x7f] the playing
// title, [0x80] the list's first row, [0x81] the bottom edge's x and how far
// it rises into the last row, [0x82] a row's title (its y counted from the
// row above), [0xa0] the loop light and [0xa1] the play light.
constexpr int kTitleX = 26, kTitleY = 29;
constexpr int kListX = 2, kListY = 79;
constexpr int kBottomX = 2, kBottomRise = 3;
constexpr int kRowTitleX = 26, kRowTitleY = 41;
constexpr int kLoopX = 187, kLoopY = 51;
constexpr int kPlayX = 2, kPlayY = 51;

// A list row is its left end then middle tiles while x < 7 tiles' width,
// and the light after them (req6_overdraw's loop, which compares the
// window-relative x with the width, not the tiles drawn).
int tiles_end(int start, int tile_width)
{
    int x = start;
    if (tile_width <= 0)
        return x;
    while (x < tile_width * 7)
        x += tile_width;
    return x;
}

bool over(const UiMouse &m, int x, int y, int w, int h)
{
    return m.x >= x && m.x < x + w && m.y >= y && m.y < y + h;
}

} // namespace

void jukebox_event(Jukebox &jukebox, Interface &ui, const Event &event)
{
    if (event.type != EV_OPEN_JUKEBOX || ui.requester() != Requester::None)
        return;
    jukebox.armed = false;
    // close_irequesters.
    for (Requester r : {Requester::Patient, Requester::Staff, Requester::Handyman, Requester::Machine})
        ui.close_window(int(r));
    ui.open_requester(Requester::Jukebox);
}

namespace
{

// The jukebox button `n` (counted from 1), held or pressed.
void jukebox_button(int n, MusicPlayer &music, bool paused, JukeboxResult &result)
{
    switch (n) {
    case kPreviousButton:
        if (!music.loop() && music.playing()) {
            music.end_song();
            music.play_previous(paused);
        }
        result.sounds.push_back(kSfxSelectX);
        break;
    case kStopButton:
        if (music.playing())
            music.stop();
        result.sounds.push_back(kSfxSelectX);
        break;
    case kPlayButton:
        if (music.playing()) {
            music.pause_song();
            music.set_playing(false);
        } else {
            music.resume_song();
            music.set_playing(true);
            if (music.song_over(paused))
                music.play(music.track());
        }
        result.sounds.push_back(kSfxSelectX);
        break;
    case kNextButton:
        if (!music.loop() && music.playing())
            music.end_song();
        result.sounds.push_back(kSfxSelectX);
        break;
    case kCloseButton:
        result.close = true;
        result.sounds.push_back(kSfxNo4);
        break;
    case kLoopButton:
        music.set_loop(!music.loop());
        result.sounds.push_back(kSfxSelectX);
        break;
    default:
        break;
    }
}

// What req6_overdraw's list clicks test: the window, the row sprites, the
// pointer and whether the left button is down.
struct JukeboxList {
    const Window &win;
    Sprite left_end, tile, light;
    const UiMouse &mouse;
    bool left;
};

// req6_overdraw's clicks on track row `i`: its title plays it, its light
// turns it on or off.
void jukebox_row_click(Jukebox &jukebox, const JukeboxList &list, int i, MusicPlayer &music, bool paused,
                       JukeboxResult &result)
{
    const Window &win = list.win;
    const Sprite &tile = list.tile;
    int y = win.y + kListY + i * tile.height;
    int x = kListX;
    for (; tile.width > 0 && x < tile.width * 7; x += tile.width) {
        if (!list.left && jukebox.armed &&
            over(list.mouse, win.x + x + list.left_end.width, y, tile.width, tile.height)) {
            if (i != music.track()) {
                music.set_loop(false);
                music.set_playing(true);
                music.play(i);
            }
            jukebox.armed = false;
        }
    }
    if (!list.left && jukebox.armed &&
        over(list.mouse, win.x + x + list.left_end.width, y, list.light.width, list.light.height)) {
        music.set_track_enabled(i, !music.track_enabled(i));
        result.sounds.push_back(kSfxSelectX);
        if (i == music.track()) {
            music.end_song();
            music.play_next(paused);
        }
        jukebox.armed = false;
    }
}

} // namespace

JukeboxResult jukebox_frame(Jukebox &jukebox, const Interface &ui, const RequesterInput &input, const RequesterArt &art,
                            MusicPlayer &music, bool paused)
{
    const InterfaceEvents &events = input.events;
    const UiMouse &mouse = input.mouse;
    JukeboxResult result;
    if (ui.requester() != Requester::Jukebox)
        return result;
    const Window &win = ui.window(int(Requester::Jukebox));

    if (events.triggered_window == int(Requester::Jukebox)) {
        int n = 1;
        for (const WindowButton &b : win.buttons) {
            if (b.flags & (kButtonHeld | kButtonPressed))
                jukebox_button(n, music, paused, result);
            n++;
        }
    }

    // req6_overdraw's clicks: lbDisplay.mleft_button arms them, and they
    // act once it is up.
    const SpriteSheet &sheet = art.sheet(int(Requester::Jukebox));
    bool left = mouse.left_held || mouse.left_pressed;
    const JukeboxList list = {win, sheet.at(kRowLeft), sheet.at(kRowTile), sheet.at(kTrackOn), mouse, left};
    if (left)
        jukebox.armed = true;
    for (int i = 0; i < music.count(); i++)
        jukebox_row_click(jukebox, list, i, music, paused, result);
    return result;
}

void draw_jukebox(UiLayer &layer, const Interface &ui, const RequesterArt &art, const MusicPlayer &music)
{
    if (ui.requester() != Requester::Jukebox)
        return;
    const Window &win = ui.window(int(Requester::Jukebox));
    const SpriteSheet &sheet = art.sheet(int(Requester::Jukebox));
    const int ox = win.x, oy = win.y;
    const Rect screen = {0, 0, kUiWidth, kUiHeight};
    const TextWindow text_window = {0, 0, kUiWidth};
    // midi_files[count()] is the list's empty slot.
    auto title = [&](int track) { return track < music.count() ? std::string_view(music.title(track)) : ""; };

    if (music.midi_playing())
        layer.draw_text(art.highlight_font(), ox + kTitleX, oy + kTitleY, title(music.track()), {text_window, screen});
    if (music.loop())
        layer.draw_sprite(ox + kLoopX, oy + kLoopY, sheet.at(kLoopLight));
    if (music.playing())
        layer.draw_sprite(ox + kPlayX, oy + kPlayY, sheet.at(kPlayLight));

    const Sprite left_end = sheet.at(kRowLeft), tile = sheet.at(kRowTile);
    int y = kListY;
    for (int i = 0; i < music.count(); i++, y += tile.height) {
        layer.draw_sprite(kListX + ox, y + oy, left_end);
        int x = tiles_end(kListX, tile.width);
        for (int tx = kListX; tx < x; tx += tile.width)
            layer.draw_sprite(tx + ox + left_end.width, y + oy, tile);
        bool on = music.loop() ? i == music.track() : music.track_enabled(i);
        layer.draw_sprite(x + ox + left_end.width, y + oy, sheet.at(on ? kTrackOn : kTrackOff));
        const Font &font = music.midi_playing() && i == music.track() ? art.highlight_font() : art.font();
        layer.draw_text(font, ox + kRowTitleX, y - tile.height + oy + kRowTitleY, title(i), {text_window, screen});
    }

    const Sprite bottom_left = sheet.at(kBottomLeft), bottom_tile = sheet.at(kBottomTile);
    y -= kBottomRise;
    layer.draw_sprite(kBottomX + ox, y + oy, bottom_left);
    int x = tiles_end(kBottomX, bottom_tile.width);
    for (int tx = kBottomX; tx < x; tx += bottom_tile.width)
        layer.draw_sprite(tx + ox + bottom_left.width, y + oy, bottom_tile);
    layer.draw_sprite(x + ox + bottom_left.width, y + oy, sheet.at(kBottomRight));
}

} // namespace th

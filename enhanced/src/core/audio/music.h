#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace th
{

// A Global Timbre Library (SOUND/SAMPLE.OPL, the OPL3 driver's, or
// SAMPLE.AD, OPL2's; the game's two are the same file): a directory of
// 6-byte entries (patch, bank, 32-bit file offset) ended by bank 0xff, each
// pointing at a timbre record that starts with its own 16-bit length.
class TimbreBank
{
  public:
    // Reads the library at `path`. On failure says why in `error`.
    bool load(const std::filesystem::path &path, std::string *error = nullptr);

    int count() const { return int(timbres_.size()); }
    // The record of timbre `bank`/`patch` (length word included), or an
    // empty span. Bank 127 holds the percussion, by key.
    std::span<const uint8_t> find(int bank, int patch) const;

  private:
    struct Timbre {
        uint8_t bank = 0;
        uint8_t patch = 0;
        std::vector<uint8_t> data;
    };
    std::vector<Timbre> timbres_;
};

// An Extended MIDI file (SOUND/MIDI/*.XMI): its first sequence, as
// XMI_find_sequence and AIL_init_sequence find it.
struct XmiSong {
    std::vector<uint8_t> image;
    // The TIMB chunk's timbres, (bank, patch), in order.
    std::vector<std::array<uint8_t, 2>> timbres;
    // Offsets in `image` of the EVNT chunk's events and of the RBRN chunk
    // (0 without one).
    size_t events = 0;
    size_t events_size = 0;
    size_t branches = 0;
    // The length in ticks (120 a second at 100% tempo) up to its end of
    // track.
    uint32_t ticks = 0;
};

// Reads `file` into `song`, checking that every event up to the end of
// track lies inside the EVNT chunk. On failure says why in `error`.
bool parse_xmi(std::span<const uint8_t> file, XmiSong &song, std::string *error = nullptr);

// The game's music (src/sound.cpp's MIDI functions over the Bullfrog sound
// library and Miles' XMIDI sequencer, src/lib/ail/ailxmidi.c) played as the
// game's OPL3 driver (SOUND/OPL3.MDI) plays it, on an emulated YMF262.
class MusicPlayer
{
  public:
    // myplayer.music_volume's full volume.
    static constexpr int kMaxVolume = 127;

    MusicPlayer();
    ~MusicPlayer();
    MusicPlayer(const MusicPlayer &) = delete;
    MusicPlayer &operator=(const MusicPlayer &) = delete;

    // setup_sound, init_midi and load_midi: reads the timbre library and
    // SOUND/MIDI/MIDI.TXT's songs (file and title line pairs, up to ten
    // whose files exist) under the game data folder `dir`. music_ok() is
    // whether any song can play. On failure says why in `error`.
    bool load(const std::filesystem::path &dir, std::string *error = nullptr);

    // statvar.music_ok with songs to play.
    bool music_ok() const { return !songs_.empty() && timbres_.count() > 0; }
    int count() const { return int(songs_.size()); }
    const std::string &title(int track) const { return songs_[size_t(track)].title; }
    const TimbreBank &timbres() const { return timbres_; }

    // myplayer.music_on (on after a successful load) and music_volume
    // (0-127, full at first).
    bool music_on() const { return music_on_ && music_ok(); }
    int volume() const { return volume_; }
    // The ticked volume item, 9 - volume * 10 / 128 (MenuContext::music_volume).
    int volume_item() const { return 9 - volume_ * 10 / 128; }
    // othervar.midi_track and music_playing.
    int track() const { return track_; }
    bool playing() const { return music_playing_; }

    // start_level's music: picks track `clock` % count() (the game passes
    // LbTimerClock()) and plays it if the music is on.
    void start_level(uint32_t clock);
    // play_next_midi, once a frame of the game view: when the song has
    // ended, plays the next one, wrapping after the last. Nothing happens
    // while `paused`.
    void play_next(bool paused);
    // play_midi: plays `track` from its start, or the next one if it cannot.
    void play(int track);
    // stop_midi: ends the song (the game does so leaving a level).
    void stop();
    // pulldown_toggle_music: music on (playing the current track from its
    // start) or off.
    void toggle_music();
    // pulldown_smusic: the volume becomes (10 - item) * 127 / 10 for menu
    // item `item`, 0 (100%) to 9 (10%), and the song fades to it over a
    // second.
    void set_volume_item(int item);

    // Adds the next `frames` frames of the music (interleaved stereo,
    // values -1 to 1) at `rate` frames per second to `out`, advancing the
    // sequencer 120 times a second of output.
    void render(float *out, int frames, int rate);
    // As render, overwriting `out` with 16-bit samples.
    void render(int16_t *out, int frames, int rate);

  private:
    struct Song {
        std::string file;
        std::string title;
        XmiSong xmi;
        bool ok = false;
    };
    struct Engine;

    // PlayMidiFromAddress for track `track`; false if it has no song.
    bool start_song(int track);
    bool song_done() const;

    TimbreBank timbres_;
    std::vector<Song> songs_;
    std::unique_ptr<Engine> engine_;
    bool music_on_ = true;
    int volume_ = kMaxVolume;
    int track_ = 8;
    bool midi_playing_ = true;
    bool music_playing_ = false;
};

} // namespace th

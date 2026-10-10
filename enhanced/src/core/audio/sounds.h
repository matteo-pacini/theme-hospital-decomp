#pragma once

#include "core/render/iso.h"
#include "core/world/world.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace th
{

// Sample sources (play_sample_using_heap's source; SNDSRC_* in
// include/enums.h). A source and a sample number name a playing sample.
constexpr uint32_t kSourceGeneral = 0xa; // sound effects at full volume in the centre
constexpr uint32_t kSourceAmbient = 0xb; // the announcements (do_ambients)
constexpr uint32_t kSourceCrowd = 0xc;   // manage_background_sounds' loops
constexpr uint32_t kSourceClock = 0xd;   // the emergency countdown's clock
// Sources above 0xe are things (the game passes the Thing's address), which
// manage_sound keeps placed on screen.
constexpr uint32_t kSourceThings = 0x10;
constexpr uint32_t thing_source(uint16_t thing)
{
    return kSourceThings + thing;
}

// Sound effect numbers (play_sound's sfx, SFX_* in include/enums.h) the
// interface plays.
enum : uint8_t {
    kSfxHighlightP2 = 0x01, // SFX_HLIGHTP2: a person's status icon appears
    kSfxWrong2 = 0x08,      // SFX_WRONG2: a click refused
    kSfxYesX = 0x41,        // SFX_YESX
    kSfxNo4 = 0x42,         // SFX_NO4
    kSfxSelectX = 0x43,     // SFX_SELECTX: a click acted on
    kSfxHighlight5 = 0x44,  // SFX_HLIGHT5: a menu item under the mouse
    kSfxResearch = 0x75,    // SFX_RESEARCH: the research screen's bubbling
    kSfxPickup = 0x99,      // SFX_PICKUP: a member of staff picked up
    kSfxSack = 0xaa,        // SFX_SACK: a member of staff sacked
    kSfxStaffDemand = 0x9a, // SFX_STAFF_DEMAND: a member of staff's demand opens
    kSfxBoo = 0xae,         // SFX_BOO: a treatment failed and its patient died
    kSfxCheer = 0xaf,       // SFX_CHEER: a patient cured
    kSfxBonus2 = 0xb8,      // SFX_BONUS2: a demand accepted
    kSfxFanfare = 0xd9,     // SFX_FANFARE: the hospital opens
    kSfxSnapshot = 0xda,    // SFX_SNAPSHOT: a view position stored
};
// Entries of sfx_details; play_sound ignores higher numbers.
constexpr int kSfxCount = 0xf2;

// Announcements (play_ambient's index into Ambients, src/sfxtable.cpp) the
// game makes, by first index; the announcer's samples are 500 on.
enum : uint8_t {
    kAnnounceRoom = 0x00,      // + room type (0-0x1e): a doctor or nurse wanted there (pager)
    kAnnounceEmergency = 0x1f, // + illness (0-0x23): an emergency lands
    kAnnounceMachine = 0x4c,   // + object (0-0x3d): a machine worn to 75% of its strength
    kAnnounceRandom = 0x89,    // + 0-33: do_ambients every 2000 turns
    kAnnounceQuake = 0xab,     // + 0-2: an earthquake starts and strikes
    kAnnounceEpidemic = 0xaf,  // + 0-2: an epidemic
    kAnnounceInspector = 0xb2, // + 0-2: the health inspector
    kAnnounceMayor = 0xb7,     // + mayor name (0-5): the mayor arrives
    kAnnounceCheat = 0xc4,     // + 0-1: a cheat
    kAnnounceLitter = 0xd7,    // the advisor on litter
    kAnnounceTooCold = 0xd8,   // the advisor on cold patients or staff
    kAnnounceTooHot = 0xd9,    // the advisor on hot patients or staff
};
// Entries of Ambients.
constexpr int kAnnouncementCount = 219;
// The announcer's sample for announcement `index` (500 + its Ambients
// sound), or 0 for an index with none.
int announcement_sample(int index);

// One sample of a sound bank: a RIFF WAVE file whose PCM data is `data`,
// frames of `channels` interleaved values, unsigned 8-bit or signed 16-bit
// little-endian.
struct Sample {
    std::string name;     // file name in the bank's directory
    uint8_t priority = 0; // the record's tag, matched by play type 3
    int channels = 0;
    int rate = 0; // frames per second
    int bits = 0; // 8 or 16
    std::span<const uint8_t> data;

    size_t frames() const { return channels && bits ? data.size() / size_t(channels * bits / 8) : 0; }
};

// A sound bank, SOUND/DATA/SOUND-<language>.DAT (or MINHEAP.DAT, the
// silent bank of a game without sound): sample files one after another,
// with a directory at the end for up to nine output formats. The game reads
// the 16-bit 22 kHz one (SoundType 0x656), the only one the banks hold.
class SoundBank
{
  public:
    // Reads the bank for `language` under the game data folder `dir`, as
    // setup_sound and init_sound_heap do. On failure says why in `error`.
    bool load(const std::filesystem::path &dir, int language, std::string *error = nullptr);
    // Reads the bank file at `path`.
    bool load_file(const std::filesystem::path &path, std::string *error = nullptr);

    // The number of records (samples_in_bank); sample numbers run from 1 to
    // count() - 1, record 0 is not a sample.
    int count() const { return int(samples_.size()); }
    // Sample `number`, or nullptr for 0, a number out of range or a record
    // that is not a PCM WAVE file.
    const Sample *sample(int number) const;

  private:
    std::vector<uint8_t> file_;
    std::vector<std::optional<Sample>> samples_;
};

// One thing's frame drawn in the view (draw_special_id_frame and
// draw_a_frame), at frame origin (x, y) in the view's pixels. Every drawn
// frame counts, with or without a sound: a frame without one clears the
// thing's last sound.
struct FrameSound {
    uint16_t thing = 0;
    uint8_t sound = 0;   // the frame's AnimFrame::sound
    bool person = false; // a patient or staff member (mapwho_person, mapwho_staff)
    int x = 0;
    int y = 0;
};

// How SoundPlayer::play_sample plays a sample: at `volume` and `pan` (0-127,
// 64 the centre), `pitch` percent of its rate, `loop` + 1 times (-1: until
// stopped).
struct SamplePlayback {
    int volume;
    int pan;
    int pitch;
    int loop;
};

// The game's sound effects: its rules for choosing, placing and repeating
// samples (src/sound.cpp, src/sndheap.cpp) over the sound library's ten
// voices (src/lib/bf1/opt: PlaySampleFromAddress and the rest, on Miles),
// mixed in software. Positions are in pixels of a screen `screen_width` wide
// (lbDisplay.physical_screen_width: the view).
class SoundPlayer
{
  public:
    // MaxSamples, as setup_sound asks for.
    static constexpr int kVoices = 10;
    // The full volume of a sample and of the sound effects setting.
    static constexpr int kMaxVolume = 127;

    // Plays from `bank` (statvar.sound_ok when it has samples); nullptr
    // stops every sample and plays nothing. The bank must outlive its use.
    void set_bank(const SoundBank *bank);
    bool sound_ok() const { return bank_ && bank_->count() > 1; }

    // myplayer.sfx_on and sfx_volume (0-127). A new player has them on at
    // full volume (as setup_sound and the default settings leave them).
    bool sfx_on() const { return sfx_on_ && sound_ok(); }
    int sfx_volume() const { return sfx_volume_; }
    // pulldown_toggle_sound: toggles the sound effects, stopping every
    // sample when they go off; they stay off without a bank.
    void toggle_sound();
    // pulldown_svol: sets the volume to (10 - item) * 127 / 10 for menu
    // item `item`, 0 (100%) to 9 (10%).
    void set_volume_item(int item);
    // The ticked volume item, 9 - volume * 10 / 128 (MenuContext::sound_volume).
    int volume_item() const { return 9 - sfx_volume_ * 10 / 128; }

    // myplayer.ambients_on and ambient_volume (0-127): the announcements,
    // on at full volume at first; off without a bank.
    bool ambients_on() const { return ambients_on_ && sound_ok(); }
    int ambient_volume() const { return ambient_volume_; }
    // pulldown_toggle_ambient.
    void toggle_ambient();
    // pulldown_sannounce: as set_volume_item for the announcements; the
    // next announcement plays at the new volume.
    void set_ambient_volume_item(int item);
    int ambient_volume_item() const { return 9 - ambient_volume_ * 10 / 128; }
    // init_ambients: empties the announcement queue (a level starts or
    // loads).
    void init_ambients();
    // play_ambient: queues announcement `index` (kAnnounce*) if the
    // announcements are on, it has a sample, the hospital has a
    // receptionist and the game is not paused. With the four places taken
    // it replaces the last of lower priority after the first, if any.
    void play_ambient(int index, bool receptionist, bool paused);
    // do_ambients, once a turn of the game view (and of a modal requester
    // over it): plays the next queued announcement when the last has
    // ended, at the announcements' volume, and every 2000 calls queues a
    // random one (kAnnounceRandom). Nothing while `paused`, on the rat
    // level or with the announcements off.
    void do_ambients(bool paused, bool rat_level, bool receptionist);

    // play_sound: plays sound effect `sfx` (SFX_*) for `source`, choosing the
    // sample and pitch by its sfx_details entry, or stops it. At screen x
    // its volume falls off linearly towards the screen's sides and it pans
    // with x; x or y -1 plays it at the setting's volume in the centre.
    // Nothing with the sound effects off.
    void play_sound(uint32_t source, int sfx, int x = -1, int y = -1, int screen_width = 640);

    // play_sample_using_heap and PlaySampleFromAddress: plays sample
    // `number` for `source` as `playback` says. `type` chooses the voice: 1 any free one, 2 a free one unless the
    // source is already playing the sample, 3 the voice of the source's
    // same sample (or same priority tag) whether or not it still plays,
    // else a free one. Returns false if nothing started.
    bool play_sample(uint32_t source, int number, const SamplePlayback &playback, int type);
    // StopSample: stops the first voice playing `number` for `source`.
    void stop_sample(uint32_t source, int number);
    // StopAllSamples.
    void stop_all();
    // IsSamplePlaying: `number` 0 asks for any sample of `source`.
    bool playing(uint32_t source, int number) const;
    // SetSampleVolume: of the first voice playing `number` for `source`
    // at another volume; volumes outside 0-127 are ignored.
    void set_sample_volume(uint32_t source, int number, int volume);

    // The frame sounds of a view's drawing, in drawing order: each frame's
    // sound plays (as play_sound for the thing, at the frame's origin)
    // unless it is the thing's last sound and its sfx_details entry does
    // not allow repeats (no entry does); the frame's sound becomes the
    // thing's last. Also counts the people drawn (othervar.visible_people)
    // for manage_background_sounds.
    void frames_drawn(std::span<const FrameSound> frames, int screen_width);

    // manage_sound, once a frame after drawing: re-places the samples of
    // things at their cell's centre on the screen (`camera` the map pixel at
    // its top-left, `screen_width` x `screen_height`), stopping those whose
    // cell is off it or whose source is not a thing of `world`.
    void manage_sound(const World &world, Point camera, int screen_width, int screen_height);

    // manage_background_sounds, once a frame after manage_sound unless
    // `paused`: loops the crowd at a volume set by the people drawn, plays a
    // random background sample every 400 calls, and the emergency clock
    // while `emergency_seconds` is set (louder in its last 33 seconds) and
    // the earthquake rumble while `quake`.
    void manage_background_sounds(bool paused, std::optional<int> emergency_seconds = std::nullopt, bool quake = false);

    // Mixes the next `frames` frames of the voices playing into `out`
    // (interleaved stereo, values -1 to 1) at `rate` frames per second,
    // advancing them; voices that end are freed.
    void mix(float *out, int frames, int rate);

    // How often play_sound has played each sound effect (SFX_*, kSfxCount
    // entries) since the player was made: the debug state text lists the
    // nonzero ones so tests can see a sound was played.
    const std::array<uint32_t, kSfxCount> &played_counts() const { return played_; }

    // The presentation's random generator (saferand's safeseed). The game
    // seeds it with the simulation's seed when a level starts.
    uint32_t safeseed = 0x546b2;

  private:
    struct Voice {
        const Sample *sample = nullptr; // nullptr: done (SMP_DONE)
        uint32_t source = 0;
        int number = 0;
        int volume = 0;
        int pan = 64;
        int pitch = 100;
        uint8_t priority = 0;
        int loops = 1;    // plays left, 0 for until stopped
        uint64_t pos = 0; // frames, 32.32 fixed point
    };

    uint16_t saferand();
    Voice *find(uint32_t source, int number);
    Voice *free_voice();
    // The voice a sample of play type `type` plays on, or nullptr for none.
    Voice *voice_for(uint32_t source, int number, const Sample &sample, int type);
    // Mixes `v` into `out` as mix() does, freeing it when it ends.
    static void mix_voice(Voice &v, float *out, int frames, int rate);

    const SoundBank *bank_ = nullptr;
    bool sfx_on_ = true;
    int sfx_volume_ = kMaxVolume;
    std::array<Voice, kVoices> voices_{};
    std::array<uint8_t, kThingCount> last_sound_{};
    std::array<uint32_t, kSfxCount> played_{};
    int visible_people_ = 0;
    int crowd_volume_ = 0;
    int clock_volume_ = 0;
    int ambient_noise_timer_ = 0;

    struct Announcement {
        uint8_t sound = 0; // sample - 500
        uint8_t priority = 0;
    };
    bool ambients_on_ = true;
    int ambient_volume_ = kMaxVolume;
    std::array<Announcement, 4> ambient_queue_{};
    int ambient_playing_ = 0;
    int ambient_timer_ = 0;
};

} // namespace th

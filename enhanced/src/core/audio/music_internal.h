#pragma once

// What the music code's files share: the OPL3 chip and driver, the XMIDI
// sequencer and the byte readers (src/lib/ail/ailxmidi.c, SOUND/OPL3.MDI).

#include "core/audio/music.h"

#include <ymfm_opl.h>

#include <array>
#include <cstdint>
#include <span>
#include <vector>

namespace th
{

inline namespace music_detail
{

uint32_t be32(const uint8_t *p);
uint32_t le32(const uint8_t *p);
uint16_t le16(const uint8_t *p);
uint32_t read_vln(const uint8_t *&p, const uint8_t *end);
int message_size(int status);

constexpr int kVoices = 18;
constexpr int kSlots = 20;
constexpr int kChannels = 16;
constexpr uint8_t kNone = 0xff;

// Each voice's modulator and carrier operator (0xc6b, 0xc7d); each
// operator's register offset and register set (0xc8f, 0xcb3); each voice's
// channel register offset (0xcd7; voices 9 on are in the second set, 0xce9).
constexpr uint8_t kModOp[kVoices] = {0, 1, 2, 6, 7, 8, 12, 13, 14, 18, 19, 20, 24, 25, 26, 30, 31, 32};
constexpr uint8_t kCarOp[kVoices] = {3, 4, 5, 9, 10, 11, 15, 16, 17, 21, 22, 23, 27, 28, 29, 33, 34, 35};
constexpr uint8_t kOpReg[18] = {0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 13, 16, 17, 18, 19, 20, 21};

// The emulated YMF262 at its usual 14.31818 MHz clock.
class Chip : public ymfm::ymfm_interface
{
  public:
    static constexpr uint32_t kClock = 14318180;

    Chip() : opl_(*this) { opl_.reset(); }
    uint32_t rate() const { return opl_.sample_rate(kClock); }
    void write(int reg, uint8_t value)
    {
        if (reg & 0x100)
            opl_.write_address_hi(uint8_t(reg));
        else
            opl_.write_address(uint8_t(reg));
        opl_.write_data(value);
    }
    // The next sample, left and right.
    void generate(int32_t &left, int32_t &right)
    {
        ymfm::ymf262::output_data out;
        opl_.generate(&out);
        left = out.data[0];
        right = out.data[1];
    }

  private:
    ymfm::ymf262 opl_;
};

class OplDriver
{
  public:
    explicit OplDriver(Chip &chip) : chip_(chip) { init(); }

    // MDI_INSTALL_T_SET for a sequence's TIMB list, from `bank`'s library:
    // installs the first missing timbre until none is; stops (false) at one
    // the library lacks.
    bool install_set(const std::vector<std::array<uint8_t, 2>> &timbres, const TimbreBank &bank);
    // A MIDI channel message (the MDI_MIDI_XMIT dispatch, 0x3b5b).
    void message(int status, int data1, int data2);

  private:
    struct Slot {
        uint8_t status = 0; // 0 free, 1 sounding (or waiting for a voice)
        uint8_t voice = kNone;
        uint8_t channel = 0;
        uint8_t key = 0;  // the note played: the MIDI note, or a drum's fixed note
        uint8_t note = 0; // the MIDI note, for note-off
        int8_t transpose = 0;
        uint8_t velocity = 0; // kVelGraph's
        bool sustained = false;
        uint8_t flags = 0;
        uint8_t b0 = 0;  // B0 last written
        uint8_t kon = 0; // 0x20 while keyed on
        uint8_t conn = 0;
        uint8_t fb_c = 0;
        uint8_t scale = 0; // bit 0: scale the modulator's level, bit 1 the carrier's
        // [0] modulator, [1] carrier.
        uint8_t flags20[2]{}; // AM, vibrato, sustain, KSR
        uint8_t mult[2]{};
        uint8_t ksl[2]{};
        uint8_t level[2]{}; // 63 - TL
        uint8_t ad[2]{};
        uint8_t sr[2]{};
        uint8_t ws[2]{};
        uint16_t priority = 0;       // its base, 0x7fff
        uint16_t steal_priority = 0; // steal_voices'
    };
    struct Channel {
        uint8_t bend_range = 0, volume = 0, pan = 0, bend_lsb = 0, bend_msb = 0, expression = 0, modulation = 0;
        uint8_t sustain = 0, voice_protect = 0, bank = 0, program = kNone;
        uint8_t timbre = kNone; // cache entry of (bank, program)
        uint8_t voices = 0;     // voices its notes hold
    };
    struct Cached {
        uint8_t bank, patch;
        std::vector<uint8_t> data;
    };

    void init();
    void write_op(int op, int base, uint8_t value)
    {
        chip_.write((op >= 18 ? 0x100 : 0) + kOpReg[op % 18] + base, value);
    }
    void write_voice(int voice, int base, uint8_t value)
    {
        chip_.write((voice >= 9 ? 0x100 : 0) + voice % 9 + base, value);
    }
    uint8_t find_timbre(int bank, int patch) const;
    void install(int bank, int patch, std::span<const uint8_t> data);
    void note_on(int ch, int key, int velocity);
    void note_off(int ch, int key);
    void release_sustain(int ch);
    void setup_2op(Slot &s, const uint8_t *t);
    void assign_voice(int slot);
    void steal_voices();
    void release(int slot);
    void update_voice(int slot);
    void update_channel(int ch, uint8_t flags);
    void control_change(int ch, int controller, uint8_t value);
    void reset_controllers(int ch);
    void all_notes_off(int ch);
    int set_steal_priorities();
    bool steal_candidates(int &needy, int &victim) const;
    void write_levels(const Slot &s, const Channel &c, const int (&ops)[2]);
    void write_frequency(Slot &s, const Channel &c);

    Chip &chip_;
    std::array<Slot, kSlots> slots_{};
    std::array<uint8_t, kVoices> voice_channel_{}; // owner channel, kNone free
    std::array<Channel, kChannels> channels_{};
    std::array<uint8_t, 128> drums_{}; // cache entry of bank 127's key
    std::vector<Cached> cache_;
    int last_voice_ = -1;
};

// --- The XMIDI sequencer ---------------------------------------------------
//
// One sequence of Miles' MIDI driver (AIL_init_sequence, XMI_serve and the
// rest of src/lib/ail/ailxmidi.c) with the preferences InitMusic sets: 120
// services a second, default volume 127, bend range 12, no loop branching.
// Channel locking and the callbacks are not ported: the game plays one
// sequence and registers none.
class Sequencer
{
  public:
    static constexpr int kServiceRate = 120;
    static constexpr int kIntervalTime = 1000000 / kServiceRate;

    explicit Sequencer(OplDriver &driver);

    enum Status { Done = 2, Playing = 4, Stopped = 8 };
    Status status() const { return status_; }

    // AIL_init_sequence and AIL_start_sequence; false if the driver lacks a
    // timbre the song lists (the game ignores that and plays on).
    bool start(const XmiSong &song, const TimbreBank &bank, int volume);
    void stop();
    void end();
    // AIL_resume_sequence.
    void resume();
    // AIL_set_sequence_volume.
    void set_volume(int volume, int milliseconds);
    // XMI_serve.
    void serve();

  private:
    static constexpr int kMaxNotes = 32;
    static constexpr int kForNest = 4;

    enum class Events { Wait, Done, Overflow };

    void send(int status, int data1, int data2);
    void for_loop(int count);
    void next_loop(int value);
    void branch(int index);
    void expire_notes();
    Events play_events();
    bool meta_event();
    bool channel_event(int status);
    void fade_volume();
    void update_volume();
    void flush_notes();

    OplDriver &driver_;
    const XmiSong *song_ = nullptr;
    Status status_ = Done;
    size_t pos_ = 0;
    int loop_count_ = 1;
    int interval_count_ = 0;
    int interval_num_ = 0;
    int volume_ = 127, volume_target_ = 127, volume_accum_ = 0, volume_period_ = 0;
    int tempo_error_ = 0;
    std::array<size_t, kForNest> for_pos_{};
    std::array<int, kForNest> for_count_{};
    std::array<int, 16> shadow_volume_{};
    std::array<int, 16> shadow_sustain_{};
    std::array<int, 16> shadow_voice_protect_{};
    std::array<int, kMaxNotes> note_chan_{};
    std::array<int, kMaxNotes> note_num_{};
    std::array<int, kMaxNotes> note_time_{};
    int note_count_ = 0;
};

} // namespace music_detail
} // namespace th

#include "core/audio/music.h"

#include "core/data/fs.h"

#include <ymfm_opl.h>

#include <algorithm>
#include <cstring>
#include <optional>

namespace th
{

namespace
{

uint32_t be32(const uint8_t *p)
{
    return uint32_t(p[0]) << 24 | uint32_t(p[1]) << 16 | uint32_t(p[2]) << 8 | uint32_t(p[3]);
}
uint32_t le32(const uint8_t *p)
{
    return uint32_t(p[0]) | uint32_t(p[1]) << 8 | uint32_t(p[2]) << 16 | uint32_t(p[3]) << 24;
}
uint16_t le16(const uint8_t *p)
{
    return uint16_t(p[0] | p[1] << 8);
}

// XMI_read_VLN: at most 4 bytes; `end` bounds the read.
uint32_t read_vln(const uint8_t *&p, const uint8_t *end)
{
    uint32_t value = 0;
    for (int i = 0; i < 4 && p < end; i++) {
        uint8_t byte = *p++;
        value = value << 7 | (byte & 0x7f);
        if (!(byte & 0x80))
            break;
    }
    return value;
}

// XMI_message_size.
int message_size(int status)
{
    switch (status & 0xf0) {
    case 0x80:
    case 0x90:
    case 0xa0:
    case 0xb0:
    case 0xe0:
        return 3;
    case 0xc0:
    case 0xd0:
        return 2;
    }
    return 0;
}

// --- The OPL3 driver -------------------------------------------------------
//
// SOUND/OPL3.MDI, Miles' YAMAHA driver for the OPL3, read from its machine
// code (SBPRO2.MDI, the driver the game's MDI.INI names, runs the same code
// and tables). Addresses below are offsets in OPL3.MDI.

// The F-number of each 1/16 semitone of an octave (freq table, 0x831). An
// entry with bit 15 clear sounds an octave below its note's block.
// clang-format off
constexpr uint16_t kFreq[192] = {
    0x2b2, 0x2b4, 0x2b7, 0x2b9, 0x2bc, 0x2be, 0x2c1, 0x2c3, 0x2c6, 0x2c9, 0x2cb, 0x2ce, 0x2d0, 0x2d3, 0x2d6, 0x2d8,
    0x2db, 0x2dd, 0x2e0, 0x2e3, 0x2e5, 0x2e8, 0x2eb, 0x2ed, 0x2f0, 0x2f3, 0x2f6, 0x2f8, 0x2fb, 0x2fe, 0x301, 0x303,
    0x306, 0x309, 0x30c, 0x30f, 0x311, 0x314, 0x317, 0x31a, 0x31d, 0x320, 0x323, 0x326, 0x329, 0x32b, 0x32e, 0x331,
    0x334, 0x337, 0x33a, 0x33d, 0x340, 0x343, 0x346, 0x349, 0x34c, 0x34f, 0x352, 0x356, 0x359, 0x35c, 0x35f, 0x362,
    0x365, 0x368, 0x36b, 0x36f, 0x372, 0x375, 0x378, 0x37b, 0x37f, 0x382, 0x385, 0x388, 0x38c, 0x38f, 0x392, 0x395,
    0x399, 0x39c, 0x39f, 0x3a3, 0x3a6, 0x3a9, 0x3ad, 0x3b0, 0x3b4, 0x3b7, 0x3bb, 0x3be, 0x3c1, 0x3c5, 0x3c8, 0x3cc,
    0x3cf, 0x3d3, 0x3d7, 0x3da, 0x3de, 0x3e1, 0x3e5, 0x3e8, 0x3ec, 0x3f0, 0x3f3, 0x3f7, 0x3fb, 0x3fe, 0xfe01, 0xfe03,
    0xfe05, 0xfe07, 0xfe08, 0xfe0a, 0xfe0c, 0xfe0e, 0xfe10, 0xfe12, 0xfe14, 0xfe16, 0xfe18, 0xfe1a, 0xfe1c, 0xfe1e,
    0xfe20, 0xfe21, 0xfe23, 0xfe25, 0xfe27, 0xfe29, 0xfe2b, 0xfe2d, 0xfe2f, 0xfe31, 0xfe34, 0xfe36, 0xfe38, 0xfe3a,
    0xfe3c, 0xfe3e, 0xfe40, 0xfe42, 0xfe44, 0xfe46, 0xfe48, 0xfe4a, 0xfe4c, 0xfe4f, 0xfe51, 0xfe53, 0xfe55, 0xfe57,
    0xfe59, 0xfe5c, 0xfe5e, 0xfe60, 0xfe62, 0xfe64, 0xfe67, 0xfe69, 0xfe6b, 0xfe6d, 0xfe6f, 0xfe72, 0xfe74, 0xfe76,
    0xfe79, 0xfe7b, 0xfe7d, 0xfe7f, 0xfe82, 0xfe84, 0xfe86, 0xfe89, 0xfe8b, 0xfe8d, 0xfe90, 0xfe92, 0xfe95, 0xfe97,
    0xfe99, 0xfe9c, 0xfe9e, 0xfea1, 0xfea3, 0xfea5, 0xfea8, 0xfeaa, 0xfead, 0xfeaf,
};
// clang-format on

// A note-on's velocity / 8 to its level scale (vel_graph, 0xc5b).
constexpr uint8_t kVelGraph[16] = {82, 85, 88, 91, 94, 97, 100, 103, 106, 109, 112, 115, 118, 121, 124, 127};

constexpr int kVoices = 18;
constexpr int kSlots = 20;
constexpr int kChannels = 16;
constexpr int kDrumChannel = 9;
constexpr uint8_t kNone = 0xff;

// Each voice's modulator and carrier operator (0xc6b, 0xc7d); each
// operator's register offset and register set (0xc8f, 0xcb3); each voice's
// channel register offset (0xcd7; voices 9 on are in the second set, 0xce9).
constexpr uint8_t kModOp[kVoices] = {0, 1, 2, 6, 7, 8, 12, 13, 14, 18, 19, 20, 24, 25, 26, 30, 31, 32};
constexpr uint8_t kCarOp[kVoices] = {3, 4, 5, 9, 10, 11, 15, 16, 17, 21, 22, 23, 27, 28, 29, 33, 34, 35};
constexpr uint8_t kOpReg[18] = {0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 13, 16, 17, 18, 19, 20, 21};

// The registers the driver's hardware set-up (0x2c4e) writes, both register
// sets, each from 1 to 0xf5: what is not listed here it clears.
struct InitRange {
    int first, last;
    uint8_t value;
};
constexpr InitRange kInitRegs[] = {
    {0x01, 0x01, 0x20}, {0x04, 0x04, 0x60}, {0x20, 0x35, 0x01}, {0x40, 0x55, 0x3f},
    {0x60, 0x75, 0xff}, {0x80, 0x95, 0x0f}, {0xbd, 0xbd, 0xc0},
};
constexpr InitRange kInitRegsHigh[] = {
    {0x105, 0x105, 0x01}, {0x120, 0x135, 0x01}, {0x140, 0x155, 0x3f}, {0x160, 0x175, 0xff}, {0x180, 0x195, 0x0f},
};

// update_voice's flags: which registers of a voice to rewrite.
enum : uint8_t {
    kUpdateFreq = 0x01,     // A0/B0: frequency and key on
    kUpdateFeedback = 0x08, // C0: feedback, connection and stereo
    kUpdateWave = 0x10,     // E0
    kUpdateEnvelope = 0x20, // 60 and 80
    kUpdateLevel = 0x40,    // 40
    kUpdateFlags = 0x80,    // 20
    kUpdateAll = 0xf9,
};

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

void OplDriver::init()
{
    // The hardware set-up (0x2c4e): OPL3 mode, then every register.
    chip_.write(0x105, 1);
    chip_.write(0x104, 0);
    for (int reg = 1; reg <= 0xf5; reg++) {
        uint8_t value = 0;
        for (const InitRange &r : kInitRegs)
            if (reg >= r.first && reg <= r.last)
                value = r.value;
        chip_.write(reg, value);
    }
    for (int reg = 0x101; reg <= 0x1f5; reg++) {
        uint8_t value = 0;
        for (const InitRange &r : kInitRegsHigh)
            if (reg >= r.first && reg <= r.last)
                value = r.value;
        chip_.write(reg, value);
    }
    // The instrument manager (0x2fe8).
    voice_channel_.fill(kNone);
    drums_.fill(kNone);
    for (Channel &c : channels_)
        c = Channel{};
}

uint8_t OplDriver::find_timbre(int bank, int patch) const
{
    for (size_t i = 0; i < cache_.size(); i++)
        if (cache_[i].bank == bank && cache_[i].patch == patch)
            return uint8_t(i);
    return kNone;
}

// The timbre cache (0x2f1c) holds 192 timbres in 4 KB and evicts the least
// recently played when full; the game's library (190 timbres of 14 bytes)
// always fits, so eviction is not ported.
void OplDriver::install(int bank, int patch, std::span<const uint8_t> data)
{
    uint8_t index = find_timbre(bank, patch);
    if (index == kNone) {
        if (cache_.size() >= 0xc0)
            return;
        index = uint8_t(cache_.size());
        cache_.push_back({uint8_t(bank), uint8_t(patch), {data.begin(), data.end()}});
    }
    for (Channel &c : channels_)
        if (c.program == patch && c.bank == bank)
            c.timbre = index;
}

bool OplDriver::install_set(const std::vector<std::array<uint8_t, 2>> &timbres, const TimbreBank &bank)
{
    for (;;) {
        auto missing = std::find_if(timbres.begin(), timbres.end(),
                                    [&](const auto &t) { return find_timbre(t[0], t[1]) == kNone; });
        if (missing == timbres.end())
            return true;
        std::span<const uint8_t> data = bank.find((*missing)[0], (*missing)[1]);
        if (data.empty())
            return false;
        install((*missing)[0], (*missing)[1], data);
    }
}

void OplDriver::message(int status, int data1, int data2)
{
    const int ch = status & 0x0f;
    Channel &c = channels_[size_t(ch)];
    switch (status & 0xf0) {
    case 0x90:
        if (data2) {
            note_on(ch, data1, data2);
            return;
        }
        [[fallthrough]];
    case 0x80:
        note_off(ch, data1);
        return;
    case 0xe0:
        c.bend_lsb = uint8_t(data1);
        c.bend_msb = uint8_t(data2);
        update_channel(ch, kUpdateFreq);
        return;
    case 0xc0:
        c.program = uint8_t(data1);
        c.timbre = find_timbre(c.bank, c.program);
        return;
    case 0xb0:
        control_change(ch, data1, uint8_t(data2));
        return;
    default:
        return;
    }
}

// message's controller changes (status 0xb0).
void OplDriver::control_change(int ch, int controller, uint8_t value)
{
    Channel &c = channels_[size_t(ch)];
    switch (controller) {
    case 114: // PATCH_BANK_SEL
        c.bank = value;
        break;
    case 112: // VOICE_PROTECT
        c.voice_protect = value;
        break;
    case 113: // TIMBRE_PROTECT: guards the cache entry from eviction, not ported
        break;
    case 1:
        c.modulation = value;
        update_channel(ch, kUpdateFlags);
        break;
    case 7:
        c.volume = value;
        update_channel(ch, kUpdateLevel);
        break;
    case 11:
        c.expression = value;
        update_channel(ch, kUpdateLevel);
        break;
    case 10:
        c.pan = value;
        update_channel(ch, kUpdateFeedback);
        break;
    case 64:
        c.sustain = value;
        if (value < 64)
            release_sustain(ch);
        break;
    case 121:
        reset_controllers(ch);
        break;
    case 123:
        all_notes_off(ch);
        break;
    case 6: // data entry: the bend range, whatever the RPN
        c.bend_range = value;
        break;
    default:
        break;
    }
}

// Controller 121, reset all controllers.
void OplDriver::reset_controllers(int ch)
{
    Channel &c = channels_[size_t(ch)];
    c.sustain = 0;
    release_sustain(ch);
    c.modulation = 0;
    c.expression = 127;
    c.bend_lsb = 0;
    c.bend_msb = 0x40;
    update_channel(ch, kUpdateFlags | kUpdateLevel | kUpdateFreq);
}

// Controller 123, all notes off; matched by the played key (see
// release_sustain).
void OplDriver::all_notes_off(int ch)
{
    for (const Slot &s : slots_)
        if (s.status == 1 && s.channel == ch)
            note_off(ch, s.key);
}

void OplDriver::update_channel(int ch, uint8_t flags)
{
    for (int i = 0; i < kSlots; i++) {
        if (slots_[size_t(i)].status && slots_[size_t(i)].channel == ch) {
            slots_[size_t(i)].flags |= flags;
            update_voice(i);
        }
    }
}

// 0x3a36. A note with no timbre or no free slot is dropped.
void OplDriver::note_on(int ch, int key, int velocity)
{
    uint8_t index = channels_[size_t(ch)].timbre;
    if (ch == kDrumChannel) {
        uint8_t &drum = drums_[size_t(key)];
        if (drum == kNone)
            drum = find_timbre(127, key);
        index = drum;
    }
    if (index == kNone)
        return;
    const uint8_t *t = cache_[index].data.data();
    auto free = std::find_if(slots_.begin(), slots_.end(), [](const Slot &s) { return s.status == 0; });
    // Only 2-operator timbres (14 bytes) are ported: the game's library has
    // no 4-operator (25-byte) ones.
    if (free == slots_.end() || le16(t) != 14)
        return;
    Slot &s = *free;
    s = Slot{};
    s.channel = uint8_t(ch);
    s.note = uint8_t(key);
    // A drum timbre's transpose byte is the note it plays.
    s.key = ch == kDrumChannel ? t[2] : uint8_t(key);
    s.transpose = ch == kDrumChannel ? 0 : int8_t(t[2]);
    s.velocity = kVelGraph[velocity >> 3];
    s.status = 1;
    setup_2op(s, t);
    assign_voice(int(free - slots_.begin()));
}

// 0x38f8: the timbre record's bytes after its length and transpose are the
// modulator's 20, 40, 60, 80 and E0 registers, C0, then the carrier's five.
void OplDriver::setup_2op(Slot &s, const uint8_t *t)
{
    s.kon = 0x20;
    s.priority = 0x7fff;
    s.fb_c = t[8];
    s.conn = t[8] & 1;
    for (int op = 0; op < 2; op++) {
        const uint8_t *r = t + 3 + op * 6;
        s.flags20[op] = r[0] & 0xf0;
        s.mult[op] = r[0] & 0x0f;
        s.ksl[op] = r[1] & 0xc0;
        s.level[op] = ~r[1] & 0x3f;
        s.ad[op] = r[2];
        s.sr[op] = r[3];
        s.ws[op] = r[4];
    }
    // The carrier's level always follows the volume, the modulator's only
    // when additive.
    s.scale = s.conn | 2;
    s.flags = kUpdateAll;
}

// 0x308c: the next free voice after the last one given, else steal_voices.
void OplDriver::assign_voice(int slot)
{
    Slot &s = slots_[size_t(slot)];
    for (int tries = 0; tries < kVoices; tries++) {
        last_voice_ = (last_voice_ + 1) % kVoices;
        if (voice_channel_[size_t(last_voice_)] == kNone) {
            s.voice = uint8_t(last_voice_);
            channels_[s.channel].voices++;
            voice_channel_[size_t(last_voice_)] = s.channel;
            s.flags = kUpdateAll;
            update_voice(slot);
            return;
        }
    }
    steal_voices();
}

// 0x36fd: a slot's priority is 0x7fff (0xffff on a voice-protected
// channel) less the voices its channel holds. While the highest waiting
// note's priority is at least the lowest sounding one's, the sounding note
// is cut and its voice goes to the waiting one.
void OplDriver::steal_voices()
{
    int active = set_steal_priorities();
    while (active) {
        int needy = -1, victim = -1;
        if (!steal_candidates(needy, victim))
            return;
        uint8_t voice = slots_[size_t(victim)].voice;
        release(victim);
        Slot &s = slots_[size_t(needy)];
        s.voice = voice;
        channels_[s.channel].voices++;
        voice_channel_[voice] = s.channel;
        s.flags = kUpdateAll;
        update_voice(needy);
        active--;
    }
}

// steal_voices' priorities, set on each sounding slot; returns how many
// there are.
int OplDriver::set_steal_priorities()
{
    int active = 0;
    for (Slot &s : slots_) {
        if (!s.status)
            continue;
        active++;
        const Channel &c = channels_[s.channel];
        int p = c.voice_protect >= 64 ? 0xffff : s.priority;
        s.steal_priority = uint16_t(std::max(p - int(c.voices), 0));
    }
    return active;
}

// steal_voices' choice: the waiting slot with the highest priority
// (`needy`) and the sounding one with the lowest (`victim`), the last of
// equals; false when no voice is to be stolen.
bool OplDriver::steal_candidates(int &needy, int &victim) const
{
    unsigned best = 0, lowest = 0xffff;
    for (int i = 0; i < kSlots; i++) {
        const Slot &s = slots_[size_t(i)];
        if (!s.status)
            continue;
        if (s.voice == kNone) {
            if (s.steal_priority >= best) {
                best = s.steal_priority;
                needy = i;
            }
        } else if (s.steal_priority <= lowest) {
            lowest = s.steal_priority;
            victim = i;
        }
    }
    return !(best < lowest || best == 0 || needy < 0 || victim < 0);
}

// 0x312e: keys the voice off and frees it, and the slot with it.
void OplDriver::release(int slot)
{
    Slot &s = slots_[size_t(slot)];
    if (s.voice == kNone)
        return;
    s.kon = 0;
    s.flags |= kUpdateFreq;
    update_voice(slot);
    channels_[s.channel].voices--;
    voice_channel_[s.voice] = kNone;
    s.voice = kNone;
    s.status = 0;
}

// 0x39d3: under the sustain pedal the note is only marked.
void OplDriver::note_off(int ch, int key)
{
    for (int i = 0; i < kSlots; i++) {
        Slot &s = slots_[size_t(i)];
        if (s.status != 1 || s.note != key || s.channel != ch)
            continue;
        if (channels_[size_t(ch)].sustain >= 64) {
            s.sustained = true;
        } else {
            release(i);
            s.status = 0;
        }
    }
}

// 0x3b25: ends the sustained notes of `ch`. The driver passes the played
// key, not the MIDI note note_off matches, so a sustained drum note is never
// ended here.
void OplDriver::release_sustain(int ch)
{
    for (const Slot &s : slots_)
        if (s.status && s.channel == ch && s.sustained)
            note_off(ch, s.key);
}

// 0x318b, for 2-operator voices.
void OplDriver::update_voice(int slot)
{
    Slot &s = slots_[size_t(slot)];
    if (s.voice == kNone)
        return;
    const Channel &c = channels_[s.channel];
    const int voice = s.voice;
    const int ops[2] = {kModOp[voice], kCarOp[voice]};
    if (s.flags & kUpdateFlags) {
        uint8_t vibrato = c.modulation >= 64 ? 0x40 : 0;
        for (int op = 0; op < 2; op++)
            write_op(ops[op], 0x20, s.flags20[op] | s.mult[op] | vibrato);
    }
    if (s.flags & kUpdateLevel)
        write_levels(s, c, ops);
    if (s.flags & kUpdateEnvelope) {
        for (int op = 0; op < 2; op++) {
            write_op(ops[op], 0x60, s.ad[op]);
            write_op(ops[op], 0x80, s.sr[op]);
        }
    }
    if (s.flags & kUpdateWave) {
        for (int op = 0; op < 2; op++)
            write_op(ops[op], 0xe0, s.ws[op]);
    }
    if (s.flags & kUpdateFeedback) {
        // Both outputs; a pan of 27 or less is left only, 100 or more right.
        uint8_t value = (s.fb_c & 0x0e) | s.conn | 0x30;
        if (c.pan <= 27)
            value &= ~0x20;
        else if (c.pan >= 100)
            value &= ~0x10;
        write_voice(voice, 0xc0, value);
    }
    if (s.flags & kUpdateFreq) {
        if (!(s.kon & 0x20))
            write_voice(voice, 0xb0, s.b0 & ~0x20);
        else
            write_frequency(s, c);
    }
    s.flags = 0;
}

// update_voice's levels (registers 40): the volume, expression and
// velocity scaling the operators scale picks.
void OplDriver::write_levels(const Slot &s, const Channel &c, const int (&ops)[2])
{
    // Rounded up unless zero, as the driver does after each product.
    auto scale = [](int a, int b) {
        int v = (a * b) >> 7 & 0xff;
        return v ? v + 1 : 0;
    };
    int volume = scale(scale(c.volume, c.expression), s.velocity);
    for (int op = 0; op < 2; op++) {
        int level = s.level[op];
        if (s.scale & (1 << op))
            level = level * volume / 127;
        write_op(ops[op], 0x40, uint8_t((~level & 0x3f) | s.ksl[op]));
    }
}

// update_voice's frequency and key on (registers A0 and B0) for a keyed-on
// slot.
void OplDriver::write_frequency(Slot &s, const Channel &c)
{
    // The pitch in 1/16 semitones: the note (moved by octaves into 12-107,
    // then less 12) plus the bend times its range.
    int bend = ((c.bend_msb << 7 | c.bend_lsb) - 0x2000) >> 5;
    int note = s.key + s.transpose - 12;
    while (note < 0)
        note += 12;
    while (note > 95)
        note -= 12;
    int pitch = int16_t(uint16_t(bend * c.bend_range + (note << 8) + 8)) >> 4;
    while (pitch < 0)
        pitch += 0xc0;
    while (pitch > 0x5ff)
        pitch -= 0xc0;
    int semitone = pitch >> 4;
    uint16_t f = kFreq[(semitone % 12) * 16 + (pitch & 0xf)];
    int block = semitone / 12 - 1;
    if (f & 0x8000)
        block++;
    if (block < 0) {
        block = 0;
        f >>= 1;
    }
    s.b0 = uint8_t(((f >> 8) & 3) | block << 2 | s.kon);
    write_voice(s.voice, 0xa0, uint8_t(f));
    write_voice(s.voice, 0xb0, s.b0);
}

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

Sequencer::Sequencer(OplDriver &driver) : driver_(driver)
{
    // XMI_construct_MDI_driver's reset of every channel.
    for (int ch = 0; ch < 16; ch++) {
        driver_.message(0xb0 | ch, 114, 0);
        driver_.message(0xc0 | ch, 0, 0);
        driver_.message(0xe0 | ch, 0, 0x40);
        driver_.message(0xb0 | ch, 112, 0);
        driver_.message(0xb0 | ch, 1, 0);
        driver_.message(0xb0 | ch, 7, 127);
        driver_.message(0xb0 | ch, 10, 64);
        driver_.message(0xb0 | ch, 11, 127);
        driver_.message(0xb0 | ch, 64, 0);
        driver_.message(0xb0 | ch, 91, 40);
        driver_.message(0xb0 | ch, 93, 0);
        driver_.message(0xb0 | ch, 100, 0);
        driver_.message(0xb0 | ch, 101, 0);
        driver_.message(0xb0 | ch, 38, 0);
        driver_.message(0xb0 | ch, 6, 12);
    }
}

bool Sequencer::start(const XmiSong &song, const TimbreBank &bank, int volume)
{
    end();
    song_ = &song;
    loop_count_ = 1;
    volume_ = volume_target_ = 127;
    volume_period_ = volume_accum_ = 0;
    tempo_error_ = 0;
    // XMI_rewind_sequence.
    shadow_volume_.fill(-1);
    shadow_sustain_.fill(-1);
    shadow_voice_protect_.fill(-1);
    for_count_.fill(-1);
    note_chan_.fill(-1);
    note_count_ = 0;
    interval_count_ = 0;
    interval_num_ = 0;
    pos_ = song.events;
    bool ok = song.timbres.empty() || driver_.install_set(song.timbres, bank);
    // PlayMidiFromAddress sets a volume below full at once.
    if (volume < 127)
        set_volume(volume, 0);
    status_ = Playing;
    return ok;
}

void Sequencer::flush_notes()
{
    for (int i = 0; i < kMaxNotes; i++) {
        if (note_chan_[size_t(i)] == -1)
            continue;
        send(note_chan_[size_t(i)] | 0x80, note_num_[size_t(i)], 0);
        note_chan_[size_t(i)] = -1;
    }
    note_count_ = 0;
}

// AIL_stop_sequence.
void Sequencer::stop()
{
    if (status_ != Playing)
        return;
    status_ = Stopped;
    flush_notes();
    for (int ch = 0; ch < 16; ch++) {
        if (shadow_sustain_[size_t(ch)] >= 64)
            driver_.message(0xb0 | ch, 64, 0);
        if (shadow_voice_protect_[size_t(ch)] >= 64)
            driver_.message(0xb0 | ch, 112, 0);
    }
}

// AIL_resume_sequence. Its XMI_refresh_channel resends every controller the
// sequence set; the driver still holds them all, nothing having played
// since, but for the sustain and voice protection stop() released.
void Sequencer::resume()
{
    if (status_ != Stopped)
        return;
    for (int ch = 0; ch < 16; ch++) {
        if (shadow_voice_protect_[size_t(ch)] != -1)
            driver_.message(0xb0 | ch, 112, shadow_voice_protect_[size_t(ch)]);
        if (shadow_sustain_[size_t(ch)] != -1)
            driver_.message(0xb0 | ch, 64, shadow_sustain_[size_t(ch)]);
    }
    status_ = Playing;
}

// AIL_end_sequence.
void Sequencer::end()
{
    if (status_ == Done)
        return;
    stop();
    status_ = Done;
}

void Sequencer::set_volume(int volume, int milliseconds)
{
    volume_target_ = volume;
    if (volume_ == volume_target_)
        return;
    if (milliseconds == 0) {
        volume_ = volume_target_;
    } else {
        volume_accum_ = 0;
        volume_period_ = milliseconds * 1000 / std::abs(volume_ - volume_target_);
    }
    update_volume();
}

// XMI_update_volume.
void Sequencer::update_volume()
{
    for (int ch = 0; ch < 16; ch++)
        if (shadow_volume_[size_t(ch)] != -1)
            send(0xb0 | ch, 7, shadow_volume_[size_t(ch)]);
}

// XMI_send_channel_voice_message, for one sequence on unlocked channels.
void Sequencer::send(int status, int data1, int data2)
{
    const int st = status & 0xf0, ch = status & 0x0f;
    if (st == 0xb0) {
        switch (data1) {
        case 7:
            shadow_volume_[size_t(ch)] = int8_t(data2);
            data2 = std::clamp(volume_ * data2 * 127 / (127 * 127), 0, 127);
            break;
        case 64:
            shadow_sustain_[size_t(ch)] = int8_t(data2);
            break;
        case 112:
            shadow_voice_protect_[size_t(ch)] = int8_t(data2);
            break;
        case 6: // the bend range goes out with its RPN select
            driver_.message(0xb0 | ch, 100, 0);
            driver_.message(0xb0 | ch, 101, 0);
            driver_.message(0xb0 | ch, 38, 0);
            break;
        case 116: // FOR_LOOP
            for_loop(data2);
            return;
        case 117: // NEXT_LOOP
            next_loop(data2);
            return;
        case 109: // SEQ_BRANCH: AIL_branch_index
            branch(data2);
            return;
        case 108: // CALLBACK_PFX
        case 110: // CHAN_LOCK
        case 111: // CHAN_PROTECT
        case 115: // INDIRECT_C_PFX
        case 118: // CLEAR_BEAT_BAR
        case 119: // CALLBACK_TRIG
            return;
        default:
            break;
        }
    }
    driver_.message(status, data1, data2);
}

// FOR_LOOP: opens a loop of `count` passes in the first free nesting level.
void Sequencer::for_loop(int count)
{
    for (int i = 0; i < kForNest; i++) {
        if (for_count_[size_t(i)] == -1) {
            for_count_[size_t(i)] = count;
            for_pos_[size_t(i)] = pos_;
            break;
        }
    }
}

// NEXT_LOOP: for a value of 64 or more, jumps back to the innermost loop's
// start, or closes it after its last pass.
void Sequencer::next_loop(int value)
{
    if (value < 64)
        return;
    int i = kForNest - 1;
    while (i >= 0 && for_count_[size_t(i)] == -1)
        i--;
    if (i < 0)
        return;
    if (for_count_[size_t(i)] == 0 || --for_count_[size_t(i)] != 0)
        pos_ = for_pos_[size_t(i)];
    else
        for_count_[size_t(i)] = -1;
}

// AIL_branch_index: jumps to the RBRN entry `index`, if the song has it.
void Sequencer::branch(int index)
{
    if (!song_->branches)
        return;
    const uint8_t *r = song_->image.data() + song_->branches;
    int n = int16_t(le16(r + 8));
    for (int i = 0; i < n; i++) {
        const uint8_t *e = r + 10 + 6 * i;
        if (le16(e) == index && le32(e + 2) < song_->events_size) {
            pos_ = song_->events + le32(e + 2);
            interval_count_ = 0;
            for_count_.fill(-1);
            break;
        }
    }
}

void Sequencer::serve()
{
    if (status_ != Playing)
        return;
    interval_num_++;
    const uint8_t *data = song_->image.data();
    const size_t end = song_->events + song_->events_size;
    bool done = false;
    tempo_error_ += 100;
    while (tempo_error_ >= 100) {
        tempo_error_ -= 100;
        expire_notes();
        if (--interval_count_ > 0)
            continue;
        const Events events = play_events();
        if (events == Events::Overflow)
            return;
        if (events == Events::Done) {
            done = true;
            break;
        }
        interval_count_ = pos_ < end ? data[pos_++] : 0;
    }
    if (done || status_ != Playing)
        return;
    fade_volume();
}

// serve's note timers: ends each note whose time has run out.
void Sequencer::expire_notes()
{
    for (int i = 0; i < kMaxNotes && note_count_ > 0; i++) {
        if (note_chan_[size_t(i)] == -1 || --note_time_[size_t(i)] > 0)
            continue;
        send(note_chan_[size_t(i)] | 0x80, note_num_[size_t(i)], 0);
        note_chan_[size_t(i)] = -1;
        note_count_--;
    }
}

// serve's events up to the next delay: Wait at a delay byte, Done at the end
// of the song, Overflow when the note queue overflowed and the sequence
// ended.
Sequencer::Events Sequencer::play_events()
{
    const uint8_t *data = song_->image.data();
    const size_t end = song_->events + song_->events_size;
    for (;;) {
        // parse_xmi checked the events up to the end of track; a jump
        // elsewhere may still run off the chunk, taken as its end.
        const int status = pos_ < end ? data[pos_] : 0xff;
        if (status < 0x80)
            return Events::Wait;
        if (pos_ >= end || status == 0xff) {
            if (meta_event())
                return Events::Done;
        } else if (status == 0xf0 || status == 0xf7) {
            // The driver ignores system exclusive messages.
            const uint8_t *p = data + pos_ + 1;
            uint32_t len = read_vln(p, data + end);
            pos_ = size_t(p - data) + len;
        } else if (!channel_event(status)) {
            return Events::Overflow;
        }
    }
}

// A meta event at pos_ (or the chunk's end): skipped, but for the end of
// track, which loops the song or ends it, returning true.
bool Sequencer::meta_event()
{
    const uint8_t *data = song_->image.data();
    const size_t end = song_->events + song_->events_size;
    int type = 0x2f;
    if (pos_ < end) {
        const uint8_t *p = data + pos_ + 2;
        type = pos_ + 1 < end ? data[pos_ + 1] : 0x2f;
        uint32_t len = read_vln(p, data + end);
        pos_ = size_t(p - data) + len;
    }
    if (type != 0x2f)
        return false;
    if (loop_count_ == 0 || --loop_count_ != 0) {
        pos_ = song_->events;
    } else {
        loop_count_ = 1;
        stop();
        status_ = Done;
    }
    return true;
}

// A channel message at pos_, sent; a note-on is queued for its note-off.
// False when the note queue overflows, which ends the sequence.
bool Sequencer::channel_event(int status)
{
    const uint8_t *data = song_->image.data();
    const size_t end = song_->events + song_->events_size;
    const int size = message_size(status);
    if (pos_ + size_t(size) > end) {
        pos_ = end;
        return true;
    }
    const size_t at = pos_;
    pos_ += size_t(size);
    send(status, data[at + 1], size == 3 ? data[at + 2] : 0);
    if ((status & 0xf0) != 0x90)
        return true;
    int slot = 0;
    while (slot < kMaxNotes && note_chan_[size_t(slot)] != -1)
        slot++;
    if (slot == kMaxNotes) {
        // "Internal note queue overflow".
        stop();
        status_ = Done;
        return false;
    }
    note_count_++;
    note_chan_[size_t(slot)] = status & 0x0f;
    note_num_[size_t(slot)] = data[at + 1];
    const uint8_t *p = data + pos_;
    note_time_[size_t(slot)] = int(read_vln(p, data + end));
    pos_ = size_t(p - data);
    return true;
}

// serve's volume fade towards volume_target_.
void Sequencer::fade_volume()
{
    if (volume_ == volume_target_)
        return;
    volume_accum_ += kIntervalTime;
    while (volume_accum_ >= volume_period_) {
        volume_accum_ -= volume_period_;
        volume_ += volume_target_ > volume_ ? 1 : -1;
        if (volume_ == volume_target_)
            break;
    }
    // Only every eighth service resends the volume, so a fade can end a few
    // steps short of its target.
    if (!(interval_num_ & 7))
        update_volume();
}

} // namespace

// --- Files -------------------------------------------------------------------

bool TimbreBank::load(const std::filesystem::path &path, std::string *error)
{
    timbres_.clear();
    auto fail = [&](const char *why) {
        if (error)
            *error = path.string() + ": " + why;
        timbres_.clear();
        return false;
    };
    auto file = read_file(path);
    if (!file)
        return fail("cannot read");
    const std::vector<uint8_t> &d = *file;
    for (size_t at = 0;; at += 6) {
        if (at + 2 > d.size())
            return fail("no end of directory");
        if (d[at + 1] == 0xff)
            break;
        if (at + 6 > d.size())
            return fail("truncated directory");
        size_t offset = le32(&d[at + 2]);
        if (offset + 2 > d.size())
            return fail("timbre outside the file");
        size_t length = le16(&d[offset]);
        if (length < 14 || offset + length > d.size())
            return fail("bad timbre length");
        timbres_.push_back({d[at + 1], d[at], {d.begin() + long(offset), d.begin() + long(offset + length)}});
    }
    return true;
}

std::span<const uint8_t> TimbreBank::find(int bank, int patch) const
{
    for (const Timbre &t : timbres_)
        if (t.bank == bank && t.patch == patch)
            return t.data;
    return {};
}

namespace
{

// The end of the IFF chunk at `at`, or 0 if it runs past the file.
size_t chunk_end(const std::vector<uint8_t> &d, size_t at)
{
    uint64_t end = uint64_t(at) + 8 + be32(&d[at + 4]);
    return end > d.size() ? 0 : size_t(end);
}

// XMI_find_sequence, sequence 0: skip chunks until one whose form type is
// XMID; a FORM is the sequence, a CAT holds it as its first FORM XMID. Sets
// `at` to the FORM; returns the failure, or nullptr.
const char *find_sequence(const std::vector<uint8_t> &d, size_t &at)
{
    at = 0;
    for (;;) {
        if (at + 12 > d.size() || (std::memcmp(&d[at], "FORM", 4) && std::memcmp(&d[at], "CAT ", 4)))
            return "no XMIDI sequence";
        if (!std::memcmp(&d[at + 8], "XMID", 4))
            break;
        if (!chunk_end(d, at))
            return "truncated chunk";
        at = chunk_end(d, at);
    }
    if (std::memcmp(&d[at], "CAT ", 4))
        return nullptr;
    size_t end = chunk_end(d, at);
    if (!end)
        return "truncated CAT";
    at += 12;
    while (at + 12 <= end && std::memcmp(&d[at + 8], "XMID", 4)) {
        if (!chunk_end(d, at))
            return "truncated chunk";
        at = chunk_end(d, at);
    }
    if (at + 12 > end)
        return "no XMIDI sequence";
    return nullptr;
}

// Reads the TIMB, RBRN and EVNT chunks of the FORM XMID at `at` into
// `song`; returns the failure, or nullptr.
const char *read_form(size_t at, XmiSong &song)
{
    const std::vector<uint8_t> &d = song.image;
    size_t end = chunk_end(d, at);
    if (!end)
        return "truncated FORM";
    for (at += 12; at + 8 <= end;) {
        size_t next = chunk_end(d, at);
        if (!next || next > end)
            return "truncated chunk";
        if (!std::memcmp(&d[at], "TIMB", 4) && next - at >= 10) {
            size_t n = le16(&d[at + 8]);
            for (size_t i = 0; i < n && at + 12 + 2 * i <= next; i++)
                song.timbres.push_back({d[at + 11 + 2 * i], d[at + 10 + 2 * i]});
        } else if (!std::memcmp(&d[at], "RBRN", 4) && next - at >= 10) {
            song.branches = at;
        } else if (!std::memcmp(&d[at], "EVNT", 4)) {
            song.events = at + 8;
            song.events_size = next - song.events;
        }
        // Miles steps chunk by chunk without IFF's pad byte.
        at = next;
    }
    return nullptr;
}

// Walks the song's events, adding up its ticks; true if they reach an end
// of track within their chunk.
bool scan_events(XmiSong &song)
{
    const std::vector<uint8_t> &d = song.image;
    const size_t ev_end = song.events + song.events_size;
    const uint8_t *p = d.data() + song.events, *e = d.data() + ev_end;
    while (p < e) {
        uint8_t status = *p;
        if (status < 0x80) {
            song.ticks += status;
            p++;
        } else if (status == 0xff) {
            if (e - p < 2)
                return false;
            uint8_t type = p[1];
            p += 2;
            uint32_t len = read_vln(p, e);
            if (len > size_t(e - p))
                return false;
            p += len;
            if (type == 0x2f)
                return true;
        } else if (status == 0xf0 || status == 0xf7) {
            p++;
            uint32_t len = read_vln(p, e);
            if (len > size_t(e - p))
                return false;
            p += len;
        } else {
            int size = message_size(status);
            if (e - p < size)
                return false;
            p += size;
            if ((status & 0xf0) == 0x90)
                read_vln(p, e);
        }
    }
    return false;
}

} // namespace

bool parse_xmi(std::span<const uint8_t> file, XmiSong &song, std::string *error)
{
    auto fail = [&](const char *why) {
        if (error)
            *error = why;
        return false;
    };
    song = XmiSong{};
    song.image.assign(file.begin(), file.end());
    size_t at = 0;
    if (const char *why = find_sequence(song.image, at))
        return fail(why);
    if (const char *why = read_form(at, song))
        return fail(why);
    if (!song.events)
        return fail("no EVNT chunk");
    if (scan_events(song))
        return true;
    return fail("events run past their chunk without an end of track");
}

// --- The player ----------------------------------------------------------------

namespace
{

// read_line_from_buf's lines of midi.txt.
std::vector<std::string> split_lines(const std::vector<uint8_t> &text)
{
    std::vector<std::string> lines;
    std::string line;
    for (uint8_t c : text) {
        if (c == '\n') {
            lines.push_back(line);
            line.clear();
        } else if (c != '\r') {
            line += char(c);
        }
    }
    if (!line.empty())
        lines.push_back(line);
    return lines;
}

// Sets `*error`, if given, to `why`; returns false.
bool fail(std::string *error, const char *why)
{
    if (error)
        *error = why;
    return false;
}

// The printable characters `s` starts with, at most `max`.
std::string printable(const std::string &s, size_t max)
{
    size_t n = 0;
    while (n < s.size() && n < max && s[n] >= 0x20 && s[n] < 0x7f)
        n++;
    return s.substr(0, n);
}

} // namespace

struct MusicPlayer::Engine {
    Chip chip;
    OplDriver driver{chip};
    Sequencer sequencer{driver};
    // The resampler: the chip's last two samples and the position between
    // them (32.32 fixed point), and the chip samples until the next service.
    int32_t prev[2]{}, cur[2]{};
    uint64_t phase = uint64_t(1) << 32;
    uint32_t serve_accum = 0;

    void next_sample()
    {
        serve_accum += Sequencer::kServiceRate;
        if (serve_accum >= chip.rate()) {
            serve_accum -= chip.rate();
            sequencer.serve();
        }
        prev[0] = cur[0];
        prev[1] = cur[1];
        chip.generate(cur[0], cur[1]);
    }

    template <typename Out> void render(int frames, int rate, Out out)
    {
        const uint64_t step = (uint64_t(chip.rate()) << 32) / uint64_t(rate);
        for (int i = 0; i < frames; i++) {
            while (phase >= uint64_t(1) << 32) {
                phase -= uint64_t(1) << 32;
                next_sample();
            }
            const int64_t frac = int64_t(phase >> 16);
            int32_t l = prev[0] + int32_t((int64_t(cur[0] - prev[0]) * frac) >> 16);
            int32_t r = prev[1] + int32_t((int64_t(cur[1] - prev[1]) * frac) >> 16);
            out(i, l, r);
            phase += step;
        }
    }
};

MusicPlayer::MusicPlayer() : engine_(std::make_unique<Engine>())
{
}
MusicPlayer::~MusicPlayer() = default;

bool MusicPlayer::load(const std::filesystem::path &dir, std::string *error)
{
    songs_.clear();
    engine_ = std::make_unique<Engine>();
    music_playing_ = false;
    midi_playing_ = true;
    // init_midi turns the ten tracks on.
    enabled_.fill(true);
    enabled_.back() = false;
    loop_ = false;
    track_ = 8;
    auto gtl = resolve_ci(dir, "sound/sample.opl");
    if (!gtl)
        return fail(error, "missing sound/sample.opl");
    if (!timbres_.load(*gtl, error))
        return false;
    auto list = resolve_ci(dir, "sound/midi/midi.txt");
    auto text = list ? read_file(*list) : std::nullopt;
    if (!text) {
        // load_midi clears othervar.midi_playing.
        midi_playing_ = false;
        return fail(error, "missing sound/midi/midi.txt");
    }
    // A name keeps its printable characters up to 29, a title up to 25.
    const std::vector<std::string> lines = split_lines(*text);
    for (size_t i = 0; i < lines.size() && songs_.size() < 10; i++) {
        std::string name = printable(lines[i], 29);
        auto path = name.empty() ? std::nullopt : resolve_ci(dir, "sound/midi/" + name);
        if (!path)
            continue;
        Song song;
        song.file = name;
        if (i + 1 < lines.size())
            song.title = printable(lines[++i], 25);
        if (auto bytes = read_file(*path))
            song.ok = parse_xmi(*bytes, song.xmi);
        songs_.push_back(std::move(song));
    }
    if (songs_.empty() && error)
        *error = "no songs in sound/midi/midi.txt";
    return !songs_.empty();
}

bool MusicPlayer::start_song(int track)
{
    // midi_files[count()] is the list's empty slot, which fails to load.
    if (track < 0 || track >= count() || !songs_[size_t(track)].ok)
        return false;
    engine_->sequencer.start(songs_[size_t(track)].xmi, timbres_, volume_);
    return true;
}

// !is_midi_playing.
bool MusicPlayer::song_done() const
{
    return music_on() && midi_playing_ && music_playing_ && engine_->sequencer.status() != Sequencer::Playing;
}

void MusicPlayer::start_level(uint32_t clock)
{
    if (!count())
        return;
    midi_playing_ = true;
    if (music_on()) {
        music_playing_ = true;
        play(int(clock % uint32_t(count())));
    }
}

void MusicPlayer::play_next(bool paused)
{
    midi_playing_ = true;
    if (paused || !song_done())
        return;
    if (loop_) {
        if (enabled_[size_t(track_)]) {
            if (start_song(track_))
                music_playing_ = true;
            else
                midi_playing_ = false;
        }
        return;
    }
    int i = 0;
    for (; i < count(); i++) {
        if (++track_ > count())
            track_ = 0;
        if (enabled_[size_t(track_)] && start_song(track_)) {
            music_playing_ = true;
            break;
        }
    }
    if (i == count())
        midi_playing_ = false;
}

void MusicPlayer::play_previous(bool paused)
{
    midi_playing_ = true;
    if (paused || !song_done())
        return;
    if (loop_) {
        if (enabled_[size_t(track_)]) {
            if (start_song(track_))
                music_playing_ = true;
            else
                midi_playing_ = false;
        }
        return;
    }
    int i = 0;
    for (; i < count(); i++) {
        if (track_ == 0)
            track_ = count();
        else
            track_--;
        if (enabled_[size_t(track_)] && start_song(track_)) {
            music_playing_ = true;
            break;
        }
    }
    if (i == count())
        midi_playing_ = false;
}

void MusicPlayer::play(int track)
{
    engine_->sequencer.end();
    if (!music_on() || !midi_playing_)
        return;
    if (enabled_.at(size_t(track)) && start_song(track)) {
        track_ = track;
        music_playing_ = true;
    } else {
        // play_next_midi, which only acts once the song is over: it is,
        // with the sequence just ended.
        play_next(false);
    }
}

void MusicPlayer::stop()
{
    music_playing_ = false;
    engine_->sequencer.end();
    loop_ = false;
}

void MusicPlayer::end_song()
{
    engine_->sequencer.end();
}

void MusicPlayer::pause_song()
{
    engine_->sequencer.stop();
}

void MusicPlayer::resume_song()
{
    engine_->sequencer.resume();
}

void MusicPlayer::toggle_music()
{
    if (!music_ok()) {
        music_on_ = false;
        return;
    }
    music_on_ = !music_on_;
    if (music_on_)
        play(track_);
    else
        stop();
}

void MusicPlayer::set_volume_item(int item)
{
    volume_ = (10 - std::clamp(item, 0, 9)) * kMaxVolume / 10;
    // FadeMusic, while a song plays.
    if (engine_->sequencer.status() != Sequencer::Done)
        engine_->sequencer.set_volume(volume_, 1000);
}

void MusicPlayer::render(float *out, int frames, int rate)
{
    if (rate <= 0)
        return;
    engine_->render(frames, rate, [out](int i, int32_t l, int32_t r) {
        out[2 * i] += float(l) / 32768.0f;
        out[2 * i + 1] += float(r) / 32768.0f;
    });
}

void MusicPlayer::render(int16_t *out, int frames, int rate)
{
    if (rate <= 0)
        return;
    engine_->render(frames, rate, [out](int i, int32_t l, int32_t r) {
        out[2 * i] = int16_t(std::clamp(l, -32768, 32767));
        out[2 * i + 1] = int16_t(std::clamp(r, -32768, 32767));
    });
}

} // namespace th

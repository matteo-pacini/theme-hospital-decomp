// The OPL3 driver (SOUND/OPL3.MDI, Miles' YAMAHA driver): voices, timbres
// and the registers of the emulated chip.

#include "core/audio/music_internal.h"

#include <algorithm>

namespace th
{

namespace
{

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

constexpr int kDrumChannel = 9;

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

} // namespace

inline namespace music_detail
{

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

} // namespace music_detail

} // namespace th

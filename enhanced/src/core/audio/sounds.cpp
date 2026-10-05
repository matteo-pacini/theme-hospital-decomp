#include "core/audio/sounds.h"

#include "core/data/fs.h"

#include <algorithm>
#include <bit>
#include <cstdlib>
#include <cstring>

namespace th
{

namespace
{

// SfxDetail.type (SFXT_*): how play_sound picks the sample.
enum : uint8_t {
    kSfxNone = 0,
    kSfxSample = 1,
    kSfxStop = 2,         // stops the sample instead
    kSfxRandomPitch = 3,  // the sample at a random pitch of kPitchGroups[group]
    kSfxRandomSample = 4, // a random sample of kSampleGroups[group]
    kSfxPitchRange = 5,   // the sample at a pitch in the range of kPitchGroups[group]
};

// The fields of the game's SfxDetail (include/hosp.h) that play_sound and
// the frame drawing read.
struct SfxDetail {
    int8_t loop;       // play_sample_using_heap's loop
    uint8_t type;      // kSfx*
    uint8_t sample;    // types 1, 2, 3 and 5
    uint8_t group;     // kPitchGroups or kSampleGroups index for types 3-5
    uint8_t play_type; // play_sample's type
    // A frame's sound plays every time the frame is drawn, not only when it
    // differs from the thing's last. The game's table sets it nowhere.
    uint16_t allow_repeat = 0;
};

// sfx_details (src/sfxtable.cpp), indexed by sound effect number. Every
// type 1 entry plays the sample of its own number.
// clang-format off
constexpr SfxDetail kSfxDetails[kSfxCount] = {
    {0, 1, 0, 0, 1}, {0, 1, 1, 0, 3}, {0, 1, 2, 0, 1}, {0, 1, 3, 0, 1},
    {0, 1, 4, 0, 1}, {0, 1, 5, 0, 1}, {0, 1, 6, 0, 1}, {0, 1, 7, 0, 1},
    {0, 1, 8, 0, 3}, {0, 1, 9, 0, 2}, {0, 1, 10, 0, 1}, {0, 1, 11, 0, 1},
    {0, 1, 12, 0, 3}, {0, 1, 13, 0, 1}, {0, 1, 14, 0, 1}, {0, 1, 15, 0, 2},
    {0, 1, 16, 0, 3}, {0, 1, 17, 0, 1}, {0, 1, 18, 0, 1}, {0, 1, 19, 0, 1},
    {0, 1, 20, 0, 1}, {0, 1, 21, 0, 1}, {0, 1, 22, 0, 1}, {0, 1, 23, 0, 1},
    {0, 1, 24, 0, 3}, {0, 1, 25, 0, 2}, {0, 1, 26, 0, 1}, {0, 1, 27, 0, 1},
    {0, 1, 28, 0, 1}, {0, 1, 29, 0, 1}, {0, 1, 30, 0, 3}, {0, 1, 31, 0, 1},
    {0, 1, 32, 0, 1}, {0, 1, 33, 0, 1}, {0, 1, 34, 0, 3}, {0, 1, 35, 0, 2},
    {0, 1, 36, 0, 1}, {0, 1, 37, 0, 1}, {0, 1, 38, 0, 1}, {0, 1, 39, 0, 1},
    {0, 1, 40, 0, 2}, {0, 1, 41, 0, 2}, {0, 1, 42, 0, 1}, {0, 1, 43, 0, 1},
    {0, 1, 44, 0, 1}, {0, 1, 45, 0, 1}, {0, 1, 46, 0, 1}, {0, 1, 47, 0, 1},
    {0, 1, 48, 0, 1}, {0, 1, 49, 0, 1}, {0, 1, 50, 0, 2}, {0, 1, 51, 0, 1},
    {0, 1, 52, 0, 1}, {0, 1, 53, 0, 1}, {0, 1, 54, 0, 1}, {0, 1, 55, 0, 1},
    {0, 1, 56, 0, 1}, {0, 1, 57, 0, 1}, {0, 1, 58, 0, 1}, {0, 1, 59, 0, 1},
    {0, 1, 60, 0, 1}, {0, 1, 61, 0, 1}, {0, 1, 62, 0, 1}, {0, 1, 63, 0, 3},
    {0, 1, 64, 0, 3}, {0, 1, 65, 0, 3}, {0, 1, 66, 0, 3}, {0, 1, 67, 0, 3},
    {0, 1, 68, 0, 3}, {0, 1, 69, 0, 3}, {0, 1, 70, 0, 3}, {0, 1, 71, 0, 3},
    {-1, 1, 72, 0, 2}, {0, 1, 73, 0, 2}, {0, 1, 74, 0, 1}, {0, 1, 75, 0, 1},
    {0, 1, 76, 0, 1}, {0, 1, 77, 0, 1}, {0, 1, 78, 0, 1}, {0, 1, 79, 0, 1},
    {-1, 1, 80, 0, 2}, {0, 1, 81, 0, 1}, {0, 1, 82, 0, 1}, {0, 1, 83, 0, 1},
    {0, 1, 84, 0, 1}, {0, 1, 85, 0, 1}, {0, 1, 86, 0, 1}, {0, 1, 87, 0, 1},
    {-1, 1, 88, 0, 1}, {0, 1, 89, 0, 1}, {0, 1, 90, 0, 1}, {0, 1, 91, 0, 1},
    {0, 1, 92, 0, 1}, {0, 1, 93, 0, 1}, {-1, 1, 94, 0, 1}, {0, 1, 95, 0, 1},
    {0, 1, 96, 0, 1}, {0, 1, 97, 0, 1}, {0, 1, 98, 0, 1}, {0, 1, 99, 0, 1},
    {0, 1, 100, 0, 3}, {0, 1, 101, 0, 3}, {0, 1, 102, 0, 1}, {0, 1, 103, 0, 1},
    {0, 1, 104, 0, 1}, {0, 1, 105, 0, 1}, {0, 1, 106, 0, 1}, {0, 1, 107, 0, 1},
    {0, 1, 108, 0, 1}, {0, 1, 109, 0, 1}, {0, 1, 110, 0, 1}, {0, 1, 111, 0, 1},
    {0, 1, 112, 0, 1}, {0, 1, 113, 0, 1}, {0, 1, 114, 0, 1}, {0, 1, 115, 0, 1},
    {0, 1, 116, 0, 1}, {-1, 1, 117, 0, 1}, {0, 1, 118, 0, 1}, {0, 1, 119, 0, 1},
    {0, 1, 120, 0, 1}, {0, 1, 121, 0, 1}, {0, 1, 122, 0, 1}, {0, 1, 123, 0, 1},
    {0, 1, 124, 0, 1}, {0, 1, 125, 0, 1}, {0, 1, 126, 0, 1}, {0, 1, 127, 0, 1},
    {0, 1, 128, 0, 1}, {0, 1, 129, 0, 1}, {0, 1, 130, 0, 1}, {0, 1, 131, 0, 1},
    {0, 1, 132, 0, 1}, {0, 1, 133, 0, 1}, {0, 1, 134, 0, 1}, {0, 1, 135, 0, 1},
    {0, 1, 136, 0, 1}, {0, 1, 137, 0, 1}, {0, 1, 138, 0, 1}, {0, 1, 139, 0, 1},
    {0, 1, 140, 0, 1}, {0, 1, 141, 0, 1}, {0, 1, 142, 0, 1}, {0, 1, 143, 0, 1},
    {0, 1, 144, 0, 1}, {0, 1, 145, 0, 1}, {0, 1, 146, 0, 1}, {0, 1, 147, 0, 1},
    {0, 1, 148, 0, 1}, {0, 1, 149, 0, 1}, {0, 1, 150, 0, 1}, {0, 1, 151, 0, 1},
    {0, 1, 152, 0, 1}, {0, 1, 153, 0, 1}, {0, 1, 154, 0, 1}, {0, 1, 155, 0, 1},
    {0, 1, 156, 0, 1}, {0, 1, 157, 0, 1}, {0, 1, 158, 0, 1}, {0, 1, 159, 0, 1},
    {0, 1, 160, 0, 1}, {0, 1, 161, 0, 1}, {0, 1, 162, 0, 1}, {0, 1, 163, 0, 1},
    {0, 1, 164, 0, 1}, {0, 1, 165, 0, 1}, {0, 1, 166, 0, 1}, {0, 1, 167, 0, 1},
    {0, 1, 168, 0, 1}, {-1, 1, 169, 0, 2}, {0, 1, 170, 0, 1}, {0, 1, 171, 0, 1},
    {0, 1, 172, 0, 1}, {0, 1, 173, 0, 1}, {0, 1, 174, 0, 1}, {0, 1, 175, 0, 1},
    {0, 4, 176, 6, 1}, {0, 1, 177, 0, 1}, {0, 1, 178, 0, 1}, {0, 1, 179, 0, 1},
    {0, 1, 180, 0, 1}, {0, 1, 181, 0, 1}, {0, 1, 182, 0, 1}, {0, 1, 183, 0, 1},
    {0, 1, 184, 0, 1}, {0, 1, 185, 0, 1}, {0, 1, 186, 0, 1}, {0, 1, 187, 0, 1},
    {0, 1, 188, 0, 1}, {0, 1, 189, 0, 1}, {0, 1, 190, 0, 1}, {0, 1, 191, 0, 1},
    {0, 1, 192, 0, 1}, {0, 1, 193, 0, 1}, {0, 1, 194, 0, 1}, {0, 1, 195, 0, 1},
    {0, 1, 196, 0, 1}, {0, 1, 197, 0, 1}, {0, 1, 198, 0, 1}, {0, 1, 199, 0, 1},
    {-1, 1, 200, 0, 2}, {0, 1, 201, 0, 1}, {0, 1, 202, 0, 1}, {0, 1, 203, 0, 1},
    {0, 1, 204, 0, 1}, {0, 1, 205, 0, 1}, {0, 1, 206, 0, 1}, {0, 1, 207, 0, 1},
    {0, 1, 208, 0, 1}, {0, 1, 209, 0, 1}, {0, 1, 210, 0, 1}, {0, 1, 211, 0, 1},
    {0, 1, 212, 0, 1}, {0, 1, 213, 0, 1}, {0, 1, 214, 0, 1}, {0, 1, 215, 0, 1},
    {-1, 1, 216, 0, 3}, {0, 1, 217, 0, 1}, {0, 1, 218, 0, 1}, {-1, 1, 219, 0, 1},
    {0, 1, 220, 0, 1}, {0, 1, 221, 0, 1}, {0, 1, 222, 0, 1}, {0, 1, 223, 0, 1},
    {0, 1, 224, 0, 1}, {0, 1, 225, 0, 1}, {0, 1, 226, 0, 1}, {0, 1, 227, 0, 2},
    {0, 1, 228, 0, 1}, {0, 1, 229, 0, 1}, {0, 1, 230, 0, 1}, {0, 1, 231, 0, 1},
    {0, 2, 216, 0, 1}, {0, 4, 0, 5, 1}, {0, 5, 35, 4, 1}, {0, 4, 0, 4, 1},
    {0, 2, 88, 0, 1}, {0, 2, 94, 0, 1}, {0, 5, 9, 3, 1}, {0, 4, 0, 3, 1},
    {0, 4, 0, 2, 1}, {0, 0, 0, 0, 0},
};
// clang-format on

struct PitchGroup {
    uint8_t count;
    // Pitches (percent) to pick from; a pitch range uses [0]..[1] instead.
    std::array<uint8_t, 5> values;
};

// pitch_group (src/sfxtable.cpp).
constexpr PitchGroup kPitchGroups[5] = {
    {3, {}}, {3, {100, 200, 50}}, {2, {100, 200}}, {2, {90, 110}}, {2, {95, 105}},
};

struct SampleGroup {
    uint8_t count;
    std::array<uint8_t, 5> samples;
};

// sample_group (src/sfxtable.cpp).
constexpr SampleGroup kSampleGroups[7] = {
    {0, {}},
    {3, {24, 17, 18}},
    {2, {58, 114}},
    {5, {118, 119, 120, 121, 122}},
    {5, {85, 86, 138, 139, 140}},
    {5, {99, 100, 181, 182, 183}},
    {4, {176, 177, 178, 179}},
};

uint32_t le32(const uint8_t *p)
{
    return uint32_t(p[0]) | uint32_t(p[1]) << 8 | uint32_t(p[2]) << 16 | uint32_t(p[3]) << 24;
}
uint16_t le16(const uint8_t *p)
{
    return uint16_t(p[0] | p[1] << 8);
}

// The format and PCM data of the RIFF WAVE file `file`, if it is one with
// 8 or 16-bit PCM.
std::optional<Sample> parse_wave(std::span<const uint8_t> file)
{
    if (file.size() < 12 || std::memcmp(file.data(), "RIFF", 4) != 0 || std::memcmp(file.data() + 8, "WAVE", 4) != 0)
        return std::nullopt;
    Sample s;
    bool have_format = false, have_data = false;
    for (size_t pos = 12; pos + 8 <= file.size();) {
        const uint8_t *chunk = file.data() + pos;
        size_t size = le32(chunk + 4);
        size_t body = pos + 8;
        if (size > file.size() - body)
            size = file.size() - body;
        if (std::memcmp(chunk, "fmt ", 4) == 0 && size >= 16) {
            if (le16(chunk + 8) != 1) // WAVE_FORMAT_PCM
                return std::nullopt;
            s.channels = le16(chunk + 10);
            s.rate = int(le32(chunk + 12));
            s.bits = le16(chunk + 22);
            have_format = true;
        } else if (std::memcmp(chunk, "data", 4) == 0) {
            s.data = file.subspan(body, size);
            have_data = true;
        }
        pos = body + size + (size & 1);
    }
    if (!have_format || !have_data || s.channels < 1 || s.channels > 2 || s.rate <= 0 || (s.bits != 8 && s.bits != 16))
        return std::nullopt;
    s.data = s.data.first(s.data.size() - s.data.size() % size_t(s.channels * s.bits / 8));
    return s;
}

// The value of channel `ch` of frame `i` of `s`, -1 to 1.
float sample_value(const Sample &s, size_t i, int ch)
{
    size_t at = (i * size_t(s.channels) + size_t(ch)) * size_t(s.bits / 8);
    if (s.bits == 8)
        return (float(s.data[at]) - 128.0f) / 128.0f;
    return float(int16_t(le16(s.data.data() + at))) / 32768.0f;
}

// The volume of a sound at screen x (play_sound, manage_sound): full in the
// middle falling to nothing at the sides; and its pan, 0 left to 127 right.
void place(int x, int screen_width, int sfx_volume, int &volume, int &pan)
{
    int half = screen_width / 2;
    volume = half > 0 ? (half - std::abs(x - half)) * sfx_volume / half : sfx_volume;
    volume = std::clamp(volume, 0, sfx_volume);
    pan = screen_width > 0 ? std::clamp(x * 0x7f / screen_width, 0, 0x7f) : 0x40;
}

// The directory entry setup_sound asks for (SoundType 0x656, 16-bit
// 22 kHz): the third of nine.
constexpr int kDirectoryEntry = 2;

} // namespace

bool SoundBank::load(const std::filesystem::path &dir, int language, std::string *error)
{
    std::string rel = "sound/data/sound-" + std::to_string(language) + ".dat";
    auto path = resolve_ci(dir, rel);
    if (!path) {
        if (error)
            *error = "missing " + rel;
        return false;
    }
    return load_file(*path, error);
}

bool SoundBank::load_file(const std::filesystem::path &path, std::string *error)
{
    auto fail = [&](const char *why) {
        if (error)
            *error = path.string() + ": " + why;
        file_.clear();
        samples_.clear();
        return false;
    };
    auto data = read_file(path);
    if (!data)
        return fail("cannot read");
    file_ = std::move(*data);
    samples_.clear();
    // parse_sound_file: the last 4 bytes give the directory's offset; after
    // an 0x12-byte header, nine 16-byte entries (records' offset, base added
    // to their data offsets, records' size).
    const size_t size = file_.size();
    if (size < 4)
        return fail("too short");
    size_t table = le32(file_.data() + size - 4);
    if (table > size || size - table < 0x12 + 9 * 16)
        return fail("bad directory");
    const uint8_t *entry = file_.data() + table + 0x12 + kDirectoryEntry * 16;
    size_t records = le32(entry), base = le32(entry + 4), records_size = le32(entry + 8);
    if (records == 0 || records_size == 0 || records > size || records_size > size - records)
        return fail("no 16-bit 22 kHz samples");
    size_t count = records_size >> 5;
    samples_.resize(count);
    std::span<const uint8_t> all(file_);
    for (size_t i = 1; i < count; i++) {
        const uint8_t *rec = file_.data() + records + i * 32;
        size_t offset = le32(rec + 0x12) + base, length = le32(rec + 0x1a);
        if (offset > size || length > size - offset)
            continue;
        std::optional<Sample> s = parse_wave(all.subspan(offset, length));
        if (!s)
            continue;
        s->name.assign(reinterpret_cast<const char *>(rec), strnlen(reinterpret_cast<const char *>(rec), 0x12));
        s->priority = rec[0x1e];
        samples_[i] = std::move(s);
    }
    return true;
}

const Sample *SoundBank::sample(int number) const
{
    if (number <= 0 || number >= count() || !samples_[size_t(number)])
        return nullptr;
    return &*samples_[size_t(number)];
}

void SoundPlayer::set_bank(const SoundBank *bank)
{
    stop_all();
    bank_ = bank;
}

void SoundPlayer::toggle_sound()
{
    sfx_on_ = sound_ok() ? !sfx_on_ : false;
    if (!sfx_on_)
        stop_all();
}

void SoundPlayer::set_volume_item(int item)
{
    sfx_volume_ = (10 - std::clamp(item, 0, 9)) * kMaxVolume / 10;
}

uint16_t SoundPlayer::saferand()
{
    safeseed = std::rotr(safeseed * 0x24a1u + 0x24dfu, 13);
    return uint16_t(safeseed);
}

void SoundPlayer::play_sound(uint32_t source, int sfx, int x, int y, int screen_width)
{
    if (!sfx_on() || sfx < 0 || sfx >= kSfxCount)
        return;
    const SfxDetail &d = kSfxDetails[sfx];
    int pitch = 100;
    int sample = d.sample;
    switch (d.type) {
    case kSfxSample:
        break;
    case kSfxStop:
        stop_sample(source, d.sample);
        return;
    case kSfxRandomPitch: {
        const PitchGroup &g = kPitchGroups[d.group];
        pitch = g.values[saferand() % g.count];
        break;
    }
    case kSfxRandomSample: {
        const SampleGroup &g = kSampleGroups[d.group];
        sample = g.samples[saferand() % g.count];
        break;
    }
    case kSfxPitchRange: {
        const PitchGroup &g = kPitchGroups[d.group];
        pitch = g.values[0] + saferand() % (g.values[1] - g.values[0]);
        break;
    }
    default:
        // SFXT_NONE: the game plays with its sample and pitch unset.
        return;
    }
    int volume = sfx_volume_, pan = 0x40;
    if (x != -1 && y != -1)
        place(x, screen_width, sfx_volume_, volume, pan);
    play_sample(source, sample, volume, pan, pitch, d.loop, d.play_type);
}

bool SoundPlayer::play_sample(uint32_t source, int number, int volume, int pan, int pitch, int loop, int type)
{
    if (!bank_ || number <= 0 || number >= bank_->count())
        return false;
    const Sample *sample = bank_->sample(number);
    if (!sample)
        return false;
    auto free_voice = [&]() -> Voice * {
        for (Voice &v : voices_)
            if (!v.sample)
                return &v;
        return nullptr;
    };
    Voice *voice = nullptr;
    switch (type) {
    case 1:
        voice = free_voice();
        break;
    case 2:
        if (playing(source, number))
            return false;
        voice = free_voice();
        break;
    case 3:
        for (Voice &v : voices_) {
            if (v.source == source && v.number == number) {
                voice = &v;
                break;
            }
        }
        if (!voice && sample->priority) {
            for (Voice &v : voices_) {
                if (v.source == source && v.priority == sample->priority) {
                    voice = &v;
                    break;
                }
            }
        }
        if (!voice)
            voice = free_voice();
        break;
    default:
        break;
    }
    if (!voice)
        return false;
    *voice = Voice{sample, source, number, volume, pan, pitch, sample->priority, loop + 1, 0};
    return true;
}

SoundPlayer::Voice *SoundPlayer::find(uint32_t source, int number)
{
    for (Voice &v : voices_)
        if (v.sample && v.source == source && v.number == number)
            return &v;
    return nullptr;
}

void SoundPlayer::stop_sample(uint32_t source, int number)
{
    if (Voice *v = find(source, number))
        v->sample = nullptr;
}

void SoundPlayer::stop_all()
{
    for (Voice &v : voices_)
        v = Voice{};
}

bool SoundPlayer::playing(uint32_t source, int number) const
{
    for (const Voice &v : voices_)
        if (v.sample && v.source == source && (number == 0 || v.number == number))
            return true;
    return false;
}

void SoundPlayer::set_sample_volume(uint32_t source, int number, int volume)
{
    if (volume < 0 || volume > kMaxVolume)
        return;
    for (Voice &v : voices_) {
        if (v.sample && v.source == source && v.number == number && v.volume != volume) {
            v.volume = volume;
            return;
        }
    }
}

void SoundPlayer::frames_drawn(std::span<const FrameSound> frames, int screen_width)
{
    visible_people_ = 0;
    for (const FrameSound &f : frames) {
        if (f.person)
            visible_people_++;
        if (f.thing >= kThingCount)
            continue;
        uint8_t &last = last_sound_[f.thing];
        bool repeat = f.sound < kSfxCount && kSfxDetails[f.sound].allow_repeat;
        if (f.sound && (repeat || last != f.sound))
            play_sound(thing_source(f.thing), f.sound, f.x, f.y, screen_width);
        last = f.sound;
    }
}

void SoundPlayer::manage_sound(const World &world, Point camera, int screen_width, int screen_height)
{
    for (Voice &v : voices_) {
        if (!v.sample || v.source <= 0xe)
            continue;
        // The game's test is on the Thing's address: inside the table, not
        // the first entry.
        uint32_t id = v.source - kSourceThings;
        if (v.source < kSourceThings || id == 0 || id >= kThingCount) {
            v.sample = nullptr;
            continue;
        }
        const Thing &t = world.things[id];
        // give_screen_xy_of_cell_at_column_row, plus half a tile.
        int col = t.x.tile, row = t.y.tile;
        int x = kTileWidth * (col - row - 1) / 2 - camera.x + kTileWidth / 2;
        int y = (col + row + 2) * kTileHeight / 2 - camera.y + kTileHeight / 2;
        if (x > 0 && x < screen_width && y > 0 && y < screen_height)
            place(x, screen_width, sfx_volume_, v.volume, v.pan);
        else
            v.sample = nullptr;
    }
}

void SoundPlayer::manage_background_sounds(bool paused, std::optional<int> emergency_seconds, bool quake)
{
    if (!sfx_on() || paused)
        return;
    int volume = std::clamp(visible_people_ * sfx_volume_ / 20, 0, sfx_volume_);
    play_sample(kSourceCrowd, 200, volume, 0x40, 100, -1, 2);
    if (crowd_volume_ != volume) {
        set_sample_volume(kSourceCrowd, 200, volume);
        crowd_volume_ = volume;
    }
    if (++ambient_noise_timer_ > 400) {
        play_sample(kSourceGeneral, saferand() % 30 + 0x10e, volume, 0x40, 100, 0, 2);
        ambient_noise_timer_ = 0;
    }
    if (emergency_seconds) {
        volume = sfx_volume_ / 2;
        if (*emergency_seconds < 0x21)
            volume += (0x21 - *emergency_seconds) * sfx_volume_ / 0x42;
        volume = std::clamp(volume, 0, sfx_volume_);
        play_sample(kSourceClock, 0xa9, volume, 0x40, 100, -1, 2);
        if (clock_volume_ != volume) {
            set_sample_volume(kSourceClock, 0xa9, volume);
            clock_volume_ = volume;
        }
    }
    if (quake)
        play_sample(kSourceCrowd, 0x48, sfx_volume_, 0x40, 100, -1, 2);
}

void SoundPlayer::mix(float *out, int frames, int rate)
{
    std::fill(out, out + size_t(frames) * 2, 0.0f);
    if (rate <= 0)
        return;
    for (Voice &v : voices_) {
        if (!v.sample)
            continue;
        const Sample &s = *v.sample;
        const size_t length = s.frames();
        if (length == 0) {
            v.sample = nullptr;
            continue;
        }
        const uint64_t end = uint64_t(length) << 32;
        const uint64_t step = (uint64_t(s.rate) * uint64_t(v.pitch) << 32) / 100 / uint64_t(rate);
        // Assumed pan law: the centre keeps both sides at full volume, a
        // side lowers the other linearly.
        const float gain = float(v.volume) / kMaxVolume;
        const float left = gain * std::min(1.0f, float(0x7f - v.pan) / 63.0f);
        const float right = gain * std::min(1.0f, float(v.pan) / 64.0f);
        for (int i = 0; i < frames && v.sample; i++) {
            size_t at = size_t(v.pos >> 32);
            float frac = float(v.pos & 0xffffffffu) / 4294967296.0f;
            size_t next = at + 1 < length ? at + 1 : (v.loops != 1 ? 0 : at);
            float l = sample_value(s, at, 0), r = s.channels > 1 ? sample_value(s, at, 1) : l;
            float nl = sample_value(s, next, 0), nr = s.channels > 1 ? sample_value(s, next, 1) : nl;
            out[2 * i] += (l + (nl - l) * frac) * left;
            out[2 * i + 1] += (r + (nr - r) * frac) * right;
            v.pos += step;
            if (v.pos >= end) {
                v.pos -= end;
                if (v.loops == 0)
                    continue;
                if (--v.loops == 0)
                    v.sample = nullptr;
            }
        }
    }
    for (int i = 0; i < frames * 2; i++)
        out[i] = std::clamp(out[i], -1.0f, 1.0f);
}

} // namespace th

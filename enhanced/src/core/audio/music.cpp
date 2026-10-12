#include "core/audio/music.h"

#include "core/audio/music_internal.h"
#include "core/data/fs.h"

#include <algorithm>

namespace th
{

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

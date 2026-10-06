#include "common.h"

#include "core/audio/music.h"
#include "core/audio/sounds.h"
#include "core/data/fs.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <optional>
#include <vector>

namespace
{

// A sound bank file holding one sample, `pcm` (16-bit mono at `rate`), as
// sample 1 with priority tag `priority`.
std::vector<uint8_t> make_sound_bank(const std::vector<int16_t> &pcm, int rate, uint8_t priority)
{
    auto put32 = [](std::vector<uint8_t> &v, uint32_t x) {
        for (int i = 0; i < 4; i++)
            v.push_back(uint8_t(x >> (8 * i)));
    };
    auto put16 = [](std::vector<uint8_t> &v, uint16_t x) {
        v.push_back(uint8_t(x));
        v.push_back(uint8_t(x >> 8));
    };
    std::vector<uint8_t> wav;
    uint32_t data_size = uint32_t(pcm.size() * 2);
    wav.insert(wav.end(), {'R', 'I', 'F', 'F'});
    put32(wav, 36 + data_size);
    wav.insert(wav.end(), {'W', 'A', 'V', 'E', 'f', 'm', 't', ' '});
    put32(wav, 16);
    put16(wav, 1);
    put16(wav, 1);
    put32(wav, uint32_t(rate));
    put32(wav, uint32_t(rate * 2));
    put16(wav, 2);
    put16(wav, 16);
    wav.insert(wav.end(), {'d', 'a', 't', 'a'});
    put32(wav, data_size);
    for (int16_t s : pcm)
        put16(wav, uint16_t(s));

    std::vector<uint8_t> file = wav;
    uint32_t records = uint32_t(file.size());
    file.resize(file.size() + 64); // record 0 unused, record 1 the sample
    uint8_t *rec = file.data() + records + 32;
    std::memcpy(rec, "TEST.WAV", 8);
    for (int i = 0; i < 4; i++)
        rec[0x1a + i] = uint8_t(wav.size() >> (8 * i));
    rec[0x1e] = priority;
    uint32_t table = uint32_t(file.size());
    file.resize(file.size() + 0x12);
    for (int e = 0; e < 9; e++) {
        bool ours = e == 2;
        put32(file, ours ? records : 0xffffffff);
        put32(file, ours ? 0 : 0xffffffff);
        put32(file, ours ? 64 : 0xffffffff);
        put32(file, ours ? records : 0xffffffff);
    }
    put32(file, table);
    return file;
}

} // namespace

void test_sound_player()
{
    auto path = std::filesystem::temp_directory_path() / "th_enhanced_test_bank.dat";
    {
        std::vector<uint8_t> bytes = make_sound_bank({16384, 16384, 16384, 16384}, 100, 0);
        std::FILE *f = std::fopen(path.string().c_str(), "wb");
        CHECK(f != nullptr);
        if (!f)
            return;
        std::fwrite(bytes.data(), 1, bytes.size(), f);
        std::fclose(f);
    }
    th::SoundBank bank;
    std::string error;
    CHECK(bank.load_file(path, &error));
    std::filesystem::remove(path);
    CHECK(bank.count() == 2);
    CHECK(!bank.sample(0));
    const th::Sample *s = bank.sample(1);
    CHECK(s && s->name == "TEST.WAV" && s->frames() == 4 && s->rate == 100 && s->bits == 16);

    th::SoundPlayer player;
    player.set_bank(&bank);
    std::vector<float> out(16);
    // Centred at full volume: both sides, once, then the voice is free.
    CHECK(player.play_sample(th::kSourceGeneral, 1, 127, 64, 100, 0, 1));
    CHECK(player.playing(th::kSourceGeneral, 1));
    player.mix(out.data(), 8, 100);
    CHECK(out[0] == 0.5f && out[1] == 0.5f && out[6] == 0.5f && out[7] == 0.5f);
    CHECK(out[8] == 0.0f && out[9] == 0.0f);
    CHECK(!player.playing(th::kSourceGeneral, 0));
    // Panned hard left at half volume.
    CHECK(player.play_sample(th::kSourceGeneral, 1, 0x3f, 0, 100, 0, 1));
    player.mix(out.data(), 1, 100);
    CHECK(out[0] > 0.24f && out[0] < 0.25f && out[1] == 0.0f);
    player.stop_all();
    // Type 2 refuses a sample the source is playing; type 3 reuses its voice.
    CHECK(player.play_sample(th::kSourceGeneral, 1, 127, 64, 100, -1, 2));
    CHECK(!player.play_sample(th::kSourceGeneral, 1, 127, 64, 100, -1, 2));
    CHECK(player.play_sample(th::kSourceClock, 1, 127, 64, 100, -1, 2));
    CHECK(player.play_sample(th::kSourceGeneral, 1, 127, 64, 100, -1, 3));
    player.mix(out.data(), 8, 100); // two voices looping forever
    CHECK(out[14] == 1.0f && player.playing(th::kSourceClock, 1));
    player.stop_sample(th::kSourceClock, 1);
    CHECK(!player.playing(th::kSourceClock, 1) && player.playing(th::kSourceGeneral, 1));
    // Ten voices at most.
    player.stop_all();
    for (int i = 0; i < th::SoundPlayer::kVoices; i++)
        CHECK(player.play_sample(th::kSourceGeneral, 1, 127, 64, 100, 0, 1));
    CHECK(!player.play_sample(th::kSourceGeneral, 1, 127, 64, 100, 0, 1));
    // Half the output rate at pitch 200 plays a frame per output frame.
    player.stop_all();
    CHECK(player.play_sample(th::kSourceGeneral, 1, 127, 64, 200, 0, 1));
    player.mix(out.data(), 8, 200);
    CHECK(out[6] == 0.5f && out[8] == 0.0f);

    // play_sound: SFX_YESX plays sample 0x41, absent here; off, nothing plays.
    player.play_sound(th::kSourceGeneral, th::kSfxYesX);
    CHECK(!player.playing(th::kSourceGeneral, 0));
    player.toggle_sound();
    CHECK(!player.sfx_on());
    player.toggle_sound();
    CHECK(player.sfx_on());
    player.set_volume_item(9);
    CHECK(player.sfx_volume() == 12 && player.volume_item() == 9);
    player.set_volume_item(0);
    CHECK(player.sfx_volume() == 127 && player.volume_item() == 0);
}

void test_sound_bank(const char *dir)
{
    th::SoundBank bank;
    std::string error;
    CHECK(bank.load(dir, 0, &error));
    if (!error.empty())
        std::fprintf(stderr, "%s\n", error.c_str());
    CHECK(bank.count() == 747);
    int usable = 0;
    for (int i = 1; i < bank.count(); i++) {
        const th::Sample *s = bank.sample(i);
        // init_sound_heap refuses a bank with a sample of 0x4b000 bytes or more.
        if (s && s->rate == 22050 && s->frames() > 0 && s->data.size() < 0x4b000)
            usable++;
    }
    CHECK(usable == bank.count() - 1);
    const th::Sample *hlight = bank.sample(th::kSfxHighlightP2);
    CHECK(hlight && hlight->name == "HLIGHTP2.WAV" && hlight->frames() == 0x88c0 / 2);
    const th::Sample *yes = bank.sample(th::kSfxYesX);
    CHECK(yes && yes->name == "YESX.WAV");
    for (int language = 1; language < 6; language++) {
        th::SoundBank other;
        CHECK(other.load(dir, language) && other.count() > 500);
    }

    // Menu sounds play centred; a frame sound plays once until the frame
    // changes, placed by its x.
    th::SoundPlayer player;
    player.set_bank(&bank);
    player.play_sound(th::kSourceGeneral, th::kSfxSelectX);
    CHECK(player.playing(th::kSourceGeneral, th::kSfxSelectX));
    th::FrameSound frame{5, th::kSfxYesX, true, 0, 100};
    player.frames_drawn({&frame, 1}, 640);
    CHECK(player.playing(th::thing_source(5), th::kSfxYesX));
    std::vector<float> out(2 * 64);
    player.mix(out.data(), 64, 44100);
    player.stop_all();
    player.frames_drawn({&frame, 1}, 640);
    CHECK(!player.playing(th::thing_source(5), 0));
    frame.sound = 0;
    player.frames_drawn({&frame, 1}, 640);
    frame.sound = th::kSfxYesX;
    frame.x = 320;
    player.frames_drawn({&frame, 1}, 640);
    CHECK(player.playing(th::thing_source(5), th::kSfxYesX));
}

// A minimal XMIDI file: FORM XDIR, then CAT XMID holding one FORM XMID with
// TIMB and EVNT chunks; cut short, it no longer parses.
void test_xmi_parse()
{
    auto be = [](std::vector<uint8_t> &v, uint32_t n) {
        for (int s = 24; s >= 0; s -= 8)
            v.push_back(uint8_t(n >> s));
    };
    std::vector<uint8_t> events = {0x90, 60, 100, 0x10, 0x20, 0xc0, 5, 0x7f, 0x01, 0xff, 0x2f, 0x00};
    std::vector<uint8_t> f;
    auto tag = [&](const char *t) { f.insert(f.end(), t, t + 4); };
    tag("FORM"), be(f, 14), tag("XDIR"), tag("INFO"), be(f, 2), f.push_back(1), f.push_back(0);
    uint32_t form = 4 + 8 + 4 + 8 + uint32_t(events.size());
    tag("CAT "), be(f, 4 + 8 + form), tag("XMID");
    tag("FORM"), be(f, form), tag("XMID");
    tag("TIMB"), be(f, 4), f.insert(f.end(), {1, 0, 36, 127});
    tag("EVNT"), be(f, uint32_t(events.size())), f.insert(f.end(), events.begin(), events.end());
    th::XmiSong song;
    std::string error;
    CHECK(th::parse_xmi(f, song, &error));
    CHECK(song.timbres.size() == 1 && song.timbres[0][0] == 127 && song.timbres[0][1] == 36);
    CHECK(song.events_size == events.size() && song.ticks == 0x20 + 0x7f + 0x01);
    f.resize(f.size() - 3);
    CHECK(!th::parse_xmi(f, song, &error));
}

namespace
{

// FNV-1a of 16-bit samples.
uint64_t hash_pcm(const std::vector<int16_t> &pcm)
{
    uint64_t h = 1469598103934665603ull;
    for (int16_t v : pcm) {
        h = (h ^ uint8_t(v)) * 1099511628211ull;
        h = (h ^ uint8_t(uint16_t(v) >> 8)) * 1099511628211ull;
    }
    return h;
}

} // namespace

void test_music(const char *dir)
{
    th::TimbreBank gtl;
    std::string error;
    auto opl = th::resolve_ci(dir, "sound/sample.opl");
    CHECK(opl && gtl.load(*opl, &error));
    CHECK(gtl.count() == 190);
    CHECK(gtl.find(0, 0).size() == 14 && gtl.find(127, 36).size() == 14 && gtl.find(1, 0).empty());

    th::MusicPlayer music;
    CHECK(music.load(dir, &error));
    if (!error.empty())
        std::fprintf(stderr, "%s\n", error.c_str());
    CHECK(music.music_ok() && music.music_on() && music.count() == 8);
    CHECK(music.title(0) == "Atlantis" && music.title(7) == "Steady Pulse");
    // Every song parses and every timbre it lists is in the library.
    const char *files[] = {"atlantis", "sixseven", "fortune", "candy", "mend", "doctor", "nightsh", "steady"};
    for (const char *name : files) {
        auto path = th::resolve_ci(dir, std::string("sound/midi/") + name + ".xmi");
        auto bytes = path ? th::read_file(*path) : std::nullopt;
        th::XmiSong song;
        CHECK(bytes && th::parse_xmi(*bytes, song, &error));
        CHECK(song.ticks > 60 * 120 && !song.timbres.empty());
        for (const auto &t : song.timbres)
            CHECK(!gtl.find(t[0], t[1]).empty());
    }

    // Nothing plays before a level starts; then the clock's track does.
    std::vector<int16_t> pcm(2 * 44100);
    music.render(pcm.data(), 4410, 44100);
    CHECK(std::all_of(pcm.begin(), pcm.begin() + 2 * 4410, [](int16_t v) { return v == 0; }));
    music.start_level(13);
    CHECK(music.playing() && music.track() == 13 % 8);
    // Three seconds of "Doctor Frog", in one-second blocks: sound, and the
    // same sound every run.
    pcm.assign(3 * 2 * 44100, 0);
    for (int i = 0; i < 3; i++)
        music.render(pcm.data() + i * 2 * 44100, 44100, 44100);
    int peak = 0;
    for (int16_t v : pcm)
        peak = std::max(peak, std::abs(int(v)));
    CHECK(peak > 2000);
    // Regression: the hash of these three seconds as first rendered.
    CHECK(hash_pcm(pcm) == 0xc1d258b809b14347ull);
    th::MusicPlayer again;
    CHECK(again.load(dir));
    std::vector<int16_t> pcm2(pcm.size());
    again.render(pcm2.data(), 4410, 44100);
    again.start_level(5);
    for (int i = 0; i < 3; i++)
        again.render(pcm2.data() + i * 2 * 44100, 44100, 44100);
    CHECK(pcm2 == pcm);

    // The menu: off stops the song, on starts the track again; the volume
    // items as the sound effects'.
    music.toggle_music();
    CHECK(!music.music_on() && !music.playing());
    music.play_next(false);
    CHECK(!music.playing());
    music.toggle_music();
    CHECK(music.music_on() && music.playing() && music.track() == 5);
    music.set_volume_item(9);
    CHECK(music.volume() == 12 && music.volume_item() == 9);
    music.stop();
    CHECK(!music.playing());
}

void test_announcer(const char *dir)
{
    // The announcer's samples are all in every language's bank.
    th::SoundBank bank;
    CHECK(bank.load(dir, 0));
    int announcements = 0;
    for (int i = 0; i < th::kAnnouncementCount; i++) {
        int number = th::announcement_sample(i);
        if (!number)
            continue;
        announcements++;
        CHECK(number > 500 && bank.sample(number) != nullptr);
    }
    CHECK(announcements == 147);
    CHECK(th::announcement_sample(0) == 0 && th::announcement_sample(7) == 548);
    CHECK(th::announcement_sample(th::kAnnounceTooHot) == 682);
    CHECK(th::announcement_sample(th::kAnnouncementCount) == 0);

    th::SoundPlayer player;
    player.set_bank(&bank);
    CHECK(player.ambients_on() && player.ambient_volume() == 127);
    // Without a receptionist, or paused, nothing is queued.
    player.play_ambient(th::kAnnounceTooHot, false, false);
    player.play_ambient(th::kAnnounceTooHot, true, true);
    player.do_ambients(false, false, true);
    CHECK(!player.playing(th::kSourceAmbient, 0));
    player.play_ambient(th::kAnnounceTooHot, true, false);
    player.play_ambient(th::kAnnounceTooCold, true, false);
    player.do_ambients(true, false, true); // paused: waits
    CHECK(!player.playing(th::kSourceAmbient, 0));
    player.do_ambients(false, false, true);
    CHECK(player.playing(th::kSourceAmbient, 682));
    player.do_ambients(false, false, true); // the next waits for this one
    CHECK(!player.playing(th::kSourceAmbient, 681));
    player.stop_all();
    player.do_ambients(false, false, true);
    player.do_ambients(false, false, true);
    CHECK(player.playing(th::kSourceAmbient, 681));
    player.init_ambients();
    // Off: nothing more; the volume items as the sound effects'.
    player.toggle_ambient();
    CHECK(!player.ambients_on());
    player.set_ambient_volume_item(5);
    CHECK(player.ambient_volume() == 63 && player.ambient_volume_item() == 5);
}

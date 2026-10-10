// The player details file: the original's 0x5c-byte HospPlayerDetails.

#include "common.h"

#include "app/player_details.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

void test_player_details_layout()
{
    th::PlayerDetails d;
    d.music_volume = 0x72;
    d.sfx_volume = 0x51;
    d.ambient_volume = 0x29;
    d.sfx_on = 0;
    d.ambients_on = 1;
    d.music_on = 0;
    d.track_enabled[3] = 0;
    d.shadows_on = 0;
    d.game_speed = 0x7d00;
    d.speed_choice = 4;
    d.language = 2;
    d.autosave = 0;
    d.room_help_seen[7] = 1;
    d.room_help_seen[29] = 1;
    d.advisor_flags[0] = 1;
    d.advisor_flags[20] = 0;
    d.years_played = 0x0203;
    auto b = th::encode_player_details(d, "ABCDEFGH");

    // include/hosp.h's offsets.
    CHECK(b.size() == 0x5c);
    CHECK(b[0] == 'A' && b[7] == 'H' && b[8] == 0);
    CHECK(b[0x9] == 0x03 && b[0xa] == 0x02);
    CHECK(b[0xc] == 0x72 && b[0xd] == 0x51 && b[0xe] == 0x29);
    CHECK(b[0xf] == 0 && b[0x10] == 1 && b[0x11] == 0);
    CHECK(b[0x12 + 2] == 1 && b[0x12 + 3] == 0);
    CHECK(b[0x1d] == 0);
    CHECK(b[0x1e] == 0x00 && b[0x1f] == 0x7d);
    CHECK(b[0x20] == 4 && b[0x21] == 2 && b[0x22] == 0);
    CHECK(b[0x23 + 7] == 1 && b[0x23 + 29] == 1);
    CHECK(b[0x41] == 1 && b[0x41 + 20] == 0);

    th::PlayerDetails r;
    th::decode_player_details(r, b);
    CHECK(r.music_volume == 0x72 && r.sfx_volume == 0x51 && r.ambient_volume == 0x29);
    CHECK(r.sfx_on == 0 && r.ambients_on == 1 && r.music_on == 0);
    CHECK(r.track_enabled[3] == 0 && r.track_enabled[4] == 1);
    CHECK(r.shadows_on == 0 && r.game_speed == 0x7d00 && r.speed_choice == 4);
    CHECK(r.language == 2 && r.autosave == 0);
    CHECK(r.room_help_seen[7] == 1 && r.room_help_seen[29] == 1 && r.room_help_seen[0] == 0);
    CHECK(r.advisor_flags[0] == 1 && r.advisor_flags[20] == 0);
    CHECK(r.years_played == 0x0203);

    // A short file changes only the bytes it holds.
    th::PlayerDetails s;
    th::decode_player_details(s, std::span<const uint8_t>(b.data(), 0x12));
    CHECK(s.music_volume == 0x72 && s.music_on == 0);
    CHECK(s.shadows_on == 1 && s.autosave == 1 && s.game_speed == th::kDefaultGameSpeed);

    // set_default_player_details leaves what the original's cleared-by-
    // nothing calls leave.
    th::set_default_player_details(r);
    CHECK(r.autosave == 1 && r.game_speed == th::kDefaultGameSpeed && r.speed_choice == 2 && r.shadows_on == 1);
    CHECK(r.sfx_on == 1 && r.music_on == 1 && r.ambients_on == 1 && r.music_volume == 0x7f);
    CHECK(r.advisor_flags[20] == 1);
    CHECK(r.room_help_seen[7] == 1 && r.advisor_flags[0] == 1 && r.track_enabled[3] == 0);
}

void test_player_details_file()
{
    namespace fs = std::filesystem;
    fs::path dir = fs::temp_directory_path() / "th_test_player_details";
    fs::remove_all(dir);
    fs::path path = th::player_details_path(dir / "save", "ANN");
    CHECK(path == dir / "save" / "ANN.hdt");

    th::PlayerDetails d;
    CHECK(th::read_player_details(path, d) == th::DetailsRead::Missing);
    d.autosave = 0;
    d.advisor_flags[3] = 1;
    CHECK(th::write_player_details(path, d, "ANN"));
    CHECK(fs::file_size(path) == th::kPlayerDetailsSize);

    th::PlayerDetails r;
    CHECK(th::read_player_details(path, r) == th::DetailsRead::Read);
    CHECK(r.autosave == 0 && r.advisor_flags[3] == 1 && r.advisor_flags[20] == 0);
    fs::remove_all(dir);
}

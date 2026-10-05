// Engine tests. Those that need the game data read it from TH_TEST_DATA_DIR
// and are skipped (exit code 77) when it is unset or missing.

#include "core/data/blocks.h"
#include "core/data/gamedata.h"
#include "core/data/rnc.h"
#include "core/render/blit.h"
#include "core/render/iso.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>

namespace
{

int failures = 0;

#define CHECK(cond)                                                                                                    \
    do {                                                                                                               \
        if (!(cond)) {                                                                                                 \
            std::fprintf(stderr, "%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #cond);                              \
            failures++;                                                                                                \
        }                                                                                                              \
    } while (0)

void test_blit_clips_and_skips()
{
    // 3x2 sprite: row 0 "skip 1, pixels 7 8", row 1 "pixels 9 9 9".
    const uint8_t rle[] = {0xff, 2, 7, 8, 0, 3, 9, 9, 9, 0};
    th::Sprite s{rle, th::SpriteFormat::Rle, 3, 2};
    th::Surface surf;
    surf.resize(4, 2);
    surf.fill(1);
    th::draw_sprite(surf, 1, 0, s);
    CHECK(surf.row(0)[1] == 1); // transparent run
    CHECK(surf.row(0)[2] == 7 && surf.row(0)[3] == 8);
    CHECK(surf.row(1)[1] == 9 && surf.row(1)[3] == 9);
    surf.fill(1);
    th::draw_sprite(surf, 2, 0, s, th::kDrawFlipX); // right column clipped
    CHECK(surf.row(0)[2] == 8 && surf.row(0)[3] == 7);
}

void test_req_format()
{
    // 70x1: copy 2 (5 6), skip 1, fill 4 with 3 (c = 64), fill 63 with 2 (255).
    const uint8_t req[] = {2, 5, 6, 0x81, 64, 3, 0xff, 63, 2, 0};
    th::Sprite s{req, th::SpriteFormat::Req, 70, 1};
    th::Surface surf;
    surf.resize(70, 1);
    surf.fill(1);
    th::draw_sprite(surf, 0, 0, s);
    CHECK(surf.row(0)[0] == 5 && surf.row(0)[1] == 6);
    CHECK(surf.row(0)[2] == 1);
    CHECK(surf.row(0)[3] == 3 && surf.row(0)[6] == 3);
    CHECK(surf.row(0)[7] == 2 && surf.row(0)[69] == 2);
}

void test_iso_round_trip()
{
    for (int col = 0; col < 128; col += 17)
        for (int row = 0; row < 128; row += 13) {
            th::Point w = th::cell_to_world(col, row);
            // A point just inside the diamond's top corner belongs to the cell.
            th::Point c = th::world_to_cell({w.x, w.y + 1});
            CHECK(c.x == col && c.y == row);
        }
}

void test_rnc_rejects_garbage()
{
    const uint8_t junk[] = {'R', 'N', 'C', 1, 0, 0, 0, 4, 0, 0, 0, 0};
    CHECK(!th::rnc_unpack(junk).has_value());
}

void test_missing_data()
{
    // A folder without the game reports every required file.
    auto dir = std::filesystem::temp_directory_path() / "th_enhanced_empty_test_dir";
    std::filesystem::create_directories(dir);
    CHECK(th::missing_data_files(dir).size() == th::required_data_files().size());
    std::filesystem::remove(dir);
}

// With the game data: every required file unpacks (RNC checks both CRCs),
// sprite tables cover their blocks, and a level builds.
void test_game_data(const char *dir)
{
    for (std::string_view rel : th::required_data_files()) {
        std::string error;
        auto data = th::load_data_file(dir, rel, &error);
        if (!data)
            std::fprintf(stderr, "%s\n", error.c_str());
        CHECK(data.has_value());
    }
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    if (!error.empty())
        std::fprintf(stderr, "%s\n", error.c_str());
    CHECK(gd.blocks().size() > 0);
    CHECK(gd.panel().size() > 14);
    auto level = gd.load_level(1, &error);
    CHECK(level.has_value());
    if (level) {
        CHECK(level->player_count >= 1 && level->player_count <= 4);
        auto map = th::build_start_map(*level);
        int walls = 0;
        for (const th::MapCell &c : map) {
            CHECK(th::block_sprite(c.floor_block) >= 0);
            walls += c.wall_n != 0 || c.wall_w != 0;
        }
        CHECK(walls > 0);
    }
}

} // namespace

int main()
{
    test_blit_clips_and_skips();
    test_req_format();
    test_iso_round_trip();
    test_rnc_rejects_garbage();
    test_missing_data();

    const char *dir = std::getenv("TH_TEST_DATA_DIR");
    bool have_data = dir && *dir && th::missing_data_files(dir).empty();
    if (have_data)
        test_game_data(dir);

    if (failures) {
        std::fprintf(stderr, "%d check(s) failed\n", failures);
        return 1;
    }
    if (!have_data) {
        std::fprintf(stderr, "game data not found: skipped the data tests\n");
        return 77;
    }
    std::printf("all tests passed\n");
    return 0;
}

// Engine tests. Those that need the game data read it from TH_TEST_DATA_DIR
// and are skipped (exit code 77) when it is unset or missing.
//
// th_tests [filter] runs only the tests whose name contains `filter`.

#include "common.h"

#include "core/data/gamedata.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

// render.cpp
void test_blit_clips_and_skips();
void test_req_format();
void test_iso_round_trip();
void test_static_shadows();
void test_shadow_sprites(const char *dir);
void test_level_objects(const char *dir);
void test_draw_world(const char *dir);
void test_panel_image(const char *dir);
void test_text_layout();

// data.cpp
void test_rnc_rejects_garbage();
void test_missing_data();
void test_game_data(const char *dir);
void test_language_and_fonts(const char *dir);
void test_balance(const char *dir);
void test_smacker_intro(const char *dir);

// audio.cpp
void test_sound_player();
void test_xmi_parse();
void test_sound_bank(const char *dir);
void test_music(const char *dir);
void test_announcer(const char *dir);

// nav.cpp
void test_wall_in_way();
void test_wallhug();
void test_orthonav_whatnext();
void test_walk_level1(const char *dir);

// patients.cpp
void test_create_an_illness();
void test_set_complex();
void test_arrivals_level1(const char *dir);
void test_patient_leaves_level1(const char *dir);
void test_litter_drawn(const char *dir);
void test_litter_dropping_level1(const char *dir);
void test_reputation_go_home(const char *dir);

// ui.cpp
void test_menu_bar(const char *dir);
void test_requester_art(const char *dir);
void test_panel_buttons(const char *dir);
void test_yes_no_requester(const char *dir);
void test_main_menu_art(const char *dir);
void test_startup_new_game(const char *dir);
void test_town_map(const char *dir);
void test_jukebox(const char *dir);
void test_charts(const char *dir);

// staff.cpp
void test_applicants_level1(const char *dir);
void test_hire_receptionist(const char *dir);
void test_hire_requester_image(const char *dir);
void test_staff_at_work_level1(const char *dir);
void test_handyman_sweeps(const char *dir);
void test_pick_up_staff(const char *dir);
void test_sack_doctor(const char *dir);
void test_staff_notice_runs_out(const char *dir);

// game.cpp
void test_game_clock(const char *dir);
void test_finance_level1(const char *dir);
void test_balance_roll(const char *dir);
void test_advisor_tutorial(const char *dir);
void test_advisor_advice(const char *dir);

// rooms.cpp
void test_furnish_corridor(const char *dir);
void test_queue_lists();
void test_entrance_rooms_level1(const char *dir);
void test_reception_queue_level1(const char *dir);
void test_events_level1(const char *dir);
void test_build_gp_office(const char *dir);
void test_gp_office_level1(const char *dir);
void test_blueprint_cleared_level1(const char *dir);
void test_general_diag_level1(const char *dir);
void test_cardiogram_level6(const char *dir);
void test_ward_level1(const char *dir);
void test_psychiatry_level1(const char *dir);

// replay.cpp
void test_snapshot_round_trip(const char *dir);
void test_snapshot_refused(const char *dir);
void test_command_log_replay(const char *dir);

namespace
{

struct Test {
    const char *name;
    void (*run)();
};

struct DataTest {
    const char *name;
    void (*run)(const char *dir);
};

#define TEST(fn) {#fn, fn}

const Test kTests[] = {
    TEST(test_blit_clips_and_skips), TEST(test_req_format),        TEST(test_iso_round_trip),
    TEST(test_rnc_rejects_garbage),  TEST(test_missing_data),      TEST(test_static_shadows),
    TEST(test_text_layout),          TEST(test_wall_in_way),       TEST(test_wallhug),
    TEST(test_orthonav_whatnext),    TEST(test_create_an_illness), TEST(test_set_complex),
    TEST(test_queue_lists),          TEST(test_sound_player),      TEST(test_xmi_parse),
};

const DataTest kDataTests[] = {
    TEST(test_game_data),
    TEST(test_sound_bank),
    TEST(test_shadow_sprites),
    TEST(test_level_objects),
    TEST(test_draw_world),
    TEST(test_panel_image),
    TEST(test_game_clock),
    TEST(test_language_and_fonts),
    TEST(test_balance),
    TEST(test_walk_level1),
    TEST(test_arrivals_level1),
    TEST(test_patient_leaves_level1),
    TEST(test_menu_bar),
    TEST(test_requester_art),
    TEST(test_panel_buttons),
    TEST(test_yes_no_requester),
    TEST(test_applicants_level1),
    TEST(test_hire_receptionist),
    TEST(test_hire_requester_image),
    TEST(test_furnish_corridor),
    TEST(test_smacker_intro),
    TEST(test_main_menu_art),
    TEST(test_startup_new_game),
    TEST(test_music),
    TEST(test_announcer),
    TEST(test_finance_level1),
    TEST(test_balance_roll),
    TEST(test_advisor_tutorial),
    TEST(test_advisor_advice),
    TEST(test_staff_at_work_level1),
    TEST(test_litter_drawn),
    TEST(test_litter_dropping_level1),
    TEST(test_handyman_sweeps),
    TEST(test_entrance_rooms_level1),
    TEST(test_reception_queue_level1),
    TEST(test_events_level1),
    TEST(test_town_map),
    TEST(test_jukebox),
    TEST(test_charts),
    TEST(test_build_gp_office),
    TEST(test_reputation_go_home),
    TEST(test_gp_office_level1),
    TEST(test_blueprint_cleared_level1),
    TEST(test_snapshot_round_trip),
    TEST(test_snapshot_refused),
    TEST(test_command_log_replay),
    TEST(test_pick_up_staff),
    TEST(test_sack_doctor),
    TEST(test_staff_notice_runs_out),
    TEST(test_general_diag_level1),
    TEST(test_cardiogram_level6),
    TEST(test_ward_level1),
    TEST(test_psychiatry_level1),
};

} // namespace

int main(int argc, char **argv)
{
    const char *filter = argc > 1 ? argv[1] : "";
    int ran = 0;
    for (const Test &t : kTests)
        if (std::strstr(t.name, filter)) {
            t.run();
            ran++;
        }

    const char *dir = std::getenv("TH_TEST_DATA_DIR");
    bool have_data = dir && *dir && th::missing_data_files(dir).empty();
    if (have_data)
        for (const DataTest &t : kDataTests)
            if (std::strstr(t.name, filter)) {
                t.run(dir);
                ran++;
            }

    if (failures) {
        std::fprintf(stderr, "%d check(s) failed\n", failures);
        return 1;
    }
    if (!have_data) {
        std::fprintf(stderr, "game data not found: skipped the data tests\n");
        return 77;
    }
    if (ran == 0) {
        std::fprintf(stderr, "no test matches \"%s\"\n", filter);
        return 1;
    }
    std::printf("all tests passed\n");
    return 0;
}

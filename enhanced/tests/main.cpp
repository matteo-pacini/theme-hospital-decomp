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
void test_drive_script();
void test_drive_quick_report();
void test_req_format();
void test_iso_round_trip();
void test_static_shadows();
void test_shadow_sprites(const char *dir);
void test_level_objects(const char *dir);
void test_draw_world(const char *dir);
void test_draw_order_stable(const char *dir);
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
void test_epidemic();
void test_arrivals_level1(const char *dir);
void test_patient_leaves_level1(const char *dir);
void test_litter_drawn(const char *dir);
void test_litter_dropping_level1(const char *dir);
void test_reputation_go_home(const char *dir);
void test_patient_dies_level1(const char *dir);

// rats.cpp
void test_rats();

// computer.cpp
void test_computer_tables();
void test_computer_players(const char *dir);

// ui.cpp
void test_menu_bar(const char *dir);
void test_requester_art(const char *dir);
void test_panel_buttons(const char *dir);
void test_yes_no_requester(const char *dir);
void test_main_menu_art(const char *dir);
void test_startup_new_game(const char *dir);
void test_startup_load_screen(const char *dir);
void test_town_map(const char *dir);
void test_jukebox(const char *dir);
void test_charts(const char *dir);
void test_info_windows(const char *dir);
void test_panel_popups(const char *dir);
void test_fax(const char *dir);

// help.cpp
void test_help_table();
void test_help_group_starts();
void test_help_timing();
void test_help_tooltip_text(const char *dir);
void test_help_text_ids(const char *dir);
void test_help_box_clamped(const char *dir);
void test_help_page(const char *dir);
void test_help_clears_after_draw(const char *dir);
void test_help_white(const char *dir);
void test_help_callback_wrapped(const char *dir);
void test_help_requester_groups();
void test_help_panel_and_requester();
void test_help_draw_gate(const char *dir);
void test_help_main_menu_font(const char *dir);
void test_help_page_shadow(const char *dir);

// icons.cpp
void test_status_icons(const char *dir);

// staff_demand.cpp
void test_staff_demand(const char *dir);

// panel_info.cpp
void test_panel_info(const char *dir);

// help_callbacks.cpp
void test_help_callbacks_panel(const char *dir);
void test_help_callbacks_patient_windows(const char *dir);
void test_help_callbacks_staff_windows(const char *dir);
void test_help_callbacks_requesters(const char *dir);
void test_help_callbacks_slist(const char *dir);
void test_help_callbacks_manager(const char *dir);
void test_help_callbacks_status(const char *dir);
void test_help_callbacks_drugs(const char *dir);

// help_screens.cpp
void test_help_charts_screens(const char *dir);
void test_help_town_map_screen(const char *dir);
void test_help_main_menu_screen(const char *dir);

// staff.cpp
void test_applicants_level1(const char *dir);
void test_hire_receptionist(const char *dir);
void test_hire_requester_image(const char *dir);
void test_staff_at_work_level1(const char *dir);
void test_handyman_sweeps(const char *dir);
void test_handyman_waters(const char *dir);
void test_pick_up_staff(const char *dir);
void test_sack_doctor(const char *dir);
void test_staff_notice_runs_out(const char *dir);
void test_adjust_handyman_values();
void test_applicant_expiry();

// game.cpp
void test_game_clock(const char *dir);
void test_fax_palette(const char *dir);
void test_open_hospital(const char *dir);
void test_finance_level1(const char *dir);
void test_balance_roll(const char *dir);
void test_advisor_tutorial(const char *dir);
void test_advisor_advice(const char *dir);
void test_advisor_counts(const char *dir);

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
void test_emergency_accept_level6(const char *dir);
void test_emergency_refused_level6(const char *dir);
void test_emergency_untreatable_level6(const char *dir);
void test_emergency_fail_level6(const char *dir);
void test_decisions_diagnosis_level1(const char *dir);
void test_decisions_treatment_level1(const char *dir);
void test_new_illness_call(const char *dir);
void test_ward_level1(const char *dir);
void test_psychiatry_level1(const char *dir);
void test_treatment_level1(const char *dir);
void test_info_windows_level1(const char *dir);
void test_nearest_room_info();
void test_staff_room_level1(const char *dir);
void test_toilets_level1(const char *dir);
void test_toilets_none_level1(const char *dir);
void test_machine_explodes_level6(const char *dir);
void test_machine_saved_level6(const char *dir);
void test_machine_replace_level6(const char *dir);
void test_do_earthquake_level6(const char *dir);
void test_disasters_level1(const char *dir);

// research.cpp
void test_research_choices();
void test_research_drugs();
void test_research_contribution();
void test_research_level3(const char *dir);

// replay.cpp
void test_snapshot_round_trip(const char *dir);
void test_snapshot_refused(const char *dir);
void test_save_slot_round_trip(const char *dir);
void test_save_slot_scripts(const char *dir);
void test_save_slot_refused(const char *dir);
void test_command_log_replay(const char *dir);

// yearend.cpp
void test_hall_scores_table();
void test_yearend_screens(const char *dir);
void test_winlose_screen(const char *dir);
void test_winlose_letters(const char *dir);

// sim_yearend.cpp
void test_yearend_criteria(const char *dir);
void test_yearend_score(const char *dir);
void test_yearend_year(const char *dir);
void test_yearend_trophy_kept(const char *dir);
void test_yearend_drinks_counted(const char *dir);
void test_yearend_object_value(const char *dir);

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
    TEST(test_blit_clips_and_skips),
    TEST(test_drive_script),
    TEST(test_drive_quick_report),
    TEST(test_req_format),
    TEST(test_iso_round_trip),
    TEST(test_rnc_rejects_garbage),
    TEST(test_missing_data),
    TEST(test_static_shadows),
    TEST(test_text_layout),
    TEST(test_wall_in_way),
    TEST(test_wallhug),
    TEST(test_orthonav_whatnext),
    TEST(test_create_an_illness),
    TEST(test_set_complex),
    TEST(test_epidemic),
    TEST(test_rats),
    TEST(test_computer_tables),
    TEST(test_queue_lists),
    TEST(test_sound_player),
    TEST(test_xmi_parse),
    TEST(test_adjust_handyman_values),
    TEST(test_applicant_expiry),
    TEST(test_nearest_room_info),
    TEST(test_research_choices),
    TEST(test_research_drugs),
    TEST(test_research_contribution),
    TEST(test_help_table),
    TEST(test_help_group_starts),
    TEST(test_help_timing),
    TEST(test_help_requester_groups),
    TEST(test_help_panel_and_requester),
    TEST(test_hall_scores_table),
};

const DataTest kDataTests[] = {
    TEST(test_game_data),
    TEST(test_computer_players),
    TEST(test_sound_bank),
    TEST(test_shadow_sprites),
    TEST(test_level_objects),
    TEST(test_draw_world),
    TEST(test_draw_order_stable),
    TEST(test_panel_image),
    TEST(test_game_clock),
    TEST(test_fax_palette),
    TEST(test_open_hospital),
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
    TEST(test_startup_load_screen),
    TEST(test_music),
    TEST(test_announcer),
    TEST(test_finance_level1),
    TEST(test_balance_roll),
    TEST(test_advisor_tutorial),
    TEST(test_advisor_advice),
    TEST(test_advisor_counts),
    TEST(test_staff_at_work_level1),
    TEST(test_litter_drawn),
    TEST(test_litter_dropping_level1),
    TEST(test_handyman_sweeps),
    TEST(test_handyman_waters),
    TEST(test_entrance_rooms_level1),
    TEST(test_reception_queue_level1),
    TEST(test_events_level1),
    TEST(test_town_map),
    TEST(test_jukebox),
    TEST(test_charts),
    TEST(test_info_windows),
    TEST(test_build_gp_office),
    TEST(test_reputation_go_home),
    TEST(test_patient_dies_level1),
    TEST(test_gp_office_level1),
    TEST(test_blueprint_cleared_level1),
    TEST(test_snapshot_round_trip),
    TEST(test_snapshot_refused),
    TEST(test_save_slot_round_trip),
    TEST(test_save_slot_scripts),
    TEST(test_save_slot_refused),
    TEST(test_command_log_replay),
    TEST(test_research_level3),
    TEST(test_pick_up_staff),
    TEST(test_sack_doctor),
    TEST(test_staff_notice_runs_out),
    TEST(test_general_diag_level1),
    TEST(test_cardiogram_level6),
    TEST(test_emergency_accept_level6),
    TEST(test_emergency_refused_level6),
    TEST(test_emergency_untreatable_level6),
    TEST(test_emergency_fail_level6),
    TEST(test_decisions_diagnosis_level1),
    TEST(test_decisions_treatment_level1),
    TEST(test_new_illness_call),
    TEST(test_panel_popups),
    TEST(test_fax),
    TEST(test_ward_level1),
    TEST(test_psychiatry_level1),
    TEST(test_treatment_level1),
    TEST(test_info_windows_level1),
    TEST(test_staff_room_level1),
    TEST(test_toilets_level1),
    TEST(test_toilets_none_level1),
    TEST(test_machine_explodes_level6),
    TEST(test_machine_saved_level6),
    TEST(test_machine_replace_level6),
    TEST(test_do_earthquake_level6),
    TEST(test_disasters_level1),
    TEST(test_help_tooltip_text),
    TEST(test_help_text_ids),
    TEST(test_help_box_clamped),
    TEST(test_help_page),
    TEST(test_help_clears_after_draw),
    TEST(test_help_white),
    TEST(test_help_callback_wrapped),
    TEST(test_help_draw_gate),
    TEST(test_help_main_menu_font),
    TEST(test_help_page_shadow),
    TEST(test_status_icons),
    TEST(test_staff_demand),
    TEST(test_panel_info),
    TEST(test_help_callbacks_panel),
    TEST(test_help_callbacks_patient_windows),
    TEST(test_help_callbacks_staff_windows),
    TEST(test_help_callbacks_requesters),
    TEST(test_help_callbacks_slist),
    TEST(test_help_callbacks_manager),
    TEST(test_help_callbacks_status),
    TEST(test_help_callbacks_drugs),
    TEST(test_help_charts_screens),
    TEST(test_help_town_map_screen),
    TEST(test_help_main_menu_screen),
    TEST(test_yearend_screens),
    TEST(test_winlose_screen),
    TEST(test_winlose_letters),
    TEST(test_yearend_criteria),
    TEST(test_yearend_score),
    TEST(test_yearend_year),
    TEST(test_yearend_trophy_kept),
    TEST(test_yearend_drinks_counted),
    TEST(test_yearend_object_value),
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

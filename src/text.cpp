#include "hosp.h"

extern "C" {

extern char **staff_names;
extern char **object_names;
extern char **misc_text;
extern char **illness_text;
extern char **computer_names;
extern char **report_text;
extern char **finance_text;
extern char **town_names;
extern char **town_details;
extern char **room_names;
extern char **supplier_names;
extern char **req1_help;
extern char **research_names;
extern char **policy_names;
extern char **department_names;
extern char **forced_advice_text;
extern char **mouse_advice_text;
extern char **pulldown_text;
extern char **slist_text;
extern char **chart_types;
extern char **trophy_text;
extern char **cabinet_text;
extern char **network_text;
extern char **skill_names;
extern char **stat_text;
extern char **win_lose;
extern char **win_text;
extern char **credit_text;
extern char **queue_text;
extern char **winlose_text;
extern char **progress_text;
extern char **debug_msgs;
extern char **real_room_names;
extern char **level_text;
extern char **good_awards_text;
extern char **bad_awards_text;
extern char **show_help_text_0;
extern char **show_help_text_1;
extern char **overview_text;
extern char **staff_unhappy_text;
extern char **trophy_bonus_messages;
extern char **win_criteria_help;
extern char **net_mission_text;
extern char **beta_demo_text_short;
}

// Points language string table `group` (the section number in the language
// file: 0 staff_names, 1 object_names, 2 misc_text, ...) at address. Returns
// 1 (also for an unknown group).
// FUNCTION: 0x0002d1c0
u8 set_group_address(s32 group, char **address)
{
    if (group == 0) {
        staff_names = address;
        return 1;
    }
    if (group == 1) {
        object_names = address;
        return 1;
    }
    if (group == 2) {
        misc_text = address;
        return 1;
    }
    if (group == 3) {
        illness_text = address;
        return 1;
    }
    if (group == 4) {
        computer_names = address;
        return 1;
    }
    if (group == 5) {
        month_text = address;
        return 1;
    }
    if (group == 6) {
        report_text = address;
        return 1;
    }
    if (group == 7) {
        finance_text = address;
        return 1;
    }
    if (group == 8) {
        staff_names_1 = address;
        return 1;
    }
    if (group == 9) {
        staff_names_2 = address;
        return 1;
    }
    if (group == 0xa) {
        advisor_text = address;
        return 1;
    }
    if (group == 0xb) {
        town_names = address;
        return 1;
    }
    if (group == 0xc) {
        town_details = address;
        return 1;
    }
    if (group == 0xd) {
        room_names = address;
        return 1;
    }
    if (group == 0xe) {
        supplier_names = address;
        return 1;
    }
    if (group == 0xf) {
        req1_help = address;
        return 1;
    }
    if (group == 0x10) {
        research_names = address;
        return 1;
    }
    if (group == 0x11) {
        policy_names = address;
        return 1;
    }
    if (group == 0x12) {
        department_names = address;
        return 1;
    }
    if (group == 0x13) {
        insurance_names = address;
        return 1;
    }
    if (group == 0x14) {
        forced_advice_text = address;
        return 1;
    }
    if (group == 0x15) {
        mouse_advice_text = address;
        return 1;
    }
    if (group == 0x16) {
        pulldown_text = address;
        return 1;
    }
    if (group == 0x17) {
        slist_text = address;
        return 1;
    }
    if (group == 0x18) {
        hall_text = address;
        return 1;
    }
    if (group == 0x19) {
        chart_types = address;
        return 1;
    }
    if (group == 0x1a) {
        trophy_text = address;
        return 1;
    }
    if (group == 0x1b) {
        extra_advice = address;
        return 1;
    }
    if (group == 0x1c) {
        cabinet_text = address;
        return 1;
    }
    if (group == 0x1d) {
        network_text = address;
        return 1;
    }
    if (group == 0x1e) {
        nickname_text = address;
        return 1;
    }
    if (group == 0x1f) {
        help_text = address;
        return 1;
    }
    if (group == 0x20) {
        room_help = address;
        return 1;
    }
    if (group == 0x21) {
        skill_names = address;
        return 1;
    }
    if (group == 0x22) {
        yesno_text = address;
        return 1;
    }
    if (group == 0x23) {
        bank_text = address;
        return 1;
    }
    if (group == 0x24) {
        stat_text = address;
        return 1;
    }
    if (group == 0x25) {
        win_lose = address;
        return 1;
    }
    if (group == 0x26) {
        win_text = address;
        return 1;
    }
    if (group == 0x27) {
        object_help = address;
        return 1;
    }
    if (group == 0x28) {
        load_text = address;
        return 1;
    }
    if (group == 0x29) {
        mayor_names = address;
        return 1;
    }
    if (group == 0x2a) {
        credit_text = address;
        return 1;
    }
    if (group == 0x2b) {
        choice_text = address;
        return 1;
    }
    if (group == 0x2c) {
        cheat_text = address;
        return 1;
    }
    if (group == 0x2d) {
        ref_general_text = address;
        return 1;
    }
    if (group == 0x2e) {
        ref_good_text = address;
        return 1;
    }
    if (group == 0x2f) {
        ref_bad_text = address;
        return 1;
    }
    if (group == 0x30) {
        queue_text = address;
        return 1;
    }
    if (group == 0x31) {
        winlose_text = address;
        return 1;
    }
    if (group == 0x32) {
        progress_text = address;
        return 1;
    }
    if (group == 0x33) {
        debug_msgs = address;
        return 1;
    }
    if (group == 0x34) {
        real_room_names = address;
        return 1;
    }
    if (group == 0x35) {
        level_text = address;
        return 1;
    }
    if (group == 0x36) {
        good_awards_text = address;
        return 1;
    }
    if (group == 0x37) {
        bad_awards_text = address;
        return 1;
    }
    if (group == 0x38) {
        show_help_text_0 = address;
        return 1;
    }
    if (group == 0x39) {
        show_help_text_1 = address;
        return 1;
    }
    if (group == 0x3a) {
        info_bar_text = address;
        return 1;
    }
    if (group == 0x3b) {
        overview_text = address;
        return 1;
    }
    if (group == 0x3c) {
        staff_unhappy_text = address;
        return 1;
    }
    if (group == 0x3d) {
        advisor_situations = address;
        return 1;
    }
    if (group == 0x3e) {
        trophy_bonus_messages = address;
        return 1;
    }
    if (group == 0x3f) {
        win_criteria_help = address;
        return 1;
    }
    if (group == 0x42) {
        net_mission_text = address;
        return 1;
    }
    if (group == 0x43) {
        beta_demo_text_short = address;
        return 1;
    }
    return 1;
}

// Loads the language file for myplayer.language and splits it into its string
// tables (set_group_address), allocating text_starts. Returns 1 on success,
// 0 if the file or memory could not be had.
// FUNCTION: 0x0002d7c4
s16 setup_lang(void)
{
    s16 *counts;
    s32 groups;
    s32 i;
    s32 total;
    s32 j;
    char **next_start;
    char *text;

    if (LbDataLoad(&langfile[myplayer.language]) != 1)
        return 0;
    groups = *(s16 *)text_buffer;
    counts = (s16 *)(text_buffer + 2);
    text = (char *)(text_buffer + groups * 2 + 2);
    total = 0;
    for (i = 0; i < groups; i++)
        total += counts[i];
    text_starts = (char **)LbMemoryAlloc(total * 4);
    if (text_starts == 0)
        return 0;
    next_start = text_starts;
    for (i = 0; i < groups; i++) {
        if (!set_group_address(i, next_start))
            return 0;
        for (j = 0; j < counts[i]; j++) {
            *next_start++ = text;
            while (*text)
                text++;
            text++;
        }
    }
    return 1;
}

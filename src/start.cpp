#include "hosp.h"

extern "C" {
extern char **misc_text;
extern u8 tutorial_display_flags[];
extern u16 all_illnesses[];

u32 lib_strlen(char *s);
#pragma aux lib_strlen "strlen_"
void setup_pulldown_menu_options(void);
extern u8 *palette;
extern char **show_help_text_0, **level_text, **win_text, **net_mission_text, **beta_demo_text_short;
extern u8 xs[], ys[];
extern u8 new_game;
extern Event D_001549d0[];
void setup_interactions(void);
}

extern WinLoseNet net_criteria[];
extern u16 ins_control[];

void copy_my_hosp(AllHosps *dst, AllHosps *src);
void balance_control(void);
void setup_insurances(void);
void init_player(u8 owner);
void setup_rat_fest_panel(void);
void rat_fest_builder(u8 level);
void training_builder(u8 level);
void create_job_market(void);
void calc_visitors_this_month(void);
void setup_computer_players(void);
void setup_next_emergency(void);
void setup_next_quake(void);
void setup_next_net_criteria(const u8 owner);
void play_midi(u8 track);
void read_variable_file(char *name);
void evaluate_reputation(const u8 treatment, const u8 owner);
void calc_pricing_factor(const u8 owner);
u8 load_player_details(void);
void init_mcga_coords(void);
void init_midi(void);
void init_andys_variables(void);
void setup_req4_references(void);
void setup_help_indices(void);
void setup_aniseq_indices(void);
void setup_indices_for_composite_help(char **strs, u16 *idx, u32 n);
void init_wibble(void *xoff, void *yoff, s32 w, s32 h, s32 width, s32 freq, s32 amp);

// One-time game setup after startup: menus, interaction and help tables,
// the pixel buffer, music and announcements. With -L or -S it starts the
// level at once (with -S, also sending event EV_LOAD_GAME_AT_START with the -G value);
// otherwise it only queues EV_GOTO_COLUMN_ROW.
// FUNCTION: 0x00090610
extern "C" void start_game(void)
{
    init_mcga_coords();
    set_font_from_cache(0);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    init_andys_variables();
    setup_req4_references();
    crequester.id = prequester.id = REQ_NONE;
    myplayer.shadows_on = 1;
    setup_menus();
    setup_interactions();
    setup_help_indices();
    setup_aniseq_indices();
    setup_indices_for_composite_help(show_help_text_0, statvar.help_page0_start, 0x1f);
    setup_indices_for_composite_help(level_text, statvar.help_page1_start, 0x13);
    setup_indices_for_composite_help(win_text, statvar.winlose_text_start, 0xd);
    setup_indices_for_composite_help(net_mission_text, statvar.net_help_start, 0xb);
    setup_indices_for_composite_help(beta_demo_text_short, statvar.net_result_start, 4);
    InitPixBuffer(0, 0, 0xb4, 0xb4, 0xfe, 1);
    init_midi();
    init_ambients();
    gamecounts.last_turn_clock = LbTimerClock();
    myplayer.game_speed = 0x19;
    init_wibble(xs, ys, pixBuf.width + 0x20, pixBuf.width + 0x20, pixBuf.width, 0x11, 2);
    measure_objects();
    choice_backup.mode = DM_GAME;
    current_text.text[0] = 0;
    if (debug.direct_start) {
        if (lib_strlen(myplayer.name_chars) == 0)
            lib_strcpy(myplayer.name_chars, misc_text[1]);
        setup_new_level((u16)level_number, 1);
        load_player_details();
        othervar.pending_palette = palette;
        if (!new_game) {
            D_001549d0[player].type = EV_LOAD_GAME_AT_START;
            D_001549d0[player].value = debug.save_slot;
        } else {
            D_001549d0[player].type = EV_GOTO_COLUMN_ROW;
        }
    } else {
        D_001549d0[player].type = EV_GOTO_COLUMN_ROW;
    }
}

// Starts level `level` (1-14; others play as 1) for this player: clears the
// game state (keeping the hospital's name, level, simulation flag and
// score), reloads the balance files if reload_balance is set, loads the
// map, builds a rat or training level, and sets up the players, staff,
// visitors, insurers, rivals, disasters and music.
// FUNCTION: 0x00090804
void setup_new_level(int level, u8 reload_balance)
{
    u32 i;
    u8 saved_fa75;
    AllHosps backup;

    myhosp = &allhosps[player];
    if (lib_strlen(myplayer.name_chars) == 0)
        lib_strcpy(myplayer.name_chars, misc_text[1]);
    lib_strcpy((char *)myhosp->name, myplayer.name_chars);
    myhosp->player_type = 1;
    if ((u16)level > 14)
        myhosp->level = 1;
    else
        myhosp->level = level;
    if (reload_balance)
        load_balance(level);
    copy_my_hosp(&backup, myhosp);
    saved_fa75 = othervar.level_after_bonus;
    myplayer.years_played += mycontrol->year;
    memset(thing, 0, 0x40164);
    memset(map, 0, 0x20000);
    memset(room_info, 0, 0xdac);
    memset(pagers, 0, 0x14a);
    memset(allhosps, 0, 0x4964);
    memset(mycontrol, 0, 0x651);
    memset(listhead, 0, 0x3a8);
    memset(&othervar, 0, sizeof(othervar));
    memset(&gamevar, 0, 0x178);
    memset(playercall, 0, 0x258);
    memset(&current_net_criteria, 0, 0xc);
    for (i = 0; i < 20; i++)
        net_criteria[i].done = 0;
    LbMemorySet(&adcounts, 0, 0xa6);
    LbMemorySet(&NRAdcounts, 0, 0x11);
    if ((u16)level != 1)
        LbMemorySet(advisor.advice_id_bytes, 0, 0x133);
    othervar.level_after_bonus = saved_fa75;
    for (i = 0; i < 9; i++)
        popup_id[i] = 1;
    mycontrol->day = 1;
    copy_my_hosp(myhosp, &backup);
    game_turn = 0;
    actual_game_turn = 1;
    safeseed = 0x546b2;
    seed = safeseed;
    load_level(level, 1);
    free_level();
    clear_sound_heap();
    if (towns[myhosp->level].special_kind == 1) {
        gamevar.rat_fest_level = 1;
        setup_rat_fest_panel();
        rat_fest_builder(towns[myhosp->level].special_arg);
    } else if (towns[myhosp->level].special_kind == 2) {
        training_builder(towns[myhosp->level].special_arg);
    }
    create_job_market();
    for (i = 0; i < 4; i++)
        init_player(i);
    othervar.full_redraw = 1;
    othervar.f33 = 1;
    calc_visitors_this_month();
    adcounts.patients = 1;
    mycontrol->statement_balance = myhosp->year_start_balance;
    mycontrol->heating_price = 750;
    sprintf(statvar.save_prefix, "save/%s.h", &myplayer);
    setup_insurances();
    setup_computer_players();
    setup_next_emergency();
    setup_next_quake();
    reset_advisor();
    LbMemorySet(tutorial_display_flags, 0, 0x26);
    othervar.selected_illness = get_next_illness(1, 0);
    check_save_games();
    othervar.popup_shutter_state = 1;
    othervar.emergency_anim_frame = 0x39;
    setup_pulldown_menu_options();
    setup_key_mappos();
    setup_next_net_criteria(player);
    if (statvar.midi_count) {
        othervar.midi_playing = 1;
        if (myplayer.music_on) {
            othervar.music_playing = 1;
            play_midi(LbTimerClock() % statvar.midi_count);
        }
    }
    LbMemorySet(&current_text, 0, 0x5e7);
    LbMemorySet(net_text, 0, 0xfa);
    othervar.net_text_entering = 0;
    close_irequesters();
}

// Copies a hospital's name, level (f29), simulation flag (f2a) and score
// (f2d) from src to dst.
// FUNCTION: 0x00090c8c
void copy_my_hosp(AllHosps *dst, AllHosps *src)
{
    lib_strcpy((char *)dst->name, (char *)src->name);
    dst->level = src->level;
    dst->player_type = src->player_type;
    dst->score = src->score;
}

// Reads the balance settings: levels/<balance_id>00.sam, then the file for
// level `level`, and derives the illness odds (balance_control). An empty
// balance_id becomes "FULL".
// FUNCTION: 0x00090cd8
void load_balance(const u16 level)
{
    char path[0x90];
    char level_path[0x90];

    if (lib_strlen(balance_id) == 0)
        lib_strcpy(balance_id, "FULL");
    if (install_type < 3) {
        sprintf(path, "%slevels/%s00.sam", cd_install_path, balance_id);
        check_cd_available(path);
        sprintf(level_path, "%slevels/%s%2.2d.sam", cd_install_path, balance_id, level);
        check_cd_available(path);
    } else {
        sprintf(path, "levels/%s00.sam", balance_id);
        sprintf(level_path, "levels/%s%2.2d.sam", balance_id, level);
    }
    read_variable_file(path);
    read_variable_file(level_path);
    balance_control();
}

// Turns the illness weights read from the balance file into cumulative
// tables (visuals, non_visuals; every non-visual illness if none is set),
// lists the illnesses in play in all_illnesses and makes their treatment
// machines researchable.
// FUNCTION: 0x00090dd4
void balance_control(void)
{
    u16 i;
    u16 kind;
    u16 count;
    u8 all;

    count = 0;
    all = 1;
    statvar.visuals_total = 0;
    for (i = 0; i < 14; i++) {
        if (visuals[i]) {
            all_illnesses[count++] = i + 2;
            kind = expertise[i + 2].kind;
            objects[kind].available_for_level = 1;
        }
        statvar.visuals_total += visuals[i];
        visuals[i] = statvar.visuals_total;
    }
    for (i = 0; i < 20; i++) {
        if (non_visuals[i]) {
            all = 0;
            break;
        }
    }
    statvar.non_visuals_total = 0;
    for (i = 0; i < 20; i++) {
        if (all)
            non_visuals[i] = 1;
        all_illnesses[count++] = i + 16;
        statvar.non_visuals_total += non_visuals[i];
        non_visuals[i] = statvar.non_visuals_total;
    }
    expertise[ILL_GENERAL_PRACTICE].known = 1;
    statvar.illness_kind_count = count;
}

// Chooses the level's three insurance companies at random out of eight
// (ins_control).
// FUNCTION: 0x00090f70
void setup_insurances(void)
{
    u16 left;
    u8 i;
    u8 pick;
    u8 j;
    u16 pool[8];

    left = 8;
    for (i = 0; i < 8; i++)
        pool[i] = i + 1;
    for (i = 0; i < 3; i++) {
        if (statvar.num_players > 1) {
            pick = old_newrand() % left;
            ins_control[i] = pool[pick];
        } else {
            pick = LbTimerClock() % left;
            ins_control[i] = pool[pick];
        }
        left--;
        for (j = pick; j < left; j++)
            pool[j] = pool[j + 1];
    }
}

// Sets up player `owner`'s hospital for a new level: starting cash, object
// prices and availability, illness prices, drugs and reputation, research
// split evenly, default policy, and the opening countdown (gbv.open_countdown) in a
// single-player level.
// FUNCTION: 0x000910a0
void init_player(u8 owner)
{
    AllHosps *hosp;
    u16 i;

    hosp = &allhosps[owner];
    hosp->playing = 2;
    hosp->balance = towns[myhosp->level].start_cash;
    allhosps[owner].rooms[0].cure_available = 1;
    for (i = 0; i < 0x3e; i++) {
        if (objects[i].available_for_level) {
            hosp->object_stats[i].equip_available = objects[i].start_avail;
            hosp->object_stats[i].available = objects[i].when_avail;
        }
        hosp->object_stats[i].cost = objects[i].cost;
        hosp->object_stats[i].strength = objects[i].start_strength;
    }
    hosp->heating_level = 5;
    for (i = 0; i < 0x2f; i++) {
        hosp->rooms[i].price = expertise[i].start_price;
        hosp->rooms[i].price_scale = 10000;
        hosp->rooms[i].discovered = expertise[i].known;
        setup_drug((u16)owner, i);
        hosp->rooms[i].rep_points = 20;
        hosp->rooms[i].rep_events = 20;
        evaluate_reputation(i, owner);
    }
    for (i = 0; i < 5; i++)
        hosp->research_alloc[i] = 20;
    count_hospital_area((u16)owner);
    if (myhosp->level == 1 || statvar.num_players > 1 || hosp->score == 0)
        hosp->score = 10000;
    for (i = 0; i < 16; i++)
        gamevar.quake_used[i] = 0;
    for (i = 0; i < 20; i++)
        gamevar.emergency_used[i] = 0;
    if (statvar.num_players > 1)
        hosp->market_share = 100 / network_num_players();
    else
        hosp->market_share = 25;
    hosp->diag_threshold = 1000;
    hosp->guess_cure_threshold = 90;
    hosp->send_home_threshold = 10;
    hosp->rest_threshold = gbv.tired;
    hosp->staff_leave_rooms = 1;
    hosp->insurance_accepted = 7;
    hosp->unknown20f[6] = 1;
    if (!(!gbv.open_countdown || statvar.num_players > 1 || gamevar.rat_fest_level)) {
        if (statvar.num_players <= 1)
            hosp->hospital_open = 0;
        hosp->opening_countdown = gbv.open_countdown;
    } else {
        if (statvar.num_players <= 1)
            hosp->hospital_open = 1;
    }
    othervar.watch_hovered = 0;
    hosp->year_start_balance = hosp->balance;
    hosp->max_loan = 15000;
    hosp->start_score = hosp->score;
    hosp->rep_points = 20;
    hosp->rep_events = 20;
    evaluate_reputation(0, owner);
    calc_pricing_factor(owner);
}

// Does nothing.
// FUNCTION: 0x000914bc
void init_mcga_coords(void)
{
}

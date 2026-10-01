#include "hosp.h"

extern "C" {
extern char **D_001602a0;
extern u8 D_00161200[];
extern u16 D_00154834[];

u32 fn_000cd716(char *s);
#pragma aux fn_000cd716 "fn_000cd716_"
void fn_0008bdd4(void);
extern u8 *D_00153f34;
extern u8 D_001581a6;
extern char **D_0016037c, **D_00160374, **D_00160320, **D_00160398, **D_0016039c;
extern u8 D_00156c04[], D_00157624[];
extern u8 D_0015f99f;
extern Event D_001549d0[];
void fn_00054d60(void);
}

extern WinLoseNet net_criteria[];
extern u16 ins_control[];

void copy_my_hosp(AllHosps *dst, AllHosps *src);
void balance_control(void);
void setup_insurances(void);
void init_player(u8 p);
void setup_rat_fest_panel(void);
void rat_fest_builder(u8 kind);
void training_builder(u8 kind);
void create_job_market(void);
void calc_visitors_this_month(void);
void setup_computer_players(void);
void setup_next_emergency(void);
void setup_next_quake(void);
void setup_next_net_criteria(const u8 owner);
void play_midi(u8 track);
void read_variable_file(char *path);
void evaluate_reputation(const u8 a, const u8 owner);
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

// FUNCTION: 0x00090610
extern "C" void fn_00090610(void)
{
    init_mcga_coords();
    set_font_from_cache(0);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    init_andys_variables();
    setup_req4_references();
    crequester.unknown2 = D_001581a6 = 0x63;
    myplayer.f1d = 1;
    setup_menus();
    fn_00054d60();
    setup_help_indices();
    setup_aniseq_indices();
    setup_indices_for_composite_help(D_0016037c, statvar.f4df, 0x1f);
    setup_indices_for_composite_help(D_00160374, statvar.f51d, 0x13);
    setup_indices_for_composite_help(D_00160320, statvar.f543, 0xd);
    setup_indices_for_composite_help(D_00160398, statvar.f55b, 0xb);
    setup_indices_for_composite_help(D_0016039c, statvar.f56f, 4);
    InitPixBuffer(0, 0, 0xb4, 0xb4, 0xfe, 1);
    init_midi();
    init_ambients();
    gamecounts.unknown14 = LbTimerClock();
    myplayer.f1e = 0x19;
    init_wibble(D_00156c04, D_00157624, pixBuf.width + 0x20, pixBuf.width + 0x20, pixBuf.width, 0x11, 2);
    measure_objects();
    choice_backup.mode = 0;
    current_text.text[0] = 0;
    if (debug.unknown1fa) {
        if (fn_000cd716(myplayer.unknown0) == 0)
            fn_000bbff0(myplayer.unknown0, D_001602a0[1]);
        fn_00090804((u16)level_number, 1);
        load_player_details();
        othervar.f24 = D_00153f34;
        if (!D_0015f99f) {
            D_001549d0[player].type = 0x1e;
            D_001549d0[player].value = debug.unknown10a;
        } else {
            D_001549d0[player].type = 0x1d;
        }
    } else {
        D_001549d0[player].type = 0x1d;
    }
}

// FUNCTION: 0x00090804
void fn_00090804(int level, u8 reload_balance)
{
    u32 i;
    u8 saved_fa75;
    AllHosps backup;

    myhosp = &allhosps[player];
    if (fn_000cd716(myplayer.unknown0) == 0)
        fn_000bbff0(myplayer.unknown0, D_001602a0[1]);
    fn_000bbff0((char *)myhosp->unknown0, myplayer.unknown0);
    myhosp->f2a = 1;
    if ((u16)level > 14)
        myhosp->f29 = 1;
    else
        myhosp->f29 = level;
    if (reload_balance)
        load_balance(level);
    copy_my_hosp(&backup, myhosp);
    saved_fa75 = othervar.fa75;
    myplayer.f9 += mycontrol->f648;
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
        LbMemorySet(advisor.f0_u8a, 0, 0x133);
    othervar.fa75 = saved_fa75;
    for (i = 0; i < 9; i++)
        popup_id[i] = 1;
    mycontrol->unknown64f = 1;
    copy_my_hosp(myhosp, &backup);
    game_turn = 0;
    actual_game_turn = 1;
    safeseed = 0x546b2;
    seed = safeseed;
    load_level(level, 1);
    free_level();
    clear_sound_heap();
    if (towns[myhosp->f29].unknown4[2] == 1) {
        gamevar.f174 = 1;
        setup_rat_fest_panel();
        rat_fest_builder(towns[myhosp->f29].unknown4[1]);
    } else if (towns[myhosp->f29].unknown4[2] == 2) {
        training_builder(towns[myhosp->f29].unknown4[1]);
    }
    create_job_market();
    for (i = 0; i < 4; i++)
        init_player(i);
    othervar.f10 = 1;
    othervar.f33 = 1;
    calc_visitors_this_month();
    adcounts.f0 = 1;
    mycontrol->f4b4 = myhosp->f1c3;
    mycontrol->f64c = 750;
    sprintf(statvar.f6c, "save/%s.h", &myplayer);
    setup_insurances();
    setup_computer_players();
    setup_next_emergency();
    setup_next_quake();
    reset_advisor();
    LbMemorySet(D_00161200, 0, 0x26);
    othervar.f8c = get_next_illness(1, 0);
    check_save_games();
    othervar.fa55 = 1;
    othervar.fa58 = 0x39;
    fn_0008bdd4();
    setup_key_mappos();
    setup_next_net_criteria(player);
    if (statvar.f5dd) {
        othervar.f9b9 = 1;
        if (myplayer.unknown11) {
            othervar.fa4f = 1;
            play_midi(LbTimerClock() % statvar.f5dd);
        }
    }
    LbMemorySet(&current_text, 0, 0x5e7);
    LbMemorySet(net_text, 0, 0xfa);
    othervar.f4a = 0;
    close_irequesters();
}

// FUNCTION: 0x00090c8c
void copy_my_hosp(AllHosps *dst, AllHosps *src)
{
    fn_000bbff0((char *)dst->unknown0, (char *)src->unknown0);
    dst->f29 = src->f29;
    dst->f2a = src->f2a;
    dst->f2d = src->f2d;
}

// FUNCTION: 0x00090cd8
void load_balance(const u16 level)
{
    char path[0x90];
    char level_path[0x90];

    if (fn_000cd716(balance_id) == 0)
        fn_000bbff0(balance_id, "FULL");
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

// FUNCTION: 0x00090dd4
void balance_control(void)
{
    u16 i;
    u16 kind;
    u16 count;
    u8 all;

    count = 0;
    all = 1;
    statvar.f8 = 0;
    for (i = 0; i < 14; i++) {
        if (visuals[i]) {
            D_00154834[count++] = i + 2;
            kind = expertise[i + 2].kind;
            objects[kind].unknown1c = 1;
        }
        statvar.f8 += visuals[i];
        visuals[i] = statvar.f8;
    }
    for (i = 0; i < 20; i++) {
        if (non_visuals[i]) {
            all = 0;
            break;
        }
    }
    statvar.fa = 0;
    for (i = 0; i < 20; i++) {
        if (all)
            non_visuals[i] = 1;
        D_00154834[count++] = i + 16;
        statvar.fa += non_visuals[i];
        non_visuals[i] = statvar.fa;
    }
    expertise[1].unknown10[8] = 1;
    statvar.f6a = count;
}

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
        if (statvar.unknownfd > 1) {
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

// FUNCTION: 0x000910a0
void init_player(u8 owner)
{
    AllHosps *hosp;
    u16 i;

    hosp = &allhosps[owner];
    hosp->f2b = 2;
    hosp->f35 = towns[myhosp->f29].f0;
    allhosps[owner].rooms[0].unknown1C = 1;
    for (i = 0; i < 0x3e; i++) {
        if (objects[i].unknown1c) {
            hosp->object_stats[i].unknownC[0] = objects[i].unknown6[1];
            hosp->object_stats[i].available = objects[i].fc;
        }
        hosp->object_stats[i].cost = objects[i].cost;
        hosp->object_stats[i].unknown8 = objects[i].unknown1d;
    }
    hosp->fc94 = 5;
    for (i = 0; i < 0x2f; i++) {
        hosp->rooms[i].unknownC = expertise[i].f2;
        hosp->rooms[i].unknown10 = 10000;
        hosp->rooms[i].unknown1D = expertise[i].unknown10[8];
        setup_drug((u16)owner, i);
        hosp->rooms[i].f4 = 20;
        hosp->rooms[i].f8 = 20;
        evaluate_reputation(i, owner);
    }
    for (i = 0; i < 5; i++)
        hosp->research_alloc[i] = 20;
    count_hospital_area((u16)owner);
    if (myhosp->f29 == 1 || statvar.unknownfd > 1 || hosp->f2d == 0)
        hosp->f2d = 10000;
    for (i = 0; i < 16; i++)
        gamevar.f14b[i] = 0;
    for (i = 0; i < 20; i++)
        gamevar.f15b[i] = 0;
    if (statvar.unknownfd > 1)
        hosp->f1cb = 100 / network_num_players();
    else
        hosp->f1cb = 25;
    hosp->f217 = 1000;
    hosp->f219 = 90;
    hosp->f21b = 10;
    hosp->f21d = gbv.fe8;
    hosp->f21f = 1;
    hosp->f216 = 7;
    hosp->unknown20f[6] = 1;
    if (!(!gbv.f154 || statvar.unknownfd > 1 || gamevar.f174)) {
        if (statvar.unknownfd <= 1)
            hosp->unknown1256 = 0;
        hosp->unknown1257 = gbv.f154;
    } else {
        if (statvar.unknownfd <= 1)
            hosp->unknown1256 = 1;
    }
    othervar.fa6b = 0;
    hosp->f1c3 = hosp->f35;
    hosp->f41 = 15000;
    hosp->f31 = hosp->f2d;
    hosp->fc82 = 20;
    hosp->fc86 = 20;
    evaluate_reputation(0, owner);
    calc_pricing_factor(owner);
}

// FUNCTION: 0x000914bc
void init_mcga_coords(void)
{
}

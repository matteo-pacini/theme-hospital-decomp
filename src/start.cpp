#include "hosp.h"

extern "C" {
extern char **D_001602a0;
extern u8 D_00155e2d;
extern u8 D_001553b8[];
extern u8 D_001581fc[];
extern u8 D_001553b4;
extern u8 D_001553c8;
extern u8 D_001553eb;
extern char D_0012096c[];
extern u8 D_00161200[];
extern u16 D_00155444;
extern u8 D_00155e0d;
extern u8 D_00155e10;
extern u8 D_0015642d;
extern u8 D_00155d71;
extern u8 D_00155e07;
extern u8 D_00155402;
extern char D_00120976[];
extern char D_0012097b[];
extern char D_0012098d[];
extern char D_001209a2[];
extern char D_001209b2[];
extern u16 D_0012b8c8[];
extern u16 D_0012b8e4[];
extern u16 D_00154834[];
extern u16 D_0012ddd4;
extern u8 D_0015538b[];
extern u8 D_0015539b[];
extern u8 D_00155e23;

u32 fn_000cd716(char *s);
#pragma aux fn_000cd716 "W?GetCurrentPlayersCallback$n(pn$TbNetworkCallbackData$$pnv)v"
void fn_0008bdd4(void);
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

// NONMATCHING: register choice
// FUNCTION: 0x00090804
void fn_00090804(int level, u8 flag)
{
    u32 i;
    u8 saved;
    AllHosps backup;

    myhosp = &allhosps[player];
    if (fn_000cd716(myplayer.unknown0) == 0)
        LbDataLoadSetModifyFilenameFunction(myplayer.unknown0, D_001602a0[1]);
    LbDataLoadSetModifyFilenameFunction((char *)myhosp->unknown0, myplayer.unknown0);
    myhosp->f2a = 1;
    if ((u16)level > 14)
        myhosp->f29 = 1;
    else
        myhosp->f29 = level;
    if (flag)
        load_balance(level);
    copy_my_hosp(&backup, myhosp);
    saved = D_00155e2d;
    myplayer.f9 += mycontrol->f648;
    memset(thing, 0, 0x40164);
    memset(map, 0, 0x20000);
    memset(room_info, 0, 0xdac);
    memset(pagers, 0, 0x14a);
    memset(allhosps, 0, 0x4964);
    memset(mycontrol, 0, 0x651);
    memset(listhead, 0, 0x3a8);
    memset(D_001553b8, 0, 0xa96);
    memset(&gamevar, 0, 0x178);
    memset(playercall, 0, 0x258);
    memset(&current_net_criteria, 0, 0xc);
    for (i = 0; i < 20; i++)
        net_criteria[i].done = 0;
    LbMemorySet(&adcounts, 0, 0xa6);
    LbMemorySet(&NRAdcounts, 0, 0x11);
    if ((u16)level != 1)
        LbMemorySet(D_001581fc, 0, 0x133);
    D_00155e2d = saved;
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
        D_001553b4 = 1;
        setup_rat_fest_panel();
        rat_fest_builder(towns[myhosp->f29].unknown4[1]);
    } else if (towns[myhosp->f29].unknown4[2] == 2) {
        training_builder(towns[myhosp->f29].unknown4[1]);
    }
    create_job_market();
    for (i = 0; i < 4; i++)
        init_player(i);
    D_001553c8 = 1;
    D_001553eb = 1;
    calc_visitors_this_month();
    adcounts.f0 = 1;
    mycontrol->f4b4 = myhosp->f1c3;
    mycontrol->f64c = 750;
    sprintf(statvar.f6c, D_0012096c, &myplayer);
    setup_insurances();
    setup_computer_players();
    setup_next_emergency();
    setup_next_quake();
    reset_advisor();
    LbMemorySet(D_00161200, 0, 0x26);
    D_00155444 = get_next_illness(1, 0);
    check_save_games();
    D_00155e0d = 1;
    D_00155e10 = 0x39;
    fn_0008bdd4();
    setup_key_mappos();
    setup_next_net_criteria(player);
    if (D_0015642d) {
        D_00155d71 = 1;
        if (myplayer.unknown11) {
            D_00155e07 = 1;
            play_midi((s32)LbTimerClock() % D_0015642d);
        }
    }
    LbMemorySet(&current_text, 0, 0x5e7);
    LbMemorySet(net_text, 0, 0xfa);
    D_00155402 = 0;
    close_irequesters();
}

// FUNCTION: 0x00090c8c
void copy_my_hosp(AllHosps *dst, AllHosps *src)
{
    LbDataLoadSetModifyFilenameFunction((char *)dst->unknown0, (char *)src->unknown0);
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
        LbDataLoadSetModifyFilenameFunction(balance_id, D_00120976);
    if (install_type < 3) {
        sprintf(path, D_0012097b, cd_install_path, balance_id);
        check_cd_available(path);
        sprintf(level_path, D_0012098d, cd_install_path, balance_id, level);
        check_cd_available(path);
    } else {
        sprintf(path, D_001209a2, balance_id);
        sprintf(level_path, D_001209b2, balance_id, level);
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
        if (D_0012b8c8[i]) {
            D_00154834[count++] = i + 2;
            kind = expertise[i + 2].kind;
            objects[kind].unknown1c = 1;
        }
        statvar.f8 += D_0012b8c8[i];
        D_0012b8c8[i] = statvar.f8;
    }
    for (i = 0; i < 20; i++) {
        if (D_0012b8e4[i]) {
            all = 0;
            break;
        }
    }
    statvar.fa = 0;
    for (i = 0; i < 20; i++) {
        if (all)
            D_0012b8e4[i] = 1;
        D_00154834[count++] = i + 16;
        statvar.fa += D_0012b8e4[i];
        D_0012b8e4[i] = statvar.fa;
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
            pick = (s32)LbTimerClock() % left;
            ins_control[i] = pool[pick];
        }
        left--;
        for (j = pick; j < left; j++)
            pool[j] = pool[j + 1];
    }
}

// FUNCTION: 0x000910a0
void init_player(u8 p)
{
    AllHosps *h;
    u16 i;

    h = &allhosps[p];
    h->f2b = 2;
    h->f35 = towns[myhosp->f29].f0;
    allhosps[p].rooms[0].unknown1C = 1;
    for (i = 0; i < 0x3e; i++) {
        if (objects[i].unknown1c) {
            h->object_stats[i].unknownC[0] = objects[i].unknown6[1];
            h->object_stats[i].available = objects[i].fc;
        }
        h->object_stats[i].cost = objects[i].cost;
        h->object_stats[i].unknown8 = objects[i].unknown1d;
    }
    h->fc94 = 5;
    for (i = 0; i < 0x2f; i++) {
        h->rooms[i].unknownC = expertise[i].f2;
        h->rooms[i].unknown10 = 10000;
        h->rooms[i].unknown1D = expertise[i].unknown10[8];
        setup_drug((u16)p, i);
        h->rooms[i].f4 = 20;
        h->rooms[i].f8 = 20;
        evaluate_reputation(i, p);
    }
    for (i = 0; i < 5; i++)
        h->research_alloc[i] = 20;
    count_hospital_area((u16)p);
    if (myhosp->f29 == 1 || statvar.unknownfd > 1 || h->f2d == 0)
        h->f2d = 10000;
    for (i = 0; i < 16; i++)
        D_0015538b[i] = 0;
    for (i = 0; i < 20; i++)
        D_0015539b[i] = 0;
    if (statvar.unknownfd > 1)
        h->f1cb = 100 / network_num_players();
    else
        h->f1cb = 25;
    h->f217 = 1000;
    h->f219 = 90;
    h->f21b = 10;
    h->f21d = *(u16 *)&expertise[59].unknown7;
    h->f21f = 1;
    h->f216 = 7;
    h->unknown20f[6] = 1;
    if (!(!D_0012ddd4 || statvar.unknownfd > 1 || D_001553b4)) {
        if (statvar.unknownfd <= 1)
            h->unknown1256 = 0;
        h->unknown1257 = D_0012ddd4;
    } else {
        if (statvar.unknownfd <= 1)
            h->unknown1256 = 1;
    }
    D_00155e23 = 0;
    h->f1c3 = h->f35;
    h->f41 = 15000;
    h->f31 = h->f2d;
    h->fc82 = 20;
    h->fc86 = 20;
    evaluate_reputation(0, p);
    calc_pricing_factor(p);
}

// FUNCTION: 0x000914bc
void init_mcga_coords(void)
{
}

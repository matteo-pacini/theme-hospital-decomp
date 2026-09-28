#include "hosp.h"

struct Advice {
    u16 id;
    u16 f2;
    u16 f4;
    u8 f6;
    u8 f7;
    u32 time;
    u32 fc;
};

struct AdvisorQueue {
    s16 fe;
    s16 f200;
};

struct AdvisorTimer {
    s16 f208;
    s16 f20a;
};

extern Advice advice[];
extern WinLose win_criteria[];
extern WinLose lose_criteria[];
extern char **advisor_text;
extern const u8 staff_unhappy[];
extern const u16 advisor_anims[];

s8 tutorial(void);
void advisor_character(void);
void advisor_text_box(void);
u8 find_advice(const u16 id);
void reset_all_count(void);
u8 is_relevant_criterion(const u8 criterion, const WinLose *const table);
void mouse_advice(void);
void forced_advice(void);
void done_announcement(const u16 type);
u8 advisor_toilets(void);
u8 advisor_plants(void);
u8 advisor_old_hardware(void);
u8 advisor_thirsty_peeps(void);
u8 advisor_peeps_unhappy(void);
u8 advisor_peeps_warmth(void);
u8 advisor_litter_problem(void);
u8 advisor_benches(void);
u8 advisor_dead_plants(void);
u8 advisor_staff_tired(void);
u8 advisor_staff_warmth(void);

extern "C" {
extern u8 D_001553b4;
extern u8 D_00154805;
extern u8 D_00154806;
extern u8 D_00154807;
extern u8 D_00154808;
extern u8 D_00154809;
extern u8 D_0015480b;
extern u8 D_0015480d;
extern u8 D_0015480e;
extern u8 D_0015480f;
extern u8 D_00154810;
extern u8 D_00154812;
extern u8 D_00154813;
extern u8 D_00154818;
extern u8 D_0015532d[];
extern u16 D_00155335;
extern u16 D_0015543f;
extern s8 D_00155e03;
extern u8 D_0015831e;
extern u8 D_0015831f;
extern u8 D_00158323;
extern u8 D_00158329;
extern u8 D_0015832a;
extern u8 D_0015832b;
extern char **D_0016029c;
extern char **D_001602a4;
extern char **D_001602d8;
extern char **D_00160370;
extern u8 D_00154811;
extern u8 D_00154814;
extern u8 D_00154815;
extern u8 D_00154816;
extern u8 D_00154817;
extern u8 D_00154819;
extern u8 D_0015480c;
extern u8 D_0015543e;
extern u8 D_00155441;
extern u8 D_00155e0c;
extern u8 D_00155e24[];
extern u8 D_00156910;
extern TbScreenMode D_0015f3f5;
extern s16 D_001581fc;
extern AdvisorQueue D_001581fe;
extern u16 D_00158202;
extern AdvisorTimer D_00158208;
extern char D_0015820c[];
extern u8 D_0015831a;
extern u8 D_0015831b;
extern u8 D_00158320;
extern u32 D_00158324;
extern u8 D_00158328;
extern u8 D_0015832c;
extern char **D_001602e8;
extern char **D_001602ec;
extern char D_00120480[];
extern u16 D_0012cdd4[];
extern u16 D_0012cdea[];
extern s16 D_0012dd7c[];
}

// FUNCTION: 0x0005dbb0
void display_advisor(void)
{
    u8 flag;
    u16 i;
    u16 elapsed;

    if (!D_001553b4) {
    if (!debug.unknownd7) {
    if (!debug.unknownd8[0]) {
    if (D_00154819) {
        if (D_0015831b & 1) {
            if (tutorial() && D_0015820c[0]) {
                D_001581fc = 1;
                advisor_character();
                if (D_0012cdea[D_0015831a]) {
                    advisor_text_box();
                    D_00156910 = 1;
                }
            } else {
                return;
            }
        } else {
            flag = 0;
            if (D_001581fc) {
                if (D_0015f3f5 != SCREEN_MODE_1 || (crequester.unknown2 == 0x63 || crequester.unknown2 == 5)) {
                    elapsed = actual_game_turn - advice[D_001581fc].time;
                    if (elapsed > 0x64 || D_00155e0c == 1)
                        flag = 1;
                    if (elapsed >= advice[D_001581fc].f2 || D_00155e0c == 1)
                        D_001581fc = 0;
                } else {
                    advice[D_001581fc].time = actual_game_turn - advice[D_001581fc].f2 / 4;
                }
            }
            if (D_001581fc == 0 || flag == 1) {
                for (i = 1; advice[i].id; i++) {
                    if (actual_game_turn > advice[i].time) {
                        if ((advice[i].f6 * advice[i].f4 << 10) / (actual_game_turn - advice[i].time) < 10) {
                            if (find_advice(i) == 1) {
                                D_00158320 = 1;
                                break;
                            }
                        }
                    }
                }
            }
            advisor_character();
            if (D_0012cdea[D_0015831a]) {
                if (D_001581fc && !D_00155e0c) {
                    advisor_text_box();
                    D_00156910 = 1;
                } else if (--D_00158208.f20a <= 0 || D_00155e0c) {
                    D_00155e0c = 0;
                    D_00158208.f20a = -1;
                    D_0015831a = D_0012cdd4[D_0015831a];
                    D_00158202 = start[advisor_anims[D_0015831a + 1]].frame;
                    D_00158328 = 0;
                } else if (!D_00158328) {
                    D_0015831a = D_0012cdd4[D_0015831a];
                    D_00158202 = start[advisor_anims[D_0015831a + 1]].frame;
                    D_00158328 = 1;
                    D_00155e0c = 0;
                }
            }
        }
    }
    }
    }
    }
}

// NONMATCHING: register choice; sar-trick reads of D_00155e03, D_00158208 and D_0015820c named after the preceding symbol in the target
// FUNCTION: 0x0005defc
u8 find_advice(const u16 id)
{
    u8 f7;
    u16 found;
    u32 extra;
    s32 crit;
    s32 tmp;
    s32 value;

    found = 0;
    extra = 0;
    f7 = 0;
    switch (advice[id].id) {
    case 0x6a:
        if (D_0015532d[2] == 2 && !D_00154812) {
            sprintf(D_0015820c, advisor_text[0x6a]);
            D_00154812 = 1;
            found = 1;
        }
        break;
    case 0x4d:
        if (NRAdcounts.unknowna[0] >= 8) {
            sprintf(D_0015820c, advisor_text[0x4d], NRAdcounts.unknowna[0] / 2);
            NRAdcounts.unknowna[0] = 0;
            found = 1;
        }
        break;
    case 0x63:
        if (NRAdcounts.unknowna[5]) {
            sprintf(D_0015820c, advisor_text[0x63], NRAdcounts.unknowna[0] / 2, NRAdcounts.unknowna[0] / 2);
            NRAdcounts.unknowna[5] = 0;
            found = 1;
        }
        break;
    case 0x5e:
        if (adcounts.unknown8e && D_0015832c) {
            if (adcounts.unknown8e == 1) {
                sprintf(D_0015820c, advisor_text[0x5e]);
                adcounts.unknown8e = 0;
                found = 1;
            }
        }
        break;
    case 0x48:
        crit = is_relevant_criterion(2, lose_criteria);
        if (crit && (statvar.unknownfd > 1 ? 1 : myhosp->unknown1256)) {
            tmp = is_relevant_criterion(2, win_criteria);
            if (tmp && myhosp->f35 - myhosp->f3d < win_criteria[tmp].value) {
                u8 i;

                for (i = 0; i < 4; i++) {
                    if (lose_criteria[crit].value + (i + 1) * fn_000cb2db(lose_criteria[crit].value) / 10 > myhosp->f35
                        && myhosp->f35 > lose_criteria[crit].value) {
                        sprintf(D_0015820c, extra_advice[0x1c + i], myhosp->f35 - lose_criteria[crit].value);
                        found = 1;
                        break;
                    }
                }
            }
        } else if (towns[myhosp->f29].f0 / 10 >= myhosp->f35 && myhosp->f3d == 0) {
            sprintf(D_0015820c, advisor_text[0x48]);
            found = 1;
        } else if (towns[myhosp->f29].f0 / 10 >= myhosp->f35 && myhosp->f3d != 0 && myhosp->f3d < myhosp->f41) {
            sprintf(D_0015820c, extra_advice[0xf]);
            found = 1;
        }
        break;
    case 0x41:
        crit = is_relevant_criterion(5, lose_criteria);
        if (crit && (statvar.unknownfd > 1 ? 1 : myhosp->unknown1256) && myhosp->f1ed && (D_00158324 & 0x80)) {
            if (lose_criteria[crit].value - myhosp->f1eb * 100 / myhosp->f1ed < 10) {
                sprintf(D_0015820c, extra_advice[0x3a]);
                found = 1;
            } else if (lose_criteria[crit].value - myhosp->f1eb * 100 / myhosp->f1ed < 0x19) {
                sprintf(D_0015820c, advisor_text[0x41], myhosp->f1ed * lose_criteria[crit].value / 100 - myhosp->f1eb);
                found = 1;
            }
            D_00158324 &= ~0x80;
        }
        break;
    case 0x42:
        crit = is_relevant_criterion(1, win_criteria);
        if (crit && (statvar.unknownfd > 1 ? 1 : myhosp->unknown1256) && mycontrol->unknown650 > 5) {
            if (myhosp->unknownc7e > win_criteria[crit].value) {
                sprintf(D_0015820c, extra_advice[0x3b], win_criteria[crit].value);
            } else if ((win_criteria[crit].value - myhosp->unknownc7e) * 100 / win_criteria[crit].value < 10) {
                sprintf(D_0015820c, extra_advice[0x3c], win_criteria[crit].value);
            } else {
                sprintf(D_0015820c, advisor_text[0x42], myhosp->unknownc7e - win_criteria[crit].value);
            }
            found = 1;
        }
        break;
    case 0x43:
        crit = is_relevant_criterion(6, win_criteria);
        if (crit && (statvar.unknownfd > 1 ? 1 : myhosp->unknown1256) && mycontrol->unknown650 > 6) {
            if (myhosp->f1bb > win_criteria[crit].value) {
                sprintf(D_0015820c, extra_advice[0x3d], win_criteria[crit].value);
                found = 1;
            } else if ((win_criteria[crit].value - myhosp->f1bb) * 100 / win_criteria[crit].value < 10) {
                sprintf(D_0015820c, extra_advice[0x3e], win_criteria[crit].value - myhosp->f1bb);
                found = 1;
            } else if (myhosp->f1bb < win_criteria[crit].value) {
                sprintf(D_0015820c, advisor_text[0x43], win_criteria[crit].value);
                found = 1;
            }
        }
        break;
    case 0x14:
        if (myhosp->f3d && myhosp->f3d * 3 < myhosp->f35) {
            sprintf(D_0015820c, extra_advice[0x38]);
            found = 1;
        } else {
            crit = is_relevant_criterion(2, win_criteria);
            if (crit && (statvar.unknownfd > 1 ? 1 : myhosp->unknown1256)) {
                if (myhosp->f35 - myhosp->f3d > win_criteria[crit].value && (D_00158324 & 0x20)) {
                    sprintf(D_0015820c, extra_advice[0x37], win_criteria[crit].value);
                    found = 1;
                } else if (myhosp->f35 - myhosp->f3d > win_criteria[crit].value - fn_000cb2db(win_criteria[crit].value) / 10
                           && myhosp->f35 - myhosp->f3d < win_criteria[crit].value && (D_00158324 & 0x20)) {
                    sprintf(D_0015820c, advisor_text[0x14], win_criteria[crit].value - (myhosp->f35 - myhosp->f3d));
                    found = 1;
                }
            }
        }
        break;
    case 0x88:
        crit = is_relevant_criterion(4, win_criteria);
        if (crit && (statvar.unknownfd > 1 ? 1 : myhosp->unknown1256) && (D_00158324 & 0x40)
            && myhosp->f1e9 > myhosp->f29 * 3) {
            if (myhosp->f1e9 > win_criteria[crit].value && adcounts.fa2 > 0x19 && D_0015832c) {
                sprintf(D_0015820c, extra_advice[0x39], win_criteria[crit].value);
                found = 1;
            } else if (myhosp->f1e9 < win_criteria[crit].value) {
                sprintf(D_0015820c, advisor_text[0x88], win_criteria[crit].value - myhosp->f1e9);
                found = 1;
            }
            D_00158324 &= ~0x40;
        }
        break;
    case 0x78:
        if (adcounts.f98 && D_0015832c) {
            sprintf(D_0015820c, advisor_text[0x78], D_0016029c[adcounts.f98]);
            found = 1;
        }
        break;
    case 3:
        if (adcounts.f8f > 0 && D_0015832c) {
            if (advisor_old_hardware())
                found = 1;
        }
        break;
    case 4:
        if (D_001581fe.fe) {
            forced_advice();
            found = 1;
        }
        break;
    case 5:
        if (D_001581fe.f200) {
            mouse_advice();
            found = 1;
        }
        break;
    case 6:
        break;
    case 7:
        if (adcounts.f0 > 3 && D_0015832c) {
            if (adcounts.f66 > 0 && adcounts.f9a == 0) {
                if ((adcounts.f52 << 8) / adcounts.f66 > (D_0012dd7c[6] << 8) / 2) {
                    sprintf(D_0015820c, advisor_text[7]);
                    found = 1;
                }
            }
        }
        break;
    case 8:
        if (myhosp->fc9e[1] >= 6 && adcounts.f9a > myhosp->fc9e[1] / 3 && D_0015832c) {
            sprintf(D_0015820c, advisor_text[8]);
            found = 1;
        }
        break;
    case 0x64:
        if (adcounts.f0 > 3 && D_0015832c) {
            if (adcounts.f62 > 0 && adcounts.f99 == 0) {
                if ((adcounts.f4e << 8) / adcounts.f62 > (D_0012dd7c[6] << 8) / 2) {
                    sprintf(D_0015820c, advisor_text[0x64]);
                    found = 1;
                }
            }
        }
        break;
    case 0x65:
        if (myhosp->fc9e[0] >= 4 && adcounts.f99 > myhosp->fc9e[0] / 4 && D_0015832c) {
            sprintf(D_0015820c, advisor_text[0x65]);
            found = 1;
        }
        break;
    case 9:
        if (mycontrol->unknown650 > 4 && (statvar.unknownfd > 1 ? 1 : myhosp->unknown1256) && mycontrol->f28c == 0
            && advice[id].f6 < 1) {
            sprintf(D_0015820c, advisor_text[9]);
            found = 1;
        }
        break;
    case 0xa:
        if (mycontrol->unknown650 > 4 && (statvar.unknownfd > 1 ? 1 : myhosp->unknown1256) && advice[id].f6 < 1
            && mycontrol->f3ac == 0 && mycontrol->f28c > 0 && myhosp->f1ed > D_00155441) {
            sprintf(D_0015820c, advisor_text[10]);
            found = 1;
        }
        break;
    case 0xb:
        if (adcounts.f0 > 5 && D_0015832c) {
            if ((adcounts.f4a << 8) / adcounts.f0 > (D_0012dd7c[6] << 9) / 4) {
                value = ((adcounts.f48 - adcounts.f4a) << 8) / D_0012dd7c[6];
                if (value * 10 < 0x100)
                    sprintf(D_0015820c, advisor_text[11]);
                else
                    sprintf(D_0015820c, extra_advice[0x32]);
                found = 1;
            }
        }
        break;
    case 0x8d:
        if (!D_00154813 && (D_00158324 & 8)) {
            sprintf(D_0015820c, advisor_situations[11]);
            D_00154813 = 1;
            D_00158324 &= ~8;
            found = 1;
        }
        break;
    case 0xd:
        if (D_00158324 & 8) {
            if (D_0015543f >= 4) {
                sprintf(D_0015820c, extra_advice[1]);
                found = 1;
            } else if (D_0015543f >= 8) {
                sprintf(D_0015820c, extra_advice[2]);
                found = 1;
            }
        }
        break;
    case 0xe:
        if (adcounts.f0 > 5 && D_0015832c) {
            if (advisor_peeps_unhappy())
                found = 1;
        }
        break;
    case 0xf:
        if (adcounts.f0 > 5 && D_0015832c) {
            if (advisor_thirsty_peeps())
                found = 1;
        }
        break;
    case 0x58:
        break;
    case 0x10:
        if (myhosp->fc9c > 3 && D_0015832c) {
            if (advisor_staff_tired()) {
            }
        }
        break;
    case 0x11:
        if (adcounts.f0 > 10 && D_0015832c) {
            if (advisor_peeps_warmth())
                found = 1;
        }
        break;
    case 0x12:
        if (myhosp->fc9c > 10 && D_0015832c) {
            if (advisor_staff_warmth())
                found = 1;
        }
        break;
    case 0x13:
        if (adcounts.f0 > 10 && D_0015832c) {
            if (advisor_toilets())
                found = 1;
        }
        break;
    case 0xc:
        value = gamevar.announce[1][0];
        if (value) {
            sprintf(D_0015820c, advisor_text[12]);
            done_announcement(1);
            found = 1;
        }
        break;
    case 0x59:
        value = gamevar.announce[11][0];
        if (value) {
            if (!D_0015480e) {
                sprintf(D_0015820c, advisor_situations[9]);
                D_0015480e = 1;
            } else {
                sprintf(D_0015820c, advisor_text[0x59]);
            }
            done_announcement(0xb);
            found = 1;
        }
        break;
    case 0x15:
        if (D_00158323) {
            if (!D_00154818) {
                sprintf(D_0015820c, advisor_situations[16]);
                D_00154818 = 1;
            } else {
                sprintf(D_0015820c, advisor_text[0x15]);
            }
            D_00158323 = 0;
            found = 1;
        }
        break;
    case 0x7a:
        value = gamevar.announce[2][0];
        if (value) {
            sprintf(D_0015820c, advisor_text[0x7a]);
            done_announcement(2);
            found = 1;
        }
        break;
    case 0x44:
        value = gamevar.announce[4][0];
        if (value) {
            if (value == 5)
                sprintf(D_0015820c, advisor_text[0x44], help_text[0x3f]);
            else if (value == 6)
                sprintf(D_0015820c, advisor_text[0x44], help_text[0x40]);
            else if (value == 8)
                sprintf(D_0015820c, advisor_text[0x44], help_text[0x41]);
            done_announcement(4);
            found = 1;
        }
        break;
    case 0x89:
        value = gamevar.announce[13][0];
        if (value) {
            sprintf(D_0015820c, advisor_text[0x89]);
            done_announcement(0xd);
            found = 1;
        }
        break;
    case 0x16:
        value = gamevar.announce[10][0];
        if (value) {
            sprintf(D_0015820c, advisor_text[0x16]);
            done_announcement(0xa);
            found = 1;
        }
        break;
    case 0x17:
        value = gamevar.announce[5][0];
        if (value) {
            sprintf(D_0015820c, advisor_text[0x17], D_0016029c[value]);
            done_announcement(5);
            found = 1;
        }
        break;
    case 0x18:
        value = gamevar.announce[6][0];
        if (value) {
            sprintf(D_0015820c, advisor_text[0x18], D_001602a4[value]);
            done_announcement(6);
            found = 1;
        }
        break;
    case 0x19:
        value = gamevar.announce[7][0];
        if (value) {
            sprintf(D_0015820c, advisor_text[0x19], D_001602a4[value]);
            done_announcement(7);
            found = 1;
        }
        break;
    case 0x1a:
        value = gamevar.announce[8][0];
        if (value) {
            sprintf(D_0015820c, advisor_text[0x1a], D_0016029c[value]);
            done_announcement(8);
            found = 1;
        }
        break;
    case 0x1b:
        value = gamevar.announce[9][0];
        if (value) {
            sprintf(D_0015820c, advisor_text[0x1b], D_0016029c[value]);
            done_announcement(9);
            found = 1;
        }
        break;
    case 0x79:
        value = gamevar.announce[3][0];
        if (value) {
            sprintf(D_0015820c, advisor_text[0x79], D_001602d8[value]);
            done_announcement(3);
            found = 1;
        }
        break;
    case 0x45:
        if (advice[id].f6 < 1 && myhosp->fcaa > 0 && myhosp->fc9e[2] == 0) {
            sprintf(D_0015820c, advisor_text[0x45]);
            found = 1;
        }
        break;
    case 0x46:
        if (myhosp->f1f1 > 0 && D_0015832c) {
            if (advisor_litter_problem())
                found = 1;
        }
        break;
    case 0x47:
        if (advice[id].f6 < 1 && myhosp->f1eb == 1) {
            sprintf(D_0015820c, advisor_text[0x47]);
            found = 1;
        }
        break;
    case 0x52:
        if (advice[id].f6 < 1 && myhosp->f1e9 == 1) {
            sprintf(D_0015820c, advisor_text[0x52]);
            found = 1;
        }
        break;
    case 0x5c:
        if (NRAdcounts.unknown0 > 0 && myhosp->fc9e[2] == 0) {
            sprintf(D_0015820c, advisor_text[0x5c]);
            found = 1;
        }
        break;
    case 0x5a:
        if (NRAdcounts.unknown0 > 2 && D_0015832c) {
            if (advisor_dead_plants())
                found = 1;
        }
        break;
    case 0x5b:
        if (myhosp->f1f1 && D_0015832c) {
            value = (NRAdcounts.unknown0 << 14) / myhosp->f1f1;
            if (value > 0x800) {
                sprintf(D_0015820c, advisor_text[0x5b]);
                found = 1;
            }
        }
        break;
    case 0x4a:
        if (adcounts.f0 > 8 && myhosp->f1f1 > 0 && D_0015832c) {
            if (advisor_plants())
                found = 1;
        }
        break;
    case 0x4b:
        if (adcounts.f0 > 5 && D_0015832c) {
            if (advisor_benches())
                found = 1;
        }
        break;
    case 0x49:
        if (myhosp->fc9c > myhosp->f29 + 5 && !listhead[player].rooms[0x19]) {
            sprintf(D_0015820c, advisor_text[0x49]);
            found = 1;
        }
        break;
    case 0x50:
        if (listhead[player].rooms[0x16] != 0 && adcounts.f20 == 0 && D_00155e03 == 0x16 && D_0015832c) {
            sprintf(D_0015820c, advisor_text[0x50]);
            found = 1;
            D_00155e03 = 0;
        }
        break;
    case 0x53:
        if (listhead[player].rooms[0x1c] != 0 && adcounts.f21 == 0 && D_00155e03 == 0x1c && D_0015832c) {
            if (!D_00154806) {
                sprintf(D_0015820c, advisor_situations[2]);
                D_00154806 = 1;
            } else {
                sprintf(D_0015820c, advisor_text[0x53]);
            }
            found = 1;
            D_00155e03 = 0;
        }
        break;
    case 0x66:
        what_is_required(0x13, player);
        if ((required.unknown0[0] & 0x20) && required.unknown6 != 10 && required.unknown4 == 2 && D_00155e03 == 10
            && required.unknown8) {
            if (!D_00154807) {
                sprintf(D_0015820c, advisor_situations[3]);
                D_00154807 = 1;
            } else {
                sprintf(D_0015820c, advisor_text[0x66]);
            }
            found = 1;
            D_00155e03 = 1;
        }
        break;
    case 0x54:
        what_is_required(0x13, player);
        if ((required.unknown0[0] & 0x20) && required.unknown6 != 10 && required.unknown4 == 2 && D_00155e03 == 10) {
            if (!D_00154807) {
                sprintf(D_0015820c, advisor_situations[3]);
                D_00154807 = 1;
            } else {
                sprintf(D_0015820c, advisor_text[0x54]);
            }
            found = 1;
            D_00155e03 = 1;
        }
        break;
    case 0x67:
        what_is_required(0x13, player);
        if ((required.unknown0[0] & 0x20) && required.unknown6 != 10 && required.unknown4 == 1 && D_00155e03 == 10
            && required.unknown8) {
            sprintf(D_0015820c, advisor_text[0x67]);
            found = 1;
        }
        break;
    case 0x55:
        what_is_required(0x13, player);
        if ((required.unknown0[0] & 0x20) && required.unknown6 != 10 && required.unknown4 == 1 && D_00155e03 == 10) {
            sprintf(D_0015820c, advisor_text[0x55]);
            found = 1;
        }
        break;
    case 0x56:
        what_is_required(0x13, player);
        if (!required.flags && required.unknown6 != 10 && required.unknown8 && D_00155e03 == 10) {
            sprintf(D_0015820c, advisor_text[0x56]);
            found = 1;
            D_00155e03 = 0;
        }
        break;
    case 0x57:
        what_is_required(0x13, player);
        if (!required.unknown8 && required.unknown9 && D_00155e03 == 9) {
            sprintf(D_0015820c, advisor_text[0x57]);
            found = 1;
            D_00155e03 = 0;
        }
        break;
    case 0x4f:
        what_is_required(5, player);
        if ((required.unknown0[0] & 4) && required.unknown6 != 0xb && D_00155e03 == 0xb) {
            sprintf(D_0015820c, advisor_text[0x4f]);
            found = 1;
            D_00155e03 = 0;
        }
        break;
    case 0x4e:
        what_is_required(4, player);
        if ((required.unknown0[0] & 0x40) && required.unknown6 != 8 && D_00155e03 == 8) {
            if (!D_00154808) {
                sprintf(D_0015820c, advisor_situations[4]);
                D_00154808 = 1;
            } else {
                sprintf(D_0015820c, advisor_text[0x4e]);
            }
            found = 1;
            D_00155e03 = 0;
        }
        break;
    case 0x5d:
        if (adcounts.f8f > 0 && myhosp->fc9e[2] == 0 && D_00155e03 > 0 && D_0015832c) {
            sprintf(D_0015820c, advisor_text[0x5d]);
            found = 1;
            D_00155e03 = 0;
        }
        break;
    case 0x4c:
        if (myhosp->f1eb > 4 && (D_00158324 & 0x80) && saferand() % 4 == 0) {
            if (saferand() % 2 == 0)
                sprintf(D_0015820c, advisor_text[0x4c], myhosp->f1eb);
            else
                sprintf(D_0015820c, extra_advice[0x25], myhosp->f1eb);
            D_00158324 &= ~0x80;
            found = 1;
        }
        break;
    case 0x51:
        if (myhosp->f1e9 > 4 && (D_00158324 & 0x40) && saferand() % 4 == 0) {
            if (saferand() % 2 == 0)
                sprintf(D_0015820c, advisor_text[0x51], myhosp->f1e9);
            else
                sprintf(D_0015820c, extra_advice[0x24], myhosp->f1e9);
            found = 1;
            D_00158324 &= ~0x40;
        }
        break;
    case 0x68:
        if (listhead[player].rooms[7] != 0 && myhosp->fc9e[1] == 0 && D_00155e03 == 7 && myhosp->f29 == 1) {
            sprintf(D_0015820c, advisor_text[0x68]);
            found = 1;
            D_00155e03 = 0;
        }
        break;
    case 0x69:
        if (listhead[player].rooms[4] != 0 && myhosp->fc9e[3] == 0 && myhosp->f29 == 1) {
            sprintf(D_0015820c, advisor_text[0x69]);
            found = 1;
            D_00155e03 = 0;
        }
        break;
    case 0x6b:
        if ((myhosp->f29 >= 2 || myhosp->f29 <= 5) && NRAdcounts.unknown10 > 2 && NRAdcounts.unknown10 < 7) {
            sprintf(D_0015820c, advisor_text[0x6b + saferand() % 3]);
            found = 1;
        }
        break;
    case 0x6e:
        if (!D_00154810 && D_00155335) {
            sprintf(D_0015820c, advisor_text[0x6e]);
            D_00154810 = 1;
            found = 1;
        }
        break;
    case 0x1c:
        if (!D_0015480f && (D_00158324 & 1)) {
            sprintf(D_0015820c, advisor_text[0x1c]);
            D_0015480f = 1;
            D_00158324 &= ~1;
            found = 1;
        }
        break;
    case 0x72:
        if (myhosp->f29 > 1 && mycontrol->unknown650 > 6 && D_0015832c) {
            tmp = 100 - adcounts.fa2;
            if (tmp < 0x32 && tmp > 0x19) {
                sprintf(D_0015820c, advisor_text[0x74]);
                found = 1;
            } else if (tmp < 0x19 && tmp > 5) {
                sprintf(D_0015820c, advisor_text[0x73]);
                found = 1;
            } else if (tmp < 5 && tmp > 0) {
                sprintf(D_0015820c, advisor_text[0x72]);
                found = 1;
            }
        }
        break;
    case 0x6f:
        if (myhosp->f29 > 1 && mycontrol->unknown650 > myhosp->f29 + 3 && D_0015832c) {
            tmp = 100 - adcounts.f9e;
            if (tmp < 0x32 && tmp > 0x19) {
                sprintf(D_0015820c, advisor_text[0x71]);
                found = 1;
            } else if (tmp < 0x19 && tmp > 5) {
                sprintf(D_0015820c, advisor_text[0x70]);
                found = 1;
            } else if (tmp < 5 && tmp > 0) {
                sprintf(D_0015820c, advisor_text[0x6f]);
                found = 1;
            }
        }
        break;
    case 0x75:
        if ((D_00158324 & 2) && D_00158208.f208 != 0) {
            sprintf(D_0015820c, advisor_text[0x75], D_001602a4[D_00158208.f208]);
            found = 1;
            D_00158324 &= ~2;
        }
        break;
    case 0x76:
        if (myhosp->fc8e > 0x96) {
            sprintf(D_0015820c, advisor_text[0x76]);
            found = 1;
        }
        break;
    case 0x77:
        if (myhosp->fc8e && myhosp->fc8e < 0x3c) {
            sprintf(D_0015820c, advisor_text[0x77]);
            found = 1;
        }
        break;
    case 0x7b:
        if (D_00158329) {
            if (D_0015832a == 1) {
                if (D_0015832b == 8) {
                    sprintf(D_0015820c, advisor_text[0x7d]);
                    found = 1;
                } else if (D_0015832b == 0x1c) {
                    sprintf(D_0015820c, advisor_text[0x7b]);
                    found = 1;
                } else if (D_0015832b == 10) {
                    sprintf(D_0015820c, advisor_text[0x7c]);
                    found = 1;
                } else if (D_0015832b != 0x1d) {
                    sprintf(D_0015820c, advisor_text[0x7e + saferand() % 2], D_00160370[rooms[D_0015832b].id]);
                    found = 1;
                }
            } else if (D_0015832a == 0) {
                if (D_0015832b != 0x1d) {
                    sprintf(D_0015820c, advisor_text[0x80 + saferand() % 2], D_00160370[rooms[D_0015832b].id]);
                    found = 1;
                }
            } else if (D_0015832a == 3) {
                sprintf(D_0015820c, advisor_text[0x82]);
                found = 1;
            }
            D_00158329 = 0;
            D_0015832a = 0;
            D_0015832b = 0;
        }
        break;
    case 0x86:
        value = gamevar.announce[12][0];
        if (value) {
            sprintf(D_0015820c, advisor_text[0x86], allhosps[value].unknown0);
            done_announcement(0xc);
            found = 1;
        }
        break;
    case 0x87:
        if (!D_0015480d && (D_00158324 & 4)) {
            sprintf(D_0015820c, advisor_text[0x87]);
            D_0015480d = 1;
            D_00158324 &= ~4;
            found = 1;
        }
        crit = D_0015480d;
        tmp = 8;
        break;
    case 0x8a:
        if (!D_0015480b && (D_00158324 & 0x100)) {
            sprintf(D_0015820c, advisor_situations[7]);
            D_0015480b = 1;
            D_00158324 &= ~0x100;
            found = 1;
        }
    case 0x8b:
        if (!D_00154809 && myhosp->fcae > 0) {
            sprintf(D_0015820c, advisor_situations[5]);
            D_00154809 = 1;
            found = 1;
        }
        break;
    case 0x8c:
        if (!D_00154805 && D_00155e03 == 0x1c) {
            sprintf(D_0015820c, advisor_situations[1]);
            D_00154805 = 1;
            found = 1;
            D_00155e03 = 0;
        }
        break;
    default:
        break;
    }
    if (found == 1 && advice[id].id && (s8)D_0015820c[0] != 0) {
        D_001581fe.f200 = 0;
        D_0015831e = 0;
        D_0015831f = 0;
        advice[id].f6++;
        advice[id].time = actual_game_turn;
        advice[id].f7 = f7;
        advice[id].fc = extra;
        D_001581fc = id;
        D_00158208.f20a = advice[id].f2 / 2;
    }
    return found;
}

// NONMATCHING: register choice, frame size
// FUNCTION: 0x000608bc
void mouse_advice(void)
{
    switch (D_001581fe.f200) {
    default:
        LbDataLoadSetModifyFilenameFunction(D_0015820c, D_001602ec[D_001581fe.f200]);
        break;
    }
}

// FUNCTION: 0x00060904
void forced_advice(void)
{
    u8 i;

    switch (D_001581fe.fe) {
    case 2:
        LbDataLoadSetModifyFilenameFunction(D_0015820c, D_001602e8[D_001581fe.fe]);
        for (i = 0; i < 4; i++) {
            if (D_00155e24[i]) {
                sprintf(D_0015820c, D_00120480, D_0015820c, allhosps[i].unknown0);
                D_00155e24[i] = 0;
            }
        }
        break;
    case 8:
        if (D_0015480c == 0) {
            sprintf(D_0015820c, advisor_situations[8]);
            D_0015480c = 1;
        } else {
            sprintf(D_0015820c, D_001602e8[D_001581fe.fe], mayor_names[allhosps[player].f1f9]);
        }
        break;
    default:
        LbDataLoadSetModifyFilenameFunction(D_0015820c, D_001602e8[D_001581fe.fe]);
        break;
    }
    D_001581fc = 0;
    D_001581fe.fe = 0;
}

// FUNCTION: 0x00060a80
void reset_advisor(void)
{
    u16 i;

    for (i = 1; advice[i].id; i++) {
        advice[i].f6 = 0;
        advice[i].f7 = 0;
        advice[i].time = 0;
        advice[i].fc = 0;
    }
    D_001581fc = 0;
    D_00158208.f20a = 0;
    D_00158208.f208 = 0;
    D_001581fe.fe = 0;
    D_001581fe.f200 = 0;
    D_0015820c[0] = 0;
    D_0015543e = 1;
    D_0015832c = 0;
    D_00158324 ^= D_00158324;
    reset_all_count();
}

// FUNCTION: 0x00060b54
void announce_this(const u8 type, const u32 value)
{
    u8 i;

    for (i = 0; i < 4; i++) {
        if (gamevar.announce[type][i] == 0) {
            gamevar.announce[type][i] = value;
            break;
        }
        if (gamevar.announce[type][i] == value)
            break;
    }
}

// FUNCTION: 0x00060bf4
void done_announcement(const u16 type)
{
    u32 i;

    for (i = 0; i < 3; i++)
        gamevar.announce[type][i] = gamevar.announce[type][i + 1];
    gamevar.announce[type][3] = 0;
    D_00158320 = 1;
}

// NONMATCHING: register choice
// FUNCTION: 0x00060c78
u8 advisor_toilets(void)
{
    s32 ratio;

    ratio = adcounts.f1c / adcounts.f0;
    if (ratio * 4 > D_0012dd7c[13] * 3 && listhead[player].objects[0x33] == 0) {
        sprintf(D_0015820c, advisor_text[0x13]);
        return 1;
    }
    if (D_0012dd7c[13] < ratio && listhead[player].objects[0x33] == 0) {
        sprintf(D_0015820c, extra_advice[10]);
        return 1;
    }
    if (D_0012dd7c[13] < ratio && listhead[player].objects[0x33] != 0) {
        sprintf(D_0015820c, extra_advice[11]);
        return 1;
    }
    if (D_0012dd7c[14] < ratio && listhead[player].objects[0x33] == 0) {
        sprintf(D_0015820c, extra_advice[0x26]);
        return 1;
    }
    if (D_0012dd7c[14] < ratio) {
        sprintf(D_0015820c, extra_advice[12]);
        return 1;
    }
    return 0;
}

// FUNCTION: 0x00060e18
u8 advisor_plants(void)
{
    s32 per_area;
    s32 need;
    s32 unhappy;

    per_area = (NRAdcounts.unknown0 << 14) / myhosp->f1f1;
    need = (adcounts.f48 << 6) / (adcounts.f0 * D_0012dd7c[6]);
    unhappy = (adcounts.fa << 10) / adcounts.f0;
    if (need > 0x30) {
        if (per_area < 0x100) {
            sprintf(D_0015820c, extra_advice[0x14]);
            return 1;
        } else if (per_area < 0x200) {
            sprintf(D_0015820c, extra_advice[0x15]);
            return 1;
        }
    } else if ((D_0012dd7c[10] << 10) > unhappy) {
        if (per_area < 0x100) {
            sprintf(D_0015820c, extra_advice[0x16]);
            return 1;
        } else if (per_area < 0x200) {
            sprintf(D_0015820c, extra_advice[0x17]);
            return 1;
        }
    } else if (per_area >= 0x400 && per_area <= 0x800) {
        sprintf(D_0015820c, advisor_text[0x4a]);
        return 1;
    }
    return 0;
}

// FUNCTION: 0x00060f90
u8 advisor_old_hardware(void)
{
    if (adcounts.f90 * 20 * adcounts.f8f > adcounts.f8f * 0x1300) {
        if (myhosp->fc9e[2] > 0) {
            sprintf(D_0015820c, advisor_text[3]);
            return 1;
        } else if (myhosp->fc9e[2] == 0) {
            sprintf(D_0015820c, extra_advice[0x21]);
            return 1;
        }
    } else if (adcounts.f90 * 5 * adcounts.f8f > adcounts.f8f * 0x400) {
        sprintf(D_0015820c, extra_advice[0x1f]);
        return 1;
    } else if (adcounts.f90 * 5 * adcounts.f8f > adcounts.f8f * 0x300) {
        if (myhosp->fc9e[2] > 0) {
            sprintf(D_0015820c, extra_advice[0x20]);
            return 1;
        } else if (myhosp->fc9e[2] == 0) {
            sprintf(D_0015820c, extra_advice[0x22]);
            return 1;
        }
    } else if (adcounts.f90 * 2 * adcounts.f8f > adcounts.f8f * 0x100) {
        if (!D_00154811) {
            sprintf(D_0015820c, advisor_situations[10]);
            D_00154811 = 1;
            return 1;
        } else if (myhosp->fc9e[2] > 0) {
            sprintf(D_0015820c, extra_advice[0x23]);
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x00061188
u8 advisor_thirsty_peeps(void)
{
    s32 thirst;

    thirst = adcounts.f10 / adcounts.f0;
    if (thirst < 0x1e) {
        if (!D_00154817) {
            sprintf(D_0015820c, advisor_situations[15]);
            D_00154817 = 1;
            return 1;
        } else if (listhead[player].objects[7] == 0) {
            sprintf(D_0015820c, advisor_text[15]);
            return 1;
        } else {
            sprintf(D_0015820c, extra_advice[4]);
            return 1;
        }
    } else if (thirst < 10) {
        if (listhead[player].objects[7] == 0) {
            sprintf(D_0015820c, advisor_text[5]);
            return 1;
        } else {
            sprintf(D_0015820c, extra_advice[0x33]);
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x000612b8
u8 advisor_peeps_unhappy(void)
{
    s32 happiness;

    happiness = (adcounts.fa << 10) / adcounts.f0;
    if (D_00155441) {
        if (D_00155441 > 1)
            sprintf(D_0015820c, extra_advice[7]);
        else
            sprintf(D_0015820c, extra_advice[0x31]);
        D_00155441 = 0;
        return 1;
    } else if ((D_0012dd7c[10] << 10) > happiness) {
        sprintf(D_0015820c, extra_advice[6]);
        return 1;
    } else if ((D_0012dd7c[9] << 10) > happiness) {
        sprintf(D_0015820c, advisor_text[14]);
        return 1;
    }
    return 0;
}

// FUNCTION: 0x000613a0
u8 advisor_peeps_warmth(void)
{
    s32 warmth;

    warmth = adcounts.f14 / adcounts.f0;
    if (warmth * 2 > 0xf0) {
        sprintf(D_0015820c, extra_advice[0x34]);
        play_ambient(0xd9);
        return 1;
    } else if (warmth > 0x50) {
        if (!D_00154816) {
            sprintf(D_0015820c, advisor_situations[14]);
            D_00154816 = 1;
        } else {
            sprintf(D_0015820c, advisor_text[0x11]);
        }
        play_ambient(0xd9);
        return 1;
    } else if (warmth < 10) {
        sprintf(D_0015820c, extra_advice[0x35]);
        play_ambient(0xd8);
        return 1;
    } else if (warmth < 0x14) {
        if (!D_00154814 && myhosp->fc92 == 0) {
            sprintf(D_0015820c, advisor_situations[12]);
            D_00154814 = 1;
        } else if (D_00154814 && !D_00154815 && myhosp->fc92 == 0) {
            sprintf(D_0015820c, advisor_situations[13]);
            D_00154815 = 1;
        } else {
            sprintf(D_0015820c, extra_advice[13]);
        }
        play_ambient(0xd8);
        return 1;
    }
    return 0;
}

// FUNCTION: 0x00061558
u8 advisor_litter_problem(void)
{
    s32 litter;

    litter = (myhosp->fcaa << 14) / myhosp->f1f1;
    if (litter > 0x600) {
        play_ambient(0xd7);
        if (myhosp->fc9e[2] > 0) {
            sprintf(D_0015820c, extra_advice[0x12]);
            return 1;
        } else if (myhosp->fc9e[2] == 0) {
            sprintf(D_0015820c, extra_advice[0x11]);
            return 1;
        }
    } else if (litter > 0x300) {
        sprintf(D_0015820c, extra_advice[0x10]);
        return 1;
    } else if (litter > 0x100) {
        if (myhosp->fc9e[2] > 1) {
            sprintf(D_0015820c, advisor_text[0x46]);
            return 1;
        } else if (myhosp->fc9e[2] == 0) {
            sprintf(D_0015820c, extra_advice[0x13]);
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x000616b4
u8 advisor_benches(void)
{
    s32 standing;

    standing = (NRAdcounts.unknown6 << 10) / adcounts.f0;
    if (standing > 0x1000) {
        sprintf(D_0015820c, extra_advice[0x1b]);
        return 1;
    } else if (standing > 0x400) {
        sprintf(D_0015820c, extra_advice[0x18]);
        return 1;
    } else if (NRAdcounts.unknown4 > myhosp->f29 + 8 && standing < -0x1400) {
        sprintf(D_0015820c, extra_advice[0x1a]);
        return 1;
    } else if (NRAdcounts.unknown4 > 8 && standing < -0x400) {
        sprintf(D_0015820c, extra_advice[0x19]);
        return 1;
    } else if (NRAdcounts.unknown4 > 4) {
        sprintf(D_0015820c, advisor_text[0x4b]);
        return 1;
    }
    return 0;
}

// FUNCTION: 0x000617e4
u8 advisor_dead_plants(void)
{
    if (NRAdcounts.unknownb * 0x190000 / NRAdcounts.unknown0 < 0xce4000) {
        if (myhosp->fc9e[2] == 0) {
            sprintf(D_0015820c, extra_advice[0x27]);
            return 1;
        } else if (myhosp->fc9e[2] > 1) {
            sprintf(D_0015820c, extra_advice[0x28]);
            return 1;
        }
    } else if (NRAdcounts.unknownb * 0x190000 / NRAdcounts.unknown0 < 0x2260000) {
        sprintf(D_0015820c, extra_advice[0x29]);
        return 1;
    } else if (NRAdcounts.unknownb * 0x190000 / NRAdcounts.unknown0 < 0x3c28000) {
        sprintf(D_0015820c, advisor_text[0x5a]);
        return 1;
    } else if (NRAdcounts.unknownb * 0x190000 / NRAdcounts.unknown0 > 0x51a4000) {
        sprintf(D_0015820c, extra_advice[0x2b]);
        return 1;
    } else if ((NRAdcounts.unknownb << 14) / NRAdcounts.unknown0 > 0x490c000) {
        sprintf(D_0015820c, extra_advice[0x2a]);
        return 1;
    }
    return 0;
}

// FUNCTION: 0x00061998
void monthly_update_of_adivsor_counts(void)
{
    advice[14].f6 = 0;
    advice[15].f6 = 0;
}

// FUNCTION: 0x000619bc
u8 advisor_staff_tired(void)
{
    s32 average;
    s32 tired;
    u8 i;
    u8 level;

    average = (adcounts.f2 << 8) / myhosp->fc9c;
    i = 0;
    level = 0;
    if ((*(u16 *)&expertise[59].category << 8) < average)
        level = 0;
    else if ((*(u16 *)&expertise[59].unknown7 << 8) < average)
        level = 0x10;
    else if ((*(u16 *)&expertise[59].unknown5[0] << 8) > average)
        level = 0x20;
    else
        level = 0x30;
    for (; i < 4; i++) {
        if (myhosp->fc9e[i]) {
            tired = (adcounts.f7a[i] << 8) / myhosp->fc9e[i];
            if ((*(u16 *)&expertise[59].category << 8) < tired) {
                if (((average - tired) << 8) / (myhosp->fc9c - myhosp->fc9e[i])
                    < (*(u16 *)&expertise[59].category << 8))
                    ++level;
                if (staff_unhappy[level])
                    return staff_unhappy[level];
            }
        }
    }
    return 0;
}

// FUNCTION: 0x00061b34
u8 advisor_staff_warmth(void)
{
    s32 warmth;

    warmth = adcounts.f16 / myhosp->fc9c;
    if (warmth > 0x50) {
        sprintf(D_0015820c, advisor_text[0x12]);
        play_ambient(0xd9);
        return 1;
    } else if (warmth < 0x14) {
        sprintf(D_0015820c, extra_advice[0x36]);
        play_ambient(0xd8);
        return 1;
    }
    return 0;
}

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
extern char **D_0016029c;
extern char **D_001602a4;
extern char **D_001602d8;
extern char **D_00160370;
extern u8 D_00156910;
extern char **D_001602e8;
extern char **D_001602ec;
extern char D_00120480[];
extern u16 D_0012cdd4[];
extern u16 D_0012cdea[];
}

// FUNCTION: 0x0005dbb0
void display_advisor(void)
{
    u8 flag;
    u16 i;
    u16 elapsed;

    if (!gamevar.f174) {
    if (!debug.unknownd7) {
    if (!debug.unknownd8[0]) {
    if (myplayer.f41[20]) {
        if (advisor.f11f & 1) {
            if (tutorial() && advisor.f10[0]) {
                advisor.f0 = 1;
                advisor_character();
                if (D_0012cdea[advisor.f11e]) {
                    advisor_text_box();
                    D_00156910 = 1;
                }
            } else {
                return;
            }
        } else {
            flag = 0;
            if (advisor.f0) {
                if (resvars.f1 != SCREEN_MODE_1 || (crequester.unknown2 == 0x63 || crequester.unknown2 == 5)) {
                    elapsed = actual_game_turn - advice[advisor.f0].time;
                    if (elapsed > 0x64 || othervar.fa54 == 1)
                        flag = 1;
                    if (elapsed >= advice[advisor.f0].f2 || othervar.fa54 == 1)
                        advisor.f0 = 0;
                } else {
                    advice[advisor.f0].time = actual_game_turn - advice[advisor.f0].f2 / 4;
                }
            }
            if (advisor.f0 == 0 || flag == 1) {
                for (i = 1; advice[i].id; i++) {
                    if (actual_game_turn > advice[i].time) {
                        if ((advice[i].f6 * advice[i].f4 << 10) / (actual_game_turn - advice[i].time) < 10) {
                            if (find_advice(i) == 1) {
                                advisor.f124 = 1;
                                break;
                            }
                        }
                    }
                }
            }
            advisor_character();
            if (D_0012cdea[advisor.f11e]) {
                if (advisor.f0 && !othervar.fa54) {
                    advisor_text_box();
                    D_00156910 = 1;
                } else if (--advisor.fe <= 0 || othervar.fa54) {
                    othervar.fa54 = 0;
                    advisor.fe = -1;
                    advisor.f11e = D_0012cdd4[advisor.f11e];
                    advisor.f6 = start[advisor_anims[advisor.f11e + 1]].frame;
                    advisor.f12c = 0;
                } else if (!advisor.f12c) {
                    advisor.f11e = D_0012cdd4[advisor.f11e];
                    advisor.f6 = start[advisor_anims[advisor.f11e + 1]].frame;
                    advisor.f12c = 1;
                    othervar.fa54 = 0;
                }
            }
        }
    }
    }
    }
    }
}

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
        if (gamevar.fef == 2 && !myplayer.f41[13]) {
            sprintf(advisor.f10, advisor_text[0x6a]);
            myplayer.f41[13] = 1;
            found = 1;
        }
        break;
    case 0x4d:
        if (NRAdcounts.unknowna[0] >= 8) {
            sprintf(advisor.f10, advisor_text[0x4d], NRAdcounts.unknowna[0] / 2);
            NRAdcounts.unknowna[0] = 0;
            found = 1;
        }
        break;
    case 0x63:
        if (NRAdcounts.unknowna[5]) {
            sprintf(advisor.f10, advisor_text[0x63], NRAdcounts.unknowna[0] / 2, NRAdcounts.unknowna[0] / 2);
            NRAdcounts.unknowna[5] = 0;
            found = 1;
        }
        break;
    case 0x5e:
        if (adcounts.unknown8e && advisor.f130) {
            if (adcounts.unknown8e == 1) {
                sprintf(advisor.f10, advisor_text[0x5e]);
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
                    if (myhosp->f35 < lose_criteria[crit].value + (i + 1) * fn_000cb2db(lose_criteria[crit].value) / 10
                        && myhosp->f35 > lose_criteria[crit].value) {
                        sprintf(advisor.f10, extra_advice[0x1c + i], myhosp->f35 - lose_criteria[crit].value);
                        found = 1;
                        break;
                    }
                }
            }
        } else if (towns[myhosp->f29].f0 / 10 >= myhosp->f35 && myhosp->f3d == 0) {
            sprintf(advisor.f10, advisor_text[0x48]);
            found = 1;
        } else if (towns[myhosp->f29].f0 / 10 >= myhosp->f35 && myhosp->f3d != 0 && myhosp->f3d < myhosp->f41) {
            sprintf(advisor.f10, extra_advice[0xf]);
            found = 1;
        }
        break;
    case 0x41:
        crit = is_relevant_criterion(5, lose_criteria);
        if (crit && (statvar.unknownfd > 1 ? 1 : myhosp->unknown1256) && myhosp->f1ed && (advisor.f128_u32 & 0x80)) {
            if (lose_criteria[crit].value - myhosp->f1eb * 100 / myhosp->f1ed < 10) {
                sprintf(advisor.f10, extra_advice[0x3a]);
                found = 1;
            } else if (lose_criteria[crit].value - myhosp->f1eb * 100 / myhosp->f1ed < 0x19) {
                sprintf(advisor.f10, advisor_text[0x41], myhosp->f1ed * lose_criteria[crit].value / 100 - myhosp->f1eb);
                found = 1;
            }
            advisor.f128_u32 &= ~0x80;
        }
        break;
    case 0x42:
        crit = is_relevant_criterion(1, win_criteria);
        if (crit && (statvar.unknownfd > 1 ? 1 : myhosp->unknown1256) && mycontrol->unknown650 > 5) {
            if (myhosp->unknownc7e > win_criteria[crit].value) {
                sprintf(advisor.f10, extra_advice[0x3b], win_criteria[crit].value);
            } else if ((win_criteria[crit].value - myhosp->unknownc7e) * 100 / win_criteria[crit].value < 10) {
                sprintf(advisor.f10, extra_advice[0x3c], win_criteria[crit].value);
            } else {
                sprintf(advisor.f10, advisor_text[0x42], myhosp->unknownc7e - win_criteria[crit].value);
            }
            found = 1;
        }
        break;
    case 0x43:
        crit = is_relevant_criterion(6, win_criteria);
        if (crit && (statvar.unknownfd > 1 ? 1 : myhosp->unknown1256) && mycontrol->unknown650 > 6) {
            if (myhosp->f1bb > win_criteria[crit].value) {
                sprintf(advisor.f10, extra_advice[0x3d], win_criteria[crit].value);
                found = 1;
            } else if ((win_criteria[crit].value - myhosp->f1bb) * 100 / win_criteria[crit].value < 10) {
                sprintf(advisor.f10, extra_advice[0x3e], win_criteria[crit].value - myhosp->f1bb);
                found = 1;
            } else if (myhosp->f1bb < win_criteria[crit].value) {
                sprintf(advisor.f10, advisor_text[0x43], win_criteria[crit].value);
                found = 1;
            }
        }
        break;
    case 0x14:
        if (myhosp->f3d && myhosp->f35 > myhosp->f3d * 3) {
            sprintf(advisor.f10, extra_advice[0x38]);
            found = 1;
        } else {
            crit = is_relevant_criterion(2, win_criteria);
            if (crit && (statvar.unknownfd > 1 ? 1 : myhosp->unknown1256)) {
                if (myhosp->f35 - myhosp->f3d > win_criteria[crit].value && (advisor.f128_u32 & 0x20)) {
                    sprintf(advisor.f10, extra_advice[0x37], win_criteria[crit].value);
                    found = 1;
                } else if (myhosp->f35 - myhosp->f3d > win_criteria[crit].value - fn_000cb2db(win_criteria[crit].value) / 10
                           && myhosp->f35 - myhosp->f3d < win_criteria[crit].value && (advisor.f128_u32 & 0x20)) {
                    sprintf(advisor.f10, advisor_text[0x14], win_criteria[crit].value - (myhosp->f35 - myhosp->f3d));
                    found = 1;
                }
            }
        }
        break;
    case 0x88:
        crit = is_relevant_criterion(4, win_criteria);
        if (crit && (statvar.unknownfd > 1 ? 1 : myhosp->unknown1256) && (advisor.f128_u32 & 0x40)
            && myhosp->f1e9 > myhosp->f29 * 3) {
            if (myhosp->f1e9 > win_criteria[crit].value && adcounts.fa2 > 0x19 && advisor.f130) {
                sprintf(advisor.f10, extra_advice[0x39], win_criteria[crit].value);
                found = 1;
            } else if (myhosp->f1e9 < win_criteria[crit].value) {
                sprintf(advisor.f10, advisor_text[0x88], win_criteria[crit].value - myhosp->f1e9);
                found = 1;
            }
            advisor.f128_u32 &= ~0x40;
        }
        break;
    case 0x78:
        if (adcounts.f98 && advisor.f130) {
            sprintf(advisor.f10, advisor_text[0x78], D_0016029c[adcounts.f98]);
            found = 1;
        }
        break;
    case 3:
        if (adcounts.f8f > 0 && advisor.f130) {
            if (advisor_old_hardware())
                found = 1;
        }
        break;
    case 4:
        if (advisor.f2_s16) {
            forced_advice();
            found = 1;
        }
        break;
    case 5:
        if (advisor.f4) {
            mouse_advice();
            found = 1;
        }
        break;
    case 6:
        break;
    case 7:
        if (adcounts.f0 > 3 && advisor.f130) {
            if (adcounts.f66 > 0 && adcounts.f9a == 0) {
                if ((adcounts.f52 << 8) / adcounts.f66 > (gbv.f108 << 8) / 2) {
                    sprintf(advisor.f10, advisor_text[7]);
                    found = 1;
                }
            }
        }
        break;
    case 8:
        if (myhosp->fc9e[1] >= 6 && adcounts.f9a > myhosp->fc9e[1] / 3 && advisor.f130) {
            sprintf(advisor.f10, advisor_text[8]);
            found = 1;
        }
        break;
    case 0x64:
        if (adcounts.f0 > 3 && advisor.f130) {
            if (adcounts.f62 > 0 && adcounts.f99 == 0) {
                if ((adcounts.f4e << 8) / adcounts.f62 > (gbv.f108 << 8) / 2) {
                    sprintf(advisor.f10, advisor_text[0x64]);
                    found = 1;
                }
            }
        }
        break;
    case 0x65:
        if (myhosp->fc9e[0] >= 4 && adcounts.f99 > myhosp->fc9e[0] / 4 && advisor.f130) {
            sprintf(advisor.f10, advisor_text[0x65]);
            found = 1;
        }
        break;
    case 9:
        if (mycontrol->unknown650 > 4 && (statvar.unknownfd > 1 ? 1 : myhosp->unknown1256) && mycontrol->f28c == 0
            && advice[id].f6 < 1) {
            sprintf(advisor.f10, advisor_text[9]);
            found = 1;
        }
        break;
    case 0xa:
        if (mycontrol->unknown650 > 4 && (statvar.unknownfd > 1 ? 1 : myhosp->unknown1256) && advice[id].f6 < 1
            && mycontrol->f3ac == 0 && mycontrol->f28c > 0 && myhosp->f1ed > othervar.f89) {
            sprintf(advisor.f10, advisor_text[10]);
            found = 1;
        }
        break;
    case 0xb:
        if (adcounts.f0 > 5 && advisor.f130) {
            if ((adcounts.f4a << 8) / adcounts.f0 > (gbv.f108 << 9) / 4) {
                value = ((adcounts.f48 - adcounts.f4a) << 8) / gbv.f108;
                if (value * 10 < 0x100)
                    sprintf(advisor.f10, advisor_text[11]);
                else
                    sprintf(advisor.f10, extra_advice[0x32]);
                found = 1;
            }
        }
        break;
    case 0x8d:
        if (!myplayer.f41[14] && (advisor.f128_u32 & 8)) {
            sprintf(advisor.f10, advisor_situations[11]);
            myplayer.f41[14] = 1;
            advisor.f128_u32 &= ~8;
            found = 1;
        }
        break;
    case 0xd:
        if (advisor.f128_u32 & 8) {
            if (othervar.f87 >= 4) {
                sprintf(advisor.f10, extra_advice[1]);
                found = 1;
            } else if (othervar.f87 >= 8) {
                sprintf(advisor.f10, extra_advice[2]);
                found = 1;
            }
        }
        break;
    case 0xe:
        if (adcounts.f0 > 5 && advisor.f130) {
            if (advisor_peeps_unhappy())
                found = 1;
        }
        break;
    case 0xf:
        if (adcounts.f0 > 5 && advisor.f130) {
            if (advisor_thirsty_peeps())
                found = 1;
        }
        break;
    case 0x58:
        break;
    case 0x10:
        if (myhosp->fc9c > 3 && advisor.f130) {
            if (advisor_staff_tired()) {
            }
        }
        break;
    case 0x11:
        if (adcounts.f0 > 10 && advisor.f130) {
            if (advisor_peeps_warmth())
                found = 1;
        }
        break;
    case 0x12:
        if (myhosp->fc9c > 10 && advisor.f130) {
            if (advisor_staff_warmth())
                found = 1;
        }
        break;
    case 0x13:
        if (adcounts.f0 > 10 && advisor.f130) {
            if (advisor_toilets())
                found = 1;
        }
        break;
    case 0xc:
        value = gamevar.announce[1][0];
        if (value) {
            sprintf(advisor.f10, advisor_text[12]);
            done_announcement(1);
            found = 1;
        }
        break;
    case 0x59:
        value = gamevar.announce[11][0];
        if (value) {
            if (!myplayer.f41[9]) {
                sprintf(advisor.f10, advisor_situations[9]);
                myplayer.f41[9] = 1;
            } else {
                sprintf(advisor.f10, advisor_text[0x59]);
            }
            done_announcement(0xb);
            found = 1;
        }
        break;
    case 0x15:
        if (advisor.f127) {
            if (!myplayer.f41[19]) {
                sprintf(advisor.f10, advisor_situations[16]);
                myplayer.f41[19] = 1;
            } else {
                sprintf(advisor.f10, advisor_text[0x15]);
            }
            advisor.f127 = 0;
            found = 1;
        }
        break;
    case 0x7a:
        value = gamevar.announce[2][0];
        if (value) {
            sprintf(advisor.f10, advisor_text[0x7a]);
            done_announcement(2);
            found = 1;
        }
        break;
    case 0x44:
        value = gamevar.announce[4][0];
        if (value) {
            if (value == 5)
                sprintf(advisor.f10, advisor_text[0x44], help_text[0x3f]);
            else if (value == 6)
                sprintf(advisor.f10, advisor_text[0x44], help_text[0x40]);
            else if (value == 8)
                sprintf(advisor.f10, advisor_text[0x44], help_text[0x41]);
            done_announcement(4);
            found = 1;
        }
        break;
    case 0x89:
        value = gamevar.announce[13][0];
        if (value) {
            sprintf(advisor.f10, advisor_text[0x89]);
            done_announcement(0xd);
            found = 1;
        }
        break;
    case 0x16:
        value = gamevar.announce[10][0];
        if (value) {
            sprintf(advisor.f10, advisor_text[0x16]);
            done_announcement(0xa);
            found = 1;
        }
        break;
    case 0x17:
        value = gamevar.announce[5][0];
        if (value) {
            sprintf(advisor.f10, advisor_text[0x17], D_0016029c[value]);
            done_announcement(5);
            found = 1;
        }
        break;
    case 0x18:
        value = gamevar.announce[6][0];
        if (value) {
            sprintf(advisor.f10, advisor_text[0x18], D_001602a4[value]);
            done_announcement(6);
            found = 1;
        }
        break;
    case 0x19:
        value = gamevar.announce[7][0];
        if (value) {
            sprintf(advisor.f10, advisor_text[0x19], D_001602a4[value]);
            done_announcement(7);
            found = 1;
        }
        break;
    case 0x1a:
        value = gamevar.announce[8][0];
        if (value) {
            sprintf(advisor.f10, advisor_text[0x1a], D_0016029c[value]);
            done_announcement(8);
            found = 1;
        }
        break;
    case 0x1b:
        value = gamevar.announce[9][0];
        if (value) {
            sprintf(advisor.f10, advisor_text[0x1b], D_0016029c[value]);
            done_announcement(9);
            found = 1;
        }
        break;
    case 0x79:
        value = gamevar.announce[3][0];
        if (value) {
            sprintf(advisor.f10, advisor_text[0x79], D_001602d8[value]);
            done_announcement(3);
            found = 1;
        }
        break;
    case 0x45:
        if (advice[id].f6 < 1 && myhosp->fcaa > 0 && myhosp->fc9e[2] == 0) {
            sprintf(advisor.f10, advisor_text[0x45]);
            found = 1;
        }
        break;
    case 0x46:
        if (myhosp->f1f1 > 0 && advisor.f130) {
            if (advisor_litter_problem())
                found = 1;
        }
        break;
    case 0x47:
        if (advice[id].f6 < 1 && myhosp->f1eb == 1) {
            sprintf(advisor.f10, advisor_text[0x47]);
            found = 1;
        }
        break;
    case 0x52:
        if (advice[id].f6 < 1 && myhosp->f1e9 == 1) {
            sprintf(advisor.f10, advisor_text[0x52]);
            found = 1;
        }
        break;
    case 0x5c:
        if (NRAdcounts.unknown0 > 0 && myhosp->fc9e[2] == 0) {
            sprintf(advisor.f10, advisor_text[0x5c]);
            found = 1;
        }
        break;
    case 0x5a:
        if (NRAdcounts.unknown0 > 2 && advisor.f130) {
            if (advisor_dead_plants())
                found = 1;
        }
        break;
    case 0x5b:
        if (myhosp->f1f1 && advisor.f130) {
            value = (NRAdcounts.unknown0 << 14) / myhosp->f1f1;
            if (value > 0x800) {
                sprintf(advisor.f10, advisor_text[0x5b]);
                found = 1;
            }
        }
        break;
    case 0x4a:
        if (adcounts.f0 > 8 && myhosp->f1f1 > 0 && advisor.f130) {
            if (advisor_plants())
                found = 1;
        }
        break;
    case 0x4b:
        if (adcounts.f0 > 5 && advisor.f130) {
            if (advisor_benches())
                found = 1;
        }
        break;
    case 0x49:
        if (myhosp->fc9c > myhosp->f29 + 5 && !listhead[player].rooms[0x19]) {
            sprintf(advisor.f10, advisor_text[0x49]);
            found = 1;
        }
        break;
    case 0x50:
        if (listhead[player].rooms[0x16] != 0 && adcounts.f20 == 0 && othervar.fa4b_s8 == 0x16 && advisor.f130) {
            sprintf(advisor.f10, advisor_text[0x50]);
            found = 1;
            othervar.fa4b_s8 = 0;
        }
        break;
    case 0x53:
        if (listhead[player].rooms[0x1c] != 0 && adcounts.f21 == 0 && othervar.fa4b_s8 == 0x1c && advisor.f130) {
            if (!myplayer.f41[1]) {
                sprintf(advisor.f10, advisor_situations[2]);
                myplayer.f41[1] = 1;
            } else {
                sprintf(advisor.f10, advisor_text[0x53]);
            }
            found = 1;
            othervar.fa4b_s8 = 0;
        }
        break;
    case 0x66:
        what_is_required(0x13, player);
        if ((required.unknown0[0] & 0x20) && required.unknown6 != 10 && required.unknown4 == 2 && othervar.fa4b_s8 == 10
            && required.unknown8) {
            if (!myplayer.f41[2]) {
                sprintf(advisor.f10, advisor_situations[3]);
                myplayer.f41[2] = 1;
            } else {
                sprintf(advisor.f10, advisor_text[0x66]);
            }
            found = 1;
            othervar.fa4b_s8 = 1;
        }
        break;
    case 0x54:
        what_is_required(0x13, player);
        if ((required.unknown0[0] & 0x20) && required.unknown6 != 10 && required.unknown4 == 2 && othervar.fa4b_s8 == 10) {
            if (!myplayer.f41[2]) {
                sprintf(advisor.f10, advisor_situations[3]);
                myplayer.f41[2] = 1;
            } else {
                sprintf(advisor.f10, advisor_text[0x54]);
            }
            found = 1;
            othervar.fa4b_s8 = 1;
        }
        break;
    case 0x67:
        what_is_required(0x13, player);
        if ((required.unknown0[0] & 0x20) && required.unknown6 != 10 && required.unknown4 == 1 && othervar.fa4b_s8 == 10
            && required.unknown8) {
            sprintf(advisor.f10, advisor_text[0x67]);
            found = 1;
        }
        break;
    case 0x55:
        what_is_required(0x13, player);
        if ((required.unknown0[0] & 0x20) && required.unknown6 != 10 && required.unknown4 == 1 && othervar.fa4b_s8 == 10) {
            sprintf(advisor.f10, advisor_text[0x55]);
            found = 1;
        }
        break;
    case 0x56:
        what_is_required(0x13, player);
        if (!required.flags && required.unknown6 != 10 && required.unknown8 && othervar.fa4b_s8 == 10) {
            sprintf(advisor.f10, advisor_text[0x56]);
            found = 1;
            othervar.fa4b_s8 = 0;
        }
        break;
    case 0x57:
        what_is_required(0x13, player);
        if (!required.unknown8 && required.unknown9 && othervar.fa4b_s8 == 9) {
            sprintf(advisor.f10, advisor_text[0x57]);
            found = 1;
            othervar.fa4b_s8 = 0;
        }
        break;
    case 0x4f:
        what_is_required(5, player);
        if ((required.unknown0[0] & 4) && required.unknown6 != 0xb && othervar.fa4b_s8 == 0xb) {
            sprintf(advisor.f10, advisor_text[0x4f]);
            found = 1;
            othervar.fa4b_s8 = 0;
        }
        break;
    case 0x4e:
        what_is_required(4, player);
        if ((required.unknown0[0] & 0x40) && required.unknown6 != 8 && othervar.fa4b_s8 == 8) {
            if (!myplayer.f41[3]) {
                sprintf(advisor.f10, advisor_situations[4]);
                myplayer.f41[3] = 1;
            } else {
                sprintf(advisor.f10, advisor_text[0x4e]);
            }
            found = 1;
            othervar.fa4b_s8 = 0;
        }
        break;
    case 0x5d:
        if (adcounts.f8f > 0 && myhosp->fc9e[2] == 0 && othervar.fa4b_s8 > 0 && advisor.f130) {
            sprintf(advisor.f10, advisor_text[0x5d]);
            found = 1;
            othervar.fa4b_s8 = 0;
        }
        break;
    case 0x4c:
        if (myhosp->f1eb > 4 && (advisor.f128_u32 & 0x80) && saferand() % 4 == 0) {
            if (saferand() % 2 == 0)
                sprintf(advisor.f10, advisor_text[0x4c], myhosp->f1eb);
            else
                sprintf(advisor.f10, extra_advice[0x25], myhosp->f1eb);
            advisor.f128_u32 &= ~0x80;
            found = 1;
        }
        break;
    case 0x51:
        if (myhosp->f1e9 > 4 && (advisor.f128_u32 & 0x40) && saferand() % 4 == 0) {
            if (saferand() % 2 == 0)
                sprintf(advisor.f10, advisor_text[0x51], myhosp->f1e9);
            else
                sprintf(advisor.f10, extra_advice[0x24], myhosp->f1e9);
            found = 1;
            advisor.f128_u32 &= ~0x40;
        }
        break;
    case 0x68:
        if (listhead[player].rooms[7] != 0 && myhosp->fc9e[1] == 0 && othervar.fa4b_s8 == 7 && myhosp->f29 == 1) {
            sprintf(advisor.f10, advisor_text[0x68]);
            found = 1;
            othervar.fa4b_s8 = 0;
        }
        break;
    case 0x69:
        if (listhead[player].rooms[4] != 0 && myhosp->fc9e[3] == 0 && myhosp->f29 == 1) {
            sprintf(advisor.f10, advisor_text[0x69]);
            found = 1;
            othervar.fa4b_s8 = 0;
        }
        break;
    case 0x6b:
        if ((myhosp->f29 >= 2 || myhosp->f29 <= 5) && NRAdcounts.unknown10 > 2 && NRAdcounts.unknown10 < 7) {
            sprintf(advisor.f10, advisor_text[0x6b + saferand() % 3]);
            found = 1;
        }
        break;
    case 0x6e:
        if (!myplayer.f41[11] && gamevar.ff5) {
            sprintf(advisor.f10, advisor_text[0x6e]);
            myplayer.f41[11] = 1;
            found = 1;
        }
        break;
    case 0x1c:
        if (!myplayer.f41[10] && (advisor.f128_u32 & 1)) {
            sprintf(advisor.f10, advisor_text[0x1c]);
            myplayer.f41[10] = 1;
            advisor.f128_u32 &= ~1;
            found = 1;
        }
        break;
    case 0x72:
        if (myhosp->f29 > 1 && mycontrol->unknown650 > 6 && advisor.f130) {
            tmp = 100 - adcounts.fa2;
            if (tmp < 0x32 && tmp > 0x19) {
                sprintf(advisor.f10, advisor_text[0x74]);
                found = 1;
            } else if (tmp < 0x19 && tmp > 5) {
                sprintf(advisor.f10, advisor_text[0x73]);
                found = 1;
            } else if (tmp < 5 && tmp > 0) {
                sprintf(advisor.f10, advisor_text[0x72]);
                found = 1;
            }
        }
        break;
    case 0x6f:
        if (myhosp->f29 > 1 && mycontrol->unknown650 > myhosp->f29 + 3 && advisor.f130) {
            tmp = 100 - adcounts.f9e;
            if (tmp < 0x32 && tmp > 0x19) {
                sprintf(advisor.f10, advisor_text[0x71]);
                found = 1;
            } else if (tmp < 0x19 && tmp > 5) {
                sprintf(advisor.f10, advisor_text[0x70]);
                found = 1;
            } else if (tmp < 5 && tmp > 0) {
                sprintf(advisor.f10, advisor_text[0x6f]);
                found = 1;
            }
        }
        break;
    case 0x75:
        if ((advisor.f128_u32 & 2) && advisor.fc != 0) {
            sprintf(advisor.f10, advisor_text[0x75], D_001602a4[advisor.fc]);
            found = 1;
            advisor.f128_u32 &= ~2;
        }
        break;
    case 0x76:
        if (myhosp->fc8e > 0x96) {
            sprintf(advisor.f10, advisor_text[0x76]);
            found = 1;
        }
        break;
    case 0x77:
        if (myhosp->fc8e && myhosp->fc8e < 0x3c) {
            sprintf(advisor.f10, advisor_text[0x77]);
            found = 1;
        }
        break;
    case 0x7b:
        if (advisor.f12d) {
            if (advisor.f12e == 1) {
                if (advisor.f12f == 8) {
                    sprintf(advisor.f10, advisor_text[0x7d]);
                    found = 1;
                } else if (advisor.f12f == 0x1c) {
                    sprintf(advisor.f10, advisor_text[0x7b]);
                    found = 1;
                } else if (advisor.f12f == 10) {
                    sprintf(advisor.f10, advisor_text[0x7c]);
                    found = 1;
                } else if (advisor.f12f != 0x1d) {
                    sprintf(advisor.f10, advisor_text[0x7e + saferand() % 2], D_00160370[rooms[advisor.f12f].id]);
                    found = 1;
                }
            } else if (advisor.f12e == 0) {
                if (advisor.f12f != 0x1d) {
                    sprintf(advisor.f10, advisor_text[0x80 + saferand() % 2], D_00160370[rooms[advisor.f12f].id]);
                    found = 1;
                }
            } else if (advisor.f12e == 3) {
                sprintf(advisor.f10, advisor_text[0x82]);
                found = 1;
            }
            advisor.f12d = 0;
            advisor.f12e = 0;
            advisor.f12f = 0;
        }
        break;
    case 0x86:
        value = gamevar.announce[12][0];
        if (value) {
            sprintf(advisor.f10, advisor_text[0x86], allhosps[value].unknown0);
            done_announcement(0xc);
            found = 1;
        }
        break;
    case 0x87:
        if (!myplayer.f41[8] && (advisor.f128_u32 & 4)) {
            sprintf(advisor.f10, advisor_text[0x87]);
            myplayer.f41[8] = 1;
            advisor.f128_u32 &= ~4;
            found = 1;
        }
        crit = myplayer.f41[8];
        tmp = 8;
        break;
    case 0x8a:
        if (!myplayer.f41[6] && (advisor.f128_u32 & 0x100)) {
            sprintf(advisor.f10, advisor_situations[7]);
            myplayer.f41[6] = 1;
            advisor.f128_u32 &= ~0x100;
            found = 1;
        }
    case 0x8b:
        if (!myplayer.f41[4] && myhosp->fcae > 0) {
            sprintf(advisor.f10, advisor_situations[5]);
            myplayer.f41[4] = 1;
            found = 1;
        }
        break;
    case 0x8c:
        if (!myplayer.f41[0] && othervar.fa4b_s8 == 0x1c) {
            sprintf(advisor.f10, advisor_situations[1]);
            myplayer.f41[0] = 1;
            found = 1;
            othervar.fa4b_s8 = 0;
        }
        break;
    default:
        break;
    }
    if (found == 1 && advice[id].id && (s8)advisor.f10[0] != 0) {
        advisor.f4 = 0;
        advisor.f122 = 0;
        advisor.f123 = 0;
        advice[id].f6++;
        advice[id].time = actual_game_turn;
        advice[id].f7 = f7;
        advice[id].fc = extra;
        advisor.f0 = id;
        advisor.fe = advice[id].f2 / 2;
    }
    return found;
}

// FUNCTION: 0x000608bc
void mouse_advice(void)
{
    s32 unused;

    switch (advisor.f4) {
    default:
        fn_000bbff0(advisor.f10, D_001602ec[advisor.f4]);
        break;
    }
}

// FUNCTION: 0x00060904
void forced_advice(void)
{
    u8 i;

    switch (advisor.f2_s16) {
    case 2:
        fn_000bbff0(advisor.f10, D_001602e8[advisor.f2_s16]);
        for (i = 0; i < 4; i++) {
            if (othervar.fa6c[i]) {
                sprintf(advisor.f10, D_00120480, advisor.f10, allhosps[i].unknown0);
                othervar.fa6c[i] = 0;
            }
        }
        break;
    case 8:
        if (myplayer.f41[7] == 0) {
            sprintf(advisor.f10, advisor_situations[8]);
            myplayer.f41[7] = 1;
        } else {
            sprintf(advisor.f10, D_001602e8[advisor.f2_s16], mayor_names[allhosps[player].f1f9]);
        }
        break;
    default:
        fn_000bbff0(advisor.f10, D_001602e8[advisor.f2_s16]);
        break;
    }
    advisor.f0 = 0;
    advisor.f2_s16 = 0;
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
    advisor.f0 = 0;
    advisor.fe = 0;
    advisor.fc = 0;
    advisor.f2_s16 = 0;
    advisor.f4 = 0;
    advisor.f10[0] = 0;
    othervar.f86 = 1;
    advisor.f130 = 0;
    advisor.f128_u32 ^= advisor.f128_u32;
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
    advisor.f124 = 1;
}

// FUNCTION: 0x00060c78
u8 advisor_toilets(void)
{
    s32 ratio;

    ratio = adcounts.f1c / adcounts.f0;
    if (ratio * 4 > gbv.f116 * 3 && listhead[player].objects[0x33] == 0) {
        sprintf(advisor.f10, advisor_text[0x13]);
        return 1;
    }
    if (gbv.f116 < ratio && listhead[player].objects[0x33] == 0) {
        sprintf(advisor.f10, extra_advice[10]);
        return 1;
    }
    if (gbv.f116 < ratio && listhead[player].objects[0x33] != 0) {
        sprintf(advisor.f10, extra_advice[11]);
        return 1;
    }
    if (gbv.f118 < ratio && listhead[player].objects[0x33] == 0) {
        sprintf(advisor.f10, extra_advice[0x26]);
        return 1;
    }
    if (gbv.f118 < ratio) {
        sprintf(advisor.f10, extra_advice[12]);
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
    need = (adcounts.f48 << 6) / (adcounts.f0 * gbv.f108);
    unhappy = (adcounts.fa << 10) / adcounts.f0;
    if (need > 0x30) {
        if (per_area < 0x100) {
            sprintf(advisor.f10, extra_advice[0x14]);
            return 1;
        } else if (per_area < 0x200) {
            sprintf(advisor.f10, extra_advice[0x15]);
            return 1;
        }
    } else if ((gbv.f110 << 10) > unhappy) {
        if (per_area < 0x100) {
            sprintf(advisor.f10, extra_advice[0x16]);
            return 1;
        } else if (per_area < 0x200) {
            sprintf(advisor.f10, extra_advice[0x17]);
            return 1;
        }
    } else if (per_area >= 0x400 && per_area <= 0x800) {
        sprintf(advisor.f10, advisor_text[0x4a]);
        return 1;
    }
    return 0;
}

// FUNCTION: 0x00060f90
u8 advisor_old_hardware(void)
{
    if (adcounts.f90 * 20 * adcounts.f8f > adcounts.f8f * 0x1300) {
        if (myhosp->fc9e[2] > 0) {
            sprintf(advisor.f10, advisor_text[3]);
            return 1;
        } else if (myhosp->fc9e[2] == 0) {
            sprintf(advisor.f10, extra_advice[0x21]);
            return 1;
        }
    } else if (adcounts.f90 * 5 * adcounts.f8f > adcounts.f8f * 0x400) {
        sprintf(advisor.f10, extra_advice[0x1f]);
        return 1;
    } else if (adcounts.f90 * 5 * adcounts.f8f > adcounts.f8f * 0x300) {
        if (myhosp->fc9e[2] > 0) {
            sprintf(advisor.f10, extra_advice[0x20]);
            return 1;
        } else if (myhosp->fc9e[2] == 0) {
            sprintf(advisor.f10, extra_advice[0x22]);
            return 1;
        }
    } else if (adcounts.f90 * 2 * adcounts.f8f > adcounts.f8f * 0x100) {
        if (!myplayer.f41[12]) {
            sprintf(advisor.f10, advisor_situations[10]);
            myplayer.f41[12] = 1;
            return 1;
        } else if (myhosp->fc9e[2] > 0) {
            sprintf(advisor.f10, extra_advice[0x23]);
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
        if (!myplayer.f41[18]) {
            sprintf(advisor.f10, advisor_situations[15]);
            myplayer.f41[18] = 1;
            return 1;
        } else if (listhead[player].objects[7] == 0) {
            sprintf(advisor.f10, advisor_text[15]);
            return 1;
        } else {
            sprintf(advisor.f10, extra_advice[4]);
            return 1;
        }
    } else if (thirst < 10) {
        if (listhead[player].objects[7] == 0) {
            sprintf(advisor.f10, advisor_text[5]);
            return 1;
        } else {
            sprintf(advisor.f10, extra_advice[0x33]);
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
    if (othervar.f89) {
        if (othervar.f89 > 1)
            sprintf(advisor.f10, extra_advice[7]);
        else
            sprintf(advisor.f10, extra_advice[0x31]);
        othervar.f89 = 0;
        return 1;
    } else if ((gbv.f110 << 10) > happiness) {
        sprintf(advisor.f10, extra_advice[6]);
        return 1;
    } else if ((gbv.f10e << 10) > happiness) {
        sprintf(advisor.f10, advisor_text[14]);
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
        sprintf(advisor.f10, extra_advice[0x34]);
        play_ambient(0xd9);
        return 1;
    } else if (warmth > 0x50) {
        if (!myplayer.f41[17]) {
            sprintf(advisor.f10, advisor_situations[14]);
            myplayer.f41[17] = 1;
        } else {
            sprintf(advisor.f10, advisor_text[0x11]);
        }
        play_ambient(0xd9);
        return 1;
    } else if (warmth < 10) {
        sprintf(advisor.f10, extra_advice[0x35]);
        play_ambient(0xd8);
        return 1;
    } else if (warmth < 0x14) {
        if (!myplayer.f41[15] && myhosp->fc92 == 0) {
            sprintf(advisor.f10, advisor_situations[12]);
            myplayer.f41[15] = 1;
        } else if (myplayer.f41[15] && !myplayer.f41[16] && myhosp->fc92 == 0) {
            sprintf(advisor.f10, advisor_situations[13]);
            myplayer.f41[16] = 1;
        } else {
            sprintf(advisor.f10, extra_advice[13]);
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
            sprintf(advisor.f10, extra_advice[0x12]);
            return 1;
        } else if (myhosp->fc9e[2] == 0) {
            sprintf(advisor.f10, extra_advice[0x11]);
            return 1;
        }
    } else if (litter > 0x300) {
        sprintf(advisor.f10, extra_advice[0x10]);
        return 1;
    } else if (litter > 0x100) {
        if (myhosp->fc9e[2] > 1) {
            sprintf(advisor.f10, advisor_text[0x46]);
            return 1;
        } else if (myhosp->fc9e[2] == 0) {
            sprintf(advisor.f10, extra_advice[0x13]);
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
        sprintf(advisor.f10, extra_advice[0x1b]);
        return 1;
    } else if (standing > 0x400) {
        sprintf(advisor.f10, extra_advice[0x18]);
        return 1;
    } else if (NRAdcounts.unknown4 > myhosp->f29 + 8 && standing < -0x1400) {
        sprintf(advisor.f10, extra_advice[0x1a]);
        return 1;
    } else if (NRAdcounts.unknown4 > 8 && standing < -0x400) {
        sprintf(advisor.f10, extra_advice[0x19]);
        return 1;
    } else if (NRAdcounts.unknown4 > 4) {
        sprintf(advisor.f10, advisor_text[0x4b]);
        return 1;
    }
    return 0;
}

// FUNCTION: 0x000617e4
u8 advisor_dead_plants(void)
{
    if (NRAdcounts.unknownb * 0x190000 / NRAdcounts.unknown0 < 0xce4000) {
        if (myhosp->fc9e[2] == 0) {
            sprintf(advisor.f10, extra_advice[0x27]);
            return 1;
        } else if (myhosp->fc9e[2] > 1) {
            sprintf(advisor.f10, extra_advice[0x28]);
            return 1;
        }
    } else if (NRAdcounts.unknownb * 0x190000 / NRAdcounts.unknown0 < 0x2260000) {
        sprintf(advisor.f10, extra_advice[0x29]);
        return 1;
    } else if (NRAdcounts.unknownb * 0x190000 / NRAdcounts.unknown0 < 0x3c28000) {
        sprintf(advisor.f10, advisor_text[0x5a]);
        return 1;
    } else if (NRAdcounts.unknownb * 0x190000 / NRAdcounts.unknown0 > 0x51a4000) {
        sprintf(advisor.f10, extra_advice[0x2b]);
        return 1;
    } else if ((NRAdcounts.unknownb << 14) / NRAdcounts.unknown0 > 0x490c000) {
        sprintf(advisor.f10, extra_advice[0x2a]);
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
    if ((gbv.fea << 8) < average)
        level = 0;
    else if ((gbv.fe8 << 8) < average)
        level = 0x10;
    else if ((gbv.fe6 << 8) > average)
        level = 0x20;
    else
        level = 0x30;
    for (; i < 4; i++) {
        if (myhosp->fc9e[i]) {
            tired = (adcounts.f7a[i] << 8) / myhosp->fc9e[i];
            if ((gbv.fea << 8) < tired) {
                if (((average - tired) << 8) / (myhosp->fc9c - myhosp->fc9e[i])
                    < (gbv.fea << 8))
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
        sprintf(advisor.f10, advisor_text[0x12]);
        play_ambient(0xd9);
        return 1;
    } else if (warmth < 0x14) {
        sprintf(advisor.f10, extra_advice[0x36]);
        play_ambient(0xd8);
        return 1;
    }
    return 0;
}

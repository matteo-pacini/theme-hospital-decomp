#include "hosp.h"

extern "C" {
extern s8 D_0012d804;
extern s8 D_0012d805;
extern s8 D_0012d806;
}

u8 year_check(s32 *values, const s32 limit, const u8 above);
void set_trophy_value(const u8 t, const s16 value, u8 type);
void general_award(s32 *p, s32 value, const u8 award, const s32 high, const s32 low, const u8 size, const u8 is_signed,
                   const u8 type, const s16 good, const s16 bad);

// FUNCTION: 0x000b3430
void check_awards_won(void)
{
    s8 i;
    s32 a;
    s32 b;
    s32 c;

    i = 12;
    a = 0;
    b = 0;
    c = 0;
    LbMemorySet(all_awards, 0, 0x4e);
    while (--i >= 0) {
        a += mycontrol->history[4][0][i];
        b += mycontrol->history[6][0][i];
    }
    if (myhosp->f29 * 3 < a) {
        for (i = 0; i < 13; i++) {
            switch (i) {
            case 0:
                if (myhosp->f29 * 2 <= b)
                    general_award(&myhosp->unknownc7e, myhosp->unknownc7e, 0, awards_trophies.f1f, awards_trophies.f21, 4, 0,
                                  2, awards_trophies.f4d, awards_trophies.f4f);
                break;
            case 1:
                if (myhosp->f1ed > myhosp->f1c7)
                    general_award((s32 *)&myhosp->f1e9, myhosp->f1e9, 1,
                                  awards_trophies.f17 * (myhosp->f1ed - myhosp->f1c7),
                                  awards_trophies.f18 * (myhosp->f1ed - myhosp->f1c7), 2, 0, 2, awards_trophies.f3f,
                                  awards_trophies.f41);
                break;
            case 2:
                if (myhosp->f1ed > myhosp->f1c7 && myhosp->f1e9)
                    general_award((s32 *)&myhosp->f1eb, myhosp->f1eb, 2,
                                  awards_trophies.f19 * (myhosp->f1ed - myhosp->f1c7),
                                  awards_trophies.f1a * (myhosp->f1ed - myhosp->f1c7), 2, 0, 2, awards_trophies.f43,
                                  awards_trophies.f45);
                break;
            case 3:
                if (myhosp->f29 * 2 <= b)
                    general_award((s32 *)&myhosp->f1cb, myhosp->f1cb, 3, awards_trophies.f1b, awards_trophies.f1c, 2, 0, 1,
                                  awards_trophies.f47, awards_trophies.f48);
                break;
            case 4:
                if (myhosp->f1eb > 0) {
                    a = myhosp->f1e9 * 0x6400 / myhosp->f1eb;
                    if (awards_trophies.f1d << 8 < a)
                        fn_000b4510(i, 1, 0, awards_trophies.f49, 2);
                    else if (awards_trophies.f1e << 8 > a)
                        fn_000b4510(i, -1, 0, awards_trophies.f4b, 2);
                } else if (myhosp->f1ed > myhosp->f1c7 && awards_trophies.f3f <= b) {
                    fn_000b4510(i, 1, 0, awards_trophies.f49, 2);
                }
                break;
            case 5:
                if (year_check(award_counts.f34, awards_trophies.f31, 1))
                    fn_000b4510(i, 1, 0, awards_trophies.f55[6], 1);
                else if (year_check(award_counts.f34, awards_trophies.f32, 0))
                    fn_000b4510(i, -1, 0, awards_trophies.f55[7], 1);
                break;
            case 6:
                general_award(&myhosp->f1bb, myhosp->f1bb, 6, awards_trophies.f23, awards_trophies.f27, 4, 0, 1,
                              awards_trophies.f51, awards_trophies.f53);
                break;
            case 7:
                if (year_check(award_counts.f0, awards_trophies.f2f, 1))
                    fn_000b4510(i, 1, 0, awards_trophies.f55[4], 1);
                else if (year_check(award_counts.f0, awards_trophies.f30, 0))
                    fn_000b4510(i, -1, 0, awards_trophies.f55[5], 1);
                break;
            case 8:
                if (myhosp->f1f1 > 0) {
                    a = (myhosp->fcaa + myhosp->fcae * 2 << 14) / myhosp->f1f1;
                    if ((awards_trophies.f2b << 14) / 100 > a)
                        fn_000b4510(i, 1, 0, awards_trophies.f55[0], 1);
                    else if (0x4000 / awards_trophies.f2c < a)
                        fn_000b4510(i, -1, 0, awards_trophies.f55[1], 1);
                }
                break;
            case 9:
                if (year_check(award_counts.f9c, awards_trophies.f33, 0))
                    fn_000b4510(i, 1, 0, awards_trophies.f55[8], 1);
                else if (year_check(award_counts.f9c, awards_trophies.f34, 1))
                    fn_000b4510(i, -1, 0, awards_trophies.f55[9], 1);
                break;
            case 10:
                if (award_counts.f13f > 2) {
                    if (awards_trophies.f2d < award_counts.f140)
                        fn_000b4510(i, 1, 0, awards_trophies.f55[2], 1);
                    else if (awards_trophies.f2e > award_counts.f140)
                        fn_000b4510(i, -1, 0, awards_trophies.f55[3], 1);
                }
                break;
            case 11:
                if (award_counts.f149 > 100) {
                    if (award_counts.f149 > awards_trophies.f37)
                        fn_000b4510(i, 1, 0, D_0012d805, 1);
                    else if (award_counts.f149 < awards_trophies.f3b)
                        fn_000b4510(i, -1, 0, D_0012d806, 1);
                }
                break;
            case 12:
                if (year_check(award_counts.f104, 1, 1) && year_check(award_counts.fd0, awards_trophies.f35, 0))
                    fn_000b4510(i, 1, 0, awards_trophies.f55[10], 1);
                else if (award_counts.f144 || (year_check(award_counts.f104, 1, 1) && year_check(award_counts.fd0, awards_trophies.f36, 1)))
                    fn_000b4510(i, -1, 0, D_0012d804, 1);
                break;
            default:
                break;
            }
        }
    }
}

// FUNCTION: 0x000b3ce4
void check_trophies_won(void)
{
    s8 i;
    s32 a;

    i = 12;
    a = 0;
    LbMemorySet(trophy, 0, 0x2d);
    while (--i >= 0)
        a += mycontrol->history[4][0][i];
    if (a > 5) {
        for (i = 0; i < 9; i++) {
            switch (i) {
            case 0:
                if (myhosp->f1ed > myhosp->f1c7) {
                    if ((myhosp->f1e9 - mycontrol->f4a4 << 8) / (myhosp->f1ed - myhosp->f1c7) > 0xf3) {
                        set_trophy_value(i, awards_trophies.f11, 2);
                        change_balance(0x23, (u16)awards_trophies.f11, 0, player);
                    }
                } else if (myhosp->f1ed == myhosp->f1e9) {
                    set_trophy_value(i, awards_trophies.f11, 2);
                    change_balance(0x23, (u16)awards_trophies.f11 * 2, 0, player);
                }
                break;
            case 1:
                if (year_check(mycontrol->history[7][0], awards_trophies.f6, 1))
                    set_trophy_value(i, awards_trophies.f8, 2);
                break;
            case 2:
                if (year_check(award_counts.f0, awards_trophies.fc, 1))
                    set_trophy_value(i, awards_trophies.fd, 1);
                break;
            case 3:
                if (award_counts.f13e > 1) {
                    if ((award_counts.f13a << 8) / award_counts.f13e < awards_trophies.f15 << 8)
                        set_trophy_value(i, awards_trophies.f16, 1);
                }
                break;
            case 4:
                if (gamevar.f177 == myhosp->f1eb)
                    set_trophy_value(i, awards_trophies.f13, 2);
                gamevar.f177 = myhosp->f1eb;
                break;
            case 5:
                if (awards_trophies.unknown0 < award_counts.f14d) {
                    set_trophy_value(i, awards_trophies.f1, 1);
                    othervar.fa59 = award_counts.f14d;
                }
                break;
            case 6:
                if (award_counts.f151 > 0 && awards_trophies.unknown0 / 2 < award_counts.f14d) {
                    if ((award_counts.f14d << 8) / award_counts.f151 > (awards_trophies.unknowne << 8) / 100) {
                        set_trophy_value(i, awards_trophies.ff, 2);
                        othervar.fa5a = award_counts.f14d * 100 / award_counts.f151;
                    }
                }
                break;
            case 7:
                if (year_check(award_counts.f68, awards_trophies.fa << 14, 1))
                    set_trophy_value(i, awards_trophies.fb, 1);
                break;
            case 8:
                if (award_counts.f138 > awards_trophies.f3)
                    set_trophy_value(i, awards_trophies.f4, 2);
                break;
            default:
                break;
            }
        }
    }
}

// FUNCTION: 0x000b40ec
u8 year_check(s32 *values, const s32 limit, const u8 above)
{
    s8 i;

    i = 0;
    if (above) {
        for (; i < 12; i++) {
            if (values[i] < limit)
                return 0;
        }
    } else {
        for (; i < 12; i++) {
            if (values[i] > limit)
                return 0;
        }
    }
    return 1;
}

// FUNCTION: 0x000b4170
void monthly_update_of_award_counts(void)
{
    if (myhosp->fc9c > 3)
        award_counts.f0[mycontrol->unknown650] = (adcounts.f4s << 8) / myhosp->fc9c;
    else
        award_counts.f0[mycontrol->unknown650] = 0;
    if (adcounts.f0 > 3) {
        award_counts.f34[mycontrol->unknown650] = (adcounts.fa << 8) / adcounts.f0;
        award_counts.f9c[mycontrol->unknown650] = (adcounts.f4a << 8) / adcounts.f0;
    } else {
        award_counts.f34[mycontrol->unknown650] = 0;
        award_counts.f9c[mycontrol->unknown650] = gbv.f108 << 8;
    }
    if (NRAdcounts.unknown0 > 7)
        award_counts.f68[mycontrol->unknown650] = NRAdcounts.unknownb * 0x190000 / NRAdcounts.unknown0;
    else
        award_counts.f68[mycontrol->unknown650] = 0;
    award_counts.fd0[mycontrol->unknown650] = adcounts.f90;
    award_counts.f104[mycontrol->unknown650] = adcounts.f8f;
}

// FUNCTION: 0x000b4358
void annual_update_of_award_counts(void)
{
    award_counts.f138 = 0;
    award_counts.f13a = 0;
    award_counts.f13e = 0;
    award_counts.f13f = 0;
    award_counts.f140 = 0;
    award_counts.f145 = 0;
    award_counts.f149 = 0;
    award_counts.f14d = 0;
    award_counts.f151 = 0;
}

// FUNCTION: 0x000b43c0
void set_trophy_value(const u8 t, const s16 value, u8 type)
{
    trophy[t].unknown0 = t;
    trophy[t].f1 = t * 3 + saferand() % 3 + 1;
    if (type == 1) {
        s32 rep;

        rep = myhosp->unknownc7e;
        reputation_affected(0, 0x17, 0, player, value);
        if (myhosp->unknownc7e != rep) {
            trophy[t].value = myhosp->unknownc7e - rep;
        } else {
            change_balance(0x23, 5000, 0, player);
            trophy[t].value = 5000;
            trophy[t].unknown0 = 2;
        }
    } else if (type == 2) {
        change_balance(0x23, value, 0, player);
        trophy[t].value = value;
    }
    trophy[t].f4 = saferand() % 1 + type - 1;
}

// FUNCTION: 0x000b4510
void fn_000b4510(const u8 a, const s8 dir, const u8 level, const s16 value, u8 type)
{
    all_awards[a].unknown0 = dir;
    all_awards[a].f1 = a;
    all_awards[a].f2 = level + a * 3 + saferand() % 1 + 1;
    if (type == 2) {
        change_balance(0x22, value, 0, player);
        all_awards[a].value = value;
    } else if (type == 1) {
        s32 rep;

        rep = myhosp->unknownc7e;
        reputation_affected(0, 0x18, 0, player, value);
        if (myhosp->unknownc7e != rep) {
            all_awards[a].value = myhosp->unknownc7e - rep;
        } else {
            if (dir > 0) {
                change_balance(0x22, 2500, 0, player);
                all_awards[a].value = 2500;
            } else if (dir < 0) {
                change_balance(0x22, -2500, 0, player);
                all_awards[a].value = -2500;
            }
            type = 2;
        }
    }
    all_awards[a].f5 = saferand() % 1 + type - 1;
}

// FUNCTION: 0x000b4698
void general_award(s32 *p, s32 value, const u8 award, const s32 high, const s32 low, const u8 size, const u8 is_signed,
                   const u8 type, const s16 good, const s16 bad)
{
    s32 h;
    s32 total;
    s8 top;
    s8 bottom;
    s8 best;

    total = 0;
    best = 1;
    top = 1;
    bottom = 1;
    h = 4;
    while (--h >= 0) {
        if (statvar.unknownfd > 1 && D_001609c4[h].active == 0)
            continue;
        if (&allhosps[h] == myhosp)
            continue;
        if (size == 4) {
            if (is_signed) {
                if (value < *(s32 *)((u8 *)p + h * 0x1259)) {
                    best = 0;
                    top = 0;
                } else {
                    total += *(s32 *)(h * 0x1259 + (u32)p);
                    bottom = 0;
                }
            } else {
                if ((u32)value < *(u32 *)((u8 *)p + h * 0x1259)) {
                    best = 0;
                    top = 0;
                } else {
                    total += *(u32 *)(h * 0x1259 + (u32)p);
                    bottom = 0;
                }
            }
        } else if (size == 2) {
            if (is_signed) {
                if (*(s16 *)(h * 0x1259 + (u32)p) > value) {
                    best = 0;
                    top = 0;
                } else {
                    total += *(s16 *)(h * 0x1259 + (u32)p);
                    bottom = 0;
                }
            } else {
                if (*(u16 *)(h * 0x1259 + (u32)p) > value) {
                    best = 0;
                    top = 0;
                } else {
                    total += *(u16 *)(h * 0x1259 + (u32)p);
                    bottom = 0;
                }
            }
        } else if (size == 1) {
            if (is_signed) {
                if (*(s8 *)(h * 0x1259 + (u32)p) > value) {
                    best = 0;
                    top = 0;
                } else {
                    total += *(s8 *)(h * 0x1259 + (u32)p);
                    bottom = 0;
                }
            } else {
                if (*(u8 *)(h * 0x1259 + (u32)p) > value) {
                    best = 0;
                    top = 0;
                } else {
                    total += *(u8 *)(h * 0x1259 + (u32)p);
                    bottom = 0;
                }
            }
        }
    }
    if (value > high) {
        if (top) {
            if (value > total && !bottom)
                fn_000b4510(award, 1, 2, good * 3, type);
            else if (top == 1)
                fn_000b4510(award, 1, 1, good * 2, type);
        } else {
            fn_000b4510(award, 1, 0, good, type);
        }
    } else if (value < low) {
        if (!bottom)
            fn_000b4510(award, -1, 0, bad, type);
        else
            fn_000b4510(award, -1, 1, bad * 2, type);
    }
}

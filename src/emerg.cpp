#include "hosp.h"

struct EmergencyDef {
    u16 start;
    u16 end;
    u8 min;
    u8 max;
    u8 illness;
    u8 f7;
    u16 f8;
};

extern "C" {
extern u32 D_0015535d;
extern u16 D_00155361;
extern u8 D_00155363;
extern u8 D_00155364;
extern u8 D_00155365;
extern u8 D_00155366;
extern u8 D_0015537e;
extern u8 D_0015539b[];
extern u16 D_00158190;
extern EmergencyDef D_0015fc3c[];
}

u32 check_emergency_criteria(void);
void setup_next_emergency(void);
void setup_emergency_stats(u16 call, u8 owner);
u16 summon_vehicle(u16 owner, u8 a, u16 b, u16 c, u16 d);
u32 old_newrand(void);

// FUNCTION: 0x000aeec0
void try_to_generate_emergency(void)
{
    s32 n;
    AllHosps *h;
    s32 j;
    s32 i;
    s32 found;

    found = 0;
    n = 1;
    if (statvar.unknownfd > 1)
        n = 4;
    for (i = 0; i < n; i++) {
        if (statvar.unknownfd <= 1 || D_001609c4[i].active != 0) {
            h = &allhosps[i];
            if (h->f1e2s == 5) {
                h->f1df = summon_vehicle(i, 0, allhosps[i].f1d7 + 0x80, 0x4cc, 0x7a7);
                if (h->f1df != 0) {
                    h->f1e2 = 3;
                    h->f1e1 = 0;
                } else {
                    h->f1e3 = 0;
                    h->f1e2 = 0;
                }
            }
        }
    }
    if (check_emergency_criteria()) {
        for (j = 0; j < n; j++) {
            if (statvar.unknownfd <= 1 || D_001609c4[j].active != 0) {
                if (allhosps[j].rooms[D_00155364].unknown1D == 1)
                    found = 1;
            }
        }
        if (found) {
            for (j = 0; j < n; j++) {
                if (statvar.unknownfd <= 1 || D_001609c4[j].active != 0)
                    setup_emergency_stats(make_a_playercall(2, 0, 0x14, j), j);
            }
        }
        setup_next_emergency();
    }
    for (i = 0; i < n; i++) {
        if (statvar.unknownfd <= 1 || D_001609c4[i].active != 0) {
            h = &allhosps[i];
            if (h->f1e2s > 5)
                h->f1e2--;
        }
    }
}

// FUNCTION: 0x000af118
u32 check_emergency_criteria(void)
{
    u8 i;
    s32 n;

    n = 1;
    if (statvar.unknownfd > 1)
        n = 4;
    if (statvar.unknownfd <= 1 && display_mode == 9 && D_00158190 == 5)
        return 0;
    if (actual_game_turn >= D_0015535d && D_00155363) {
        for (i = 0; i < n; i++) {
            if (statvar.unknownfd <= 1 || D_001609c4[i].active != 0) {
                if (check_playercall(3, 0, i))
                    return 0;
                if (allhosps[i].f20d)
                    return 0;
                if (allhosps[i].f1e2s != 0)
                    return 0;
                if (!(statvar.unknownfd > 1 ? 1 : allhosps[i].unknown1256))
                    return 0;
            }
        }
        if (statvar.f6a != 0 && D_0015537e < 4)
            return 1;
    }
    return 0;
}

// FUNCTION: 0x000af2c0
void setup_next_emergency(void)
{
    u32 month;
    u32 r;
    u32 i;

    month = mycontrol->f648 * 12 + mycontrol->unknown650;
    D_00155363 = 0;
    for (i = 0; i < 20; i++) {
        if (D_0015539b[i] == 0) {
            D_0015539b[i] = 1;
            if (D_0015fc3c[i].end > month && D_0015fc3c[i].end > D_0015fc3c[i].start) {
                r = old_newrand() % ((D_0015fc3c[i].end + 1 - D_0015fc3c[i].start) * 0x5dc);
                D_0015535d = r + D_0015fc3c[i].start * 0x5dc;
                D_00155364 = D_0015fc3c[i].illness;
                r = old_newrand() % (D_0015fc3c[i].max + 1 - D_0015fc3c[i].min);
                D_00155365 = D_0015fc3c[i].min + r;
                D_00155366 = D_0015fc3c[i].f7;
                D_00155361 = D_0015fc3c[i].f8;
                D_00155363 = 1;
                break;
            }
        }
    }
}

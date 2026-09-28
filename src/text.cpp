#include "hosp.h"

extern "C" {

extern char **D_00160298;
extern char **D_0016029c;
extern char **D_001602a0;
extern char **D_001602a4;
extern char **D_001602a8;
extern char **D_001602b0;
extern char **D_001602b4;
extern char **D_001602c4;
extern char **D_001602c8;
extern char **D_001602cc;
extern char **D_001602d0;
extern char **D_001602d4;
extern char **D_001602d8;
extern char **D_001602dc;
extern char **D_001602e0;
extern char **D_001602e8;
extern char **D_001602ec;
extern char **D_001602f0;
extern char **D_001602f4;
extern char **D_001602fc;
extern char **D_00160364;
extern char **D_00160304;
extern char **D_00160308;
extern char **D_00160318;
extern char **D_0016032c;
extern char **D_0016031c;
extern char **D_00160320;
extern char **D_0016033c;
extern char **D_00160354;
extern char **D_00160358;
extern char **D_0016035c;
extern char **D_00160360;
extern char **D_00160370;
extern char **D_00160374;
extern char **D_0016036c;
extern char **D_00160368;
extern char **D_0016037c;
extern char **D_00160380;
extern char **D_00160378;
extern char **D_00160388;
extern char **D_00160390;
extern char **D_00160394;
extern char **D_00160398;
extern char **D_0016039c;

}

// FUNCTION: 0x0002d1c0
u8 set_group_address(s32 group, char **address)
{
    if (group == 0) {
        D_00160298 = address;
        return 1;
    }
    if (group == 1) {
        D_0016029c = address;
        return 1;
    }
    if (group == 2) {
        D_001602a0 = address;
        return 1;
    }
    if (group == 3) {
        D_001602a4 = address;
        return 1;
    }
    if (group == 4) {
        D_001602a8 = address;
        return 1;
    }
    if (group == 5) {
        month_text = address;
        return 1;
    }
    if (group == 6) {
        D_001602b0 = address;
        return 1;
    }
    if (group == 7) {
        D_001602b4 = address;
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
        D_001602c4 = address;
        return 1;
    }
    if (group == 0xc) {
        D_001602c8 = address;
        return 1;
    }
    if (group == 0xd) {
        D_001602cc = address;
        return 1;
    }
    if (group == 0xe) {
        D_001602d0 = address;
        return 1;
    }
    if (group == 0xf) {
        D_001602d4 = address;
        return 1;
    }
    if (group == 0x10) {
        D_001602d8 = address;
        return 1;
    }
    if (group == 0x11) {
        D_001602dc = address;
        return 1;
    }
    if (group == 0x12) {
        D_001602e0 = address;
        return 1;
    }
    if (group == 0x13) {
        insurance_names = address;
        return 1;
    }
    if (group == 0x14) {
        D_001602e8 = address;
        return 1;
    }
    if (group == 0x15) {
        D_001602ec = address;
        return 1;
    }
    if (group == 0x16) {
        D_001602f0 = address;
        return 1;
    }
    if (group == 0x17) {
        D_001602f4 = address;
        return 1;
    }
    if (group == 0x18) {
        hall_text = address;
        return 1;
    }
    if (group == 0x19) {
        D_001602fc = address;
        return 1;
    }
    if (group == 0x1a) {
        D_00160364 = address;
        return 1;
    }
    if (group == 0x1b) {
        extra_advice = address;
        return 1;
    }
    if (group == 0x1c) {
        D_00160304 = address;
        return 1;
    }
    if (group == 0x1d) {
        D_00160308 = address;
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
        D_00160318 = address;
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
        D_0016032c = address;
        return 1;
    }
    if (group == 0x25) {
        D_0016031c = address;
        return 1;
    }
    if (group == 0x26) {
        D_00160320 = address;
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
        D_0016033c = address;
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
        D_00160354 = address;
        return 1;
    }
    if (group == 0x31) {
        D_00160358 = address;
        return 1;
    }
    if (group == 0x32) {
        D_0016035c = address;
        return 1;
    }
    if (group == 0x33) {
        D_00160360 = address;
        return 1;
    }
    if (group == 0x34) {
        D_00160370 = address;
        return 1;
    }
    if (group == 0x35) {
        D_00160374 = address;
        return 1;
    }
    if (group == 0x36) {
        D_0016036c = address;
        return 1;
    }
    if (group == 0x37) {
        D_00160368 = address;
        return 1;
    }
    if (group == 0x38) {
        D_0016037c = address;
        return 1;
    }
    if (group == 0x39) {
        D_00160380 = address;
        return 1;
    }
    if (group == 0x3a) {
        info_bar_text = address;
        return 1;
    }
    if (group == 0x3b) {
        D_00160378 = address;
        return 1;
    }
    if (group == 0x3c) {
        D_00160388 = address;
        return 1;
    }
    if (group == 0x3d) {
        advisor_situations = address;
        return 1;
    }
    if (group == 0x3e) {
        D_00160390 = address;
        return 1;
    }
    if (group == 0x3f) {
        D_00160394 = address;
        return 1;
    }
    if (group == 0x42) {
        D_00160398 = address;
        return 1;
    }
    if (group == 0x43) {
        D_0016039c = address;
        return 1;
    }
    return 1;
}

// FUNCTION: 0x0002d7c4
s16 setup_lang(void)
{
    s16 *counts;
    s32 groups;
    s32 i;
    s32 total;
    s32 j;
    char **starts;
    char *p;

    if (LbDataLoadAll(&langfile[myplayer.f21]) != 1)
        return 0;
    groups = *(s16 *)text_buffer;
    counts = (s16 *)(text_buffer + 2);
    p = (char *)(text_buffer + groups * 2 + 2);
    total = 0;
    for (i = 0; i < groups; i++)
        total += counts[i];
    text_starts = (char **)LbMemoryAlloc(total * 4);
    if (text_starts == 0)
        return 0;
    starts = text_starts;
    for (i = 0; i < groups; i++) {
        if (!set_group_address(i, starts))
            return 0;
        for (j = 0; j < counts[i]; j++) {
            *starts++ = p;
            while (*p)
                p++;
            p++;
        }
    }
    return 1;
}

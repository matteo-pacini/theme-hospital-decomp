#include "hosp.h"

struct MenuItem {
    u8 unknown0[8];
    u16 text;
    u8 unknown1[6];
};

extern "C" {

extern u32 D_00188c38;
extern u8 D_00155c6e;
extern u16 D_00155da9;
extern u32 D_00155dab;
extern u32 D_00155c5a;
extern u16 D_00155e30;
extern Event D_001549d0[];
extern View *D_0015f430;
extern u16 D_00188c74;
extern u16 D_00155444;
extern char D_0012017c[], D_0012017f[], D_00120182[], D_00120187[], D_0012018b[], D_0012018f[];
extern char D_00120192[], D_00120195[], D_00120198[], D_0012019b[], D_0012019e[], D_001201a1[];
extern char **D_00160304, **D_001602a4;
extern Point *D_0015f434;
extern Button *D_0015f43c;
extern u8 D_00188c7b, D_00188c7e;
extern volatile s16 D_00188c5c, D_00188c60;
extern u16 D_00155376;
extern MenuItem *D_0015f420;
extern u16 D_00155c64;
extern char D_0015544e[];
extern char **D_00160318;

void fn_0001b7e0(void *dst, char *text, int flags);

}

// FUNCTION: 0x00012690
void display_drugs(void)
{
    u8 unused30;
    u8 count;
    u8 unused18;
    u8 noStaff;
    u8 unavailable;
    u8 lowFee;
    u8 i;
    u8 unused28;
    u8 unused24;
    u8 unused20;
    u8 unused1c;
    u16 value;
    s16 offset;
    u16 room;
    Expertise *def;
    int result;
    int n;
    u8 list[18];
    char buf[100];

    n = 0;
    unused28 = 0;
    unused24 = 0;
    unused20 = 0;
    unused1c = 0;
    unused18 = 0;
    noStaff = 0;
    unavailable = 0;
    lowFee = 0;
    unused30 = 0;
    compute_help_text(0, 0, 0xe);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    D_00155c6e = 0;
    switch (drugs.unknown2) {
    case 0:
        fade_out_menu();
        load_ingame_pic(2, 1, 0x1b, 0, SCREEN_MODE_13);
        drugs.unknown2 = 2;
        drugs.unknown0 = 0;
        D_00155da9 = 0x100;
        D_00155e30 = 0;
        break;
    case 1:
        D_001549d0[player].type = 0x70;
        D_001549d0[player].value = 0;
        break;
    case 2:
        copy_screen_to_wscreen(bscreen);
        break;
    default:
        break;
    }
    set_font_from_cache(0xe);
    offset = lbFontPtr ? lbFontPtr[34].height : 0;
    LbTextSetWindow(D_0015f430->unknown0, D_0015f430->unknown4, D_0015f430->unknown8, D_0015f430->unknownC);
    D_00188c74 = 0x100;
    room = D_00155444;
    def = &expertise[room];
    result = max_price_for_treatment(room, player);
    for (i = 0; i + 0x11 != 0x1d; i++) {
        switch (i) {
        case 6:
            if (room >= 0x24 || room == 1) {
                sprintf(buf, D_0012017c, myhosp->unknownc7e);
            } else {
                sprintf(buf, D_0012017f, myhosp->rooms[room].unknown0);
            }
            break;
        case 7:
            if (D_00155e30) {
                D_00155e30--;
                sprintf(buf, D_00120182, myhosp->rooms[room].unknown10 / 100);
            } else {
                sprintf(buf, D_00120187, myhosp->rooms[room].unknownC);
            }
            break;
        case 8:
            sprintf(buf, D_0012018b, myhosp->rooms[room].unknown14);
            break;
        case 9:
            sprintf(buf, D_0012018f, myhosp->rooms[room].unknown1E);
            break;
        case 10:
            sprintf(buf, D_00120192, myhosp->rooms[room].unknown20);
            break;
        case 11:
            sprintf(buf, D_00120195, myhosp->rooms[room].unknown24);
            break;
        default:
            sprintf(buf, D_00120198, D_00160304[i]);
            fn_0001b7e0(buf, buf, 1);
            break;
        }
        LbTextDraw(D_0015f434[i + 0x11].x, D_0015f434[i + 0x11].y, buf);
    }
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0;
    for (i = 0; i < 0x12; i++) {
        list[i] = 0;
    }
    n = 0;
    list[8] = D_00155444;
    count = 0;
    if (D_00155444 + 1 < 0x2e) {
        for (i = D_00155444 + 1; i < 0x2e; i++) {
            if ((i < 0x24 && myhosp->rooms[i].unknown1D)
                || (i >= 0x24 && allhosps[player].rooms[i].unknown1C == 1)
                || (i == 0x2c && (allhosps[player].equipment[55].unknown0 != 0 && objects[55].unknown1c != 0)
                    && listhead[player].rooms[0x1c])) {
                if (count >= 7) {
                    break;
                }
                list[count + 9] = i;
                n++;
                count++;
            }
        }
    }
    if (D_00155444 - 1 > 0) {
        n = 7;
        count = 7;
        for (i = D_00155444 - 1; i > 0; i--) {
            if (((i < 0x24 && myhosp->rooms[i].unknown1D)
                 || (i >= 0x24 && allhosps[player].rooms[i].unknown1C == 1))
                && i < 0x2c) {
                if (count == 0) {
                    break;
                }
                list[count] = i;
                n++;
                count--;
            }
        }
    }
    count = 0;
    n = 0;
    for (i = 0; i < 0x11; i++) {
        if (list[i]) {
            sprintf(buf, D_0012019b, D_001602a4[list[i]]);
            fn_0001b7e0(buf, buf, 0);
            if (i == 8) {
                set_font_from_cache(0xf);
            } else {
                set_font_from_cache(0xe);
                if (D_00188c7b) {
                    if (over_hotspot(D_00188c5c, D_00188c60, D_0015f434[i].x, D_0015f434[i].y,
                                    D_00188c34 - D_0015f434[i].y, offset)) {
                        D_00155444 = list[i];
                        play_sound(10, 0xa8, -1, -1);
                        D_00188c7b = 0;
                    }
                }
            }
            LbTextDraw(D_0015f434[i].x, D_0015f434[i].y, buf);
        }
    }
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0;
    set_font_from_cache(0xe);
    D_00188c74 = 0x100;
    LbTextSetWindow(D_0015f434[0x1e].x, D_0015f434[0x1e].y, 0x36, 0x1a);
    LbTextDraw(0, (0x1a - (lbFontPtr ? lbFontPtr[34].height : 0)) / 2, D_00160304[7]);
    D_00188c74 = 0;
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    if (!(room >= 0x24 || room == 1)) {
        if (allhosps[player].rooms[room].unknown1C == 1) {
            if (def->kind == 0x27) {
                value = allhosps[player].rooms[room].unknown1B;
                reqspr_draw(D_0015f434[0x1f].x, D_0015f434[0x1f].y, screen_sprites_tab_start + 7);
                set_font_from_cache(0x2e);
                sprintf(buf, D_0012019e, value);
                LbTextSetWindow(0x134, D_0015f434[0x21].y, 0x1c, 0x32);
                D_00188c74 = 0x100;
                LbTextDraw(0, 0, buf);
                if (value < 100) {
                    lowFee = 1;
                }
                LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
                D_00188c74 = 0;
            } else if (def->category == 10) {
                reqspr_draw(D_0015f434[0x1f].x, D_0015f434[0x1f].y, screen_sprites_tab_start + 8);
            } else if (def->unknown8 != 1 && def->unknown8 != 5 && def->unknown8 != 4) {
                reqspr_draw(D_0015f434[0x1f].x, D_0015f434[0x1f].y, screen_sprites_tab_start + 6);
            }
            reqspr_draw(D_0015f434[0x20].x, D_0015f434[0x20].y, screen_sprites_tab_start + 11);
        } else {
            reqspr_draw(D_0015f434[0x20].x, D_0015f434[0x20].y, screen_sprites_tab_start + 12);
            if (def->unknown8 != 1 && def->unknown8 != 5 && def->unknown8 != 4) {
                if (def->kind == 0x27) {
                    reqspr_draw(D_0015f434[0x1f].x, D_0015f434[0x1f].y, screen_sprites_tab_start + 7);
                    set_font_from_cache(0x2e);
                    sprintf(buf, D_001201a1);
                    LbTextSetWindow(0x134, D_0015f434[0x21].y, 0x1c, 0x32);
                    D_00188c74 = 0x100;
                    LbTextDraw(0, 0, buf);
                    unavailable = 1;
                    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
                    D_00188c74 = 0;
                } else if (allhosps[player].equipment[def->kind].unknown0 == 1
                           && objects[def->kind].unknown1c == 1) {
                    reqspr_draw(D_0015f434[0x1f].x, D_0015f434[0x1f].y, screen_sprites_tab_start + 6);
                    if (listhead[player].rooms[def->category] == 0) {
                        noStaff = 1;
                    }
                    if (def->unknown0 == 0x40) {
                        reqspr_draw(D_0015f434[0x1f].x, D_0015f434[0x1f].y, screen_sprites_tab_start + 10);
                    }
                    if (def->category == 10) {
                        reqspr_draw(D_0015f434[0x1f].x, D_0015f434[0x1f].y, screen_sprites_tab_start + 8);
                    }
                } else {
                    reqspr_draw(D_0015f434[0x1f].x, D_0015f434[0x1f].y, screen_sprites_tab_start + 9);
                    unavailable = 1;
                }
            }
        }
    } else if (room >= 0x24) {
        if (allhosps[player].rooms[room].unknown1C == 1) {
            reqspr_draw(D_0015f434[0x20].x, D_0015f434[0x20].y, screen_sprites_tab_start + 11);
        } else {
            reqspr_draw(D_0015f434[0x20].x, D_0015f434[0x20].y, screen_sprites_tab_start + 12);
        }
    }
    if (expertise[room].unknown5[0] != 0) {
        if (room == (statvar.unknownfd > 1 ? allhosps[player].unknown1256 : D_00155376)) {
            set_font_from_cache(0xf);
        } else {
            set_font_from_cache(0xe);
        }
        fn_0001b7e0(buf, D_00160304[6], 1);
        LbTextDraw(D_0015f434[0x1d].x, D_0015f434[0x1d].y, buf);
    }
    process_drug_panel();
    process_drug_keys();
    D_00188c74 = 0;
    if (exit_quick_requester()) {
        drugs.unknown2 = 1;
    }
    set_font_from_cache(0);
}

// FUNCTION: 0x00013540
void process_drug_panel(void)
{
    u8 i;

    if (D_00188c7b) {
        drugs.unknown0 = 0;
        for (i = 0; D_0015f43c[i + 0x12].width != 0 && D_0015f43c[i + 0x12].height != 0; i++) {
            if (over_hotspot(D_00188c5c, D_00188c60, D_0015f43c[i + 0x12].x, D_0015f43c[i + 0x12].y,
                            (s16)D_0015f43c[i + 0x12].width, (s16)D_0015f43c[i + 0x12].height)) {
                D_0015f43c[i + 0x12].callback(i);
                if (D_0015f43c[i + 0x12].sprite) {
                    reqspr_draw(D_0015f43c[i + 0x12].unknownC, D_0015f43c[i + 0x12].unknownE,
                                (TbSprite *)((u32)screen_sprites_tab_start + D_0015f43c[i + 0x12].sprite * 6));
                }
                break;
            }
        }
        D_00155da9 = 0x100;
        D_00155dab = D_00155c5a;
    }
    if (D_00188c7e) {
        for (i = 0; D_0015f43c[i + 0x12].width != 0 && D_0015f43c[i + 0x12].height != 0; i++) {
            if (over_hotspot((s16)mouse_dx, (s16)mouse_dy, D_0015f43c[i + 0x12].x, D_0015f43c[i + 0x12].y,
                            (s16)D_0015f43c[i + 0x12].width, (s16)D_0015f43c[i + 0x12].height)) {
                if (i == 0 || i == 1) {
                    D_0015f43c[i + 0x12].callback(i);
                }
                if (D_0015f43c[i + 0x12].sprite) {
                    reqspr_draw(D_0015f43c[i + 0x12].unknownC, D_0015f43c[i + 0x12].unknownE,
                                (TbSprite *)((u32)screen_sprites_tab_start + D_0015f43c[i + 0x12].sprite * 6));
                }
                break;
            }
        }
    }
    D_00188c7b = 0;
}

// FUNCTION: 0x0001387c
void price_up(void)
{
    if (D_00155c5a - D_00155dab > 0xfa || D_00155da9 == 0x100) {
        D_001549d0[player].type = 0x6e;
        D_001549d0[player].value = D_00155da9 >> 8;
        D_001549d0[player].arg = D_00155444;
        if (D_00155da9 < 0xa00) {
            D_00155da9 += 0x10;
        } else {
            D_00155da9 = 0xa00;
        }
        if (myhosp->rooms[D_00155444].unknown10 < 20000) {
            play_sound(10, 0x43, -1, -1);
        } else if (!drugs.unknown0) {
            play_sound(10, 8, -1, -1);
            drugs.unknown0 = 1;
        }
    }
}

// FUNCTION: 0x0001397c
void price_down(void)
{
    if (D_00155c5a - D_00155dab > 0xfa || D_00155da9 == 0x100) {
        D_001549d0[player].type = 0x6f;
        D_001549d0[player].value = D_00155da9 >> 8;
        D_001549d0[player].arg = D_00155444;
        if (D_00155da9 < 0xa00) {
            D_00155da9 += 0x10;
        } else {
            D_00155da9 = 0xa00;
        }
        if (myhosp->rooms[D_00155444].unknown10 > 5000) {
            play_sound(10, 0x43, -1, -1);
        } else if (!drugs.unknown0) {
            play_sound(10, 8, -1, -1);
            drugs.unknown0 = 1;
        }
    }
}

// FUNCTION: 0x00013a7c
void process_drug_keys(void)
{
    u8 room;

    if (lbKeyOn[0x48] || lbKeyOn[0xc8]) {
        room = get_next_illness(0, D_00155444);
        if (room != 0xfa) {
            D_00155444 = room;
            play_sound(10, 0xa8, -1, -1);
        } else {
            play_sound(10, 8, -1, -1);
        }
    }
    if (lbKeyOn[0x50] || lbKeyOn[0xd0]) {
        room = get_next_illness(1, D_00155444);
        if (room != 0xfa) {
            D_00155444 = room;
            play_sound(10, 0xa8, -1, -1);
        } else {
            play_sound(10, 8, -1, -1);
        }
    }
    if (lbKeyOn[0x4d] || lbKeyOn[0xcd]) {
        price_up();
    } else if (lbKeyOn[0x4b] || lbKeyOn[0xcb]) {
        price_down();
    } else if (!D_00188c7e) {
        D_00155da9 = 0x100;
        D_00155dab = D_00155c5a;
    }
}

// FUNCTION: 0x00013bc4
void process_drug_button(u8 action)
{
    u8 room;

    debug.unknown8 = action;
    switch (action) {
    case 0:
        price_up();
        D_00155e30 = 0x32;
        break;
    case 1:
        price_down();
        D_00155e30 = 0x32;
        break;
    case 2:
        room = get_next_illness(0, D_00155444);
        if (room != 0xfa) {
            D_00155444 = room;
            play_sound(10, 0xa8, -1, -1);
        } else {
            play_sound(10, 8, -1, -1);
        }
        D_00155e30 = 0x32;
        break;
    case 3:
        room = get_next_illness(1, D_00155444);
        if (room != 0xfa) {
            D_00155444 = room;
            play_sound(10, 0xa8, -1, -1);
        } else {
            play_sound(10, 8, -1, -1);
        }
        D_00155e30 = 0x32;
        break;
    case 4:
        drugs.unknown2 = 1;
        play_sound(10, 0x42, -1, -1);
        break;
    case 5:
        D_001549d0[player].type = 0x72;
        D_001549d0[player].value = D_00155444;
        play_sound(10, 0x43, -1, -1);
        break;
    }
}

// FUNCTION: 0x00013d8c
u8 get_next_illness(const u8 forward, const u8 start)
{
    u8 i;

    switch (forward) {
    case 0:
        if (start - 1 > 0) {
            for (i = start - 1; i > 0; i--) {
                if ((i < 0x24 && myhosp->rooms[i].unknown1D)
                    || (i >= 0x24 && allhosps[player].rooms[i].unknown1C == 1)
                    || (i == 0x2c && (allhosps[player].equipment[55].unknown0 != 0 && objects[55].unknown1c != 0)
                        && listhead[player].rooms[0x1c])) {
                    return i;
                }
            }
        }
        break;
    default:
        if (start + 1 < 0x2e) {
            for (i = start + 1; i < 0x2e; i++) {
                if ((i < 0x24 && myhosp->rooms[i].unknown1D)
                    || (i >= 0x24 && allhosps[player].rooms[i].unknown1C == 1)
                    || (i == 0x2c && (allhosps[player].equipment[55].unknown0 != 0 && objects[55].unknown1c != 0)
                        && listhead[player].rooms[0x1c])) {
                    return i;
                }
            }
        }
        break;
    }
    return 0xfa;
}

// FUNCTION: 0x00013fb0
void setup_drug(u16 a, u16 b)
{
    if (expertise[b].kind == 0x27) {
        allhosps[a].rooms[b].unknown1B = expertise[57].unknown10[3];
        allhosps[a].rooms[b].unknown18 = *(u16 *)&expertise[57].unknown10[5];
    }
}

// FUNCTION: 0x0001403c
void drugs_help_concentrate(void)
{
    if (D_00155c6e) {
        LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[D_0015f420[D_00155c64].text]);
    }
}

// FUNCTION: 0x00014090
void drugs_help_cure_type(void)
{
    if (D_00155444 >= 0x24 || D_00155444 == 1) {
        return;
    }
    if (expertise[D_00155444].kind == 0x27) {
        if (allhosps[player].rooms[D_00155444].unknown1C == 1) {
            sprintf(D_0015544e, help_text[148], allhosps[player].rooms[D_00155444].unknown1B);
        } else {
            LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[147]);
        }
    } else if (expertise[D_00155444].unknown0 == 0x40) {
        LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[149]);
    } else if (expertise[D_00155444].category == 10) {
        LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[150]);
    } else {
        LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[151]);
    }
}

// FUNCTION: 0x000141f0
void drugs_help_handled(void)
{
    if (allhosps[player].rooms[D_00155444].unknown1C == 1) {
        LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[152]);
        return;
    }
    what_is_required(D_00155444, player);
    if (required.unknown7) {
        LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[153]);
    } else if (required.unknown6) {
        LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[154]);
    } else if (required.unknown4 == 2) {
        LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[155]);
    } else if (required.unknown4 == 1) {
        LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[156]);
    } else if (required.unknown3) {
        sprintf(D_0015544e, help_text[157], D_00160318[required.unknown3]);
    } else if (required.unknown8) {
        LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[158]);
    } else if (required.unknown9) {
        LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[159]);
    } else {
        LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[160]);
    }
}

// FUNCTION: 0x0001438c
void update_cure_costs(void)
{
    u32 hospital;
    u32 room;

    for (hospital = 0; hospital < 4; hospital++) {
        for (room = 0; room < 0x2f; room++) {
            update_single_cure_cost(room, hospital);
        }
    }
}

// FUNCTION: 0x000143e0
void update_single_cure_cost(u8 room, u8 hospital)
{
    s32 count;
    s32 target;
    s32 diff;

    count = max_price_for_treatment(room, hospital);
    target = count * allhosps[hospital].rooms[room].unknown10 / 10000;
    if (target - allhosps[hospital].rooms[room].unknownC < 0) {
        diff = -(target - allhosps[hospital].rooms[room].unknownC);
    } else {
        diff = target - allhosps[hospital].rooms[room].unknownC;
    }
    if (target / 100 <= diff) {
        allhosps[hospital].rooms[room].unknownC = target;
    }
}

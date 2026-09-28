#include "hosp.h"

struct PanelIcon {
    s16 x;
    s16 y;
    s16 x2;
    s16 y2;
    u8 sprite;
    u8 unknown9[2];
    u16 unknownB;
    u8 pressed;
};

struct PopupIconControl {
    u32 unknown0;
};

struct PanelLayout {
    u8 unknown0[0x150];
    s32 unknown150;
    s32 unknown154;
    s32 unknown158;
    u8 unknown15c[4];
    s32 unknown160;
    s32 unknown164;
    s32 unknown168;
    u8 unknown16c[0xd4];
    s32 unknown240;
    u8 unknown244[4];
    s32 unknown248;
    u8 unknown24c[4];
    s32 unknown250;
    s32 unknown254;
    s32 unknown258;
    s32 unknown25c;
    s32 unknown260;
    u8 unknown264[0x1bc];
    s32 unknown420;
    s32 unknown424;
    u8 unknown428[8];
    s32 unknown430;
    s32 unknown434;
    s32 unknown438;
    s32 unknown43c;
    s32 unknown440;
    s32 unknown444;
    Point slots[9];
    u8 unknown490[0x10];
    s32 unknown4a0;
    s32 unknown4a4;
    s32 unknown4a8;
    s32 unknown4ac;
    u8 unknown4b0[0x10];
    s32 unknown4c0;
    s32 unknown4c4;
    s32 unknown4c8;
    s32 unknown4cc;
    u8 unknown4d0[0xa0];
    s32 unknown570;
    u8 unknown574[8];
    s32 unknown57c;
    u8 unknown580[4];
    s32 unknown584;
    u8 unknown588[0xc];
    s32 unknown594;
    s32 unknown598;
    s32 unknown59c;
    u8 unknown5a0[0x40];
    s32 unknown5e0;
    s32 unknown5e4;
    s32 unknown5e8;
    s32 unknown5ec;
};

void draw_panel(u8 all);
void choose_panel_icons(void);
void draw_money_on_panel(void);
void draw_date_on_panel(void);
void draw_rep_on_panel(void);
void panel_flash(void);
void draw_ratfest_on_panel(void);
u8 display_panel_special_info(void);
void display_panel_person_info(void);
void display_panel_staff_info(void);
void display_panel_object_info(void);
void display_panel_queue_info(void);
void display_panel_mayor_info(void);
void display_panel_usage_info(void);
void maintain_panel_popup(void);

extern Icons panel_icons_over[];
extern Icons panel_icons_normal[];
extern Icons panel_icons_ratfest[];
extern PopupIconControl popup_icons[];
extern u8 panel_emergency_spr[];

extern "C" {

extern u8 D_001553b4;
extern u8 D_00155e01;
extern u8 D_00155e0d;
extern u8 D_00155d74;
extern u16 D_00155e21;
extern char **D_001602cc;
extern s32 D_001553f3;
extern u8 D_00155434;
extern u8 D_00155435;
extern u16 D_00155436;
extern char **D_0016029c;
extern TbScreenMode D_0015f3f5;
extern char D_0012031d[];
extern u16 D_0015831c;
extern u16 D_0015536e;
extern Event D_001549d0[];
extern u16 D_00158204;
extern u16 D_00158206;
extern u8 D_00156910;
extern u16 D_00188c74;
extern s32 D_00188c38;
extern PanelLayout *D_0015f40c;
extern PanelLayout *D_0015f410;
extern TbScreenMode D_0015f3f4;
extern TbSprite *D_00153f80;
extern TbSprite *D_00153f88;
extern TbLoadFiles D_0013f680[];
extern char D_00120310[];
extern char D_00120327[];
extern char D_0015544e[];
extern u8 D_001553f7;
extern u8 D_00155e1e;
extern u16 D_00155e1f;
extern char **D_001602a0;
extern u8 D_001553b5;
extern char D_00120323[];
extern u8 D_00155d7a;
extern u8 D_00155e0e;
extern u8 D_00155e0f;
extern u8 D_00155e10;
extern Point *D_0015f428;
extern char **D_00160298;
extern char **D_00160318;
extern char D_00120316[];
extern char D_00120317[];
extern char **D_001602a4;
extern u16 D_0012dd7c[];
extern u8 D_0015831b;
extern u8 D_00155e23;
extern u8 D_0012d778[];
extern u16 D_0012ddd4;
extern char D_00120300[];
extern char D_00120304[];
extern char D_00120308[];
extern char D_0012030c[];

void fn_000c93bf(char *dst, char *src);
void fn_000bce44(TbLoadFiles *files);
void fn_00035824(void);
void fn_00036594(s32 value, char *text);
void fn_00036730(char *text, u8 *line, u8 lines);
void LbSpriteReset(TbSprite *start, TbSprite *end, void *data);
void LbTextSetClipWindow(s32 x, s32 y, s32 w, s32 h);
void fn_000ccac4(s32 x0, s32 y0, s32 x1, s32 y1, u8 colour);
#pragma aux fn_000ccac4 "__wcpp_2_undefed_cdtor__";

}

// FUNCTION: 0x00031c70
void update_panel(void)
{
    u8 drawn;

    drawn = 0;
    switch (D_00155e01) {
    case 1:
        if (statvar.unknown6 <= mouse_dy && !D_001553b4)
            D_00156910 = 1;
        break;
    case 2:
        if (statvar.unknown6 > mouse_dy || D_001553b4)
            D_00156910 = 1;
        break;
    }
    if (D_001553f3 != myhosp->f35)
        D_00156910 = 1;
    if (D_00156910 == 1) {
        choose_panel_icons();
        LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
        D_00156910 = 0;
        drawn = 1;
        draw_panel(1);
        set_all_refresh_zones_in_window(0, statvar.unknown4, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
    }
    if (!D_001553b4) {
        if (!display_panel_special_info() && D_00155e01 == 1) {
            switch (D_00155434) {
            case 0:
                if (D_00155435 != 0 || drawn)
                    clear_panel_info_bar();
                break;
            case 2:
                display_panel_person_info();
                break;
            case 1:
                display_panel_staff_info();
                break;
            case 3:
                display_panel_object_info();
                break;
            case 4:
                display_panel_queue_info();
                break;
            case 5:
                display_panel_mayor_info();
                break;
            case 6:
                display_panel_usage_info();
                break;
            }
        }
    }
}

// FUNCTION: 0x00031e20
void draw_panel(u8 all)
{
    s32 unused;
    s32 i;

    unused = 0;
    if (D_001553b4) {
        for (i = 0; ((PanelIcon *)panel)[i].sprite > 0; i++)
            reqspr_draw(((PanelIcon *)panel)[i].x, ((PanelIcon *)panel)[i].y, &game_panel[((PanelIcon *)panel)[i].sprite]);
        draw_ratfest_on_panel();
    } else if (all == 1 || display_mode != 0) {
        for (i = 0; ((PanelIcon *)panel)[i].sprite > 0; i++)
            reqspr_draw(((PanelIcon *)panel)[i].x, ((PanelIcon *)panel)[i].y, &game_panel[((PanelIcon *)panel)[i].sprite]);
        draw_money_on_panel();
        draw_date_on_panel();
        draw_rep_on_panel();
    }
    panel_flash();
}

// FUNCTION: 0x00031f5c
void choose_panel_icons(void)
{
    if (statvar.unknown6 <= mouse_dy && !D_001553b4 && !gamevar.unknown21) {
        panel = panel_icons_over;
        D_00155e01 = 2;
    } else {
        panel = panel_icons_normal;
        D_00155e01 = 1;
    }
    if (D_001553b4)
        panel = panel_icons_ratfest;
    statvar.unknown6 = ((PanelIcon *)panel)[1].y;
    D_00158204 = D_0015f410->unknown240;
    D_00158206 = statvar.unknown6;
}

// FUNCTION: 0x00031ffc
void setup_panel_icons(const u8 mode)
{
    u8 i;
    u16 x;
    s8 pass;

    i = 0;
    x = 0;
    pass = 2;
    if (!D_001553b4) {
        panel = panel_icons_over;
    } else {
        panel = panel_icons_ratfest;
        --pass;
    }
    while (--pass >= 0) {
        if (pass == 0 && !D_001553b4) {
            panel = panel_icons_normal;
            x = 0;
            i = 0;
        }
        while (((PanelIcon *)panel)[i].sprite > 0) {
            ((PanelIcon *)panel)[i].y = lbScreenModeInfo[mode].height - game_panel[((PanelIcon *)panel)[i].sprite].height;
            ((PanelIcon *)panel)[i].y2 = lbScreenModeInfo[mode].height;
            ((PanelIcon *)panel)[i].x = x;
            ((PanelIcon *)panel)[i].x2 = x + game_panel[((PanelIcon *)panel)[i].sprite].width;
            x += game_panel[((PanelIcon *)panel)[i].sprite].width;
            i++;
        }
    }
    D_00155e21 = 0;
    D_00155e0d = 1;
}

// FUNCTION: 0x000321b4
u8 which_icon_is_mouse_over(void)
{
    u8 i;
    u8 over;

    i = 0;
    over = 0x63;
    if (statvar.unknown6 <= mouse_dy && !D_001553b4) {
        while (((PanelIcon *)panel)[i].sprite > 0) {
            if (((PanelIcon *)panel)[i].x <= mouse_dx && ((PanelIcon *)panel)[i].x2 >= mouse_dx) {
                over = i;
                break;
            }
            i++;
        }
        D_00156910 = 1;
    }
    return over;
}

// FUNCTION: 0x00032268
void panel_ml_button(const u16 icon)
{
    if (icon != 0x63) {
        if (((PanelIcon *)panel)[icon].unknownB != 0x40) {
            reqspr_draw(((PanelIcon *)panel)[icon].x, ((PanelIcon *)panel)[icon].y, &game_panel[((PanelIcon *)panel)[icon].pressed]);
            D_00156910 = 1;
        }
    }
}

// FUNCTION: 0x00032308
void panel_l_button(const u16 icon)
{
    if (D_0015536e == 0xe) {
        play_sound(10, 8, -1, -1);
    } else if (icon != 0x63) {
        switch (icon) {
        case 0:
            D_001549d0[player].type = 0x33;
            D_001549d0[player].value = 6;
            play_sound(10, 0x43, -1, -1);
            break;
        case 2:
            if (!(D_0015536e == 1 || D_0015536e == 2 || D_0015536e == 3 || D_0015536e == 0xe || D_0015536e == 0x10)) {
                set_mouse_mode(0);
                D_001549d0[player].type = 0x37;
                play_sound(10, 0x43, -1, -1);
            } else {
                play_sound(10, 8, -1, -1);
            }
            break;
        case 5:
            if (!(D_0015536e == 1 || D_0015536e == 2 || D_0015536e == 3 || D_0015536e == 0xe || D_0015536e == 0x10)) {
                set_mouse_mode(0);
                D_001549d0[player].type = 0x2e;
                play_sound(10, 0x43, -1, -1);
            } else {
                play_sound(10, 8, -1, -1);
            }
            break;
        case 4:
            if (D_0015536e == 0 && crequester.unknown2 == 0x63) {
                set_mouse_mode(0xc);
                allhosps[player].unknown11b7 = 0;
                play_sound(10, 0x43, -1, -1);
            } else if (D_0015536e == 0xc && crequester.unknown2 == 0x63) {
                set_mouse_mode(0);
                allhosps[player].unknown11b7 = 0;
                play_sound(10, 0x43, -1, -1);
            } else {
                play_sound(10, 8, -1, -1);
            }
            break;
        case 3:
            if (!(D_0015536e == 1 || D_0015536e == 2 || D_0015536e == 3 || D_0015536e == 0xe || D_0015536e == 0x10)) {
                set_mouse_mode(0);
                D_001549d0[player].type = 0x62;
                play_sound(10, 0x43, -1, -1);
            } else {
                play_sound(10, 8, -1, -1);
            }
            break;
        case 6:
            D_001549d0[player].type = 0x33;
            D_001549d0[player].value = 5;
            play_sound(10, 0x43, -1, -1);
            break;
        case 7:
            if (crequester.unknown2 != 5 && (crequester.unknown2 != 10 || crequester.unknown4 == 1)) {
                display_mode = 4;
                town.unknown2 = 0;
                play_sound(10, 0x43, -1, -1);
            } else {
                play_sound(10, 8, -1, -1);
            }
            break;
        case 8:
            D_001549d0[player].type = 0x33;
            D_001549d0[player].value = 1;
            play_sound(10, 0x43, -1, -1);
            break;
        case 9:
            if (listhead[player].rooms[0x1c]) {
                D_001549d0[player].type = 0x33;
                D_001549d0[player].value = 3;
                play_sound(10, 0x43, -1, -1);
            } else {
                play_sound(10, 8, -1, -1);
            }
            break;
        case 10:
            D_001549d0[player].type = 0x33;
            D_001549d0[player].value = 7;
            play_sound(10, 0x43, -1, -1);
            break;
        case 11:
            D_001549d0[player].type = 0x33;
            D_001549d0[player].value = 4;
            play_sound(10, 0x43, -1, -1);
            break;
        case 12:
            D_001549d0[player].type = 0x33;
            D_001549d0[player].value = 2;
            play_sound(10, 0x43, -1, -1);
            break;
        case 1:
        default:
            play_sound(10, 8, -1, -1);
            break;
        }
        D_00156910 = 1;
    }
}


// FUNCTION: 0x000328c0
void panel_r_button(const u16 icon)
{
    if (D_0015536e == 0xe) {
        play_sound(10, 8, -1, -1);
    } else if (icon != 0x63) {
        switch (icon) {
        case 0:
            D_001549d0[player].type = 0x33;
            D_001549d0[player].value = 0;
            play_sound(10, 0x43, -1, -1);
            break;
        case 5:
            D_001549d0[player].type = 0x33;
            D_001549d0[player].value = 5;
            play_sound(10, 0x43, -1, -1);
            break;
        default:
            play_sound(10, 8, -1, -1);
            break;
        }
        D_00156910 = 1;
    }
}

// FUNCTION: 0x000329d4
void draw_money_on_panel(void)
{
    u8 digits;
    u8 height;
    u32 diff;
    s32 div;
    char buf[16];

    digits = 0;
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    LbTextSetClipWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0;
    set_font_from_cache(5);
    height = lbFontPtr ? lbFontPtr[18].height : 0;
    if (D_001553f3 != myhosp->f35) {
        diff = fn_000cb2db(D_001553f3 - myhosp->f35);
        for (div = 1000000; div >= 0; div /= 10) {
            if (diff >= div) {
                digits++;
                break;
            }
        }
        LbTextSetClipWindow(D_0015f40c->unknown150 + D_0015f410->unknown248 * digits, D_0015f40c->unknown154,
                            D_0015f40c->unknown158 - D_0015f410->unknown248 * digits, height);
        sprintf(buf, D_00120300, myhosp->f35);
        LbTextDraw(D_0015f410->unknown250, D_0015f410->unknown254 - (height - D_001553f7 * 6), buf);
        sprintf(buf, D_00120304, D_001553f3);
        LbTextDraw(D_0015f410->unknown250, D_0015f410->unknown254 + D_001553f7 * 6, buf);
        if (digits) {
            LbTextSetClipWindow(D_0015f40c->unknown160, D_0015f40c->unknown164,
                                D_0015f410->unknown248 * (digits - 3) + D_0015f40c->unknown168, height);
            sprintf(buf, D_00120308, myhosp->f35);
            LbTextDraw(D_0015f410->unknown250, D_0015f410->unknown254, buf);
        }
        D_001553f7++;
        if (D_001553f7 == 4) {
            D_001553f7 = 0;
            D_001553f3 = myhosp->f35;
            D_00156910 = 1;
        }
    } else {
        LbTextSetClipWindow(D_0015f40c->unknown150, D_0015f40c->unknown154, D_0015f40c->unknown158, height);
        sprintf(buf, D_0012030c, myhosp->f35);
        LbTextDraw(D_0015f410->unknown250, D_0015f410->unknown254, buf);
    }
}


// FUNCTION: 0x00032cc4
void draw_date_on_panel(void)
{
    char buf[12];

    set_font_from_cache(6);
    D_00188c74 = 0x100;
    LbTextSetWindow(D_0015f40c->unknown4c0, D_0015f40c->unknown4c4, D_0015f40c->unknown4c8, D_0015f40c->unknown4cc);
    sprintf(buf, D_00120310, mycontrol->unknown64f, month_text[mycontrol->unknown650]);
    LbTextDraw(D_0015f410->unknown440, D_0015f410->unknown444, buf);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0;
}

// FUNCTION: 0x00032da0
void draw_rep_on_panel(void)
{
    s32 offset;

    offset = (D_0015f410->unknown260 - D_0015f410->unknown258) * myhosp->unknownc7e / 1000;
    D_00188c74 = 0;
    reqspr_draw(D_0015f410->unknown258 + offset, D_0015f410->unknown25c, &game_panel[36]);
}

// NONMATCHING: register choice
// FUNCTION: 0x00032e20
void draw_emergency(void)
{
    u8 height;
    u8 off;

    off = 0;
    if (!D_001553b4) {
        D_00188c74 = 0;
        LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
        set_font_from_cache(6);
        height = lbFontPtr ? lbFontPtr[25].height : 0;
        if (myhosp->f1d9 || (myhosp->unknown1257 && !(D_0015831b & 1)) || myhosp->f207 || myhosp->f20e || (s8)myhosp->f1e2 > 1) {
            reqspr_draw(D_0015f410->unknown420, D_0015f410->unknown424, &watch[13]);
            set_all_refresh_zones_in_window(D_0015f410->unknown420, D_0015f410->unknown424,
                                            watch[13].width + D_0015f410->unknown420, watch[13].height + D_0015f410->unknown424, 3);
            if (myhosp->f1d9) {
                if ((s16)myhosp->f1d9 / 8 >= 6) {
                    reqspr_draw(D_0015f410->unknown420 + D_0015f410->unknown438, D_0015f410->unknown424 + D_0015f410->unknown43c, &watch[1]);
                    set_all_refresh_zones_in_window(D_0015f410->unknown420 + D_0015f410->unknown438, D_0015f410->unknown424 + D_0015f410->unknown43c,
                                                    watch[1].width + (D_0015f410->unknown420 + D_0015f410->unknown438),
                                                    watch[1].height + (D_0015f410->unknown424 + D_0015f410->unknown43c), 3);
                }
                reqspr_draw(D_0015f410->unknown420 + D_0015f410->unknown438, D_0015f410->unknown424 + D_0015f410->unknown43c,
                            &watch[D_0012d778[(s16)myhosp->f1d9 / 8]]);
                set_all_refresh_zones_in_window(D_0015f410->unknown420 + D_0015f410->unknown438, D_0015f410->unknown424 + D_0015f410->unknown43c,
                                                watch[D_0012d778[(s16)myhosp->f1d9 / 8]].width + (D_0015f410->unknown420 + D_0015f410->unknown438),
                                                watch[D_0012d778[(s16)myhosp->f1d9 / 8]].height + (D_0015f410->unknown424 + D_0015f410->unknown43c), 3);
            } else if (myhosp->f207) {
                if (myhosp->f1d9)
                    off = watch[15].width / 4 + watch[15].width;
                if (myhosp->f20b && !myhosp->f207)
                    D_00188c74 = 4;
                reqspr_draw(off + D_0015f410->unknown430, D_0015f410->unknown434, &watch[14]);
                set_all_refresh_zones_in_window(off + D_0015f410->unknown430, D_0015f410->unknown434,
                                                watch[14].width + (D_0015f410->unknown430 + off), watch[14].height + D_0015f410->unknown434, 3);
                D_00188c74 = 0;
                if (over_hotspot(mouse_dx, mouse_dy, off + D_0015f410->unknown430, D_0015f410->unknown434, watch[14].width, watch[14].height)) {
                    reqspr_draw(D_0015f410->unknown430 + off, D_0015f410->unknown434, &watch[15]);
                    set_all_refresh_zones_in_window(D_0015f410->unknown430 + off, D_0015f410->unknown434,
                                                    watch[15].width + (off + D_0015f410->unknown430), watch[15].height + D_0015f410->unknown434, 3);
                    if (!D_00155e23)
                        play_sound(10, 0x44, -1, -1);
                    D_00155e23 = 1;
                } else {
                    D_00155e23 = 0;
                }
                if (myhosp->f20b && !myhosp->f207)
                    D_00188c74 = 4;
                if ((s16)myhosp->f207 / 8 >= 6) {
                    reqspr_draw(D_0015f410->unknown420 + off + D_0015f410->unknown438, D_0015f410->unknown424 + D_0015f410->unknown43c, &watch[1]);
                    set_all_refresh_zones_in_window(D_0015f410->unknown420 + off + D_0015f410->unknown438, D_0015f410->unknown424 + D_0015f410->unknown43c,
                                                    watch[1].width + (off + D_0015f410->unknown420 + D_0015f410->unknown438),
                                                    D_0015f410->unknown424 + D_0015f410->unknown43c + watch[1].height, 3);
                }
                reqspr_draw(D_0015f410->unknown420 + off + D_0015f410->unknown438, D_0015f410->unknown424 + D_0015f410->unknown43c,
                            &watch[D_0012d778[(s16)myhosp->f207 / 8]]);
                set_all_refresh_zones_in_window(D_0015f410->unknown420 + off + D_0015f410->unknown438, D_0015f410->unknown424 + D_0015f410->unknown43c,
                                                watch[D_0012d778[(s16)myhosp->f207 / 8]].width + (D_0015f410->unknown420 + off + D_0015f410->unknown438),
                                                watch[D_0012d778[(s16)myhosp->f207 / 8]].height + (D_0015f410->unknown424 + D_0015f410->unknown43c), 3);
                D_00188c74 = 0;
            } else if (myhosp->unknown1257) {
                reqspr_draw(D_0015f410->unknown430, D_0015f410->unknown434, &watch[16]);
                set_all_refresh_zones_in_window(D_0015f410->unknown430, D_0015f410->unknown434,
                                                watch[16].width + D_0015f410->unknown430, watch[16].height + D_0015f410->unknown434, 3);
                if (over_hotspot(mouse_dx, mouse_dy, D_0015f410->unknown430, D_0015f410->unknown434, watch[14].width, watch[14].height)) {
                    reqspr_draw(D_0015f410->unknown430, D_0015f410->unknown434, &watch[17]);
                    set_all_refresh_zones_in_window(D_0015f410->unknown430, D_0015f410->unknown434,
                                                    watch[17].width + D_0015f410->unknown430, watch[17].height + D_0015f410->unknown434, 3);
                    if (!D_00155e23)
                        play_sound(10, 0x44, -1, -1);
                    D_00155e23 = 1;
                } else {
                    D_00155e23 = 0;
                }
                if (myhosp->unknown1257 / (D_0012ddd4 / 13) >= 6) {
                    reqspr_draw(D_0015f410->unknown420 + D_0015f410->unknown438, D_0015f410->unknown424 + D_0015f410->unknown43c, &watch[1]);
                    set_all_refresh_zones_in_window(D_0015f410->unknown420 + D_0015f410->unknown438, D_0015f410->unknown424 + D_0015f410->unknown43c,
                                                    watch[1].width + (D_0015f410->unknown420 + D_0015f410->unknown438),
                                                    watch[1].height + (D_0015f410->unknown424 + D_0015f410->unknown43c), 3);
                }
                reqspr_draw(D_0015f410->unknown420 + D_0015f410->unknown438, D_0015f410->unknown424 + D_0015f410->unknown43c,
                            &watch[D_0012d778[myhosp->unknown1257 / (D_0012ddd4 / 13)]]);
                set_all_refresh_zones_in_window(D_0015f410->unknown420 + D_0015f410->unknown438, D_0015f410->unknown424 + D_0015f410->unknown43c,
                                                watch[D_0012d778[myhosp->unknown1257 / (D_0012ddd4 / 13)]].width + (D_0015f410->unknown420 + D_0015f410->unknown438),
                                                watch[D_0012d778[myhosp->unknown1257 / (D_0012ddd4 / 13)]].height + (D_0015f410->unknown424 + D_0015f410->unknown43c), 3);
            }
        }
    }
}


// FUNCTION: 0x00033a40
void clear_panel_info_bar(void)
{
    u8 i;

    if (!D_001553b4) {
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
        reqspr_draw(((PanelIcon *)panel)[13].x, ((PanelIcon *)panel)[13].y, &game_panel[((PanelIcon *)panel)[13].sprite]);
        set_all_refresh_zones_in_window(((PanelIcon *)panel)[13].x, ((PanelIcon *)panel)[13].y,
                                        ((PanelIcon *)panel)[13].x + game_panel[((PanelIcon *)panel)[13].sprite].width,
                                        ((PanelIcon *)panel)[13].y + game_panel[((PanelIcon *)panel)[13].sprite].height, 3);
        for (i = 0; ((PanelIcon *)panel)[i].sprite > 0; i++) {
            if (((PanelIcon *)panel)[i].sprite == 0xd) {
                reqspr_draw(((PanelIcon *)panel)[i].x, ((PanelIcon *)panel)[i].y, &game_panel[((PanelIcon *)panel)[i].sprite]);
                set_all_refresh_zones_in_window(((PanelIcon *)panel)[i].x, ((PanelIcon *)panel)[i].y,
                                                ((PanelIcon *)panel)[i].x + game_panel[((PanelIcon *)panel)[i].sprite].width,
                                                ((PanelIcon *)panel)[i].y + game_panel[((PanelIcon *)panel)[i].sprite].height, 3);
            }
        }
    }
}


// FUNCTION: 0x00033c8c
void display_panel_person_info(void)
{
    u8 height;
    u8 line;
    u8 lines;
    char *text;
    RoomInfo *room;
    Thing *w;
    char buf[100];

    w = &thing[D_00155436];
    line = 0;
    height = D_0015f410->unknown57c;
    clear_panel_info_bar();
    LbTextSetWindow(panel->x, panel->y, lbDisplay.unknown18 - panel->x, lbDisplay.unknown1c - panel->y);
    set_font_from_cache(1);
    text = 0;
    if (w->unknown118 & 4)
        text = info_bar_text[4];
    else if (w->unknown118 & 8)
        text = info_bar_text[15];
    else if (w->unknown118 & 0x10)
        text = info_bar_text[16];
    else if (w->unknown118 & 0x80)
        text = info_bar_text[19];
    if (text) {
        line = 1;
        fn_00036730(text, &line, 3);
        return;
    }
    if (D_0015f3f5 == SCREEN_MODE_13)
        lines = 2;
    else
        lines = 1;
    if (w->f74 & 0x10) {
        fn_00036730(info_bar_text[5], &line, lines);
    } else if (w->f74 & 0x80) {
        if (w->f75 & 4)
            fn_00036730(info_bar_text[17], &line, lines);
        else
            fn_00036730(info_bar_text[18], &line, lines);
    } else {
        room = &room_info[w->unknown140];
        if (room > room_info && room < end_room_info) {
            sprintf(buf, info_bar_text[6], D_001602cc[rooms[room->type].id]);
            fn_00036730(buf, &line, lines);
        }
        room = &room_info[w->unknown89];
        if (room > room_info && room < end_room_info && w->unknown89 != w->unknown140) {
            sprintf(buf, info_bar_text[7], D_001602cc[rooms[room->type].id]);
            fn_00036730(buf, &line, lines);
        }
    }
    if (w->f75 & 0x20)
        fn_00036730(info_bar_text[8], &line, lines);
    else if (w->f75 & 1)
        fn_00036730(info_bar_text[24], &line, lines);
    else if (w->f75 & 0x40)
        fn_00036730(info_bar_text[9], &line, lines);
    else if (w->f75 & 0x80)
        fn_00036730(info_bar_text[10], &line, lines);
    else if (w->unknown118 & 1)
        fn_00036730(info_bar_text[11], &line, lines);
    else if (w->f74 & 0x20)
        fn_00036730(info_bar_text[25], &line, lines);
    if (line < 3 && w->style != 0) {
        if (w->f74 & 0x40) {
            line = 2;
            sprintf(buf, info_bar_text[22], D_001602a4[w->style]);
            fn_00036730(buf, &line, 3);
        } else if (w->flags32 & 0x21000) {
            u8 k;

            k = w->unknown169;
            line = 2;
            if (w->unknown118 & 2)
                sprintf(buf, info_bar_text[12], D_001602a4[k]);
            else
                sprintf(buf, info_bar_text[13], D_001602a4[k]);
            fn_00036730(buf, &line, 3);
        } else {
            fn_00036594(w->unknown146 * 100 / D_0012dd7c[0], info_bar_text[14]);
        }
    }
}


// NONMATCHING: register choice, u8 local slot order
// FUNCTION: 0x00034118
void display_panel_staff_info(void)
{
    u8 i;
    u8 height;
    u8 line;
    u8 pct;
    u8 n;
    Thing *w;
    RoomInfo *room;
    char buf[100];
    char names[4][50];

    w = &thing[D_00155436];
    line = 0;
    height = D_0015f410->unknown57c;
    if (w->owner != 0x63) {
        clear_panel_info_bar();
        LbTextSetWindow(panel->x, panel->y, lbDisplay.unknown18 - panel->x, lbDisplay.unknown1c - panel->y);
        set_font_from_cache(1);
        fn_000bbff0(buf, D_00160298[w->subtype]);
        if ((w->subtype == 1 || w->subtype == 4) && (w->unknown124 & 0x1e0)) {
            fn_000bbff0(buf, D_00120316);
            n = 0;
            if (w->unknown124 & 0x80)
                fn_000bbff0(names[n++], D_00160318[7]);
            if (w->unknown124 & 0x100)
                fn_000bbff0(names[n++], D_00160318[8]);
            if (w->unknown124 & 0x20)
                fn_000bbff0(names[n++], D_00160318[5]);
            if (w->unknown124 & 0x40) {
                if (n == 3)
                    fn_000bbff0(names[n++], info_bar_text[26]);
                else
                    fn_000bbff0(names[n++], D_00160318[6]);
            }
            fn_000bbff0(buf, names[0]);
            for (i = 1; i < n; i++)
                sprintf(buf, D_00120317, buf, names[i]);
        }
        fn_00036730(buf, &line, 2);
        if (D_0015f3f5 == SCREEN_MODE_13) {
            room = &room_info[w->unknown89];
            if (room > room_info) {
                sprintf(buf, info_bar_text[7], D_001602cc[rooms[room->type].id]);
                fn_00036730(buf, &line, 2);
            } else if ((w->subtype == 1 && w->kind == 0x14) || w->state_b == 0x20) {
                fn_00036730(info_bar_text[23], &line, 2);
            } else if (w->kind == 0x34) {
                fn_00036730(info_bar_text[27], &line, 2);
            }
        }
        pct = allhosps[w->owner].f21d * 100 / 1000;
        pct > 100 ? pct = 100 : 0;
        fn_00036594(w->f78 * 100 / 1000, info_bar_text[28]);
        fn_000ccac4((D_0015f40c->unknown5e8 - 2) * pct / 100 + (D_0015f40c->unknown5e0 + panel->x + 1),
                    D_0015f40c->unknown5e4 + (D_0015f410->unknown584 + panel->y + D_0015f410->unknown57c * 2 + 1),
                    panel->x + D_0015f40c->unknown5e0 + 1 + (D_0015f40c->unknown5e8 - 2) * pct / 100,
                    D_0015f410->unknown584 + panel->y + D_0015f410->unknown57c * 2 + D_0015f40c->unknown5ec - 1 + D_0015f40c->unknown5e4,
                    color.unknown7);
    }
}


// FUNCTION: 0x000345e0
void display_panel_object_info(void)
{
    Thing *w;
    u8 line;
    char buf[100];

    w = &thing[D_00155436];
    line = 0;
    clear_panel_info_bar();
    if (w->renderer == 6) {
        LbTextSetWindow(panel->x, panel->y, lbDisplay.unknown18 - panel->x, lbDisplay.unknown1c - panel->y);
        set_font_from_cache(1);
        fn_00036730(D_0016029c[w->subtype], &line, 3);
        if (D_0015f3f5 == SCREEN_MODE_13) {
            sprintf(buf, info_bar_text[30], w->value);
            fn_00036730(buf, &line, 3);
            sprintf(buf, info_bar_text[31], w->mode);
            fn_00036730(buf, &line, 3);
        } else {
            sprintf(buf, info_bar_text[31], w->mode);
            sprintf(buf, D_0012031d, buf, w->value);
            line = 2;
            fn_00036730(buf, &line, 3);
        }
    }
}

// FUNCTION: 0x0003474c
void display_panel_queue_info(void)
{
    Thing *w;
    RoomInfo *room;
    u8 line;
    u8 height;
    char buf[100];

    w = &thing[D_00155436];
    line = 0;
    height = D_0015f410->unknown57c;
    if (w->renderer == 4)
        room = w->room;
    else
        room = &room_info[w->unknown89];
    if (room > room_info) {
        clear_panel_info_bar();
        LbTextSetWindow(panel->x, panel->y, lbDisplay.unknown18 - panel->x, lbDisplay.unknown1c - panel->y);
        set_font_from_cache(1);
        fn_00036730(D_001602cc[rooms[room->type].id], &line, 3);
        sprintf(buf, info_bar_text[32], room->unknown31);
        if (D_0015f3f5 != SCREEN_MODE_13)
            line = 2;
        fn_00036730(buf, &line, 3);
        if (D_0015f3f5 == SCREEN_MODE_13) {
            sprintf(buf, info_bar_text[33], room->unknown32);
            fn_00036730(buf, &line, 3);
        }
    }
}

// FUNCTION: 0x000348d4
void display_panel_mayor_info(void)
{
    Thing *w;
    u8 line;

    w = &thing[D_00155436];
    line = 0;
    clear_panel_info_bar();
    LbTextSetWindow(panel->x, panel->y, lbDisplay.unknown18 - panel->x, lbDisplay.unknown1c - panel->y);
    set_font_from_cache(1);
    if (w->renderer == 0x14) {
        fn_00036730(info_bar_text[20], &line, 3);
        fn_00036730(mayor_names[allhosps[w->owner].f1f9], &line, 3);
    } else if (w->renderer == 0x16) {
        fn_00036730(info_bar_text[21], &line, 3);
    }
}

// FUNCTION: 0x000349e8
void display_panel_usage_info(void)
{
    Thing *w;
    u8 line;
    char buf[100];

    w = &thing[D_00155436];
    line = 0;
    clear_panel_info_bar();
    LbTextSetWindow(panel->x, panel->y, lbDisplay.unknown18 - panel->x, lbDisplay.unknown1c - panel->y);
    set_font_from_cache(1);
    fn_00036730(D_0016029c[w->subtype], &line, 3);
    if (D_0015f3f5 != SCREEN_MODE_13)
        line = 2;
    sprintf(buf, info_bar_text[31], w->mode);
    fn_00036730(buf, &line, 3);
}

// NONMATCHING: control flow of the p->flash update: the target routes the (flash != 1 || D_00155e0d == 3) test and the following while loop's tail through jmp stubs
// FUNCTION: 0x00034acc
void draw_popup_icons(void)
{
    u8 redraw;
    u8 no_flash;
    u8 j;
    s32 y;
    s32 y1;
    PlayerCall *p;
    TbSprite *spr;
    s32 i;
    s32 x;
    s32 x1;

    redraw = 0;
    no_flash = 1;
    if (!D_001553b4) {
        maintain_panel_popup();
        D_00155d7a = 0;
        if (D_0015f3f5 == SCREEN_MODE_13) {
            LbScreenSetGraphicsWindow(0xb1, ((PanelIcon *)panel)[0].y, 100, 100);
            LbDrawBox(0, 0, 0x1c, 3, color.unknown8);
        }
        D_00155e10++;
        if (panel_emergency_spr[D_00155e10] == 0)
            D_00155e10 = 0;
        switch (D_00155e0d) {
        case 1:
        case 2:
        case 3:
        case 4:
            redraw = 1;
            break;
        }
        LbScreenSetGraphicsWindow(0, ((PanelIcon *)panel)[0].y - game_panel[popup_icons[1].unknown0].height + D_0015f410->unknown4ac,
                                  D_00188c34, game_panel[popup_icons[1].unknown0].height);
        for (i = 1; popup_icons[i].unknown0 != 0; i++) {
            if (listhead[player].first[i]) {
                spr = &game_panel[popup_icons[i].unknown0];
                p = &playercall[listhead[player].first[i]];
                if (p->slot < 9) {
                    if (p->flash) {
                        if (p->flash > 0x22)
                            x = D_0015f410->slots[p->slot].x + D_0015f410->unknown4a8;
                        else
                            x = D_0015f410->slots[0].x + D_0015f410->unknown4a8;
                    } else {
                        x = D_0015f410->slots[p->slot].x + D_0015f410->unknown4a8;
                    }
                    y = 0;
                    if (p->flash) {
                        x += D_0015f428[p->flash].x;
                        y += D_0015f428[p->flash].y;
                        if (D_0015f428[p->flash].x == 0 && D_0015f428[p->flash].y == 0) {
                            p->flash = 0;
                        } else {
                            if (p->flash != 1 || D_00155e0d == 3)
                                p->flash++;
                            while (D_0015f428[p->flash].x < D_0015f410->slots[p->slot].x) {
                                if (D_0015f428[p->flash].x == 0 && D_0015f428[p->flash].y == 0) {
                                    if (p->flash > 0x22) {
                                        p->flash = 0;
                                    } else {
                                        switch (p->slot) {
                                        case 4:
                                        case 5:
                                        case 6:
                                        case 7:
                                            p->flash = 0;
                                            break;
                                        default:
                                            p->flash = 0x22;
                                            break;
                                        }
                                    }
                                    break;
                                }
                                p->flash++;
                            }
                        }
                    }
                    x1 = x + spr->width;
                    y1 = y + spr->height;
                    if (y) {
                        reqspr_draw(x, y, spr);
                        if (popup_icons[i].unknown0 == 0x2b)
                            reqspr_draw(x + D_0015f410->unknown598, y + D_0015f410->unknown59c, &game_panel[panel_emergency_spr[D_00155e10]]);
                        D_00155e0d = 2;
                        no_flash = 0;
                    }
                    redraw = 1;
                    y += ((PanelIcon *)panel)[0].y - game_panel[popup_icons[1].unknown0].height;
                    y1 += ((PanelIcon *)panel)[0].y - game_panel[popup_icons[1].unknown0].height;
                    if (mouse_dx >= x && mouse_dx < x1 && mouse_dy >= y && mouse_dy < y1)
                        D_00155d7a = i;
                    set_all_refresh_zones_in_window(x, y, x1, y1, 3);
                }
            }
        }
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
        if (no_flash)
            D_00155e0d = 4;
        if (redraw) {
            reqspr_draw(((PanelIcon *)panel)[0].x, ((PanelIcon *)panel)[0].y, &game_panel[((PanelIcon *)panel)[0].sprite]);
            reqspr_draw(((PanelIcon *)panel)[1].x, ((PanelIcon *)panel)[1].y, &game_panel[((PanelIcon *)panel)[1].sprite]);
            draw_money_on_panel();
            draw_date_on_panel();
            draw_rep_on_panel();
            set_all_refresh_zones_in_window(((PanelIcon *)panel)[0].x, ((PanelIcon *)panel)[0].y,
                                            ((PanelIcon *)panel)[0].x2, ((PanelIcon *)panel)[0].y2, 3);
        }
        LbScreenSetGraphicsWindow(0, ((PanelIcon *)panel)[0].y - game_panel[popup_icons[1].unknown0].height + D_0015f410->unknown4ac,
                                  D_00188c34, game_panel[popup_icons[1].unknown0].height);
        for (i = 1; popup_icons[i].unknown0 != 0; i++) {
            if (listhead[player].first[i]) {
                spr = &game_panel[popup_icons[i].unknown0];
                p = &playercall[listhead[player].first[i]];
                if (p->slot < 9) {
                    if (p->flash) {
                        if (p->flash > 0x22)
                            x = D_0015f410->slots[p->slot].x + D_0015f410->unknown4a8;
                        else
                            x = D_0015f410->slots[0].x + D_0015f410->unknown4a8;
                    } else {
                        x = D_0015f410->slots[p->slot].x + D_0015f410->unknown4a8;
                    }
                    y = 0;
                    if (p->flash) {
                        x += D_0015f428[p->flash].x;
                        y += D_0015f428[p->flash].y;
                    }
                    if (y == 0) {
                        x1 = x + spr->width;
                        y1 = y + spr->height;
                        reqspr_draw(x, y, spr);
                        if (popup_icons[i].unknown0 == 0x2b)
                            reqspr_draw(x + D_0015f410->unknown598, y + D_0015f410->unknown59c, &game_panel[panel_emergency_spr[D_00155e10]]);
                        y += ((PanelIcon *)panel)[0].y - game_panel[popup_icons[1].unknown0].height;
                        y1 += ((PanelIcon *)panel)[0].y - game_panel[popup_icons[1].unknown0].height;
                        set_all_refresh_zones_in_window(x, y, x1, y1, 3);
                    }
                }
            }
        }
        if (D_0015f3f5 == SCREEN_MODE_13)
            LbScreenSetGraphicsWindow(0xb1, ((PanelIcon *)panel)[0].y, 100, 100);
        x = D_0015f410->unknown4a0;
        y = D_0015f410->unknown4a4;
        switch (D_00155e0d) {
        case 2:
            if (D_00155e0e - 2 >= 0)
                D_00155e0e -= 2;
            else
                D_00155e0e = 0;
            if (D_00155e0e == 0) {
                D_00155e0d = 3;
                D_00155e0f = 0;
            } else {
                for (j = 0; j < D_00155e0e; j++) {
                    if (D_0015f3f5 == SCREEN_MODE_13)
                        reqspr_draw(x, y, &game_panel[41]);
                    x += game_panel[41].width;
                }
                if (D_0015f3f5 == SCREEN_MODE_13)
                    reqspr_draw(x, y, &game_panel[42]);
            }
            break;
        case 3:
            D_00155e0e = 0;
            D_00155e0f++;
            if (D_00155e0f > 0x28)
                D_00155e0d = 4;
            break;
        case 4:
            if (D_00155e0f <= 0x28) {
                D_00155e0f++;
            } else {
                if (D_00155e0e + 2 < 0x18)
                    D_00155e0e += 2;
                else
                    D_00155e0e = 0x19;
                for (j = 0; j < D_00155e0e; j++) {
                    if (D_0015f3f5 == SCREEN_MODE_13)
                        reqspr_draw(x, y, &game_panel[41]);
                    x += game_panel[41].width;
                }
                if (D_0015f3f5 == SCREEN_MODE_13)
                    reqspr_draw(x, y, &game_panel[42]);
                if (D_00155e0e >= 0x18)
                    D_00155e0d = 1;
            }
            break;
        case 1:
            if (D_0015f3f5 == SCREEN_MODE_13)
                reqspr_draw(x, y, &game_panel[40]);
            for (j = 0; j < 0x19; j++) {
                if (D_0015f3f5 == SCREEN_MODE_13)
                    reqspr_draw(x, y, &game_panel[41]);
                x += game_panel[41].width;
            }
            if (D_0015f3f5 == SCREEN_MODE_13)
                reqspr_draw(x, y, &game_panel[42]);
            D_00155e0e = 0x18;
            break;
        default:
            D_00155e0d = 1;
            break;
        }
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
        D_00188c74 = 0;
    }
}


// FUNCTION: 0x00035624
void delete_panel_popup(PlayerCall *p)
{
    u8 i;

    if (p->owner == player) {
        for (i = 0; i < 9; i++) {
            if (panel_popup_pos[i] == p - playercall)
                remove_panel_popup(i);
            if (panel_popup_pos[i] == 0)
                remove_panel_popup(i);
        }
    }
}

// NONMATCHING: register choice; target names panel_popup_pos[-1] as popup_id+0x10
// FUNCTION: 0x000356c4
void remove_panel_popup(const u8 slot)
{
    u8 i;
    PlayerCall *p;

    for (i = slot + 1; i < 9; i++) {
        p = &playercall[panel_popup_pos[i]];
        if (p->slot > 0)
            p->slot--;
        panel_popup_pos[i - 1] = panel_popup_pos[i];
    }
    panel_popup_pos[8] = 0;
}

// FUNCTION: 0x00035760
void add_panel_popup(PlayerCall *p)
{
    u8 i;

    fn_00035824();
    if (p->owner == player) {
        for (i = 0; panel_popup_pos[i] != 0; i++)
            ;
        if (i < 9) {
            p->slot = i;
            panel_popup_pos[i] = p - playercall;
            p->flash = 1;
            D_00155e0d = 2;
        } else {
            p->slot = 9;
        }
        play_sound(10, 0xdd, -1, -1);
    }
}

// FUNCTION: 0x00035824
void fn_00035824(void)
{
    u8 i;
    u16 j;
    PlayerCall *p;

    for (i = 0; i < 9; i++)
        panel_popup_pos[i] = 0;
    for (j = 1; popup_icons[j].unknown0 != 0; j++) {
        if (listhead[player].first[j]) {
            p = &playercall[listhead[player].first[j]];
            panel_popup_pos[p->slot] = p - playercall;
        }
    }
}

// NONMATCHING: register choice; target names panel_popup_pos[-1] as popup_id+0x10
// FUNCTION: 0x00035914
void maintain_panel_popup(void)
{
    u8 slot;
    u8 found;
    u16 j;
    u16 k;
    u16 i;
    PlayerCall *p;
    PlayerCall *q;

    fn_00035824();
    for (i = 0; i < 9; i++) {
        if (panel_popup_pos[i]) {
            p = &playercall[panel_popup_pos[i]];
            found = 1;
            for (j = 1; popup_icons[j].unknown0 != 0; j++) {
                if (panel_popup_pos[i] == listhead[player].first[j])
                    found = 0;
            }
            if (found)
                remove_panel_popup(i);
        } else {
            remove_panel_popup(i);
        }
    }
    for (j = 1; popup_icons[j].unknown0 != 0; j++) {
        if (listhead[player].first[j]) {
            p = &playercall[listhead[player].first[j]];
            if (p->slot < 9 && p->slot > 0) {
                if (!panel_popup_pos[p->slot - 1]) {
                    remove_panel_popup(p->slot - 1);
                    p->slot--;
                    panel_popup_pos[p->slot] = p - playercall;
                    p->flash = 1;
                }
            }
        }
    }
    for (j = 1; popup_icons[j].unknown0 != 0; j++) {
        if (listhead[player].first[j]) {
            p = &playercall[listhead[player].first[j]];
            if (p->slot < 9) {
                slot = p->slot;
                for (k = 1; popup_icons[k].unknown0 != 0; k++) {
                    if (listhead[player].first[k]) {
                        q = &playercall[listhead[player].first[k]];
                        if (q->slot < 9) {
                            if (q != p) {
                                if (slot == q->slot)
                                    q->slot++;
                            }
                        }
                    }
                }
            }
        }
    }
}


// FUNCTION: 0x00035c68
void panel_flash(void)
{
    set_all_refresh_zones_in_window(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    if (D_0015831c >= 0x10 && D_0015831c <= 0x1b) {
        D_00155d74++;
        if (D_00155d74 % 10 == 0) {
            reqspr_draw(((PanelIcon *)panel)[D_0015831c - 0x10].x, ((PanelIcon *)panel)[D_0015831c - 0x10].y, &game_panel[((PanelIcon *)panel)[D_0015831c - 0x10].pressed]);
            D_00155d74 = 0;
            D_00156910 = 1;
        }
    }
}

// FUNCTION: 0x00035d5c
void draw_ratfest_on_panel(void)
{
    u8 level;
    u8 s3;
    u8 s0;
    u8 s1;
    u8 s2;
    char buf[100];

    LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    set_font_from_cache(0x29);
    sprintf(buf, D_00120323, myhosp->fd27u);
    LbTextDraw(0x5d, 0x1be, buf);
    switch (D_001553b5) {
    case 0:
        reqspr_draw(((PanelIcon *)panel)[1].x, ((PanelIcon *)panel)[1].y + 0xb, &game_panel[((PanelIcon *)panel)[1].pressed]);
        break;
    case 1:
        reqspr_draw(((PanelIcon *)panel)[2].x, ((PanelIcon *)panel)[2].y + 0xb, &game_panel[((PanelIcon *)panel)[2].pressed]);
        break;
    case 2:
        reqspr_draw(((PanelIcon *)panel)[3].x, ((PanelIcon *)panel)[3].y + 0xb, &game_panel[((PanelIcon *)panel)[3].pressed]);
        break;
    case 3:
        reqspr_draw(((PanelIcon *)panel)[4].x, ((PanelIcon *)panel)[4].y + 0xb, &game_panel[((PanelIcon *)panel)[4].pressed]);
        break;
    }
    if (myhosp->fcaa < 100)
        level = 10 - myhosp->fcaa / 8;
    else
        level = 10;
    s0 = s1 = s2 = s3 = 0;
    switch (level) {
    case 0:
        if (actual_game_turn & 3) {
            s0 = 0x1a;
            s1 = 0x1a;
            s2 = 0x1a;
            s3 = 0x1a;
        } else {
            s0 = 0x1b;
            s1 = 0x1b;
            s2 = 0x1b;
            s3 = 0x1b;
        }
        break;
    case 1:
        s0 = 0x1a;
        s1 = 0x1a;
        s2 = 0x1a;
        s3 = 0x1a;
        break;
    case 2:
        s0 = 0x1a;
        s1 = 0x1a;
        s2 = 0x1a;
        s3 = 0x19;
        break;
    case 3:
        s0 = 0x1a;
        s1 = 0x1a;
        s2 = 0x1a;
        s3 = 0x18;
        break;
    case 4:
        s0 = 0x1a;
        s1 = 0x1a;
        s2 = 0x1a;
        break;
    case 5:
        s0 = 0x1a;
        s1 = 0x1a;
        s2 = 0x19;
        break;
    case 6:
        s0 = 0x1a;
        s1 = 0x1a;
        s2 = 0x18;
        break;
    case 7:
        s0 = 0x1a;
        s1 = 0x1a;
        break;
    case 8:
        s0 = 0x1a;
        s1 = 0x19;
        break;
    case 9:
        s0 = 0x1a;
        s1 = 0x18;
        break;
    case 10:
        s0 = 0x1a;
        break;
    case 11:
        s0 = 0x19;
        break;
    case 12:
        s0 = 0x18;
        break;
    }
    reqspr_draw(((PanelIcon *)panel)[5].x + 0x2c, ((PanelIcon *)panel)[5].y + 0xc, &game_panel[s0]);
    reqspr_draw(((PanelIcon *)panel)[5].x + 0x2c + game_panel[26].width, ((PanelIcon *)panel)[5].y + 0xc, &game_panel[s1]);
    reqspr_draw(((PanelIcon *)panel)[5].x + 0x2c + game_panel[26].width * 2, ((PanelIcon *)panel)[5].y + 0xc, &game_panel[s2]);
    reqspr_draw(((PanelIcon *)panel)[5].x + 0x2c + game_panel[26].width * 3, ((PanelIcon *)panel)[5].y + 0xc, &game_panel[s3]);
}


// FUNCTION: 0x000361a4
u8 display_panel_special_info(void)
{
    u8 i;
    u8 line;
    u8 y;
    s16 x;
    Thing *w;

    w = &thing[D_00155436];
    line = 0;
    y = D_0015f410->unknown594;
    i = 0;
    x = 0;
    D_00188c74 = 0;
    if (gamevar.unknown21 || D_00155e1e) {
        if (!D_00155e21) {
            if (statvar.unknown6 <= mouse_dy && !D_001553b4 && !gamevar.unknown21) {
                D_00155e1e = 0;
                return 0;
            }
        } else {
            D_00155e21--;
        }
        panel = panel_icons_normal;
        while (((PanelIcon *)panel)[i].sprite > 0) {
            ((PanelIcon *)panel)[i].y = lbDisplay.unknown1c - game_panel[((PanelIcon *)panel)[i].sprite].height;
            ((PanelIcon *)panel)[i].y2 = lbDisplay.unknown1c;
            ((PanelIcon *)panel)[i].x = x;
            ((PanelIcon *)panel)[i].x2 = x + game_panel[((PanelIcon *)panel)[i].sprite].width;
            x += game_panel[((PanelIcon *)panel)[i].sprite].width;
            i++;
        }
        statvar.unknown6 = ((PanelIcon *)panel)[1].y;
        clear_panel_info_bar();
        LbTextSetWindow(panel->x, panel->y, lbDisplay.unknown18 - panel->x, lbDisplay.unknown1c - panel->y);
        set_font_from_cache(0x2c);
        if (gamevar.unknown21) {
            LbTextDraw(D_0015f410->unknown570, y + D_0015f410->unknown584, D_001602a0[21]);
        } else if (D_00155e1e) {
            LbTextDraw(D_0015f410->unknown570, y + D_0015f410->unknown584, D_001602a0[D_00155e1e]);
            D_00155e1f--;
            if (D_00155e1f == 0)
                D_00155e1e = 0;
        }
        return 1;
    }
    return 0;
}


// FUNCTION: 0x0003646c
void panel_watch_help(void)
{
    u8 text;

    if (myhosp->f1d9)
        text = 0xee;
    else if (myhosp->f207)
        text = 0xef;
    else if (myhosp->unknown1257)
        text = 0xed;
    else
        return;
    fn_000bbff0(D_0015544e, help_text[text]);
}

// FUNCTION: 0x000364e0
void panel_rep_help(void)
{
    char buf[12];

    fn_000bbff0(D_0015544e, help_text[9]);
    sprintf(buf, D_00120327, myhosp->unknownc7e);
    fn_000c93bf(D_0015544e, buf);
}

// FUNCTION: 0x00036534
void setup_rat_fest_panel(void)
{
    LbSpriteReset(game_panel, D_00153f88, D_00153f80);
    if (D_0015f3f4 != SCREEN_MODE_1) {
        fn_000bce44(D_0013f680);
        LbSpriteSetup(game_panel, D_00153f88, D_00153f80);
        setup_panel_icons(D_0015f3f4);
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x00036594
void fn_00036594(s32 value, char *text)
{
    s32 x;
    s32 y;

    x = D_0015f40c->unknown5e0 + panel->x;
    y = D_0015f40c->unknown5e4 + (D_0015f410->unknown584 + panel->y + D_0015f410->unknown57c * 2);
    value > 100 ? (value = 100) : 0;
    LbTextDraw(D_0015f410->unknown570, D_0015f410->unknown57c * 2 + D_0015f410->unknown584, text);
    LbDrawBox(x, y, D_0015f40c->unknown5e8, D_0015f40c->unknown5ec, color.unknown7);
    LbDrawBox(x + 1, y + 1, (D_0015f40c->unknown5e8 - 2) * value / 100, D_0015f40c->unknown5ec - 2, color.unknownD);
    LbDrawBox(x + 1 + (D_0015f40c->unknown5e8 - 2) * value / 100, y + 1, D_0015f40c->unknown5e8 - 2 - (D_0015f40c->unknown5e8 - 2) * value / 100, D_0015f40c->unknown5ec - 2, color.unknown8);
}

// NONMATCHING: register choice
// FUNCTION: 0x00036730
void fn_00036730(char *text, u8 *line, u8 lines)
{
    s32 height;
    s32 width;
    s32 total;
    char *p;
    char *last;

    height = D_0015f410->unknown57c;
    width = lbDisplay.unknown18 - panel->x - D_0015f410->unknown570 * 2;
    if (LbTextStringWidth(text) < width) {
        if (*line + 1 <= lines) {
            LbTextDraw(D_0015f410->unknown570, D_0015f410->unknown584 + *line * height, text);
            (*line)++;
        }
    } else if (*line + 2 <= lines) {
        total = 0;
        p = text;
        while (total < width && *p) {
            last = p;
            while (*p && *p != ' ') {
                total += lbFontPtr ? lbFontPtr[*p - 0x1f].width : 0;
                p++;
            }
            if (*p) {
                total += lbFontPtr ? lbFontPtr[*p - 0x1f].width : 0;
                p++;
            }
        }
        last[-1] = 0;
        LbTextDraw(D_0015f410->unknown570, D_0015f410->unknown584 + *line * height, text);
        last[-1] = ' ';
        (*line)++;
        LbTextDraw(D_0015f410->unknown570, D_0015f410->unknown584 + *line * height, last);
        (*line)++;
    }
}

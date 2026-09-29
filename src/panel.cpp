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

extern char **D_001602cc;
extern char **D_0016029c;
extern char D_0012031d[];
extern Event D_001549d0[];
extern u8 D_00156910;
extern TbSprite *D_00153f80;
extern TbSprite *D_00153f88;
extern TbLoadFiles D_0013f680[];
extern char D_00120310[];
extern char D_00120327[];
extern char **D_001602a0;
extern char D_00120323[];
extern char **D_00160298;
extern char **D_00160318;
extern char D_00120316[];
extern char D_00120317[];
extern char **D_001602a4;
extern u8 D_0012d778[];
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
#pragma aux fn_000ccac4 "LbDrawLine_";

}

// FUNCTION: 0x00031c70
void update_panel(void)
{
    u8 drawn;

    drawn = 0;
    switch (othervar.fa49) {
    case 1:
        if (statvar.unknown6 <= lbDisplay.mmouse_y && !gamevar.f174)
            D_00156910 = 1;
        break;
    case 2:
        if (statvar.unknown6 > lbDisplay.mmouse_y || gamevar.f174)
            D_00156910 = 1;
        break;
    }
    if (othervar.f3b != myhosp->f35)
        D_00156910 = 1;
    if (D_00156910 == 1) {
        choose_panel_icons();
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        D_00156910 = 0;
        drawn = 1;
        draw_panel(1);
        set_all_refresh_zones_in_window(0, statvar.unknown4, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
    }
    if (!gamevar.f174) {
        if (!display_panel_special_info() && othervar.fa49 == 1) {
            switch (othervar.f7c) {
            case 0:
                if (othervar.f7d != 0 || drawn)
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
    if (gamevar.f174) {
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
    if (statvar.unknown6 <= lbDisplay.mmouse_y && !gamevar.f174 && !gamevar.unknown21) {
        panel = panel_icons_over;
        othervar.fa49 = 2;
    } else {
        panel = panel_icons_normal;
        othervar.fa49 = 1;
    }
    if (gamevar.f174)
        panel = panel_icons_ratfest;
    statvar.unknown6 = ((PanelIcon *)panel)[1].y;
    advisor.f8_u16 = resvars.f1c_PanelLayoutp->unknown240;
    advisor.fa_u16 = statvar.unknown6;
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
    if (!gamevar.f174) {
        panel = panel_icons_over;
    } else {
        panel = panel_icons_ratfest;
        --pass;
    }
    while (--pass >= 0) {
        if (pass == 0 && !gamevar.f174) {
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
    othervar.fa69 = 0;
    othervar.fa55 = 1;
}

// FUNCTION: 0x000321b4
u8 which_icon_is_mouse_over(void)
{
    u8 i;
    u8 over;

    i = 0;
    over = 0x63;
    if (statvar.unknown6 <= lbDisplay.mmouse_y && !gamevar.f174) {
        while (((PanelIcon *)panel)[i].sprite > 0) {
            if (((PanelIcon *)panel)[i].x <= lbDisplay.mmouse_x && ((PanelIcon *)panel)[i].x2 >= lbDisplay.mmouse_x) {
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
    if (gamevar.f12e == 0xe) {
        play_sound(10, 8, -1, -1);
    } else if (icon != 0x63) {
        switch (icon) {
        case 0:
            D_001549d0[player].type = 0x33;
            D_001549d0[player].value = 6;
            play_sound(10, 0x43, -1, -1);
            break;
        case 2:
            if (!(gamevar.f12e == 1 || gamevar.f12e == 2 || gamevar.f12e == 3 || gamevar.f12e == 0xe || gamevar.f12e == 0x10)) {
                set_mouse_mode(0);
                D_001549d0[player].type = 0x37;
                play_sound(10, 0x43, -1, -1);
            } else {
                play_sound(10, 8, -1, -1);
            }
            break;
        case 5:
            if (!(gamevar.f12e == 1 || gamevar.f12e == 2 || gamevar.f12e == 3 || gamevar.f12e == 0xe || gamevar.f12e == 0x10)) {
                set_mouse_mode(0);
                D_001549d0[player].type = 0x2e;
                play_sound(10, 0x43, -1, -1);
            } else {
                play_sound(10, 8, -1, -1);
            }
            break;
        case 4:
            if (gamevar.f12e == 0 && crequester.unknown2 == 0x63) {
                set_mouse_mode(0xc);
                allhosps[player].unknown11b7 = 0;
                play_sound(10, 0x43, -1, -1);
            } else if (gamevar.f12e == 0xc && crequester.unknown2 == 0x63) {
                set_mouse_mode(0);
                allhosps[player].unknown11b7 = 0;
                play_sound(10, 0x43, -1, -1);
            } else {
                play_sound(10, 8, -1, -1);
            }
            break;
        case 3:
            if (!(gamevar.f12e == 1 || gamevar.f12e == 2 || gamevar.f12e == 3 || gamevar.f12e == 0xe || gamevar.f12e == 0x10)) {
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
    if (gamevar.f12e == 0xe) {
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
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetClipWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    set_font_from_cache(5);
    height = lbFontPtr ? lbFontPtr[18].height : 0;
    if (othervar.f3b != myhosp->f35) {
        diff = fn_000cb2db(othervar.f3b - myhosp->f35);
        for (div = 1000000; div >= 0; div /= 10) {
            if (diff >= div) {
                digits++;
                break;
            }
        }
        LbTextSetClipWindow(resvars.f18_PanelLayoutp->unknown150 + resvars.f1c_PanelLayoutp->unknown248 * digits, resvars.f18_PanelLayoutp->unknown154,
                            resvars.f18_PanelLayoutp->unknown158 - resvars.f1c_PanelLayoutp->unknown248 * digits, height);
        sprintf(buf, D_00120300, myhosp->f35);
        LbTextDraw(resvars.f1c_PanelLayoutp->unknown250, resvars.f1c_PanelLayoutp->unknown254 - (height - othervar.f3f * 6), buf);
        sprintf(buf, D_00120304, othervar.f3b);
        LbTextDraw(resvars.f1c_PanelLayoutp->unknown250, resvars.f1c_PanelLayoutp->unknown254 + othervar.f3f * 6, buf);
        if (digits) {
            LbTextSetClipWindow(resvars.f18_PanelLayoutp->unknown160, resvars.f18_PanelLayoutp->unknown164,
                                resvars.f1c_PanelLayoutp->unknown248 * (digits - 3) + resvars.f18_PanelLayoutp->unknown168, height);
            sprintf(buf, D_00120308, myhosp->f35);
            LbTextDraw(resvars.f1c_PanelLayoutp->unknown250, resvars.f1c_PanelLayoutp->unknown254, buf);
        }
        othervar.f3f++;
        if (othervar.f3f == 4) {
            othervar.f3f = 0;
            othervar.f3b = myhosp->f35;
            D_00156910 = 1;
        }
    } else {
        LbTextSetClipWindow(resvars.f18_PanelLayoutp->unknown150, resvars.f18_PanelLayoutp->unknown154, resvars.f18_PanelLayoutp->unknown158, height);
        sprintf(buf, D_0012030c, myhosp->f35);
        LbTextDraw(resvars.f1c_PanelLayoutp->unknown250, resvars.f1c_PanelLayoutp->unknown254, buf);
    }
}


// FUNCTION: 0x00032cc4
void draw_date_on_panel(void)
{
    char buf[12];

    set_font_from_cache(6);
    lbDisplay.draw_flags = 0x100;
    LbTextSetWindow(resvars.f18_PanelLayoutp->unknown4c0, resvars.f18_PanelLayoutp->unknown4c4, resvars.f18_PanelLayoutp->unknown4c8, resvars.f18_PanelLayoutp->unknown4cc);
    sprintf(buf, D_00120310, mycontrol->unknown64f, month_text[mycontrol->unknown650]);
    LbTextDraw(resvars.f1c_PanelLayoutp->unknown440, resvars.f1c_PanelLayoutp->unknown444, buf);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
}

// FUNCTION: 0x00032da0
void draw_rep_on_panel(void)
{
    s32 offset;

    offset = (resvars.f1c_PanelLayoutp->unknown260 - resvars.f1c_PanelLayoutp->unknown258) * myhosp->unknownc7e / 1000;
    lbDisplay.draw_flags = 0;
    reqspr_draw(resvars.f1c_PanelLayoutp->unknown258 + offset, resvars.f1c_PanelLayoutp->unknown25c, &game_panel[36]);
}

// FUNCTION: 0x00032e20
void draw_emergency(void)
{
    u8 height;
    u8 off;

    off = 0;
    if (!gamevar.f174) {
        lbDisplay.draw_flags = 0;
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        set_font_from_cache(6);
        height = lbFontPtr ? lbFontPtr[25].height : 0;
        if (myhosp->f1d9 || (myhosp->unknown1257 && !(advisor.f11f & 1)) || myhosp->f207 || myhosp->f20e || (s8)myhosp->f1e2 > 1) {
            reqspr_draw(resvars.f1c_PanelLayoutp->unknown420, resvars.f1c_PanelLayoutp->unknown424, &watch[13]);
            set_all_refresh_zones_in_window(resvars.f1c_PanelLayoutp->unknown420, resvars.f1c_PanelLayoutp->unknown424,
                                            watch[13].width + resvars.f1c_PanelLayoutp->unknown420, resvars.f1c_PanelLayoutp->unknown424 + watch[13].height, 3);
            if (myhosp->f1d9) {
                if ((s16)myhosp->f1d9 / 8 >= 6) {
                    reqspr_draw(resvars.f1c_PanelLayoutp->unknown420 + resvars.f1c_PanelLayoutp->unknown438, resvars.f1c_PanelLayoutp->unknown424 + resvars.f1c_PanelLayoutp->unknown43c, &watch[1]);
                    set_all_refresh_zones_in_window(resvars.f1c_PanelLayoutp->unknown420 + resvars.f1c_PanelLayoutp->unknown438, resvars.f1c_PanelLayoutp->unknown424 + resvars.f1c_PanelLayoutp->unknown43c,
                                                    resvars.f1c_PanelLayoutp->unknown420 + resvars.f1c_PanelLayoutp->unknown438 + watch[1].width,
                                                    resvars.f1c_PanelLayoutp->unknown424 + resvars.f1c_PanelLayoutp->unknown43c + watch[1].height, 3);
                }
                reqspr_draw(resvars.f1c_PanelLayoutp->unknown420 + resvars.f1c_PanelLayoutp->unknown438, resvars.f1c_PanelLayoutp->unknown424 + resvars.f1c_PanelLayoutp->unknown43c,
                            &watch[D_0012d778[(s16)myhosp->f1d9 / 8]]);
                set_all_refresh_zones_in_window(resvars.f1c_PanelLayoutp->unknown420 + resvars.f1c_PanelLayoutp->unknown438, resvars.f1c_PanelLayoutp->unknown424 + resvars.f1c_PanelLayoutp->unknown43c,
                                                (resvars.f1c_PanelLayoutp->unknown420 + resvars.f1c_PanelLayoutp->unknown438) + watch[D_0012d778[(s16)myhosp->f1d9 / 8]].width,
                                                (resvars.f1c_PanelLayoutp->unknown424 + resvars.f1c_PanelLayoutp->unknown43c) + watch[D_0012d778[(s16)myhosp->f1d9 / 8]].height, 3);
            } else if (myhosp->f207) {
                if (myhosp->f1d9)
                    off = watch[15].width / 4 + watch[15].width;
                if (myhosp->f20b && !myhosp->f207)
                    lbDisplay.draw_flags = 4;
                reqspr_draw(off + resvars.f1c_PanelLayoutp->unknown430, resvars.f1c_PanelLayoutp->unknown434, &watch[14]);
                set_all_refresh_zones_in_window(off + resvars.f1c_PanelLayoutp->unknown430, resvars.f1c_PanelLayoutp->unknown434,
                                                off + resvars.f1c_PanelLayoutp->unknown430 + watch[14].width, watch[14].height + resvars.f1c_PanelLayoutp->unknown434, 3);
                lbDisplay.draw_flags = 0;
                if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, off + resvars.f1c_PanelLayoutp->unknown430, resvars.f1c_PanelLayoutp->unknown434, watch[14].width, watch[14].height)) {
                    reqspr_draw(off + resvars.f1c_PanelLayoutp->unknown430, resvars.f1c_PanelLayoutp->unknown434, &watch[15]);
                    set_all_refresh_zones_in_window(off + resvars.f1c_PanelLayoutp->unknown430, resvars.f1c_PanelLayoutp->unknown434,
                                                    watch[15].width + (off + resvars.f1c_PanelLayoutp->unknown430), watch[15].height + resvars.f1c_PanelLayoutp->unknown434, 3);
                    if (!othervar.fa6b)
                        play_sound(10, 0x44, -1, -1);
                    othervar.fa6b = 1;
                } else {
                    othervar.fa6b = 0;
                }
                if (myhosp->f20b && !myhosp->f207)
                    lbDisplay.draw_flags = 4;
                if ((s16)myhosp->f207 / 8 >= 6) {
                    reqspr_draw(off + resvars.f1c_PanelLayoutp->unknown420 + resvars.f1c_PanelLayoutp->unknown438, resvars.f1c_PanelLayoutp->unknown424 + resvars.f1c_PanelLayoutp->unknown43c, &watch[1]);
                    set_all_refresh_zones_in_window(off + resvars.f1c_PanelLayoutp->unknown420 + resvars.f1c_PanelLayoutp->unknown438, resvars.f1c_PanelLayoutp->unknown424 + resvars.f1c_PanelLayoutp->unknown43c,
                                                    watch[1].width + (off + resvars.f1c_PanelLayoutp->unknown420 + resvars.f1c_PanelLayoutp->unknown438),
                                                    resvars.f1c_PanelLayoutp->unknown424 + resvars.f1c_PanelLayoutp->unknown43c + watch[1].height, 3);
                }
                reqspr_draw(off + resvars.f1c_PanelLayoutp->unknown420 + resvars.f1c_PanelLayoutp->unknown438, resvars.f1c_PanelLayoutp->unknown424 + resvars.f1c_PanelLayoutp->unknown43c,
                            &watch[D_0012d778[(s16)myhosp->f207 / 8]]);
                set_all_refresh_zones_in_window(off + resvars.f1c_PanelLayoutp->unknown420 + resvars.f1c_PanelLayoutp->unknown438, resvars.f1c_PanelLayoutp->unknown424 + resvars.f1c_PanelLayoutp->unknown43c,
                                                watch[D_0012d778[(s16)myhosp->f207 / 8]].width + (off + resvars.f1c_PanelLayoutp->unknown420 + resvars.f1c_PanelLayoutp->unknown438),
                                                (resvars.f1c_PanelLayoutp->unknown424 + resvars.f1c_PanelLayoutp->unknown43c) + watch[D_0012d778[(s16)myhosp->f207 / 8]].height, 3);
                lbDisplay.draw_flags = 0;
            } else if (myhosp->unknown1257) {
                reqspr_draw(resvars.f1c_PanelLayoutp->unknown430, resvars.f1c_PanelLayoutp->unknown434, &watch[16]);
                set_all_refresh_zones_in_window(resvars.f1c_PanelLayoutp->unknown430, resvars.f1c_PanelLayoutp->unknown434,
                                                watch[16].width + resvars.f1c_PanelLayoutp->unknown430, watch[16].height + resvars.f1c_PanelLayoutp->unknown434, 3);
                if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.f1c_PanelLayoutp->unknown430, resvars.f1c_PanelLayoutp->unknown434, watch[14].width, watch[14].height)) {
                    reqspr_draw(resvars.f1c_PanelLayoutp->unknown430, resvars.f1c_PanelLayoutp->unknown434, &watch[17]);
                    set_all_refresh_zones_in_window(resvars.f1c_PanelLayoutp->unknown430, resvars.f1c_PanelLayoutp->unknown434,
                                                    watch[17].width + resvars.f1c_PanelLayoutp->unknown430, resvars.f1c_PanelLayoutp->unknown434 + watch[17].height, 3);
                    if (!othervar.fa6b)
                        play_sound(10, 0x44, -1, -1);
                    othervar.fa6b = 1;
                } else {
                    othervar.fa6b = 0;
                }
                if (myhosp->unknown1257 / (gbv.f154 / 13) >= 6) {
                    reqspr_draw(resvars.f1c_PanelLayoutp->unknown420 + resvars.f1c_PanelLayoutp->unknown438, resvars.f1c_PanelLayoutp->unknown424 + resvars.f1c_PanelLayoutp->unknown43c, &watch[1]);
                    set_all_refresh_zones_in_window(resvars.f1c_PanelLayoutp->unknown420 + resvars.f1c_PanelLayoutp->unknown438, resvars.f1c_PanelLayoutp->unknown424 + resvars.f1c_PanelLayoutp->unknown43c,
                                                    resvars.f1c_PanelLayoutp->unknown420 + resvars.f1c_PanelLayoutp->unknown438 + watch[1].width,
                                                    resvars.f1c_PanelLayoutp->unknown424 + resvars.f1c_PanelLayoutp->unknown43c + watch[1].height, 3);
                }
                reqspr_draw(resvars.f1c_PanelLayoutp->unknown420 + resvars.f1c_PanelLayoutp->unknown438, resvars.f1c_PanelLayoutp->unknown424 + resvars.f1c_PanelLayoutp->unknown43c,
                            &watch[D_0012d778[myhosp->unknown1257 / (gbv.f154 / 13)]]);
                set_all_refresh_zones_in_window(resvars.f1c_PanelLayoutp->unknown420 + resvars.f1c_PanelLayoutp->unknown438, resvars.f1c_PanelLayoutp->unknown424 + resvars.f1c_PanelLayoutp->unknown43c,
                                                (resvars.f1c_PanelLayoutp->unknown420 + resvars.f1c_PanelLayoutp->unknown438) + watch[D_0012d778[myhosp->unknown1257 / (gbv.f154 / 13)]].width,
                                                (resvars.f1c_PanelLayoutp->unknown424 + resvars.f1c_PanelLayoutp->unknown43c) + watch[D_0012d778[myhosp->unknown1257 / (gbv.f154 / 13)]].height, 3);
            }
        }
    }
}


// FUNCTION: 0x00033a40
void clear_panel_info_bar(void)
{
    u8 i;

    if (!gamevar.f174) {
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

    w = &thing[othervar.f7e];
    line = 0;
    height = resvars.f1c_PanelLayoutp->unknown57c;
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
    if (resvars.f1 == SCREEN_MODE_13)
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
            fn_00036594(w->unknown146 * 100 / gbv.ffc, info_bar_text[14]);
        }
    }
}


// FUNCTION: 0x00034118
void display_panel_staff_info(void)
{
    u8 n;
    u8 height;
    u8 i;
    u8 pct;
    u8 line;
    Thing *w;
    RoomInfo *room;
    char buf[100];
    char names[4][50];

    w = &thing[othervar.f7e];
    line = 0;
    height = resvars.f1c_PanelLayoutp->unknown57c;
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
        if (resvars.f1 == SCREEN_MODE_13) {
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
        fn_000ccac4(pct * (resvars.f18_PanelLayoutp->unknown5e8 - 2) / 100 + (resvars.f18_PanelLayoutp->unknown5e0 + panel->x + 1),
                    resvars.f18_PanelLayoutp->unknown5e4 + (panel->y + resvars.f1c_PanelLayoutp->unknown584 + resvars.f1c_PanelLayoutp->unknown57c * 2 + 1),
                    panel->x + resvars.f18_PanelLayoutp->unknown5e0 + 1 + pct * (resvars.f18_PanelLayoutp->unknown5e8 - 2) / 100,
                    panel->y + resvars.f1c_PanelLayoutp->unknown584 + resvars.f1c_PanelLayoutp->unknown57c * 2 + resvars.f18_PanelLayoutp->unknown5ec - 1 + resvars.f18_PanelLayoutp->unknown5e4,
                    color.unknown7);
    }
}


// FUNCTION: 0x000345e0
void display_panel_object_info(void)
{
    Thing *w;
    u8 line;
    char buf[100];

    w = &thing[othervar.f7e];
    line = 0;
    clear_panel_info_bar();
    if (w->renderer == 6) {
        LbTextSetWindow(panel->x, panel->y, lbDisplay.unknown18 - panel->x, lbDisplay.unknown1c - panel->y);
        set_font_from_cache(1);
        fn_00036730(D_0016029c[w->subtype], &line, 3);
        if (resvars.f1 == SCREEN_MODE_13) {
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

    w = &thing[othervar.f7e];
    line = 0;
    height = resvars.f1c_PanelLayoutp->unknown57c;
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
        if (resvars.f1 != SCREEN_MODE_13)
            line = 2;
        fn_00036730(buf, &line, 3);
        if (resvars.f1 == SCREEN_MODE_13) {
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

    w = &thing[othervar.f7e];
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

    w = &thing[othervar.f7e];
    line = 0;
    clear_panel_info_bar();
    LbTextSetWindow(panel->x, panel->y, lbDisplay.unknown18 - panel->x, lbDisplay.unknown1c - panel->y);
    set_font_from_cache(1);
    fn_00036730(D_0016029c[w->subtype], &line, 3);
    if (resvars.f1 != SCREEN_MODE_13)
        line = 2;
    sprintf(buf, info_bar_text[31], w->mode);
    fn_00036730(buf, &line, 3);
}

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
    if (!gamevar.f174) {
        maintain_panel_popup();
        othervar.f9c2 = 0;
        if (resvars.f1 == SCREEN_MODE_13) {
            LbScreenSetGraphicsWindow(0xb1, ((PanelIcon *)panel)[0].y, 100, 100);
            LbDrawBox(0, 0, 0x1c, 3, color.unknown8);
        }
        othervar.fa58++;
        if (panel_emergency_spr[othervar.fa58] == 0)
            othervar.fa58 = 0;
        switch (othervar.fa55) {
        case 1:
        case 2:
        case 3:
        case 4:
            redraw = 1;
            break;
        }
        LbScreenSetGraphicsWindow(0, ((PanelIcon *)panel)[0].y - game_panel[popup_icons[1].unknown0].height + resvars.f1c_PanelLayoutp->unknown4ac,
                                  lbDisplay.graphics_screen_width, game_panel[popup_icons[1].unknown0].height);
        for (i = 1; popup_icons[i].unknown0 != 0; i++) {
            if (listhead[player].first[i]) {
                spr = &game_panel[popup_icons[i].unknown0];
                p = &playercall[listhead[player].first[i]];
                if (p->slot < 9) {
                    if (p->flash) {
                        if (p->flash > 0x22)
                            x = resvars.f1c_PanelLayoutp->slots[p->slot].x + resvars.f1c_PanelLayoutp->unknown4a8;
                        else
                            x = resvars.f1c_PanelLayoutp->slots[0].x + resvars.f1c_PanelLayoutp->unknown4a8;
                    } else {
                        x = resvars.f1c_PanelLayoutp->slots[p->slot].x + resvars.f1c_PanelLayoutp->unknown4a8;
                    }
                    y = 0;
                    if (p->flash) {
                        x += resvars.f34_Pointp[p->flash].x;
                        y += resvars.f34_Pointp[p->flash].y;
                        if (resvars.f34_Pointp[p->flash].x == 0 && resvars.f34_Pointp[p->flash].y == 0) {
                            p->flash = 0;
                        } else {
                            if (p->flash == 1 && othervar.fa55 != 3) {
                            } else {
                                p->flash++;
                            }
                            for (;;) {
                                if (resvars.f34_Pointp[p->flash].x < resvars.f1c_PanelLayoutp->slots[p->slot].x) {
                                    if (resvars.f34_Pointp[p->flash].x == 0 && resvars.f34_Pointp[p->flash].y == 0) {
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
                                } else {
                                    break;
                                }
                            }
                        }
                    }
                    x1 = x + spr->width;
                    y1 = y + spr->height;
                    if (y) {
                        reqspr_draw(x, y, spr);
                        if (popup_icons[i].unknown0 == 0x2b)
                            reqspr_draw(x + resvars.f1c_PanelLayoutp->unknown598, y + resvars.f1c_PanelLayoutp->unknown59c, &game_panel[panel_emergency_spr[othervar.fa58]]);
                        othervar.fa55 = 2;
                        no_flash = 0;
                    }
                    redraw = 1;
                    y += ((PanelIcon *)panel)[0].y - game_panel[popup_icons[1].unknown0].height;
                    y1 += ((PanelIcon *)panel)[0].y - game_panel[popup_icons[1].unknown0].height;
                    if (lbDisplay.mmouse_x >= x && lbDisplay.mmouse_x < x1 && lbDisplay.mmouse_y >= y && lbDisplay.mmouse_y < y1)
                        othervar.f9c2 = i;
                    set_all_refresh_zones_in_window(x, y, x1, y1, 3);
                }
            }
        }
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
        if (no_flash)
            othervar.fa55 = 4;
        if (redraw) {
            reqspr_draw(((PanelIcon *)panel)[0].x, ((PanelIcon *)panel)[0].y, &game_panel[((PanelIcon *)panel)[0].sprite]);
            reqspr_draw(((PanelIcon *)panel)[1].x, ((PanelIcon *)panel)[1].y, &game_panel[((PanelIcon *)panel)[1].sprite]);
            draw_money_on_panel();
            draw_date_on_panel();
            draw_rep_on_panel();
            set_all_refresh_zones_in_window(((PanelIcon *)panel)[0].x, ((PanelIcon *)panel)[0].y,
                                            ((PanelIcon *)panel)[0].x2, ((PanelIcon *)panel)[0].y2, 3);
        }
        LbScreenSetGraphicsWindow(0, ((PanelIcon *)panel)[0].y - game_panel[popup_icons[1].unknown0].height + resvars.f1c_PanelLayoutp->unknown4ac,
                                  lbDisplay.graphics_screen_width, game_panel[popup_icons[1].unknown0].height);
        for (i = 1; popup_icons[i].unknown0 != 0; i++) {
            if (listhead[player].first[i]) {
                spr = &game_panel[popup_icons[i].unknown0];
                p = &playercall[listhead[player].first[i]];
                if (p->slot < 9) {
                    if (p->flash) {
                        if (p->flash > 0x22)
                            x = resvars.f1c_PanelLayoutp->slots[p->slot].x + resvars.f1c_PanelLayoutp->unknown4a8;
                        else
                            x = resvars.f1c_PanelLayoutp->slots[0].x + resvars.f1c_PanelLayoutp->unknown4a8;
                    } else {
                        x = resvars.f1c_PanelLayoutp->slots[p->slot].x + resvars.f1c_PanelLayoutp->unknown4a8;
                    }
                    y = 0;
                    if (p->flash) {
                        x += resvars.f34_Pointp[p->flash].x;
                        y += resvars.f34_Pointp[p->flash].y;
                    }
                    if (y == 0) {
                        x1 = x + spr->width;
                        y1 = y + spr->height;
                        reqspr_draw(x, y, spr);
                        if (popup_icons[i].unknown0 == 0x2b)
                            reqspr_draw(x + resvars.f1c_PanelLayoutp->unknown598, y + resvars.f1c_PanelLayoutp->unknown59c, &game_panel[panel_emergency_spr[othervar.fa58]]);
                        y += ((PanelIcon *)panel)[0].y - game_panel[popup_icons[1].unknown0].height;
                        y1 += ((PanelIcon *)panel)[0].y - game_panel[popup_icons[1].unknown0].height;
                        set_all_refresh_zones_in_window(x, y, x1, y1, 3);
                    }
                }
            }
        }
        if (resvars.f1 == SCREEN_MODE_13)
            LbScreenSetGraphicsWindow(0xb1, ((PanelIcon *)panel)[0].y, 100, 100);
        x = resvars.f1c_PanelLayoutp->unknown4a0;
        y = resvars.f1c_PanelLayoutp->unknown4a4;
        switch (othervar.fa55) {
        case 2:
            if (othervar.fa56 - 2 >= 0)
                othervar.fa56 -= 2;
            else
                othervar.fa56 = 0;
            if (othervar.fa56 == 0) {
                othervar.fa55 = 3;
                othervar.fa57 = 0;
            } else {
                for (j = 0; j < othervar.fa56; j++) {
                    if (resvars.f1 == SCREEN_MODE_13)
                        reqspr_draw(x, y, &game_panel[41]);
                    x += game_panel[41].width;
                }
                if (resvars.f1 == SCREEN_MODE_13)
                    reqspr_draw(x, y, &game_panel[42]);
            }
            break;
        case 3:
            othervar.fa56 = 0;
            othervar.fa57++;
            if (othervar.fa57 > 0x28)
                othervar.fa55 = 4;
            break;
        case 4:
            if (othervar.fa57 <= 0x28) {
                othervar.fa57++;
            } else {
                if (othervar.fa56 + 2 < 0x18)
                    othervar.fa56 += 2;
                else
                    othervar.fa56 = 0x19;
                for (j = 0; j < othervar.fa56; j++) {
                    if (resvars.f1 == SCREEN_MODE_13)
                        reqspr_draw(x, y, &game_panel[41]);
                    x += game_panel[41].width;
                }
                if (resvars.f1 == SCREEN_MODE_13)
                    reqspr_draw(x, y, &game_panel[42]);
                if (othervar.fa56 >= 0x18)
                    othervar.fa55 = 1;
            }
            break;
        case 1:
            if (resvars.f1 == SCREEN_MODE_13)
                reqspr_draw(x, y, &game_panel[40]);
            for (j = 0; j < 0x19; j++) {
                if (resvars.f1 == SCREEN_MODE_13)
                    reqspr_draw(x, y, &game_panel[41]);
                x += game_panel[41].width;
            }
            if (resvars.f1 == SCREEN_MODE_13)
                reqspr_draw(x, y, &game_panel[42]);
            othervar.fa56 = 0x18;
            break;
        default:
            othervar.fa55 = 1;
            break;
        }
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
        lbDisplay.draw_flags = 0;
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
            othervar.fa55 = 2;
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
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    if (advisor.f120 >= 0x10 && advisor.f120 <= 0x1b) {
        othervar.f9bc++;
        if (othervar.f9bc % 10 == 0) {
            reqspr_draw(((PanelIcon *)panel)[advisor.f120 - 0x10].x, ((PanelIcon *)panel)[advisor.f120 - 0x10].y, &game_panel[((PanelIcon *)panel)[advisor.f120 - 0x10].pressed]);
            othervar.f9bc = 0;
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
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0x29);
    sprintf(buf, D_00120323, myhosp->fd27u);
    LbTextDraw(0x5d, 0x1be, buf);
    switch (gamevar.f175) {
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

    w = &thing[othervar.f7e];
    line = 0;
    y = resvars.f1c_PanelLayoutp->unknown594;
    i = 0;
    x = 0;
    lbDisplay.draw_flags = 0;
    if (gamevar.unknown21 || othervar.fa66) {
        if (!othervar.fa69) {
            if (statvar.unknown6 <= lbDisplay.mmouse_y && !gamevar.f174 && !gamevar.unknown21) {
                othervar.fa66 = 0;
                return 0;
            }
        } else {
            othervar.fa69--;
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
            LbTextDraw(resvars.f1c_PanelLayoutp->unknown570, y + resvars.f1c_PanelLayoutp->unknown584, D_001602a0[21]);
        } else if (othervar.fa66) {
            LbTextDraw(resvars.f1c_PanelLayoutp->unknown570, y + resvars.f1c_PanelLayoutp->unknown584, D_001602a0[othervar.fa66]);
            othervar.fa67--;
            if (othervar.fa67 == 0)
                othervar.fa66 = 0;
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
    fn_000bbff0(othervar.f96, help_text[text]);
}

// FUNCTION: 0x000364e0
void panel_rep_help(void)
{
    char buf[12];

    fn_000bbff0(othervar.f96, help_text[9]);
    sprintf(buf, D_00120327, myhosp->unknownc7e);
    fn_000c93bf(othervar.f96, buf);
}

// FUNCTION: 0x00036534
void setup_rat_fest_panel(void)
{
    LbSpriteReset(game_panel, D_00153f88, D_00153f80);
    if (resvars.f0 != SCREEN_MODE_1) {
        fn_000bce44(D_0013f680);
        LbSpriteSetup(game_panel, D_00153f88, D_00153f80);
        setup_panel_icons(resvars.f0);
    }
}

// FUNCTION: 0x00036594
void fn_00036594(s32 value, char *text)
{
    s32 x;
    s32 y;

    x = resvars.f18_PanelLayoutp->unknown5e0 + panel->x;
    y = resvars.f1c_PanelLayoutp->unknown584 + panel->y + resvars.f1c_PanelLayoutp->unknown57c * 2 + resvars.f18_PanelLayoutp->unknown5e4;
    value > 100 ? (value = 100) : 0;
    LbTextDraw(resvars.f1c_PanelLayoutp->unknown570, resvars.f1c_PanelLayoutp->unknown57c * 2 + resvars.f1c_PanelLayoutp->unknown584, text);
    LbDrawBox(x, y, resvars.f18_PanelLayoutp->unknown5e8, resvars.f18_PanelLayoutp->unknown5ec, color.unknown7);
    LbDrawBox(x + 1, y + 1, (resvars.f18_PanelLayoutp->unknown5e8 - 2) * value / 100, resvars.f18_PanelLayoutp->unknown5ec - 2, color.unknownD);
    LbDrawBox(x + 1 + (resvars.f18_PanelLayoutp->unknown5e8 - 2) * value / 100, y + 1, resvars.f18_PanelLayoutp->unknown5e8 - 2 - (resvars.f18_PanelLayoutp->unknown5e8 - 2) * value / 100, resvars.f18_PanelLayoutp->unknown5ec - 2, color.unknown8);
}

// FUNCTION: 0x00036730
void fn_00036730(char *text, u8 *line, u8 lines)
{
    s32 height;
    s32 width;
    s32 total;
    char *p;
    char *last;

    height = resvars.f1c_PanelLayoutp->unknown57c;
    width = lbDisplay.unknown18 - panel->x - resvars.f1c_PanelLayoutp->unknown570 * 2;
    if (LbTextStringWidth(text) < width) {
        if (*line + 1 <= lines) {
            LbTextDraw(resvars.f1c_PanelLayoutp->unknown570, resvars.f1c_PanelLayoutp->unknown584 + *line * height, text);
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
        LbTextDraw(resvars.f1c_PanelLayoutp->unknown570, resvars.f1c_PanelLayoutp->unknown584 + *line * height, text);
        last[-1] = ' ';
        (*line)++;
        LbTextDraw(resvars.f1c_PanelLayoutp->unknown570, resvars.f1c_PanelLayoutp->unknown584 + *line * height, last);
        (*line)++;
    }
}

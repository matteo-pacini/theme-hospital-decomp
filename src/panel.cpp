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
    u8 unknown0[0x240];
    s32 unknown240;
    u8 unknown244[0x14];
    s32 unknown258;
    s32 unknown25c;
    s32 unknown260;
    u8 unknown264[0x1dc];
    s32 unknown440;
    s32 unknown444;
    u8 unknown448[0x78];
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
    u8 unknown588[0x58];
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

extern Icons panel_icons_over[];
extern Icons panel_icons_normal[];
extern Icons panel_icons_ratfest[];
extern PopupIconControl popup_icons[];

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

void fn_000c93bf(char *dst, char *src);
void fn_000bce44(TbLoadFiles *files);
void fn_00035824(void);
void fn_00036594(s32 value, char *text);
void fn_00036730(char *text, u8 *line, u8 lines);
void LbSpriteReset(TbSprite *start, TbSprite *end, void *data);

}

// NONMATCHING: register choice
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

// NONMATCHING: register choice
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
    s32 x;
    s8 pass;

    i = 0;
    x = 0;
    pass = 2;
    if (!D_001553b4) {
        panel = panel_icons_over;
    } else {
        panel = panel_icons_ratfest;
        pass--;
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

// NONMATCHING: register choice
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

// NONMATCHING: register choice
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

// NONMATCHING: register choice
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

// NONMATCHING: register choice
// FUNCTION: 0x00032da0
void draw_rep_on_panel(void)
{
    s32 offset;

    offset = (D_0015f410->unknown260 - D_0015f410->unknown258) * myhosp->unknownc7e / 1000;
    D_00188c74 = 0;
    reqspr_draw(D_0015f410->unknown258 + offset, D_0015f410->unknown25c, &game_panel[36]);
}

// NONMATCHING: register choice
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

// NONMATCHING: register choice
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

// NONMATCHING: register choice
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

// NONMATCHING: register choice
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

// NONMATCHING: register choice
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

// NONMATCHING: register choice
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
    LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[text]);
}

// NONMATCHING: register choice
// FUNCTION: 0x000364e0
void panel_rep_help(void)
{
    char buf[12];

    LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[9]);
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

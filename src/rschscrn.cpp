#include "hosp.h"

struct ChoiceBox {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
};

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

extern "C" {
extern s32 D_00188c38;
extern u8 D_00155e3c;
extern Event D_001549d0[];
extern Point *D_0015f434;
extern volatile u16 D_00188c74;
extern u8 D_00188c7b;
extern u8 D_00188c7c;
extern u8 D_00188c81;
extern u8 D_00188c83;
extern u8 D_00188c85;
extern u8 D_00188c7e;
extern volatile s16 D_00188c6c, D_00188c70;
extern Button *D_0015f43c;
extern u16 D_00155da9;
extern u32 D_00155dab;
extern u32 D_00155c5a;
extern char **D_001602d8;
extern char **D_001602a0;
extern ChoiceBox *D_0015f430;
extern char D_00121afc[];
extern char D_00121aff[];
void StopSample(u32 a, u32 b);
}

extern Icons panel_icons_over[];

void process_res_panel(void);
void draw_research_text(void);
s32 find_new_cure_cost(u8 illness, u8 owner);
u8 choose_next_research_illness(u8 owner, s32 (*cost)(u8, u8));

// FUNCTION: 0x000b26d0
void display_research(void)
{
    u8 frame_no;

    frame_no = (game_turn & 0xf) / 2 + 1;
    compute_help_text(0, 0, 0x10);
    switch (research.unknown2) {
    case 0:
        load_ingame_pic(6, 1, 0x1e, 0, SCREEN_MODE_13);
        research.unknown2 = 1;
        research.unknown0 = 0;
        play_sound(0xc, 0x75, -1, -1);
        D_00155e3c = 0;
        panel = panel_icons_over;
        ((PanelIcon *)panel)[9].sprite = 0x15;
        break;
    case 1:
        copy_screen_to_wscreen(bscreen);
        break;
    case 2:
        D_001549d0[player].type = 0x70;
        D_001549d0[player].value = 0;
        StopSample(0xc, 0x75);
        break;
    default:
        break;
    }
    reqspr_draw(D_0015f434[85].x, D_0015f434[85].y, &screen_sprites_tab_start[frame_no + 12]);
    reqspr_draw(D_0015f434[86].x, D_0015f434[86].y, &screen_sprites_tab_start[frame_no + 4]);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c85 = color.unknown8;
    D_00188c74 |= 0x40;
    set_font_from_cache(1);
    process_res_panel();
    draw_research_text();
    D_00188c7b = D_00188c7c = D_00188c81 = D_00188c83 = 0;
    D_00188c74 = 0;
    if (exit_quick_requester())
        research.unknown2 = 2;
}

// NONMATCHING: register choice, stack slot of unused height
// FUNCTION: 0x000b28b4
void draw_research_text(void)
{
    u8 total;
    u16 i;
    s32 height;
    u16 illness;
    u16 x;
    s16 len;
    char buf[100];

    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0;
    set_font_from_cache(9);
    height = lbFontPtr ? lbFontPtr[34].height : 0;
    for (i = 0; i < 5; i++) {
        LbTextDraw(D_0015f434[i + 0x57].x, D_0015f434[i + 0x57].y, D_001602d8[i]);
        if (myhosp->research_alloc[i] >= 0) {
            sprintf(buf, D_00121afc, myhosp->research_alloc[i]);
            LbTextDraw(D_0015f434[i + 0x61].x, D_0015f434[i + 0x61].y, buf);
            if (myhosp->research_cost[i] > 0) {
                illness = choose_next_research_illness(player, find_new_cure_cost);
                if (illness) {
                    len = myhosp->illness_research[illness] * 0x122 / myhosp->research_cost[i];
                    if (len < 0)
                        len = 0;
                    else if (len > 0x122)
                        len = 0x122;
                    for (x = 0; x < len; x += 5)
                        reqspr_draw(x + D_0015f434[i + 0x5c].x, D_0015f434[i + 0x5c].y, &screen_sprites_tab_start[3]);
                }
            }
        } else {
            LbDrawBox(D_0015f430[i + 0x29].x, D_0015f430[i + 0x29].y, D_0015f430[i + 0x29].w, D_0015f430[i + 0x29].h,
                      0xb);
            LbTextDraw(D_0015f434[i + 0x61].x, D_0015f434[i + 0x61].y, D_001602a0[0x14]);
        }
    }
    total = 0;
    for (i = 0; i < 5; i++) {
        if (myhosp->research_alloc[i] > 0)
            total += myhosp->research_alloc[i];
    }
    sprintf(buf, D_001602d8[6]);
    LbTextDraw(D_0015f434[0x66].x, D_0015f434[0x66].y, buf);
    sprintf(buf, D_00121aff, total);
    LbTextDraw(D_0015f434[0x67].x, D_0015f434[0x67].y, buf);
}

// FUNCTION: 0x000b2c74
void alter_research(u8 cat, u8 value, u8 owner)
{
    u8 old;
    AllHosps *h;
    u8 total;
    u8 i;

    h = &allhosps[owner];
    old = h->research_alloc[cat];
    if (old >= 0) {
        h->research_alloc[cat] = value;
        total = 0;
        for (i = 0; i < 5; i++) {
            if (h->research_alloc[i] > 0)
                total += h->research_alloc[i];
        }
        if (total > 100)
            h->research_alloc[cat] = old;
    }
}

// FUNCTION: 0x000b2d44
void process_res_panel(void)
{
    u8 i;

    if (D_00188c81) {
        for (i = 0; D_0015f43c[i + 0x19].width != 0 && D_0015f43c[i + 0x19].height != 0; i++) {
            if (over_hotspot(D_00188c6c, D_00188c70, D_0015f43c[i + 0x19].x, D_0015f43c[i + 0x19].y,
                             D_0015f43c[i + 0x19].width, D_0015f43c[i + 0x19].height)) {
                if (i == 0)
                    D_0015f43c[i + 0x19].callback(i);
                reqspr_draw(D_0015f43c[i + 0x19].unknownC, D_0015f43c[i + 0x19].unknownE,
                            &screen_sprites_tab_start[D_0015f43c[i + 0x19].sprite]);
            }
        }
        D_00155da9 = 0x100;
        D_00155dab = D_00155c5a;
        research.unknown0 = 0;
        D_00188c81 = 0;
    }
    if (D_00188c7e) {
        for (i = 0; D_0015f43c[i + 0x19].width != 0 && D_0015f43c[i + 0x19].height != 0; i++) {
            if (over_hotspot((s16)mouse_dx, (s16)mouse_dy, D_0015f43c[i + 0x19].x, D_0015f43c[i + 0x19].y,
                             D_0015f43c[i + 0x19].width, D_0015f43c[i + 0x19].height)) {
                if (i != 0)
                    D_0015f43c[i + 0x19].callback(i);
                reqspr_draw(D_0015f43c[i + 0x19].unknownC, D_0015f43c[i + 0x19].unknownE,
                            &screen_sprites_tab_start[D_0015f43c[i + 0x19].sprite]);
                break;
            }
        }
    } else {
        D_00155da9 = 0x100;
        D_00155dab = D_00155c5a;
    }
}

// FUNCTION: 0x000b3060
void process_res_button(u8 n)
{
    u16 i;
    u32 total;

    switch (n) {
    case 0:
        research.unknown2 = 2;
        play_sound(10, 0x42, -1, -1);
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        if (myhosp->research_alloc[n - 1] >= 0) {
            i = myhosp->research_alloc[n - 1];
            if (i > 0) {
                if (D_00155c5a - D_00155dab > 0xfa || D_00155da9 == 0x100) {
                    D_001549d0[player].type = 0x50;
                    D_001549d0[player].value = n - 1;
                    D_001549d0[player].arg = myhosp->research_alloc[n - 1];
                    if (D_00155da9 < 0xa00)
                        D_00155da9 += 0x10;
                    else
                        D_00155da9 = 0xa00;
                    D_001549d0[player].arg -= D_00155da9 >> 8;
                    if ((s16)D_001549d0[player].arg < 0)
                        D_001549d0[player].arg = 0;
                    play_sound(10, 0x43, -1, -1);
                }
            } else if (!research.unknown0) {
                play_sound(10, 8, -1, -1);
                research.unknown0 = 1;
            }
        }
        break;
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
        if (myhosp->research_alloc[n - 6] >= 0) {
            total = 0;
            for (i = 0; i < 5; i++) {
                if (i != n - 6) {
                    if (myhosp->research_alloc[i] >= 0)
                        total += myhosp->research_alloc[i];
                }
            }
            i = myhosp->research_alloc[n - 6];
            if (i + total < 100) {
                if (D_00155c5a - D_00155dab > 0xfa || D_00155da9 == 0x100) {
                    D_001549d0[player].type = 0x50;
                    D_001549d0[player].value = n - 6;
                    D_001549d0[player].arg = myhosp->research_alloc[n - 6];
                    if (D_00155da9 < 0xa00)
                        D_00155da9 += 0x10;
                    else
                        D_00155da9 = 0xa00;
                    D_001549d0[player].arg += D_00155da9 >> 8;
                    if (D_001549d0[player].arg + total > 100)
                        D_001549d0[player].arg = 100 - total;
                    play_sound(10, 0x43, -1, -1);
                }
            } else if (!research.unknown0) {
                play_sound(10, 8, -1, -1);
                research.unknown0 = 1;
            }
        }
        break;
    default:
        break;
    }
}

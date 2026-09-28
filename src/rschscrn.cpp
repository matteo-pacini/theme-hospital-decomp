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
void StopSample(u32 a, u32 b);
}

extern Icons panel_icons_over[];

void process_res_panel(void);
void draw_research_text(void);

// NONMATCHING: register choice
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

#include "hosp.h"

struct Graphs {
    u8 unknown0;
    u8 visible;
    u8 colour;
    u8 unknown3;
};

extern Graphs graph[];

extern "C" {
extern Button *D_0015f43c;
extern u8 D_00188c7e;
extern u8 D_00188c7f;
extern u8 D_00188c81;
extern u8 D_00188c83;
extern volatile s16 D_00188c6c;
extern volatile s16 D_00188c70;
extern u16 D_001553e5;
}

// NONMATCHING: register choice
// FUNCTION: 0x00079d30
void process_graph_panel(void)
{
    u8 i;

    if (D_00188c81 || D_00188c83) {
        for (i = 0; D_0015f43c[i + 0x25].width != 0 && D_0015f43c[i + 0x25].height != 0; i++) {
            if (over_hotspot(D_00188c6c, D_00188c70, D_0015f43c[i + 0x25].x, D_0015f43c[i + 0x25].y,
                             D_0015f43c[i + 0x25].width, D_0015f43c[i + 0x25].height)) {
                D_0015f43c[i + 0x25].callback(i);
                reqspr_draw(D_0015f43c[i + 0x25].unknownC, D_0015f43c[i + 0x25].unknownE,
                            &screen_sprites_tab_start[D_0015f43c[i + 0x25].sprite]);
                break;
            }
        }
        D_00188c81 = D_00188c83 = 0;
    }
    if (D_00188c7e || D_00188c7f) {
        for (i = 0; D_0015f43c[i + 0x25].width != 0 && D_0015f43c[i + 0x25].height != 0; i++) {
            if (over_hotspot((s16)mouse_dx, (s16)mouse_dy, D_0015f43c[i + 0x25].x, D_0015f43c[i + 0x25].y,
                             D_0015f43c[i + 0x25].width, D_0015f43c[i + 0x25].height)) {
                reqspr_draw(D_0015f43c[i + 0x25].unknownC, D_0015f43c[i + 0x25].unknownE,
                            &screen_sprites_tab_start[D_0015f43c[i + 0x25].sprite]);
                break;
            }
        }
    }
}

// FUNCTION: 0x0007a008
void process_graph_button(u8 n)
{
    switch (n) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        if (D_00188c83) {
            graph[0].visible = 0;
            graph[1].visible = 0;
            graph[2].visible = 0;
            graph[3].visible = 0;
            graph[4].visible = 0;
            graph[5].visible = 0;
            graph[6].visible = 0;
            graph[7].visible = 0;
            graph[n].visible = 1;
        } else if (graph[n].visible) {
            graph[n].visible = 0;
        } else {
            graph[n].visible = 1;
        }
        play_sound(10, 0x43, -1, -1);
        break;
    case 8:
        switch (D_001553e5) {
        case 0:
            D_001553e5 = 1;
            break;
        case 1:
            D_001553e5 = 2;
            break;
        case 2:
            D_001553e5 = 0;
            break;
        }
        play_sound(10, 0x43, -1, -1);
        break;
    case 9:
        graphs.unknown2 = 2;
        play_sound(10, 0x42, -1, -1);
        break;
    }
}

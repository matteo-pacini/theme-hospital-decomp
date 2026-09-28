#include "hosp.h"

struct Box {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
};

extern "C" {
extern u8 *D_0015f402;
extern u16 *D_0015f406;
extern Box *D_0015f40c;
extern Point *D_0015f410;
extern Event D_001549d0[];
extern TbScreenMode D_0015f3f4;
extern TbScreenMode D_0015f3f5;
extern u8 D_0015831b;
extern u16 D_0015831c;
extern u8 D_00155d74;
extern u16 D_00155d78;
extern volatile u16 D_00188c74;
extern u8 D_00188c7e;
extern s32 D_00145328;
extern u8 D_001553c7;
extern Blob63 D_00155c6f;
extern char **D_0016029c;
extern char **D_001602a0;
extern char **D_001602cc;
extern char D_00121ad8[];

void LbTextSetClipWindow(s32 x, s32 y, s32 w, s32 h);
void LbTextSetJustifyWindow(s32 x, s32 y, s32 w);
}

void post_shop_readjust_build_palette(void);
u8 does_room_contain_compulsory_objects(RoomInfo *room);

// NONMATCHING: register choice
// FUNCTION: 0x0009f0c0
void req14(Window *win)
{
    WindowButton *b;
    s32 i;

    b = win->buttons;
    for (b = win->buttons, i = 1; b; b = b->next, i++) {
        if (!(b->flags & 6) || (crequester.unknown4 & (1 << i)))
            continue;
        if (D_0015f402[0x4f] == i) {
            setup_abort_all_requesters();
            play_sound(10, 0x42, -1, -1);
        } else if (D_0015f402[0x50] == i) {
            D_001549d0[player].type = 0x5b;
            play_sound(10, 0x43, -1, -1);
        } else if (D_0015f402[0x51] == i) {
            set_mouse_mode(0xb);
            set_edit_room_requester_mode(8);
            crequester.unknown9 = 0;
            play_sound(10, 0x43, -1, -1);
        } else if (D_0015f402[0x52] == i) {
            switch (crequester.unknown0) {
            case 0:
                D_001549d0[player].type = 0x12;
                set_mouse_mode(0);
                break;
            case 4:
                D_001549d0[player].type = 4;
                D_001549d0[player].value = crequester.unknown8;
                play_sound(10, 0x41, -1, -1);
                break;
            case 6:
            case 8:
                D_001549d0[player].type = 1;
                play_sound(10, 0x41, -1, -1);
                break;
            case 1:
            case 2:
            case 3:
            case 5:
            case 7:
            default:
                break;
            }
        }
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x0009f2e0
void req14_overdraw(Window *win)
{
    u8 i;
    u8 hover;
    s32 off;
    WindowButton *b;
    s32 ox;
    s32 oy;
    s32 by;
    s32 n;
    s32 on;
    s32 j;
    s32 bx;
    u32 k;
    s32 count;
    s32 top;
    char buf[8];

    ox = win->x;
    oy = win->y;
    n = 0;
    bx = ox + D_0015f410[0x6e].x;
    hover = 0;
    if (D_0015831b & 1) {
        i = 1;
        D_00155d74++;
        if (D_00155d74 % 10 == 0) {
            if (D_0015831c == D_0015f402[0x4f]) {
                for (b = win->buttons; b; b = b->next) {
                    if (i == D_0015f402[0x4f])
                        break;
                    i++;
                }
                reqspr_draw(b->x + (s16)ox, b->y + (s16)oy, &button_sprite[D_0015f406[0x18]]);
            } else if (D_0015831c == D_0015f402[0x50]) {
                for (b = win->buttons; b; b = b->next) {
                    if (i == D_0015f402[0x50])
                        break;
                    i++;
                }
                reqspr_draw(b->x + (s16)ox, b->y + (s16)oy, &button_sprite[D_0015f406[0x19]]);
            } else if (D_0015831c == D_0015f402[0x51]) {
                for (b = win->buttons; b; b = b->next) {
                    if (i == D_0015f402[0x51])
                        break;
                    i++;
                }
                reqspr_draw(b->x + (s16)ox, b->y + (s16)oy, &button_sprite[D_0015f406[0x1a]]);
            } else if (D_0015831c == D_0015f402[0x52]) {
                for (b = win->buttons; b; b = b->next) {
                    if (i == D_0015f402[0x52])
                        break;
                    i++;
                }
                reqspr_draw(b->x + (s16)ox, b->y + (s16)oy, &button_sprite[D_0015f406[0x1b]]);
            }
            D_00155d74 = 0;
        }
    }
    compute_help_text(ox, oy, 0xd);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
    LbTextSetWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
    for (b = win->buttons, j = 1; b; b = b->next, j++) {
        off = 0;
        on = 0;
        if (!((crequester.unknown4 & (1 << j)) || (crequester.unknown6 & (1 << j))))
            continue;
        if (D_0015f402[0x4f] == j) {
            on = D_0015f406[0x14];
            off = D_0015f406[0x18];
        } else if (D_0015f402[0x50] == j) {
            on = D_0015f406[0x15];
            off = D_0015f406[0x19];
        } else if (D_0015f402[0x51] == j) {
            on = D_0015f406[0x16];
            off = D_0015f406[0x1a];
        } else if (D_0015f402[0x52] == j) {
            on = D_0015f406[0x17];
            off = D_0015f406[0x1b];
        }
        if (crequester.unknown4 & (1 << j))
            reqspr_draw(b->x + ox, b->y + oy, &button_sprite[on]);
        else
            reqspr_draw(b->x + ox, b->y + oy, &button_sprite[off]);
    }
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
    LbTextSetWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
    LbTextSetJustifyWindow(ox + D_0015f40c[0x25].x, oy + D_0015f40c[0x25].y, D_0015f40c[0x25].w);
    set_font_from_cache(1);
    D_00188c74 = 0x100;
    LbTextDraw(0, 0, D_001602cc[rooms[crequester.unknown8].id]);
    if (D_0015f3f4 == SCREEN_MODE_13) {
        D_00188c74 = 0x20;
        LbTextSetJustifyWindow(ox + D_0015f40c[0x26].x, oy + D_0015f40c[0x26].y, D_0015f40c[0x26].w);
        LbTextDraw(0, 0, D_001602a0[D_00155d78]);
        D_00188c74 = 0;
    }
    top = oy + D_0015f410[0x6e].y;
    by = top;
    if (crequester.unknown0 >= 6) {
        for (k = 0; k < 0x3e && n < (D_0015f3f5 == SCREEN_MODE_13 ? 8 : 6); k++) {
            count = gamevar.buy_count[k];
            if (count) {
                reqspr_draw(bx, by, &button_sprite[D_0015f406[0x1d]]);
                if ((ox + D_0015f40c[0x27].x <= mouse_dx && ox + D_0015f40c[0x27].x + D_0015f40c[0x27].w > mouse_dx)
                    && by + D_0015f40c[0x29].y <= mouse_dy
                    && by + D_0015f40c[0x27].y + D_0015f40c[0x27].h > mouse_dy) {
                    if (D_00145328 != k)
                        play_sound(10, 0x44, -1, -1);
                    D_00145328 = k;
                    hover = 1;
                    if (D_00188c7e) {
                        if (crequester.unknown9 != k)
                            play_sound(10, 0x43, -1, -1);
                        crequester.unknown9 = k;
                        init_place_object(k, gamevar.unknown26, gamevar.unknown2f, 0);
                        set_edit_room_requester_mode(6);
                    }
                }
                by += D_0015f410[0x6f].y;
                n++;
            }
        }
        if (!hover)
            D_00145328 = -1;
        if (n) {
            reqspr_draw(bx, by + D_0015f410[0x71].y, &button_sprite[D_0015f406[0x1e]]);
            reqspr_draw(bx, oy + D_0015f410[0x70].y, &button_sprite[D_0015f406[0x1c]]);
        }
        gamevar.unknown71 = n;
        D_00188c74 = 0x100;
        by = top;
        n = 0;
        for (k = 0; k < 0x3e && n < (D_0015f3f5 == SCREEN_MODE_13 ? 8 : 6); k++) {
            count = gamevar.buy_count[k];
            if (count) {
                if (crequester.unknown9 == k) {
                    D_00188c74 |= 0x10;
                    LbDrawBox(ox + D_0015f40c[0x27].x, by + D_0015f40c[0x27].y, D_0015f40c[0x27].w, D_0015f40c[0x27].h,
                        color.unknownD);
                    D_00188c74 ^= 0x10;
                }
                if (k == D_00145328)
                    set_font_from_cache(2);
                else
                    set_font_from_cache(1);
                LbTextSetClipWindow(ox + D_0015f40c[0x27].x, by + D_0015f40c[0x27].y,
                    D_0015f40c[0x29].w + D_0015f40c[0x28].w + 0x14, D_0015f40c[0x27].h);
                LbTextSetJustifyWindow(ox + D_0015f40c[0x28].x, by + D_0015f40c[0x28].y, D_0015f40c[0x28].w);
                LbTextDraw(0, 0, D_0016029c[k]);
                LbTextSetJustifyWindow(ox + D_0015f40c[0x29].x, by + D_0015f40c[0x29].y, D_0015f40c[0x29].w);
                sprintf(buf, D_00121ad8, count);
                LbTextDraw(0, 0, buf);
                by += D_0015f410[0x6f].y;
                n++;
            }
        }
        D_00188c74 = 0;
    }
    win->height = by;
    set_all_refresh_zones_in_window(ox, oy, win->width + ox, win->height + oy, 3);
}

// NONMATCHING: register choice
// FUNCTION: 0x0009fcbc
void set_edit_room_requester_mode(const u8 mode)
{
    crequester.unknown0 = mode;
    crequester.unknown6 = 0;
    switch (mode) {
    case 0:
        crequester.unknown4 = (1 << D_0015f402[0x50]) | (1 << D_0015f402[0x51]);
        D_00155d78 = 0xa;
        break;
    case 2:
        crequester.unknown4 = (1 << D_0015f402[0x50]) | (1 << D_0015f402[0x51]) | (1 << D_0015f402[0x52]);
        D_00155d78 = 0xb;
        break;
    case 4:
        crequester.unknown4 = (1 << D_0015f402[0x50]) | (1 << D_0015f402[0x51]);
        D_00155d78 = 0xc;
        break;
    case 8:
        crequester.unknown6 = 1 << D_0015f402[0x51];
    case 6:
        if (crequester.unknown8 == 3 || room_info[allhosps[player].unknown11b7].first_object == 0)
            crequester.unknown4 |= 1 << D_0015f402[0x51];
        if (crequester.unknown8 != 3) {
            if (does_room_contain_compulsory_objects(&room_info[allhosps[player].unknown11b7])) {
                crequester.unknown4 = 0;
                D_00155d78 = 0xe;
                D_0015831b |= 0x10;
            } else {
                crequester.unknown4 = 1 << D_0015f402[0x52];
                D_00155d78 = 0xd;
                D_0015831b &= ~0x10;
            }
        }
        break;
    case 0xa:
        crequester.unknown4 = (1 << D_0015f402[0x52]) | (1 << D_0015f402[0x50]) | (1 << D_0015f402[0x51]);
        D_00155d78 = 0x10;
        crequester.unknown8 = 3;
        break;
    case 1:
    case 3:
    case 5:
    case 7:
    case 9:
    default:
        break;
    }
    if (mode == 8)
        D_00155d78 = 0xf;
    if (crequester.unknown2 != 5)
        open_edit_room_requester();
}

// FUNCTION: 0x0009ff68
void open_edit_room_requester(void)
{
    if (crequester.unknown2 == 0x63) {
        close_irequesters();
        crequester.unknown2 = 5;
        fn_0001bc94(crequester.unknown2, 1);
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x0009ffa8
void go_back_to_place_object_requester(u8 flag)
{
    u8 obj;

    close_irequesters();
    close_crequester(10);
    open_edit_room_requester();
    if (flag)
        gamevar.unknown33 = D_00155c6f;
    else
        post_shop_readjust_build_palette();
    obj = next_object_to_build();
    if (obj) {
        init_place_object(obj, 0, allhosps[D_001553c7].unknown11b7, 0);
    } else {
        set_mouse_mode(0);
        setup_place_object_requester();
    }
}

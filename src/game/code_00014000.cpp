#include "common.h"

struct MenuItem {
    u8 unknown0[8];
    u16 text;
    u8 unknown1[6];
};

struct RoomDef {
    u16 unknown0;
    u8 unknown2[2];
    u8 kind;
    u8 unknown5[4];
    u8 category;
    u8 unknown10[9];
};

struct RoomStat {
    s32 unknown0;
    s32 unknown4;
    u8 unknown8[7];
    u8 value;
    u8 flag;
    u8 unknown17[7];
    u16 unknown24;
    u8 unknown26[12];
};

struct Hospital {
    u8 unknown0[0x1cf];
    s16 unknown1cf[4];
    u8 unknown1d7[0x22c - 0x1d7];
    RoomStat rooms[0x2f];
    u8 unknown[0x1259 - 0x22c - 0x2f * 0x26];
};

union Coord {
    u16 raw;
    struct {
        u8 frac;
        u8 tile;
    };
};

struct Room;

struct Entity {
    s16 prev;
    s16 next;
    Coord x;
    Coord y;
    u16 sprite;
    s16 height;
    u8 renderer;
    u8 kind;
    u8 subtype;
    u8 anim_speed;
    u8 palette;
    u8 unknown17;
    u8 unknown18;
    u8 unknown19;
    u8 unknown20;
    s8 depth;
    u8 owner;
    u8 unknown23[32];
    union {
        s32 mode;
        s16 offset;
        u16 link;
        Room *room;
    };
    union {
        s32 value;
        struct {
            s16 timer;
            u16 unknown61;
        };
    };
    union {
        char text[4];
        u16 unknown63;
    };
    u16 icon;
    u16 unknown69;
    u16 unknown71;
    u16 unknown73;
    u8 unknown75[2];
    u16 room_prev;
    u16 room_next;
    u8 unknown81[4];
    u16 unknown85;
    u8 unknown87[2];
    u16 unknown89;
    u16 unknown91;
    u8 unknown93;
    union {
        u8 state_a;
        struct {
            u8 unknown94;
            u16 group;
        };
    };
    u8 unknown97[3];
    union {
        char label[0x44];
        struct {
            u16 unknown100;
            u8 state_b;
            u8 unknown103;
            u8 unknown104;
            u8 unknown105;
            u8 unknown106;
            u8 unknown107;
            u8 unknown108[8];
            u16 flags;
            u8 unknown118;
            u8 unknown119[3];
            s16 unknown122;
            u16 unknown124;
            u8 unknown126[4];
            s16 unknown130;
            u8 unknown132[2];
            s16 unknown134;
            s16 unknown136;
            u8 unknown138[2];
            u16 unknown140;
            u8 unknown142[6];
            u8 unknown148[10];
            u8 unknown158[10];
        };
    };
    u8 style;
    u8 unknown169;
    u8 unknown170;
    u8 unknown171;
    u8 unknown172[2];
    u8 unknown174;
};

enum ScreenMode {
    SCREEN_MODE_1 = 1,
    SCREEN_MODE_13 = 13
};

struct AnimFrame {
    u8 unknown0[5];
    u8 length;
    u8 unknown6;
    u8 flags;
    u16 next;
};

struct Room {
    u8 type;
    u8 unknown1[3];
    u16 unknown4;
    u16 unknown6;
    u8 unknown8[4];
    u16 first_object;
    u16 first;
    u8 unknown16[4];
    u16 count;
    u32 unknown22;
    u8 unknown26[2];
    u8 owner;
    u8 unknown29[3];
    u8 unknown32;
    u8 unknown33[2];
};

struct HumanoidType {
    u8 unknown0;
    u8 unknown1;
    u8 unknown2;
    u8 unknown3[3];
};

struct AnimLink {
    u8 next;
    u8 unknown1;
};

struct HumanoidAnims {
    u16 idle;
    u8 unknown2[0x116];
};

struct TileInfo {
    u8 type;
};

struct TileType {
    u8 unknown0[0x24];
    u8 unknown36;
    u8 unknown37[5];
};

struct SelectionState {
    u8 unknown0[0xd];
    u16 widget;
    u8 unknown15[0x1d];
};

struct DrawNode {
    DrawNode *next;
    Entity *widget;
    int x;
    int y;
};

struct Cell {
    s16 first;
    u8 overlay;
    u8 unknown3;
    u8 unknown4;
    u8 flags;
    u8 floor;
    u8 unknown7;
};

struct SpriteInfo {
    u32 data;
    u8 width;
    u8 height;
};

struct SpriteRef {
    SpriteInfo *info;
    u8 width;
    s8 height;
};

struct Anim {
    u32 frame;
    u32 flags;
};

struct AnimRef {
    u16 anim;
    u8 unknown2[6];
};

struct FloorDef {
    u16 anim;
    u16 animated;
};

struct HospitalStaff {
    u8 unknown0[0x9c];
    u16 unknown9c[16];
    u8 unknownbc[0xea - 0xbc];
};

struct Offset {
    s32 x;
    s32 y;
};

extern "C" {

extern u8 D_00155c6e;
extern u16 D_00155c64;
extern MenuItem *D_0015f420;
extern char **D_00160310;
extern char D_0015544e[];
extern u16 D_00155444;
extern RoomDef D_0012d900[];
extern s32 D_00154b58;
extern Hospital *D_001547b4;
extern u8 D_0015691b, D_0015691c, D_0015691e, D_0015691f, D_00156920, D_00156921;
extern char **D_00160318;

void fn_000bbff0(char *dst, char *src);
int fn_000bc13a(char *buf, char *fmt, ...);
void fn_00075fe4(u8 room, u8 hospital);
u32 fn_00058e34(u16 room, u8 hospital);
void fn_000143e0(u8 room, u8 hospital);
void fn_0002e0ec(Entity *w, s16 x, s16 y);
void fn_0002d910(u16 sprite, s16 x, s16 y, int flags, u8 palette);
void fn_00031a44(int x, int y, s32 value);
void fn_00031760(char *text, u8 font, int x, int y);
extern u8 D_001549c0;
extern char D_001201a4[];
extern s32 D_00188c64, D_00188c68;
extern s16 D_001567b0[8];
extern u16 D_00155d9d, D_00155d9f;
extern Entity *D_001549a4;
extern s16 D_001553f9, D_00155427;
extern u8 D_00155429;
extern u16 D_001553fb;
extern u16 D_00155e2a;
extern DrawNode D_00154024[];
extern DrawNode D_00154664[];
extern DrawNode *D_00153fa4[32];
extern Cell *D_00153f78;
extern Anim *D_00154894;
extern AnimRef D_0012923c[];
extern FloorDef D_001379f0[];
extern Offset *D_0015f42c;
extern SpriteRef D_0013f71c[];
extern u8 D_0015f3f7, D_0015f3f8;
extern ScreenMode D_0015f3f4;
void fn_000bc0b0(void *dst, int value, u32 size);
void fn_00102c80(int x, int y, SpriteInfo *sprite);
void fn_00014b88(Entity *w, int x, int y, int left, int top);
void fn_00014fbc(Entity *w, int x, int y, int left, int top);
void fn_00015334(Entity *w, int x, int y, int left, int top);
void fn_0001557c(Entity *w, int x, int y);
void fn_000155d4(Entity *w, int x, int y, int left, int top);
void fn_00015760(Entity *w, int x, int y);
void fn_000157b8(Entity *w, int x, int y);
void fn_000157f0(Entity *w, int x, int y);
void fn_000158e8(Entity *w, int x, int y, int left, int top);
void fn_0001599c(Entity *w, int x, int y);
void fn_00015b68(Entity *w);
void fn_00015a20(Entity *w, Coord x, Coord y);
void fn_0001665c(Entity *w);
TileInfo *fn_00023a64(u16 tile);
u8 fn_000417ac(Entity *w, int action, int arg);
void fn_00040524(Entity *w);
extern u32 D_001549c4;
extern AnimFrame *D_00153f64;
extern TileType D_0012c198[];
u8 fn_00047ed0(Entity *w);
void fn_00070f80(Entity *w);
u16 fn_0004b244(Entity *w);
void fn_00017c28(Entity *w, Room *room);
s16 fn_0003791c(u16 tile, int a, int b, int c, u8 owner);
s16 fn_00037d28(u16 id, int a);
int fn_0008dcd0(u16 anim, Entity *w);
s16 fn_000164f4(Entity *w);
u32 fn_000b0100(void);
u16 fn_00019918(Entity *w, u16 tile);
extern s16 D_0012ccfe[];
void fn_00018710(int a, u16 b, u8 owner);
void fn_0003e668(Entity *w);
void fn_000194f0(Entity *w);
Room *fn_00019fe0(int a, int b, u8 owner);
void fn_00016760(Entity *w);
void fn_00041f84(Entity *w);
void fn_000168a0(Entity *w);
void fn_000987c8(Entity *w);
void fn_00087b4c(void);
void fn_000199e8(Entity *w);
extern SelectionState D_001581b2;
Room *fn_0002384c(int tile);
void fn_00016808(Room *room, Entity *w);
u16 fn_0001974c(void);
extern Room *D_001547a4;
extern HumanoidType D_0012cf54[];
extern HumanoidAnims D_00136422[];
extern HumanoidAnims D_00137482[];
extern AnimLink D_0012922c[];
extern s8 D_0012b90c[][15];
void fn_000444e0(Entity *w);
int fn_0001905c(Entity *w);
extern Room *D_001547a8;
extern HumanoidAnims D_00136410[];
void fn_00049820(int a, u16 room, int b, int c, u8 owner);
void fn_0004195c(Entity *w, u16 a);
Room *fn_00049e88(int type, Entity *w);
void fn_0003c4ec(int a, Entity *w);
u16 fn_000382d8(Entity *w, int a, int b);
void fn_00060b54(int a, int b);
Entity *fn_0001b150(Room *room, int a, int b);
void fn_0005cc70(Entity *a, Entity *b);
u16 fn_00037bec(int a, int b, int c, u8 owner);
void fn_0007a710(u16 tile, s8 owner, int a);
void fn_000b05f0(Entity *w);
void fn_00098950(Entity *w);
u8 fn_0003e468(Entity *w);
void fn_000b0428(Entity *w, int a);
extern u16 D_0015543a;
extern HospitalStaff *D_00154820;
extern s32 D_0015f78e;
extern s16 D_0012dd7c[14];
extern u8 D_00155441;
void fn_000b0300(Entity *w, int a);
Room *fn_0001a0a0(Entity *w, int a);
void fn_0004b058(Entity *w);
void fn_00030198(u16 sprite, s16 x, s16 y, int flags, u8 palette, int scale);
extern u16 D_001553fd;
extern u8 D_001553d4;
void fn_0002f47c(Entity *w, s16 x, s16 y);
void fn_0002e804(Entity *w, s16 x, s16 y);
void fn_0002fa3c(Entity *w, s16 x, s16 y);
void fn_0002fe18(Entity *w, s16 x, s16 y);
void fn_000305fc(Entity *w, s16 x, s16 y);
void fn_00030f34(u16 icon, s16 x, s16 y, int flags, char *label, u8 palette);

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x0001403c
void fn_0001403c(void)
{
    if (D_00155c6e) {
        fn_000bbff0(D_0015544e, D_00160310[D_0015f420[D_00155c64].text]);
    }
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x00014090
void fn_00014090(void)
{
    if (D_00155444 >= 0x24 || D_00155444 == 1) {
        return;
    }
    if (D_0012d900[D_00155444].kind == 0x27) {
        if (D_001547b4[D_00154b58].rooms[D_00155444].flag == 1) {
            fn_000bc13a(D_0015544e, D_00160310[148], D_001547b4[D_00154b58].rooms[D_00155444].value);
        } else {
            fn_000bbff0(D_0015544e, D_00160310[147]);
        }
    } else if (D_0012d900[D_00155444].unknown0 == 0x40) {
        fn_000bbff0(D_0015544e, D_00160310[149]);
    } else if (D_0012d900[D_00155444].category == 10) {
        fn_000bbff0(D_0015544e, D_00160310[150]);
    } else {
        fn_000bbff0(D_0015544e, D_00160310[151]);
    }
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x000141f0
void fn_000141f0(void)
{
    if (D_001547b4[D_00154b58].rooms[D_00155444].flag == 1) {
        fn_000bbff0(D_0015544e, D_00160310[152]);
        return;
    }
    fn_00075fe4(D_00155444, D_00154b58);
    if (D_0015691f) {
        fn_000bbff0(D_0015544e, D_00160310[153]);
    } else if (D_0015691e) {
        fn_000bbff0(D_0015544e, D_00160310[154]);
    } else if (D_0015691c == 2) {
        fn_000bbff0(D_0015544e, D_00160310[155]);
    } else if (D_0015691c == 1) {
        fn_000bbff0(D_0015544e, D_00160310[156]);
    } else if (D_0015691b) {
        fn_000bc13a(D_0015544e, D_00160310[157], D_00160318[D_0015691b]);
    } else if (D_00156920) {
        fn_000bbff0(D_0015544e, D_00160310[158]);
    } else if (D_00156921) {
        fn_000bbff0(D_0015544e, D_00160310[159]);
    } else {
        fn_000bbff0(D_0015544e, D_00160310[160]);
    }
}

// FUNCTION: 0x0001438c
void fn_0001438c(void)
{
    u32 hospital;
    u32 room;

    for (hospital = 0; hospital < 4; hospital++) {
        for (room = 0; room < 0x2f; room++) {
            fn_000143e0(room, hospital);
        }
    }
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x000143e0
void fn_000143e0(u8 room, u8 hospital)
{
    s32 count;
    s32 target;
    s32 diff;

    count = fn_00058e34(room, hospital);
    target = count * D_001547b4[hospital].rooms[room].unknown4 / 10000;
    if (target - D_001547b4[hospital].rooms[room].unknown0 < 0) {
        diff = -(target - D_001547b4[hospital].rooms[room].unknown0);
    } else {
        diff = target - D_001547b4[hospital].rooms[room].unknown0;
    }
    if (target / 100 <= diff) {
        D_001547b4[hospital].rooms[room].unknown0 = target;
    }
}

// FUNCTION: 0x0001557c
void fn_0001557c(Entity *w, int x, int y)
{
    if (w->kind != 0xf2 || !(D_001549c0 & 1)) {
        if (w->kind == 6) {
            return;
        }
        fn_0002e0ec(w, x, y);
    }
}

// FUNCTION: 0x00015760
void fn_00015760(Entity *w, int x, int y)
{
    fn_0002d910(w->sprite, x, y - (w->offset >> 8), 0, w->palette);
}

// FUNCTION: 0x000157b8
void fn_000157b8(Entity *w, int x, int y)
{
    fn_0002e0ec(w, x, y - w->offset);
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x000157f0
void fn_000157f0(Entity *w, int x, int y)
{
    char buf[100];

    switch (w->mode) {
    case 3:
        fn_00031a44(x, w->unknown18 + y - w->anim_speed, w->value);
        break;
    default:
        if (w->mode == 0) {
            fn_000bc13a(buf, D_001201a4, w->value);
        } else if (w->mode == 1) {
            fn_000bbff0(buf, w->text);
        } else if (w->mode == 2) {
            fn_000bc13a(buf, w->text, w->value);
        }
        fn_00031760(buf, w->subtype, x, y + w->unknown18 - w->anim_speed);
        break;
    }
}

// FUNCTION: 0x000155d4
void fn_000155d4(Entity *w, int x, int y, int left0, int top0)
{
    int mouse_x;
    int mouse_y;
    int left;
    int right;
    int top;
    int bottom;

    mouse_x = D_00188c64;
    mouse_y = D_00188c68;
    fn_0002e0ec(w, x, y);
    left = left0 + D_001567b0[4] - D_00155d9d;
    right = left + D_001567b0[6] + D_00155d9d * 2;
    top = top0 + D_001567b0[5] - D_00155d9d;
    bottom = top + D_001567b0[7] + D_00155d9d * 2;
    if (mouse_x > left && mouse_x < right && mouse_y > top && mouse_y < bottom) {
        if (left + D_00155d9d - D_00155d9f < mouse_x && right - D_00155d9d + D_00155d9f > mouse_x
            && top + D_00155d9d - D_00155d9f < mouse_y && bottom - D_00155d9d + D_00155d9f > mouse_y) {
            D_001553f9 = w - D_001549a4;
            D_00155427 = w - D_001549a4;
        } else {
            D_00155429 = 1;
        }
    }
}

// NONMATCHING: register allocation; also the D_00153fa4[bucket] store (index in edx), see MATCHING.md known unsolved
// FUNCTION: 0x00014510
void fn_00014510(Cell *cell, int x, int y, int left, int top)
{
    u8 floor;
    u16 screen_y;
    s16 id;
    s16 bucket;
    u16 screen_x;
    DrawNode *node;
    int i;
    Entity *w;

    node = D_00154024;
    fn_000bc0b0(D_00153fa4, 0, sizeof(D_00153fa4));
    D_001567b0[0] = D_001567b0[1] = 999;
    D_001567b0[2] = D_001567b0[3] = 0;
    for (id = cell->first; id > 0; id = w->next) {
        w = &D_001549a4[id];
        screen_x = D_0015f3f4 == SCREEN_MODE_13 ? ((w->x.frac - w->y.frac) >> 3) + 0x20 : ((w->x.frac - w->y.frac) >> 4) + 0x10;
        screen_y = D_0015f3f4 == SCREEN_MODE_13 ? (w->y.frac + w->x.frac) >> 4 : (w->y.frac + w->x.frac) >> 5;
        bucket = ((w->y.frac + w->x.frac) >> 4) + w->depth;
        if (node < D_00154664) {
            node->next = D_00153fa4[bucket];
            D_00153fa4[bucket] = node;
            node->widget = w;
            node->x = screen_x;
            node->y = screen_y;
            node++;
        }
    }
    if (cell->flags & 0xc0) {
        u32 sprite;

        sprite = D_00154894[((cell->flags & 0xc0) >> 6) + 625].frame;
        if (cell->flags & 0x10) {
            fn_00102c80(x, D_0015f3f7 + y - D_0013f71c[cell->overlay].info->height, D_0013f71c[cell->overlay].info);
        }
        fn_0002d910(sprite, D_0015f3f8 / 2 + x, D_0015f3f7 / 2 + y, 0, 0);
    }
    if (id < 0) {
        fn_0002d910(D_00154894[D_0012923c[-id].anim].frame, (D_0015f3f8 >> 1) + x, (D_0015f3f7 >> 1) + y, 0,
            D_00154894[D_0012923c[-id].anim].flags);
    }
    floor = cell->floor & 0xf;
    if (floor) {
        u16 frame;
        u8 palette;

        frame = D_00154894[D_001379f0[floor].anim].frame;
        palette = D_00154894[D_001379f0[floor].anim].flags;
        if (D_001379f0[floor].animated) {
            fn_0002d910(frame, x + (D_0015f3f4 == SCREEN_MODE_13 ? 32 : 16), y + (D_0015f3f4 == SCREEN_MODE_13 ? 16 : 8), 0, palette);
        } else {
            Offset *offset;

            offset = &D_0015f42c[((u16)(cell - D_00153f78) * 0x2e4a4321 >> 16) & 7];
            fn_0002d910(frame, x + D_0015f3f8 / 2 + offset->x, y + D_0015f3f7 / 2 + offset->y, 0, palette);
        }
    }
    for (i = 0; i < 32; i++) {
        for (node = D_00153fa4[i]; node != 0; node = node->next) {
            int draw_x;
            int draw_y;

            draw_x = node->x + x;
            draw_y = node->y + y;
            switch (node->widget->renderer) {
            case 2:
                fn_00014b88(node->widget, draw_x, draw_y, left, top);
                break;
            case 4:
                fn_00014fbc(node->widget, draw_x, draw_y, left, top);
                break;
            case 6:
            case 8:
                fn_00015334(node->widget, draw_x, draw_y, left, top);
                break;
            case 10:
                fn_0001557c(node->widget, draw_x, draw_y);
                break;
            case 12:
                fn_000155d4(node->widget, draw_x, draw_y, left, top);
                break;
            case 14:
                fn_00015760(node->widget, draw_x, draw_y);
                break;
            case 16:
                fn_000157b8(node->widget, draw_x, draw_y);
                break;
            case 18:
                fn_000157f0(node->widget, draw_x, draw_y);
                break;
            case 20:
            case 22:
                fn_000158e8(node->widget, draw_x, draw_y, left, top);
                break;
            case 24:
                fn_0001599c(node->widget, draw_x, draw_y);
            }
        }
    }
}

// FUNCTION: 0x00014b88
void fn_00014b88(Entity *w, int x, int y, int left, int top)
{
    D_00155e2a++;
    switch (w->kind) {
    case 4:
    case 6:
    case 8:
    case 12:
    case 16:
    case 18:
    case 20:
    case 22:
    case 24:
    case 26:
    case 28:
    case 32:
    case 34:
    case 36:
    case 38:
    case 42:
    case 44:
    case 50:
    case 54:
    case 56:
    case 58:
    case 60:
    case 64:
    case 68:
    case 70:
    case 240:
        if (w->style == 6) {
            fn_0002f47c(w, x, y);
        } else if (w->style == 12 && D_001553d4 == 0 && w->unknown69 == 0) {
            fn_0002e804(w, x, y);
        } else if ((u16)(w - D_001549a4) == D_001553fb) {
            fn_000305fc(w, x, y);
        } else if ((u16)(w - D_001549a4) == D_001553fd) {
            fn_000305fc(w, x, y);
        } else {
            fn_0002e0ec(w, x, y);
        }
        break;
    case 46:
        fn_0002fa3c(w, x, y);
        break;
    case 48:
        if (D_001553d4 == 0) {
            fn_0002fe18(w, x, y);
        } else {
            fn_0002e0ec(w, x, y);
        }
        break;
    case 0xf2:
        break;
    case 52:
    case 72:
        fn_0002e0ec(w, x, y - w->height);
        break;
    case 14:
    case 40:
    case 66:
        return;
    default:
        break;
    }
    if (D_001567b0[4] + left < D_00188c64 && D_001567b0[4] + D_001567b0[6] + left > D_00188c64
        && D_001567b0[5] + top < D_00188c68 && D_001567b0[5] + D_001567b0[7] + top > D_00188c68) {
        D_001553f9 = w - D_001549a4;
    }
}

// FUNCTION: 0x00014fbc
void fn_00014fbc(Entity *w, int x, int y, int left, int top)
{
    D_00155e2a++;
    switch (w->kind) {
    case 2:
    case 4:
    case 6:
    case 8:
    case 10:
    case 12:
    case 14:
    case 16:
    case 18:
    case 20:
    case 24:
    case 26:
    case 28:
    case 32:
    case 34:
    case 36:
    case 38:
    case 40:
    case 44:
    case 48:
    case 50:
    case 52:
    case 54:
    case 58:
    case 60:
    case 62:
    case 64:
    case 66:
    case 70:
    case 72:
    case 74:
    case 76:
    case 78:
    case 80:
    case 240:
        if ((u16)(w - D_001549a4) == D_001553fb) {
            fn_000305fc(w, x, y);
        } else {
            fn_0002e0ec(w, x, y);
        }
        break;
    case 42:
    case 46:
    case 56:
        return;
    default:
        break;
    }
    if (D_001567b0[4] + left < D_00188c64 && D_001567b0[4] + D_001567b0[6] + left > D_00188c64
        && D_001567b0[5] + top < D_00188c68 && D_001567b0[5] + D_001567b0[7] + top > D_00188c68) {
        D_001553f9 = w - D_001549a4;
    }
}

// FUNCTION: 0x00015334
void fn_00015334(Entity *w, int x, int y, int left, int top)
{
    switch (w->kind) {
    case 2:
    case 6:
    case 10:
    case 14:
    case 16:
    case 20:
    case 22:
    case 24:
    case 26:
    case 28:
        if ((u16)(w - D_001549a4) == D_001553fb || (w->group && w->group == D_001553fb)) {
            fn_000305fc(w, x, y);
        } else {
            fn_0002e0ec(w, x, y);
        }
        if (D_001567b0[4] + left < D_00188c64 && D_001567b0[4] + D_001567b0[6] + left > D_00188c64
            && D_001567b0[5] + top < D_00188c68 && D_001567b0[5] + D_001567b0[7] + top > D_00188c68) {
            D_001553f9 = w - D_001549a4;
        }
        if (w->icon) {
            fn_00030f34(w->icon, x, y, 0, w->label, w->palette);
        }
        break;
    case 4:
        if (w->icon) {
            fn_00030f34(w->icon, x, y, 0, w->label, w->palette);
        }
        break;
    }
}

// FUNCTION: 0x000158e8
void fn_000158e8(Entity *w, int x, int y, int left, int top)
{
    fn_0002e0ec(w, x, y);
    if (D_001567b0[4] + left < D_00188c64 && D_001567b0[4] + D_001567b0[6] + left > D_00188c64
        && D_001567b0[5] + top < D_00188c68 && D_001567b0[5] + D_001567b0[7] + top > D_00188c68) {
        D_001553f9 = w - D_001549a4;
    }
}

// FUNCTION: 0x0001599c
void fn_0001599c(Entity *w, int x, int y)
{
    Entity *other;

    other = &D_001549a4[w->link];
    fn_00030198(w->sprite, x, y, 0, w->palette, 0x640000 / (other->offset + 100));
}

// FUNCTION: 0x00015a20
void fn_00015a20(Entity *w, Coord x, Coord y)
{
    u16 first;
    Cell *cell;
    int id;
    Entity *other;

    cell = &D_00153f78[(y.tile << 7) + x.tile];
    id = w - D_001549a4;
    first = cell->first;
    if (first) {
        w->next = first;
        if (w->next > 0) {
            other = &D_001549a4[first];
            other->prev = id;
        }
    }
    cell->first = id;
}

// FUNCTION: 0x00015ac4
int fn_00015ac4(Entity *w, Coord x, Coord y)
{
    if (x.tile >= 0x80 || y.tile >= 0x80) {
        return 0;
    }
    if (x.tile != w->x.tile || y.tile != w->y.tile) {
        fn_00015b68(w);
        fn_00015a20(w, x, y);
        w->x = x;
        w->y = y;
        return 1;
    }
    w->x = x;
    w->y = y;
    return 0;
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x00015b68
void fn_00015b68(Entity *w)
{
    Cell *cell;
    Entity *other;

    if (w->prev == 0) {
        cell = &D_00153f78[((s16)w->y.tile << 7) + (s16)w->x.tile];
        cell->first = w->next;
        if (cell->first > 0) {
            other = &D_001549a4[w->next];
            other->prev = 0;
        }
    } else {
        if ((D_001549a4[w->prev].next = w->next) > 0) {
            D_001549a4[w->next].prev = w->prev;
        }
    }
    w->next = 0;
    w->prev = 0;
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x00015c50
void fn_00015c50(Entity *w)
{
    if (D_001549c4 % w->anim_speed == 0) {
        w->sprite = D_00153f64[w->sprite].next;
    }
    if (w->timer-- <= 0) {
        fn_0001665c(w);
    }
}

// NONMATCHING: register allocation of the tile index expression
// FUNCTION: 0x00015cc0
void fn_00015cc0(Entity *w)
{
    u16 tile;
    TileInfo *info;

    tile = ((u16)w->y.tile << 7) + (u16)w->x.tile;
    info = fn_00023a64(tile);
    if (info != 0 && D_0012c198[info->type].unknown36 != 0) {
        fn_000417ac(w, 0x32, 0);
        fn_00040524(w);
    } else {
        w->kind = 0x14;
    }
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x00015d48
void fn_00015d48(Entity *w)
{
    AnimFrame *frame;
    Room *room;

    if (D_001549c4 % w->anim_speed == 0) {
        w->sprite = D_00153f64[w->sprite].next;
        frame = &D_00153f64[w->sprite];
        if (frame->flags & 1) {
            w->height--;
            if (w->height <= 0) {
                if (D_0013f71c[D_00153f78[((s16)w->y.tile << 7) + (s16)w->x.tile].overlay].width == 0x20
                    && fn_00047ed0(w)) {
                    return;
                }
                fn_00070f80(w);
                if (w->unknown130 > 0) {
                    room = &D_001547a4[fn_0004b244(w)];
                    if (room > D_001547a4) {
                        fn_00017c28(w, room);
                    }
                    if (D_0012cf54[w->subtype].unknown0 && w->unknown134 < 30 && w->icon != 60) {
                        if (fn_0003791c(((u16)w->y.tile << 7) + (u16)w->x.tile, 7, 1, 150, w->owner)) {
                            w->state_a = 0x18;
                            w->state_b = 0x18;
                            fn_000417ac(w, 60, 0);
                            fn_00040524(w);
                            return;
                        }
                    }
                    if (w->icon != 70) {
                        u16 anim;

                        if (D_0012cf54[w->subtype].unknown1) {
                            if (fn_00037d28(w - D_001549a4, 4)) {
                                if (w->owner == D_00154b58) {
                                    D_0015f78e -= 5;
                                }
                                w->state_a = 0x18;
                                w->state_b = 0x18;
                                fn_000417ac(w, 70, 0);
                                fn_00040524(w);
                                return;
                            }
                            if (w->owner == D_00154b58) {
                                D_0015f78e++;
                            }
                        }
                        anim = fn_0008dcd0(D_00136422[w->subtype].idle, w) + (u16)w->unknown17;
                        w->sprite = D_00154894[anim].frame;
                        w->palette = D_00154894[anim].flags > 0 ? (u8)1 : (u8)0;
                        if (fn_000164f4(w)) {
                            return;
                        }
                    }
                    w->height = w->unknown18 == 0 ? fn_0001974c() : 0x100 / w->unknown18;
                    w->kind = 0x18;
                }
            }
        }
    }
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x000160a8
void fn_000160a8(Entity *w)
{
    AnimFrame *frame;
    Room *room;

    if (D_001549c4 % w->anim_speed == 0) {
        w->sprite = D_00153f64[w->sprite].next;
        frame = &D_00153f64[w->sprite];
        if (frame->flags & 1) {
            w->height--;
            if (w->height <= 0) {
                if (D_0013f71c[D_00153f78[((s16)w->y.tile << 7) + (s16)w->x.tile].overlay].width == 0x20
                    && fn_00047ed0(w)) {
                    return;
                }
                fn_00070f80(w);
                if (w->unknown130 > 0) {
                    room = &D_001547a4[fn_0004b244(w)];
                    if (room > D_001547a4) {
                        fn_00017c28(w, room);
                    }
                    if (w->unknown124 > D_0012dd7c[6] || (w->flags & 0x100)) {
                        if (w->owner == D_00154b58) {
                            D_00155441++;
                        }
                        fn_000b0300(w, 7);
                        fn_0001665c(w);
                        w->flags |= 0x4000;
                        return;
                    }
                    if (D_0012cf54[w->subtype].unknown0 && w->unknown134 < 30 && w->icon != 60) {
                        if (fn_0003791c(((u16)w->y.tile << 7) + (u16)w->x.tile, 7, 1, 150, w->owner)) {
                            w->state_a = 0x18;
                            w->state_b = 0x18;
                            fn_000417ac(w, 60, 0);
                            fn_00040524(w);
                            return;
                        }
                    }
                    if (D_0012cf54[w->subtype].unknown2 && w->unknown136 > D_0012dd7c[13] && w->icon != 90) {
                        room = fn_0001a0a0(w, 0x1d);
                        if (room != 0) {
                            fn_0004b058(w);
                            w->state_a = 0x18;
                            w->state_b = 0x36;
                            w->unknown103 = 0;
                            fn_000417ac(w, 360, room->unknown4);
                            fn_00040524(w);
                            return;
                        }
                    }
                    if (w->icon != 70) {
                        u16 anim;

                        if (D_0012cf54[w->subtype].unknown1) {
                            if (fn_00037d28(w - D_001549a4, 4)) {
                                if (w->owner == D_00154b58) {
                                    D_0015f78e -= 5;
                                }
                                w->state_a = 0x18;
                                w->state_b = 0x18;
                                fn_000417ac(w, 70, 0);
                                fn_00040524(w);
                                return;
                            }
                            if (w->owner == D_00154b58) {
                                D_0015f78e++;
                            }
                        }
                        anim = fn_0008dcd0(D_00136422[w->subtype].idle, w) + (u16)w->unknown17;
                        w->sprite = D_00154894[anim].frame;
                        w->palette = D_00154894[anim].flags > 0 ? (u8)1 : (u8)0;
                        if (fn_000164f4(w)) {
                            return;
                        }
                    }
                    w->height = w->unknown18 == 0 ? fn_0001974c() : 0x100 / w->unknown18;
                }
            }
        }
    }
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x000164f4
s16 fn_000164f4(Entity *w)
{
    u16 base;
    u16 i;
    u16 tile;
    Cell *target;
    Cell *cell;

    cell = &D_00153f78[((s16)w->y.tile << 7) + (s16)w->x.tile];
    if (w->prev != 0 || (cell->flags & 7)) {
        base = (w->y.tile << 7) + (u16)w->x.tile;
        for (i = fn_000b0100() % 12 + 1; i <= D_0012ccfe[1]; i++) {
            tile = base + D_0012ccfe[i + 1];
            target = &D_00153f78[tile];
            if (target->first == 0 && !(target->flags & 7) && fn_00019918(w, tile) > 0 && !(target->unknown7 & 0x10)) {
                break;
            }
        }
        if (D_0012ccfe[i + 1]) {
            w->state_a = w->kind;
            w->state_b = w->kind;
            fn_000417ac(w, 430, tile);
            fn_00040524(w);
            return 1;
        }
    }
    return 0;
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x0001665c
void fn_0001665c(Entity *w)
{
    if (w->unknown91) {
        fn_00018710(8, w->unknown91, w->owner);
        fn_00018710(4, w->unknown91, w->owner);
        w->flags &= ~0x10;
        w->unknown91 = 0;
    }
    w->state_b = 0;
    fn_0003e668(w);
    fn_000194f0(w);
    fn_0004b058(w);
    if (fn_000417ac(w, 20, fn_00019fe0(1, 0x65, w->owner)->unknown4)) {
        fn_00040524(w);
    }
    fn_000b0300(w, 7);
}

// FUNCTION: 0x0001673c
void fn_0001673c(Entity *w)
{
    fn_00016760(w);
}

// FUNCTION: 0x00016760
void fn_00016760(Entity *w)
{
    fn_0004b058(w);
    fn_00041f84(w);
    fn_000168a0(w);
    fn_000987c8(w);
    if (&D_001549a4[D_001581b2.widget] == w) {
        fn_00087b4c();
    }
    fn_000199e8(w);
}

// FUNCTION: 0x000167c4
void fn_000167c4(Entity *w, u16 id)
{
    Room *room;

    room = fn_0002384c(id);
    if (room != 0) {
        fn_00016808(room, w);
    } else {
        fn_000168a0(w);
    }
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x00016808
void fn_00016808(Room *room, Entity *w)
{
    u16 first;
    int id;

    id = w - D_001549a4;
    if (room != 0) {
        first = room->first;
        if (first) {
            w->room_next = first;
            D_001549a4[first].room_prev = id;
        }
        room->first = id;
    }
    w->room = room;
    if (w->renderer == 2) {
        room->count++;
    }
}

// FUNCTION: 0x000168a0
void fn_000168a0(Entity *w)
{
    Room *room;
    Entity *prev;
    Entity *next;

    room = w->room;
    if (room != 0) {
        if (room->first) {
            prev = &D_001549a4[w->room_prev];
            if (prev > D_001549a4) {
                prev->room_next = w->room_next;
            } else {
                room->first = w->room_next;
            }
            next = &D_001549a4[w->room_next];
            if (next > D_001549a4) {
                next->room_prev = w->room_prev;
            }
        }
        w->room = 0;
        w->room_next = 0;
        w->room_prev = 0;
    }
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x00016974
void fn_00016974(Entity *w)
{
    if (w->renderer == 2) {
        if (D_001549c4 % 6 == 0) {
            u16 anim;

            w->unknown17 = D_0012922c[w->unknown17].next;
            anim = fn_0008dcd0(D_00136422[w->subtype].idle, w) + (u16)w->unknown17;
            w->sprite = D_00154894[anim].frame;
            w->palette = D_00154894[anim].flags > 0 ? (u8)1 : (u8)0;
        }
        if (!(D_001549c4 & 0xf)) {
            w->kind = w->state_a;
            fn_000444e0(w);
        }
    } else {
        if (D_001549c4 % 6 == 0) {
            u16 anim;

            w->unknown17 = D_0012922c[w->unknown17].next;
            anim = fn_0008dcd0(D_00137482[w->subtype].idle, w) + (u16)w->unknown17;
            w->sprite = D_00154894[anim].frame;
            w->palette = D_00154894[anim].flags > 0 ? (u8)1 : (u8)0;
        }
        if (!(D_001549c4 & 0xf)) {
            w->kind = w->state_a;
            fn_000444e0(w);
        }
    }
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x00016b30
u8 fn_00016b30(Entity *w, u8 level)
{
    s8 subtype;

    subtype = D_0012b90c[level][w->subtype];
    if (w->renderer == 2) {
        w->unknown170 = w->subtype;
    }
    if (subtype != -1) {
        w->subtype = subtype;
        return 1;
    }
    return 0;
}

// FUNCTION: 0x00016ba8
u16 fn_00016ba8(Entity *w, u16 id, u16 action, u16 arg, u8 follow)
{
    Entity *other;

    other = &D_001549a4[id];
    while (1) {
        if (other->unknown130 <= 0) {
            id = other->unknown122;
            if (id == 0) {
                break;
            }
            fn_0004b058(other);
            other = &D_001549a4[id];
        } else {
            if (fn_0001905c(other)) {
                w->unknown73 = id;
                w->timer = 200;
                w->unknown122 = (fn_000b0100() & 3) + 3;
                fn_0004b058(other);
                if (fn_000417ac(other, action, arg)) {
                    fn_00040524(other);
                }
                other->state_b = 0x46;
                return id;
            }
            if (follow == 0) {
                break;
            }
            id = other->unknown122;
            if (id == 0) {
                break;
            }
            other = &D_001549a4[id];
        }
    }
    return 0;
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x00016cdc
u8 fn_00016cdc(Entity *w, u16 action, u16 arg)
{
    Entity *other;
    u16 tile;

    other = &D_001549a4[w->unknown73];
    if (other > D_001549a4) {
        if (other->unknown130 <= 0) {
            return 0;
        }
        if (other->renderer != 2) {
            return 0;
        }
        if (other->unknown140) {
            return 0;
        }
        if (!fn_0001905c(other)) {
            w->timer = 200;
            return 2;
        }
        if (other->unknown61 / 10 * 10 != action) {
            if (other->unknown61 == 0) {
                if (fn_000417ac(other, action, arg)) {
                    fn_00040524(other);
                }
                other->state_b = 0x46;
            } else if (other->unknown63 != action) {
                if (other->unknown63 == 0) {
                    if (fn_000417ac(other, action, arg)) {
                        fn_00040524(other);
                    }
                    other->state_b = 0x46;
                } else {
                    return 0;
                }
            }
        } else {
            tile = ((u16)other->unknown20 << 7) + (u16)other->unknown19;
            if (tile != arg) {
                return 0;
            }
        }
        if (other->state_b != 0x46) {
            other->state_b = 0x46;
        }
        w->timer = 200;
        w->unknown122--;
        if (w->unknown122 <= 0) {
            return 1;
        }
    } else {
        return 0;
    }
    return 2;
}

// NONMATCHING: register allocation; stack slots of anim and the ternary temporary are swapped
// FUNCTION: 0x00016eb0
void fn_00016eb0(Entity *w)
{
    Room *room;
    Entity *other;

    w->timer = fn_000b0100() % 300 + 300;
    for (room = D_001547a4 + 1; room < D_001547a8; room++) {
        u16 anim;

        if (room->owner != w->owner || room->type != 9) {
            continue;
        }
        for (other = &D_001549a4[room->first_object]; other > D_001549a4; other = &D_001549a4[other->unknown71]) {
            if (other->subtype != 8 || other->unknown91 != 0) {
                continue;
            }
            fn_00049820(4, room - D_001547a4, 1, 1, w->owner);
            w->unknown89 = room - D_001547a4;
            fn_0004195c(w, room->unknown4);
            anim = fn_0008dcd0(D_00136410[w->subtype].idle, w) + (u16)w->unknown17;
            w->sprite = D_00154894[anim].frame;
            w->palette = D_00154894[anim].flags > 0 ? (u8)1 : (u8)0;
            w->kind = 0x24;
            room->unknown32++;
            return;
        }
    }
    room = fn_00049e88(0x2a, w);
    room->unknown32++;
    fn_00049820(4, room - D_001547a4, 1, 1, w->owner);
    w->unknown89 = room - D_001547a4;
    {
        u16 anim;

        anim = fn_0008dcd0(D_00136410[w->subtype].idle, w) + (u16)w->unknown17;
        w->sprite = D_00154894[anim].frame;
        w->palette = D_00154894[anim].flags > 0 ? (u8)1 : (u8)0;
    }
    w->kind = 0x24;
    fn_0004195c(w, room->unknown4);
}

// FUNCTION: 0x00017194
void fn_00017194(Entity *w)
{
    if (w->timer != 0) {
        if (D_001549c4 % 30 == 0) {
            fn_00070f80(w);
        }
        w->timer--;
        if (w->timer == 0) {
            fn_000417ac(w, 240, 0);
            fn_00040524(w);
            fn_0003c4ec(0, w);
        }
    }
}

// FUNCTION: 0x0001720c
u16 fn_0001720c(u16 room_id, u8 random, u8 renderer, u8 kind, u8 owner)
{
    u16 count;
    Room *room;
    Entity *obj;

    count = 0;
    room = &D_001547a4[room_id];
    if (random) {
        for (obj = &D_001549a4[room->first]; obj > D_001549a4; obj = &D_001549a4[obj->room_next]) {
            if (obj->owner == owner) {
                if (renderer && obj->renderer != renderer) {
                    continue;
                }
                if (kind && obj->kind != kind) {
                    continue;
                }
                count++;
            }
        }
        if (count == 0) {
            return 0;
        }
        count = fn_000b0100() % count + 1;
    } else {
        count = 1;
    }
    for (obj = &D_001549a4[room->first]; obj > D_001549a4; obj = &D_001549a4[obj->room_next]) {
        if (obj->owner == owner) {
            if (renderer && obj->renderer != renderer) {
                continue;
            }
            if (kind && obj->kind != kind) {
                continue;
            }
            if (--count == 0) {
                return obj - D_001549a4;
            }
        }
    }
    return 0;
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x000173c0
void fn_000173c0(Entity *w)
{
    u8 go;
    u16 done;
    u16 flag;
    u16 id;
    u16 arg;
    Entity *other;
    Room *room;

    done = 0;
    flag = 0;
    go = 0;
    if (w->room > D_001547a4) {
        if (fn_000417ac(w, 90, w->room->unknown6)) {
            go = 1;
        }
        flag = 1;
    }
    id = fn_000382d8(w, 0, 1);
    if (id) {
        other = &D_001549a4[id];
        arg = other->room_next;
        w->unknown89 = D_001549a4[other->unknown91].room - D_001547a4;
        if (fn_000417ac(w, 470, arg)) {
            go = 1;
        }
        done = 1;
    }
    if (done == 0) {
        room = &D_001547a4[D_001547b4[w->owner].unknown1cf[fn_000b0100() & 3]];
        arg = room->unknown4;
        if (fn_000417ac(w, 20, arg)) {
            go = 1;
        }
    }
    if (go) {
        fn_00040524(w);
    }
}

// NONMATCHING: register allocation of the tile index argument
// FUNCTION: 0x0001752c
void fn_0001752c(Entity *w)
{
    Room *room;

    if (w->room == 0) {
        room = fn_0002384c((u16)(((u16)w->unknown20 << 7) + (u16)w->unknown19));
        if (room != 0 && D_001549c4 - 500 < room->unknown22) {
            fn_000173c0(w);
            return;
        }
    }
    if (w->state_b == 0x14 || w->state_b == 0x18) {
        w->kind = w->state_b;
        return;
    }
    w->unknown105 = w->kind;
    w->unknown106 = w->unknown19;
    w->unknown107 = w->unknown20;
    w->kind = 0x3c;
    w->height = 0;
    if (w->owner == D_00154b58) {
        fn_00060b54(1, 1);
    }
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x00017620
void fn_00017620(Entity *w)
{
    Entity *found;
    Entity *other;
    AnimFrame *frame;

    other = &D_001549a4[w->unknown69];
    if (other > D_001549a4) {
        if (D_001549c4 % w->anim_speed == 0) {
            w->sprite = D_00153f64[w->sprite].next;
            other->unknown100 = D_00153f64[other->unknown100].next;
        }
        if (w->subtype == 14) {
            other->unknown103 += 2;
            frame = &D_00153f64[other->unknown100];
        } else {
            other->unknown103++;
            frame = &D_00153f64[w->sprite];
        }
        if (other->unknown103 > frame->length) {
            w->kind = 0x20;
            w->sprite = other->unknown100;
            w->palette = other->state_b;
            if (w->subtype == 14) {
                found = fn_0001b150(w->room, 4, 1);
                if (found != 0) {
                    fn_0005cc70(found, found + 1);
                }
            }
        }
    }
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x00017780
void fn_00017780(Entity *w)
{
    u16 next;
    Entity *other;

    other = &D_001549a4[w->unknown69];
    if (other > D_001549a4) {
        if (D_001549c4 % w->anim_speed == 0) {
            next = D_00153f64[w->sprite].next;
            if (!(D_00153f64[next].flags & 1)) {
                w->sprite = D_00153f64[w->sprite].next;
                other->unknown100 = D_00153f64[other->unknown100].next;
            }
        }
    }
}

// FUNCTION: 0x00017854
void fn_00017854(Entity *w)
{
    if (w->unknown103 == 0) {
        if (fn_0003791c(((u16)w->y.tile << 7) + (u16)w->x.tile, 0x33, 1, 150, w->owner)) {
            w->state_b = 0x36;
            w->state_a = 0x38;
            w->unknown103 = 1;
            fn_000417ac(w, 330, 0);
            fn_00040524(w);
        } else {
            w->kind = 0x38;
        }
    } else if (w->unknown103 == 1) {
        w->unknown124 = 0;
        if (w->style >= 0x10 && fn_0003791c(((u16)w->y.tile << 7) + (u16)w->x.tile, 0x20, 1, 150, w->owner)) {
            w->state_b = 0x38;
            w->state_a = 0x38;
            fn_000417ac(w, 350, 0);
            fn_00040524(w);
        } else {
            w->kind = 0x38;
        }
    } else {
        w->kind = 0x38;
    }
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x000179a8
void fn_000179a8(Entity *w)
{
    w->state_b = 0x26;
    w->state_a = 0x38;
    fn_000417ac(w, 90, w->room->unknown6);
    fn_00040524(w);
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x000179f8
void fn_000179f8(Entity *w)
{
    u16 id;
    u16 arg;
    Room *room;
    Entity *other;

    arg = 0;
    if (w->unknown140) {
        room = &D_001547a4[w->unknown140];
        w->state_a = 0x18;
        w->state_b = 0x18;
        if (room->type == 4) {
            id = fn_00037bec(11, 0, 0, w->owner);
            if (id) {
                other = &D_001549a4[id];
                arg = other->room_next;
            }
        } else {
            arg = room->unknown4;
        }
        if (arg) {
            fn_000417ac(w, 30, arg);
            fn_00040524(w);
            return;
        }
    }
    w->kind = 0x18;
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x00017ae0
void fn_00017ae0(Entity *w)
{
    u16 next;
    AnimFrame *frame;

    if (D_001549c4 % w->anim_speed == 0) {
        next = D_00153f64[w->sprite].next;
        frame = &D_00153f64[next];
        if (frame->flags & 1) {
            w->kind = w->unknown103;
            w->sprite = w->unknown85;
            w->palette = w->unknown104;
            w->unknown85 = w->unknown104 = w->unknown103 = 0;
            if (w->flags & 0x4) {
                fn_0007a710(((u16)w->y.tile << 7) + (u16)w->x.tile, (s8)w->owner, 5);
                w->flags &= ~0x4;
            } else if (w->flags & 0x8) {
                fn_0007a710(((u16)w->y.tile << 7) + (u16)w->x.tile, (s8)w->owner, 6);
                w->flags &= ~0x8;
            }
        } else {
            w->sprite = next;
        }
    }
}

// FUNCTION: 0x00017c28
void fn_00017c28(Entity *w, Room *room)
{
    s8 count;

    if (room->type == 10) {
        count = 2;
    } else {
        count = 1;
    }
    fn_00049820(D_0012d900[w->unknown171].unknown0, room - D_001547a4, 1, count, w->owner);
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x00017cb0
void fn_00017cb0(Entity *w)
{
    if (D_001549c4 % w->anim_speed == 0) {
        w->sprite = D_00153f64[w->sprite].next;
    }
    if (w->height < 400) {
        w->height += w->unknown18;
    } else {
        fn_00016760(w);
    }
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x00017d2c
void fn_00017d2c(Entity *w)
{
    w->state_b = 0x26;
    if (fn_000417ac(w, 90, w->room->unknown6)) {
        fn_00040524(w);
    }
}

// FUNCTION: 0x00017d78
void fn_00017d78(Entity *w, u8 owner)
{
    u8 i;

    if (!(w->flags & 0x40)) {
        fn_000194f0(w);
        fn_0004b058(w);
        w->owner = owner;
        w->unknown118 |= 1;
        if (w->flags & 0x20) {
            fn_000b05f0(w);
            w->unknown174 = 100;
            w->flags &= ~0x20;
            fn_00098950(w);
        }
        w->unknown169 = 1;
        w->unknown171 = 1;
        for (i = 0; i < 10; i++) {
            w->unknown148[i] = 0;
        }
        fn_000173c0(w);
    }
}

// NONMATCHING: register allocation (target evaluates pointer/operand loads directly into the destination register)
// FUNCTION: 0x00017e30
void fn_00017e30(Entity *w)
{
    u8 style;
    u8 category;
    u8 done;
    AnimFrame *frame;

    if (D_001549c4 % w->anim_speed == 0) {
        w->sprite = D_00153f64[w->sprite].next;
        frame = &D_00153f64[w->sprite];
        if (frame->flags & 1) {
            w->height--;
            if (w->height <= 0) {
                u16 anim;

                if (w->flags & 0x400) {
                    done = fn_0003e468(w);
                    if (done || (u16)(D_0015543a - w->timer) >= 1500) {
                        w->flags &= ~0x10;
                        fn_000b0428(w, 0x1b);
                        if (!(w->flags & 0x80)) {
                            fn_00018710(8, w->unknown91, w->owner);
                            w->unknown91 = 0;
                        }
                        if (done) {
                            w->kind = 0x26;
                        } else {
                            fn_0001665c(w);
                            D_001547b4[w->owner].rooms[w->style].unknown24++;
                            w->unknown118 |= 8;
                        }
                        w->flags &= 0xfb7f;
                        return;
                    }
                } else {
                    if (w->unknown172[0]) {
                        style = w->unknown172[0];
                    } else {
                        style = w->unknown171;
                    }
                    category = D_0012d900[style].category;
                    done = D_001547b4[w->owner].rooms[style].flag == 1 && D_00154820[w->owner].unknown9c[category]
                        ? (u8)1 : (u8)0;
                    if (done || (u16)(D_0015543a - w->timer) >= 1500) {
                        w->flags &= ~0x10;
                        fn_000b0428(w, 0x1b);
                        if (w->flags & 0x80) {
                            w->flags &= ~0x80;
                            if (!done) {
                                fn_0001665c(w);
                                D_001547b4[w->owner].rooms[w->style].unknown24++;
                                w->unknown118 |= 0x10;
                            } else {
                                fn_00018710(4, w->unknown91, w->owner);
                                w->kind = 0x26;
                                w->unknown91 = 0;
                            }
                        } else {
                            fn_0001665c(w);
                            w->unknown118 |= 0x10;
                        }
                        return;
                    }
                }
                if (D_0013f71c[D_00153f78[((s16)w->y.tile << 7) + (s16)w->x.tile].overlay].width == 0x20
                    && fn_00047ed0(w)) {
                    return;
                }
                anim = fn_0008dcd0(D_00136422[w->subtype].idle, w) + (u16)w->unknown17;
                w->sprite = D_00154894[anim].frame;
                w->palette = D_00154894[anim].flags > 0 ? (u8)1 : (u8)0;
                if (fn_000164f4(w)) {
                    return;
                }
                w->height = w->unknown18 == 0 ? fn_0001974c() : 0x100 / w->unknown18;
            }
        }
    }
}

}

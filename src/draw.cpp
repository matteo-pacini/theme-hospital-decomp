#include "hosp.h"

struct ElementBits {
    u32 unknown0 : 4;
    u32 layer : 5;
    u32 id : 7;
};

struct IconScreen {
    u8 unknown0[0x4fc];
    s32 unknown4fc;
    u8 unknown500[0x60];
    s32 unknown560;
    s32 unknown564;
    s32 unknown568;
};

struct FrameBox {
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
    s16 x;
    s16 y;
    s16 w;
    s16 h;
};

extern "C" {

extern FrameBox D_001567b0;
extern u16 D_00188c74;
extern u8 D_0015536d;
extern TbScreenMode D_0015f3f4;
extern s32 D_00156911;
extern u8 D_00155e0a;
extern u8 D_00188c85;
extern TbScreenMode D_0015f3f5;
extern u8 D_00155e45;
extern TbSprite *D_00153f5c;
extern char D_001202fc[];
extern IconScreen *D_0015f410;
extern s32 D_00188c40;
extern s32 D_00188c48;
extern u8 D_00130280[];

void LbDrawHVLine(s32 x, s32 y, TbSprite *spr, u8 colour);
void LbSpriteDrawScaled(s32 x, s32 y, TbSprite *spr, s32 w, s32 h);
void fn_000cbe20(s32 x, s32 y, TbSprite *spr, u8 *remap);

u32 ror13(u32 x);
#pragma aux ror13 = "ror eax,13" parm [eax] value [eax];

}

// FUNCTION: 0x0002d910
void draw_a_frame(const u16 anim, const s16 x, const s16 y, const u8 flags, const u8 flip)
{
    CompFrame *f;
    u16 *el;
    CompElement *elem;
    TbSprite *spr;

    f = &frame[anim];
    D_001567b0.x = D_001567b0.y = 999;
    if (flip) {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            spr = (TbSprite *)((u8 *)sprite + elem->sprite);
            if (spr > sprite) {
                D_00188c74 = elem->flags | flags;
                D_00188c74 ^= 1;
                LbSpriteDraw(x - (elem->x - 0x8d) - spr->width, y + (elem->y - 0xca), spr);
                if (D_001567b0.x > -(spr->width + (elem->x - 0x8d)))
                    D_001567b0.x = -(elem->x - 0x8d + spr->width);
                if (D_001567b0.y > elem->y - 0xca)
                    D_001567b0.y = elem->y - 0xca;
            }
        }
    } else {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            spr = (TbSprite *)((u8 *)sprite + elem->sprite);
            if (spr > sprite) {
                D_00188c74 = elem->flags | flags;
                LbSpriteDraw(elem->x - 0x8d + x, y + (elem->y - 0xca), spr);
                if (D_001567b0.x > elem->x - 0x8d)
                    D_001567b0.x = elem->x - 0x8d;
                if (D_001567b0.y > elem->y - 0xca)
                    D_001567b0.y = elem->y - 0xca;
            }
        }
    }
    if (f->sound && D_0015536d == 0)
        play_sound(10, f->sound, x, y);
    D_001567b0.x += x;
    D_001567b0.y += y;
    D_001567b0.w = f->width;
    if (D_0015f3f4 == SCREEN_MODE_13) {
        D_001567b0.h = f->length + D_00156911;
    } else {
        D_001567b0.w /= 2;
        D_001567b0.h = f->length / 2 + D_00156911;
    }
    if (D_001567b0.x0 > D_001567b0.x)
        D_001567b0.x0 = D_001567b0.x;
    if (D_001567b0.y0 > D_001567b0.y)
        D_001567b0.y0 = D_001567b0.y;
    if (D_001567b0.x1 < D_001567b0.x + D_001567b0.w)
        D_001567b0.x1 = D_001567b0.x + D_001567b0.w;
    if (D_001567b0.y1 < D_001567b0.y + D_001567b0.h)
        D_001567b0.y1 = D_001567b0.y + D_001567b0.h;
    D_00188c74 = 0;
}

// NONMATCHING: register choice
// FUNCTION: 0x0002dd1c
void draw_a_one_colour_frame(const u16 anim, const s16 x, const s16 y, const u16 colour, const u8 flip)
{
    CompFrame *f;
    u16 *el;
    CompElement *elem;
    TbSprite *spr;

    f = &frame[anim];
    D_001567b0.x = D_001567b0.y = 999;
    if (flip) {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            spr = (TbSprite *)((u8 *)sprite + elem->sprite);
            if (spr > sprite) {
                D_00188c74 = elem->flags;
                D_00188c74 ^= 1;
                LbDrawHVLine(x - (elem->x - 0x8d) - spr->width, y + (elem->y - 0xca), spr, colour);
                if (D_001567b0.x > -(spr->width + (elem->x - 0x8d)))
                    D_001567b0.x = -(elem->x - 0x8d + spr->width);
                if (D_001567b0.y > elem->y - 0xca)
                    D_001567b0.y = elem->y - 0xca;
            }
        }
    } else {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            spr = (TbSprite *)((u8 *)sprite + elem->sprite);
            if (spr > sprite) {
                D_00188c74 = elem->flags;
                LbDrawHVLine(elem->x - 0x8d + x, elem->y - 0xca + y, spr, colour);
                if (D_001567b0.x > elem->x - 0x8d)
                    D_001567b0.x = elem->x - 0x8d;
                if (D_001567b0.y > elem->y - 0xca)
                    D_001567b0.y = elem->y - 0xca;
            }
        }
    }
    D_001567b0.x += x;
    D_001567b0.y += y;
    D_001567b0.w = f->width;
    if (D_0015f3f4 == SCREEN_MODE_13) {
        D_001567b0.h = f->length + D_00156911;
    } else {
        D_001567b0.w /= 2;
        D_001567b0.h = f->length / 2 + D_00156911;
    }
    if (D_001567b0.x0 > D_001567b0.x)
        D_001567b0.x0 = D_001567b0.x;
    if (D_001567b0.y0 > D_001567b0.y)
        D_001567b0.y0 = D_001567b0.y;
    if (D_001567b0.x1 < D_001567b0.x + D_001567b0.w)
        D_001567b0.x1 = D_001567b0.x + D_001567b0.w;
    if (D_001567b0.y1 < D_001567b0.y + D_001567b0.h)
        D_001567b0.y1 = D_001567b0.y + D_001567b0.h;
    D_00188c74 = 0;
}

// NONMATCHING: register choice; icon_draw_from_special_id_frame arguments are narrowed differently
// FUNCTION: 0x0002e0ec
void draw_special_id_frame(Thing *w, const s16 x, const s16 y)
{
    CompFrame *f;
    u8 icon;
    u16 *el;
    CompElement *elem;
    s32 kind;
    TbSprite *spr;
    ElementBits *bits;

    f = &frame[w->sprite];
    icon = 0;
    D_001567b0.x = D_001567b0.y = 999;
    if (w->palette) {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            if (!D_00155e0a) {
                kind = elem->flags & 0xfff0;
                if (w->unknown24 > 0) {
                    if ((kind == 0x200 || kind == 0x250) && icon_table[w->unknown24].unknown1 > 0) {
                        spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                        if (spr > sprite)
                            icon_draw_from_special_id_frame(w->unknown27, x, y, (s16)-(spr->width / 2 + (elem->x - 0x8d)), (s16)(elem->y - 0xca));
                    } else if (w->renderer == 6) {
                        icon = 1;
                        D_00155e0a = 1;
                    }
                } else {
                    D_00155e0a = 1;
                }
            }
            if (elem->flags & 0xffb0) {
                bits = (ElementBits *)&elem->flags;
                if (w->layers[bits->layer] != bits->id)
                    continue;
            }
            spr = (TbSprite *)((u8 *)sprite + elem->sprite);
            if (spr > sprite) {
                D_00188c74 = elem->flags & 0xf;
                D_00188c74 ^= 1;
                LbSpriteDraw(x - (elem->x - 0x8d) - spr->width, y + (elem->y - 0xca), spr);
                if (D_001567b0.x > -(spr->width + (elem->x - 0x8d)))
                    D_001567b0.x = -(elem->x - 0x8d + spr->width);
                if (D_001567b0.y > elem->y - 0xca)
                    D_001567b0.y = elem->y - 0xca;
            }
        }
    } else {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            if (!D_00155e0a) {
                kind = elem->flags & 0xfff0;
                if (w->unknown24 > 0) {
                    if ((kind == 0x200 || kind == 0x250) && icon_table[w->unknown24].unknown1 > 0) {
                        spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                        if (spr > sprite)
                            icon_draw_from_special_id_frame(w->unknown27, x, y, (s16)(spr->width / 2 + (elem->x - 0x8d)), (s16)(elem->y - 0xca));
                    } else if (w->renderer == 6) {
                        icon = 1;
                        D_00155e0a = 1;
                    }
                } else {
                    D_00155e0a = 1;
                }
            }
            if (elem->flags & 0xffb0) {
                bits = (ElementBits *)&elem->flags;
                if (w->layers[bits->layer] != bits->id)
                    continue;
            }
            spr = (TbSprite *)((u8 *)sprite + elem->sprite);
            if (spr > sprite) {
                D_00188c74 = elem->flags & 0xf;
                LbSpriteDraw(elem->x - 0x8d + x, elem->y - 0xca + y, spr);
                if (D_001567b0.x > elem->x - 0x8d)
                    D_001567b0.x = elem->x - 0x8d;
                if (D_001567b0.y > elem->y - 0xca)
                    D_001567b0.y = elem->y - 0xca;
            }
        }
    }
    if (f->sound && D_0015536d == 0 && (sfx_details[f->sound].unknownC || (int)w->unknown23 != (int)f->sound))
        play_sound((u32)w, f->sound, x, y);
    if (D_0015536d == 0)
        w->unknown23 = f->sound;
    if (icon)
        icon_draw_from_special_id_frame(w->unknown27, x, y, f->width / 2 + D_001567b0.x, D_001567b0.y);
    D_001567b0.x += x;
    D_001567b0.y += y;
    D_001567b0.w = f->width;
    if (D_0015f3f4 == SCREEN_MODE_13) {
        D_001567b0.h = f->length + D_00156911;
    } else {
        D_001567b0.w /= 2;
        D_001567b0.h = f->length / 2 + D_00156911;
    }
    D_00156911 = 0;
    if (D_001567b0.x0 > D_001567b0.x)
        D_001567b0.x0 = D_001567b0.x;
    if (D_001567b0.y0 > D_001567b0.y)
        D_001567b0.y0 = D_001567b0.y;
    if (D_001567b0.x1 < D_001567b0.x + D_001567b0.w)
        D_001567b0.x1 = D_001567b0.x + D_001567b0.w;
    if (D_001567b0.y1 < D_001567b0.y + D_001567b0.h)
        D_001567b0.y1 = D_001567b0.y + D_001567b0.h;
    D_00188c74 = 0;
    D_00155e0a = 0;
}

extern "C" {

// NONMATCHING: register choice
// FUNCTION: 0x0002e804
void fn_0002e804(Thing *w, s16 x, s16 y)
{
    u16 drawn;
    u8 icon;
    CompFrame *f;
    u32 phase;
    u16 *el;
    CompElement *elem;
    s32 kind;
    TbSprite *spr;
    s16 ix;
    s16 iy;
    ElementBits *bits;

    f = &frame[w->sprite];
    phase = actual_game_turn % 17;
    drawn = 0;
    icon = 0;
    D_001567b0.x = D_001567b0.y = 999;
    if (w->palette) {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            if (!D_00155e0a) {
                kind = elem->flags & 0xfff0;
                if (w->unknown24 > 0) {
                    if ((kind == 0x200 || kind == 0x250) && icon_table[w->unknown24].unknown1 > 0) {
                        spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                        if (spr > sprite) {
                            icon = 1;
                            ix = -(elem->x - 0x8d) - spr->width / 2;
                            iy = elem->y - 0xca;
                        }
                    }
                } else {
                    D_00155e0a = 1;
                }
            }
            if (elem->flags & 0xfff0) {
                if (elem->flags & 0xffb0) {
                    bits = (ElementBits *)&elem->flags;
                    if (w->layers[bits->layer] != bits->id)
                        continue;
                }
                DrawIntoPixBuffer();
                spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                if (spr > sprite) {
                    D_00188c74 = elem->flags & 0xf;
                    D_00188c74 ^= 1;
                    LbSpriteDraw(0x3a - (elem->x - 0x8d) - spr->width, elem->y - 0xca + 0x74, spr);
                    drawn = 1;
                    if (D_001567b0.x > -(spr->width + (elem->x - 0x8d)))
                        D_001567b0.x = -(elem->x - 0x8d + spr->width);
                    if (D_001567b0.y > elem->y - 0xca)
                        D_001567b0.y = elem->y - 0xca;
                }
                RestoreFromPixBuffer();
            } else {
                if (drawn) {
                    drawn = 0;
                    draw_wibble(x + D_001567b0.x - 4, y + D_001567b0.y - 4,
                                pixBuf.data + 0x3a + D_001567b0.x - 4 + (D_001567b0.y + 0x70) * 0xb4,
                                f->width + 8 < 0x7a - D_001567b0.x + 3 ? f->width + 8 : 0x7a - D_001567b0.x + 3,
                                f->length + 8 < 0x40 - D_001567b0.y + 3 ? f->length + 8 : 0x40 - D_001567b0.y + 3,
                                phase);
                    FillBoxInPixBuffer(0xfe, D_001567b0.x + 0x3a, D_001567b0.y + 0x74, f->width, f->length);
                }
                spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                if (spr > sprite) {
                    D_00188c74 = elem->flags & 0xf;
                    D_00188c74 ^= 1;
                    LbSpriteDraw(x - (elem->x - 0x8d) - spr->width, y + (elem->y - 0xca), spr);
                    if (D_001567b0.x > -(spr->width + (elem->x - 0x8d)))
                        D_001567b0.x = -(elem->x - 0x8d + spr->width);
                    if (D_001567b0.y > elem->y - 0xca)
                        D_001567b0.y = elem->y - 0xca;
                }
            }
        }
    } else {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            if (!D_00155e0a) {
                kind = elem->flags & 0xfff0;
                if (w->unknown24 > 0) {
                    if ((kind == 0x200 || kind == 0x250) && icon_table[w->unknown24].unknown1 > 0) {
                        spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                        if (spr > sprite) {
                            icon = 1;
                            ix = spr->width / 2 + (elem->x - 0x8d);
                            iy = elem->y - 0xca;
                        }
                    }
                } else {
                    D_00155e0a = 1;
                }
            }
            if (elem->flags & 0xfff0) {
                if (elem->flags & 0xffb0) {
                    bits = (ElementBits *)&elem->flags;
                    if (w->layers[bits->layer] != bits->id)
                        continue;
                }
                DrawIntoPixBuffer();
                spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                if (spr > sprite) {
                    D_00188c74 = elem->flags & 0xf;
                    LbSpriteDraw(elem->x - 0x8d + 0x3a, elem->y - 0xca + 0x74, spr);
                    drawn = 1;
                    if (D_001567b0.x > elem->x - 0x8d)
                        D_001567b0.x = elem->x - 0x8d;
                    if (D_001567b0.y > elem->y - 0xca)
                        D_001567b0.y = elem->y - 0xca;
                }
                RestoreFromPixBuffer();
            } else {
                if (drawn) {
                    drawn = 0;
                    draw_wibble(x + D_001567b0.x - 4, y + D_001567b0.y - 4,
                                pixBuf.data + 0x3a + D_001567b0.x - 4 + (D_001567b0.y + 0x70) * 0xb4,
                                f->width + 8 < 0x7a - D_001567b0.x + 3 ? f->width + 8 : 0x7a - D_001567b0.x + 3,
                                f->length + 8 < 0x40 - D_001567b0.y + 3 ? f->length + 8 : 0x40 - D_001567b0.y + 3,
                                phase);
                    FillBoxInPixBuffer(0xfe, D_001567b0.x + 0x3a, D_001567b0.y + 0x74, f->width, f->length);
                }
                spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                if (spr > sprite) {
                    D_00188c74 = elem->flags & 0xf;
                    LbSpriteDraw(elem->x - 0x8d + x, y + (elem->y - 0xca), spr);
                    if (D_001567b0.x > elem->x - 0x8d)
                        D_001567b0.x = elem->x - 0x8d;
                    if (D_001567b0.y > elem->y - 0xca)
                        D_001567b0.y = elem->y - 0xca;
                }
            }
        }
    }
    if (drawn) {
        draw_wibble(x + D_001567b0.x - 4, y + D_001567b0.y - 4,
                    pixBuf.data + 0x3a + D_001567b0.x - 4 + (D_001567b0.y + 0x70) * 0xb4,
                    f->width + 8 < 0x7a - D_001567b0.x + 3 ? f->width + 8 : 0x7a - D_001567b0.x + 3,
                    f->length + 8 < 0x40 - D_001567b0.y + 3 ? f->length + 8 : 0x40 - D_001567b0.y + 3,
                    phase);
    }
    FillBoxInPixBuffer(0xfe, D_001567b0.x + 0x3a, D_001567b0.y + 0x74, f->width, f->length);
    if (f->sound && D_0015536d == 0 && (sfx_details[f->sound].unknownC || (int)w->unknown23 != (int)f->sound))
        play_sound((u32)w, f->sound, x, y);
    if (D_0015536d == 0)
        w->unknown23 = f->sound;
    D_001567b0.x += x;
    D_001567b0.y += y;
    D_001567b0.w = f->width;
    if (D_0015f3f4 == SCREEN_MODE_13) {
        D_001567b0.h = f->length + D_00156911;
    } else {
        D_001567b0.w /= 2;
        D_001567b0.h = f->length / 2 + D_00156911;
    }
    D_00156911 = 0;
    D_001567b0.x -= 4;
    D_001567b0.y -= 4;
    D_001567b0.w += 8;
    D_001567b0.h += 8;
    if (D_001567b0.x0 > D_001567b0.x)
        D_001567b0.x0 = D_001567b0.x;
    if (D_001567b0.y0 > D_001567b0.y)
        D_001567b0.y0 = D_001567b0.y;
    if (D_001567b0.x1 < D_001567b0.x + D_001567b0.w)
        D_001567b0.x1 = D_001567b0.x + D_001567b0.w;
    if (D_001567b0.y1 < D_001567b0.y + D_001567b0.h)
        D_001567b0.y1 = D_001567b0.y + D_001567b0.h;
    D_00188c74 = 0;
    D_00155e0a = 0;
}

// NONMATCHING: register choice
// FUNCTION: 0x0002f47c
void fn_0002f47c(Thing *w, s16 x, s16 y)
{
    u8 g;
    u8 glow;
    CompFrame *f;
    u16 *el;
    CompElement *elem;
    s32 kind;
    TbSprite *spr;
    ElementBits *bits;

    f = &frame[w->sprite];
    glow = D_00130280[((actual_game_turn + w->unknown29 + w->unknown30[0]) >> 1) & 7];
    D_001567b0.x = D_001567b0.y = 999;
    for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
        elem = &element[*el];
        if (elem->flags & 0xfff0) {
            if (!D_00155e0a) {
                kind = elem->flags & 0xfff0;
                if (w->unknown24 > 0) {
                    if ((kind == 0x200 || kind == 0x250) && icon_table[w->unknown24].unknown1 > 0) {
                        spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                        if (spr > sprite) {
                            if (w->palette)
                                icon_draw_from_special_id_frame(w->unknown27, x, y, -(spr->width / 2 + (elem->x - 0x8d)), (s16)(elem->y - 0xca));
                            else
                                icon_draw_from_special_id_frame(w->unknown27, x, y, spr->width / 2 + (elem->x - 0x8d), (s16)(elem->y - 0xca));
                        }
                    }
                } else {
                    D_00155e0a = 1;
                }
            }
            if (elem->flags & 0xffb0) {
                bits = (ElementBits *)&elem->flags;
                if (w->layers[bits->layer] != bits->id)
                    continue;
            }
            g = glow;
        } else {
            g = 0;
        }
        spr = (TbSprite *)((u8 *)sprite + elem->sprite);
        if (spr > sprite) {
            D_00188c74 = elem->flags & 0xf;
            if (g)
                D_00188c74 |= 4;
            if (w->palette) {
                D_00188c74 ^= 1;
                sprite_draw_glow(x - (elem->x - 0x8d) - spr->width, y + (elem->y - 0xca), spr, g);
                if (D_001567b0.x > -(spr->width + (elem->x - 0x8d)))
                    D_001567b0.x = -(elem->x - 0x8d + spr->width);
                if (D_001567b0.y > elem->y - 0xca)
                    D_001567b0.y = elem->y - 0xca;
            } else {
                sprite_draw_glow(elem->x - 0x8d + x, y + (elem->y - 0xca), spr, g);
                if (D_001567b0.x > elem->x - 0x8d)
                    D_001567b0.x = elem->x - 0x8d;
                if (D_001567b0.y > elem->y - 0xca)
                    D_001567b0.y = elem->y - 0xca;
            }
        }
    }
    if (f->sound && D_0015536d == 0 && (sfx_details[f->sound].unknownC || (int)w->unknown23 != (int)f->sound))
        play_sound((u32)w, f->sound, x, y);
    if (D_0015536d == 0)
        w->unknown23 = f->sound;
    D_001567b0.x += x;
    D_001567b0.y += y;
    D_001567b0.w = f->width;
    if (D_0015f3f4 == SCREEN_MODE_13) {
        D_001567b0.h = f->length + D_00156911;
    } else {
        D_001567b0.w /= 2;
        D_001567b0.h = f->length / 2 + D_00156911;
    }
    D_00156911 = 0;
    if (D_001567b0.x0 > D_001567b0.x)
        D_001567b0.x0 = D_001567b0.x;
    if (D_001567b0.y0 > D_001567b0.y)
        D_001567b0.y0 = D_001567b0.y;
    if (D_001567b0.x1 < D_001567b0.x + D_001567b0.w)
        D_001567b0.x1 = D_001567b0.x + D_001567b0.w;
    if (D_001567b0.y1 < D_001567b0.y + D_001567b0.h)
        D_001567b0.y1 = D_001567b0.y + D_001567b0.h;
    D_00188c74 = 0;
    D_00155e0a = 0;
}

}

// FUNCTION: 0x0002fa3c
void draw_split_frame(Thing *w, const s16 x, const s16 y)
{
    s32 old_top;
    s32 old_height;
    Thing *o;
    CompFrame *f;
    u8 h;
    u8 rest;

    old_top = D_00188c40;
    old_height = D_00188c48;
    o = &thing[w->unknown69];
    if (o > thing) {
        if (w->subtype == 0xe)
            draw_special_id_frame(o, x, y);
        f = &frame[w->sprite];
        h = o->unknown103;
        rest = f->length - h;
        D_001567b0.x = D_001567b0.y = 999;
        {
            s32 top = y - h;

            D_00188c48 = top - D_00188c40 > D_00188c48 ? D_00188c48 : top - D_00188c40;
        }
        full_draw_frame(w, w->sprite, x, y, 0, 0, w->palette);
        {
            s32 top = y - h;

            D_00188c40 = top > old_top ? top : old_top;
        }
        D_00188c48 = old_height + old_top - D_00188c40;
        full_draw_frame(w, o->unknown100, x, y - D_00188c40, 0, 0, o->state_b);
        D_00188c40 = old_top;
        D_00188c48 = old_height;
        D_00188c74 = 0;
        if (f->sound && D_0015536d == 0 && (sfx_details[f->sound].unknownC || w->unknown23 != f->sound))
            play_sound((u32)w, f->sound, x, y);
        if (D_0015536d == 0)
            w->unknown23 = f->sound;
        D_001567b0.x += x;
        D_001567b0.y += y;
        D_001567b0.w = f->width;
        if (D_0015f3f4 == SCREEN_MODE_13) {
            D_001567b0.h = f->length + D_00156911;
        } else {
            D_001567b0.w /= 2;
            D_001567b0.h = f->length / 2 + D_00156911;
        }
        if (D_001567b0.x0 > D_001567b0.x)
            D_001567b0.x0 = D_001567b0.x;
        if (D_001567b0.y0 > D_001567b0.y)
            D_001567b0.y0 = D_001567b0.y;
        if (D_001567b0.x1 < D_001567b0.x + D_001567b0.w)
            D_001567b0.x1 = D_001567b0.x + D_001567b0.w;
        if (D_001567b0.y1 < D_001567b0.y + D_001567b0.h)
            D_001567b0.y1 = D_001567b0.y + D_001567b0.h;
        if (w->subtype == 0xe) {
            s32 anim;

            if (w->unknown17 == 0)
                anim = 0x787;
            else
                anim = 0x784;
            anim += actual_game_turn % 3;
            draw_a_frame(start[anim].frame, x, y - h, 0, start[anim].flags > 0);
        }
    }
}


// FUNCTION: 0x0002fe18
void draw_energise_frame(Thing *w, const s16 x, const s16 y)
{
    CompFrame *f;
    u32 r;
    Thing *o;
    u16 i;

    o = &thing[w->unknown69];
    r = 0x546b1;
    if (o > thing) {
        f = &frame[w->sprite];
        D_001567b0.x = D_001567b0.y = 999;
        if (o->unknown105 != 0xff)
            full_draw_frame(w, w->sprite, x, y, o->unknown105, 0, w->palette);
        InitPixBuffer(0, 0, 0x40, 0x40, 0, 0);
        DrawIntoPixBuffer();
        full_draw_frame(w, w->sprite, 0x20, 0x3a, 0, 0, w->palette);
        RestoreFromPixBuffer();
        for (i = o->unknown103 * 10; i > 0; i--) {
            pixBuf.data[r & 0xfff] = 0xfe;
            pixBuf.data[(r >> 10) & 0xfff] = 0xfe;
            pixBuf.data[(r >> 20) & 0xfff] = 0xfe;
            r = r * 0x24a1 + 0x24df;
            r = ror13(r);
        }
        pixBuf.x = x - 0x20;
        pixBuf.y = y - 0x3a;
        DrawPixBuffer(o->unknown104, 0, 0, 0x40, 0x40);
        FillPixBuffer(0xfe);
        InitPixBuffer(0, 0, 0xb4, 0xb4, 0, 0);
        if (f->sound && D_0015536d == 0 && (sfx_details[f->sound].unknownC || w->unknown23 != f->sound))
            play_sound((u32)w, f->sound, x, y);
        if (D_0015536d == 0)
            w->unknown23 = f->sound;
        D_001567b0.x += x;
        D_001567b0.y += y;
        D_001567b0.w = f->width;
        if (D_0015f3f4 == SCREEN_MODE_13) {
            D_001567b0.h = f->length + D_00156911;
        } else {
            D_001567b0.w /= 2;
            D_001567b0.h = f->length / 2 + D_00156911;
        }
        if (D_001567b0.x0 > D_001567b0.x)
            D_001567b0.x0 = D_001567b0.x;
        if (D_001567b0.y0 > D_001567b0.y)
            D_001567b0.y0 = D_001567b0.y;
        if (D_001567b0.x1 < D_001567b0.x + D_001567b0.w)
            D_001567b0.x1 = D_001567b0.x + D_001567b0.w;
        if (D_001567b0.y1 < D_001567b0.y + D_001567b0.h)
            D_001567b0.y1 = D_001567b0.y + D_001567b0.h;
        D_00188c74 = 0;
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x00030198
void draw_a_frame_scaled(const u16 anim, const s16 x, const s16 y, u8 flags, const u8 flip, s32 scale)
{
    CompFrame *f;
    u16 *el;
    CompElement *elem;
    TbSprite *spr;

    f = &frame[anim];
    D_001567b0.x = D_001567b0.y = 999;
    if (flip) {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            spr = (TbSprite *)((u8 *)sprite + elem->sprite);
            if (spr > sprite) {
                D_00188c74 = elem->flags | flags;
                D_00188c74 ^= 1;
                LbSpriteDrawScaled(x - ((elem->x - 0x8d + spr->width) * scale >> 16), y + ((elem->y - 0xca) * scale >> 16),
                                   spr, spr->width * scale >> 16, spr->height * scale >> 16);
                if (D_001567b0.x > -(spr->width + (elem->x - 0x8d)))
                    D_001567b0.x = -(elem->x - 0x8d + spr->width);
                if (D_001567b0.y > elem->y - 0xca)
                    D_001567b0.y = elem->y - 0xca;
            }
        }
    } else {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            spr = (TbSprite *)((u8 *)sprite + elem->sprite);
            if (spr > sprite) {
                D_00188c74 = elem->flags | flags;
                LbSpriteDrawScaled(((elem->x - 0x8d) * scale >> 16) + x, y + ((elem->y - 0xca) * scale >> 16),
                                   spr, spr->width * scale >> 16, spr->height * scale >> 16);
                if (D_001567b0.x > elem->x - 0x8d)
                    D_001567b0.x = elem->x - 0x8d;
                if (D_001567b0.y > elem->y - 0xca)
                    D_001567b0.y = elem->y - 0xca;
            }
        }
    }
    if (f->sound && D_0015536d == 0)
        play_sound(10, f->sound, x, y);
    D_001567b0.x += x;
    D_001567b0.y += y;
    D_001567b0.w = f->width;
    if (D_0015f3f4 == SCREEN_MODE_13) {
        D_001567b0.h = f->length + D_00156911;
    } else {
        D_001567b0.w /= 2;
        D_001567b0.h = f->length / 2 + D_00156911;
    }
    if (D_001567b0.x0 > D_001567b0.x)
        D_001567b0.x0 = D_001567b0.x;
    if (D_001567b0.y0 > D_001567b0.y)
        D_001567b0.y0 = D_001567b0.y;
    if (D_001567b0.x1 < D_001567b0.x + D_001567b0.w)
        D_001567b0.x1 = D_001567b0.x + D_001567b0.w;
    if (D_001567b0.y1 < D_001567b0.y + D_001567b0.h)
        D_001567b0.y1 = D_001567b0.y + D_001567b0.h;
    D_00188c74 = 0;
}


// FUNCTION: 0x000305fc
void draw_hilight_frame(Thing *w, const s16 x, const s16 y)
{
    u8 phase;
    u8 hidden;
    CompFrame *f;
    s32 colour;

    f = &frame[w->sprite];
    hidden = 0;
    D_001567b0.x = D_001567b0.y = 999;
    phase = game_turn % 6;
    if (phase >= 4)
        phase = 6 - phase;
    switch (phase) {
    case 0:
        colour = 0;
        break;
    case 1:
        colour = 4;
        break;
    case 2:
        colour = 8;
        break;
    case 3:
        colour = 0;
        hidden = 1;
        break;
    }
    full_draw_frame(w, w->sprite, x, y, colour, hidden, w->palette);
    if (f->sound && D_0015536d == 0 && (sfx_details[f->sound].unknownC || (int)w->unknown23 != (int)f->sound))
        play_sound((u32)w, f->sound, x, y);
    if (D_0015536d == 0)
        w->unknown23 = f->sound;
    D_001567b0.x += x;
    D_001567b0.y += y;
    D_001567b0.w = f->width;
    if (D_0015f3f4 == SCREEN_MODE_13) {
        D_001567b0.h = f->length + D_00156911;
    } else {
        D_001567b0.w /= 2;
        D_001567b0.h = f->length / 2 + D_00156911;
    }
    if (D_001567b0.x0 > D_001567b0.x)
        D_001567b0.x0 = D_001567b0.x;
    if (D_001567b0.y0 > D_001567b0.y)
        D_001567b0.y0 = D_001567b0.y;
    if (D_001567b0.x1 < D_001567b0.x + D_001567b0.w)
        D_001567b0.x1 = D_001567b0.x + D_001567b0.w;
    if (D_001567b0.y1 < D_001567b0.y + D_001567b0.h)
        D_001567b0.y1 = D_001567b0.y + D_001567b0.h;
    D_00188c74 = 0;
}


// FUNCTION: 0x000308a0
void draw_from_frame(const u16 anim, const s16 x, const s16 y, u8 *remap, const u8 flip)
{
    CompFrame *f;
    u16 *el;
    CompElement *elem;
    TbSprite *spr;

    f = &frame[anim];
    D_001567b0.x0 = D_001567b0.y0 = 999;
    D_001567b0.x1 = D_001567b0.y1 = 0;
    D_001567b0.x = D_001567b0.y = 999;
    if (flip) {
        if (remap) {
            for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
                elem = &element[*el];
                spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                if (spr > sprite) {
                    D_00188c74 = elem->flags;
                    D_00188c74 ^= 1;
                    fn_000cbe20(x - (elem->x - 0x8d) - spr->width, y + (elem->y - 0xca), spr, remap);
                    if (D_001567b0.x > -(spr->width + (elem->x - 0x8d)))
                        D_001567b0.x = -(elem->x - 0x8d + spr->width);
                    if (D_001567b0.y > elem->y - 0xca)
                        D_001567b0.y = elem->y - 0xca;
                }
            }
        } else {
            for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
                elem = &element[*el];
                spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                if (spr > sprite) {
                    D_00188c74 = elem->flags;
                    D_00188c74 ^= 1;
                    LbSpriteDraw(x - (elem->x - 0x8d) - spr->width, y + (elem->y - 0xca), spr);
                    if (D_001567b0.x > -(spr->width + (elem->x - 0x8d)))
                        D_001567b0.x = -(elem->x - 0x8d + spr->width);
                    if (D_001567b0.y > elem->y - 0xca)
                        D_001567b0.y = elem->y - 0xca;
                }
            }
        }
    } else {
        if (remap) {
            for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
                elem = &element[*el];
                spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                if (spr > sprite) {
                    D_00188c74 = elem->flags;
                    fn_000cbe20(elem->x - 0x8d + x, y + (elem->y - 0xca), spr, remap);
                    if (D_001567b0.x > elem->x - 0x8d)
                        D_001567b0.x = elem->x - 0x8d;
                    if (D_001567b0.y > elem->y - 0xca)
                        D_001567b0.y = elem->y - 0xca;
                }
            }
        } else {
            for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
                elem = &element[*el];
                spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                if (spr > sprite) {
                    D_00188c74 = elem->flags;
                    LbSpriteDraw(elem->x - 0x8d + x, elem->y - 0xca + y, spr);
                    if (D_001567b0.x > elem->x - 0x8d)
                        D_001567b0.x = elem->x - 0x8d;
                    if (D_001567b0.y > elem->y - 0xca)
                        D_001567b0.y = elem->y - 0xca;
                }
            }
        }
    }
    D_001567b0.x += x;
    D_001567b0.y += y;
    D_001567b0.w = f->width;
    if (D_0015f3f4 == SCREEN_MODE_13) {
        D_001567b0.h = f->length + D_00156911;
    } else {
        D_001567b0.w /= 2;
        D_001567b0.h = f->length / 2 + D_00156911;
    }
    if (D_001567b0.x0 > D_001567b0.x)
        D_001567b0.x0 = D_001567b0.x;
    if (D_001567b0.y0 > D_001567b0.y)
        D_001567b0.y0 = D_001567b0.y;
    if (D_001567b0.x1 < D_001567b0.x + D_001567b0.w)
        D_001567b0.x1 = D_001567b0.x + D_001567b0.w;
    if (D_001567b0.y1 < D_001567b0.y + D_001567b0.h)
        D_001567b0.y1 = D_001567b0.y + D_001567b0.h;
    set_all_refresh_zones_in_window(refresh.x0, D_001567b0.y0, D_001567b0.x1, D_001567b0.y1, 3);
    D_00188c74 = 0;
}


// FUNCTION: 0x00030f34
void draw_from_special_id_frame(const u16 anim, const s16 x, const s16 y, u8 *remap, u8 *layers, const u8 flip)
{
    CompFrame *f;
    u16 *el;
    CompElement *elem;
    ElementBits *bits;
    TbSprite *spr;

    f = &frame[anim];
    D_001567b0.x = D_001567b0.y = 999;
    if (flip) {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            if (elem->flags & 0xffb0) {
                bits = (ElementBits *)&elem->flags;
                if (layers[bits->layer] != bits->id)
                    continue;
            }
            spr = (TbSprite *)((u8 *)sprite + elem->sprite);
            if (spr > sprite) {
                D_00188c74 = elem->flags & 0xf;
                D_00188c74 ^= 1;
                if (remap)
                    fn_000cbe20(x - (elem->x - 0x8d) - spr->width, y + (elem->y - 0xca), spr, remap);
                else
                    LbSpriteDraw(x - (elem->x - 0x8d) - spr->width, y + (elem->y - 0xca), spr);
                if (D_001567b0.x > -(spr->width + (elem->x - 0x8d)))
                    D_001567b0.x = -(elem->x - 0x8d + spr->width);
                if (D_001567b0.y > elem->y - 0xca)
                    D_001567b0.y = elem->y - 0xca;
            }
        }
    } else {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            if (elem->flags & 0xffb0) {
                bits = (ElementBits *)&elem->flags;
                if (layers[bits->layer] != bits->id)
                    continue;
            }
            spr = (TbSprite *)((u8 *)sprite + elem->sprite);
            if (spr > sprite) {
                D_00188c74 = elem->flags & 0xf;
                if (remap)
                    fn_000cbe20(x + (elem->x - 0x8d), y + (elem->y - 0xca), spr, remap);
                else
                    LbSpriteDraw(x + (elem->x - 0x8d), y + (elem->y - 0xca), spr);
                if (D_001567b0.x > elem->x - 0x8d)
                    D_001567b0.x = elem->x - 0x8d;
                if (D_001567b0.y > elem->y - 0xca)
                    D_001567b0.y = elem->y - 0xca;
            }
        }
    }
    D_001567b0.x += x;
    D_001567b0.y += y;
    D_001567b0.w = f->width;
    if (D_0015f3f4 == SCREEN_MODE_13) {
        D_001567b0.h = f->length + D_00156911;
    } else {
        D_001567b0.w /= 2;
        D_001567b0.h = f->length / 2 + D_00156911;
    }
    if (D_001567b0.x0 > D_001567b0.x)
        D_001567b0.x0 = D_001567b0.x;
    if (D_001567b0.y0 > D_001567b0.y)
        D_001567b0.y0 = D_001567b0.y;
    if (D_001567b0.x1 < D_001567b0.x + D_001567b0.w)
        D_001567b0.x1 = D_001567b0.x + D_001567b0.w;
    if (D_001567b0.y1 < D_001567b0.y + D_001567b0.h)
        D_001567b0.y1 = D_001567b0.y + D_001567b0.h;
    set_all_refresh_zones_in_window(refresh.x0, D_001567b0.y0, D_001567b0.x1, D_001567b0.y1, 3);
    D_00188c74 = 0;
}


// NONMATCHING: register choice
// FUNCTION: 0x00031434
void full_draw_frame(Thing *w, const u16 anim, const s16 x, const s16 y, const u8 flags, const u8 hidden, const u8 flip)
{
    CompFrame *f;
    u16 *el;
    CompElement *elem;
    ElementBits *bits;
    TbSprite *spr;

    f = &frame[anim];
    if (flip) {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            if (elem->flags & 0xffb0) {
                bits = (ElementBits *)&elem->flags;
                if (w->layers[bits->layer] != bits->id)
                    continue;
            }
            spr = (TbSprite *)((u8 *)sprite + elem->sprite);
            if (spr > sprite) {
                if (!hidden) {
                    D_00188c74 = flags | elem->flags & 0xf;
                    D_00188c74 ^= 1;
                    LbSpriteDraw(x - (elem->x - 0x8d) - spr->width, y + (elem->y - 0xca), spr);
                }
                if (D_001567b0.x > -(spr->width + (elem->x - 0x8d)))
                    D_001567b0.x = -(elem->x - 0x8d + spr->width);
                if (D_001567b0.y > elem->y - 0xca)
                    D_001567b0.y = elem->y - 0xca;
            }
        }
    } else {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            if (elem->flags & 0xffb0) {
                bits = (ElementBits *)&elem->flags;
                if (w->layers[bits->layer] != bits->id)
                    continue;
            }
            spr = (TbSprite *)((u8 *)sprite + elem->sprite);
            if (spr > sprite) {
                if (!hidden) {
                    D_00188c74 = elem->flags & 0xf | flags;
                    LbSpriteDraw(elem->x - 0x8d + x, y + (elem->y - 0xca), spr);
                }
                if (D_001567b0.x > elem->x - 0x8d)
                    D_001567b0.x = elem->x - 0x8d;
                if (D_001567b0.y > elem->y - 0xca)
                    D_001567b0.y = elem->y - 0xca;
            }
        }
    }
}

// FUNCTION: 0x00031760
void draw_text_on_map(char *text, const u8 colour, const s32 x, const s32 y)
{
    D_00188c74 = 0;
    set_font_from_cache(0);
    D_00188c85 = colour;
    D_00188c74 |= 0x40;
    LbTextDraw(x, y, text);
    D_001567b0.x = x;
    D_001567b0.y = y;
    D_001567b0.w = LbTextStringWidth(text);
    D_001567b0.h = LbTextStringHeight(text);
    if (D_001567b0.x0 > D_001567b0.x)
        D_001567b0.x0 = D_001567b0.x;
    if (D_001567b0.y0 > D_001567b0.y)
        D_001567b0.y0 = D_001567b0.y;
    if (D_001567b0.x1 < D_001567b0.x + D_001567b0.w)
        D_001567b0.x1 = D_001567b0.x + D_001567b0.w;
    if (D_001567b0.y1 < D_001567b0.y + D_001567b0.h)
        D_001567b0.y1 = D_001567b0.y + D_001567b0.h;
}

// NONMATCHING: register choice
// FUNCTION: 0x00031874
void icon_draw_from_special_id_frame(const u16 icon, const s16 x, const s16 y, const s16 dx, const s16 dy)
{
    CompFrame *f;
    u16 *el;
    CompElement *elem;
    TbSprite *spr;

    f = &frame[icon];
    for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
        elem = &element[*el];
        spr = (TbSprite *)((u8 *)sprite + elem->sprite);
        if (spr > sprite) {
            D_00188c74 = elem->flags & 0xf;
            LbSpriteDraw(x + dx - spr->width / 2, elem->y - 0xca + (y + dy) - f->length - D_0015f410->unknown4fc, spr);
            if (D_001567b0.x > dx - spr->width / 2)
                D_001567b0.x = dx - spr->width / 2;
            if (D_001567b0.y > dy - f->length - D_0015f410->unknown4fc)
                D_001567b0.y = dy - f->length - D_0015f410->unknown4fc;
            D_00156911 = D_0015f410->unknown4fc + f->length;
        }
    }
    D_00188c74 = 0;
    D_00155e0a = 1;
}

// FUNCTION: 0x00031a44
void draw_money(s32 x, s32 y, s32 value)
{
    char buf[16];
    char *p;
    s32 width;
    s32 px;
    s32 py;
    u8 digit;

    x += -2;
    D_00188c74 = 0;
    LbSpriteDraw(x, y, D_00153f5c + 1);
    D_001567b0.x = x;
    D_001567b0.y = y;
    D_001567b0.w = D_00153f5c[1].width;
    D_001567b0.h = D_00153f5c[1].height;
    if (D_0015f3f5 == SCREEN_MODE_13 && !D_00155e45) {
        sprintf(buf, D_001202fc, value);
        width = -1;
        for (p = buf; *p; p++) {
            digit = *p - '0';
            width += D_00153f5c[digit + 2].width + 1;
        }
        px = x + D_0015f410->unknown568 + (D_0015f410->unknown560 - width) / 2;
        py = y + D_0015f410->unknown564;
        for (p = buf; *p; p++) {
            digit = *p - '0';
            LbSpriteDraw(px, py, &D_00153f5c[digit + 2]);
            px += D_00153f5c[digit + 2].width + 1;
        }
    }
    if (D_001567b0.x0 > D_001567b0.x)
        D_001567b0.x0 = D_001567b0.x;
    if (D_001567b0.y0 > D_001567b0.y)
        D_001567b0.y0 = D_001567b0.y;
    if (D_001567b0.x1 < D_001567b0.x + D_001567b0.w)
        D_001567b0.x1 = D_001567b0.x + D_001567b0.w;
    if (D_001567b0.y1 < D_001567b0.y + D_001567b0.h)
        D_001567b0.y1 = D_001567b0.y + D_001567b0.h;
}

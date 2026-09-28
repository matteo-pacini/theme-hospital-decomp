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

void LbDrawHVLine(s32 x, s32 y, TbSprite *spr, u8 colour);

}

// NONMATCHING: register choice
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

// NONMATCHING: register choice
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

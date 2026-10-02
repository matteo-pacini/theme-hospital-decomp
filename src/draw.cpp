#include "hosp.h"

struct ElementBits {
    u32 unknown0 : 4;
    u32 layer : 5;
    u32 id : 7;
};

/*
 * Status icon and money bag entries of the in-game position table, read
 * through resvars.coords.
 */
struct IconScreen {
    u8 unknown0[0x4fc];
    // 0x4fc: Extra lift of a status icon above the thing's frame.
    s32 icon_gap;
    u8 unknown500[0x60];
    // 0x560: Width the value's digits are centred in on a money bag.
    s32 money_text_width;
    // 0x564: Y offset of the digits on the bag.
    s32 money_text_y;
    // 0x568: X offset of the centring area on the bag.
    s32 money_text_x;
};

struct FrameBox {
    // 0x0: Dirty rectangle grown to cover every frame drawn since it was reset.
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
    // 0x8: Box of the last frame drawn.
    s16 x;
    s16 y;
    s16 w;
    s16 h;
};

extern "C" {

extern FrameBox frame_bounds;
extern s32 frame_extra_height;
extern TbSprite *money;

void LbSpriteDrawOneColour(s32 x, s32 y, TbSprite *spr, u8 colour);
void LbSpriteDrawScaled(s32 x, s32 y, TbSprite *sprite, s32 dest_width, s32 dest_height);
void LbSpriteDrawRemap(s32 x, s32 y, TbSprite *spr, u8 *remap);

u32 ror13(u32 x);
#pragma aux ror13 = "ror eax,13" parm [eax] value [eax];
}

static u8 glow_path[] = {0x4d, 0x4e, 0x4f, 0x50, 0x50, 0x4f, 0x4e, 0x4d};

// Draws composite frame frame_id with its origin at screen x,y, mirrored if
// flip, with flags ORed into each element's draw flags; plays the frame's
// sound. Records the frame's box in frame_bounds (x, y, w, h) and grows its
// dirty rectangle (x0, y0, x1, y1) to cover it.
// FUNCTION: 0x0002d910
void draw_a_frame(const u16 frame_id, const s16 x, const s16 y, const u8 flags, const u8 flip)
{
    CompFrame *f;
    u16 *el;
    CompElement *elem;
    TbSprite *spr;

    f = &frame[frame_id];
    frame_bounds.x = frame_bounds.y = 999;
    if (flip) {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            spr = (TbSprite *)((u8 *)sprite + elem->sprite);
            if (spr > sprite) {
                lbDisplay.draw_flags = elem->flags | flags;
                lbDisplay.draw_flags ^= 1;
                LbSpriteDraw(x - (elem->x - 0x8d) - spr->width, y + (elem->y - 0xca), spr);
                if (frame_bounds.x > -(spr->width + (elem->x - 0x8d)))
                    frame_bounds.x = -(elem->x - 0x8d + spr->width);
                if (frame_bounds.y > elem->y - 0xca)
                    frame_bounds.y = elem->y - 0xca;
            }
        }
    } else {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            spr = (TbSprite *)((u8 *)sprite + elem->sprite);
            if (spr > sprite) {
                lbDisplay.draw_flags = elem->flags | flags;
                LbSpriteDraw(elem->x - 0x8d + x, y + (elem->y - 0xca), spr);
                if (frame_bounds.x > elem->x - 0x8d)
                    frame_bounds.x = elem->x - 0x8d;
                if (frame_bounds.y > elem->y - 0xca)
                    frame_bounds.y = elem->y - 0xca;
            }
        }
    }
    if (f->sound && gamevar.turn_skipped == 0)
        play_sound(SNDSRC_GENERAL, f->sound, x, y);
    frame_bounds.x += x;
    frame_bounds.y += y;
    frame_bounds.w = f->width;
    if (resvars.screen_mode == SCREEN_MODE_13) {
        frame_bounds.h = f->height + frame_extra_height;
    } else {
        frame_bounds.w /= 2;
        frame_bounds.h = f->height / 2 + frame_extra_height;
    }
    if (frame_bounds.x0 > frame_bounds.x)
        frame_bounds.x0 = frame_bounds.x;
    if (frame_bounds.y0 > frame_bounds.y)
        frame_bounds.y0 = frame_bounds.y;
    if (frame_bounds.x1 < frame_bounds.x + frame_bounds.w)
        frame_bounds.x1 = frame_bounds.x + frame_bounds.w;
    if (frame_bounds.y1 < frame_bounds.y + frame_bounds.h)
        frame_bounds.y1 = frame_bounds.y + frame_bounds.h;
    lbDisplay.draw_flags = 0;
}

// As draw_a_frame, drawing every element in the single colour colour and
// without the sound.
// FUNCTION: 0x0002dd1c
void draw_a_one_colour_frame(const u16 frame_id, const s16 x, const s16 y, const u16 colour, const u8 flip)
{
    CompFrame *f;
    u16 *el;
    CompElement *elem;
    TbSprite *spr;

    f = &frame[frame_id];
    frame_bounds.x = frame_bounds.y = 999;
    if (flip) {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            spr = (TbSprite *)((u8 *)sprite + elem->sprite);
            if (spr > sprite) {
                lbDisplay.draw_flags = elem->flags;
                lbDisplay.draw_flags ^= 1;
                LbSpriteDrawOneColour(x - (elem->x - 0x8d) - spr->width, y + (elem->y - 0xca), spr, colour);
                if (frame_bounds.x > -(spr->width + (elem->x - 0x8d)))
                    frame_bounds.x = -(elem->x - 0x8d + spr->width);
                if (frame_bounds.y > elem->y - 0xca)
                    frame_bounds.y = elem->y - 0xca;
            }
        }
    } else {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            spr = (TbSprite *)((u8 *)sprite + elem->sprite);
            if (spr > sprite) {
                lbDisplay.draw_flags = elem->flags;
                LbSpriteDrawOneColour(elem->x - 0x8d + x, y + (elem->y - 0xca), spr, colour);
                if (frame_bounds.x > elem->x - 0x8d)
                    frame_bounds.x = elem->x - 0x8d;
                if (frame_bounds.y > elem->y - 0xca)
                    frame_bounds.y = elem->y - 0xca;
            }
        }
    }
    frame_bounds.x += x;
    frame_bounds.y += y;
    frame_bounds.w = f->width;
    if (resvars.screen_mode == SCREEN_MODE_13) {
        frame_bounds.h = f->height + frame_extra_height;
    } else {
        frame_bounds.w /= 2;
        frame_bounds.h = f->height / 2 + frame_extra_height;
    }
    if (frame_bounds.x0 > frame_bounds.x)
        frame_bounds.x0 = frame_bounds.x;
    if (frame_bounds.y0 > frame_bounds.y)
        frame_bounds.y0 = frame_bounds.y;
    if (frame_bounds.x1 < frame_bounds.x + frame_bounds.w)
        frame_bounds.x1 = frame_bounds.x + frame_bounds.w;
    if (frame_bounds.y1 < frame_bounds.y + frame_bounds.h)
        frame_bounds.y1 = frame_bounds.y + frame_bounds.h;
    lbDisplay.draw_flags = 0;
}

// Draws w's current frame at x,y, showing only the layered elements that match
// w's layers (head, colours, ...) and drawing w's status icon over its head
// element. Plays the frame's sound once (or every time if its sfx_details
// entry allows repeats), records the box as draw_a_frame does and clears
// frame_extra_height.
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
    frame_bounds.x = frame_bounds.y = 999;
    if (w->flip) {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            if (!othervar.frame_icon_drawn) {
                kind = elem->flags & 0xfff0;
                if (w->icon_id > 0) {
                    if ((kind == 0x200 || kind == 0x250) && icon_table[w->icon_id].anim > 0) {
                        spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                        if (spr > sprite)
                            icon_draw_from_special_id_frame(w->icon_frame, x, y,
                                                            (s16) - ((elem->x - 0x8d) + spr->width / 2),
                                                            (s16)(elem->y - 0xca));
                    } else if (w->type == THING_OBJECT) {
                        icon = 1;
                        othervar.frame_icon_drawn = 1;
                    }
                } else {
                    othervar.frame_icon_drawn = 1;
                }
            }
            if (elem->flags & 0xffb0) {
                bits = (ElementBits *)&elem->flags;
                if (w->layers[bits->layer] != bits->id)
                    continue;
            }
            spr = (TbSprite *)((u8 *)sprite + elem->sprite);
            if (spr > sprite) {
                lbDisplay.draw_flags = elem->flags & 0xf;
                lbDisplay.draw_flags ^= 1;
                LbSpriteDraw(x - (elem->x - 0x8d) - spr->width, y + (elem->y - 0xca), spr);
                if (frame_bounds.x > -(spr->width + (elem->x - 0x8d)))
                    frame_bounds.x = -(elem->x - 0x8d + spr->width);
                if (frame_bounds.y > elem->y - 0xca)
                    frame_bounds.y = elem->y - 0xca;
            }
        }
    } else {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            if (!othervar.frame_icon_drawn) {
                kind = elem->flags & 0xfff0;
                if (w->icon_id > 0) {
                    if ((kind == 0x200 || kind == 0x250) && icon_table[w->icon_id].anim > 0) {
                        spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                        if (spr > sprite)
                            icon_draw_from_special_id_frame(
                                w->icon_frame, x, y, (s16)((elem->x - 0x8d) + spr->width / 2), (s16)(elem->y - 0xca));
                    } else if (w->type == THING_OBJECT) {
                        icon = 1;
                        othervar.frame_icon_drawn = 1;
                    }
                } else {
                    othervar.frame_icon_drawn = 1;
                }
            }
            if (elem->flags & 0xffb0) {
                bits = (ElementBits *)&elem->flags;
                if (w->layers[bits->layer] != bits->id)
                    continue;
            }
            spr = (TbSprite *)((u8 *)sprite + elem->sprite);
            if (spr > sprite) {
                lbDisplay.draw_flags = elem->flags & 0xf;
                LbSpriteDraw(elem->x - 0x8d + x, y + (elem->y - 0xca), spr);
                if (frame_bounds.x > elem->x - 0x8d)
                    frame_bounds.x = elem->x - 0x8d;
                if (frame_bounds.y > elem->y - 0xca)
                    frame_bounds.y = elem->y - 0xca;
            }
        }
    }
    if (f->sound && gamevar.turn_skipped == 0 &&
        (sfx_details[f->sound].allow_repeat || (int)w->last_sound != (int)f->sound))
        play_sound((u32)w, f->sound, x, y);
    if (gamevar.turn_skipped == 0)
        w->last_sound = f->sound;
    if (icon)
        icon_draw_from_special_id_frame(w->icon_frame, x, y, f->width / 2 + frame_bounds.x, frame_bounds.y);
    frame_bounds.x += x;
    frame_bounds.y += y;
    frame_bounds.w = f->width;
    if (resvars.screen_mode == SCREEN_MODE_13) {
        frame_bounds.h = f->height + frame_extra_height;
    } else {
        frame_bounds.w /= 2;
        frame_bounds.h = f->height / 2 + frame_extra_height;
    }
    frame_extra_height = 0;
    if (frame_bounds.x0 > frame_bounds.x)
        frame_bounds.x0 = frame_bounds.x;
    if (frame_bounds.y0 > frame_bounds.y)
        frame_bounds.y0 = frame_bounds.y;
    if (frame_bounds.x1 < frame_bounds.x + frame_bounds.w)
        frame_bounds.x1 = frame_bounds.x + frame_bounds.w;
    if (frame_bounds.y1 < frame_bounds.y + frame_bounds.h)
        frame_bounds.y1 = frame_bounds.y + frame_bounds.h;
    lbDisplay.draw_flags = 0;
    othervar.frame_icon_drawn = 0;
}

extern "C" {

// As draw_special_id_frame, but draws the layered elements through the pixel
// buffer with a wobble (draw_wibble) that changes each turn. The status
// icon's position is worked out but the icon is not drawn, and the box
// recorded is 4 pixels larger on each side.
// FUNCTION: 0x0002e804
void draw_wibble_frame(Thing *w, s16 x, s16 y)
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
    frame_bounds.x = frame_bounds.y = 999;
    if (w->flip) {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            if (!othervar.frame_icon_drawn) {
                kind = elem->flags & 0xfff0;
                if (w->icon_id > 0) {
                    if ((kind == 0x200 || kind == 0x250) && icon_table[w->icon_id].anim > 0) {
                        spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                        if (spr > sprite) {
                            icon = 1;
                            ix = -(elem->x - 0x8d) - spr->width / 2;
                            iy = elem->y - 0xca;
                        }
                    }
                } else {
                    othervar.frame_icon_drawn = 1;
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
                    lbDisplay.draw_flags = elem->flags & 0xf;
                    lbDisplay.draw_flags ^= 1;
                    LbSpriteDraw(0x3a - (elem->x - 0x8d) - spr->width, elem->y - 0xca + 0x74, spr);
                    drawn = 1;
                    if (frame_bounds.x > -(spr->width + (elem->x - 0x8d)))
                        frame_bounds.x = -(elem->x - 0x8d + spr->width);
                    if (frame_bounds.y > elem->y - 0xca)
                        frame_bounds.y = elem->y - 0xca;
                }
                RestoreFromPixBuffer();
            } else {
                if (drawn) {
                    drawn = 0;
                    draw_wibble(x + frame_bounds.x - 4, y + frame_bounds.y - 4,
                                pixBuf.data + 0x3a + frame_bounds.x - 4 + (frame_bounds.y + 0x70) * 0xb4,
                                f->width + 8 < 0x7a - frame_bounds.x + 3 ? f->width + 8 : 0x7a - frame_bounds.x + 3,
                                f->height + 8 < 0x40 - frame_bounds.y + 3 ? f->height + 8 : 0x40 - frame_bounds.y + 3,
                                phase);
                    FillBoxInPixBuffer(0xfe, frame_bounds.x + 0x3a, frame_bounds.y + 0x74, f->width, f->height);
                }
                spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                if (spr > sprite) {
                    lbDisplay.draw_flags = elem->flags & 0xf;
                    lbDisplay.draw_flags ^= 1;
                    LbSpriteDraw(x - (elem->x - 0x8d) - spr->width, y + (elem->y - 0xca), spr);
                    if (frame_bounds.x > -(spr->width + (elem->x - 0x8d)))
                        frame_bounds.x = -(elem->x - 0x8d + spr->width);
                    if (frame_bounds.y > elem->y - 0xca)
                        frame_bounds.y = elem->y - 0xca;
                }
            }
        }
    } else {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            if (!othervar.frame_icon_drawn) {
                kind = elem->flags & 0xfff0;
                if (w->icon_id > 0) {
                    if ((kind == 0x200 || kind == 0x250) && icon_table[w->icon_id].anim > 0) {
                        spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                        if (spr > sprite) {
                            icon = 1;
                            ix = elem->x - 0x8d + spr->width / 2;
                            iy = elem->y - 0xca;
                        }
                    }
                } else {
                    othervar.frame_icon_drawn = 1;
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
                    lbDisplay.draw_flags = elem->flags & 0xf;
                    LbSpriteDraw(elem->x - 0x8d + 0x3a, elem->y - 0xca + 0x74, spr);
                    drawn = 1;
                    if (frame_bounds.x > elem->x - 0x8d)
                        frame_bounds.x = elem->x - 0x8d;
                    if (frame_bounds.y > elem->y - 0xca)
                        frame_bounds.y = elem->y - 0xca;
                }
                RestoreFromPixBuffer();
            } else {
                if (drawn) {
                    drawn = 0;
                    draw_wibble(x + frame_bounds.x - 4, y + frame_bounds.y - 4,
                                pixBuf.data + 0x3a + frame_bounds.x - 4 + (frame_bounds.y + 0x70) * 0xb4,
                                f->width + 8 < 0x7a - frame_bounds.x + 3 ? f->width + 8 : 0x7a - frame_bounds.x + 3,
                                f->height + 8 < 0x40 - frame_bounds.y + 3 ? f->height + 8 : 0x40 - frame_bounds.y + 3,
                                phase);
                    FillBoxInPixBuffer(0xfe, frame_bounds.x + 0x3a, frame_bounds.y + 0x74, f->width, f->height);
                }
                spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                if (spr > sprite) {
                    lbDisplay.draw_flags = elem->flags & 0xf;
                    LbSpriteDraw(elem->x - 0x8d + x, y + (elem->y - 0xca), spr);
                    if (frame_bounds.x > elem->x - 0x8d)
                        frame_bounds.x = elem->x - 0x8d;
                    if (frame_bounds.y > elem->y - 0xca)
                        frame_bounds.y = elem->y - 0xca;
                }
            }
        }
    }
    if (drawn) {
        draw_wibble(x + frame_bounds.x - 4, y + frame_bounds.y - 4,
                    pixBuf.data + 0x3a + frame_bounds.x - 4 + (frame_bounds.y + 0x70) * 0xb4,
                    f->width + 8 < 0x7a - frame_bounds.x + 3 ? f->width + 8 : 0x7a - frame_bounds.x + 3,
                    f->height + 8 < 0x40 - frame_bounds.y + 3 ? f->height + 8 : 0x40 - frame_bounds.y + 3, phase);
    }
    FillBoxInPixBuffer(0xfe, frame_bounds.x + 0x3a, frame_bounds.y + 0x74, f->width, f->height);
    if (f->sound && gamevar.turn_skipped == 0 &&
        (sfx_details[f->sound].allow_repeat || (int)w->last_sound != (int)f->sound))
        play_sound((u32)w, f->sound, x, y);
    if (gamevar.turn_skipped == 0)
        w->last_sound = f->sound;
    frame_bounds.x += x;
    frame_bounds.y += y;
    frame_bounds.w = f->width;
    if (resvars.screen_mode == SCREEN_MODE_13) {
        frame_bounds.h = f->height + frame_extra_height;
    } else {
        frame_bounds.w /= 2;
        frame_bounds.h = f->height / 2 + frame_extra_height;
    }
    frame_extra_height = 0;
    frame_bounds.x -= 4;
    frame_bounds.y -= 4;
    frame_bounds.w += 8;
    frame_bounds.h += 8;
    if (frame_bounds.x0 > frame_bounds.x)
        frame_bounds.x0 = frame_bounds.x;
    if (frame_bounds.y0 > frame_bounds.y)
        frame_bounds.y0 = frame_bounds.y;
    if (frame_bounds.x1 < frame_bounds.x + frame_bounds.w)
        frame_bounds.x1 = frame_bounds.x + frame_bounds.w;
    if (frame_bounds.y1 < frame_bounds.y + frame_bounds.h)
        frame_bounds.y1 = frame_bounds.y + frame_bounds.h;
    lbDisplay.draw_flags = 0;
    othervar.frame_icon_drawn = 0;
}

// As draw_special_id_frame, but draws the layered elements glowing in a colour
// that cycles over time (glow_path).
// FUNCTION: 0x0002f47c
void draw_glow_frame(Thing *w, s16 x, s16 y)
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
    glow = glow_path[((actual_game_turn + w->layers[0] + w->layers[1]) >> 1) & 7];
    frame_bounds.x = frame_bounds.y = 999;
    for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
        elem = &element[*el];
        if (elem->flags & 0xfff0) {
            if (!othervar.frame_icon_drawn) {
                kind = elem->flags & 0xfff0;
                if (w->icon_id > 0) {
                    if ((kind == 0x200 || kind == 0x250) && icon_table[w->icon_id].anim > 0) {
                        spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                        if (spr > sprite) {
                            if (w->flip)
                                icon_draw_from_special_id_frame(
                                    w->icon_frame, x, y, -((elem->x - 0x8d) + spr->width / 2), (s16)(elem->y - 0xca));
                            else
                                icon_draw_from_special_id_frame(w->icon_frame, x, y, (elem->x - 0x8d) + spr->width / 2,
                                                                (s16)(elem->y - 0xca));
                        }
                    }
                } else {
                    othervar.frame_icon_drawn = 1;
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
            lbDisplay.draw_flags = elem->flags & 0xf;
            if (g)
                lbDisplay.draw_flags |= 4;
            if (w->flip) {
                lbDisplay.draw_flags ^= 1;
                sprite_draw_glow(x - (elem->x - 0x8d) - spr->width, y + (elem->y - 0xca), spr, g);
                if (frame_bounds.x > -(spr->width + (elem->x - 0x8d)))
                    frame_bounds.x = -(elem->x - 0x8d + spr->width);
                if (frame_bounds.y > elem->y - 0xca)
                    frame_bounds.y = elem->y - 0xca;
            } else {
                sprite_draw_glow(elem->x - 0x8d + x, y + (elem->y - 0xca), spr, g);
                if (frame_bounds.x > elem->x - 0x8d)
                    frame_bounds.x = elem->x - 0x8d;
                if (frame_bounds.y > elem->y - 0xca)
                    frame_bounds.y = elem->y - 0xca;
            }
        }
    }
    if (f->sound && gamevar.turn_skipped == 0 &&
        (sfx_details[f->sound].allow_repeat || (int)w->last_sound != (int)f->sound))
        play_sound((u32)w, f->sound, x, y);
    if (gamevar.turn_skipped == 0)
        w->last_sound = f->sound;
    frame_bounds.x += x;
    frame_bounds.y += y;
    frame_bounds.w = f->width;
    if (resvars.screen_mode == SCREEN_MODE_13) {
        frame_bounds.h = f->height + frame_extra_height;
    } else {
        frame_bounds.w /= 2;
        frame_bounds.h = f->height / 2 + frame_extra_height;
    }
    frame_extra_height = 0;
    if (frame_bounds.x0 > frame_bounds.x)
        frame_bounds.x0 = frame_bounds.x;
    if (frame_bounds.y0 > frame_bounds.y)
        frame_bounds.y0 = frame_bounds.y;
    if (frame_bounds.x1 < frame_bounds.x + frame_bounds.w)
        frame_bounds.x1 = frame_bounds.x + frame_bounds.w;
    if (frame_bounds.y1 < frame_bounds.y + frame_bounds.h)
        frame_bounds.y1 = frame_bounds.y + frame_bounds.h;
    lbDisplay.draw_flags = 0;
    othervar.frame_icon_drawn = 0;
}
}

// Draws w using a machine (its object): w's own frame above the machine's
// split line (split_height), and the machine's aux_sprite frame below it, so
// that w appears inside the machine. An alien (subtype PK_ALIEN) also has the
// machine drawn first and a looping overlay animation drawn at the split line.
// Does nothing if w has no object.
// FUNCTION: 0x0002fa3c
void draw_split_frame(Thing *w, const s16 x, const s16 y)
{
    s32 old_top;
    s32 old_height;
    Thing *obj;
    CompFrame *f;
    u8 split_height;
    u8 rest;

    old_top = lbDisplay.graphics_window_y;
    old_height = lbDisplay.graphics_window_height;
    obj = &thing[w->object];
    if (obj > thing) {
        if (w->subtype == PK_ALIEN)
            draw_special_id_frame(obj, x, y);
        f = &frame[w->sprite];
        split_height = obj->split_height;
        rest = f->height - split_height;
        frame_bounds.x = frame_bounds.y = 999;
        {
            s32 top = y - split_height;

            lbDisplay.graphics_window_height = top - lbDisplay.graphics_window_y > lbDisplay.graphics_window_height
                                                   ? lbDisplay.graphics_window_height
                                                   : top - lbDisplay.graphics_window_y;
        }
        full_draw_frame(w, w->sprite, x, y, 0, 0, w->flip);
        {
            s32 top = y - split_height;

            lbDisplay.graphics_window_y = top > old_top ? top : old_top;
        }
        lbDisplay.graphics_window_height = old_height + old_top - lbDisplay.graphics_window_y;
        full_draw_frame(w, obj->aux_sprite, x, y - lbDisplay.graphics_window_y, 0, 0, obj->aux_flip);
        lbDisplay.graphics_window_y = old_top;
        lbDisplay.graphics_window_height = old_height;
        lbDisplay.draw_flags = 0;
        if (f->sound && gamevar.turn_skipped == 0 && (sfx_details[f->sound].allow_repeat || w->last_sound != f->sound))
            play_sound((u32)w, f->sound, x, y);
        if (gamevar.turn_skipped == 0)
            w->last_sound = f->sound;
        frame_bounds.x += x;
        frame_bounds.y += y;
        frame_bounds.w = f->width;
        if (resvars.screen_mode == SCREEN_MODE_13) {
            frame_bounds.h = f->height + frame_extra_height;
        } else {
            frame_bounds.w /= 2;
            frame_bounds.h = f->height / 2 + frame_extra_height;
        }
        if (frame_bounds.x0 > frame_bounds.x)
            frame_bounds.x0 = frame_bounds.x;
        if (frame_bounds.y0 > frame_bounds.y)
            frame_bounds.y0 = frame_bounds.y;
        if (frame_bounds.x1 < frame_bounds.x + frame_bounds.w)
            frame_bounds.x1 = frame_bounds.x + frame_bounds.w;
        if (frame_bounds.y1 < frame_bounds.y + frame_bounds.h)
            frame_bounds.y1 = frame_bounds.y + frame_bounds.h;
        if (w->subtype == PK_ALIEN) {
            s32 anim;

            if (w->facing == 0)
                anim = 0x787;
            else
                anim = 0x784;
            anim += actual_game_turn % 3;
            draw_a_frame(start[anim].frame, x, y - split_height, 0, start[anim].flags > 0);
        }
    }
}

// Draws w using an energising machine: w normally (with the machine's
// aux_draw_flags, unless 0xff), then a speckled copy in colour aux_param whose
// density grows with the machine's split_height. Does nothing if w has no
// object.
// FUNCTION: 0x0002fe18
void draw_energise_frame(Thing *w, const s16 x, const s16 y)
{
    CompFrame *f;
    u32 rng;
    Thing *obj;
    u16 i;

    obj = &thing[w->object];
    rng = 0x546b1;
    if (obj > thing) {
        f = &frame[w->sprite];
        frame_bounds.x = frame_bounds.y = 999;
        if (obj->aux_draw_flags != 0xff)
            full_draw_frame(w, w->sprite, x, y, obj->aux_draw_flags, 0, w->flip);
        InitPixBuffer(0, 0, 0x40, 0x40, 0, 0);
        DrawIntoPixBuffer();
        full_draw_frame(w, w->sprite, 0x20, 0x3a, 0, 0, w->flip);
        RestoreFromPixBuffer();
        for (i = obj->split_height * 10; i > 0; i--) {
            pixBuf.data[rng & 0xfff] = 0xfe;
            pixBuf.data[(rng >> 10) & 0xfff] = 0xfe;
            pixBuf.data[(rng >> 20) & 0xfff] = 0xfe;
            rng = rng * 0x24a1 + 0x24df;
            rng = ror13(rng);
        }
        pixBuf.x = x - 0x20;
        pixBuf.y = y - 0x3a;
        DrawPixBuffer(obj->aux_param, 0, 0, 0x40, 0x40);
        FillPixBuffer(0xfe);
        InitPixBuffer(0, 0, 0xb4, 0xb4, 0, 0);
        if (f->sound && gamevar.turn_skipped == 0 && (sfx_details[f->sound].allow_repeat || w->last_sound != f->sound))
            play_sound((u32)w, f->sound, x, y);
        if (gamevar.turn_skipped == 0)
            w->last_sound = f->sound;
        frame_bounds.x += x;
        frame_bounds.y += y;
        frame_bounds.w = f->width;
        if (resvars.screen_mode == SCREEN_MODE_13) {
            frame_bounds.h = f->height + frame_extra_height;
        } else {
            frame_bounds.w /= 2;
            frame_bounds.h = f->height / 2 + frame_extra_height;
        }
        if (frame_bounds.x0 > frame_bounds.x)
            frame_bounds.x0 = frame_bounds.x;
        if (frame_bounds.y0 > frame_bounds.y)
            frame_bounds.y0 = frame_bounds.y;
        if (frame_bounds.x1 < frame_bounds.x + frame_bounds.w)
            frame_bounds.x1 = frame_bounds.x + frame_bounds.w;
        if (frame_bounds.y1 < frame_bounds.y + frame_bounds.h)
            frame_bounds.y1 = frame_bounds.y + frame_bounds.h;
        lbDisplay.draw_flags = 0;
    }
}

// As draw_a_frame, scaled by scale (16.16 fixed point). The recorded box is
// unscaled.
// FUNCTION: 0x00030198
void draw_a_frame_scaled(const u16 frame_id, const s16 x, const s16 y, u8 flags, const u8 flip, s32 scale)
{
    CompFrame *f;
    u16 *el;
    CompElement *elem;
    TbSprite *spr;

    f = &frame[frame_id];
    frame_bounds.x = frame_bounds.y = 999;
    if (flip) {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            spr = (TbSprite *)((u8 *)sprite + elem->sprite);
            if (spr > sprite) {
                lbDisplay.draw_flags = elem->flags | flags;
                lbDisplay.draw_flags ^= 1;
                LbSpriteDrawScaled(x - ((elem->x - 0x8d + spr->width) * scale >> 16),
                                   y + ((elem->y - 0xca) * scale >> 16), spr, spr->width * scale >> 16,
                                   spr->height * scale >> 16);
                if (frame_bounds.x > -(elem->x - 0x8d + spr->width))
                    frame_bounds.x = -(elem->x - 0x8d + spr->width);
                if (frame_bounds.y > elem->y - 0xca)
                    frame_bounds.y = elem->y - 0xca;
            }
        }
    } else {
        for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
            elem = &element[*el];
            spr = (TbSprite *)((u8 *)sprite + elem->sprite);
            if (spr > sprite) {
                lbDisplay.draw_flags = elem->flags | flags;
                LbSpriteDrawScaled(x + ((elem->x - 0x8d) * scale >> 16), y + ((elem->y - 0xca) * scale >> 16), spr,
                                   spr->width * scale >> 16, spr->height * scale >> 16);
                if (frame_bounds.x > elem->x - 0x8d)
                    frame_bounds.x = elem->x - 0x8d;
                if (frame_bounds.y > elem->y - 0xca)
                    frame_bounds.y = elem->y - 0xca;
            }
        }
    }
    if (f->sound && gamevar.turn_skipped == 0)
        play_sound(SNDSRC_GENERAL, f->sound, x, y);
    frame_bounds.x += x;
    frame_bounds.y += y;
    frame_bounds.w = f->width;
    if (resvars.screen_mode == SCREEN_MODE_13) {
        frame_bounds.h = f->height + frame_extra_height;
    } else {
        frame_bounds.w /= 2;
        frame_bounds.h = f->height / 2 + frame_extra_height;
    }
    if (frame_bounds.x0 > frame_bounds.x)
        frame_bounds.x0 = frame_bounds.x;
    if (frame_bounds.y0 > frame_bounds.y)
        frame_bounds.y0 = frame_bounds.y;
    if (frame_bounds.x1 < frame_bounds.x + frame_bounds.w)
        frame_bounds.x1 = frame_bounds.x + frame_bounds.w;
    if (frame_bounds.y1 < frame_bounds.y + frame_bounds.h)
        frame_bounds.y1 = frame_bounds.y + frame_bounds.h;
    lbDisplay.draw_flags = 0;
}

// Draws w's frame flashing (cycling the draw flags every turn, hidden one turn
// in six), as for a highlighted thing.
// FUNCTION: 0x000305fc
void draw_hilight_frame(Thing *w, const s16 x, const s16 y)
{
    u8 phase;
    u8 hidden;
    CompFrame *f;
    s32 flags;

    f = &frame[w->sprite];
    hidden = 0;
    frame_bounds.x = frame_bounds.y = 999;
    phase = game_turn % 6;
    if (phase >= 4)
        phase = 6 - phase;
    switch (phase) {
    case 0:
        flags = 0;
        break;
    case 1:
        flags = 4;
        break;
    case 2:
        flags = 8;
        break;
    case 3:
        flags = 0;
        hidden = 1;
        break;
    }
    full_draw_frame(w, w->sprite, x, y, flags, hidden, w->flip);
    if (f->sound && gamevar.turn_skipped == 0 &&
        (sfx_details[f->sound].allow_repeat || (int)w->last_sound != (int)f->sound))
        play_sound((u32)w, f->sound, x, y);
    if (gamevar.turn_skipped == 0)
        w->last_sound = f->sound;
    frame_bounds.x += x;
    frame_bounds.y += y;
    frame_bounds.w = f->width;
    if (resvars.screen_mode == SCREEN_MODE_13) {
        frame_bounds.h = f->height + frame_extra_height;
    } else {
        frame_bounds.w /= 2;
        frame_bounds.h = f->height / 2 + frame_extra_height;
    }
    if (frame_bounds.x0 > frame_bounds.x)
        frame_bounds.x0 = frame_bounds.x;
    if (frame_bounds.y0 > frame_bounds.y)
        frame_bounds.y0 = frame_bounds.y;
    if (frame_bounds.x1 < frame_bounds.x + frame_bounds.w)
        frame_bounds.x1 = frame_bounds.x + frame_bounds.w;
    if (frame_bounds.y1 < frame_bounds.y + frame_bounds.h)
        frame_bounds.y1 = frame_bounds.y + frame_bounds.h;
    lbDisplay.draw_flags = 0;
}

// Draws composite frame frame_id at x,y, through the colour table remap if not
// 0, mirrored if flip. Resets the dirty rectangle to this frame's box and flags
// it for redraw.
// FUNCTION: 0x000308a0
void draw_from_frame(const u16 frame_id, const s16 x, const s16 y, u8 *remap, const u8 flip)
{
    CompFrame *f;
    u16 *el;
    CompElement *elem;
    TbSprite *spr;

    f = &frame[frame_id];
    frame_bounds.x0 = frame_bounds.y0 = 999;
    frame_bounds.x1 = frame_bounds.y1 = 0;
    frame_bounds.x = frame_bounds.y = 999;
    if (flip) {
        if (remap) {
            for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
                elem = &element[*el];
                spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                if (spr > sprite) {
                    lbDisplay.draw_flags = elem->flags;
                    lbDisplay.draw_flags ^= 1;
                    LbSpriteDrawRemap(x - (elem->x - 0x8d) - spr->width, y + (elem->y - 0xca), spr, remap);
                    if (frame_bounds.x > -(spr->width + (elem->x - 0x8d)))
                        frame_bounds.x = -(elem->x - 0x8d + spr->width);
                    if (frame_bounds.y > elem->y - 0xca)
                        frame_bounds.y = elem->y - 0xca;
                }
            }
        } else {
            for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
                elem = &element[*el];
                spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                if (spr > sprite) {
                    lbDisplay.draw_flags = elem->flags;
                    lbDisplay.draw_flags ^= 1;
                    LbSpriteDraw(x - (elem->x - 0x8d) - spr->width, y + (elem->y - 0xca), spr);
                    if (frame_bounds.x > -(spr->width + (elem->x - 0x8d)))
                        frame_bounds.x = -(elem->x - 0x8d + spr->width);
                    if (frame_bounds.y > elem->y - 0xca)
                        frame_bounds.y = elem->y - 0xca;
                }
            }
        }
    } else {
        if (remap) {
            for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
                elem = &element[*el];
                spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                if (spr > sprite) {
                    lbDisplay.draw_flags = elem->flags;
                    LbSpriteDrawRemap(elem->x - 0x8d + x, y + (elem->y - 0xca), spr, remap);
                    if (frame_bounds.x > elem->x - 0x8d)
                        frame_bounds.x = elem->x - 0x8d;
                    if (frame_bounds.y > elem->y - 0xca)
                        frame_bounds.y = elem->y - 0xca;
                }
            }
        } else {
            for (el = &elelist[f->list]; *el != (u16)0xffff; el++) {
                elem = &element[*el];
                spr = (TbSprite *)((u8 *)sprite + elem->sprite);
                if (spr > sprite) {
                    lbDisplay.draw_flags = elem->flags;
                    LbSpriteDraw(elem->x - 0x8d + x, y + (elem->y - 0xca), spr);
                    if (frame_bounds.x > elem->x - 0x8d)
                        frame_bounds.x = elem->x - 0x8d;
                    if (frame_bounds.y > elem->y - 0xca)
                        frame_bounds.y = elem->y - 0xca;
                }
            }
        }
    }
    frame_bounds.x += x;
    frame_bounds.y += y;
    frame_bounds.w = f->width;
    if (resvars.screen_mode == SCREEN_MODE_13) {
        frame_bounds.h = f->height + frame_extra_height;
    } else {
        frame_bounds.w /= 2;
        frame_bounds.h = f->height / 2 + frame_extra_height;
    }
    if (frame_bounds.x0 > frame_bounds.x)
        frame_bounds.x0 = frame_bounds.x;
    if (frame_bounds.y0 > frame_bounds.y)
        frame_bounds.y0 = frame_bounds.y;
    if (frame_bounds.x1 < frame_bounds.x + frame_bounds.w)
        frame_bounds.x1 = frame_bounds.x + frame_bounds.w;
    if (frame_bounds.y1 < frame_bounds.y + frame_bounds.h)
        frame_bounds.y1 = frame_bounds.y + frame_bounds.h;
    set_all_refresh_zones_in_window(frame_bounds.x0, frame_bounds.y0, frame_bounds.x1, frame_bounds.y1, 3);
    lbDisplay.draw_flags = 0;
}

// As draw_from_frame, showing only the layered elements that match layers.
// Grows rather than resets the dirty rectangle.
// FUNCTION: 0x00030f34
void draw_from_special_id_frame(const u16 frame_id, const s16 x, const s16 y, u8 *remap, u8 *layers, const u8 flip)
{
    CompFrame *f;
    u16 *el;
    CompElement *elem;
    ElementBits *bits;
    TbSprite *spr;

    f = &frame[frame_id];
    frame_bounds.x = frame_bounds.y = 999;
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
                lbDisplay.draw_flags = elem->flags & 0xf;
                lbDisplay.draw_flags ^= 1;
                if (remap)
                    LbSpriteDrawRemap(x - (elem->x - 0x8d) - spr->width, y + (elem->y - 0xca), spr, remap);
                else
                    LbSpriteDraw(x - (elem->x - 0x8d) - spr->width, y + (elem->y - 0xca), spr);
                if (frame_bounds.x > -(spr->width + (elem->x - 0x8d)))
                    frame_bounds.x = -(elem->x - 0x8d + spr->width);
                if (frame_bounds.y > elem->y - 0xca)
                    frame_bounds.y = elem->y - 0xca;
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
                lbDisplay.draw_flags = elem->flags & 0xf;
                if (remap)
                    LbSpriteDrawRemap(x + (elem->x - 0x8d), y + (elem->y - 0xca), spr, remap);
                else
                    LbSpriteDraw(x + (elem->x - 0x8d), y + (elem->y - 0xca), spr);
                if (frame_bounds.x > elem->x - 0x8d)
                    frame_bounds.x = elem->x - 0x8d;
                if (frame_bounds.y > elem->y - 0xca)
                    frame_bounds.y = elem->y - 0xca;
            }
        }
    }
    frame_bounds.x += x;
    frame_bounds.y += y;
    frame_bounds.w = f->width;
    if (resvars.screen_mode == SCREEN_MODE_13) {
        frame_bounds.h = f->height + frame_extra_height;
    } else {
        frame_bounds.w /= 2;
        frame_bounds.h = f->height / 2 + frame_extra_height;
    }
    if (frame_bounds.x0 > frame_bounds.x)
        frame_bounds.x0 = frame_bounds.x;
    if (frame_bounds.y0 > frame_bounds.y)
        frame_bounds.y0 = frame_bounds.y;
    if (frame_bounds.x1 < frame_bounds.x + frame_bounds.w)
        frame_bounds.x1 = frame_bounds.x + frame_bounds.w;
    if (frame_bounds.y1 < frame_bounds.y + frame_bounds.h)
        frame_bounds.y1 = frame_bounds.y + frame_bounds.h;
    set_all_refresh_zones_in_window(frame_bounds.x0, frame_bounds.y0, frame_bounds.x1, frame_bounds.y1, 3);
    lbDisplay.draw_flags = 0;
}

// Draws frame frame_id at x,y for w (only the layered elements matching w's
// layers), with flags ORed into the draw flags and mirrored if flip; with
// hidden set nothing is drawn. Only grows frame_bounds's x,y to the frame's top
// left (relative to x,y), for the caller to finish.
// FUNCTION: 0x00031434
void full_draw_frame(Thing *w, const u16 frame_id, const s16 x, const s16 y, const u8 flags, const u8 hidden,
                     const u8 flip)
{
    CompFrame *f;
    u16 *el;
    CompElement *elem;
    ElementBits *bits;
    TbSprite *spr;

    f = &frame[frame_id];
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
                    lbDisplay.draw_flags = elem->flags & 0xf | flags;
                    lbDisplay.draw_flags ^= 1;
                    LbSpriteDraw(x - (elem->x - 0x8d) - spr->width, y + (elem->y - 0xca), spr);
                }
                if (frame_bounds.x > -(spr->width + (elem->x - 0x8d)))
                    frame_bounds.x = -(elem->x - 0x8d + spr->width);
                if (frame_bounds.y > elem->y - 0xca)
                    frame_bounds.y = elem->y - 0xca;
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
                    lbDisplay.draw_flags = elem->flags & 0xf | flags;
                    LbSpriteDraw(elem->x - 0x8d + x, y + (elem->y - 0xca), spr);
                }
                if (frame_bounds.x > elem->x - 0x8d)
                    frame_bounds.x = elem->x - 0x8d;
                if (frame_bounds.y > elem->y - 0xca)
                    frame_bounds.y = elem->y - 0xca;
            }
        }
    }
}

// Draws text at x,y in colour colour with font 0, and records its box in
// frame_bounds.
// FUNCTION: 0x00031760
void draw_text_on_map(char *text, const u8 colour, const s32 x, const s32 y)
{
    lbDisplay.draw_flags = 0;
    set_font_from_cache(0);
    lbDisplay.draw_colour = colour;
    lbDisplay.draw_flags |= 0x40;
    LbTextDraw(x, y, text);
    frame_bounds.x = x;
    frame_bounds.y = y;
    frame_bounds.w = LbTextStringWidth(text);
    frame_bounds.h = LbTextStringHeight(text);
    if (frame_bounds.x0 > frame_bounds.x)
        frame_bounds.x0 = frame_bounds.x;
    if (frame_bounds.y0 > frame_bounds.y)
        frame_bounds.y0 = frame_bounds.y;
    if (frame_bounds.x1 < frame_bounds.x + frame_bounds.w)
        frame_bounds.x1 = frame_bounds.x + frame_bounds.w;
    if (frame_bounds.y1 < frame_bounds.y + frame_bounds.h)
        frame_bounds.y1 = frame_bounds.y + frame_bounds.h;
}

// Draws icon frame icon centred at x + dx, above y + dy by the frame's height
// and the layout's icon gap, and notes the extra height in frame_extra_height.
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
            lbDisplay.draw_flags = elem->flags & 0xf;
            LbSpriteDraw(x + dx - spr->width / 2,
                         (y + dy) + (elem->y - 0xca) - f->height - resvars.coords_IconScreenp->icon_gap, spr);
            if (frame_bounds.x > dx - spr->width / 2)
                frame_bounds.x = dx - spr->width / 2;
            if (frame_bounds.y > dy - f->height - resvars.coords_IconScreenp->icon_gap) {
                frame_bounds.y = dy - f->height - resvars.coords_IconScreenp->icon_gap;
                frame_extra_height = resvars.coords_IconScreenp->icon_gap + f->height;
            }
        }
    }
    lbDisplay.draw_flags = 0;
    othervar.frame_icon_drawn = 1;
}

// Draws a money bag at x,y, with value written on it in high resolution
// (unless othervar.use_lores_assets), and records its box in frame_bounds.
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
    lbDisplay.draw_flags = 0;
    LbSpriteDraw(x, y, money + 1);
    frame_bounds.x = x;
    frame_bounds.y = y;
    frame_bounds.w = money[1].width;
    frame_bounds.h = money[1].height;
    if (resvars.game_mode == SCREEN_MODE_13 && !othervar.use_lores_assets) {
        sprintf(buf, "%d", value);
        width = -1;
        for (p = buf; *p; p++) {
            digit = *p - '0';
            width += money[digit + 2].width + 1;
        }
        px = x + resvars.coords_IconScreenp->money_text_x + (resvars.coords_IconScreenp->money_text_width - width) / 2;
        py = y + resvars.coords_IconScreenp->money_text_y;
        for (p = buf; *p; p++) {
            digit = *p - '0';
            LbSpriteDraw(px, py, &money[digit + 2]);
            px += money[digit + 2].width + 1;
        }
    }
    if (frame_bounds.x0 > frame_bounds.x)
        frame_bounds.x0 = frame_bounds.x;
    if (frame_bounds.y0 > frame_bounds.y)
        frame_bounds.y0 = frame_bounds.y;
    if (frame_bounds.x1 < frame_bounds.x + frame_bounds.w)
        frame_bounds.x1 = frame_bounds.x + frame_bounds.w;
    if (frame_bounds.y1 < frame_bounds.y + frame_bounds.h)
        frame_bounds.y1 = frame_bounds.y + frame_bounds.h;
}

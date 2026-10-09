#include "core/render/frame.h"

#include "core/data/gamedata.h"
#include "core/render/blit.h"

#include <algorithm>
#include <array>
#include <climits>

namespace th
{

namespace
{

// Whether draw_frame draws element `e` for the layer choices `layers`.
bool element_drawn(const AnimElement &e, const uint8_t *layers)
{
    // The game tests the layer bits with 0xffb0, so an element tagged only
    // 0x40 (layer 4, id 0) is always drawn.
    if (layers && (e.flags & 0xffb0)) {
        int layer = (e.flags >> 4) & 0x1f;
        int choice = e.flags >> 9;
        if (layer >= kLayerCount || layers[layer] != choice)
            return false;
    }
    // Element sprites are byte offsets into the 6-byte TbSprite table;
    // offset 0 (the table's first entry) means no sprite.
    return e.sprite != 0;
}

// draw_a_frame_scaled's element: sprite `s` at `offset` (unmirrored) from
// the frame's origin `at`, mirrored if `flip`, scaled by `scale` (16.16).
struct ScaledElement {
    Point at;
    Point offset;
    bool flip;
    int32_t scale;
};

void draw_element_scaled(Surface &dst, const ScaledElement &e, const Sprite &s, unsigned flags, const uint8_t *ghost)
{
    int32_t k = e.scale;
    int x = e.flip ? e.at.x - ((e.offset.x + s.width) * k >> 16) : e.at.x + (e.offset.x * k >> 16);
    draw_sprite_scaled(dst, {x, e.at.y + (e.offset.y * k >> 16), s.width * k >> 16, s.height * k >> 16}, s,
                       e.flip ? flags ^ kDrawFlipX : flags, ghost);
}

// The box drawn, relative to a frame's origin: the game's frame_bounds
// left/top/right/bottom.
struct Bounds {
    int left = INT_MAX, top = INT_MAX, right = INT_MIN, bottom = INT_MIN;
    bool empty() const { return left == INT_MAX; }
    void grow(int x, int y, int w, int h)
    {
        left = std::min(left, x);
        top = std::min(top, y);
        right = std::max(right, x + w);
        bottom = std::max(bottom, y + h);
    }
};

// The status icon's anchor on element `e`, if it is a head element
// (draw_special_id_frame's 0x200/0x250 test): the icon's centre offset from
// the frame's origin. False otherwise.
bool icon_anchor(const AnimElement &e, bool flip, const GameData &data, Point &offset)
{
    uint16_t kind = e.flags & 0xfff0;
    if ((kind != 0x200 && kind != 0x250) || e.sprite == 0)
        return false;
    Sprite s = data.sprites().at(e.sprite / 6);
    int ax = e.x - kElementBiasX;
    offset = {flip ? -(ax + s.width / 2) : ax + s.width / 2, e.y - kElementBiasY};
    return true;
}

// icon_draw_from_special_id_frame (0x31874): draws status-icon frame `icon`
// centred at `offset` from the thing's origin, its elements' bottoms lifted
// by the icon frame's height and kIconGap, growing `bounds`. Not mirrored.
void draw_icon(Surface &dst, const GameData &data, uint32_t icon, Point at, Point offset, Bounds &bounds)
{
    const AnimData &anims = data.anims();
    if (icon >= anims.frames.size())
        return;
    const AnimFrame &f = anims.frames[icon];
    for (size_t i = f.list; i < anims.lists.size() && anims.lists[i] != kListEnd; i++) {
        uint16_t id = anims.lists[i];
        if (id >= anims.elements.size())
            continue;
        const AnimElement &e = anims.elements[id];
        if (e.sprite == 0)
            continue;
        Sprite s = data.sprites().at(e.sprite / 6);
        int ex = offset.x - s.width / 2;
        int ey = offset.y + (e.y - kElementBiasY) - f.height - kIconGap;
        draw_sprite(dst, at.x + ex, at.y + ey, s, e.flags & 0xf, data.ghost().data());
        bounds.grow(ex, ey, s.width, s.height);
    }
}

// What a frame's element is drawn with: the surface and origin, the frame's
// mirroring, the game data and the options.
struct ElementContext {
    Surface &dst;
    Point at;
    bool flip;
    const GameData &data;
    const FrameOptions &options;
};

// Draws one element of a frame (draw_a_frame: its sprite at its offset,
// mirrored and scaled) and grows `b`.
void draw_element(const ElementContext &c, const AnimElement &e, Bounds &b)
{
    Sprite s = c.data.sprites().at(e.sprite / 6);
    int ex = e.x - kElementBiasX;
    int ey = e.y - kElementBiasY;
    unsigned f = (e.flags & 0xf) | c.options.flags;
    // draw_glow_frame: the elements tagged with a layer blend with the glow,
    // the others with colour 0 if their own flags say so.
    const bool layered = (e.flags & 0xfff0) != 0;
    const uint8_t glow = layered && c.options.glow >= 0 ? uint8_t(c.options.glow) : uint8_t(0);
    if (layered && c.options.glow >= 0)
        f |= kDrawGhost;
    if (c.options.scale != 0x10000)
        draw_element_scaled(c.dst, {c.at, {ex, ey}, c.flip, c.options.scale}, s, f, c.data.ghost().data());
    if (c.flip) {
        f ^= kDrawFlipX;
        ex = -ex - s.width;
    }
    if (c.options.scale == 0x10000 && c.options.glow >= 0)
        draw_sprite_glow(c.dst, {c.at.x + ex, c.at.y + ey}, s, f, glow, c.data.ghost().data());
    else if (c.options.scale == 0x10000)
        draw_sprite(c.dst, c.at.x + ex, c.at.y + ey, s, f, c.data.ghost().data());
    b.grow(ex, ey, s.width, s.height);
}

} // namespace

Rect draw_frame(Surface &dst, Point at, uint32_t frame_id, bool flip, const GameData &data, const FrameOptions &options)
{
    const int x = at.x, y = at.y;
    const AnimData &anims = data.anims();
    if (frame_id >= anims.frames.size())
        return {};
    const AnimFrame &frame = anims.frames[frame_id];
    Bounds b;
    Point icon_offset;
    bool have_icon = false;
    int icon_extra = 0;
    const ElementContext ctx{dst, at, flip, data, options};
    for (size_t i = frame.list; i < anims.lists.size() && anims.lists[i] != kListEnd; i++) {
        uint16_t id = anims.lists[i];
        if (id >= anims.elements.size())
            continue;
        const AnimElement &e = anims.elements[id];
        // The status icon hangs over the first head element, drawn there
        // before the remaining elements so that they cover it
        // (draw_special_id_frame tests this before the layer choice, so an
        // element skipped by the layers still anchors it).
        if (!have_icon && options.icon && !options.icon_over_frame && icon_anchor(e, flip, data, icon_offset)) {
            int top_before = b.top;
            draw_icon(dst, data, options.icon, at, icon_offset, b);
            if (b.top < top_before)
                icon_extra = top_before - b.top;
            have_icon = true;
        }
        if (!element_drawn(e, options.layers))
            continue;
        draw_element(ctx, e, b);
    }
    // A machine's icon hangs over its box, after its elements.
    if (options.icon && options.icon_over_frame && !b.empty()) {
        Point offset = {b.left + frame.width / 2, b.top};
        int top_before = b.top;
        draw_icon(dst, data, options.icon, at, offset, b);
        if (b.top < top_before)
            icon_extra = top_before - b.top;
    }
    if (options.extent)
        *options.extent = b.empty() ? Rect{} : Rect{x + b.left, y + b.top, b.right - b.left, b.bottom - b.top};
    if (b.empty())
        return {};
    return {x + b.left, y + b.top, frame.width, frame.height + icon_extra};
}

uint8_t glow_colour(uint32_t turn, const uint8_t *layers)
{
    static constexpr std::array<uint8_t, 8> kGlowPath = {0x4d, 0x4e, 0x4f, 0x50, 0x50, 0x4f, 0x4e, 0x4d};
    return kGlowPath[((turn + layers[0] + layers[1]) >> 1) & 7];
}

} // namespace th

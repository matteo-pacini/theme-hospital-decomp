#include "core/render/frame.h"

#include "core/data/gamedata.h"
#include "core/render/blit.h"

#include <algorithm>
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

} // namespace

Rect draw_frame(Surface &dst, Point at, uint32_t frame_id, bool flip, const GameData &data, const FrameOptions &options)
{
    const int x = at.x, y = at.y;
    const AnimData &anims = data.anims();
    if (frame_id >= anims.frames.size())
        return {};
    const AnimFrame &frame = anims.frames[frame_id];
    int left = INT_MAX, top = INT_MAX, right = INT_MIN, bottom = INT_MIN;
    for (size_t i = frame.list; i < anims.lists.size() && anims.lists[i] != kListEnd; i++) {
        uint16_t id = anims.lists[i];
        if (id >= anims.elements.size())
            continue;
        const AnimElement &e = anims.elements[id];
        if (!element_drawn(e, options.layers))
            continue;
        Sprite s = data.sprites().at(e.sprite / 6);
        int ex = e.x - kElementBiasX;
        int ey = e.y - kElementBiasY;
        unsigned f = (e.flags & 0xf) | options.flags;
        if (options.scale != 0x10000)
            draw_element_scaled(dst, {at, {ex, ey}, flip, options.scale}, s, f, data.ghost().data());
        if (flip) {
            f ^= kDrawFlipX;
            ex = -ex - s.width;
        }
        if (options.scale == 0x10000)
            draw_sprite(dst, x + ex, y + ey, s, f, data.ghost().data());
        left = std::min(left, ex);
        top = std::min(top, ey);
        right = std::max(right, ex + s.width);
        bottom = std::max(bottom, ey + s.height);
    }
    if (options.extent)
        *options.extent = left == INT_MAX ? Rect{} : Rect{x + left, y + top, right - left, bottom - top};
    if (left == INT_MAX)
        return {};
    return {x + left, y + top, frame.width, frame.height};
}

} // namespace th

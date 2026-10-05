#include "core/render/frame.h"

#include "core/data/gamedata.h"
#include "core/render/blit.h"

#include <algorithm>
#include <climits>

namespace th
{

Rect draw_frame(Surface &dst, int x, int y, uint32_t frame_id, bool flip, unsigned flags, const GameData &data,
                const uint8_t *layers, Rect *extent)
{
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
        // The game tests the layer bits with 0xffb0, so an element tagged only
        // 0x40 (layer 4, id 0) is always drawn.
        if (layers && (e.flags & 0xffb0)) {
            int layer = (e.flags >> 4) & 0x1f;
            int choice = e.flags >> 9;
            if (layer >= kLayerCount || layers[layer] != choice)
                continue;
        }
        // Element sprites are byte offsets into the 6-byte TbSprite table;
        // offset 0 (the table's first entry) means no sprite.
        if (e.sprite == 0)
            continue;
        Sprite s = data.sprites().at(e.sprite / 6);
        int ex = e.x - kElementBiasX;
        int ey = e.y - kElementBiasY;
        unsigned f = (e.flags & 0xf) | flags;
        if (flip) {
            f ^= kDrawFlipX;
            ex = -ex - s.width;
        }
        draw_sprite(dst, x + ex, y + ey, s, f, data.ghost().data());
        left = std::min(left, ex);
        top = std::min(top, ey);
        right = std::max(right, ex + s.width);
        bottom = std::max(bottom, ey + s.height);
    }
    if (extent)
        *extent = left == INT_MAX ? Rect{} : Rect{x + left, y + top, right - left, bottom - top};
    if (left == INT_MAX)
        return {};
    return {x + left, y + top, frame.width, frame.height};
}

} // namespace th

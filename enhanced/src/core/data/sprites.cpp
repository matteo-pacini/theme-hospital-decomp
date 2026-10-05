#include "core/data/sprites.h"

namespace th
{

bool SpriteSheet::load(std::vector<uint8_t> tab, std::vector<uint8_t> dat, SpriteFormat format)
{
    format_ = format;
    if (tab.size() % 6 != 0)
        return false;
    sprites_.clear();
    for (size_t i = 0; i < tab.size(); i += 6) {
        Entry e;
        e.offset =
            uint32_t(tab[i]) | uint32_t(tab[i + 1]) << 8 | uint32_t(tab[i + 2]) << 16 | uint32_t(tab[i + 3]) << 24;
        e.width = tab[i + 4];
        e.height = tab[i + 5];
        if (e.offset > dat.size())
            return false;
        sprites_.push_back(e);
    }
    data_ = std::move(dat);
    return true;
}

Sprite SpriteSheet::at(size_t index) const
{
    if (index >= sprites_.size())
        return {};
    const Entry &e = sprites_[index];
    return {data_.data() + e.offset, format_, e.width, e.height};
}

} // namespace th

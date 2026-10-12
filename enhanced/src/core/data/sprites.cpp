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
    Sprite s{data_.data() + e.offset, format_, e.width, e.height, {}};
    if (index < hires_.entries.size() && hires_.entries[index].width > 0) {
        const HiresSheet::Entry &h = hires_.entries[index];
        s.hi = {hires_.data.data() + h.offset, h.width, h.height, hires_.factor};
    }
    return s;
}

void SpriteSheet::attach_hires(HiresSheet hires)
{
    hires_ = std::move(hires);
}

} // namespace th

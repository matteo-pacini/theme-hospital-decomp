#include "core/data/palette.h"

namespace th
{

bool palette_from_vga(std::span<const uint8_t> vga, Palette &out)
{
    if (vga.size() < 768)
        return false;
    for (int i = 0; i < 256; i++) {
        auto c8 = [&](int k) { return uint32_t(((vga[i * 3 + k] & 0x3f) << 2) | ((vga[i * 3 + k] & 0x3f) >> 4)); };
        out[i] = 0xff000000u | c8(0) << 16 | c8(1) << 8 | c8(2);
    }
    return true;
}

} // namespace th

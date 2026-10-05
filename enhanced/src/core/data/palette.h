#pragma once

#include <array>
#include <cstdint>
#include <span>

namespace th
{

// 256 colours as 0xAARRGGBB, opaque.
using Palette = std::array<uint32_t, 256>;

// Converts a VGA palette (256 RGB triples of 6-bit values, as in
// MPALETTE.DAT) to 8 bits per channel. Returns false if `vga` is too short.
bool palette_from_vga(std::span<const uint8_t> vga, Palette &out);

} // namespace th

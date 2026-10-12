#include "core/render/upscale/hires.h"

namespace th
{

namespace
{

void put16(std::vector<uint8_t> &out, size_t v)
{
    out.push_back(uint8_t(v));
    out.push_back(uint8_t(v >> 8));
}

} // namespace

void encode_hires(const IndexedImage &image, std::vector<uint8_t> &out)
{
    for (int y = 0; y < image.height; y++) {
        const uint8_t *row = image.pixels.data() + size_t(y) * size_t(image.width);
        const size_t count_at = out.size();
        put16(out, 0);
        size_t runs = 0;
        size_t col = 0;
        size_t last_end = 0;
        const size_t width = size_t(image.width);
        while (col < width) {
            if (row[col] == kUpscaleTransparent) {
                col++;
                continue;
            }
            size_t end = col;
            while (end < width && row[end] != kUpscaleTransparent)
                end++;
            put16(out, col - last_end);
            put16(out, end - col);
            out.insert(out.end(), row + col, row + end);
            runs++;
            last_end = col = end;
        }
        out[count_at] = uint8_t(runs);
        out[count_at + 1] = uint8_t(runs >> 8);
    }
}

HiresSheet build_hires(const SpriteSheet &sheet, const Palette &palette, const Scaler &scaler, int factor,
                       const std::function<bool(size_t)> &progress)
{
    HiresSheet out;
    out.factor = factor;
    PaletteSnapper snapper(palette);
    out.entries.resize(sheet.size());
    for (size_t i = 0; i < sheet.size(); i++) {
        const IndexedImage image = upscale_sprite(sheet.at(i), palette, scaler, factor, snapper);
        if (image.width != 0) {
            out.entries[i] = {uint32_t(out.data.size()), uint16_t(image.width), uint16_t(image.height)};
            encode_hires(image, out.data);
        }
        if (progress && !progress(i + 1))
            break;
    }
    return out;
}

} // namespace th

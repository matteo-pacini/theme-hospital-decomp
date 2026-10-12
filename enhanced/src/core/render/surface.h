#pragma once

#include <cstdint>
#include <vector>

namespace th
{

struct Rect {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
};

// An 8-bit indexed framebuffer of any size, drawn into as the original drew
// into its 640x480 screen, so palette effects keep working on indices.
//
// Its size, clip and every coordinate the drawing functions take are in game
// pixels. A surface of scale N holds N x N indices for each game pixel: a
// drawing function makes use of the upscaled sprites where it has them and
// otherwise copies each game pixel N x N times. Code that touches the
// indices itself works in pixels (pixel_width, pixel_height, row) or goes
// through put and at.
class Surface
{
  public:
    // Sizes the surface to `width` x `height` game pixels at `scale`.
    void resize(int width, int height, int scale = 1);
    void fill(uint8_t colour);

    // The size in game pixels, and the scale.
    int width() const { return width_; }
    int height() const { return height_; }
    int scale() const { return scale_; }
    // The size in indices.
    int pixel_width() const { return width_ * scale_; }
    int pixel_height() const { return height_ * scale_; }
    // Row `y` of indices, `y` counted in indices (a game pixel's row is
    // y * scale()).
    uint8_t *row(int y) { return pixels_.data() + size_t(y) * size_t(pixel_width()); }
    const uint8_t *row(int y) const { return pixels_.data() + size_t(y) * size_t(pixel_width()); }
    const uint8_t *data() const { return pixels_.data(); }

    // Sets the game pixel (x, y) (inside the surface) to `colour`.
    void put(int x, int y, uint8_t colour);
    // The index at the top-left of the game pixel (x, y).
    uint8_t at(int x, int y) const { return row(y * scale_)[x * scale_]; }

    // Drawing is limited to this rectangle (the whole surface by default), in
    // game pixels.
    void set_clip(Rect clip);
    void reset_clip() { clip_ = {0, 0, width_, height_}; }
    const Rect &clip() const { return clip_; }

  private:
    int width_ = 0;
    int height_ = 0;
    int scale_ = 1;
    std::vector<uint8_t> pixels_;
    Rect clip_;
};

} // namespace th

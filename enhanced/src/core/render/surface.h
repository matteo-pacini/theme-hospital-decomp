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
class Surface
{
  public:
    void resize(int width, int height);
    void fill(uint8_t colour);

    int width() const { return width_; }
    int height() const { return height_; }
    uint8_t *row(int y) { return pixels_.data() + size_t(y) * size_t(width_); }
    const uint8_t *data() const { return pixels_.data(); }

    // Drawing is limited to this rectangle (the whole surface by default).
    void set_clip(Rect clip);
    void reset_clip() { clip_ = {0, 0, width_, height_}; }
    const Rect &clip() const { return clip_; }

  private:
    int width_ = 0;
    int height_ = 0;
    std::vector<uint8_t> pixels_;
    Rect clip_;
};

} // namespace th

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace th
{

// The game's composite animations (DATA/VSTART-1, VFRA-1, VELE-1 and
// VLIST-1.ANI, drawn with the VSPR-0 sprites). An animation (start) names its
// first frame; each frame lists the sprite elements drawn at offsets from a
// common origin and links to the next frame.
struct AnimStart {
    uint32_t frame = 0; // index into frames
    uint32_t flip = 0;  // nonzero: drawn mirrored (copied to Thing.flip)
};

struct AnimFrame {
    uint32_t list = 0; // index into lists of the frame's element ids, ending at 0xffff
    uint8_t width = 0; // bounding box, for mouse hits
    uint8_t height = 0;
    uint8_t sound = 0; // sound played when the frame is drawn, 0 for none
    uint8_t flags = 0; // bit 0: last frame of its animation
    uint16_t next = 0; // next frame (animations loop)
};

struct AnimElement {
    uint16_t sprite = 0; // byte offset of the sprite in VSPR-0.TAB; 0 for none
    uint8_t x = 0;       // offset from the frame's origin, biased by kElementBiasX
    uint8_t y = 0;       // ... and kElementBiasY
    uint16_t flags = 0;  // low nibble: draw flags; above it the person layer bits
};

constexpr int kElementBiasX = 0x8d;
constexpr int kElementBiasY = 0xca;
constexpr uint16_t kListEnd = 0xffff;

class AnimData
{
  public:
    // Loads the four tables from `dir`. On failure says why in `error`.
    bool load(const std::filesystem::path &dir, std::string *error);

    std::vector<AnimStart> starts;
    std::vector<AnimFrame> frames;
    std::vector<AnimElement> elements;
    std::vector<uint16_t> lists;
};

} // namespace th

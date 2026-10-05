#pragma once

#include "core/data/palette.h"
#include "core/render/surface.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace th
{

// The format of one of a Smacker file's seven audio tracks (Smack.TrackType,
// read as soundopen in src/lib/smk/smackinp.c reads it). A track without
// audio has rate 0.
struct SmackerAudioFormat {
    int rate = 0;     // frames per second
    int channels = 0; // 1 or 2
    int bits = 0;     // 8 or 16 in the file; decoded audio is always 16-bit

    bool present() const { return rate != 0; }
};

// A Smacker 2 video (the game's .SMK files, Smacker 2.2k): the header,
// frame tables and Huffman trees read as SmackOpen does, each frame's
// palette, audio chunks and video data split as setuptheframe does, and the
// video and audio decoded as Unsmack and SmackDoPCM (src/lib/smk/unsmack.s,
// assembly only) do.
class SmackerVideo
{
  public:
    // Reads a whole file. Fails, saying why in `error`, if it is not a
    // Smacker 2 file or its tables are damaged. Files whose video is stored
    // at half height (SmackerType bits 1 and 2) are refused: the game has
    // none.
    bool open(std::vector<uint8_t> file, std::string *error = nullptr);
    bool open(const std::filesystem::path &path, std::string *error = nullptr);

    int width() const { return width_; }
    int height() const { return height_; }
    // Frames in a pass through the video (Smack.Frames).
    int frame_count() const { return frames_; }
    // How long each frame shows, in microseconds (from MSPerFrame: positive
    // in milliseconds, negative in hundredths of one).
    int64_t frame_duration_us() const { return frame_us_; }
    SmackerAudioFormat audio_format(int track) const { return audio_formats_.at(size_t(track)); }

    // Decodes the next frame; false after the last frame or if its data is
    // damaged. The first call decodes frame 0.
    bool next_frame();
    // The frame now decoded, -1 before the first.
    int frame_index() const { return frame_; }

    // The frame's pixels, width() x height() palette indices, rows width()
    // apart. Blocks a frame does not change keep the previous frame's.
    const std::vector<uint8_t> &pixels() const { return pixels_; }
    // The palette as the game sets it (the frame's 6-bit values on the VGA).
    const Palette &palette() const { return palette_; }
    // The same, 256 RGB triples of 6-bit values.
    const std::array<uint8_t, 768> &vga_palette() const { return vga_; }
    // This frame carried a palette (Smack.NewPalette).
    bool palette_changed() const { return palette_changed_; }

    // The audio of track `track` that came with this frame: interleaved
    // signed 16-bit samples in the track's format (8-bit tracks scaled up).
    // Empty if the frame has none.
    const std::vector<int16_t> &audio(int track) const { return audio_.at(size_t(track)); }

  private:
    struct Tree {
        // A node is kNode | the size of its left (0 bit) subtree, which
        // follows it; a leaf is its value.
        std::vector<int32_t> nodes{0};
        // The three entries of a 16-bit tree that hold the last values
        // decoded (the escape codes' leaves).
        std::array<size_t, 3> last{};
    };
    class Bits;
    static bool read_byte_tree(Bits &bits, Tree &tree);
    static int build_byte_tree(Bits &bits, Tree &tree, int depth);
    static bool read_big_tree(Bits &bits, Tree &tree, size_t limit);
    static int build_big_tree(Bits &bits, Tree &tree, const Tree &lo, const Tree &hi, const std::array<int, 3> &escapes,
                              std::array<int, 3> &last, size_t limit, int depth);
    static int byte_code(Bits &bits, const Tree &tree);
    static int big_code(Bits &bits, Tree &tree);
    bool decode_video(std::span<const uint8_t> data);
    bool decode_audio(int track, std::span<const uint8_t> data);

    std::vector<uint8_t> file_;
    int width_ = 0, height_ = 0, frames_ = 0;
    int64_t frame_us_ = 0;
    std::array<SmackerAudioFormat, 7> audio_formats_{};
    std::array<uint32_t, 7> track_type_{};
    std::vector<uint32_t> frame_sizes_;
    std::vector<uint8_t> frame_types_;
    size_t first_frame_ = 0;
    Tree mmap_, mclr_, full_, type_;

    int frame_ = -1;
    size_t frame_pos_ = 0;
    std::vector<uint8_t> pixels_;
    std::array<uint8_t, 768> vga_{};
    Palette palette_{};
    bool palette_changed_ = false;
    std::array<std::vector<int16_t>, 7> audio_;
};

// play_smk's flags (AnimTable.play_flags, include/bflib.h).
enum SmackerPlayFlags : unsigned {
    kSmkNoSound = 0x1,
    kSmkNoSkip = 0x2, // Esc, Enter, Space and the left button do not stop it
    kSmkDoubleHeight = 0x4,
    kSmkInterlace = 0x8, // double height, every other row left as it was
    kSmkDoubleWidth = 0x80,
};

// An entry of the game's animation_table (src/files.cpp), what
// hosp_play_smk(index) plays.
struct MovieInfo {
    std::string_view file; // relative to the game data folder
    unsigned flags = 0;    // SmackerPlayFlags
};

// The movies by animation_table index (ANIM_* in include/enums.h). Level L's
// briefing is index L + 7, as play_area_smk plays it.
enum MovieIndex : int {
    kMovieLose1 = 0,
    kMovieWinLevel = 6,
    kMovieArea = 7,
    kMovieMainMenu1 = 0x16,
    kMovieMainMenu2 = 0x17,
    kMovieWinGame = 0x18,
    kMovieIntroSm2 = 0x19,
    kMovieIntroSm4 = 0x1a,
    kMovieAttract = 0x1b,
};
inline constexpr int kMovieCount = 0x1c;

// animation_table[index]; index must be below kMovieCount.
const MovieInfo &movie_info(int index);

// copy_to_screen (src/lib/smk/dossmk.cpp): draws the video's current frame
// centred on `screen`, scaled by `flags`' kSmkDoubleHeight, kSmkInterlace
// and kSmkDoubleWidth. Pixels it does not cover are left as they were.
void copy_frame_to_screen(const SmackerVideo &video, Surface &screen, unsigned flags);

} // namespace th

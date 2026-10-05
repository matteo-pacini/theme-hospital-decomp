#include "core/data/smacker.h"

#include "core/data/fs.h"

#include <cstring>

namespace th
{

namespace
{

constexpr uint32_t kSmk2 = 0x324b4d53; // "SMK2"
constexpr int32_t kNode = int32_t(0x40000000);
constexpr size_t kHeaderSize = 104;
// FFmpeg's limit for the 16-bit trees' depth; the game's files stay well
// under it.
constexpr int kMaxBigDepth = 500;

uint32_t le32(const uint8_t *p)
{
    return uint32_t(p[0]) | uint32_t(p[1]) << 8 | uint32_t(p[2]) << 16 | uint32_t(p[3]) << 24;
}

// Runs of a block type (the type code's bits 2-7), as Unsmack's table.
constexpr std::array<int, 64> kBlockRuns = {
    1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,  17,  18,  19,   20,   21, 22,
    23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,  39,  40,  41,   42,   43, 44,
    45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 128, 256, 512, 1024, 2048,
};

bool fail(std::string *error, std::string message)
{
    if (error)
        *error = std::move(message);
    return false;
}

} // namespace

// A little-endian bit stream: bits are taken from each byte lowest first.
// Reading past the end gives 0 bits and sets overrun().
class SmackerVideo::Bits
{
  public:
    explicit Bits(std::span<const uint8_t> data) : data_(data) {}

    int bit()
    {
        if (pos_ >= data_.size() * 8) {
            overrun_ = true;
            return 0;
        }
        int b = (data_[pos_ >> 3] >> (pos_ & 7)) & 1;
        pos_++;
        return b;
    }
    int bits(int n)
    {
        int v = 0;
        for (int i = 0; i < n; i++)
            v |= bit() << i;
        return v;
    }
    bool overrun() const { return overrun_; }

  private:
    std::span<const uint8_t> data_;
    size_t pos_ = 0;
    bool overrun_ = false;
};

int SmackerVideo::build_byte_tree(Bits &bits, Tree &tree, int depth)
{
    if (depth > 32 || bits.overrun())
        return -1;
    if (!bits.bit()) {
        tree.nodes.push_back(bits.bits(8));
        return 1;
    }
    size_t at = tree.nodes.size();
    tree.nodes.push_back(kNode);
    int left = build_byte_tree(bits, tree, depth + 1);
    if (left < 0)
        return -1;
    tree.nodes[at] = kNode | left;
    int right = build_byte_tree(bits, tree, depth + 1);
    return right < 0 ? -1 : 1 + left + right;
}

// An 8-bit tree: a bit saying whether it is there (an absent tree decodes
// every code as 0 from no bits), the tree, and a 0 bit.
bool SmackerVideo::read_byte_tree(Bits &bits, Tree &tree)
{
    tree.nodes = {0};
    if (!bits.bit())
        return true;
    tree.nodes.clear();
    if (build_byte_tree(bits, tree, 0) < 0)
        return false;
    bits.bit();
    return !bits.overrun();
}

int SmackerVideo::build_big_tree(Bits &bits, Tree &tree, const Tree &lo, const Tree &hi,
                                 const std::array<int, 3> &escapes, std::array<int, 3> &last, size_t limit, int depth)
{
    if (depth > kMaxBigDepth || bits.overrun() || tree.nodes.size() >= limit)
        return -1;
    if (!bits.bit()) {
        int value = byte_code(bits, lo) | byte_code(bits, hi) << 8;
        for (int i = 0; i < 3; i++) {
            if (value == escapes[size_t(i)]) {
                last[size_t(i)] = int(tree.nodes.size());
                value = 0;
                break;
            }
        }
        tree.nodes.push_back(value);
        return 1;
    }
    size_t at = tree.nodes.size();
    tree.nodes.push_back(kNode);
    int left = build_big_tree(bits, tree, lo, hi, escapes, last, limit, depth + 1);
    if (left < 0)
        return -1;
    tree.nodes[at] = kNode | left;
    int right = build_big_tree(bits, tree, lo, hi, escapes, last, limit, depth + 1);
    return right < 0 ? -1 : 1 + left + right;
}

// A 16-bit tree of the header (MMAP, MCLR, FULL, TYPE): a bit saying
// whether it is there, its low and high byte trees, three 16-bit escape
// codes, the tree (leaves coded with the byte trees; the leaves holding an
// escape code become the three most recent values), and a 0 bit.
bool SmackerVideo::read_big_tree(Bits &bits, Tree &tree, size_t limit)
{
    tree.nodes = {0, 0};
    tree.last = {1, 1, 1};
    if (!bits.bit())
        return true;
    Tree lo, hi;
    if (!read_byte_tree(bits, lo) || !read_byte_tree(bits, hi))
        return false;
    std::array<int, 3> escapes{};
    for (int &e : escapes)
        e = bits.bits(16);
    std::array<int, 3> last = {-1, -1, -1};
    tree.nodes.clear();
    if (build_big_tree(bits, tree, lo, hi, escapes, last, limit, 0) < 0)
        return false;
    bits.bit();
    for (size_t i = 0; i < 3; i++) {
        if (last[i] < 0) {
            last[i] = int(tree.nodes.size());
            tree.nodes.push_back(0);
        }
        tree.last[i] = size_t(last[i]);
    }
    return !bits.overrun();
}

int SmackerVideo::byte_code(Bits &bits, const Tree &tree)
{
    size_t i = 0;
    while (tree.nodes[i] & kNode) {
        if (bits.bit())
            i += size_t(tree.nodes[i] & ~kNode);
        i++;
    }
    return tree.nodes[i];
}

int SmackerVideo::big_code(Bits &bits, Tree &tree)
{
    int v = byte_code(bits, tree);
    std::vector<int32_t> &n = tree.nodes;
    if (v != n[tree.last[0]]) {
        n[tree.last[2]] = n[tree.last[1]];
        n[tree.last[1]] = n[tree.last[0]];
        n[tree.last[0]] = v;
    }
    return v;
}

bool SmackerVideo::open(const std::filesystem::path &path, std::string *error)
{
    auto file = read_file(path);
    if (!file)
        return fail(error, path.string() + ": cannot be read");
    return open(std::move(*file), error);
}

bool SmackerVideo::open(std::vector<uint8_t> file, std::string *error)
{
    *this = SmackerVideo{};
    file_ = std::move(file);
    if (file_.size() < kHeaderSize || le32(file_.data()) != kSmk2)
        return fail(error, "not a Smacker 2 file");
    const uint8_t *h = file_.data();
    width_ = int(le32(h + 4));
    height_ = int(le32(h + 8));
    frames_ = int(le32(h + 12));
    int32_t ms_per_frame = int32_t(le32(h + 16));
    uint32_t type = le32(h + 20);
    uint32_t table_size = le32(h + 52);
    std::array<uint32_t, 4> tree_sizes = {le32(h + 56), le32(h + 60), le32(h + 64), le32(h + 68)};
    if (width_ <= 0 || height_ <= 0 || width_ > 4096 || height_ > 4096 || frames_ <= 0 || frames_ > 100000)
        return fail(error, "bad Smacker header");
    if (type & 6)
        return fail(error, "Smacker video stored at half height");

    // ms100perframe in SmackOpen: hundredths of a millisecond.
    int64_t ms100 = ms_per_frame < 0 ? -int64_t(ms_per_frame) : int64_t(ms_per_frame) * 100;
    frame_us_ = ms100 * 10;
    for (int t = 0; t < 7; t++) {
        uint32_t tt = le32(h + 72 + t * 4);
        track_type_[size_t(t)] = tt;
        if ((tt >> 24) & 0x40) {
            SmackerAudioFormat &f = audio_formats_[size_t(t)];
            f.rate = int(tt & 0xffffff);
            f.channels = (tt >> 28) & 1 ? 2 : 1;
            f.bits = (tt >> 29) & 1 ? 16 : 8;
        }
    }

    // The ring frame (SmackerType bit 0) follows the last.
    size_t n = size_t(frames_) + (type & 1);
    size_t pos = kHeaderSize;
    if (file_.size() < pos + n * 5 + table_size)
        return fail(error, "Smacker file cut short");
    frame_sizes_.resize(n);
    for (size_t i = 0; i < n; i++)
        frame_sizes_[i] = le32(h + pos + i * 4) & ~3u; // the low bits are flags, not size
    pos += n * 4;
    frame_types_.assign(file_.begin() + ptrdiff_t(pos), file_.begin() + ptrdiff_t(pos + n));
    pos += n;

    Bits bits({file_.data() + pos, table_size});
    Tree *trees[4] = {&mmap_, &mclr_, &full_, &type_};
    for (int i = 0; i < 4; i++) {
        if (!read_big_tree(bits, *trees[i], (tree_sizes[size_t(i)] + 3) / 4 + 3))
            return fail(error, "damaged Smacker tree");
    }
    first_frame_ = pos + table_size;

    size_t total = 0;
    for (uint32_t s : frame_sizes_)
        total += s;
    if (file_.size() < first_frame_ + total)
        return fail(error, "Smacker file cut short");

    pixels_.assign(size_t(width_) * size_t(height_), 0);
    frame_pos_ = first_frame_;
    return true;
}

bool SmackerVideo::next_frame()
{
    if (frame_ + 1 >= frames_)
        return false;
    frame_++;
    for (auto &a : audio_)
        a.clear();
    palette_changed_ = false;

    const uint8_t *data = file_.data() + frame_pos_;
    size_t size = frame_sizes_[size_t(frame_)];
    frame_pos_ += size;
    uint8_t type = frame_types_[size_t(frame_)];
    size_t pos = 0;

    if (type & 1) {
        size_t chunk = size_t(data[0]) * 4;
        if (chunk == 0 || chunk > size)
            return false;
        // setuptheframe's palette chunk: 0x80 + n keeps n + 1 colours, 0x40 + n
        // copies n + 1 colours of the previous palette from the next byte's
        // index, anything else is a colour (three 6-bit values).
        std::array<uint8_t, 768> old = vga_;
        size_t src = 1;
        int i = 0;
        while (i < 256) {
            if (src >= chunk)
                return false;
            uint8_t c = data[src];
            if (c & 0x80) {
                int keep = (c & 0x7f) + 1;
                if (i + keep > 256)
                    return false;
                std::memcpy(&vga_[size_t(i) * 3], &old[size_t(i) * 3], size_t(keep) * 3);
                i += keep;
                src++;
            } else if (c & 0x40) {
                int copy = (c & 0x3f) + 1;
                if (src + 1 >= chunk)
                    return false;
                int from = data[src + 1];
                if (i + copy > 256 || from + copy > 256)
                    return false;
                std::memcpy(&vga_[size_t(i) * 3], &old[size_t(from) * 3], size_t(copy) * 3);
                i += copy;
                src += 2;
            } else {
                if (src + 2 >= chunk)
                    return false;
                vga_[size_t(i) * 3] = c & 0x3f;
                vga_[size_t(i) * 3 + 1] = data[src + 1] & 0x3f;
                vga_[size_t(i) * 3 + 2] = data[src + 2] & 0x3f;
                i++;
                src += 3;
            }
        }
        palette_from_vga(vga_, palette_);
        palette_changed_ = true;
        pos = chunk;
    }
    for (int t = 0; t < 7; t++) {
        if (!(type & (2 << t)))
            continue;
        if (pos + 4 > size)
            return false;
        size_t len = le32(data + pos);
        if (len < 4 || pos + len > size)
            return false;
        if (audio_formats_[size_t(t)].present() && !decode_audio(t, {data + pos + 4, len - 4}))
            return false;
        pos += len;
    }
    return decode_video({data + pos, size - pos});
}

bool SmackerVideo::decode_video(std::span<const uint8_t> data)
{
    Bits bits(data);
    for (Tree *t : {&mmap_, &mclr_, &full_, &type_}) {
        for (size_t i : t->last)
            t->nodes[i] = 0;
    }
    int bw = width_ / 4;
    int blocks = bw * (height_ / 4);
    int block = 0;
    while (block < blocks) {
        int type = big_code(bits, type_);
        int run = kBlockRuns[size_t((type >> 2) & 0x3f)];
        switch (type & 3) {
        case 0: // two colours, chosen per pixel by a 16-bit map
            for (; run > 0 && block < blocks; run--, block++) {
                int colours = big_code(bits, mclr_);
                int map = big_code(bits, mmap_);
                uint8_t hi = uint8_t(colours >> 8), lo = uint8_t(colours);
                uint8_t *out = pixels_.data() + size_t(block / bw) * 4 * size_t(width_) + size_t(block % bw) * 4;
                for (int y = 0; y < 4; y++, out += width_) {
                    for (int x = 0; x < 4; x++, map >>= 1)
                        out[x] = map & 1 ? hi : lo;
                }
            }
            break;
        case 1: // every pixel, two at a time, right pair first
            for (; run > 0 && block < blocks; run--, block++) {
                uint8_t *out = pixels_.data() + size_t(block / bw) * 4 * size_t(width_) + size_t(block % bw) * 4;
                for (int y = 0; y < 4; y++, out += width_) {
                    int right = big_code(bits, full_);
                    int left = big_code(bits, full_);
                    out[0] = uint8_t(left);
                    out[1] = uint8_t(left >> 8);
                    out[2] = uint8_t(right);
                    out[3] = uint8_t(right >> 8);
                }
            }
            break;
        case 2: // unchanged
            block += run;
            break;
        case 3: // one colour, the type code's high byte
            for (; run > 0 && block < blocks; run--, block++) {
                uint8_t *out = pixels_.data() + size_t(block / bw) * 4 * size_t(width_) + size_t(block % bw) * 4;
                for (int y = 0; y < 4; y++, out += width_)
                    std::memset(out, type >> 8, 4);
            }
            break;
        }
        if (bits.overrun())
            return false;
    }
    return true;
}

bool SmackerVideo::decode_audio(int track, std::span<const uint8_t> data)
{
    std::vector<int16_t> &out = audio_[size_t(track)];
    const SmackerAudioFormat &f = audio_formats_[size_t(track)];
    if (!(track_type_[size_t(track)] & 0x80000000u)) {
        if (f.bits == 16) {
            for (size_t i = 0; i + 1 < data.size(); i += 2)
                out.push_back(int16_t(data[i] | data[i + 1] << 8));
        } else {
            for (uint8_t v : data)
                out.push_back(int16_t((v - 128) * 256));
        }
        return true;
    }
    // Compressed: the unpacked size, then a bit for any data, a bit for
    // stereo and one for 16-bit, a byte tree per channel (two per channel
    // for 16-bit, low byte first), each channel's first sample (16-bit ones
    // high byte first, the right channel's before the left's), then the
    // differences from the previous sample of the same channel.
    if (data.size() < 4)
        return false;
    size_t unpacked = le32(data.data());
    Bits bits(data.subspan(4));
    if (!bits.bit())
        return true;
    int stereo = bits.bit();
    int wide = bits.bit();
    if (stereo != (f.channels == 2) || wide != (f.bits == 16))
        return false;
    std::array<Tree, 4> trees;
    for (int i = 0; i < 1 << (wide + stereo); i++) {
        if (!read_byte_tree(bits, trees[size_t(i)]))
            return false;
    }
    int channels = stereo + 1;
    size_t samples = wide ? unpacked / 2 : unpacked;
    out.reserve(samples);
    std::array<int, 2> pred{};
    if (wide) {
        for (int c = stereo; c >= 0; c--) {
            int hi = bits.bits(8);
            pred[size_t(c)] = int16_t(hi << 8 | bits.bits(8));
        }
    } else {
        for (int c = stereo; c >= 0; c--)
            pred[size_t(c)] = bits.bits(8);
    }
    for (size_t i = 0; i < samples; i++) {
        int c = int(i) % channels;
        if (i >= size_t(channels)) {
            if (wide) {
                int lo = byte_code(bits, trees[size_t(c) * 2]);
                int hi = byte_code(bits, trees[size_t(c) * 2 + 1]);
                pred[size_t(c)] = int16_t(pred[size_t(c)] + int16_t(lo | hi << 8));
            } else {
                pred[size_t(c)] = uint8_t(pred[size_t(c)] + int8_t(byte_code(bits, trees[size_t(c)])));
            }
        }
        out.push_back(wide ? int16_t(pred[size_t(c)]) : int16_t((pred[size_t(c)] - 128) * 256));
    }
    return !bits.overrun();
}

namespace
{

// animation_table (src/files.cpp): file and play_smk flags.
constexpr std::array<MovieInfo, kMovieCount> kMovies = {{
    {"anims/lose1.smk", kSmkNoSkip | kSmkInterlace | kSmkDoubleWidth},
    {"anims/lose2.smk", kSmkNoSkip | kSmkInterlace | kSmkDoubleWidth},
    {"anims/lose3.smk", kSmkNoSkip | kSmkInterlace | kSmkDoubleWidth},
    {"anims/lose4.smk", kSmkNoSkip | kSmkInterlace | kSmkDoubleWidth},
    {"anims/lose5.smk", kSmkNoSkip | kSmkInterlace | kSmkDoubleWidth},
    {"anims/lose6.smk", kSmkNoSkip | kSmkInterlace | kSmkDoubleWidth},
    {"anims/winlevel.smk", kSmkNoSkip | kSmkDoubleHeight | kSmkDoubleWidth},
    {"anims/area01v.smk", kSmkNoSound},
    {"anims/area02v.smk", kSmkNoSound},
    {"anims/area02v.smk", kSmkNoSound},
    {"anims/area03v.smk", kSmkNoSound},
    {"anims/area04v.smk", kSmkNoSound},
    {"anims/area05v.smk", kSmkNoSound},
    {"anims/area06v.smk", kSmkNoSound},
    {"anims/area07v.smk", kSmkNoSound},
    {"anims/area08v.smk", kSmkNoSound},
    {"anims/area09v.smk", kSmkNoSound},
    {"anims/area10v.smk", kSmkNoSound},
    {"anims/area11v.smk", kSmkNoSound},
    {"anims/area12v.smk", kSmkNoSound},
    {"anims/area13v.smk", kSmkNoSound},
    {"anims/area14v.smk", kSmkNoSound},
    {"anims/mmenu1-0.smk", 0},
    {"anims/mmenu2-0.smk", 0},
    {"anims/wingame.smk", kSmkNoSkip | kSmkDoubleHeight | kSmkDoubleWidth},
    {"intro/intro.sm2", 0},
    {"intro/intro.sm4", 0},
    {"intro/attract.smk", 0},
}};

} // namespace

const MovieInfo &movie_info(int index)
{
    return kMovies.at(size_t(index));
}

void copy_frame_to_screen(const SmackerVideo &video, Surface &screen, unsigned flags)
{
    int w = video.width(), h = video.height();
    bool tall = flags & (kSmkDoubleHeight | kSmkInterlace);
    bool wide = flags & kSmkDoubleWidth;
    int out_w = wide ? w * 2 : w;
    int x0 = (screen.width() - out_w) >> 1;
    int y0 = (screen.height() - (tall ? h * 2 : h)) >> 1;
    const uint8_t *src = video.pixels().data();
    for (int y = 0; y < h; y++, src += w) {
        int rows = flags & kSmkDoubleHeight ? 2 : 1;
        int dy = y0 + (tall ? y * 2 : y);
        for (int r = 0; r < rows; r++) {
            if (dy + r < 0 || dy + r >= screen.height())
                continue;
            uint8_t *dst = screen.row(dy + r);
            for (int x = 0; x < out_w; x++) {
                int dx = x0 + x;
                if (dx >= 0 && dx < screen.width())
                    dst[dx] = src[wide ? x / 2 : x];
            }
        }
    }
}

} // namespace th

#include "core/data/rnc.h"

#include <array>

// RNC method 1, as unpacked by the game's UnpackM1 (src/asm/rnc_1fm.s): an
// 18-byte header, then per chunk three Huffman tables (literal run length,
// match distance, match length) and a count of literal-run/match pairs. The
// bit stream is read 16 bits at a time, little-endian, and literal runs are
// copied straight from the byte stream in between.

namespace th
{

namespace
{

constexpr size_t kHeaderSize = 18;

uint32_t be32(const uint8_t *p)
{
    return uint32_t(p[0]) << 24 | uint32_t(p[1]) << 16 | uint32_t(p[2]) << 8 | p[3];
}

uint16_t be16(const uint8_t *p)
{
    return uint16_t(p[0] << 8 | p[1]);
}

uint16_t rnc_crc(std::span<const uint8_t> data)
{
    static const std::array<uint16_t, 256> table = [] {
        std::array<uint16_t, 256> t{};
        for (uint16_t i = 0; i < 256; i++) {
            uint16_t v = i;
            for (int j = 0; j < 8; j++)
                v = (v & 1) ? (v >> 1) ^ 0xa001 : v >> 1;
            t[i] = v;
        }
        return t;
    }();
    uint16_t crc = 0;
    for (uint8_t b : data) {
        crc ^= b;
        crc = (crc >> 8) ^ table[crc & 0xff];
    }
    return crc;
}

// Bit reader over the packed stream. `pos` is the byte offset of the 16-bit
// word most recently loaded into the top of `bits`; literal runs are copied
// from there, after which copy_literal() reloads the buffer behind them.
class BitStream
{
  public:
    BitStream(std::span<const uint8_t> src, size_t pos) : src_(src), pos_(pos)
    {
        bits_ = word(pos_);
        count_ = 16;
    }

    uint32_t peek(uint32_t mask) const { return bits_ & mask; }

    void advance(int n)
    {
        bits_ >>= n;
        count_ -= n;
        if (count_ < 16) {
            pos_ += 2;
            bits_ |= word(pos_) << count_;
            count_ += 16;
        }
    }

    uint32_t read(uint32_t mask, int n)
    {
        uint32_t v = peek(mask);
        advance(n);
        return v;
    }

    // Copies a literal run of `len` bytes from the byte stream. Returns false
    // if it runs past the input.
    bool copy_literal(std::vector<uint8_t> &out, size_t len)
    {
        if (pos_ + len > src_.size())
            return false;
        out.insert(out.end(), src_.begin() + pos_, src_.begin() + pos_ + len);
        pos_ += len;
        count_ -= 16;
        bits_ &= (1u << count_) - 1;
        bits_ |= word(pos_) << count_;
        count_ += 16;
        return true;
    }

  private:
    // Reads past the end as zeros: the last chunk's final bits may sit in a
    // partial word.
    uint32_t word(size_t at) const
    {
        uint32_t lo = at < src_.size() ? src_[at] : 0;
        uint32_t hi = at + 1 < src_.size() ? src_[at + 1] : 0;
        return lo | hi << 8;
    }

    std::span<const uint8_t> src_;
    size_t pos_;
    uint32_t bits_;
    int count_;
};

struct HuffTable {
    struct Entry {
        uint32_t code;
        int length;
        int value;
    };
    std::array<Entry, 32> entries{};
    int count = 0;

    static uint32_t mirror(uint32_t x, int n)
    {
        uint32_t r = 0;
        for (int i = 0; i < n; i++, x >>= 1)
            r = r << 1 | (x & 1);
        return r;
    }

    void read(BitStream &bs)
    {
        int num = static_cast<int>(bs.read(0x1f, 5));
        count = 0;
        if (num == 0)
            return;
        std::array<int, 32> lengths{};
        int max_len = 1;
        for (int i = 0; i < num; i++) {
            lengths[i] = static_cast<int>(bs.read(0x0f, 4));
            if (lengths[i] > max_len)
                max_len = lengths[i];
        }
        uint32_t code = 0;
        for (int len = 1; len <= max_len; len++) {
            for (int j = 0; j < num; j++) {
                if (lengths[j] == len)
                    entries[count++] = {mirror(code++, len), len, j};
            }
            code <<= 1;
        }
    }

    // Decodes one value: the symbol's index selects how many extra bits
    // follow (index 0 and 1 are the values themselves). -1 if no code
    // matches.
    long decode(BitStream &bs) const
    {
        for (int i = 0; i < count; i++) {
            const Entry &e = entries[i];
            if (bs.peek((1u << e.length) - 1) == e.code) {
                bs.advance(e.length);
                long v = e.value;
                if (v >= 2) {
                    long base = 1L << (v - 1);
                    v = base | static_cast<long>(bs.read(static_cast<uint32_t>(base - 1), e.value - 1));
                }
                return v;
            }
        }
        return -1;
    }
};

bool fail(std::string *error, const char *why)
{
    if (error)
        *error = why;
    return false;
}

} // namespace

bool rnc_is_packed(std::span<const uint8_t> in)
{
    return in.size() >= kHeaderSize && in[0] == 'R' && in[1] == 'N' && in[2] == 'C' && in[3] == 1;
}

std::optional<std::vector<uint8_t>> rnc_unpack(std::span<const uint8_t> in, std::string *error)
{
    if (!rnc_is_packed(in)) {
        fail(error, "not RNC method 1 data");
        return std::nullopt;
    }
    uint32_t unpacked_size = be32(&in[4]);
    uint32_t packed_size = be32(&in[8]);
    uint16_t unpacked_crc = be16(&in[12]);
    uint16_t packed_crc = be16(&in[14]);
    int chunks = in[17];
    if (kHeaderSize + packed_size > in.size()) {
        fail(error, "truncated RNC data");
        return std::nullopt;
    }
    std::span<const uint8_t> packed = in.subspan(kHeaderSize, packed_size);
    if (rnc_crc(packed) != packed_crc) {
        fail(error, "RNC packed data CRC mismatch");
        return std::nullopt;
    }

    std::vector<uint8_t> out;
    out.reserve(unpacked_size);
    BitStream bs(packed, 0);
    bs.advance(2); // lock and key flags, unused by the game's files
    HuffTable literal, distance, length;
    for (int chunk = 0; chunk < chunks; chunk++) {
        literal.read(bs);
        distance.read(bs);
        length.read(bs);
        long pairs = static_cast<long>(bs.read(0xffff, 16));
        for (;;) {
            long run = literal.decode(bs);
            if (run < 0) {
                fail(error, "bad RNC literal code");
                return std::nullopt;
            }
            if (run > 0 && !bs.copy_literal(out, static_cast<size_t>(run))) {
                fail(error, "RNC literal run past end of input");
                return std::nullopt;
            }
            if (--pairs <= 0)
                break;
            long dist = distance.decode(bs);
            long len = length.decode(bs);
            if (dist < 0 || len < 0) {
                fail(error, "bad RNC match code");
                return std::nullopt;
            }
            dist += 1;
            len += 2;
            if (static_cast<size_t>(dist) > out.size()) {
                fail(error, "RNC match before start of output");
                return std::nullopt;
            }
            for (long i = 0; i < len; i++)
                out.push_back(out[out.size() - dist]);
        }
    }
    if (out.size() != unpacked_size) {
        fail(error, "RNC unpacked size mismatch");
        return std::nullopt;
    }
    if (rnc_crc(out) != unpacked_crc) {
        fail(error, "RNC unpacked data CRC mismatch");
        return std::nullopt;
    }
    return out;
}

} // namespace th

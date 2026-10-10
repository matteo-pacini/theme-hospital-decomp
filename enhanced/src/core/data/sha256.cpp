#include "core/data/sha256.h"

#include <algorithm>
#include <cstring>

namespace th
{

namespace
{

constexpr uint32_t kRound[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

uint32_t rotr(uint32_t x, int n)
{
    return x >> n | x << (32 - n);
}

} // namespace

Sha256::Sha256()
    : state_{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19}
{
}

void Sha256::compress(const uint8_t *block)
{
    uint32_t w[64];
    for (int i = 0; i < 16; i++)
        w[i] = uint32_t(block[i * 4]) << 24 | uint32_t(block[i * 4 + 1]) << 16 | uint32_t(block[i * 4 + 2]) << 8 |
               uint32_t(block[i * 4 + 3]);
    for (int i = 16; i < 64; i++) {
        uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
        uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    uint32_t a = state_[0], b = state_[1], c = state_[2], d = state_[3];
    uint32_t e = state_[4], f = state_[5], g = state_[6], h = state_[7];
    for (int i = 0; i < 64; i++) {
        uint32_t t1 = h + (rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25)) + ((e & f) ^ (~e & g)) + kRound[i] + w[i];
        uint32_t t2 = (rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }
    state_[0] += a;
    state_[1] += b;
    state_[2] += c;
    state_[3] += d;
    state_[4] += e;
    state_[5] += f;
    state_[6] += g;
    state_[7] += h;
}

void Sha256::update(const void *data, size_t size)
{
    const uint8_t *p = static_cast<const uint8_t *>(data);
    length_ += size;
    if (buffered_) {
        size_t take = std::min(size, buffer_.size() - buffered_);
        std::memcpy(buffer_.data() + buffered_, p, take);
        buffered_ += take;
        p += take;
        size -= take;
        if (buffered_ < buffer_.size())
            return;
        compress(buffer_.data());
        buffered_ = 0;
    }
    for (; size >= 64; p += 64, size -= 64)
        compress(p);
    std::memcpy(buffer_.data(), p, size);
    buffered_ = size;
}

Sha256::Digest Sha256::final()
{
    uint64_t bits = length_ * 8;
    uint8_t pad[72] = {0x80};
    // Zero padding up to 56 mod 64, then the length in bits, big endian.
    size_t zeros = (buffered_ < 56 ? 56 : 120) - buffered_;
    for (int i = 0; i < 8; i++)
        pad[zeros + i] = uint8_t(bits >> (56 - 8 * i));
    update(pad, zeros + 8);

    Digest digest;
    for (size_t i = 0; i < 8; i++)
        for (size_t j = 0; j < 4; j++)
            digest[i * 4 + j] = uint8_t(state_[i] >> (24 - 8 * j));
    return digest;
}

std::string to_hex(const Sha256::Digest &digest)
{
    static const char kDigits[] = "0123456789abcdef";
    std::string hex;
    for (uint8_t b : digest) {
        hex += kDigits[b >> 4];
        hex += kDigits[b & 15];
    }
    return hex;
}

} // namespace th

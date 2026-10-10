#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace th
{

// SHA-256 (FIPS 180-4), fed in pieces: construct, update() as often as
// needed, then final() once.
class Sha256
{
  public:
    using Digest = std::array<uint8_t, 32>;

    Sha256();

    void update(const void *data, size_t size);

    // Ends the message and returns its digest. The object is not reusable.
    Digest final();

  private:
    void compress(const uint8_t *block);

    std::array<uint32_t, 8> state_;
    std::array<uint8_t, 64> buffer_{};
    size_t buffered_ = 0;
    uint64_t length_ = 0;
};

// The digest as 64 lower-case hex digits.
std::string to_hex(const Sha256::Digest &digest);

} // namespace th

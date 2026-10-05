#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace th
{

// True if `in` starts with a Rob Northen Compression method 1 header.
bool rnc_is_packed(std::span<const uint8_t> in);

// Unpacks RNC method 1 data, checking the sizes and both CRCs in the header.
// On failure returns nothing and, if `error` is given, says why.
std::optional<std::vector<uint8_t>> rnc_unpack(std::span<const uint8_t> in, std::string *error = nullptr);

} // namespace th

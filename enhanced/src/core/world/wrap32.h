#pragma once

#include <cstdint>

namespace th
{

// The original's 32-bit int arithmetic (Watcom): a sum or product that leaves
// the range wraps modulo 2^32 where C++ signed overflow is undefined.
inline int32_t add32(int32_t a, int32_t b)
{
    return int32_t(uint32_t(a) + uint32_t(b));
}

// `v << n` with the count taken modulo 32, as the CPU's shift does (a count of
// 32 or more is undefined in C++); owner 99 reaches it as `1 << owner`.
inline int32_t shl32(int32_t v, unsigned n)
{
    return int32_t(uint32_t(v) << (n & 31));
}

inline int32_t mul32(int32_t a, int32_t b)
{
    return int32_t(uint32_t(a) * uint32_t(b));
}

} // namespace th

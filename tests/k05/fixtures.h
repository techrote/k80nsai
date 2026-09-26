#pragma once
#include "q1_reference.h"
#include <cstring>
#include <iomanip>
#include <sstream>
#include <string>

namespace k05_fixtures {
namespace ref = k80nsai::q1_reference;
constexpr std::uint32_t default_seed = 0x004b3035;
// Defined uint32 arithmetic, independent of standard-library distributions.
struct Random {
    std::uint32_t state;
    std::uint32_t next() {
        state ^= state << 13; state ^= state >> 17; state ^= state << 5;
        return state;
    }
    ref::Group activation() {
        ref::Group x{};
        for (float & value : x) value = float(int(next() % 8193) - 4096) / 127.0f;
        return x;
    }
    ref::Mask mask() {
        ref::Mask m{};
        for (auto & value : m) value = std::uint8_t(next() & 255);
        return m;
    }
};
inline ref::Mask filled(std::uint8_t byte) {
    ref::Mask m{}; m.fill(byte); return m;
}
inline ref::Group repeat(std::initializer_list<float> values) {
    ref::Group x{};
    for (std::size_t i = 0; i < x.size(); ++i) x[i] = values.begin()[i % values.size()];
    return x;
}
inline void put(std::vector<std::uint8_t> & bytes, std::size_t offset,
                std::uint16_t half, const ref::Mask & mask) {
    std::memcpy(bytes.data() + offset, &half, 2); // native host byte order
    for (std::size_t i = 0; i < mask.size(); ++i) bytes[offset + 2 + i] = mask[i];
}
inline std::vector<std::uint8_t> block(std::uint16_t half, const ref::Mask & mask) {
    std::vector<std::uint8_t> bytes(18); put(bytes, 0, half, mask); return bytes;
}
inline ref::NativeView view(const std::vector<std::uint8_t> & b) {
    return {b.data(), b.size(), 0, 1, 128, 18};
}
inline std::string dump(const ref::NativeView & v, std::size_t row,
                        const std::vector<ref::Group> & activation) {
    std::ostringstream out;
    out << "row=" << row << " offset=" << v.offset << " rows=" << v.rows
        << " columns=" << v.columns << " stride=" << v.row_stride_bytes
        << " storage_bytes=" << v.storage_bytes << '\n';
    for (std::size_t g = 0; g < activation.size(); ++g) {
        const auto start = v.offset + row * v.row_stride_bytes + g * 18;
        std::uint16_t half;
        std::memcpy(&half, v.storage + start, 2);
        out << "group=" << g << " half_bits=0x" << std::hex << half << " block_bytes=";
        for (std::size_t b = 0; b < 18; ++b)
            out << std::setw(2) << std::setfill('0') << unsigned(v.storage[start + b]);
        out << std::dec << "\nx[" << g << "]=" << std::hexfloat;
        for (float x : activation[g]) out << x << ',';
        out << std::defaultfloat << '\n';
    }
    return out.str();
}
} // namespace k05_fixtures
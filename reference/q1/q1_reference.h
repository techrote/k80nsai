#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

// Host mathematics only. This is not K08's runtime/device descriptor.
namespace k80nsai::q1_reference {
constexpr std::size_t group_size = 128;
constexpr std::size_t native_group_bytes = 18;
using Group = std::array<float, group_size>;
using Mask = std::array<std::uint8_t, group_size / 8>; // bit 1 = +1, LSB first

enum class ErrorCode { layout, activation, scale, arithmetic, depth, threshold, energy_underflow };
class Error : public std::runtime_error {
public:
    Error(ErrorCode code, const char * message);
    ErrorCode code() const noexcept { return code_; }
private:
    ErrorCode code_;
};

// Native in-memory Q1 only: half is a host-endian uint16_t storage object.
// Arbitrary byte offsets and row padding are accepted; groups remain contiguous.
// Caller guarantees that storage really addresses storage_bytes readable bytes.
enum class NativeFormat { q1_0, unsupported };
struct NativeView {
    const std::uint8_t * storage;
    std::size_t storage_bytes;
    std::size_t offset;
    std::size_t rows;
    std::size_t columns;
    std::size_t row_stride_bytes;
    std::size_t group_stride_bytes = native_group_bytes;
    NativeFormat format = NativeFormat::q1_0;
};

// Finite IEEE binary16 only; signed zero and subnormals are preserved exactly.
float half_to_float(std::uint16_t bits);
void validate(const NativeView & view);
Group decode_group(const NativeView & view, std::size_t row, std::size_t group);
std::vector<float> decode_row(const NativeView & view, std::size_t row);
Mask weight_mask(const NativeView & view, std::size_t row, std::size_t group);
float weight_scale(const NativeView & view, std::size_t row, std::size_t group);

struct Plane {
    float alpha = 0;
    Mask signs{};
};
struct Basis {
    std::array<Plane, 3> planes{};
    Group residual{}; // final residual, diagnostic only; never used to reconstruct
    unsigned computed_depth = 0;
    unsigned consumed_depth = 0;
};
struct Adaptive {
    Basis basis;
    float energy0 = 0;
    std::array<float, 2> residual_ratio{};
    unsigned evaluated_stops = 0; // only these ratio entries were evaluated
};

Basis encode_fixed(const Group & x, unsigned depth);
Adaptive encode_adaptive(const Group & x, float tau1, float tau2);
Group reconstruct(const Basis & basis);
int mask_dot(const Mask & a, const Mask & b); // exact [-128,128]

// Original activation and reconstructed routes accumulate element by element
// across the whole row. Mask route accumulates scaled group outputs.
float dot_original(const NativeView & view, std::size_t row,
                   const float * activation, std::size_t count);
float dot_reconstructed(const NativeView & view, std::size_t row,
                        const std::vector<Basis> & groups);
float dot_masks(const NativeView & view, std::size_t row,
                const std::vector<Basis> & groups);
} // namespace k80nsai::q1_reference

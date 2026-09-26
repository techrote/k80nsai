#include "q1_reference.h"

#include <cfenv>
#include <cmath>
#include <cstring>
#include <limits>

namespace k80nsai::q1_reference {
static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559 &&
              std::numeric_limits<float>::digits == 24, "IEEE binary32 required");
static_assert(sizeof(std::uint16_t) == 2, "16-bit half storage required");

Error::Error(ErrorCode code, const char * message) : std::runtime_error(message), code_(code) {}
namespace {
void require(bool condition, ErrorCode code, const char * message) {
    if (!condition) throw Error(code, message);
}
float finite(float value) {
    require(std::isfinite(value), ErrorCode::arithmetic, "non-finite FP32 intermediate/output");
    return value;
}
void environment() {
    require(std::fegetround() == FE_TONEAREST, ErrorCode::arithmetic, "round-to-nearest required");
}
void activation_check(const float * x, std::size_t count) {
    require(x != nullptr, ErrorCode::activation, "null FP32 activation");
    for (std::size_t i = 0; i < count; ++i)
        require(std::isfinite(x[i]), ErrorCode::activation, "non-finite FP32 activation");
    environment();
}
const std::uint8_t * block(const NativeView & v, std::size_t row, std::size_t group) {
    validate(v);
    require(row < v.rows && group < v.columns / group_size, ErrorCode::layout, "row/group out of range");
    return v.storage + v.offset + row * v.row_stride_bytes + group * native_group_bytes;
}
float scale(const std::uint8_t * bytes) {
    std::uint16_t bits;
    std::memcpy(&bits, bytes, sizeof(bits)); // unaligned-safe native storage load
    return half_to_float(bits);
}
bool positive(const Mask & mask, std::size_t i) {
    return ((mask[i / 8] >> (i % 8)) & 1) != 0;
}
void basis_check(const Basis & b) {
    require(b.computed_depth <= 3 && b.consumed_depth <= b.computed_depth,
            ErrorCode::depth, "invalid computed/consumed depth");
    for (unsigned j = 0; j < b.consumed_depth; ++j)
        require(std::isfinite(b.planes[j].alpha) && b.planes[j].alpha >= 0,
                ErrorCode::arithmetic, "invalid active basis coefficient");
}
void next_plane(Basis & b) {
    Plane & p = b.planes[b.computed_depth];
    float sum = 0;
    for (std::size_t i = 0; i < group_size; ++i) {
        if (b.residual[i] >= 0) p.signs[i / 8] |= std::uint8_t(1u << (i % 8));
        sum = finite(sum + std::fabs(b.residual[i]));
    }
    p.alpha = sum / float(group_size);
    for (std::size_t i = 0; i < group_size; ++i) {
        const float signed_alpha = positive(p.signs, i) ? p.alpha : -p.alpha;
        b.residual[i] = finite(b.residual[i] - signed_alpha);
    }
    ++b.computed_depth;
    b.consumed_depth = b.computed_depth;
}
float energy(const Group & x) {
    float sum = 0;
    for (float value : x) {
        const float square = finite(value * value);
        sum = finite(sum + square);
    }
    return sum;
}
void row_basis_check(const NativeView & v, std::size_t row, const std::vector<Basis> & groups) {
    validate(v);
    require(row < v.rows, ErrorCode::layout, "row out of range");
    require(groups.size() == v.columns / group_size, ErrorCode::layout, "basis group count mismatch");
    environment();
    for (const auto & b : groups) basis_check(b);
}
} // namespace

float half_to_float(std::uint16_t bits) {
    const int exponent = (bits >> 10) & 31;
    const int fraction = bits & 1023;
    require(exponent != 31, ErrorCode::scale, "non-finite binary16 scale");
    // All finite binary16 values are exact in binary32, including 2^-24.
    const float magnitude = exponent == 0 ? std::ldexp(float(fraction), -24)
        : std::ldexp(float(1024 + fraction), exponent - 25);
    return (bits & 0x8000) ? -magnitude : magnitude;
}

void validate(const NativeView & v) {
    require(v.format == NativeFormat::q1_0, ErrorCode::layout, "unsupported native format");
    require(v.storage != nullptr, ErrorCode::layout, "null native storage");
    require(v.rows > 0 && v.columns > 0 && v.columns % group_size == 0,
            ErrorCode::layout, "positive rows and complete 128-element groups required");
    require(v.group_stride_bytes == native_group_bytes, ErrorCode::layout, "native group stride must be 18");
    const auto groups = v.columns / group_size;
    require(groups <= std::numeric_limits<std::size_t>::max() / native_group_bytes,
            ErrorCode::layout, "packed row byte count overflow");
    const auto packed = groups * native_group_bytes;
    require(v.row_stride_bytes >= packed, ErrorCode::layout, "row stride smaller than packed row");
    require(v.offset <= v.storage_bytes && packed <= v.storage_bytes - v.offset,
            ErrorCode::layout, "insufficient row bytes or invalid view offset");
    require(v.rows - 1 <= (v.storage_bytes - v.offset - packed) / v.row_stride_bytes,
            ErrorCode::layout, "insufficient storage for strided rows");
}

Group decode_group(const NativeView & v, std::size_t row, std::size_t group) {
    const auto * bytes = block(v, row, group);
    const float d = scale(bytes);
    Group result{};
    for (std::size_t i = 0; i < group_size; ++i)
        result[i] = ((bytes[2 + i / 8] >> (i % 8)) & 1) ? d : -d;
    return result;
}
std::vector<float> decode_row(const NativeView & v, std::size_t row) {
    validate(v);
    require(row < v.rows, ErrorCode::layout, "row out of range");
    std::vector<float> result(v.columns);
    for (std::size_t g = 0; g < v.columns / group_size; ++g) {
        const auto values = decode_group(v, row, g);
        for (std::size_t i = 0; i < group_size; ++i) result[g * group_size + i] = values[i];
    }
    return result;
}
Mask weight_mask(const NativeView & v, std::size_t row, std::size_t group) {
    const auto * bytes = block(v, row, group);
    (void)scale(bytes); // reject malformed scales even for a mask-only read
    Mask result{};
    for (std::size_t i = 0; i < result.size(); ++i) result[i] = bytes[2 + i];
    return result;
}
float weight_scale(const NativeView & v, std::size_t row, std::size_t group) {
    return scale(block(v, row, group));
}

Basis encode_fixed(const Group & x, unsigned depth) {
    require(depth >= 1 && depth <= 3, ErrorCode::depth, "fixed depth must be 1, 2 or 3");
    activation_check(x.data(), x.size());
    Basis b;
    b.residual = x;
    while (b.computed_depth < depth) next_plane(b);
    return b;
}
Adaptive encode_adaptive(const Group & x, float tau1, float tau2) {
    require(std::isfinite(tau1) && std::isfinite(tau2) &&
            tau1 >= 0 && tau1 <= 1 && tau2 >= 0 && tau2 <= 1,
            ErrorCode::threshold, "thresholds must be finite in [0,1]");
    activation_check(x.data(), x.size());
    Adaptive a;
    a.basis.residual = x;
    bool zero = true;
    for (float value : x) zero = zero && value == 0;
    if (zero) return a; // explicitly supported depth 0; no division or plane work
    a.energy0 = energy(x);
    require(a.energy0 > 0, ErrorCode::energy_underflow, "nonzero activation energy underflowed to zero");
    const float thresholds[2] = {tau1, tau2};
    for (unsigned j = 0; j < 2; ++j) {
        next_plane(a.basis);
        a.residual_ratio[j] = finite(energy(a.basis.residual) / a.energy0);
        ++a.evaluated_stops;
        if (a.residual_ratio[j] <= thresholds[j]) return a;
    }
    next_plane(a.basis);
    return a;
}
Group reconstruct(const Basis & b) {
    basis_check(b);
    environment();
    Group result{};
    for (std::size_t i = 0; i < group_size; ++i)
        for (unsigned j = 0; j < b.consumed_depth; ++j)
            result[i] = finite(result[i] + (positive(b.planes[j].signs, i) ? b.planes[j].alpha : -b.planes[j].alpha));
    return result;
}
int mask_dot(const Mask & a, const Mask & b) {
    int different = 0;
    for (std::size_t byte = 0; byte < a.size(); ++byte) {
        unsigned value = a[byte] ^ b[byte];
        while (value != 0) { different += int(value & 1u); value >>= 1; }
    }
    return int(group_size) - 2 * different;
}
float dot_original(const NativeView & v, std::size_t row, const float * x, std::size_t count) {
    validate(v);
    require(row < v.rows, ErrorCode::layout, "row out of range");
    require(count == v.columns, ErrorCode::layout, "activation count must equal row columns");
    activation_check(x, count);
    float sum = 0;
    for (std::size_t g = 0; g < count / group_size; ++g) {
        const auto w = decode_group(v, row, g);
        for (std::size_t i = 0; i < group_size; ++i) {
            const float product = finite(w[i] * x[g * group_size + i]);
            sum = finite(sum + product);
        }
    }
    return sum;
}
float dot_reconstructed(const NativeView & v, std::size_t row, const std::vector<Basis> & groups) {
    row_basis_check(v, row, groups);
    float sum = 0;
    for (std::size_t g = 0; g < groups.size(); ++g) {
        const auto w = decode_group(v, row, g);
        const auto x = reconstruct(groups[g]);
        for (std::size_t i = 0; i < group_size; ++i) {
            const float product = finite(w[i] * x[i]);
            sum = finite(sum + product);
        }
    }
    return sum;
}
float dot_masks(const NativeView & v, std::size_t row, const std::vector<Basis> & groups) {
    row_basis_check(v, row, groups);
    float sum = 0;
    for (std::size_t g = 0; g < groups.size(); ++g) {
        const Mask w = weight_mask(v, row, g);
        float plane_sum = 0;
        for (unsigned j = 0; j < groups[g].consumed_depth; ++j) {
            const auto & p = groups[g].planes[j];
            const float product = finite(p.alpha * float(mask_dot(w, p.signs)));
            plane_sum = finite(plane_sum + product);
        }
        const float scaled = finite(weight_scale(v, row, g) * plane_sum);
        sum = finite(sum + scaled);
    }
    return sum;
}
} // namespace k80nsai::q1_reference

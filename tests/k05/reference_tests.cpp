#include "fixtures.h"
#include <algorithm>
#include <cfenv>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <type_traits>

namespace ref = k80nsai::q1_reference;
namespace fx = k05_fixtures;
static_assert(!std::is_convertible<const std::uint16_t *, const float *>::value,
              "FP16 activation storage is not an accepted FP32 input");

namespace {
std::uint32_t seed = fx::default_seed;
std::string context;
unsigned cases = 0, checks = 0;
void check(bool condition, const std::string & message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}
void integer(int actual, int expected, const std::string & what) {
    check(actual == expected, what + " expected=" + std::to_string(expected) + " actual=" + std::to_string(actual));
}
std::string thresholds_text(float t1, float t2) {
    std::ostringstream out;
    out << std::hexfloat << "tau1=" << t1 << " tau2=" << t2 << "\n";
    return out.str();
}
std::uint32_t bits(float x) {
    std::uint32_t value; std::memcpy(&value, &x, 4); return value;
}
void exact(float actual, float expected, const std::string & what, bool signed_zero = false) {
    std::ostringstream s;
    s << what << " expected=" << std::hexfloat << expected << " actual=" << actual
      << " expected_bits=0x" << std::hex << bits(expected) << " actual_bits=0x" << bits(actual);
    check(actual == expected && (!signed_zero || bits(actual) == bits(expected)), s.str());
}
void close(float actual, float expected, double bound, const std::string & what) {
    std::ostringstream s;
    s << std::setprecision(17) << what << " expected=" << expected << " actual=" << actual << " bound=" << bound;
    check(std::isfinite(actual) && std::isfinite(expected) && std::abs(double(actual) - expected) <= bound, s.str());
}
void run(const std::string & name, const std::function<void()> & test) {
    context = "case=" + name + "\n";
    test();
    ++cases;
}
void rejects(ref::ErrorCode code, const std::function<void()> & test) {
    try { test(); }
    catch (const ref::Error & e) {
        check(e.code() == code, std::string("wrong rejection code: ") + e.what());
        return;
    }
    check(false, "expected explicit rejection, actual=success");
}
int scalar_sign_dot(const ref::Mask & a, const ref::Mask & b) {
    int result = 0;
    for (std::size_t i = 0; i < 128; ++i) {
        const int left = (a[i / 8] & (1u << (i % 8))) ? 1 : -1;
        const int right = (b[i / 8] & (1u << (i % 8))) ? 1 : -1;
        result += left * right;
    }
    return result;
}
double gamma(unsigned n) {
    const double nu = n * std::ldexp(1.0, -24);
    return nu / (1 - nu);
}
// Compare two rearrangements of the SAME stored alphas/signs, not approximation error.
// N scalar products/sums plus <=3 reconstruction adds, versus <=3 plane products
// and sums plus group scaling/summing: gamma(N+8)+gamma(G+8) times L1 plane mass.
double dot_bound(const ref::NativeView & v, std::size_t row, const std::vector<ref::Basis> & basis) {
    double mass = 0;
    for (std::size_t g = 0; g < basis.size(); ++g) {
        double alpha = 0;
        for (unsigned j = 0; j < basis[g].consumed_depth; ++j) alpha += basis[g].planes[j].alpha;
        mass += std::abs(double(ref::weight_scale(v, row, g))) * 128 * alpha;
    }
    return (gamma(unsigned(v.columns + 8)) + gamma(unsigned(basis.size() + 8))) * mass
        + double(v.columns + basis.size() + 16) * std::numeric_limits<float>::denorm_min();
}
void routes(const ref::NativeView & v, std::size_t row, const std::vector<ref::Basis> & basis) {
    for (std::size_t g = 0; g < basis.size(); ++g) {
        const auto w = ref::weight_mask(v, row, g);
        for (unsigned j = 0; j < basis[g].consumed_depth; ++j)
            check(ref::mask_dot(w, basis[g].planes[j].signs) ==
                  scalar_sign_dot(w, basis[g].planes[j].signs), "integer mask dot mismatch");
    }
    close(ref::dot_masks(v, row, basis), ref::dot_reconstructed(v, row, basis),
          dot_bound(v, row, basis), "mask/reconstruction FP32 dot");
}
void half_and_bits() {
    struct Half { std::uint16_t bits; float value; };
    const Half halves[] = {
        {0x0000, 0.0f}, {0x8000, -0.0f}, {0x3c00, 1}, {0xbc00, -1},
        {0x3800, .5f}, {0xb800, -.5f}, {0x3555, 0x1.554p-2f},
        {0xb555, -0x1.554p-2f}, {0x6400, 1024}, {0xe400, -1024},
        {0x7bff, 65504}, {0xfbff, -65504}, {0x0400, 0x1p-14f},
        {0x8400, -0x1p-14f}, {0x03ff, 0x1.ff8p-15f},
        {0x0001, 0x1p-24f}, {0x8001, -0x1p-24f}
    };
    for (const auto & h : halves) run("half-" + std::to_string(h.bits), [&] {
        exact(ref::half_to_float(h.bits), h.value, "known half", true);
        for (std::uint8_t pattern : {0x00, 0xff, 0x55, 0xaa}) {
            const auto b = fx::block(h.bits, fx::filled(pattern));
            const auto v = fx::view(b);
            context += fx::dump(v, 0, {fx::repeat({1})});
            const auto decoded = ref::decode_group(v, 0, 0);
            for (std::size_t i = 0; i < 128; ++i)
                exact(decoded[i], (pattern & (1u << (i % 8))) ? h.value : -h.value, "decoded half/sign", true);
        }
    });
    for (unsigned bit : {0u,1u,7u,8u,31u,32u,63u,64u,95u,96u,127u})
        for (bool start_positive : {false, true}) run("boundary-" + std::to_string(bit) + "-" + std::to_string(start_positive), [&] {
            auto mask = fx::filled(start_positive ? 255 : 0);
            mask[bit / 8] ^= std::uint8_t(1u << (bit % 8));
            const auto b = fx::block(0x3c00, mask); const auto v = fx::view(b);
            context += fx::dump(v, 0, {fx::repeat({1})});
            check(ref::weight_mask(v, 0, 0) == mask, "mask byte layout");
            const auto w = ref::decode_group(v, 0, 0);
            for (unsigned i = 0; i < 128; ++i)
                exact(w[i], (start_positive != (i == bit)) ? 1.0f : -1.0f, "boundary sign");
        });
    for (unsigned mismatch = 0; mismatch <= 128; ++mismatch) run("mask-distance-" + std::to_string(mismatch), [&] {
        auto a = fx::filled(0xaa), b = a;
        for (unsigned i = 0; i < mismatch; ++i) b[i / 8] ^= std::uint8_t(1u << (i % 8));
        check(ref::mask_dot(a, b) == 128 - 2 * int(mismatch), "exact XOR popcount identity");
        check(ref::mask_dot(a, b) == scalar_sign_dot(a, b), "independent scalar integer dot");
    });
}
void layouts() {
    for (std::size_t groups : {1u,2u,3u})
        for (std::size_t offset : {0u,1u,3u})
            for (std::size_t padding : {0u,7u}) run("layout-g" + std::to_string(groups) +
                "-o" + std::to_string(offset) + "-p" + std::to_string(padding), [&] {
                const std::size_t rows = 3, packed = groups * 18, stride = packed + padding;
                // Sentinel prefix, row gaps and suffix. View extent excludes suffix.
                std::vector<std::uint8_t> bytes(offset + (rows - 1) * stride + packed + 11, 0xcd);
                for (std::size_t r = 0; r < rows; ++r)
                    for (std::size_t g = 0; g < groups; ++g)
                        fx::put(bytes, offset + r * stride + g * 18,
                            (r + g) % 2 ? 0xb800 : 0x3c00, fx::filled(std::uint8_t(0x11u * (r + g + 1))));
                const auto snapshot = bytes;
                const ref::NativeView v{bytes.data(), bytes.size() - 11, offset, rows, groups * 128, stride};
                ref::validate(v);
                for (std::size_t r = 0; r < rows; ++r) {
                    context += fx::dump(v, r, std::vector<ref::Group>(groups, fx::repeat({1})));
                    const auto w = ref::decode_row(v, r);
                    for (std::size_t g = 0; g < groups; ++g)
                        for (std::size_t i = 0; i < 128; ++i) {
                            const float d = (r + g) % 2 ? -.5f : 1;
                            const unsigned pattern = 0x11u * unsigned(r + g + 1);
                            exact(w[g * 128 + i], (pattern & (1u << (i % 8))) ? d : -d, "strided decode");
                        }
                }
                check(bytes == snapshot, "input/sentinels modified");
            });
}
void analytic() {
    run("analytic-quarter-sparse", [] {
        const auto x = fx::repeat({0,0,0,4});
        const float alpha[] = {1, 1.5f, .75f};
        const float residual[3][4] = {{-1,-1,-1,3},{.5f,.5f,.5f,1.5f},{-.25f,-.25f,-.25f,.75f}};
        const float reconstructed[3][4] = {{1,1,1,1},{-.5f,-.5f,-.5f,2.5f},{.25f,.25f,.25f,3.25f}};
        for (unsigned p = 1; p <= 3; ++p) {
            const auto basis = ref::encode_fixed(x, p);
            check(basis.computed_depth == p && basis.consumed_depth == p, "fixed depth");
            const auto hat = ref::reconstruct(basis);
            for (unsigned j = 0; j < p; ++j) {
                exact(basis.planes[j].alpha, alpha[j], "analytic alpha");
                check(basis.planes[j].signs == fx::filled(j == 1 ? 0x88 : 0xff), "analytic signs");
            }
            for (std::size_t i = 0; i < 128; ++i) {
                exact(basis.residual[i], residual[p - 1][i % 4], "analytic residual");
                exact(hat[i], reconstructed[p - 1][i % 4], "analytic reconstruction");
            }
            for (bool negative : {false,true}) for (bool matching : {false,true}) {
                const auto b = fx::block(negative ? 0xbc00 : 0x3c00, fx::filled(matching ? 0x88 : 0xff));
                const auto v = fx::view(b);
                context = "case=analytic-quarter-sparse depth=" + std::to_string(p) + "\n" + fx::dump(v, 0, {x});
                const float all[] = {128,32,128}, match[] = {-64,128,80};
                const float expected = (negative ? -1.0f : 1.0f) * (matching ? match[p - 1] : all[p - 1]);
                exact(ref::dot_original(v, 0, x.data(), x.size()), negative ? -128.0f : 128.0f, "original FP32");
                exact(ref::dot_masks(v, 0, {basis}), expected, "analytic mask output");
                exact(ref::dot_reconstructed(v, 0, {basis}), expected, "analytic reconstructed output");
            }
        }
    });
    run("analytic-four-magnitudes", [] {
        const auto x = fx::repeat({1,2,3,4});
        const float alphas[] = {2.5f,1,.5f};
        const unsigned patterns[] = {255,204,170};
        for (unsigned p = 1; p <= 3; ++p) {
            const auto basis = ref::encode_fixed(x, p);
            for (unsigned j = 0; j < p; ++j) {
                exact(basis.planes[j].alpha, alphas[j], "four-magnitude alpha");
                check(basis.planes[j].signs == fx::filled(std::uint8_t(patterns[j])), "four-magnitude signs");
            }
            if (p == 3) {
                check(ref::reconstruct(basis) == x, "B3 exact reconstruction");
                check(basis.residual == ref::Group{}, "B3 zero residual");
            }
        }
    });
    const std::vector<ref::Group> inputs = {
        fx::repeat({0.0f,-0.0f}), fx::repeat({2}), fx::repeat({-2}),
        fx::repeat({1,-1}), fx::repeat({-4,.25f,0,2}),
        fx::repeat({1024,-1024,.125f,-.125f}), fx::repeat({0,0,0,1}),
        [] { ref::Group x{}; x[127] = 128; return x; }()
    };
    for (std::size_t c = 0; c < inputs.size(); ++c) run("activation-" + std::to_string(c), [&] {
        for (unsigned p = 1; p <= 3; ++p) {
            const auto basis = ref::encode_fixed(inputs[c], p);
            if (c < 4) {
                exact(basis.planes[0].alpha, c == 0 ? 0 : c == 3 ? 1 : 2, "constant magnitude alpha");
                for (unsigned j = 1; j < p; ++j) exact(basis.planes[j].alpha, 0, "zero residual alpha");
                check(ref::reconstruct(basis) == inputs[c], "constant magnitude reconstruction");
                if (c == 0) for (unsigned j = 0; j < p; ++j)
                    check(basis.planes[j].signs == fx::filled(255), "sign(+/-0)=+1");
            }
            for (std::uint16_t scale : {0x0000,0x8000,0x3800,0xb800,0x3555,0x6400,0x0001}) {
                const auto b = fx::block(scale, fx::filled(0xaa)); const auto v = fx::view(b);
                context = "case=activation-" + std::to_string(c) + " depth=" + std::to_string(p) + "\n" + fx::dump(v, 0, {inputs[c]});
                routes(v, 0, {basis});
            }
        }
    });
    run("unit-dots", [] {
        const auto x = fx::repeat({1});
        for (const auto pattern : {0u,255u}) {
            const auto bytes = fx::block(0x3c00, fx::filled(std::uint8_t(pattern)));
            const auto v = fx::view(bytes);
            context = "case=unit-dots\n" + fx::dump(v, 0, {x});
            const float expected = pattern ? 128.0f : -128.0f;
            exact(ref::dot_original(v, 0, x.data(), x.size()), expected, "unit original dot");
            for (unsigned p = 1; p <= 3; ++p) {
                const auto basis = ref::encode_fixed(x, p);
                exact(ref::dot_reconstructed(v, 0, {basis}), expected, "unit reconstructed dot");
                exact(ref::dot_masks(v, 0, {basis}), expected, "unit mask dot");
            }
        }
    });
    run("original-unquantized", [] {
        const auto x = fx::repeat({.1f});
        const auto b = fx::block(0x3c00, fx::filled(255)); const auto v = fx::view(b);
        float sum = 0; for (float value : x) sum += value;
        exact(ref::dot_original(v, 0, x.data(), x.size()), sum, "original FP32 order");
        check(ref::dot_original(v, 0, x.data(), x.size()) != 128 * ref::half_to_float(0x2e66),
              "original activations were rounded to half");
    });
}
void adaptive() {
    const auto x = fx::repeat({0,0,0,4});
    struct Trial { float t1, t2; unsigned depth; };
    const Trial trials[] = {
        {.75f,0,1},{std::nextafter(.75f,0.0f),.1875f,2},
        {0,.1875f,2},{0,std::nextafter(.1875f,0.0f),3},
        {0,0,3},{1,0,1},{0,1,2},{1,1,1}
    };
    for (const auto & t : trials) run("adaptive-boundary", [&] {
        context += thresholds_text(t.t1, t.t2);
        const auto a = ref::encode_adaptive(x, t.t1, t.t2);
        exact(a.energy0, 512, "E0");
        exact(a.residual_ratio[0], .75f, "e1");
        if (t.depth > 1) exact(a.residual_ratio[1], .1875f, "e2");
        integer(int(a.basis.computed_depth), int(t.depth), "adaptive computed boundary"); integer(int(a.basis.consumed_depth), int(t.depth), "adaptive consumed boundary");
        check(a.evaluated_stops == std::min(t.depth, 2u), "computed adaptive stop count");
        for (unsigned j = t.depth; j < 3; ++j) {
            exact(a.basis.planes[j].alpha, 0, "uncomputed alpha");
            check(a.basis.planes[j].signs == ref::Mask{}, "uncomputed signs");
        }
        check(ref::reconstruct(a.basis) == ref::reconstruct(ref::encode_fixed(x, t.depth)), "adaptive/fixed consistency");
    });
    for (float value : {0.0f,-0.0f,2.0f,-2.0f}) run("adaptive-constant-" + std::to_string(value), [&] {
        const auto a = ref::encode_adaptive(fx::repeat({value}), 0, 0);
        const unsigned expected = value == 0 ? 0 : 1;
        check(a.basis.computed_depth == expected && a.basis.consumed_depth == expected, "zero/constant adaptive depth");
        check(a.evaluated_stops == expected, "zero/constant stop count");
        check(ref::reconstruct(a.basis) == fx::repeat({value}), "zero/constant reconstruction");
    });
    run("inactive-poison", [] {
        auto b = ref::encode_fixed(fx::repeat({2}), 1);
        for (unsigned j = 1; j < 3; ++j) {
            b.planes[j].alpha = std::numeric_limits<float>::quiet_NaN();
            b.planes[j].signs.fill(0xcd);
        }
        b.computed_depth = 3; // consumer must obey consumed depth even with larger computed capacity
        b.residual.fill(std::numeric_limits<float>::quiet_NaN());
        const auto bytes = fx::block(0x3c00, fx::filled(255)); const auto v = fx::view(bytes);
        exact(ref::dot_reconstructed(v, 0, {b}), 256, "inactive planes not read (reconstruction)");
        exact(ref::dot_masks(v, 0, {b}), 256, "inactive planes not read (mask)");
    });
}
void negative_case(const std::string & name) {
    auto bytes = fx::block(0x3c00, fx::filled(255));
    auto v = fx::view(bytes); auto x = fx::repeat({1});
    context += "negative=" + name + " fixture=negative_case-v1 row=0 group=0\n";
    context += "defaults: native=18 bytes, half=0x3c00, signs=all+, x=128*1; override:\n";
    // The named mutation is a compact deterministic reproducer; do not print stale
    // default metadata as though it describes the mutated input.
    if (name == "nan" || name == "inf" || name == "minus-inf") context += "x[63]=" + name + "\n";
    else if (name == "half-inf") context += "half_bits=0xfc00\n";
    else if (name == "half-nan") context += "half_bits=0x7e01\n";
    else if (name == "energy-product") context += "x=constant(1e20f)\n";
    else if (name == "energy-sum") context += "x=constant(2e18f)\n";
    else if (name == "energy-underflow") context += "x=constant(FLT_TRUE_MIN)\n";
    else if (name == "alpha-overflow") context += "x=constant(FLT_MAX)\n";
    else if (name == "dot-product") context += "half_bits=0x7bff x=constant(FLT_MAX)\n";
    else if (name == "dot-sum") context += "x=constant(FLT_MAX/64)\n";
    else context += "apply named mutation '" + name + "' from negative_case-v1\n";
    if (name == "storage") { v.storage_bytes = 17; ref::validate(v); }
    else if (name == "tail") { v.columns = 129; ref::validate(v); }
    else if (name == "zero-columns") { v.columns = 0; ref::validate(v); }
    else if (name == "zero-rows") { v.rows = 0; ref::validate(v); }
    else if (name == "block-count") { v.columns = 256; v.row_stride_bytes = 36; ref::validate(v); }
    else if (name == "stride") { v.row_stride_bytes = 17; ref::validate(v); }
    else if (name == "group-stride") { v.group_stride_bytes = 20; ref::validate(v); }
    else if (name == "offset") { v.offset = std::numeric_limits<std::size_t>::max(); ref::validate(v); }
    else if (name == "extent-overflow") { v.rows = std::numeric_limits<std::size_t>::max(); v.row_stride_bytes = 36; ref::validate(v); }
    else if (name == "row-bytes") { v.rows = 2; v.storage_bytes = 18; ref::validate(v); }
    else if (name == "dtype") { v.format = ref::NativeFormat::unsupported; ref::validate(v); }
    else if (name == "null-storage") { v.storage = nullptr; ref::validate(v); }
    else if (name == "row-index") { (void)ref::decode_row(v, 1); }
    else if (name == "group-index") { (void)ref::decode_group(v, 0, 1); }
    else if (name == "basis-count") { (void)ref::dot_masks(v, 0, {}); }
    else if (name == "activation-count") { (void)ref::dot_original(v, 0, x.data(), 127); }
    else if (name == "null-activation") { (void)ref::dot_original(v, 0, nullptr, 128); }
    else if (name == "nan" || name == "inf" || name == "minus-inf") {
        x[63] = name == "nan" ? std::numeric_limits<float>::quiet_NaN()
            : name == "inf" ? INFINITY : -INFINITY;
        (void)ref::encode_fixed(x, 1);
    }
    else if (name == "half-inf" || name == "half-nan") {
        fx::put(bytes, 0, name == "half-inf" ? 0xfc00 : 0x7e01, fx::filled(255));
        (void)ref::decode_group(v, 0, 0);
    }
    else if (name == "threshold") { (void)ref::encode_adaptive(x, -.1f, 0); }
    else if (name == "threshold-nan") { (void)ref::encode_adaptive(x, 0, NAN); }
    else if (name == "threshold-inf") { (void)ref::encode_adaptive(x, INFINITY, 0); }
    else if (name == "threshold-above") { (void)ref::encode_adaptive(x, 0, 1.01f); }
    else if (name == "depth") { (void)ref::encode_fixed(x, 0); }
    else if (name == "depth-four") { (void)ref::encode_fixed(x, 4); }
    else if (name == "energy-product") { x.fill(1e20f); (void)ref::encode_adaptive(x, 0, 0); }
    else if (name == "energy-sum") { x.fill(2e18f); (void)ref::encode_adaptive(x, 0, 0); }
    else if (name == "energy-underflow") { x.fill(std::numeric_limits<float>::denorm_min()); (void)ref::encode_adaptive(x, 0, 0); }
    else if (name == "alpha-overflow") { x.fill(std::numeric_limits<float>::max()); (void)ref::encode_fixed(x, 1); }
    else if (name == "dot-product") {
        fx::put(bytes, 0, 0x7bff, fx::filled(255)); x.fill(std::numeric_limits<float>::max());
        (void)ref::dot_original(v, 0, x.data(), 128);
    }
    else if (name == "dot-sum") { x.fill(std::numeric_limits<float>::max() / 64); (void)ref::dot_original(v, 0, x.data(), 128); }
    else if (name == "bad-basis") { auto b = ref::encode_fixed(x, 1); b.consumed_depth = 2; (void)ref::reconstruct(b); }
    else if (name == "computed-depth") { ref::Basis b; b.computed_depth = 4; (void)ref::reconstruct(b); }
    else if (name == "negative-alpha") { auto b = ref::encode_fixed(x, 1); b.planes[0].alpha = -1; (void)ref::reconstruct(b); }
    else if (name == "bad-alpha") { auto b = ref::encode_fixed(x, 1); b.planes[0].alpha = NAN; (void)ref::reconstruct(b); }
    else throw std::runtime_error("unknown negative control: " + name);
}
const std::vector<std::pair<std::string, ref::ErrorCode>> negatives = {
    {"storage",ref::ErrorCode::layout},{"tail",ref::ErrorCode::layout},
    {"zero-columns",ref::ErrorCode::layout},{"zero-rows",ref::ErrorCode::layout},
    {"block-count",ref::ErrorCode::layout},{"stride",ref::ErrorCode::layout},
    {"group-stride",ref::ErrorCode::layout},{"offset",ref::ErrorCode::layout},
    {"extent-overflow",ref::ErrorCode::layout},{"row-bytes",ref::ErrorCode::layout},
    {"dtype",ref::ErrorCode::layout},{"null-storage",ref::ErrorCode::layout},
    {"row-index",ref::ErrorCode::layout},{"group-index",ref::ErrorCode::layout},
    {"basis-count",ref::ErrorCode::layout},{"activation-count",ref::ErrorCode::layout},
    {"null-activation",ref::ErrorCode::activation},{"nan",ref::ErrorCode::activation},
    {"inf",ref::ErrorCode::activation},{"minus-inf",ref::ErrorCode::activation},
    {"half-inf",ref::ErrorCode::scale},{"half-nan",ref::ErrorCode::scale},
    {"threshold",ref::ErrorCode::threshold},{"threshold-nan",ref::ErrorCode::threshold},
    {"threshold-inf",ref::ErrorCode::threshold},{"threshold-above",ref::ErrorCode::threshold},
    {"depth",ref::ErrorCode::depth},{"depth-four",ref::ErrorCode::depth},
    {"energy-product",ref::ErrorCode::arithmetic},{"energy-sum",ref::ErrorCode::arithmetic},
    {"energy-underflow",ref::ErrorCode::energy_underflow},{"alpha-overflow",ref::ErrorCode::arithmetic},
    {"dot-product",ref::ErrorCode::arithmetic},{"dot-sum",ref::ErrorCode::arithmetic},
    {"bad-basis",ref::ErrorCode::depth},{"bad-alpha",ref::ErrorCode::arithmetic},
    {"computed-depth",ref::ErrorCode::depth},{"negative-alpha",ref::ErrorCode::arithmetic}
};
void random_cases() {
    fx::Random rng{seed};
    for (unsigned c = 0; c < 32; ++c) run("random-" + std::to_string(c), [&] {
        const auto generator_state = rng.state;
        constexpr std::size_t offset = 3, stride = 61, rows = 3, groups = 3;
        std::vector<std::uint8_t> bytes(offset + (rows - 1) * stride + groups * 18, 0xcd);
        const std::uint16_t scales[] = {0x0000,0x8000,0x3800,0xb800,0x3555,0x6400,0xe400,0x0400,0x0001};
        std::vector<ref::Group> x;
        std::vector<float> flat;
        for (unsigned g = 0; g < groups; ++g) {
            x.push_back(rng.activation());
            flat.insert(flat.end(), x.back().begin(), x.back().end());
        }
        for (unsigned r = 0; r < rows; ++r) for (unsigned g = 0; g < groups; ++g) {
            const auto half = scales[rng.next() % 9]; const auto mask = rng.mask();
            fx::put(bytes, offset + r * stride + g * 18, half, mask);
        }
        const auto snapshot = bytes;
        const ref::NativeView v{bytes.data(), bytes.size(), offset, rows, groups * 128, stride};
        for (unsigned r = 0; r < rows; ++r) {
            context = "case=random-" + std::to_string(c) + " generator_state=" + std::to_string(generator_state) + "\n" + fx::dump(v, r, x);
            const auto w = ref::decode_row(v, r);
            float expected = 0;
            for (std::size_t i = 0; i < w.size(); ++i) { const float product = w[i] * flat[i]; expected += product; }
            exact(ref::dot_original(v, r, flat.data(), flat.size()), expected, "original sequential FP32 row");
            for (unsigned p = 1; p <= 3; ++p) {
                std::vector<ref::Basis> encoded;
                for (const auto & group : x) encoded.push_back(ref::encode_fixed(group, p));
                context += "requested_depth=" + std::to_string(p) + "\n";
                routes(v, r, encoded);
            }
            for (const auto thresholds : {std::pair<float,float>{1.0f,1.0f},{0.0f,1.0f},{0.0f,0.0f},{.25f,.1f}}) {
                std::vector<ref::Basis> encoded;
                for (const auto & group : x) {
                    const auto a = ref::encode_adaptive(group, thresholds.first, thresholds.second);
                    const auto b1 = ref::encode_fixed(group, 1), b2 = ref::encode_fixed(group, 2);
                    float e0 = 0, e1 = 0, e2 = 0;
                    for (unsigned i = 0; i < 128; ++i) {
                        const float z0 = group[i] * group[i], z1 = b1.residual[i] * b1.residual[i], z2 = b2.residual[i] * b2.residual[i];
                        e0 += z0; e1 += z1; e2 += z2;
                    }
                    const unsigned depth = e1 / e0 <= thresholds.first ? 1 : e2 / e0 <= thresholds.second ? 2 : 3;
                    check(a.basis.computed_depth == depth && a.basis.consumed_depth == depth, "random adaptive depth");
                    const auto fixed = ref::encode_fixed(group, depth);
                    check(ref::reconstruct(a.basis) == ref::reconstruct(fixed), "random adaptive reconstruction");
                    encoded.push_back(a.basis);
                }
                context += thresholds_text(thresholds.first, thresholds.second);
                routes(v, r, encoded);
            }
        }
        check(bytes == snapshot, "random input/sentinels modified");
    });
}
void inject(const std::string & name) {
    fx::Random rng{seed};
    const auto x = rng.activation(); const auto bytes = fx::block(0xb800, rng.mask()); const auto v = fx::view(bytes);
    context = "case=injected-" + name + " requested_depth=3 tau1=0 tau2=0\n" + fx::dump(v, 0, {x});
    if (name == "decode") exact(ref::decode_group(v, 0, 0)[0], 99, "deliberate decode mismatch");
    else if (name == "mask") integer(ref::mask_dot(ref::weight_mask(v, 0, 0), fx::filled(255)), 129, "deliberate integer mismatch");
    else if (name == "basis") exact(ref::encode_fixed(x, 3).planes[0].alpha, -1, "deliberate alpha mismatch");
    else if (name == "adaptive") integer(int(ref::encode_adaptive(x, 0, 0).basis.computed_depth), 4, "deliberate depth mismatch");
    else throw std::runtime_error("unknown injection");
}
} // namespace
int main(int argc, char ** argv) {
    try {
        std::string reject, injection;
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (i + 1 >= argc) throw std::runtime_error("option needs a value");
            if (arg == "--seed") {
                std::size_t used = 0; const std::string value = argv[++i];
                const auto parsed = std::stoull(value, &used, 0);
                if (used != value.size() || parsed == 0 || parsed > UINT32_MAX) throw std::runtime_error("seed must be uint32 nonzero");
                seed = std::uint32_t(parsed);
            } else if (arg == "--reject") reject = argv[++i];
            else if (arg == "--inject") injection = argv[++i];
            else throw std::runtime_error("unknown option: " + arg);
        }
        if (!reject.empty()) { negative_case(reject); throw std::runtime_error("negative control unexpectedly accepted"); }
        if (!injection.empty()) inject(injection);
        half_and_bits(); layouts(); analytic(); adaptive();
        for (const auto & n : negatives) run("reject-" + n.first, [&] { rejects(n.second, [&] { negative_case(n.first); }); });
        // Non-finites must be rejected even if scales/depths could hide their effects.
        run("nonfinite-all-entrypoints", [] {
            auto x = fx::repeat({0}); const auto b = fx::block(0, fx::filled(255)); const auto v = fx::view(b);
            for (float bad : {NAN,INFINITY,-INFINITY}) {
                x[127] = bad;
                rejects(ref::ErrorCode::activation, [&] { (void)ref::encode_adaptive(x, 1, 1); });
                rejects(ref::ErrorCode::activation, [&] { (void)ref::dot_original(v, 0, x.data(), x.size()); });
            }
            for (std::uint16_t h : {0x7c00,0xfc00,0x7c01,0x7fff,0xfc01,0xffff})
                rejects(ref::ErrorCode::scale, [&] { (void)ref::half_to_float(h); });
        });
        random_cases();
        std::cout << "PASS cases=" << cases << " checks=" << checks << " random_cases=32 rows_per_case=3 groups_per_row=3 seed=" << seed << "\n";
        return 0;
    } catch (const ref::Error & e) {
        std::cerr << "REJECT seed=" << seed << " code=" << int(e.code()) << "\n" << context << e.what() << "\n";
        return 1;
    } catch (const std::exception & e) {
        std::cerr << "FAIL seed=" << seed << "\n" << context << e.what() << "\n";
        return 1;
    }
}
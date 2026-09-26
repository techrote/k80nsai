# K05 independent native Q1 and binary-basis reference

Task [#5](https://github.com/techrote/k80nsai/issues/5). This small C++17 host
library defines the intended approximation from native Q1 bytes and original
finite FP32 activations. It is an oracle for later implementations, not a model
runtime or a GPU implementation. Evidence and acceptance state: [STATUS.md](STATUS.md).

## Frozen source and ABI

Accepted K03/main base: `fcf54b8671035b3959e851d842bbf5c8e7ba2fe7`, merged
PR #25; issue #3 was closed completed at claim and rechecked before committing.
K05 oracle implementation: `8a07e2ebf95aafdce199bbf7110cbfb56749e721`.
Final test diagnostics: `0c5b6b9482d15efb6681c7fab7372b3fbd813203`.

- Runtime: `ggml-org/llama.cpp@56381e407c0ccfb3a6f71e668a27a901001d22ce`.
- Upstream tree: `24d31963e64a61fc390ecbdd12f7245a9feb73f9`.
- Pristine import: `25ef5ba867cdc6bc0089d9cb05e7d89090f71560`.
- Lock: [llama.cpp.lock.json](../../../vendor/llama.cpp.lock.json).
- No vendor, root build, K04, workflow contract or runtime descriptor changes.

Reverified against the actual import, including full reads of
[ggml-common.h](../../../vendor/llama.cpp/ggml/src/ggml-common.h) and
[ggml-quants.c](../../../vendor/llama.cpp/ggml/src/ggml-quants.c):

| Source | Verified fact |
|---|---|
| common.h lines 6, 16, 180–185 | Host half storage is uint16_t; QK1_0=128; scale d at offset 0, qs[16] at offset 2; sizeof=18 |
| quants.c lines 40–72 | Ordinary encoder uses mean absolute weight as half scale and sign >=0 |
| quants.c lines 419–437 | Decoder uses stored scale; bit i is (qs[i/8] >> (i%8)) & 1; 1 selects d, 0 selects -d |
| quants.c lines 5352, 5532 | Native validation rejects non-finite half scales |
| ggml-cpu/ggml-cpu.c lines 228–233; ggml-cpu/quants.c lines 127–176 | Runtime CPU Q1 dot consumes Q8_0 activations, four Q8 blocks per Q1 group |

The separate [native_abi.cpp](../../../tests/k05/native_abi.cpp) target imports
only the header and compiles seven ABI assertions. It does not supply any oracle
arithmetic. Adjacent groups start 18 bytes apart: with a mod-4 aligned base, sign
arrays alternate offsets 2 and 0 modulo 4. Odd view offsets also make half storage
unaligned. No native sign-byte or scale location is dereferenced through a wider
integer pointer.

## Independence and scale semantics

[reference/q1](../../../reference/q1/q1_reference.h) is a standalone static
library with no imported headers or libraries, CUDA helpers, model objects,
Q8 conversion, runtime dequantizer or runtime dot calls. A source reviewer checked
the dependency boundary. There is no runtime decode comparison in this suite;
known half values, hand-derived fixtures and the separate ABI assertions constrain
the implementation without making runtime arithmetic its definition.

The half loader copies two bytes into a local uint16_t using memcpy. This API
accepts **native host-endian in-memory storage**, not a portable GGUF/file parser.
A caller reading serialized data must first establish its byte order. Sign bytes
are always LSB-first. No cross-endian execution claim is made.

The independent half conversion uses sign/exponent/fraction arithmetic:

- exponent 0: magnitude = fraction * 2^-24;
- exponents 1..30: magnitude = (1024 + fraction) * 2^(exponent-25);
- exponent 31: reject Inf/NaN;
- apply the stored sign, including negative zero.

Every finite binary16 value is exact in binary32. Scale is never recomputed,
rounded again, normalized or made positive. With a negative scale, the stored
bit still denotes the multiplier +1/-1; it does not describe the numeric sign of
the resulting weight. Decoding preserves signed zeros exactly. Reordered dot
routes promise numeric equality within the stated bound, not identical zero sign.

## Host API and validation boundary

The namespace is `k80nsai::q1_reference`. Include `q1_reference.h` and link
`k80nsai-q1-reference`; its CMake directory can be added independently.

| API/type | Meaning |
|---|---|
| NativeView | Byte pointer, readable byte count, base offset, rows, columns, row byte stride, group byte stride, native format tag |
| validate / decode_group / decode_row | Validate the view; return one Group=array<float,128> or an owning row vector |
| half_to_float / weight_scale / weight_mask | Independent finite scale conversion and byte-mask extraction |
| encode_fixed(x,p) | Construct exactly p=1,2,3 planes; return coefficients, sign bytes, final residual and depth counts |
| encode_adaptive(x,tau1,tau2) | Return Basis, E0, evaluated residual ratios and number of stop tests |
| reconstruct(basis) | Reconstruct only consumed planes; final residual is diagnostic, not an input |
| mask_dot(a,b) | Exact integer 128-2*popcount(a XOR b), counting all 128 valid bits |
| dot_original(view,row,x,count) | Decode weights, use original FP32 x and accumulate separate FP32 product/add operations in row element order |
| dot_reconstructed(view,row,bases) | Decode weights, reconstruct each activation group and accumulate in row element order |
| dot_masks(view,row,bases) | Sum alpha*integer-dot per group, apply its stored d, then sum groups |

This host mathematical view is not K08's runtime descriptor. It has no stream,
device, allocation, generation, dispatch or lifetime policy.

Only positive row/column counts and complete g128 groups are supported. Group
stride must be exactly 18; column count must be divisible by 128. Row stride must
be at least the packed row bytes. Row padding may be arbitrary, including a stride
not divisible by 18; that is a host decoder capability, **not GPU eligibility**.
Offsets 0, 1 and 3 and three-row views are tested. Zero dimensions and unsupported
format tags reject; no padded native tail is invented. Activations are typed float
storage only; FP16 pointers do not match the API (compile-time fixture), and original
dot count must equal the row columns.

Validation bounds the complete view before pointer arithmetic. It compares
`rows-1 <= (storage_bytes-offset-packed_row)/row_stride` after checking the
subtractions, rather than overflowing an extent multiplication. The caller must
provide a real readable buffer of the claimed size and keep it alive during calls;
no C++ pointer API can infer its actual allocation extent.

Results own their storage; there is no caller-owned output/scratch buffer to
overrun. Input buffers, padding and sentinels remain unchanged. A poisoned inactive
plane fixture tests computed_depth=3, consumed_depth=1 and a poisoned diagnostic
residual. Active depth/count/coefficient validity is checked. Returned encoder
results always have computed_depth=consumed_depth; the separate values allow a
future consumer test to distinguish work done from work consumed.

## Equations and FP32 behavior

For finite x[0..127], r0=x. Each actually constructed plane uses:

```text
sj[i] = +1 if rj[i] >= 0 else -1
alpha_j = sum_i abs(rj[i]) / 128
r(j+1)[i] = rj[i] - alpha_j * sj[i]
x_hat[i] = sum_j alpha_j * sj[i]
```

Coefficients, sums, residuals and reconstruction use scalar FP32, increasing element
and plane order. sign(+0)=sign(-0)=+1. Fixed B1/B2/B3 construct exactly 1/2/3
planes, including zero-coefficient planes. Zero reconstruction is +0.

The mask route independently uses byte XOR and scalar bit counting:

```text
dot_j = 128 - 2 * popcount(W XOR Sj)
y_group = d * sum_j(alpha_j * dot_j)
y_row = sum_g y_group
```

Integer dots are exact, even integers in [-128,128]. This route never obtains its
answer from the decoded-weight/reconstruction route. Conversely, reconstruction
does not use popcount. Original-FP32 dot is separate from both approximation routes
and from the imported CPU Q8_0 / CUDA Q8_1 reference paths.

Build with IEEE binary32, round-to-nearest and gradual underflow (FTZ/DAZ disabled).
The API checks the current rounding mode; it does not inspect architecture-specific
FTZ/DAZ registers. A caller that changes those modes must restore the documented
environment. The supplied target uses MSVC `/fp:strict`, or GCC/Clang
`-fno-fast-math -ffp-contract=off`, inherited by its test executable. No FMA,
reassociation, hidden half/Q8 conversion, learned coefficient or calibration is
part of this oracle.

## Adaptive behavior and error policy

Compute E0=sum_i x[i]^2 in sequential FP32. The true all-zero group (including
signed zero) returns depth 0, all coefficients/reconstruction zero, E0=0 and zero
evaluated stop tests; it performs no plane or division work. Fixed zero groups
still report their requested depth.

For nonzero x: construct B1, directly sum r1[i]^2, divide by E0, and stop at depth
1 when e1<=tau1. Otherwise construct B2, test e2<=tau2, or construct B3. No third
plane is computed before deciding to use it. Only the first evaluated_stops entries
of residual_ratio are meaningful. Both thresholds must be finite and in [0,1],
even for a zero group. There are no API defaults; .25/.1 in random fixtures are
screening choices only.

Failures throw `Error` with an `ErrorCode` and message; callers may explicitly
translate that into their future fallback policy. The oracle never silently falls
back, clamps or coerces.

| Input/condition | Behavior |
|---|---|
| NaN, +Inf, -Inf activation | activation error before encoding/dot, even for zero weight scale |
| Half scale exponent 31 | scale error, including mask-only extraction |
| Non-finite sum(abs), residual, energy product/sum, scaled product or accumulation | arithmetic error |
| Nonzero x whose FP32 E0 underflows to zero | energy_underflow error; never mislabeled all-zero |
| Invalid threshold, depth, active alpha, counts, layout or dtype | corresponding threshold/depth/arithmetic/layout error |
| Non-nearest rounding | arithmetic error |

Fixed encoding does not compute unused adaptive energy. It may accept finite x
that adaptive rejects because E0 would overflow. FP32 underflow in ordinary
arithmetic otherwise follows the declared IEEE environment. No finite-range
guarantee beyond explicitly checked operations is implied.

## Fixtures and exact expected values

The reference executable has **282 named cases / 35,339 checks** at the default
seed. Check count varies with random adaptive depths at other seeds.

| Category | Cases / coverage |
|---|---|
| Half/sign interpretation | 17 known scales × four sign patterns; +0/-0, +/-1, +/-1/2, +/-1365/4096, +/-1024, +/-65504, min normal, max/min subnormal, negative tiny values |
| Boundary bits | 22 cases: each of 0,1,7,8,31,32,63,64,95,96,127 flipped from all+ and all- |
| Exact mask identity | 129 cases, every mismatch count 0..128; independent scalar +/-1 products |
| Native layouts | 18 cases: 1/2/3 adjacent groups, offsets 0/1/3, packed/+7-byte rows, three rows, immutable padding/suffix sentinels |
| Analytic/input families | 12 cases: zero/signed zero, positive/negative constants, alternating signs, mixed magnitudes, cancellation, sparse zeros, outlier; all fixed depths and selected signed/tiny scales |
| Adaptive/consumption | 13 cases: zero/constants, depth1/2/3, exact boundaries and nextafter-below, thresholds 0/1, inactive poison |
| API rejections | 38 named invalid cases plus one multi-entrypoint non-finite case |
| Deterministic random | 32 cases, three activation groups and three weight rows each; all fixed depths and four threshold pairs |

Hand-derived `[0,0,0,4]` repeated 32 times gives E0=512:

| Depth | New alpha | New signs per quartet | Final residual per quartet | x_hat |
|---|---|---|---|---|
| B1 | 1 | ++++ | -1,-1,-1,3 | 1,1,1,1 |
| B2 | 1.5 | ---+ | .5,.5,.5,1.5 | -.5,-.5,-.5,2.5 |
| B3 | .75 | ++++ | -.25,-.25,-.25,.75 | .25,.25,.25,3.25 |

e1=.75 and e2=.1875 are exactly representable. Equality stops; nextafter toward
zero continues. For d=1, all+ weights give original dot 128 and B1/B2/B3
128/32/128. Repeating ---+ weights give integer plane dots -64/128/-64 and
outputs -64/128/80. d=-1 flips these outputs. The separate repeated
`[1,2,3,4]` fixture has alphas 2.5/1/.5 and exact B3 reconstruction.
All-positive/all-negative unit weights × all-ones x give exact +/-128 through all
three dot routes. These analytic checks use exact equality, not the random bound.

## Numerical comparisons

Decode values, zero signs, mask bytes, integer dots and analytic values/depths
are checked exactly. Random adaptive depth is checked exactly against explicit
fixed-depth residual-energy decisions using the same sequential FP32 contract.
Original-FP32 row-dot checks require exact equality in the specified accumulation
order. This validates the traversal/order alongside the independent analytic
expected outputs; it is not a claim of exact real-number summation.

For the two rearranged approximation routes, let u=2^-24,
gamma(n)=n*u/(1-n*u), N=row columns, G=groups, and

```text
L = sum_g abs(d_g) * 128 * sum_j abs(alpha_gj)
bound = [gamma(N+8) + gamma(G+8)] * L
        + (N+G+16) * FLT_TRUE_MIN
```

Tests require abs(mask_output-reconstructed_output)<=bound. N+8 covers <=3
reconstruction additions, weight multiplication and the N-term scalar accumulation;
G+8 covers <=3 plane products/sums, group scaling and group accumulation. L is an
absolute-contribution scale so cancellation near zero cannot defeat a relative-only
test. The tiny absolute term covers gradual-underflow rounding; it is not a
generic epsilon. For these bounded N<=384 fixtures, both gammas are small and
positive. Coefficients are the same stored FP32 coefficients on both routes, so
coefficient-construction error is not counted twice as route disagreement.
Hand-derived alpha/residual/reconstruction fixtures remain exact. The checker
uses double only to evaluate the tolerance/comparison, never to define the oracle.

This bound is fixture-specific accounting, not a universal GPU acceptance epsilon.
Future tests must account for their accumulation order/length; keep exact mask and
analytic checks alongside any scaled tolerance.

## Determinism, diagnostics and negative controls

[fixtures.h](../../../tests/k05/fixtures.h) is the reusable fixture schema: Group,
Mask, native-half byte writer, repeat-pattern constructor, xorshift32 generator
and native row/group dump. Default seed is **0x004b3035 = 4927541**. Its unsigned
32-bit shifts are 13/17/5; random activations use
`float(int(next()%8193)-4096)/127.0f`; sign bytes use next()&255.
The fixed draw order, scale list and dimensions are in reference_tests.cpp.
No standard-library random distribution or model input is used.

Failures write stderr and return 1. Dumps carry seed, case, row/group, shape,
offset/stride, half bits, block bytes, exact hexfloat activations, depth/thresholds
and relevant expected/actual values. Adaptive boundary thresholds are emitted as
hexfloat, preserving nextafter distinctions. Negative cases identify the versioned
named mutation and its overrides, rather than claiming stale default inputs.
A seed reruns the complete small suite; a negative/injection name is a direct
reproducer. Invalid seed/options also fail nonzero.

The Python [negative_controls.py](../../../tests/k05/negative_controls.py) harness
launches all 38 API rejection cases and four deliberately wrong expectations.
It requires process exit 1 plus diagnostic markers (including input dump and
expected/actual for injected mismatches). Thus CTest success cannot conceal a
failure path that exits zero. The harness itself returns zero only when all
42 controls behave as expected. Selected raw failures are retained in the
[clean transcript](evidence/clean-checkout.txt).

## Reproduction commands and actual results

From an ordinary checkout containing the K03 import history, native Windows:

```powershell
python scripts/verify_upstream.py
cmake -S tests/k05 -B ../build-k05 -G "Visual Studio 17 2022" -A x64
cmake --build ../build-k05 --config Release --parallel 2
ctest --test-dir ../build-k05 -C Release --output-on-failure
& ../build-k05/Release/k05-reference-tests.exe
python tests/k05/negative_controls.py ../build-k05/Release/k05-reference-tests.exe
& ../build-k05/Release/k05-reference-tests.exe --seed 1
& ../build-k05/Release/k05-reference-tests.exe --seed 4294967295
& ../build-k05/Release/k05-reference-tests.exe --inject decode
& ../build-k05/Release/k05-reference-tests.exe --reject half-inf
```

The last two commands intentionally exit **1**; all preceding commands expect 0.
For an exact adaptive-boundary failure, rerun the default suite; its analytic
trials do not depend on the seed. Other single-config C++17 generators may use the
same CMake source; use their executable location and omit --config/-C as appropriate.
Only the stated Windows/MSVC host was executed.

K03 regression, unchanged root entry point, from the same checkout:

```powershell
cmake -S . -B ../build-k05-k03 -G "Visual Studio 17 2022" -A x64
cmake --build ../build-k05-k03 --config Release --target k80nsai-host-smoke --parallel 2
ctest --test-dir ../build-k05-k03 -C Release --output-on-failure
& ../build-k05-k03/Release/k80nsai-host-smoke.exe
python scripts/check_workflow.py
python scripts/check_workflow.py --live
git diff --check fcf54b8671035b3959e851d842bbf5c8e7ba2fe7
git diff --exit-code 25ef5ba867cdc6bc0089d9cb05e7d89090f71560 HEAD -- vendor/llama.cpp
git status --porcelain=v1
```

Development Release build: K05 CTest 3/3; default 282 cases / 35,339 checks;
42 process controls passed. Source/provenance and local/live workflow checks passed.
The independent clone passed K05 3/3 and K03 2/2; after the final diagnostic fix,
K05 was rebuilt and all tests/seeds passed again ([final-source.txt](evidence/final-source.txt)).
Detailed clean-checkout results and command exits are recorded in STATUS and both transcripts. Build
products remain outside source. No root build integration was necessary: K04
PR #26 remained open, so the standalone test project avoids its owned files.
A future root integration can add the reference library and test targets without
changing their mathematical API or introducing CUDA.

## Handoff and limitations

After maintainer acceptance of K05, K08 can start using this host reference while
defining its own runtime eligibility/descriptor. K09 consumes the fixed encoder's
planes/sign bytes/alphas and exact mask-dot fixtures; K10/K11 compare both output
routes, retaining original-FP32 dot as a separate baseline. K12 consumes the exact
adaptive boundaries and computed/consumed depth semantics. K17 reuses storage
offset/stride, unsupported-tail/dtype and failure fixtures. K07 consumes the native
layout evidence. Their other dependency and hardware/model gates remain intact.

Keep the oracle independent when wiring later code: translate GPU results into
these host values for comparison; do not replace its decoder, half conversion,
encoder or scalar accumulation with the implementation under test. Reuse the
recorded seed/dump when a defect appears. No downstream task was started.

Evidence is **source-inspected, implemented and host-tested** only. No CUDA/native
sm_37 compilation, K80 execution, model acquisition/inference, performance or
language-quality conclusion follows. The suite is deliberately bounded (96 random
activation groups / 288 random native weight groups), not exhaustive fuzzing.
Linux/macOS/big-endian hosts, cross-endian serialized inputs, arbitrary floating
environments and runtime GPU eligibility remain untested/outside this API.
# Binary-basis kernel contract

## Native weight layout — inspect, do not assume

Observed at PrismML revision `d8f26eec76da6d09bb708bcba51ef64b8cd868a3`: `QK1_0=128`; `block_q1_0` is an FP16 `d` followed by `uint8_t qs[16]`, with a static assertion of **18 bytes**. Bit `i` is `(qs[i/8] >> (i%8)) & 1`; 1 means `+d`, 0 means `-d`. Source links are in SOURCES_AND_DECISIONS.

This is not the previous synthetic `uint32_t bits[4]; float scale;` format. Preserve the selected revision's ABI and group scale exactly. Read row/view byte strides from the tensor contract. The 18-byte stride and two-byte sign-array offset mean arbitrary casts to aligned `uint32_t*` are unsafe. Start with safe byte assembly or a proven unaligned-safe source helper; an explicitly accounted aligned repack is optional later. Test alternating group alignment and nontrivial row strides.

## Definition

For row r, group g and i=0..127, weights are `w[r,g,i] = d[r,g] q[r,g,i]`, with `q` in {-1,+1}. Let x be the real graph activation group (initially supported FP32; optional FP16 input may convert to FP32). Do not unnecessarily quantize x to Q8 before constructing bases.

Initialize `r0=x`. For j=0..p-1:

```
sj[i] = +1 if rj[i] >= 0 else -1
alpha_j = sum_i(abs(rj[i])) / 128
r(j+1)[i] = rj[i] - alpha_j * sj[i]
x_hat = sum_j(alpha_j * sj)
```

Use FP32 coefficients and residual arithmetic initially. Define sign(0)=+1; an all-zero group must produce zero output, with zero coefficients (depth zero is an optional explicitly supported encoding). Preserve the actual weight scale even if zero or signed. Exclude/reject non-finite inputs according to the runtime policy; do not conceal them.

For masks W and Sj with bit 1 denoting +1:

```
dot_j = 128 - 2 * sum(word=0..3, popcount(W[word] XOR Sj[word]))
y[r]  = sum_g d[r,g] * sum_j alpha[g,j] * dot[r,g,j]
```

This avoids complement-width mistakes in XNOR. `dot_j` is an exact integer in [-128,128]. Convert/apply coefficients and scale once per group/plane as appropriate, accumulate FP32. Padded inactive bits must not contribute: either accept only full verified g128 groups or use a documented valid mask/count. Unsupported tails fall back; do not invent padding in native tensors.

## Common representation and implementation

K08 publishes one internal descriptor for group count, plane count, mask/alpha offsets, valid lanes, dtype/strides and owning stream/generation. K09 implements one parameterized encoder for fixed p=1,2,3. K10 implements the shared POPC consumer; K11 extends that consumer to p=2,3 rather than creating unrelated code.

Initial encoding is once per input tensor operation, reused across output rows. Packing separately for every output row defeats the design. Reuse across distinct operations is optional and must obey generation-safe lifetime rules in INTEGRATION. Avoid host transfers and hot-path allocation. FP32 graph outputs remain the boundary between linear and nonlinear operators.

Warp ballot/reduction may create sign masks, but all participating lanes and synchronization must satisfy the selected CUDA/Kepler implementation. Never read inactive lanes or put a block barrier behind a nonuniform return. Test a safe scalar/CPU oracle first. Native sm_37 code, not merely a CUDA-looking source file, is required for the target claim.

## Adaptive mode

Let E0 = sum_i x[i]^2. For a zero group, emit the supported zero representation. Construct B1; compute e1 = sum_i r1[i]^2 / E0. Stop if e1 <= tau1. Otherwise construct B2; compute e2 = sum_i r2[i]^2 / E0 and stop if e2 <= tau2. Otherwise use B3. Expose finite thresholds in [0,1], with documented defaults as **screening choices, not learned optima**.

Use stable FP32 accumulation for the expected finite range; if the energy computation overflows, report/fallback explicitly rather than treating NaN as a routing choice. Small negative energies from an algebraic shortcut must be explained and bounded; direct residual reduction is the initial reference.

Do not build all three planes then claim encoding work was skipped. Report both computed planes and consumed planes. A fixed-capacity three-plane buffer with per-group depth is acceptable initially, provided the inactive work is not executed by the consumer and allocation bytes are reported honestly. Group depth is independent of output row, so coordinate work to avoid unnecessary lane divergence. Reduced local residual error is not proof of better language or faster execution.

## Minimal correctness fixtures

- all + signs/all +1 activations; all - weights/all +1; alternating signs;
- all-zero x, constant positive/negative x, mixed magnitudes, outlier and cancellation groups;
- bit positions 0,1,7,8,31,32,63,64,95,96,127;
- adjacent native 18-byte groups, two rows, nontrivial byte strides;
- scales zero, fractional, negative where representable; group boundaries;
- deterministic random finite x/W cases for fixed p=1,2,3 and adaptive depth extremes;
- unsupported dtype/shape fallback, ragged M, and poisoned scratch buffers.

Independent CPU reconstruction computes x_hat and then a straightforward dot with decoded native weights. GPU mask dots must match exactly; scaled results use an explicit sensible FP32 tolerance with failing case dump. This is a small debugging suite, not the retired 100,000-group research prerequisite.

## Performance candidates, not mandates

Try a small number of real-shape mappings: warp-per-row, subwarp-per-row, several rows/block, cached versus shared activation reuse. Weight rereads in source do not prove DRAM rereads; shared staging can add barriers and lose. Inspect relevant generated code when necessary, but no architecture roofline dissertation is required. Fewer POPCs does not guarantee end-to-end speedup when weights, other operators, launches or encoding dominate.

## Optional sign/magnitude extension

X01 may represent x_hat[i]=s[i](alpha+beta*m[i]), m in {0,1}. If admitted later, the masked dot is `popcount(M) - 2*popcount((W XOR S) AND M)`, summed over words; it is not an ordinary binary dot of `S AND M`. The mask-only count can be reused across rows. This correction preserves the idea without the earlier ambiguous sign/mask algebra.

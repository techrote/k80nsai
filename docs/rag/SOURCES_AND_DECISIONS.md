# Source register and design corrections

Audit date: 2026-09-12. This records direct source observations and engineering decisions. It does not certify a working GPU build. The earlier generated v2 benchmark files were not present in the current working filesystem; this workflow reconciles the visible conversation and freshly inspected sources, not a newly repeated audit of those missing files or their claimed exhaustive tests.

## K01 selection update — 2026-09-26

The candidate-only status of S1/S2 below is superseded by the [K01 source decision](../tasks/K01/SOURCE_MAP.md), proposed for maintainer acceptance: mainline `56381e407c0ccfb3a6f71e668a27a901001d22ce`. The source map and five focused notes contain the current verified comparison; the original register remains provenance, not a second runtime selection. Source inspection establishes neither CUDA11 compilation nor K80 inference. [Status and activation](../tasks/K01/STATUS.md) distinguish research, publication and downstream readiness.

## Primary sources and observed pins

**S1 — Prism runtime candidate.** Repository default branch observed as `prism`; head observed as `d8f26eec76da6d09bb708bcba51ef64b8cd868a3`.

- https://github.com/PrismML-Eng/llama.cpp/tree/d8f26eec76da6d09bb708bcba51ef64b8cd868a3
- https://github.com/PrismML-Eng/llama.cpp/blob/d8f26eec76da6d09bb708bcba51ef64b8cd868a3/ggml/src/ggml-common.h
- https://github.com/PrismML-Eng/llama.cpp/blob/d8f26eec76da6d09bb708bcba51ef64b8cd868a3/ggml/src/ggml-cuda/dequantize.cuh

Directly inspected: QK1_0=128, FP16 scale plus 16 packed bytes, size assertion 18, LSB-first element decoding, bit 1 -> +d. Consequence: actual ABI/alignment tests are required; the synthetic FP32-scale struct must not be reused as native GGUF storage.

**S2 — Mainline candidate.** `ggml-org/llama.cpp` master head observed as `56381e407c0ccfb3a6f71e668a27a901001d22ce`; this is a candidate snapshot, not a tested target pin.

- https://github.com/ggml-org/llama.cpp/tree/56381e407c0ccfb3a6f71e668a27a901001d22ce
- https://github.com/PrismML-Eng/Bonsai-demo/blob/main/README.md

Prism's current demo documents upstream binary Q1 support. K01 may choose mainline when it reduces compatibility work while retaining the real target architecture. Do not assume a fresh Prism fork is always required.

**S3 — External Kepler patch reference.** Observed `babal35/llamacpp-kepler` master SHA `b367989573b1a97e37b001cbf6718365844b94ca`.

- https://github.com/babal35/llamacpp-kepler/blob/b367989573b1a97e37b001cbf6718365844b94ca/KEPLER.md

Its author documents a CUDA 11.4/Linux/470.256.02 experiment and build/BF16 gating changes. Treat as a patch lead and third-party execution report, not our evidence or a guarantee for current Bonsai. Its architecture-list example adds virtual code; our artifact requirement is native sm_37.

**S4 — Binary model cards / acquisition leads.**

- https://huggingface.co/prism-ml/Bonsai-1.7B-gguf
- https://huggingface.co/prism-ml/Bonsai-27B-gguf
- https://prismml.com/news/bonsai-27b

The 27B card describes hybrid attention and distinguishes optional vision/drafter components. Its displayed architecture tag and prose need reconciling with actual GGUF metadata, not guessing. Weight footprint and published non-K80 speeds are not K80 fit or throughput evidence. K06 must pin actual file revision and SHA-256.

**S5 — NVIDIA toolchain compatibility.**

- https://docs.nvidia.com/cuda/archive/11.8.0/cuda-toolkit-release-notes/index.html
- https://docs.nvidia.com/deploy/cuda-compatibility/minor-version-compatibility.html
- https://docs.nvidia.com/cuda/archive/11.8.0/cuda-installation-guide-microsoft-windows/index.html
- https://docs.nvidia.com/cuda/wsl-user-guide/index.html

Use exact host/compiler/toolkit/driver support tables and actual compile/run probes. CUDA minor compatibility and toolkit-bundled driver versions are distinct. WSL's supported hardware range is not Kepler. Tool compatibility must be checked separately from compiler compatibility.

## Decisions after reconciling the conversation

**D01 — Real models first.** Keep the final agreed CLI/Poc direction. Retire the formal four-kernel synthetic benchmark as a prerequisite. Preserve only small diagnostic tests and honest timing.

**D02 — No unsupported effort forecasts.** Earlier numerical effort ratios and implied speedups were estimates without implementation evidence. Do not use them as schedule or throughput commitments. K80 compatibility is the first uncertainty.

**D03 — Actual storage beats synthetic convenience.** Native groups use observed 18-byte FP16-scale ABI, not 20-byte FP32-scale layout. Loading/repacking must respect byte alignment and row/view strides.

**D04 — Earlier 27B admission.** A smaller model does not exercise the same graph. Attempt reference 27B as soon as base runtime works. Do not require good B1 output before this test or discard 27B because small-model B1 collapses.

**D05 — Approximation has a specification.** Test GPU arithmetic against CPU reconstruction of the chosen basis approximation. Divergent reference text is allowed; wrong implementation of the intended approximation is not.

**D06 — Shared arithmetic, not five unrelated backends.** One encoder parameterized by plane count and one POPC consumer reduce effort. Adaptive must count computed versus consumed planes and avoid claiming savings from unused already-computed work.

**D07 — Scope and coverage first.** Start with eligible Q1 single-column linear operations; explicitly distinguish decode phase from single-column shape. Retain non-Q1/unsupported paths. Report coverage and permit minimal layer/role exclusion for sensitivity checks.

**D08 — Preserve state semantics.** Switch modes via a fresh context or complete replay. Clear recurrent/linear-attention state as well as KV, prompt and prepared-basis caches. No pointer-only activation caching.

**D09 — Same-device preprocessing.** No per-layer GPU0/GPU1 round trips. Autoregressive and layer dependencies prevent effortless overlap. The second GPU can later host independent workers, but board-level contention must not contaminate initial timing.

**D10 — No mandatory shared-memory dogma.** Test reuse mappings, but do not infer DRAM traffic from logical loads or assume shared-memory staging beats cache. Fewer instructions can leave bandwidth, attention or launch overhead unchanged.

**D11 — Measure the product, not only the primitive.** Unprofiled actual token throughput, fixed policy, small repeat count and saved outputs are required. Full formal statistics, new profilers, giant test campaigns and a proof report are not.

**D12 — Fresh-prefix quality comparison.** Teacher forcing is required for meaningful aligned logit comparison; free-running text is judged as text. Same seed alone does not hold histories equal.

**D13 — Preserve speculative ideas without scope creep.** FP64, sketches, adaptive lossy packing, braided streams and persistent compressed hidden state remain opt-in. Sketch-domain language survival, unchanged attention with permanently binary state, and easy multi-stream carry contamination were hypotheses, not established techniques.

**D14 — Correct optional Q2 algebra.** For sign/magnitude coding, a masked signed dot needs a mask count and masked mismatch correction, not treating `S AND M` as a normal binary sign mask. KERNELS gives the exact formula.

**D15 — Honest closure and useful negatives.** A complete, slower or semantically broken experiment is valuable. A code-only implementation without target execution is partial. Keep those distinct in issues and final report.

# K01 runtime source map and selection

Purpose: frozen source-selection and source-archaeology entry point for K03 and all downstream tasks. Status: proposed for maintainer acceptance; source-inspected, not built or K80-validated. Verified: 2026-09-26. Repository base: `8deeac521f7a76e92b54bdf2c95d31377b5a4f65`. Assignment: [K01/#2](https://github.com/techrote/k80nsai/issues/2).

## Which runtime is selected and why?

**Select `ggml-org/llama.cpp` at full commit `56381e407c0ccfb3a6f71e668a27a901001d22ce`, observed branch context `master`.** This is a reproducible source decision for K03 import, not a claim that the unmodified runtime builds or runs on Kepler. Runtime identity is the commit, never the moving branch. Retain [LICENSE](https://github.com/ggml-org/llama.cpp/blob/56381e407c0ccfb3a6f71e668a27a901001d22ce/LICENSE) (MIT) and existing third-party notices.

The selected pin contains native Q1_0 CPU/CUDA handling, the small model's `qwen3` graph, the 27B `qwen35` hybrid graph, reference fallbacks, hybrid memory lifecycle and explicit CUDA architecture configuration. Prism contains these too; branding is not a required feature. Its inspected differences add modern-device optimizations and optional graph/state behavior that do not establish a required text-model capability absent from mainline. A material correctness-relevant difference also favors mainline: its Qwen35 GDN Q/K normalization uses `x / sqrt(sum(x*x) + eps)`, while Prism uses `x / max(norm(x), eps)`. Mainline carries the referenced upstream epsilon-placement correction; it is not just a launch optimization. Preserve this reference behavior and verify actual model epsilon in K06/K07. [Pinned formula comparison](research/candidate-comparison-and-import.md#correctness-difference-qwen35-gdn-normalization-is-not-merely-fused-launches). Keeping the upstream pin avoids adopting additional differences without need. Full comparison: [candidate comparison and import](research/candidate-comparison-and-import.md).

This selection remains **conditional on later compatibility validation**, as every source-only pin must. No inspected requirement currently forces Prism or the old Kepler tree. Shared CUDA11/sm37 risks are assigned to K04 and model execution to K07, rather than hidden behind a certainty claim. If those tasks demonstrate a fatal source-specific obstacle, use the documented coordinated repin procedure; do not silently change pins under parallel agents. Exact Q1 GGUF metadata remains a K06 gate and can reveal a material incompatibility requiring that procedure.

## What exact candidates and provenance were inspected?

| Candidate | Branch context | Full commit | Role |
|---|---|---|---|
| [ggml-org/llama.cpp](https://github.com/ggml-org/llama.cpp/tree/56381e407c0ccfb3a6f71e668a27a901001d22ce) | observed `master` | `56381e407c0ccfb3a6f71e668a27a901001d22ce` | Selected import base |
| [PrismML-Eng/llama.cpp](https://github.com/PrismML-Eng/llama.cpp/tree/d8f26eec76da6d09bb708bcba51ef64b8cd868a3) | observed `prism` | `d8f26eec76da6d09bb708bcba51ef64b8cd868a3` | Compared target-capable alternative |
| [babal35/llamacpp-kepler](https://github.com/babal35/llamacpp-kepler/tree/b367989573b1a97e37b001cbf6718365844b94ca) | branch not used as identity | `b367989573b1a97e37b001cbf6718365844b94ca` | Historical patch lead and third-party report |
| [Kepler documented upstream base](https://github.com/ggml-org/llama.cpp/tree/a95a11e5b834057e684712963f90bbb730f4745c) | historical upstream | `a95a11e5b834057e684712963f90bbb730f4745c` | Exact reference diff only, not a fourth runtime candidate |

All four full SHAs resolved through the public GitHub commit API. Exact codeload snapshots were extracted once into a shared immutable research cache outside the documentation worktree. Full directory tip-to-tip comparison was used for candidates, not a merge-base-only PR diff. No runtime source is imported or vendored by this documentation PR. Provenance and observed tree IDs are recorded in [STATUS.md](STATUS.md).

## What does native Q1 mean at the selected pin?

**Source fact:** `QK1_0=128`, a stored FP16 scale followed by 16 sign bytes, total **18 bytes**. Bit i is LSB-first in byte i/8; one means positive stored scale and zero its negation. A native group does not contain a float scale or naturally aligned four-word mask. Preserve signed scale and row/view byte strides. Detailed ABI evidence, alignment reasoning and oracle requirements: [Q1 representation](research/q1-abi-and-activation-path.md#1-native-q1-representation).

**Source fact:** ordinary CUDA MMVQ takes original F32 activations, quantizes them in per-32-element Q8_1 groups, evaluates `vec_dot_q1_0_q8_1`, and accumulates/writes FP32. The future B1/B2/B3 encoder must consume the original graph values before that quantization. The CPU runtime Q1 dot uses Q8_0 and is not an independent exact-F32 oracle. K05 supplies that oracle; K08 owns the actual interface/seam decision. [Activation map](research/q1-abi-and-activation-path.md#2-f32-activation--q8_1-reference--f32-output).

**Source inference:** for ordinary eligible Q1/F32/F32 at cc370, source predicates select MMVQ for 1–8 columns; larger inputs fall through MMQ rejection to dequantized F32 cuBLAS, absent overrides. The custom experiment still admits only the contract's eligible single-column operations. Native DP4A is not a prerequisite of the Q1 helper: scalar signed-byte emulation exists below cc610. This does not prove toolkit compilation. [Dispatch and fallback](research/q1-abi-and-activation-path.md#3-cc370-dispatch-and-fallback-conclusions).

## Why are small and 27B separate acceptance gates?

Public small-model metadata identifies Qwen3 dense attention; the 27B card describes Qwen3.6-derived hybrid attention while the repository GGUF API identifies `qwen35`. Source `qwen35.cpp` includes full attention, convolution/recurrent state and Gated Delta Net. K06 must verify the exact acquired Q1 file header, tensor types, dimensions and template; API aggregate metadata may describe another GGUF variant. Provider-advertised file hashes are acquisition leads, not local checksum results. [Model identities and graph map](research/small-and-27b-operator-paths.md#model-identity-bounded-public-evidence).

Both candidates advertise CUDA fused GDN without a Kepler-specific rejection. Mainline defaults fused AR/CH on and auto-resolution off; Prism enables placement-based auto-resolution. Neither exposes a public force-decomposition flag. Mainline's default fused F32 kernel is the first bounded compile/run candidate. If it fails, a reviewed diagnostic change must test the replacement graph: single-token recurrence differs from multi-token CS64 chunking; the latter includes cumulative sums, triangular operations and a 64x64 cuBLAS solve. CPU backend implementations exist but a failed CUDA launch is not automatic safe fallback. [GDN selectors and replacement](research/small-and-27b-operator-paths.md#fused-versus-decomposed-gdn-actual-selectors-not-an-invented-flag), [operator matrix](research/small-and-27b-operator-paths.md#cc-370-operator-matrix-and-real-fallback-limits).

Optional vision, drafter/DSpark, MTP/speculation and non-target graph optimizations do not enter this text POC. Small success never establishes 27B memory fit, operator compatibility, recurrent reset or language quality.

## Where are the implementation seams and who owns them?

| Question | Inspected source symbols / paths | Consumer and canonical detail |
|---|---|---|
| Native storage and independent arithmetic | `block_q1_0`, `quantize_row_q1_0_ref`, `dequantize_row_q1_0`; ggml-common.h / ggml-quants.c | K05; [ABI note](research/q1-abi-and-activation-path.md) |
| Original activation and reference dot | `ggml_cuda_mul_mat`, `ggml_cuda_mul_mat_vec_q`, `quantize_row_q8_1_cuda`, `vec_dot_q1_0_q8_1` | K08 owns seam/config; K09–K12 consume its accepted API; [activation note](research/q1-abi-and-activation-path.md) |
| Architecture and library compatibility | CUDA CMakeLists, vendors/cuda.h, common.cuh, templated `ggml_cuda_mul_mat_cublas_impl` and final compute-type selection | K04; [build matrix](research/cuda11-sm37-compatibility.md) |
| Hybrid graph and actual fallback | `llama_model_qwen35`, `llm_build_delta_net_base::build_delta_net`, `ggml_cuda_op_gated_delta_net`, `ggml_cuda_op_solve_tri` | K06 identities, K07 execution, K04 compatibility; [model/operator note](research/small-and-27b-operator-paths.md) |
| Clear versus remove versus recreate | `llama_memory_hybrid::clear`, `seq_rm`, public memory APIs, context/sampler/application lifecycle | K08 contract/K13 application/K17 regressions; [state note](research/state-cli-and-device.md) |
| Explicit single device and evaluation phase | model `devices`, `split_mode`, `main_gpu`, actual CLI/context construction and phase signals | K08/K13/K14; [device and CLI note](research/state-cli-and-device.md) |

These are source entry points, not a predeclared K08 API. K08 must publish actual implemented symbols, descriptor layout, generation/stream ownership, complete reset rules, role/phase/device eligibility and telemetry hooks before consumers code against them.

## What is the bounded build strategy?

Use an already available, explicitly compatible native host/CUDA11/compiler tuple from the [primary-source toolchain matrix](research/cuda11-sm37-compatibility.md#toolchainhostdriver-options-from-primary-sources). Both pins already honor explicit `CMAKE_CUDA_ARCHITECTURES`; no default architecture-list patch is needed for `37-real`. Plain `37` requests native plus virtual code; `37-virtual` alone is insufficient. Inspect the actual backend artifact for native sm_37 SASS/cubins.

The Kepler fork changes only two code/build files relative to its documented base, plus docs. Its BF16 predicate predates the selected pin's templated compute-type path and must be translated semantically. Runtime-disabled features still leave headers, translation units and template instances to compile. The selected pin also has batched Ex cuBLAS calls needing a separate cc370 probe/possible typed-F32 route; merely forcing F32 does not solve every library call. [Bounded K04 probe sequence](research/cuda11-sm37-compatibility.md#blockeroptions-matrix-and-bounded-k04-sequence).

No compiler, driver or host mutation was performed. No CUDA binary, K80 execution or performance result exists from this research run.

## Which unresolved questions block later work?

| Unresolved question | Owner / precise resolution | Gate preserved |
|---|---|---|
| Exact selected-source import and root layout | K03 verifies tracked snapshot/lock/licence and clean host configuration | K04/K05/K06 independent after accepted import |
| CUDA11 headers, all TUs/templates, native target artifact | K04 staged probes and narrow failure-driven fixes; inspect backend SASS | `sm37-compiled` required, not configure-only |
| BF16 overrides and batched/broadcast Ex on GK210 | K04 minimal typed-route tests/patch; K07 actual graph execution | Build evidence separated from library/device execution |
| Actual Q1 metadata, recurrent dimensions and templates | K06 full-file checksums and bounded GGUF inspection for each model | Small artifact published early; full K06 still needs both |
| Fused GDN, replacement graph, reference outputs and peak memory | K07 default fused first; failure-specific reproducer and reviewed fallback test | Small reference artifact unblocks K10 without closing 27B |
| Complete application reset, phase classification and device policy | K08 publishes one interface; K13 implements chat; K17 tests | No stale context, inferred decode-only or multi-GPU shortcut |
| Accuracy of approximation and actual speed/usefulness | K09–K19 oracle/target/model work, then K21 measured report | No numerical performance forecast from this source map |

Exact retained issue mapping, output paths, readiness and blocked continuation: [IMPLEMENTATION_HANDOFF.md](IMPLEMENTATION_HANDOFF.md). Task-specific executable refinements: [WORK_ORDERS.md](WORK_ORDERS.md). Research acceptance and implementation readiness are separate.

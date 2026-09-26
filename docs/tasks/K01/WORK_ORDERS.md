# Source-backed implementation work orders

Purpose: durable execution refinements for retained issues K03–K21. Status: proposed until the K01 documentation PR is accepted. Verified: 2026-09-26. Read the assigned issue's original objective/acceptance plus its section below; no required acceptance is removed. [Handoff and activation rules](IMPLEMENTATION_HANDOFF.md) define live-state checks, common evidence metadata, concurrency, blocked-state rules and the identity mapping. [SOURCE_MAP.md](SOURCE_MAP.md) is the sole source-selection decision.

All new output paths below are requirements for future implementation, not files claimed to exist. Exact APIs belong to K08; runtime compilation to K04; actual model-file inspection to K06; hardware reference evidence to K07. Root build/experimental commands do not exist yet unless their predecessor artifact demonstrates otherwise.


## K03 — issue #3

**Objective and completion boundary:** Import the accepted exact runtime revision, retain its licence and notices, and establish a clean reproducible workspace. No CUDA patches, model acquisition or experimental arithmetic in the import commit.

**Inputs and prerequisites:** Completed K01 source selection after documentation acceptance. Use the selected source in SOURCE_MAP and the accepted K03 lock/patch ledger; never a moving upstream head. Actual APIs/commands must come from predecessor outputs.

**Required reading and evidence:** Read AGENTS.md, then [the relevant source investigation](research/candidate-comparison-and-import.md), [the handoff](IMPLEMENTATION_HANDOFF.md#what-must-every-future-agent-do-before-editing), and the assigned issue's original ordered contract pack. The source note contains inspected symbols, exact paths/line ranges and pinned primary references. Those facts establish the gap; unexecuted build/model conclusions remain explicitly unresolved.

**Bounded implementation instructions:** Keep the unmodified snapshot and local integration changes in separate commits. Publish an upstream lock with repository, full SHA, branch context, retrieval command, tree verification and licence paths. Use tracked vendor/llama.cpp by default; document any equally reproducible alternative before dependent work. Preserve root documentation and isolate upstream automation: do not promote the vendor tree's CI workflows into root .github/workflows. Establish a root host build/test entry point without claiming CUDA success. Keep upstream and K80NSAI implementation commits separately labelled: nested vendor build-info Git commands can discover the outer repository, so a bare --version hash is not upstream provenance. Do not silently enable LLAMA_USE_SYSTEM_GGML or mix backend binaries from another pin.

**Validation and failure diagnosis:** From a clean checkout verify the lock resolves the exact source, compare imported files to that pin (record deliberate exclusions), run the established root host configure/build smoke and python scripts/check_workflow.py. Expected: no moving branch dependency, retained licences and reproducible source/build entry point. Do not invent root build commands before defining them.

**Evidence and output contract:** docs/tasks/K03/IMPORT.md plus a tracked upstream lock and clean-checkout transcript. Also publish STATUS.md with the common metadata and exact failure/continuation information in the [output contract](IMPLEMENTATION_HANDOFF.md#what-is-the-evidence-and-output-contract).

**Completion and blocked state:** Close only after the original issue's full acceptance plus these checks is evidenced and reviewed. If a required artifact/toolchain/device/model is absent, leave the gate pending, identify the exact missing input and next command, preserve useful partial work and continue only independent work. A documentation PR or implementation merge is not an execution pass.

**Handoff and concurrency:** K04, K05 and K06 may run independently after accepted import; K08 also needs K05. Check live claims and the atlas before edits; do not overlap the sole import, shared interface or sequential consumer owner. Reserve the entire K80 board for headline measurements. No self-merge or privileged host changes are authorized.

## K04 — issue #4

**Objective and completion boundary:** Produce a compiled native sm_37 reference runtime with narrowly justified compatibility patches. Do not infer model execution from build success.

**Inputs and prerequisites:** Completed K03 reproducible import. Use the selected source in SOURCE_MAP and the accepted K03 lock/patch ledger; never a moving upstream head. Actual APIs/commands must come from predecessor outputs.

**Required reading and evidence:** Read AGENTS.md, then [the relevant source investigation](research/cuda11-sm37-compatibility.md), [the handoff](IMPLEMENTATION_HANDOFF.md#what-must-every-future-agent-do-before-editing), and the assigned issue's original ordered contract pack. The source note contains inspected symbols, exact paths/line ranges and pinned primary references. Those facts establish the gap; unexecuted build/model conclusions remain explicitly unresolved.

**Bounded implementation instructions:** Follow the note's staged probes: exact supported host/toolkit pairing; explicit real architecture configuration; complete CUDA translation-unit build; native-code inspection; separate library/runtime probes on authorized K80 later. Start with CMAKE_CUDA_ARCHITECTURES=37-real when supported; plain 37 includes real and virtual, while 37-virtual alone fails this task. Translate BF16/compute-type selection semantically in the templated cuBLAS code. Inspect batched and broadcast Ex calls, modern headers, source globbing/generated instances and compile-time guards. Disabling a runtime route does not eliminate its compilation. Preserve reference fallback and coordinate shared ggml_cuda_mul_mat edits with K08; propose typed SGEMM fallback only from a concrete probe. Do not copy the historical two-line patch blindly.

**Validation and failure diagnosis:** Record cmake/nvcc/host compiler versions, complete configure/build commands and exit codes, relevant verbose compile commands and cuobjdump native sm_37 evidence for the final backend artifact. CPU host tests must still pass. A toolkit-only configuration, PTX-only artifact, build of an unrelated target or other-GPU run fails native-target acceptance. Record first compile failure by file/symbol and rerun after each narrow change.

**Evidence and output contract:** docs/tasks/K04/BUILD.md with toolchain manifest, patch ledger, compile log and native artifact hash/inspection. Also publish STATUS.md with the common metadata and exact failure/continuation information in the [output contract](IMPLEMENTATION_HANDOFF.md#what-is-the-evidence-and-output-contract).

**Completion and blocked state:** Close only after the original issue's full acceptance plus these checks is evidenced and reviewed. If a required artifact/toolchain/device/model is absent, leave the gate pending, identify the exact missing input and next command, preserve useful partial work and continue only independent work. A documentation PR or implementation merge is not an execution pass.

**Handoff and concurrency:** K07 and K09; actual K80 library/model behavior remains K07, with compatibility fixes owned here. Check live claims and the atlas before edits; do not overlap the sole import, shared interface or sequential consumer owner. Reserve the entire K80 board for headline measurements. No self-merge or privileged host changes are authorized.

## K05 — issue #5

**Objective and completion boundary:** Build the independent CPU oracle for native Q1 decoding and the contract's intended basis approximation; do not reproduce the reference Q8_1 activation error as the basis definition.

**Inputs and prerequisites:** Completed K03 reproducible import. Use the selected source in SOURCE_MAP and the accepted K03 lock/patch ledger; never a moving upstream head. Actual APIs/commands must come from predecessor outputs.

**Required reading and evidence:** Read AGENTS.md, then [the relevant source investigation](research/q1-abi-and-activation-path.md), [the handoff](IMPLEMENTATION_HANDOFF.md#what-must-every-future-agent-do-before-editing), and the assigned issue's original ordered contract pack. The source note contains inspected symbols, exact paths/line ranges and pinned primary references. Those facts establish the gap; unexecuted build/model conclusions remain explicitly unresolved.

**Bounded implementation instructions:** Recheck imported block_q1_0 and source strides. Decode 18-byte blocks using FP16 scale at offset 0 and 16 packed bytes at offset 2, bit 1 positive and LSB-first. Keep byte decoding and direct decoded-weight dot independent from GPU mask logic. Use original finite graph activation values, sign(0)=+1, FP32 residual coefficients and documented accumulation tolerance. Never cast arbitrary native groups to aligned uint32_t pointers. Publish helper/fixture interfaces for K08/K09 without designing the GPU descriptor.

**Validation and failure diagnosis:** Run the full small fixture set in KERNELS: every listed boundary bit, adjacent 18-byte blocks, odd/even group alignment, row/view strides, zero/signed/fractional scales, zero/constant/outlier/cancellation inputs, fixed depths and adaptive extremes. Inject unsupported tails/dtypes and non-finites; require explicit rejection or recorded fallback. Integer mask sums exact, scaled output tolerance justified. Failures return nonzero with seed/input dump.

**Evidence and output contract:** docs/tasks/K05/REFERENCE.md, reusable host oracle, fixture schema and exact test command. Also publish STATUS.md with the common metadata and exact failure/continuation information in the [output contract](IMPLEMENTATION_HANDOFF.md#what-is-the-evidence-and-output-contract).

**Completion and blocked state:** Close only after the original issue's full acceptance plus these checks is evidenced and reviewed. If a required artifact/toolchain/device/model is absent, leave the gate pending, identify the exact missing input and next command, preserve useful partial work and continue only independent work. A documentation PR or implementation merge is not an execution pass.

**Handoff and concurrency:** K07, K08, K09–K12 and K17; no K80 claim. Check live claims and the atlas before edits; do not overlap the sole import, shared interface or sequential consumer owner. Reserve the entire K80 board for headline measurements. No self-merge or privileged host changes are authorized.

## K06 — issue #6

**Objective and completion boundary:** Acquire and inspect the two exact text GGUFs with reproducible identity and template records; public card/API metadata are acquisition leads, not local file validation.

**Inputs and prerequisites:** Completed K03 reproducible import. Use the selected source in SOURCE_MAP and the accepted K03 lock/patch ledger; never a moving upstream head. Actual APIs/commands must come from predecessor outputs.

**Required reading and evidence:** Read AGENTS.md, then [the relevant source investigation](research/small-and-27b-operator-paths.md), [the handoff](IMPLEMENTATION_HANDOFF.md#what-must-every-future-agent-do-before-editing), and the assigned issue's original ordered contract pack. The source note contains inspected symbols, exact paths/line ranges and pinned primary references. Those facts establish the gap; unexecuted build/model conclusions remain explicitly unresolved.

**Bounded implementation instructions:** Use the note's pinned small and 27B repository revisions/file leads. Verify actual acquired Q1 file SHA-256, size, licence, general.architecture, tensor types/shapes, tokenizer/template/end tokens and context/recurrent metadata using tools from the accepted import. Reconcile small qwen3 and 27B qwen35 metadata separately; do not use the model card's base_model label as a GGUF architecture. API aggregate gguf size may describe another variant. Do not load optional mmproj/vision/drafter assets. Support partial download recovery and reject hash mismatches. Publish the verified small-model identity/template before 27B acquisition finishes. Record the exact GDN normalization epsilon fields, S_v/head dimensions and convolution channels needed by the source operator predicates.

**Validation and failure diagnosis:** Existing valid file accepted; incomplete file resumes or fails with exact continuation; checksum mismatch rejected; paths with spaces work; metadata extraction runs without remote code execution. Acceptance requires actual local file metadata and checksums for both models. An advertised LFS hash alone is insufficient. Never commit weight bytes.

**Evidence and output contract:** docs/tasks/K06/MODELS.md, small_model_identity_and_template and separate 27B identity/template record with exact manifest and loading commands. Also publish STATUS.md with the common metadata and exact failure/continuation information in the [output contract](IMPLEMENTATION_HANDOFF.md#what-is-the-evidence-and-output-contract).

**Completion and blocked state:** Close only after the original issue's full acceptance plus these checks is evidenced and reviewed. If a required artifact/toolchain/device/model is absent, leave the gate pending, identify the exact missing input and next command, preserve useful partial work and continue only independent work. A documentation PR or implementation merge is not an execution pass.

**Handoff and concurrency:** Small artifact unblocks K07 start and K13 real chat acceptance independently of full K06; 27B artifact permits its early K07 attempt. Check live claims and the atlas before edits; do not overlap the sole import, shared interface or sequential consumer owner. Reserve the entire K80 board for headline measurements. No self-merge or privileged host changes are authorized.

## K07 — issue #7

**Objective and completion boundary:** Validate real reference inference separately on small qwen3 and hybrid qwen35 27B, with one selected GK210. Preserve early small-model evidence even if 27B blocks.

**Inputs and prerequisites:** Completed K04 and K05, plus K06's accepted small_model_identity_and_template artifact. Full closure additionally requires K06 and both actual model runs. Use the selected source in SOURCE_MAP and the accepted K03 lock/patch ledger; never a moving upstream head. Actual APIs/commands must come from predecessor outputs.

**Required reading and evidence:** Read AGENTS.md, then [the relevant source investigation](research/small-and-27b-operator-paths.md), [the handoff](IMPLEMENTATION_HANDOFF.md#what-must-every-future-agent-do-before-editing), and the assigned issue's original ordered contract pack. The source note contains inspected symbols, exact paths/line ranges and pinned primary references. Those facts establish the gap; unexecuted build/model conclusions remain explicitly unresolved.

**Bounded implementation instructions:** Consume K04 native-build and K05 oracle acceptance plus K06 small identity/template artifact. Verify selected GK210 and offload routing, then perform short small reference prefill and generation. Publish small_model_reference_on_K80 immediately. As soon as the separate 27B file is ready, exercise prefill at lengths crossing the chunk path and one-token evaluation; record fused configuration, recurrent/attention state, backend assignments and operator failures. Where fused GDN fails, coordinate a recorded diagnostic internal cparams change or reviewed local configuration surface (neither pin has a public force-decomposition flag), then verify the complete replacement graph including cumsum/tri/solve_tri and cuBLAS dependencies; simply switching off fusion is not acceptance. Keep a modest declared context, reference precision/offload and no vision/drafter. Do not expand the entire model to FP32. Preserve mainline GDN normalization x/sqrt(sum(x*x)+eps); it differs from Prism's max-norm formula. Consume K06's exact epsilon metadata and include a bounded small-norm reference fixture if normalization is affected by compatibility changes.

**Validation and failure diagnosis:** Actual target reference text, meaningful GPU work and same-policy reproducible commands for each model. Record failing node/symbol, shape, dtype, phase and backend for unsupported operators, OOM, library failures or nonfinite outputs. Source support predicates and GPU banners are not execution evidence. Compare reset/replay to fresh context; preserve CPU fallback details for later parity. Missing 27B/K80 evidence leaves full issue open.

**Evidence and output contract:** docs/tasks/K07/REFERENCE_RUNS.md with distinct small and 27B records, raw public outputs and exact operator reproducers. Also publish STATUS.md with the common metadata and exact failure/continuation information in the [output contract](IMPLEMENTATION_HANDOFF.md#what-is-the-evidence-and-output-contract).

**Completion and blocked state:** Close only after the original issue's full acceptance plus these checks is evidenced and reviewed. If a required artifact/toolchain/device/model is absent, leave the gate pending, identify the exact missing input and next command, preserve useful partial work and continue only independent work. A documentation PR or implementation merge is not an execution pass.

**Handoff and concurrency:** Small reference artifact satisfies K10's narrow close gate; full K07 plus later mode inputs unblocks K16. Check live claims and the atlas before edits; do not overlap the sole import, shared interface or sequential consumer owner. Reserve the entire K80 board for headline measurements. No self-merge or privileged host changes are authorized.


## K08 — issue #8

**Objective and completion boundary:** Implement and publish the single shared experimental interface, intercept original activations safely and freeze lifecycle/phase/device/coverage contracts before consumers.

**Inputs and prerequisites:** Completed K03 and K05; their import/oracle artifacts must name accepted commits. Use the selected source in SOURCE_MAP and the accepted K03 lock/patch ledger; never a moving upstream head. Actual APIs/commands must come from predecessor outputs.

**Required reading and evidence:** Read AGENTS.md, then [the relevant source investigation](research/q1-abi-and-activation-path.md), [the handoff](IMPLEMENTATION_HANDOFF.md#what-must-every-future-agent-do-before-editing), and the assigned issue's original ordered contract pack. The source note contains inspected symbols, exact paths/line ranges and pinned primary references. Those facts establish the gap; unexecuted build/model conclusions remain explicitly unresolved.

**Bounded implementation instructions:** Read also state-cli-and-device.md and small-and-27b-operator-paths.md. Use ggml_cuda_mul_mat and the pre-quantize_row_q8_1_cuda input in ggml_cuda_mul_mat_vec_q as observed seams, then choose the narrowest correct routing point in the accepted import. The choice must account for fused MMVQ and architecture fallback routes; mmvq presence alone does not prove cc370 reaches it. Basis inputs are original supported F32 graph values, never dequantized Q8_1. Publish actual tensor dtype/layout/nb/view/group eligibility, policy (single-column unless a real semantic phase signal is implemented), caller ownership, output F32 and reference fallback. Own the context-associated configuration, buffer descriptor, overflow/lifetime/stream/generation handling and telemetry hooks. Specify reset transaction for attention and recurrent state, application/sampler/transcript/prefix/prepared caches and in-flight work; initial fresh-context replacement is acceptable. Do not invent this API in downstream tasks.

**Validation and failure diagnosis:** K08 closes on host-only acceptance; no CUDA compiler or GK210 is required. Implement host fixtures/test doubles for context configuration/default reference, all mode names, invalid thresholds/unavailable kernels, tensor eligibility and byte strides, size overflow, generation invalidation and reset transaction. Inject fake reference/experimental callbacks with original finite F32 values (including values changed by Q8 conversion) to verify selection and value/stride preservation without K09 arithmetic. Cover one-column final prefill, two-column prefill, unsupported IDs/fusion/views, failed reset and independent contexts. Audit the actual CUDA adapter's placement before quantize_row_q8_1_cuda in source review. Host doubles establish the contract only: INTERFACE.md must mark native adapter compilation and real stream/seam execution unverified. K09 rebuilds integrated native glue; K10 verifies original-input interception and real model routing; K17 verifies actual stream/lifetime/reset behavior. Missing required host compiler or predecessor artifacts blocks this host acceptance.

**Evidence and output contract:** docs/tasks/K08/INTERFACE.md with actual headers/symbols, descriptor layout, caller examples, ownership matrix, phase/device rules, capability/fallback rules, telemetry event contract, reset transaction and tests. Also publish STATUS.md with the common metadata and exact failure/continuation information in the [output contract](IMPLEMENTATION_HANDOFF.md#what-is-the-evidence-and-output-contract).

**Completion and blocked state:** Close only after the original issue's full acceptance plus these checks is evidenced and reviewed. If a required artifact/toolchain/device/model is absent, leave the gate pending, identify the exact missing input and next command, preserve useful partial work and continue only independent work. A documentation PR or implementation merge is not an execution pass.

**Handoff and concurrency:** K09/K10/K13/K14/K17 consume the accepted implemented interface. Any missing interface decision returns here rather than spawning competing definitions. Check live claims and the atlas before edits; do not overlap the sole import, shared interface or sequential consumer owner. Reserve the entire K80 board for headline measurements. No self-merge or privileged host changes are authorized.

## K09 — issue #9

**Objective and completion boundary:** Implement the fixed-depth encoder using the accepted K08 interface and original activations, with actual target fixture validation.

**Inputs and prerequisites:** Completed K04, K05 and K08, including the accepted implemented interface. Use the selected source in SOURCE_MAP and the accepted K03 lock/patch ledger; never a moving upstream head. Actual APIs/commands must come from predecessor outputs.

**Required reading and evidence:** Read AGENTS.md, then [the relevant source investigation](research/q1-abi-and-activation-path.md), [the handoff](IMPLEMENTATION_HANDOFF.md#what-must-every-future-agent-do-before-editing), and the assigned issue's original ordered contract pack. The source note contains inspected symbols, exact paths/line ranges and pinned primary references. Those facts establish the gap; unexecuted build/model conclusions remain explicitly unresolved.

**Bounded implementation instructions:** Resolve actual descriptor and launch symbols from K08/INTERFACE.md before coding. Implement one depth-parameterized path for B1/B2/B3, per-operation preparation reused over rows, four 32-bit sign masks per full group and FP32 coefficients. Use explicit valid lane participation, stream ownership and no host copy/hot allocation. Read activation strides from accepted eligibility rules; never feed reference Q8_1 into basis construction. Keep all other layouts on recorded reference fallback.

**Validation and failure diagnosis:** Compare exact masks and documented-tolerance coefficients/reconstruction to K05 across zero/residual ties/outliers/nonfinites/adjacent groups/ragged work. Compile native sm_37 then run actual GK210 fixtures with poisoned scratch and consecutive generations. Publish unavailable hardware as pending, not a skipped pass. Rebuild the integrated backend including K08 adapter glue and record compile diagnostics rather than assuming K08 host acceptance proved target compilation.

**Evidence and output contract:** docs/tasks/K09/ENCODER.md with implemented symbols, descriptor version, fixture results and launch/continuation commands. Also publish STATUS.md with the common metadata and exact failure/continuation information in the [output contract](IMPLEMENTATION_HANDOFF.md#what-is-the-evidence-and-output-contract).

**Completion and blocked state:** Close only after the original issue's full acceptance plus these checks is evidenced and reviewed. If a required artifact/toolchain/device/model is absent, leave the gate pending, identify the exact missing input and next command, preserve useful partial work and continue only independent work. A documentation PR or implementation merge is not an execution pass.

**Handoff and concurrency:** K10 and K17; encoder edits later coordinated sequentially with K12. Check live claims and the atlas before edits; do not overlap the sole import, shared interface or sequential consumer owner. Reserve the entire K80 board for headline measurements. No self-merge or privileged host changes are authorized.

## K10 — issue #10

**Objective and completion boundary:** Connect B1 to actual eligible model operations and establish nonzero target execution against the independent intended-approximation oracle.

**Inputs and prerequisites:** Completed K08 and K09. Before closure, K07's accepted small_model_reference_on_K80 artifact, not all of K07. Use the selected source in SOURCE_MAP and the accepted K03 lock/patch ledger; never a moving upstream head. Actual APIs/commands must come from predecessor outputs.

**Required reading and evidence:** Read AGENTS.md, then [the relevant source investigation](research/q1-abi-and-activation-path.md), [the handoff](IMPLEMENTATION_HANDOFF.md#what-must-every-future-agent-do-before-editing), and the assigned issue's original ordered contract pack. The source note contains inspected symbols, exact paths/line ranges and pinned primary references. Those facts establish the gap; unexecuted build/model conclusions remain explicitly unresolved.

**Bounded implementation instructions:** Consume accepted K08/K09 artifacts; use safe byte assembly or proven helper for Q1 signs at offset 2 with stride 18. Apply signed FP16 weight scales and FP32 basis coefficients; preserve FP32 graph output. Route only accepted eligibility and retain the best working Kepler reference fallback. Exercise actual dispatch/capability decisions rather than assuming Q1 switch cases are reached. Use the small reference artifact for same-input policy comparisons; 27B is attempted when separately ready.

**Validation and failure diagnosis:** Exact mask dots and bounded FP32 result error versus K05; ragged output rows, adjacent block alignment, poisoned output/scratch, unsupported shapes and reference no-op. Actual small-model GK210 generation must have nonzero approximate counters and saved text. Zero coverage, stale state and CPU substitution fail; incoherent approximate language alone does not fail implementation correctness. Prove the real interception receives original F32 values before Q8 conversion (bounded diagnostic capture/fixture with independently known inputs), not only nonzero counters.

**Evidence and output contract:** docs/tasks/K10/B1.md with source/model/device identity, route traces, fixtures and real generated output. Also publish STATUS.md with the common metadata and exact failure/continuation information in the [output contract](IMPLEMENTATION_HANDOFF.md#what-is-the-evidence-and-output-contract).

**Completion and blocked state:** Close only after the original issue's full acceptance plus these checks is evidenced and reviewed. If a required artifact/toolchain/device/model is absent, leave the gate pending, identify the exact missing input and next command, preserve useful partial work and continue only independent work. A documentation PR or implementation merge is not an execution pass.

**Handoff and concurrency:** K11 sequential consumer extension; K16/K17. Do not require full K07 closure for the small reference artifact. Check live claims and the atlas before edits; do not overlap the sole import, shared interface or sequential consumer owner. Reserve the entire K80 board for headline measurements. No self-merge or privileged host changes are authorized.

## K11 — issue #11

**Objective and completion boundary:** Extend the accepted common consumer to B2/B3 and retain reference/B1 behavior.

**Inputs and prerequisites:** Completed K10 and its common consumer handoff. Use the selected source in SOURCE_MAP and the accepted K03 lock/patch ledger; never a moving upstream head. Actual APIs/commands must come from predecessor outputs.

**Required reading and evidence:** Read AGENTS.md, then [the relevant source investigation](research/q1-abi-and-activation-path.md), [the handoff](IMPLEMENTATION_HANDOFF.md#what-must-every-future-agent-do-before-editing), and the assigned issue's original ordered contract pack. The source note contains inspected symbols, exact paths/line ranges and pinned primary references. Those facts establish the gap; unexecuted build/model conclusions remain explicitly unresolved.

**Bounded implementation instructions:** Read K09 encoder and K10 B1 handoffs, then extend their existing symbols. Reuse native weight masks, per-operation bases and K08 configuration. Preserve full-group validity, signed scales, output dtype, stream lifetime and fallback rules. No new descriptor, independent backend or full reconstructed activation in the consumer.

**Validation and failure diagnosis:** K05 B2/B3 reconstruction, residual ties/cancellation/outliers/adjacent block alignment and fixed-mode regressions on actual K80; both modes must generate small-model tokens with nonzero coverage. Try 27B once its reference record exists, retaining negative text results.

**Evidence and output contract:** docs/tasks/K11/B2_B3.md with exact common symbols, tests and both model-output records. Also publish STATUS.md with the common metadata and exact failure/continuation information in the [output contract](IMPLEMENTATION_HANDOFF.md#what-is-the-evidence-and-output-contract).

**Completion and blocked state:** Close only after the original issue's full acceptance plus these checks is evidenced and reviewed. If a required artifact/toolchain/device/model is absent, leave the gate pending, identify the exact missing input and next command, preserve useful partial work and continue only independent work. A documentation PR or implementation merge is not an execution pass.

**Handoff and concurrency:** K12; mode rows consumed by K16/K17. Shared consumer edits remain sequential. Check live claims and the atlas before edits; do not overlap the sole import, shared interface or sequential consumer owner. Reserve the entire K80 board for headline measurements. No self-merge or privileged host changes are authorized.

## K12 — issue #12

**Objective and completion boundary:** Extend the same encoder/consumer to contract-defined adaptive depth, with honest computed versus consumed work.

**Inputs and prerequisites:** Completed K11 and its fixed-mode regressions. Use the selected source in SOURCE_MAP and the accepted K03 lock/patch ledger; never a moving upstream head. Actual APIs/commands must come from predecessor outputs.

**Required reading and evidence:** Read AGENTS.md, then [the relevant source investigation](research/q1-abi-and-activation-path.md), [the handoff](IMPLEMENTATION_HANDOFF.md#what-must-every-future-agent-do-before-editing), and the assigned issue's original ordered contract pack. The source note contains inspected symbols, exact paths/line ranges and pinned primary references. Those facts establish the gap; unexecuted build/model conclusions remain explicitly unresolved.

**Bounded implementation instructions:** Consume K11 and K08 accepted generation/descriptor rules. Implement residual-energy threshold decisions over original activation groups using direct FP32 residual reductions initially. Handle zero and nonfinite energy explicitly. Stop later-plane construction when claiming encoder savings, skip inactive consumer planes and report allocated bytes independently. No new global mode state or cross-operation pointer cache.

**Validation and failure diagnosis:** Threshold endpoints, depth extremes, exact-zero residual, mixed-depth groups, nonfinite rejection and reference/fixed-mode regressions against K05 on K80. Record actual computed and consumed histograms and real-model generation; reduced consumed depth alone cannot support encoding-speed claims.

**Evidence and output contract:** docs/tasks/K12/ADAPTIVE.md with thresholds, plane/byte accounting, target results and limitations. Also publish STATUS.md with the common metadata and exact failure/continuation information in the [output contract](IMPLEMENTATION_HANDOFF.md#what-is-the-evidence-and-output-contract).

**Completion and blocked state:** Close only after the original issue's full acceptance plus these checks is evidenced and reviewed. If a required artifact/toolchain/device/model is absent, leave the gate pending, identify the exact missing input and next command, preserve useful partial work and continue only independent work. A documentation PR or implementation merge is not an execution pass.

**Handoff and concurrency:** Full mode-matrix completion in K16 and adaptive lifecycle tests in K17. Check live claims and the atlas before edits; do not overlap the sole import, shared interface or sequential consumer owner. Reserve the entire K80 board for headline measurements. No self-merge or privileged host changes are authorized.


## K13 — issue #13

**Objective and completion boundary:** Build persistent text chat using accepted configuration and true full-state reset, on an explicitly selected single device.

**Inputs and prerequisites:** Completed K03 and K08. Before real chat acceptance, K06's small_model_identity_and_template artifact, not all of K06. Use the selected source in SOURCE_MAP and the accepted K03 lock/patch ledger; never a moving upstream head. Actual APIs/commands must come from predecessor outputs.

**Required reading and evidence:** Read AGENTS.md, then [the relevant source investigation](research/state-cli-and-device.md), [the handoff](IMPLEMENTATION_HANDOFF.md#what-must-every-future-agent-do-before-editing), and the assigned issue's original ordered contract pack. The source note contains inspected symbols, exact paths/line ranges and pinned primary references. Those facts establish the gap; unexecuted build/model conclusions remain explicitly unresolved.

**Bounded implementation instructions:** Consume actual K08 symbols. Both candidate tools/cli implementations launch a localhost HTTP server internally; do not assume the old in-process CLI. To preserve the no-exposed-server boundary, adapt the existing in-process tools/completion/completion.cpp / llama-completion-impl path with its common parser/init, Jinja chat and sampler facilities. Preserve explicit thinking/kwargs through initial and later template calls, validate hybrid sequence-removal failures during context limits, and disable old-policy saved-state/prefix restoration. examples/simple-chat is only a minimal alternative if a recorded concrete completion-path obstruction justifies it; neither path satisfies K13 as-is. Use existing device parsing/enumeration, a one-entry device list plus split-mode none, and verify resulting device identity/offload rather than main-gpu alone. Implement an announced fresh-session transaction after in-flight work completes: recreate inference context and sampler from fixed configuration; reset transcript/prompt/prefix and prepared-state generations. clear(false), seq_rm, common_sampler_reset or a token-counter reset alone is not a complete session reset. Keep default reference and reject unavailable experimental modes. Expose semantic phase only if propagated deliberately from prompt-versus-sampled-token batch construction; otherwise label policy single-column.

**Validation and failure diagnosis:** Real small identity/template artifact needed for chat acceptance. Test multi-turn continuity, /reset, each available mode switch against independent fresh context, Unicode/EOF/Ctrl+C/path spaces/invalid input, context-limit behavior and stale prefix/sampler state. Cover hybrid recurrent reset with available real model or explicit test fixture; mark unexecuted real 27B checks pending for K17/K19. Device selector cases: empty/invalid, one device, both visible, split prohibited, reported selection matching execution. Verify no listener/server thread starts and no silently successful partial-sequence-removal failure or template-thinking-policy drift.

**Evidence and output contract:** docs/tasks/K13/CHAT.md with tested commands, reset transaction, template policy and host-versus-K80 evidence. Also publish STATUS.md with the common metadata and exact failure/continuation information in the [output contract](IMPLEMENTATION_HANDOFF.md#what-is-the-evidence-and-output-contract).

**Completion and blocked state:** Close only after the original issue's full acceptance plus these checks is evidenced and reviewed. If a required artifact/toolchain/device/model is absent, leave the gate pending, identify the exact missing input and next command, preserve useful partial work and continue only independent work. A documentation PR or implementation merge is not an execution pass.

**Handoff and concurrency:** K15/K16/K17/K20; independent of encoder editing after K08. Check live claims and the atlas before edits; do not overlap the sole import, shared interface or sequential consumer owner. Reserve the entire K80 board for headline measurements. No self-merge or privileged host changes are authorized.

## K14 — issue #14

**Objective and completion boundary:** Provide shared mode-aware coverage and end-to-end measurement without confusing tensor shape with evaluation phase.

**Inputs and prerequisites:** Completed K08 and its actual telemetry/config interface. Use the selected source in SOURCE_MAP and the accepted K03 lock/patch ledger; never a moving upstream head. Actual APIs/commands must come from predecessor outputs.

**Required reading and evidence:** Read AGENTS.md, then [the relevant source investigation](research/state-cli-and-device.md), [the handoff](IMPLEMENTATION_HANDOFF.md#what-must-every-future-agent-do-before-editing), and the assigned issue's original ordered contract pack. The source note contains inspected symbols, exact paths/line ranges and pinned primary references. Those facts establish the gap; unexecuted build/model conclusions remain explicitly unresolved.

**Bounded implementation instructions:** Consume K08 telemetry/config contract; read q1-abi-and-activation-path.md for routes and small-and-27b-operator-paths.md for recurrent operator fallbacks. Attribute eligible/executed/fallback operations, phase policy, dtype/shape/role/device and computed/consumed basis depth. An n_tokens or one-column predicate alone cannot certify decode. Include any one-column prefill approximation in metadata. Keep default reference and parity checks identical between runner/chat.

**Validation and failure diagnosis:** Mock EOS-shortened generation, fixed-step mode, zero elapsed time/error, synchronized completion, first-token boundary, skipped/failed evaluations and zero experimental coverage. Validate JSON/JSONL output against required fields in VALIDATION. One warmup and three retained unprofiled repeats on target for numerical claims; host runner tests alone suffice only for implementation bookkeeping.

**Evidence and output contract:** docs/tasks/K14/MEASUREMENT.md plus stable run schema and exact runner invocation. Also publish STATUS.md with the common metadata and exact failure/continuation information in the [output contract](IMPLEMENTATION_HANDOFF.md#what-is-the-evidence-and-output-contract).

**Completion and blocked state:** Close only after the original issue's full acceptance plus these checks is evidenced and reviewed. If a required artifact/toolchain/device/model is absent, leave the gate pending, identify the exact missing input and next command, preserve useful partial work and continue only independent work. A documentation PR or implementation merge is not an execution pass.

**Handoff and concurrency:** K15 and K16; shared dispatch call-site changes go through K08. Check live claims and the atlas before edits; do not overlap the sole import, shared interface or sequential consumer owner. Reserve the entire K80 board for headline measurements. No self-merge or privileged host changes are authorized.

## K15 — issue #15

**Objective and completion boundary:** Create the fixed small public prompt corpus and a recoverable runner using the accepted real chat/measurement paths.

**Inputs and prerequisites:** Completed K06, K13 and K14. Use the selected source in SOURCE_MAP and the accepted K03 lock/patch ledger; never a moving upstream head. Actual APIs/commands must come from predecessor outputs.

**Required reading and evidence:** Read AGENTS.md, then [the relevant source investigation](research/state-cli-and-device.md), [the handoff](IMPLEMENTATION_HANDOFF.md#what-must-every-future-agent-do-before-editing), and the assigned issue's original ordered contract pack. The source note contains inspected symbols, exact paths/line ranges and pinned primary references. Those facts establish the gap; unexecuted build/model conclusions remain explicitly unresolved.

**Bounded implementation instructions:** Consume K06 template identity, K13 reset semantics and K14 run schema. Reset full context for independent prompts, keep thinking/template/EOS/sampler settings stable and resume by immutable prompt/model/mode/policy identity. For aligned logits use identical teacher-forced prefixes; do not equate same random seed with same history. No automatic execution of generated code.

**Validation and failure diagnosis:** Check prompt ID uniqueness, duplicate/resume protection, failed run retention, mode labels, template identity and fresh-context behavior using fixtures/reference runs. Human quality grades need actual human review attribution; otherwise mark provisional. Do not claim target performance from bookkeeping tests.

**Evidence and output contract:** docs/tasks/K15/QUALITY.md, versioned corpus, runner tests and saved-output schema. Also publish STATUS.md with the common metadata and exact failure/continuation information in the [output contract](IMPLEMENTATION_HANDOFF.md#what-is-the-evidence-and-output-contract).

**Completion and blocked state:** Close only after the original issue's full acceptance plus these checks is evidenced and reviewed. If a required artifact/toolchain/device/model is absent, leave the gate pending, identify the exact missing input and next command, preserve useful partial work and continue only independent work. A documentation PR or implementation merge is not an execution pass.

**Handoff and concurrency:** K16 quality screening; K19 reuse without silently changing corpus. Check live claims and the atlas before edits; do not overlap the sole import, shared interface or sequential consumer owner. Reserve the entire K80 board for headline measurements. No self-merge or privileged host changes are authorized.

## K16 — issue #16

**Objective and completion boundary:** Measure the untuned binary 27B frontier with all five required modes, maintaining identical reference operator/state policy.

**Inputs and prerequisites:** Completed K07, K10, K13 and K14 to start; K11, K12 and K15 additionally before closure. Use the selected source in SOURCE_MAP and the accepted K03 lock/patch ledger; never a moving upstream head. Actual APIs/commands must come from predecessor outputs.

**Required reading and evidence:** Read AGENTS.md, then [the relevant source investigation](research/small-and-27b-operator-paths.md), [the handoff](IMPLEMENTATION_HANDOFF.md#what-must-every-future-agent-do-before-editing), and the assigned issue's original ordered contract pack. The source note contains inspected symbols, exact paths/line ranges and pinned primary references. Those facts establish the gap; unexecuted build/model conclusions remain explicitly unresolved.

**Bounded implementation instructions:** Consume accepted full K07 27B reference and each available mode artifact. Record qwen35 fused/nonfused and CPU fallback settings identically across modes. Reserve whole board; use one GK210, explicit context, fixed prompt/template/sampler, fresh contexts and nonzero approximate coverage. Run short samples even after small-model collapse, then declared bounded sensitivity scope if needed. Full closure also requires K11/K12/K15.

**Validation and failure diagnosis:** Retain one warmup and three screening repeats plus raw outputs/actual counts, memory and state/fallback/coverage records. Distinguish PP512, TTFT, decode and combined preparation/consumer cost. Reject hidden route or baseline changes. Missing required mode, target or model remains pending; measured negative quality/performance remains valid.

**Evidence and output contract:** docs/tasks/K16/27B_FRONTIER.md with all-mode raw records and fastest-executed/useful distinction. Also publish STATUS.md with the common metadata and exact failure/continuation information in the [output contract](IMPLEMENTATION_HANDOFF.md#what-is-the-evidence-and-output-contract).

**Completion and blocked state:** Close only after the original issue's full acceptance plus these checks is evidenced and reviewed. If a required artifact/toolchain/device/model is absent, leave the gate pending, identify the exact missing input and next command, preserve useful partial work and continue only independent work. A documentation PR or implementation merge is not an execution pass.

**Handoff and concurrency:** K18 and K19; do not open deferred X work. Check live claims and the atlas before edits; do not overlap the sole import, shared interface or sequential consumer owner. Reserve the entire K80 board for headline measurements. No self-merge or privileged host changes are authorized.

## K17 — issue #17

**Objective and completion boundary:** Automate the source-identified layout, route and hybrid lifecycle regressions using existing oracle/interface implementations.

**Inputs and prerequisites:** Completed K08, K09 and K10 to start; K11, K12 and K13 additionally before closure. Use the selected source in SOURCE_MAP and the accepted K03 lock/patch ledger; never a moving upstream head. Actual APIs/commands must come from predecessor outputs.

**Required reading and evidence:** Read AGENTS.md, then [the relevant source investigation](research/state-cli-and-device.md), [the handoff](IMPLEMENTATION_HANDOFF.md#what-must-every-future-agent-do-before-editing), and the assigned issue's original ordered contract pack. The source note contains inspected symbols, exact paths/line ranges and pinned primary references. Those facts establish the gap; unexecuted build/model conclusions remain explicitly unresolved.

**Bounded implementation instructions:** Read also q1-abi-and-activation-path.md and small-and-27b-operator-paths.md. Cover aligned/unaligned native groups and views, original activation interception, fused eligibility/fallback, one-column prefill, unsupported dtype/tail, consecutive tensor generations and output poisoning. Include recurrent seq_rm failure, clear(false) versus data clearing, full reset/mode change/fresh-context equivalence, sampler/prefix/prepared-state invalidation and explicit single-device selection. Coordinate fixes with owners; do not rewrite common API inside tests. Include seq_rm(-1,0,-1) against the source-identified unsigned-cast rejection and verify refusal cannot leave application counters inconsistent with hybrid memory.

**Validation and failure diagnosis:** Run K05 host fixtures and all applicable actual K80 primitives plus short real small/27B cases. Record a tested/unavailable matrix per mode, graph path and reset case. Compatible checking tools only; source/compiler review is not runtime memory validation. Failures return nonzero and preserve exact reproducers.

**Evidence and output contract:** docs/tasks/K17/REGRESSIONS.md and compact regression suite with source/model/tool identities. Also publish STATUS.md with the common metadata and exact failure/continuation information in the [output contract](IMPLEMENTATION_HANDOFF.md#what-is-the-evidence-and-output-contract).

**Completion and blocked state:** Close only after the original issue's full acceptance plus these checks is evidenced and reviewed. If a required artifact/toolchain/device/model is absent, leave the gate pending, identify the exact missing input and next command, preserve useful partial work and continue only independent work. A documentation PR or implementation merge is not an execution pass.

**Handoff and concurrency:** K18/K19 after all required closures; shared consumer fixes serialized with K10–K12. Check live claims and the atlas before edits; do not overlap the sole import, shared interface or sequential consumer owner. Reserve the entire K80 board for headline measurements. No self-merge or privileged host changes are authorized.


## K18 — issue #18

**Objective and completion boundary:** Tune only measured real-model bottlenecks after untuned target evidence and regressions.

**Inputs and prerequisites:** Completed K16 and K17, actual K80 available and whole-board reservation. Use the selected source in SOURCE_MAP and the accepted K03 lock/patch ledger; never a moving upstream head. Actual APIs/commands must come from predecessor outputs.

**Required reading and evidence:** Read AGENTS.md, then [the relevant source investigation](research/q1-abi-and-activation-path.md), [the handoff](IMPLEMENTATION_HANDOFF.md#what-must-every-future-agent-do-before-editing), and the assigned issue's original ordered contract pack. The source note contains inspected symbols, exact paths/line ranges and pinned primary references. Those facts establish the gap; unexecuted build/model conclusions remain explicitly unresolved.

**Bounded implementation instructions:** Consume K16/K17; choose three to five bounded candidates in advance and reserve shared code ownership plus whole-board timing. Keep scope, approximation thresholds, model/context/state/offload and sampler fixed for pure optimization comparisons. Native alignment/stride and original-activation semantics remain invariant. Use actual tensor traces; source load counts are not DRAM measurements.

**Validation and failure diagnosis:** Relevant oracle and lifecycle regressions after each retained edit, then combined preparation+consumer and end-to-end unprofiled repeats. Retain losing candidates and restore simpler code if no net benefit. No performance estimate without actual measurement.

**Evidence and output contract:** docs/tasks/K18/TUNING.md with candidate ledger, reproducible commands and retain/no-change decision. Also publish STATUS.md with the common metadata and exact failure/continuation information in the [output contract](IMPLEMENTATION_HANDOFF.md#what-is-the-evidence-and-output-contract).

**Completion and blocked state:** Close only after the original issue's full acceptance plus these checks is evidenced and reviewed. If a required artifact/toolchain/device/model is absent, leave the gate pending, identify the exact missing input and next command, preserve useful partial work and continue only independent work. A documentation PR or implementation merge is not an execution pass.

**Handoff and concurrency:** K19 clean integrated rerun; no deferred scope activation. Check live claims and the atlas before edits; do not overlap the sole import, shared interface or sequential consumer owner. Reserve the entire K80 board for headline measurements. No self-merge or privileged host changes are authorized.

## K19 — issue #19

**Objective and completion boundary:** Verify clean integrated reference/all-mode operation on both actual model architectures and target hardware.

**Inputs and prerequisites:** Completed K16, K17 and K18, actual K80 available and whole-board reservation. Use the selected source in SOURCE_MAP and the accepted K03 lock/patch ledger; never a moving upstream head. Actual APIs/commands must come from predecessor outputs.

**Required reading and evidence:** Read AGENTS.md, then [the relevant source investigation](research/small-and-27b-operator-paths.md), [the handoff](IMPLEMENTATION_HANDOFF.md#what-must-every-future-agent-do-before-editing), and the assigned issue's original ordered contract pack. The source note contains inspected symbols, exact paths/line ranges and pinned primary references. Those facts establish the gap; unexecuted build/model conclusions remain explicitly unresolved.

**Bounded implementation instructions:** Consume K16/K17/K18 and freeze code/configuration during reserved-board measurements. Rebuild from one clean accepted implementation commit, verify upstream/native artifact/model hashes, and exercise same-device offload, fresh/reset/mode-switch and reference parity. Small qwen3 results never substitute for qwen35 hybrid evidence.

**Validation and failure diagnosis:** All required modes on both models, target regressions, short persistent chat, reset equivalence, meaningful device/route coverage, raw timing/output repeats and honest CPU fallback. Record missing driver/tool/model/hardware as blocker. Sequential second-device functional selection is optional; concurrent headline timings are prohibited.

**Evidence and output contract:** docs/tasks/K19/ACCEPTANCE.md with clean-build transcript, raw evidence index and supported tested host list. Also publish STATUS.md with the common metadata and exact failure/continuation information in the [output contract](IMPLEMENTATION_HANDOFF.md#what-is-the-evidence-and-output-contract).

**Completion and blocked state:** Close only after the original issue's full acceptance plus these checks is evidenced and reviewed. If a required artifact/toolchain/device/model is absent, leave the gate pending, identify the exact missing input and next command, preserve useful partial work and continue only independent work. A documentation PR or implementation merge is not an execution pass.

**Handoff and concurrency:** Final K20 package acceptance and K21 report. Check live claims and the atlas before edits; do not overlap the sole import, shared interface or sequential consumer owner. Reserve the entire K80 board for headline measurements. No self-merge or privileged host changes are authorized.

## K20 — issue #20

**Objective and completion boundary:** Package the accepted implementation with source/model provenance and instructions actually exercised from a clean checkout.

**Inputs and prerequisites:** Completed K13 to start; K19 additionally before closure. Use the selected source in SOURCE_MAP and the accepted K03 lock/patch ledger; never a moving upstream head. Actual APIs/commands must come from predecessor outputs.

**Required reading and evidence:** Read AGENTS.md, then [the relevant source investigation](research/candidate-comparison-and-import.md), [the handoff](IMPLEMENTATION_HANDOFF.md#what-must-every-future-agent-do-before-editing), and the assigned issue's original ordered contract pack. The source note contains inspected symbols, exact paths/line ranges and pinned primary references. Those facts establish the gap; unexecuted build/model conclusions remain explicitly unresolved.

**Bounded implementation instructions:** Consume K13 for early documentation and K19 for final acceptance. Preserve exact upstream lock/licence and tracked local patches, actual device/reset/phase controls and reference default. Document only tested hosts and working commands; Windows path quoting is tested when supported, WSL K80 is not assumed. Exclude weights, logs containing private prompts, caches and credentials.

**Validation and failure diagnosis:** Execute written build/acquisition-reference/chat/mode/evaluation steps from clean checkout, including paths with spaces and failure exit propagation. Verify source package identity and exclusions. Do not publish a working-K80 claim when target acceptance is missing.

**Evidence and output contract:** docs/tasks/K20/HANDOFF.md with package/source identity, licence paths, verified instructions and limits. Also publish STATUS.md with the common metadata and exact failure/continuation information in the [output contract](IMPLEMENTATION_HANDOFF.md#what-is-the-evidence-and-output-contract).

**Completion and blocked state:** Close only after the original issue's full acceptance plus these checks is evidenced and reviewed. If a required artifact/toolchain/device/model is absent, leave the gate pending, identify the exact missing input and next command, preserve useful partial work and continue only independent work. A documentation PR or implementation merge is not an execution pass.

**Handoff and concurrency:** K21; any untested hardware release claim remains pending. Check live claims and the atlas before edits; do not overlap the sole import, shared interface or sequential consumer owner. Reserve the entire K80 board for headline measurements. No self-merge or privileged host changes are authorized.

## K21 — issue #21

**Objective and completion boundary:** Synthesize measured POC results and reconcile acceptance without converting documentation or merged code into hardware success.

**Inputs and prerequisites:** Completed K20 and K19. Use the selected source in SOURCE_MAP and the accepted K03 lock/patch ledger; never a moving upstream head. Actual APIs/commands must come from predecessor outputs.

**Required reading and evidence:** Read AGENTS.md, then [the relevant source investigation](research/candidate-comparison-and-import.md), [the handoff](IMPLEMENTATION_HANDOFF.md#what-must-every-future-agent-do-before-editing), and the assigned issue's original ordered contract pack. The source note contains inspected symbols, exact paths/line ranges and pinned primary references. Those facts establish the gap; unexecuted build/model conclusions remain explicitly unresolved.

**Bounded implementation instructions:** Consume K19/K20 and actual frontier/tuning/quality artifacts. Separate small and 27B, source/host/native/K80/model evidence, reference policy, coverage/fallback and fastest-executed versus fastest-useful. Reconcile each issue only against its acceptance. Recommend one measured next step without admitting X01/X02 automatically.

**Validation and failure diagnosis:** Every numerical claim links raw observation and immutable identity; every completion statement links accepted output/test evidence. Missing execution stays partial. Check python scripts/check_workflow.py and live issue mapping after reconciliation.

**Evidence and output contract:** POC_REPORT.md plus docs/tasks/K21/STATUS.md with reproduction/evidence index and unresolved blockers. Also publish STATUS.md with the common metadata and exact failure/continuation information in the [output contract](IMPLEMENTATION_HANDOFF.md#what-is-the-evidence-and-output-contract).

**Completion and blocked state:** Close only after the original issue's full acceptance plus these checks is evidenced and reviewed. If a required artifact/toolchain/device/model is absent, leave the gate pending, identify the exact missing input and next command, preserve useful partial work and continue only independent work. A documentation PR or implementation merge is not an execution pass.

**Handoff and concurrency:** Maintainer decision; X01/X02 only after explicit separate admission. Check live claims and the atlas before edits; do not overlap the sole import, shared interface or sequential consumer owner. Reserve the entire K80 board for headline measurements. No self-merge or privileged host changes are authorized.

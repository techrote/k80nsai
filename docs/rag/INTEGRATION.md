# Runtime integration and interface boundaries

## Choose and pin the runtime

K01 evaluates a bounded set of sources, beginning with the observed PrismML `prism` revision in SOURCES_AND_DECISIONS. Mainline already advertises Q1 support, so Prism is a candidate rather than dogma. The source must support both the chosen small GGUF and the 27B architecture and have a defensible CUDA 11/sm_37 route. Record why the selected pin wins. Do not chase moving heads during kernel work.

K03 should use a tracked source snapshot under `vendor/llama.cpp/` plus an upstream lock and retained licence, or a clearly justified equally reproducible alternative. Do not leave kernel changes only in an uncommitted submodule. Do not replace the root repository/history to turn it into a different fork. Keep local changes reviewable and preserve the workflow documents.

## Interception seam

K08 traces actual source operations and freezes a small interface shared by CLI, encoder, consumer, telemetry and tests. File names and flags in this bundle are proposed local API names, not claims about existing upstream APIs.

Intercept the eligible Q1 matrix multiplication before irreversible Q8 conversion where practical. Match actual tensor types, dimensions, byte strides, views, device, operation role and phase. Establish explicitly which projected matrices and fused operations are covered. An embedding gather is not GEMV; a fused hybrid-attention operator may not pass through the same dispatch point.

Start with one input column and single selected device. Multi-column prefill and unsupported graph nodes use the unmodified reference path. Graph scheduling must see truthful capability predicates and buffer ownership. Do not advertise support and fail later with invalid kernel launches.

## Configuration / policy

Per-context or per-backend immutable-for-evaluation settings should include mode, scope, thresholds, device and policy generation. Avoid mutable process-global switches that can affect another context. Stable local CLI proposal:

```
--k80-mode reference|b1|b2|b3|adaptive
--k80-scope all-eligible|mlp-only|<explicit mapped selection>
--k80-b1-threshold <0..1>
--k80-b2-threshold <0..1>
```

Use the runtime's real device selector instead of inventing incompatible `--device` syntax. Validate flags centrally so chat and measurement cannot run different modes by accident. Default to reference. Unknown/unsupported experimental modes fail clearly rather than silently falling back for every operation.

## Scratch, streams and graph reuse

Use the backend's buffer pool/allocator and stream ordering. No cudaMalloc/free or CPU round trip on every matrix operation. Scratch size includes actual groups, plane capacity, alpha buffers and metadata. Guard overflow in size arithmetic. Prepared masks/alphas must remain alive until consumers finish.

Do not cache by activation pointer alone: runtimes reuse addresses between layers, token steps and graph executions. Initially prepare each operation independently. Any reuse key must include input tensor identity/view, strides/dtype, evaluation generation, mode/depth/threshold generation, device/stream and producer completion. Invalidate on reset, model reload and policy change. CUDA graph capture may be disabled for this legacy route; do not change unrelated hardware paths globally.

## Chat and state changes

K01 source inspection found that both pinned upstream CLIs launch an internal HTTP server; see [state/CLI source evidence](../tasks/K01/research/state-cli-and-device.md). For the existing no-exposed-server boundary, K13 should adapt the existing in-process `tools/completion/completion.cpp` / `llama-completion-impl` path with its common argument/model/context, Jinja template and sampler facilities, then add the required lifecycle/error behavior and explicit thinking/kwargs propagation. It builds without the server-backed CLI. `examples/simple-chat` is only a minimal fallback illustration; neither existing path alone satisfies all K13 acceptance. Preserve the GGUF chat template. A thin launcher/command wrapper is acceptable; no GUI, no exposed server. Avoid resetting conversation on every ordinary turn.

Required commands: `/help`, `/stats`, `/reset`, `/quit`; `/mode` is useful but must be correct. On a mode/scope/threshold change, either explicitly start a fresh session or rebuild state from the retained transcript under the new policy. The simplest POC is an announced fresh session. Never reuse old-policy KV or recurrent/linear-attention state and call the result a clean comparison. Clear prompt/prefix cache and prepared activation state too. If full state reset cannot be guaranteed, recreate the inference context, not just a token counter.

Preserve end-of-generation handling, template formatting, model reasoning/thinking mode, context-limit behavior, Unicode input and Ctrl+C cleanup. Never silently truncate accumulated history. Stream text; print stats after the response, not between tokens. Launcher scripts must handle spaces in paths and propagate failure codes. No system-wide execution-policy changes.

## Coverage and fallback

K14 records attempted eligible operations, executed approximate operations, fallback counts/reasons, tensor roles/layers/shapes, computed/consumed basis depth, and device residency. A selected mode with zero approximate execution is a failed experiment, not a successful fast result.

Reference and approximate runs must share model file/revision, context/KV/state precision, offload policy, sampler, prompt token IDs and timing definition. Any necessary CPU fallback must be listed and unchanged for parity comparisons. Log whether reported prefill includes any approximate single-column work. Separate full-model token throughput from isolated linear-kernel timings.

## 27B specifics

Treat 27B as its own graph-compatibility test. Inspect actual GGUF architecture identifier, tensor types, recurrent/linear-attention operators, full-attention state, fused paths and required precision. Disable optional vision/drafter loading for text-only POC. Start with a modest explicit context, for example 2048 tokens if supported, and measure peak VRAM. A small compressed file does not guarantee every temporary/state allocation fits in 12 GB.

Small-model approximation collapse does not predict 27B collapse. Once reference 27B is functional and the primitive is validated, test each approximate mode with a short, bounded generation even when the small model looked poor.

# Lean validation and measurement

## Evidence ladder

Distinguish host unit tests; CUDA target compilation; K80 primitive execution; real small-model execution; real 27B execution; useful text; speedup. One does not imply the next. Mark missing tools/devices as unavailable, not passed/skipped-equivalent-to-passed.

The minimum debugging layer is an independent native Q1 decoder and basis oracle plus targeted boundary tests in KERNELS. Preserve reference output availability. For approximate kernels compare against the same approximation, not identical reference tokens. Keep one failing input dump/reproducer for every corrected packing/dispatch defect.

## Runtime regressions

Cover mode reference/no-op, all modes on real tensors, invalid flags, scope exclusions, unsupported shapes/types, ragged output rows, scratch reuse, evaluation generation, reset/model switch, Ctrl+C cleanup and device selection. Test mode switching against a genuinely fresh context, including recurrent state—not just KV counters. A tiny graph or mocked CLI test may cover host behavior; it does not establish K80 inference.

Use compatible checking tools where available. Do not force a current Nsight/Compute Sanitizer workflow onto unsupported Kepler hardware. Record exact tool version and its support limits. Source/compiler/SASS review plus targeted guard tests are fallback evidence, not a substitute for all memory-error detection.

## Fast performance protocol

Use one model, one chosen CUDA device, fixed modest context, batch-one generation, the same prompt and generation length, sampler and offload policy. Warm once; perform three unprofiled repeats for screening, saving all observations and median. This is not a significance claim. Interleave reference/candidate when temperature/clock drift is plausible. Reserve the entire K80 board for headline timing; keep the other GK210 idle.

Primary measurement is wall-clock end-to-end decode tokens/s with actual generated/evaluated token count, after completion synchronization and excluding human think time. Specify first-token accounting. Do not count EOS-shortened text as 128 generated tokens. A dedicated fixed-step TG128 runner may ignore EOS, but label that policy and do not treat forced continuation as normal chat quality.

Keep PP512, prefill time, time-to-first-token and decode time separate. Prefill staying reference is expected; do not attribute PP512 gains to a decode-only kernel. Reference and candidate must have the same input history/state policy. Same sampling seed does not mean same generated history once logits diverge.

Record CPU execution/fallback, model residency, context/KV settings, memory peak, GPU temperature/clock/power telemetry when available. No clock/power-limit changes are required. Diagnostics may perturb execution; use unprofiled runs for headline rates.

## Encoder / kernel diagnostics

Optionally add CUDA-event timings for preparation and dot kernels on the actual execution stream, sampled or batched. Do not synchronize every layer in normal chat. Measure combined preparation+consumer time directly; do not add independently computed medians and call that a measured pipeline latency. No full profiler campaign is required.

Reduced consumed planes are not necessarily reduced computed planes or memory bytes. Report all three separately. Matrix shapes come from actual model traces, not remembered generic transformer dimensions.

## Quality screening

K15 creates roughly 24 short, original/public synthetic prompts across chat, instructions, summarization, arithmetic, code, explanation and continuation. Keep system/template/thinking settings fixed. Save ordinary free-running outputs; compare reference, B1/B2/B3/adaptive. Classify informal human observations as A useful/coherent, B degraded/coherent, C intermittent, D mostly broken, E collapsed. Do not call those benchmark scores.

Tiny arithmetic/format tests may have checkable expected answers. Do not execute generated code automatically. If no human review occurred, mark the quality judgment provisional/automated rather than inventing an assessment.

For token/logit agreement, replay identical token prefixes (teacher forcing) through fresh independent state in each mode. Compare distributions before sampling at aligned positions. Comparing the nth logits of already-diverged free-running sequences is invalid as a numerical fidelity measure. Perplexity is optional and requires a fixed corpus/tokenization; no inference-speed conclusion follows from a proxy quality metric alone.

## Minimal run record

Use JSONL or JSON plus human-readable outputs. Required fields:

```
run_id, timestamp, implementation_commit, upstream_commit,
model_repo_revision, model_file_sha256, model_architecture,
host_os, driver, toolkit, gpu_device, gpu_compute_capability,
mode, scope, thresholds, dispatch_policy, phase,
context_size, kv_or_recurrent_state_policy, offload_policy,
prompt_id, prompt_hash_or_token_ids, sampler, seed,
requested_tokens, actual_tokens, eos_policy,
prefill_ms, decode_ms, decode_tokens_per_s,
approx_op_count, eligible_op_count, fallback_reasons,
computed_plane_histogram, consumed_plane_histogram,
peak_vram_if_available, diagnostics_enabled,
status, output_path, limitations
```

Unavailable fields are null with reason, not zero measurements. Store no credentials/private prompts. Retain raw observations, not just the best row. A final comparison lists fastest executed mode separately from fastest useful mode and may conclude there is no gain.

## Bounded tuning

After the untuned 27B matrix, try a few measured changes: row mapping, activation reuse, group traversal, launch count, scale loads. Restore the initial implementation if no net end-to-end benefit. Keep the same quality policy when comparing speed; changing scope/thresholds is a different operating point, not pure kernel optimization. Stop after the agreed small candidate set rather than growing another research project.

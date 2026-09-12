# POC contract

## Required product

A text-only terminal chat based on a pinned Bonsai-capable llama.cpp source tree. Target native CUDA 11.x code for one Tesla K80 GK210 (`sm_37`) at a time. Required modes are `reference`, `b1`, `b2`, `b3`, `adaptive`. Default is reference until the user selects an experiment. Device/model/mode/policy/context must be visible at startup and in saved run metadata.

A small binary Bonsai model is for debugging. The binary 27B model is the main evaluation target and has its own early reference bring-up requirement. Current model-card descriptions identify a hybrid-attention 27B architecture; do not extrapolate its operator compatibility or state management from the small model. Inspect actual GGUF architecture metadata.

## Deliberately narrow first dispatch

Start with verified Q1_0 group-128 weights, supported activation dtype/layout, and single-column eligible linear operations. Keep multi-column prefill, attention/state operators, embedding gathers, normalization, sampling, non-Q1 weights, and unsupported layouts on the reference path. Decode is the primary experiment. Preserve FP32 graph outputs; no persistent compressed hidden-state architecture is required.

Use explicit evaluation-phase policy where the runtime exposes it. A tensor having one column is not proof it belongs to generation: the final prefill microbatch may also have one column. If the POC can only classify by shape, call the policy `single-column`, record that fact, and apply it identically in chat and measurement. Do not market it as decode-only.

## Correctness versus approximation

The basis representation may radically change logits and text. The GPU implementation must nevertheless agree with an independent CPU computation of **the same intended approximation**, within stated FP32 summation tolerance. Exact small mask/popcount tests must pass. There is no requirement to match reference tokens.

Wrong bit order, wrong scales, misaligned access, stale cache state, missing output initialization, omitted operators, broken reductions, silent CPU routing, NaNs or overflow are not licensed approximation methods. Detect non-finite values and stop/report; do not silently clip them and call the result a speedup. Arithmetic changes such as clipping would require an explicit separately named experiment.

## Baseline and scope controls

Reference uses the same executable, model, context, sampler, device and supported runtime features, with experimental interception disabled. Use the best already working Kepler-compatible reference route, not an intentionally slowed toy implementation. Preserve attention precision and KV/state policy across comparisons. No optional drafter, vision tower, speculative decoding, cross-device model splitting, or changes to sampling may enter a headline speedup comparison.

Implement a minimal layer/operator allowlist and exclusions. `all-eligible` is a useful experiment; `mlp-only` or explicit mapped layer sets can locate sensitivity if all-layer approximation collapses. Neither is a different model. Always report actual coverage and policy so excluded layers cannot masquerade as faster arithmetic.

## Delivery boundaries

Do not build a runtime, model format, general quantization framework, GUI, public server, or long research suite from scratch. Reuse the loader, tokenizer, graph, attention, sampler and CLI infrastructure. Windows-friendly launching matters; native Linux is acceptable as a documented execution host, but do not silently require the user to change OS. No WSL K80 acceleration promise.

Deferred: ternary formats, FP64 packing, Q1×Q8 four-kernel benchmark, sketches/engrams, braided streams, cross-GPU per-layer encoding, persistent binary hidden state, training/fine-tuning, learned calibration. X01/X02 are opt-in follow-ups, not required tasks.

## Outcomes

A valid POC can conclude all approximate modes are slower or unusable. It must still distinguish implemented, compiled, K80-tested, and real-model-tested work. If 27B cannot run after concrete compatibility attempts, retain the working smaller-model result and the precise blocker; do not close the full 27B acceptance as achieved.

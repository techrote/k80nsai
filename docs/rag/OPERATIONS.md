# Operations, reproducibility and orchestration

## Workspace and provenance

Work in this repository; do not replace it with a different clone/history. K03 imports the K01-selected source under `vendor/llama.cpp/` or documents an equally reproducible layout. Retain upstream licence notices, exact source SHA and all local modifications. An uncommitted submodule is not a deliverable.

Use one worktree/branch per task. K01/K03 own pin/import; K04 build compatibility; K08 shared interfaces; K09 encoder; K10/K11 consumer; K12 adaptive extension; K13 CLI; K14 instrumentation. Resolve overlapping edits before they happen. Small API changes should land before dependent implementations. The initial publication is documentation and workflow tooling, not an inference implementation.

## Build inputs, not a separate diagnostic application

K02's optional automatic environment-doctor task was retired before publication. Do not implement it as hidden scope. K04 uses ordinary build-tool version output and the available/operator-supplied host information to document its compiler/toolkit/driver assumptions. Missing target hardware or compiler is an explicit blocker, not a passed test.

Select a specific CUDA 11.x and supported host compiler combination. The external Kepler fork's CUDA 11.4/Linux/R470 report is a patch lead, not proof that every current Bonsai operator works. CUDA minor-version compatibility and a toolkit's bundled driver version are distinct. Never assume an arbitrary current driver supports K80.

Emit native sm_37 code, using a compatible CMake architecture setting or explicit gencode, and inspect the actual artifact. Gate unsupported headers/instructions/features narrowly. Half storage/conversion and native half arithmetic are different requirements. A successful CPU build or PTX-only output is not the target build.

Native Windows and native Linux support depend on evidence. Keep user-facing launching straightforward and Windows-friendly where tested. Do not assume WSL supplies K80 GPU acceleration; official WSL support targets later hardware. A container cannot restore unsupported host-driver compatibility. Any OS migration or privileged installation is a separate user decision.

## Hardware reservations

Use an already appropriate power/cooling configuration. Do not change drivers, clocks, power limits, firmware or system-wide policy as part of this workflow. The entire K80 board is reserved for one headline timing run at a time. The second device can be tested sequentially for functional selection; it is not a per-layer encoder/inference pipeline in this POC.

Record available ordinary runtime/device information and measurement limitations without publishing personal host identifiers. No private prompts, credentials or personal paths belong in public evidence.

## Model acquisition

K06 resolves the exact small binary GGUF and the binary text model from `prism-ml/Bonsai-27B-gguf`. Record revision, file SHA-256, bytes, licence, architecture, tensor types and chat template. Publish the small-model identity/template artifact as soon as verified so reference/UI work need not await the 27B download.

Use supplied local files where available. Download only needed text-model assets; no optional vision/drafter components. Keep weights in ignored storage, never Git. Support partial-download recovery and checksum validation. Respect access conditions and do not enable remote-code execution merely to inspect metadata. Filename alone is not immutable model identity.

## Dependencies and evidence

Issue bodies and `docs/workflow.json` contain linked dependencies. They are not assumed to be server-enforced GitHub relations. Milestones are documented evidence checkpoints. Issue state/comments are live status; the manifest is the contract.

A named artifact gate is narrower than whole-issue closure. K07 may publish small-model reference evidence while remaining open on 27B; K10 can use that artifact. Do not translate every artifact edge into a blocking dependency on the producer's full completion. Update atlas and JSON together if gates change.

Use `docs/tasks/Kxx/STATUS.md` with base SHA, touched interfaces, actual commands/exit codes, evidence level, results links and next command. One concise record per task is enough. Open focused implementation PRs. Do not assume self-merge authority from another repository or automatically close hardware-gated issues when code alone merges.

## Packaging

K20 supplies commands for the actually tested native host, handling spaces in paths, working directory, missing model/build and nonzero exit codes. Prefer existing CLI infrastructure over a new frontend dependency. A small evaluation script is acceptable; a GUI/server is not required.

A source ZIP, if produced, uses a versioned inner directory and excludes weights, secrets, local caches and private outputs. Do not claim binaries before a build or bundle driver/toolkit redistributables without checking terms. Final instructions must be exercised from a clean checkout rather than reconstructed from memory.

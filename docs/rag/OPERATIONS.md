# Operations, reproducibility and orchestration

## Workspace and provenance

Start from this repository, not a replacement clone. K03 imports the chosen runtime under `vendor/llama.cpp/` or documents an equally reproducible layout. Keep upstream licence notices and an exact source SHA. The initial workflow publication itself is documentation only.

Use one branch/worktree per issue. K01/K03 own upstream pin/import, K04 build compatibility, K08 shared interfaces, K09 encoder, K10/K11 dot consumer, K12 adaptive extension, K13 UI, K14 instrumentation. Resolve overlaps before simultaneous edits. Small shared-interface PRs should land before dependent implementations; do not guess an API from an unmerged draft.

Issue dependencies are readable links plus `docs/workflow.json`. They are not assumed to be server-enforced GitHub dependency relations. Milestones are documented evidence checkpoints; issue titles include stable task IDs. GitHub issue state/comments are live status; the graph is the dependency contract. Update graph and atlas together if scope/dependencies change.

## Environment preflight

K02 records OS, GPU/device mapping, accessible VRAM, `nvidia-smi` details if present, nvcc/toolkit, compiler, CMake, driver, available disk/RAM, and whether the host is native or virtualized. Avoid personal host identifiers. The tool must distinguish no tool/no GPU/no permission/unsupported version. No privileged installs or system changes.

CUDA 11.x is the intended Kepler compilation family; select and record a particular supported toolkit/host compiler pair rather than asserting every CUDA 11 release works with every compiler. The external Kepler fork documents a CUDA 11.4/Linux/R470 experiment, not proof that current Bonsai plus all operators works. CUDA 11.8 minor compatibility with an older driver has constraints; toolkit-bundled driver version is not the same as the minimum compatibility driver. Never upgrade to an arbitrary current driver that might not support K80.

Build native `sm_37` code (for a compatible CMake, `CMAKE_CUDA_ARCHITECTURES=37-real` or equivalent explicit gencode), and inspect the artifact with cuobjdump or a supported equivalent. CUDA-looking source or a PTX-only artifact is not the agreed deliverable. Gate unsupported BF16, half arithmetic, DP4A, MMA and graph features where actually necessary; do not globally cripple unrelated backends. FP16 storage/conversion and native FP16 arithmetic are different requirements.

Native Windows and native Linux may be supported according to actual evidence. The user's CLI should remain Windows-friendly. Do not propose WSL as a K80 GPU workaround: NVIDIA's WSL support targets Pascal-or-later hardware, not this Kepler target. Docker does not restore unsupported host-driver/GPU support. Any OS migration is a separate user decision.

## Hardware safety and reservations

The board needs an appropriate existing power/cooling setup. Preflight records available temperature/throttling telemetry and warns rather than altering power or clocks. Do not script shutdowns, fan rewiring, driver replacement, BIOS changes, or system-wide security policy changes. One hardware owner runs performance measurements at a time across the board. Functional dual-device independence tests may be labelled separately; no cross-device inference pipeline is in scope.

## Model acquisition

K06 resolves the exact small binary GGUF and `prism-ml/Bonsai-27B-gguf` file/revision, records SHA-256, byte size, licence, architecture, Q1 group interpretation and chat template. Obtain only needed text-model files, not optional mmproj/drafter assets. Do not commit weights, caches, credentials or auth tokens. Use an ignored local models directory and an explicit user-supplied existing file when available.

Use resumable downloads where practical and fail on partial/checksum-mismatched files. Respect gated access/licences without bypass. Do not blindly use `trust_remote_code` or execute repository scripts. Prefer explicit download/inspection functions. Byte identity is authoritative; a friendly filename is not a pin.

## Agent handoff artifacts

Use `docs/tasks/Kxx/STATUS.md` with the task's base SHA, touched interfaces, commands/exit codes, evidence level, results links, blockers and the next exact command. One concise status per issue is enough. Do not create a dozen root scratchpads.

Use PRs for implementation. Commit import/build/API changes separately from kernel changes where practical. A review/merge is not evidence of GPU correctness. Do not auto-close execution-gated issues with a generic `Fixes #...` before their acceptance is satisfied. No self-merge authority is inherited from other repositories.

## Packaging

K20 provides tested commands for the actually supported host, including a PowerShell launcher if that route is supported or a clear native-Linux execution instruction otherwise. Handle paths containing spaces, current directory, missing model/CUDA and nonzero exit codes. Avoid a Python dependency merely for chat if the existing CLI works. A tiny Python evaluation helper is acceptable if documented.

No executable binaries are claimed before a build. No CUDA/driver redistributable bundling without checking distribution terms. A source ZIP, if produced, has a versioned inner directory, no model files, no secrets, and exact source/model instructions.

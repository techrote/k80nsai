# Agent instructions — k80nsai

## Start here

Read your assigned GitHub issue, its comments, [OVERVIEW.md](OVERVIEW.md), [docs/rag/INDEX.md](docs/rag/INDEX.md), and the issue's required reference sections. Read the actual pinned runtime code before editing it. This bundle supersedes earlier conversational implementation sketches where they conflict.

The user authorized a real-model proof of concept, not an expanding formal research programme. Implement the bounded task, test it, leave usable code and exact continuation evidence. Do not merely add scaffolding or another plan.

## Non-negotiable boundaries

- Core modes: reference, B1, B2, B3, adaptive. CLI chat, text only, one selected GK210 device. No model parallelism, per-layer cross-GPU encoding, speculative decoding, training, engrams, or FP64 research in required tasks.
- Model approximation is permitted; memory corruption, invalid tensor interpretation, stale caches, hidden fallback, and invented measurements are not.
- Q1 storage and model graph come from pinned source plus inspected GGUF, not the earlier synthetic 20-byte struct. Preserve upstream licences and do not commit model weights.
- Missing hardware is an external blocker, not evidence of success. Source implementation, target compilation, K80 execution, and real-model evaluation are separate evidence states.
- Keep reference operation available. Prefer default `reference`; experimental mode must be explicit. Report eligible/executed/fallback operations. Do not silently substitute a different model or device.

## Working protocol

1. Check current issue state/dependencies and `git status`; record base commit. Claim the issue in a comment including proposed file ownership and whether K80 access exists.
2. Use `work/Kxx-short-description` in a separate worktree. Do not edit another agent's branch. Do not force-push, reset unrelated work, or import another repository over this repository's docs.
3. Respect ownership locks in the atlas. Shared dispatch/API changes go through K08's owner; source-base changes through K01/K03. Coordinate before overlapping edits.
4. Read only relevant RAG sections plus their dependencies. Follow exact source permalinks. A search/vector database is not required.
5. Implement, test, and open a focused PR linked to the issue. Include actual command lines, exit codes, source/model identities, limitations, and output locations. Do not assume permission to merge your own PRs; retain maintainer review unless explicitly delegated for this repository.
6. Close only on the issue's acceptance evidence. A PR merge alone does not satisfy a hardware-validation gate. Code-ready/hardware-pending work may be committed but must remain explicitly pending where execution is required.

## State and privacy

Keep task notes under `docs/tasks/Kxx/`; results under `results/<run-id>/` once implemented. Do not scatter scratchpads at repository root. Publish only synthetic/public prompts and sanitized host details. Do not expose credentials, private chat logs, personal paths, or email addresses. Use an existing approved commit identity; do not mine accounts for one. Never change drivers, BIOS, power limits, clocks, system policy, or install privileged services without separate explicit authorization.

## Completion comment

State: implemented paths; tests actually run; hardware/model used; results and limitations; dependent task now unblocked; remaining blocker and exact next command. Do not fabricate a benchmark, CI success, test pass, or quality judgment. An approximate mode that genuinely collapses is a valid negative result, not a reason to disguise bugs or omit the mode.

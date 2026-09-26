# K01 research and publication status

Date: 2026-09-26. Scope: source selection, durable Markdown research and evidence-backed instructions for future implementation. No runtime import, kernels, CPU oracle, CLI or inference implementation was performed. Assignment: [#2](https://github.com/techrote/k80nsai/issues/2). Canonical entry: [SOURCE_MAP.md](SOURCE_MAP.md).

## Provenance and live-state baseline

Authoritative repository base: `8deeac521f7a76e92b54bdf2c95d31377b5a4f65`. Dedicated branch: `work/K01-research-and-implementation-handoff`; separate worktree. Initial live checks found only `main`, no pull requests, no assignees or active task claims. Existing #6/#13 comments refine small-model artifact gates; both are retained. K01 was claimed in [this scope/ownership comment](https://github.com/techrote/k80nsai/issues/2#issuecomment-5844816564). No K80 environment is available or claimed.

The local workspace was empty. A normal clone and separate worktree were prepared. The Windows sandbox helper failed before starting a process with OS error 206; approved ordinary shell calls were used for repository operations. This was an infrastructure launch failure, not a source/build failure. No host policy or security settings changed.

Exact GitHub commit API resolutions:

| Repository / role | Commit | Git tree |
|---|---|---|
| ggml-org/llama.cpp selected candidate | `56381e407c0ccfb3a6f71e668a27a901001d22ce` | `24d31963e64a61fc390ecbdd12f7245a9feb73f9` |
| PrismML-Eng/llama.cpp alternative | `d8f26eec76da6d09bb708bcba51ef64b8cd868a3` | `e0b3e2e8cfdcc93a0ff3a6ecdf6d7a96eb6c4e91` |
| babal35/llamacpp-kepler patch lead | `b367989573b1a97e37b001cbf6718365844b94ca` | `44c8551044f26a8c04c8b7637c82cc39ef3b2a73` |
| ggml-org/llama.cpp historical base | `a95a11e5b834057e684712963f90bbb730f4745c` | `6682fa1bda75b74b165710df05feb5aa0c15a290` |

Exact codeload tar snapshots were downloaded once and inspected outside the documentation worktree. Complete candidate tip comparison found 1015 changed paths (not a merge-base diff); relevant changes are classified in the candidate note. Historical Kepler comparison found only two code/build files changed plus README/KEPLER docs. `git diff --no-index` exit 1 indicates differences, not a test failure. No model weights were downloaded; model cards/API identities are marked advertised until K06 verifies acquired bytes.

## Research and independent review

Five bounded investigators cover Q1/activations, model graphs, CUDA11/Kepler, state/CLI/device, and candidate/import differences. They share immutable snapshots, write isolated research reports and do not publish issues, choose the authoritative pin or implement downstream tasks. The lead reconciles and publishes the five focused notes.

Two fresh reviewers completed technical soundness and a cold-start K08 execution review. Both re-read concrete corrections and reported no remaining findings in their bounded scope. [REVIEWS.md](REVIEWS.md) records the missing operator-test build step, server-disabled CLI target, completion-based chat reuse and K08 host-versus-target acceptance correction. These reviews establish document/source consistency, not implementation success.

## Publication status and activation

The reviewed research/work-order snapshot is ready for branch/PR publication. GitHub issue updates and readback are performed after an exact documentation commit and PR URL exist; the final publication ledger is recorded below and in K01/#2. This preparation paragraph describes the pre-publication snapshot, not a claim that issues were already updated. The retained existing-task strategy maps K01/K03–K21 and X01/X02 to their original issue numbers; no implementation work is closed or superseded by planning.

Implementation activation requires maintainer acceptance of the documentation PR, K01 signoff and each existing task/artifact gate. K03 is then the only newly ready implementation task; K04/K05/K06 follow accepted import independently. K04 awaits compatible tooling/native artifact; K06 awaits actual GGUF checksums/metadata; hardware gates await real GK210 execution. No future implementation agents were launched by this run.

## Validation performed and limits

Repository-triggered automation was inspected before issue publication: `.github` contains only the PR template and the GitHub Actions workflow API returned total_count=0. No implementation-triggering workflow or dispatch was added or invoked. This does not assert knowledge of external service configuration.

The existing workflow checker was run during drafting and correctly reported the not-yet-created linked documents. It also exposed a pre-existing Markdown-parser false positive in inline Q2 notation `s[i](...)`; adding an explicit multiplication sign preserves the mathematics and removes the accidental link pattern. The checker was not weakened. Final checks on the assembled reviewed documents passed: `python scripts/check_workflow.py` exit 0; `python scripts/check_workflow.py --live` exit 0 (20 core tasks, 2 deferred; actual issue IDs/titles checked); `git diff --check` exit 0. A bounded Python check validated 135 relative Markdown links/section anchors and 244 pinned upstream file/range links against the shared snapshots, and compared every task/artifact dependency and ownership object with the base manifest unchanged. Counts precede this added review link. The existing checker was not modified. Only research/planning Markdown and workflow navigation metadata are changed.

No CUDA compilation, native-code artifact inspection, K80 test, acquired-model validation, inference, timing, performance estimate or model-quality result was produced. The selected source remains an implementation input, not a certified K80 runtime.
## Final publication ledger

- Documentation PR: [#24](https://github.com/techrote/k80nsai/pull/24), open for maintainer review, not merged. Reviewed research/work-order commit: `181cdcc7343a0bdb316c2c99621007c7f5f7d009`. Issues link that exact existing commit; this ledger is a later publication-status update only.
- Updated actual implementation issues: K03–K21, [#3](https://github.com/techrote/k80nsai/issues/3), [#4](https://github.com/techrote/k80nsai/issues/4), [#5](https://github.com/techrote/k80nsai/issues/5), [#6](https://github.com/techrote/k80nsai/issues/6), [#7](https://github.com/techrote/k80nsai/issues/7), [#8](https://github.com/techrote/k80nsai/issues/8), [#9](https://github.com/techrote/k80nsai/issues/9), [#10](https://github.com/techrote/k80nsai/issues/10), [#11](https://github.com/techrote/k80nsai/issues/11), [#12](https://github.com/techrote/k80nsai/issues/12), [#13](https://github.com/techrote/k80nsai/issues/13), [#14](https://github.com/techrote/k80nsai/issues/14), [#15](https://github.com/techrote/k80nsai/issues/15), [#16](https://github.com/techrote/k80nsai/issues/16), [#17](https://github.com/techrote/k80nsai/issues/17), [#18](https://github.com/techrote/k80nsai/issues/18), [#19](https://github.com/techrote/k80nsai/issues/19), [#20](https://github.com/techrote/k80nsai/issues/20), [#21](https://github.com/techrote/k80nsai/issues/21). Also updated master [#1](https://github.com/techrote/k80nsai/issues/1) and K01 [#2](https://github.com/techrote/k80nsai/issues/2).
- Every original objective/acceptance is retained beneath a source-backed execution supplement. No replacement chain, new implementation issue, closed issue, assignment or changed dependency edge was introduced. X01/#22 and X02/#23 bodies are unchanged. K01 stays open for acceptance.
- Readback verified all 21 published bodies against exact prepared text, allowing only CRLF/LF normalization; 184 exact-commit documentation links/section anchors validated. All remained open/unassigned. The first local CLI publication damaged Unicode punctuation; structured GitHub API updates restored it and the final exact-content comparison passed. Draft material is not being counted as publication.
- Final `python scripts/check_workflow.py --live`: exit 0, 20 core tasks and two deferred extensions. Bounded checks: 136 local links/anchors, 244 pinned source paths/ranges, unchanged task/artifact/ownership objects. `git diff --cached --check` passed before the research commit; trailing blank lines detected during staging were removed.
- Research completion: five investigations, reconciled source pin, durable maps and executable refinements complete. Publication completion: documentation PR and all intended issue updates published/read back. Implementation readiness: pending maintainer acceptance/K01 signoff, then K03; later task/artifact/toolchain/model/K80 gates remain as documented. No hardware-gated issue was closed.

Next permitted action after review is K03's reproducible import, not an automatic continuation of this research run. Source-specific compatibility failures return exact diagnostics to K04/K01/K03; model identity questions to K06; reference graph/fit evidence to K07; shared state/phase/API decisions to K08. No downstream implementation work is authorized by mere publication of this ledger.

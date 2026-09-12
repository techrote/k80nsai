# Workflow publication audit

2026-09-12. Scope: the conversation-derived real-model POC workflow, shared contracts and GitHub task deployment. This is not a report of an implemented inference backend.

## Published structure

The repository contains a master issue, 20 core implementation issues and two deferred extensions: 23 issues total. The [atlas](../OVERVIEW.md) and [JSON graph](workflow.json) map stable task IDs to actual issue numbers. Each implementation issue includes its own prompt, reading pack, dependencies/ownership, acceptance evidence and handoff requirements.

K02 was an optional automatic environment-doctor proposal. Its publication was rejected by the connector and the utility was dropped, rather than moved to another publication route. Ordinary compilation prerequisites remain in K04. No separate diagnostic application is required, and no core mode was removed. The final task IDs deliberately retain this gap.

Milestones and dependency relations are documented, linked and machine-readable. No native GitHub Milestone objects or server-enforced issue-blocking relationships are claimed. Issues are unassigned; source/implementation work should be claimed explicitly before concurrent edits.

## Technical corrections retained

- Actual inspected Q1 ABI: 18-byte group with FP16 scale, not the old synthetic 20-byte structure; safe byte/alignment/stride handling required.
- Pinned candidate source revisions and actual small/27B architecture inspection replace assumptions about moving upstream branches.
- Early 27B reference bring-up, while small-model artifacts independently unblock B1 work.
- Shared encoder/consumer architecture; GPU arithmetic checked against the intended approximation, not identical reference tokens.
- True decode-phase versus single-column policy, visible operation coverage and explicit fallback.
- Context-safe policy changes, including recurrent state, KV and prepared-buffer generations.
- Adaptive computed versus consumed depth, and combined encoding/inference cost.
- Fixed-prefix numerical quality comparisons and actual generated-token accounting.
- Bounded real-model tuning; optional radical kernels remain deferred.

Sources and rationale are in [the source register](rag/SOURCES_AND_DECISIONS.md); implementation contracts are in [the RAG bundle](rag/INDEX.md).

## Validation actually performed during publication

The corrected dependency graph was checked locally with Python: 20 core tasks, two deferred tasks, unique issue numbers, an acyclic dependency graph, valid artifact producers, and no required task depending on an opt-in extension. The checker also rejected four deliberately broken fixtures: a cycle, a core-to-deferred edge, an unknown evidence producer and a duplicate issue number.

GitHub returned successful creation results for issues 1–23 and successful documentation commits. Final references are reconciled against those actual issue numbers, not assumed future numbering. The public connector is used for repository readback. Local Git/network access was unavailable, so a full local clone-based `--live` check is not claimed; the standalone checker is supplied for subsequent clean-checkout validation.

No CUDA compiler execution, K80 kernel test, model download, inference run, speedup, or quality result was produced in this publication task. Earlier generated benchmark files were not present locally, and their previously claimed exhaustive FP64 tests were not rerun. Those limits are intentional and explicit.

## Orchestration correction

A verified artifact is not the same as a closed parent issue. K07 can publish its small-model reference evidence while still working on 27B. K10 depends on that small artifact only. K07 can start from K06's small-model identity/template before the larger acquisition completes. The graph encodes these evidence gates separately to avoid artificial serialization.

The next executable task is [K01/#2](https://github.com/techrote/k80nsai/issues/2), followed by [K03/#3](https://github.com/techrote/k80nsai/issues/3). Build compatibility, reference tests and model acquisition then form the first parallel work lanes.

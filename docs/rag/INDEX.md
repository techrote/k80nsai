# Shared reference / retrieval bundle

Version 1.1. This is a small human- and agent-readable context bundle, not a vector-database project. Read by file and heading. Each issue contains its implementation prompt and links here instead of duplicating all contracts.

## Authority

Current repository scope/contracts govern scope. Actual pinned source and GGUF govern representation and runtime behavior. Issue acceptance governs closure; execution evidence governs performance claims. Earlier conversational sketches are historical proposals, not binding facts. Resolve conflicts in task notes and amend the shared contract before dependent work proceeds.

## Retrieval map

| Question | Read |
|---|---|
| Required product, deferred work, permitted approximation | [CONTRACT.md](CONTRACT.md) |
| B1/B2/B3/adaptive formulas, native format, minimal fixtures | [KERNELS.md](KERNELS.md) |
| Real graph interception, mode API, buffers, reset and fallback | [INTEGRATION.md](INTEGRATION.md) |
| Enough testing, honest timing, quality screening | [VALIDATION.md](VALIDATION.md) |
| Source workspace, build inputs, model acquisition, handoff | [OPERATIONS.md](OPERATIONS.md) |
| Checked sources, candidate commits and corrected assumptions | [SOURCES_AND_DECISIONS.md](SOURCES_AND_DECISIONS.md) |
| Task order, concurrency, artifact gates and blockers | [../../OVERVIEW.md](../../OVERVIEW.md), [../workflow.json](../workflow.json) |
| Publication scope and validation limits | [../DEPLOYMENT_AUDIT.md](../DEPLOYMENT_AUDIT.md) |

## Minimal task packs

K01/K03/K04: CONTRACT, OPERATIONS, SOURCES. K05/K09–K12: CONTRACT, KERNELS, relevant INTEGRATION/VALIDATION sections. K06: model sections of OPERATIONS/SOURCES. K07/K16/K18/K19: VALIDATION and model/dispatch parts of INTEGRATION. K08/K13/K17: INTEGRATION, CONTRACT, VALIDATION. K14/K15: VALIDATION. K20/K21: atlas plus actual evidence, not another general literature review.

All agents read AGENTS.md. Keep new evidence in `docs/tasks/Kxx/`, linked from the issue. Do not paste the entire conversation everywhere or scatter root scratchpads. Candidate source SHAs in this bundle are inspected snapshots, not a claim that those revisions already run on K80. K02 is a retired optional task ID, not a missing prerequisite.

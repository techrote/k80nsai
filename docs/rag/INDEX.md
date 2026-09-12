# Shared reference / retrieval bundle

Version 1.0. This is a small, human- and agent-readable source of shared context, not a vector-database project. Read by file and heading. Issues contain task-specific implementation prompts and link here rather than duplicating every contract.

## Authority

User-approved scope and this repository's current contract govern scope. Actual pinned code/GGUF govern representation and runtime behavior. Issue acceptance governs closure; verified execution evidence governs performance claims. Earlier conversation sketches are historical proposals, not binding facts. Resolve a conflict in the task notes and amend the contract through a focused PR before dependent work proceeds.

## Retrieval map

| Question | Read |
|---|---|
| What is required, deferred, or allowed to be approximate? | [CONTRACT.md](CONTRACT.md) |
| What exactly are B1/B2/B3/adaptive and what must be tested? | [KERNELS.md](KERNELS.md) |
| Where/how can kernels replace real runtime operations safely? | [INTEGRATION.md](INTEGRATION.md) |
| What tests and measurements are enough for this POC? | [VALIDATION.md](VALIDATION.md) |
| How do agents coordinate, build, acquire models, and hand off? | [OPERATIONS.md](OPERATIONS.md) |
| What was checked, what changed, and which sources apply? | [SOURCES_AND_DECISIONS.md](SOURCES_AND_DECISIONS.md) |
| Which task is next and what can run concurrently? | [../../OVERVIEW.md](../../OVERVIEW.md) and [../workflow.json](../workflow.json) |

## Minimal task packs

K01–K04: CONTRACT, OPERATIONS, SOURCES. K05/K09–K12: CONTRACT, KERNELS, relevant INTEGRATION and VALIDATION sections. K06: model identity sections of OPERATIONS and SOURCES. K07/K16/K18/K19: VALIDATION plus model/dispatch sections of INTEGRATION. K08/K13/K17: INTEGRATION, CONTRACT, VALIDATION. K14/K15: VALIDATION. K20/K21: atlas plus actual evidence, not a fresh general literature review.

All agents read AGENTS.md. Do not paste the whole conversation into every issue. Store new evidence in `docs/tasks/Kxx/` and link it from the issue. Keep source SHAs, models and benchmark policy in structured records when implementation creates them. This bundle contains observed candidate source SHAs, not an assertion that those revisions already compile or run on K80.

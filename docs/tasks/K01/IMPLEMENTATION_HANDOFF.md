# K01 implementation handoff and publication gates

Purpose: executable handoff from source research to the existing implementation programme. Status: proposed documentation; implementation activation requires maintainer acceptance of the K01 documentation PR and the prerequisites below. Verified: 2026-09-26. Source selection: [SOURCE_MAP.md](SOURCE_MAP.md). Research/publication evidence: [STATUS.md](STATUS.md).

## Which execution route is authoritative?

Retain the existing K task IDs and GitHub issues. No replacement chain, new namespace, superseded issue, or completed implementation is created by this research run. K02 remains retired; X01/X02 remain deferred. The existing [workflow manifest](../../workflow.json) remains the dependency authority. Enriched issue bodies retain their original objective and acceptance, adding source evidence and explicit handoffs. No historical evidence or comments are removed.

Before acceptance, the exact commit linked from each updated issue is a reviewable proposal. Future agents must check the PR and issue comments; existence of an issue is not readiness. After acceptance, K01's source-selection acceptance can unblock K03. Do not import from an unaccepted pin, infer permission to merge, or treat documentation acceptance as a GPU result.

## What must every future agent do before editing?

1. Read the assigned issue and all its comments, `AGENTS.md`, its ordered reading pack and the exact predecessor artifacts named below. Check live claims, PRs, repository status and accepted implementation commit.
2. Record the base commit and claim bounded file ownership in the assigned issue. Create a separate `work/Kxx-...` worktree. If a shared file has an owner, coordinate before editing it.
3. Verify each whole-task prerequisite from accepted evidence, and each named-artifact gate from its producer's linked artifact. An open parent may have a usable accepted artifact. Conversely an issue's closed state alone does not establish missing hardware evidence.
4. Implement only the assigned task. Planned flags, descriptors, filenames and commands are proposals until their owning predecessor publishes the actual implementation. Consume K08's accepted interface document and symbols; do not invent a parallel API.
5. Run the task's checks, `python scripts/check_workflow.py` when documents/dependencies change, and a clean-checkout reproduction where required. Submit a focused PR without assuming self-merge authority.

## What is the evidence and output contract?

Each task publishes its primary artifact below plus `docs/tasks/Kxx/STATUS.md`. Include implementation/base/upstream commits, files owned, dependency artifact links and revisions, exact commands with working directory and exit codes, tool versions, evidence classification, failing-case reproducer, remaining blockers, next command, and tasks now unblocked. Proposed commands must be labelled until implemented and executed.

Hardware records also identify the exact model repository revision, GGUF SHA-256 and architecture, selected GK210 UUID or stable device identity (sanitized), compute capability, toolkit/driver, context and state precision, CPU fallback, offload policy, sampler/template, coverage, actual tokens, timing boundaries and raw output paths. Use `results/<run-id>/` for public/synthetic results. No weights, credentials, private prompts, machine-specific private paths or build debris enter Git.

Distinguish source-inspected, host-tested, sm37-compiled, k80-tested and model-tested. Missing evidence is `blocked` or `pending` with reason, never a zero measurement or implicit pass. A failed compatibility experiment is useful evidence; it does not satisfy an execution acceptance criterion.

## How do the existing issues map to the handoff?

All mappings are identity mappings; there are no successor issue numbers.

| Existing task / retained issue | Required output and consumers | Start and completion distinctions |
|---|---|---|
| K01 [#2](https://github.com/techrote/k80nsai/issues/2) | `SOURCE_MAP.md`, supporting research, this handoff; K03 | Documentation acceptance; source evidence only |
| K03 [#3](https://github.com/techrote/k80nsai/issues/3) | `docs/tasks/K03/IMPORT.md`, tracked upstream lock, licence, clean source tree; K04/K05/K06/K08 | K01 accepted; one import owner |
| K04 [#4](https://github.com/techrote/k80nsai/issues/4) | `docs/tasks/K04/BUILD.md`, toolchain manifest, verbose build and native-code evidence; K07/K09 | K03; compiler/native sm_37 evidence required, no K80 inference claim |
| K05 [#5](https://github.com/techrote/k80nsai/issues/5) | `docs/tasks/K05/REFERENCE.md`, independent oracle and reproducible fixtures; K07/K08/K09–K12/K17 | K03; host tests independent of build/model lanes |
| K06 [#6](https://github.com/techrote/k80nsai/issues/6) | `docs/tasks/K06/MODELS.md`; `small_model_identity_and_template` and separate 27B identity/template artifact; K07/K13/K15 | K03; publish small artifact early, both files needed for full closure |
| K07 [#7](https://github.com/techrote/k80nsai/issues/7) | `docs/tasks/K07/REFERENCE_RUNS.md`; `small_model_reference_on_K80` and separate 27B reference record; K10/K16 | K04/K05 + K06 small artifact; full closure also K06 and both model runs |
| K08 [#8](https://github.com/techrote/k80nsai/issues/8) | `docs/tasks/K08/INTERFACE.md`, accepted implemented descriptor/call/config/reset/telemetry contract; K09/K10/K13/K14/K17 | K03/K05; sole shared-interface owner |
| K09 [#9](https://github.com/techrote/k80nsai/issues/9) | `docs/tasks/K09/ENCODER.md`, shared fixed-depth encoder and oracle comparisons; K10/K17 | K04/K05/K08; actual K80 fixture execution for closure |
| K10 [#10](https://github.com/techrote/k80nsai/issues/10) | `docs/tasks/K10/B1.md`, common consumer and nonzero real-model coverage; K11/K16/K17 | K08/K09; K07 small reference artifact before closure, not whole K07 |
| K11 [#11](https://github.com/techrote/k80nsai/issues/11) | `docs/tasks/K11/B2_B3.md`, shared consumer extension; K12/K16/K17 | K10; sequential shared consumer integration |
| K12 [#12](https://github.com/techrote/k80nsai/issues/12) | `docs/tasks/K12/ADAPTIVE.md`, actual computed/consumed depths; K16/K17 | K11; coordinate both encoder/consumer ownership |
| K13 [#13](https://github.com/techrote/k80nsai/issues/13) | `docs/tasks/K13/CHAT.md`, tested persistent CLI and reset policy; K15/K16/K17/K20 | K03/K08; small identity/template artifact needed for real chat acceptance |
| K14 [#14](https://github.com/techrote/k80nsai/issues/14) | `docs/tasks/K14/MEASUREMENT.md`, runner/counter schema; K15/K16 | K08; mock timing/counter checks may be host-only |
| K15 [#15](https://github.com/techrote/k80nsai/issues/15) | `docs/tasks/K15/QUALITY.md`, fixed corpus and recoverable output bookkeeping; K16 | K06/K13/K14; no automatic generated-code execution |
| K16 [#16](https://github.com/techrote/k80nsai/issues/16) | `docs/tasks/K16/27B_FRONTIER.md`, all-mode raw observations; K18/K19 | K07/K10/K13/K14 to start, K11/K12/K15 also to close |
| K17 [#17](https://github.com/techrote/k80nsai/issues/17) | `docs/tasks/K17/REGRESSIONS.md`, compact lifecycle/layout regressions; K18/K19 | K08/K09/K10 to start; K11/K12/K13 also to close |
| K18 [#18](https://github.com/techrote/k80nsai/issues/18) | `docs/tasks/K18/TUNING.md`, bounded measured retain/reject decision; K19 | K16/K17; exclusive shared hot-code edits and board reservation |
| K19 [#19](https://github.com/techrote/k80nsai/issues/19) | `docs/tasks/K19/ACCEPTANCE.md`, clean build and all-mode/model execution; K20/K21 | K16/K17/K18; freeze changes during measurements |
| K20 [#20](https://github.com/techrote/k80nsai/issues/20) | `docs/tasks/K20/HANDOFF.md`, tested launch/package identity; K21 | K13 to start; K19 also to close |
| K21 [#21](https://github.com/techrote/k80nsai/issues/21) | `POC_REPORT.md`, evidence and next-step decision; maintainer | K20/K19; deferred extensions still require owner admission |

Paths above are required future outputs, not existing artifacts. The named small-model artifacts retain the exact schema-v2 gate names. The 27B identity/reference records are separately published sections or artifacts in K06/K07; they do not replace either task's whole-issue acceptance.

## What is ready now and after acceptance?

At research publication: K01 is source-research-complete subject to review; no downstream implementation has started or is claimed ready. After the documentation PR is accepted and K01 signed off, K03 alone is ready for bounded import. After K03, K04/K05/K06 can proceed independently. K08 requires the host oracle and import, not K04 completion. K08 closes on host-tested configuration/eligibility/generation/reset and injected-callback contracts plus reviewed adapter placement; CUDA compilation or K80 execution is not its closure gate. Its INTERFACE.md must mark native adapter and stream behavior target-unverified. K09 rebuilds integrated native glue, K10 verifies the original-input seam and real routing, and K17 verifies actual stream/lifetime/reset behavior. Encoder, CLI and instrumentation separate only after the actual K08 interface is accepted.

No K80 is available in this research run. K04 still needs a compatible CUDA 11 host compiler/toolkit and inspected native artifact. K06 still needs exact acquired-file checksums and GGUF inspection. K07 and all GPU acceptance gates still need the actual selected GK210. Use existing user-approved environments; no driver, BIOS, clocks, power, runner or security changes are authorized by this handoff.

The whole K80 board is one headline measurement reservation. Record reservation owner/window in the executing issue; verify no competing board work before timing. The second GK210 is not a per-layer worker. Functional tests on another GPU are labelled separately and cannot satisfy a K80 gate.

## How are unresolved findings assigned?

The [source map](SOURCE_MAP.md) assigns each source/build/model uncertainty to K04, K06, K07 or K08. A runtime repin requires a concrete failing reproducer, bounded comparison within the inspected candidates, K01/K03 coordination and a documented rebase plan for dependent branches. Do not chase a new upstream head as a routine workaround.

If blocked, publish the exact failing command/operator/dtype/shape or missing input; preserve partial useful work; leave the acceptance open. Resume with the named missing input and rerun that failing stage, then the relevant reference/oracle regression. Do not expand scope or reinterpret an unmet hardware gate to obtain closure.

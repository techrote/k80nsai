# Independent review record

Verified: 2026-09-26. Two fresh reviewers were not primary investigators. Neither implemented runtime code, mutated GitHub or claimed target execution. The lead corrected concrete findings and both reviewers re-read the corrections before reporting no remaining findings in their bounded scope.

## Technical soundness review

Read set: AGENTS/RAG contracts, SOURCE_MAP, handoff/work orders, all five research notes and selected lines of the four exact source snapshots. This was a source spot-check, not a whole-runtime correctness audit.

| Finding | Evidence and correction | Disposition |
|---|---|---|
| P2: build requested unavailable `llama-cli` with SERVER off | Selected `tools/CMakeLists.txt:24–28` only adds CLI inside LLAMA_BUILD_SERVER. Build note now explicitly enables tools and requests `ggml-cuda llama-completion`, preserving server-off configuration. | Re-read and resolved |
| P2: operator tests not built before proposed invocation | Initial compile isolated backend with LLAMA_BUILD_TESTS=OFF. Build note now reconfigures the same recorded cache with tests ON and explicitly builds `test-backend-ops` before operator probes. | Re-read and resolved |
| P2: chat plan overlooked fuller in-process reuse | `tools/completion/completion.cpp` already supplies common parsing, Jinja/model/context/sampler and prompt/generation loop. State note, INTEGRATION and K13 work order now prefer completion/llama-completion-impl; simple-chat is only a concrete-obstruction fallback. | Re-read and resolved |

The reviewer independently checked ABI18/bit order, original F32 to Q8_1 boundary, CPU Q8_0 distinction, cc370 MMVQ/MMQ/fusion predicates, GDN selectors and CS64 solve, epsilon-placement formulas, hybrid clear and recurrent wildcard caveat, filtered-device main_gpu indexing and semantic-phase limits. It confirmed that source selection remains conditional on compile/file/device evidence. Native adapter verification was also checked as explicitly deferred from K08 host acceptance to K09/K10/K17.

## Cold-start implementation review

Input was one proposed complete K08 issue and only the repository documentation it linked. No investigator reports, source snapshots or conversational background were supplied as missing implementation context. Linked state documentation was initially absent; the reviewer re-read it when delivered and retired that absence finding.

**P2: K08 acceptance could be mistaken for requiring CUDA or downstream kernels.** The graph makes K08 depend on K03/K05 and marks it host-only, but the draft graph/call-site test wording did not say which tests can close it before K09/K10. Waiting for those dependent kernels would create unnecessary blocking.

Correction: K08 host fixtures cover configuration, eligibility/strides, overflow, generation/reset and injected callbacks with original F32 values; reviewed source establishes adapter placement. Doubles do not certify target execution. INTERFACE.md must mark native adapter/stream verification unverified; K09 rebuilds integrated native glue, K10 proves real original-input interception/model routing, K17 proves target stream/lifetime/reset. Missing required host compiler or predecessor artifacts blocks K08; absence of CUDA/GK210 does not block its host acceptance. The issue draft, work order, handoff and downstream obligations were synchronized and re-read.

The reviewer found no remaining concrete missing input, invented predecessor API, dependency deadlock, ownership conflict, output/handoff gap or reliance on private context in the reviewed K08 pack. Actual predecessor artifacts and live claims remain future start gates. This cold review samples K08; it does not claim each later issue was independently cold-executed.

## Limits and remaining gates

No compiler, K80, acquired GGUF, inference or performance checks were performed by either reviewer. K04/K06/K07 and subsequent target-validation tasks remain necessary. No review finding changes mathematical approximation definitions, default reference policy, model target, single-device scope or deferred-X admission rules.

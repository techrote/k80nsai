# K04 implementation and evidence status

Task: [K04/#4](https://github.com/techrote/k80nsai/issues/4).
**Build preparation implemented and host-regression-tested; CUDA compilation-pending.**
K04 acceptance remains open. Reproduction, exact tool paths and continuation:
[BUILD.md](BUILD.md).

## Revisions, dependencies and owned interfaces

- Fetched main / accepted K03 merge: `fcf54b8671035b3959e851d842bbf5c8e7ba2fe7`.
- K03/#3 closed completed, PR #25 merged; verified before edits on 2026-09-26.
- Pristine import: `25ef5ba867cdc6bc0089d9cb05e7d89090f71560`.
- Frozen upstream: `ggml-org/llama.cpp@56381e407c0ccfb3a6f71e668a27a901001d22ce`.
- Upstream tree: `24d31963e64a61fc390ecbdd12f7245a9feb73f9`.
- Implementation: `09f620d64d2403d84aba4c327e9afec3aaffa56b`.
- Branch: `work/K04-sm37-compat`; [bounded claim](https://github.com/techrote/k80nsai/issues/4#issuecomment-5845631837).
- Owned: root opt-in switch, `cmake/k04-cuda.cmake`, `scripts/inspect_sm37.py`,
  `tests/test_inspect_sm37.py`, this task's documentation/evidence, README link.
- Accepted inputs: [K03 IMPORT](../K03/IMPORT.md), [K03 STATUS](../K03/STATUS.md),
  [lock](../../../vendor/llama.cpp.lock.json), [K01 K04 work order](../K01/WORK_ORDERS.md#k04--issue-4)
  at the accepted base. No shared dispatch/interface edits or vendor patches.
- No open PR or competing K04/K08/shared build claim at start. Lead retained writes;
  toolchain, compatibility and provenance investigators were read-only.

## Evidence classification

| Evidence | State / precise boundary |
|---|---|
| source-inspected | yes; exact pinned source/guards/targets, no repin |
| implemented | yes; explicit CUDA11/37-real build lane and backend inspection tool |
| configured | **K03 yes; K04 no**. Real K04 configure stops before compiler at missing CUDA root |
| sm37-compiled | **no** |
| native-artifact-inspected | **no**; artifact path/hash unavailable |
| operator-test-compiled | **no**; test-backend-ops target confirmed in source only |
| host-regression-tested | yes; default K03 configure/build, CTest 2/2 and direct smoke |
| inspection-harness-tested | yes; 7 synthetic negative/positive parser tests, not CUDA output |
| k80-tested | **no** |
| other-GPU-functional-tested | **no**; GTX1650 SUPER inventory only |
| model-tested / Q1 correctness / quality / speed | **no** |

## Executed validation

Working directory: checkout root, unless named otherwise in each transcript.
[Environment](evidence/environment.txt), [preflight and parser tests](evidence/preflight.txt),
[host configure/build/CTest](evidence/host-regression.txt),
[workflow local/live](evidence/workflow.txt) retain exact argument vectors and exits.

- `python scripts/verify_upstream.py`: 0, 3,583 files / 171,774,604 bytes and notices.
- Fresh K04 Ninja configure without a toolkit: expected 1, exact missing-root error.
- Fresh PTX-only, system-ggml and CUDA-OFF negative controls: expected 1 each.
- `python tests/test_inspect_sm37.py`: 0, seven tests; no real backend artifact used.
- Default VS2022 host configure/build/CTest/direct smoke: 0; CTest 2/2, CPU only.
- `python scripts/check_workflow.py` and `--live`: 0.
- `git diff --check`, vendor diff from pristine import: 0.

No CUDA header/TU/template compilation was attempted. There are no CUDA compiler
failures to list beyond the missing-tool prerequisite, and no compatibility patch
to accept. All vendor bytes, Git modes, lock/provenance verifier and licences remain
unchanged. The [empty patch ledger](BUILD.md#patch-ledger-and-integrity) explains
why future vendor changes must first extend deterministic verification.

## Actual environment, limitations and next command

Windows11 AMD64, CMake4.4.3, Python3.14.7. Existing VS2019/MSVC19.29.30159.0 and
Ninja1.10.2 are the continuation candidate; VS2022/MSVC19.44.35228.0 is the tested
K03 host compiler. **CUDA toolkit/nvcc/cuobjdump are absent.** No CUDA version is
inferred from nvidia-smi. Device inventory: GTX1650 SUPER cc7.5 driver616.92,
not K80. No installations/system changes or WSL substitution occurred.

Exact missing input: operator-supplied complete CUDA11.8 toolkit at a recorded
absolute path, including cuBLAS/runtime headers/libraries and matching cuobjdump.
Once available, execute the [Windows continuation block](BUILD.md#exact-cuda-continuation-commands-not-executed),
starting with its VS2019 environment and `nvcc --version`, then the explicit root
CMake configure and `ggml-cuda llama-completion` build. Preserve the first real
diagnostic, implement only its narrow fix with provenance, repeat the full build,
inspect the actual backend, compile test-backend-ops, and rerun K03.

Positive CUDA configure and collector integration with real cuobjdump remain
unverified. BF16/forced half compute, batched/broadcast Ex library calls and fused
27B operators remain execution risks; see BUILD's exact paths. Native compile
success alone would not establish K80, model or library execution.

**Tasks now unblocked by this PR: none of K07/K09's execution gates.** Useful
build/inspection commands are prepared, but K04 remains open until native target
acceptance is evidenced and reviewed. No self-merge, issue closure or dependent
task implementation was performed.

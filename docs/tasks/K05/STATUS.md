# K05 implementation and evidence status

Task [K05/#5](https://github.com/techrote/k80nsai/issues/5).
**Implemented and host-tested; ready for maintainer review.**
K05 is not self-accepted, merged or closed. Mathematical/API specification,
reproduction commands and downstream handoff: [REFERENCE.md](REFERENCE.md).
Durable executed command/output/exit record:
[clean-checkout.txt](evidence/clean-checkout.txt).

## Revisions, prerequisites and ownership

- Accepted K03/main base: `fcf54b8671035b3959e851d842bbf5c8e7ba2fe7`.
  PR #25 merged; issue #3 closed completed, verified live before claim and commit.
- K05 implementation/test/build commit:
  `8a07e2ebf95aafdce199bbf7110cbfb56749e721`.
  This is the clean-build oracle revision. Final test-only diagnostic correction:
  `0c5b6b9482d15efb6681c7fab7372b3fbd813203`; later publication is documentation
  and evidence only. The PR and issue evidence comment identify the final head.
- Immutable upstream: `ggml-org/llama.cpp@56381e407c0ccfb3a6f71e668a27a901001d22ce`;
  tree `24d31963e64a61fc390ecbdd12f7245a9feb73f9`.
- Pristine import: `25ef5ba867cdc6bc0089d9cb05e7d89090f71560`.
  [Lock](../../../vendor/llama.cpp.lock.json) and vendor files are unchanged.
- Predecessor K01 reviewed artifacts:
  `181cdcc7343a0bdb316c2c99621007c7f5f7d009`; final accepted ledger
  `1481eb016c93e633ad173d68adb4cb563d53cdf1`, via PR #24.
  Read [SOURCE_MAP](../K01/SOURCE_MAP.md),
  [Q1 source investigation](../K01/research/q1-abi-and-activation-path.md),
  [IMPLEMENTATION_HANDOFF](../K01/IMPLEMENTATION_HANDOFF.md), and the
  [K05 work order](../K01/WORK_ORDERS.md#k05--issue-5).
- K03 host integration revision `cd24a030b4a9ddfb1f39d08d1bb114e8c7606049`,
  final ledger `ad3dc0d`, accepted by the base merge. Read
  [IMPORT](../K03/IMPORT.md) and [STATUS](../K03/STATUS.md).
  Historical pending language there is superseded by live K03 completion.
- Dedicated branch/worktree: `work/K05-q1-cpu-oracle`.
  [Bounded claim](https://github.com/techrote/k80nsai/issues/5#issuecomment-5845809239).
- Owned paths: `reference/q1/**`, `tests/k05/**`, `docs/tasks/K05/**`.
  Lead made all writes; reviewers were read-only.
- Before claim, issue #5/all comments, open PRs and K04/K08/K09 claims were read.
  K04 PR #26 remains open at `2233fcf30c2f2c3389a0fa8ef697e913beb1805c`;
  K08/K09 had no claims. Root build/K04/vendor/shared runtime files were not edited.
  No unrelated worktree was changed.

## Delivered behavior

Eight implementation/build/test files expose one small C++17 static reference
library, a standalone host CMake/CTest project, ABI assertion executable,
deterministic fixture helpers and a process-level negative-control driver.

The native decoder preserves the independently interpreted IEEE binary16 scale,
LSB-first signs and 18-byte stride. It supports byte offsets, row padding and
multiple rows with checked bounds and complete g128 groups. The original-FP32
dot never uses Q8 or runtime dot code. Fixed B1/B2/B3, reconstruction, exact
integer mask identity and adaptive <= threshold decisions are implemented with
explicit computed/consumed depths. Zero adaptive groups perform depth-zero work;
non-finites, overflow, underflowed nonzero E0 and malformed inputs reject visibly.

## Actual clean-checkout validation

Lead created an independent full clone using `git clone --no-local --branch
work/K05-q1-cpu-oracle <SOURCE> "<CHECKOUT>"`. This transfers objects without
sharing source hardlinks or alternates. The clone contains implementation commit
8a07e2e, has no `.git/objects/info/alternates`, and passes `git fsck --full`.
Source and both build paths contain spaces. Builds are out of source.
The clone's working tree was clean before and after the full sequence.
No source-cache, installed ggml, model or CUDA input was used.

The transcript retains the exact cwd, argument vectors, UTC timestamps, outputs
and exit codes. Symbolic paths replace workspace/profile prefixes; trailing
whitespace/line endings are normalized. These are actual results, not proposed
commands:

| Command/check (cwd: clean checkout) | Exit and result |
|---|---|
| Git clone / fsck / initial status | 0; independent clone, clean |
| git/cmake/ctest/python/gh version commands | 0; versions below |
| `python scripts/verify_upstream.py` | 0; 3,583 files / 171,774,604 bytes; committed/import trees, index modes, working bytes and notices intact |
| `git rev-parse HEAD:vendor/llama.cpp` | 0; exact frozen upstream tree |
| `cmake -S tests/k05 -B "<K05_BUILD>" -G "Visual Studio 17 2022" -A x64` | 0 |
| `cmake --build "<K05_BUILD>" --config Release --parallel 2` | 0; oracle and both test executables built, no K05 warnings |
| `ctest --test-dir "<K05_BUILD>" -C Release --output-on-failure` | 0; **3/3** |
| `<K05_BUILD>/Release/k05-reference-tests.exe` | 0; **282 cases / 34,187 checks**, seed 4927541 |
| Same executable `--seed 1` | 0; 282 cases / 34,178 checks |
| Same executable `--seed 4294967295` | 0; 282 cases / 34,181 checks |
| `python tests/k05/negative_controls.py <K05_BUILD>/Release/k05-reference-tests.exe` | 0; **42** subprocess controls; each child exited 1 with required diagnostics |
| `--inject decode/mask/basis/adaptive` (four separate invocations) | **1 expected**; exact seed/input/expected/actual dumps retained |
| `--reject half-inf`, `--reject energy-product` | **1 expected**; mutated scale/activation and explicit rejection retained |
| `cmake -S . -B "<K03_BUILD>" -G "Visual Studio 17 2022" -A x64` | 0; unmodified root profile |
| `cmake --build "<K03_BUILD>" --config Release --target k80nsai-host-smoke --parallel 2` | 0; bundled CPU llama/ggml |
| `ctest --test-dir "<K03_BUILD>" -C Release --output-on-failure` | 0; **2/2** |
| `<K03_BUILD>/Release/k80nsai-host-smoke.exe` | 0; CPU initialization, no model; full upstream/implementation IDs, configure dirty=false |
| `python scripts/check_workflow.py` and `--live` | 0 each |
| `git diff --check <BASE>` | 0 |
| `git diff --exit-code <IMPORT> HEAD -- vendor/llama.cpp` | 0; empty |
| Final `git status --porcelain=v1` | 0; empty |

K03 emitted the existing upstream CMake CMP0194 and MSVC
MSB8027/C4297/C4244/C4834 warnings, retained in the transcript. In particular,
MSB8027 identifies the upstream llama.cpp/model llama.cpp unity-build naming
collision previously recorded by K03. Its bounded build/link/initialization passed;
this does not prove model execution. No vendor change was made to suppress warnings.

## Review, falsification and reproduction

Read-only native ABI and mathematical reviewers read the accepted sources and
the implementation. They found no blocking ABI, independence or mathematical
defect. The review led to exact +/-128 all-unit dot fixtures, actual mutated-input
diagnostics, hexfloat adaptive thresholds, expected/actual integer failure output,
and extra invalid/consumed-depth tests. These were included before the clean build; a final random-case context correction
was subsequently rebuilt and tested in that independent clone.

The 38 rejection fixtures cover malformed block counts, incomplete columns,
short storage, row/group/offset bounds, invalid strides/type/depth/coefficients,
non-finite activations/scales/thresholds, FP32 energy/alpha/dot overflow and nonzero
energy underflow. Return-by-value APIs avoid caller output-buffer overruns; input
sentinels and unused poisoned planes are tested. The four intentional wrong
expectations verify diagnostic/nonzero-exit behavior, rather than falsely
recording those expected failures as ordinary test failures.

A cold reviewer independently reran the clean default/UINT32_MAX binaries, ABI
assertions, all 42 controls and K03 smoke, checked the clean clone/no alternates,
and found no core arithmetic/layout blocker. Their final finding was that random
adaptive context should precede encoding and include the active group plus
expected/actual depths. Test-only commit 0c5b6b9 addresses it.

[Final-source transcript](evidence/final-source.txt): the independent clone was
fast-forwarded to 0c5b6b9 (exit 0). A Git diff proved the oracle, root K03 build,
host smoke and vendor unchanged from the fully built revision (exit 0). K05
Release rebuilt; CTest **3/3**, default **282 cases / 35,339 checks**, seed 1
**35,330 checks**, UINT32_MAX **35,333 checks**, all **42** controls, source
verifier, live workflow, whitespace and clean status all passed (exit 0).
The check-count increase splits computed/consumed depth into separate assertions;
it adds no random workload and changes no oracle arithmetic. K03's clean 2/2
regression therefore remains applicable. The cold reviewer then checked the final
diagnostic correction, STATUS, REFERENCE and final-source transcript and found no
remaining blocker. Further changes are documentation only.

To reproduce a failure, use the exact seed or `--reject <name>` /
`--inject <name>` command in REFERENCE and the recorded input dump. The acceptance
suite itself should exit zero. No corrected mathematical/layout defect was found
requiring a separate regression input; the review corrections were diagnostics
and explicit analytic coverage.

## Host, evidence classification and limits

Executed on native Windows AMD64, OS version 10.0.26200, Windows SDK 10.0.26100.0;
MSVC 19.44.35228.0 (toolset 14.44.35207), MSBuild 17.14.51;
CMake/CTest 4.4.3; Python 3.14.7; Git 2.55.0.windows.5;
GitHub CLI 2.100.0. No tools, drivers, services or system settings were installed
or changed.

Classification: **source-inspected, implemented, host-built, host-tested**.
No CUDA/native sm_37, K80/GK210, model, performance or language-quality evidence.
GPU/model/toolkit fields are not applicable to this host-only run. No model
weights were downloaded. No Linux/macOS, big-endian or arbitrary-FP-environment
execution is claimed. Native host byte order and IEEE gradual-underflow
preconditions are explicit in REFERENCE.

## Remaining gate and exact handoff

No technical K05 host blocker remains. Maintainer review/acceptance is pending;
no self-merge or issue closure is authorized or performed.

Next check from an ordinary checkout:
`cmake -S tests/k05 -B ../build-k05 -G "Visual Studio 17 2022" -A x64`,
then build Release and CTest as recorded above. The independent library target
can be integrated later without overlapping K04's current root-build ownership.

After accepted K05, **K08 may start** using this host oracle and accepted K03.
K07/K09–K12/K17 can consume the layout/math/fixture artifacts only when their
other dependency gates are met. K09 uses fixed planes; K10/K11 use original,
reconstructed and mask dots; K12 uses exact adaptive boundaries and actual
computed/consumed depths; K17 reuses view/failure/poison fixtures. K07 retains
K04 and K06 small-model gates. No K08/K09 implementation was started.
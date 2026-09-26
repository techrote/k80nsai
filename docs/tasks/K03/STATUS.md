# K03 implementation and evidence status

Task: [K03/#3](https://github.com/techrote/k80nsai/issues/3). **Implemented and
host-tested; ready for maintainer review.** Acceptance remains pending. Scope,
import method and continuation: [IMPORT.md](IMPORT.md). Full independent evidence:
[clean-checkout transcript and link audit](evidence/clean-checkout.txt).

## Revisions and ownership

- Accepted main/K01 base: `58a100dba15f9bca08c80a892224a342c8819f75`.
- Predecessor reviewed source/handoff/work-order revision:
  `181cdcc7343a0bdb316c2c99621007c7f5f7d009`; final K01 ledger:
  `1481eb016c93e633ad173d68adb4cb563d53cdf1`, accepted via PR #24 and
  [explicit signoff](https://github.com/techrote/k80nsai/issues/2#issuecomment-5845335994).
- Pristine import (commit A): `25ef5ba867cdc6bc0089d9cb05e7d89090f71560`.
- Root integration (commit B), independently tested branch head:
  `cd24a030b4a9ddfb1f39d08d1bb114e8c7606049`.
- Upstream: `ggml-org/llama.cpp@56381e407c0ccfb3a6f71e668a27a901001d22ce`.
- Upstream tree: `24d31963e64a61fc390ecbdd12f7245a9feb73f9`.
- Subsequent publication changes only this evidence/handoff; no build/source
  changes after the tested integration. The PR/issue evidence names final head.
- Owned: `vendor/llama.cpp/**`, `vendor/llama.cpp.lock.json`, `.gitattributes`,
  root `CMakeLists.txt`, `cmake/provenance.*.in`, `tests/host-smoke.cpp`,
  `scripts/verify_upstream.py`, `README.md`, `docs/tasks/K03/**`.
  No existing workflow contracts, dependency edges or checker were changed.

## Actual validation

Independent reviewer cloned commit B with `git clone --no-local --branch
work/K03-runtime-import <SOURCE> "<CHECKOUT>"` (exit 0). Source and build directory
names contained spaces; the build was outside source. The clone has no Git object
alternates and `git fsck --full` exited 0. It did not access the acquisition cache,
an external source checkout or installed llama/ggml. It remained clean after all
checks. Public symbolic paths and every command's working directory, exact
argument vector, exit code and output are retained in the transcript.

All following commands ran from that clean checkout unless otherwise stated:

| Command/check | Exit / actual result |
|---|---|
| `python scripts/verify_upstream.py` | 0; HEAD/import tree, index modes, 3,583 current files, 171,774,604 bytes, all notices; no exclusions |
| `git rev-parse HEAD:vendor/llama.cpp` | 0; `24d31963e64a61fc390ecbdd12f7245a9feb73f9` |
| `cmake -S "<CHECKOUT>" -B "<BUILD>" -G "Visual Studio 17 2022" -A x64` | 0 |
| `cmake --build "<BUILD>" --config Release --target k80nsai-host-smoke --parallel 2` | 0; bundled llama/ggml CPU compiled and linked |
| `ctest --test-dir "<BUILD>" -C Release --output-on-failure` | 0; 2/2 passed |
| `"<BUILD>\Release\k80nsai-host-smoke.exe"` | 0; CPU only, no model loaded; upstream SHA and full implementation SHA distinct; configure dirty=false |
| `python scripts/check_workflow.py` | 0; 20 core tasks / 2 deferred |
| `python scripts/check_workflow.py --live` | 0; live mapping intact |
| `git diff --check` | 0 |
| `git diff --exit-code 25ef5ba867cdc6bc0089d9cb05e7d89090f71560 HEAD -- vendor/llama.cpp` | 0; no upstream modifications |
| `git status --porcelain=v1` | 0; empty before/after validation |

The lead additionally resolved the upstream Git commit/tree from an independent
exact-SHA fetch and GitHub's commit API, ran the verifier with `--upstream-repo`
(exit 0), checked root licence SHA-256, ran full-PR `git diff --check <base>`
(exit 0), checked unchanged contracts/checker and checked for build/cache/model
weight additions outside the pristine source (none). Initial development host
build/CTest and workflow local/live also exited 0. The original pristine import's
inherited-whitespace exit 2 and its narrowly scoped attributes policy are
explicitly explained in IMPORT; upstream bytes were never cleaned up.

## Falsification and review

In the independent disposable clone, all expected negative controls were rejected:

| Deliberate input | Verification/configuration result |
|---|---|
| Append bytes to vendor `README.md` | verifier exit 1: modified file |
| Temporarily remove vendor `LICENSE` | verifier exit 1: missing file |
| Add ignored `vendor/llama.cpp/build/independent-extra.bin` | verifier exit 1: extra inventory entry |
| Stage executable bit on vendor `LICENSE` | verifier exit 1: index mismatch |
| Configure separate directory with `-DLLAMA_USE_SYSTEM_GGML=ON` | CMake exit 1: outside host smoke profile |

Each source mutation was restored; the final verifier and clean status passed.
Generated VS project/linker/cache audit showed only vendor-built llama, ggml,
ggml-base, ggml-cpu and standard Windows SDK libraries. No generating-project
reference pointed back to the original implementation checkout. Backend loading,
CUDA, server/UI/common/tools/MTMD and source downloading were disabled.

Separate read-only provenance and build reviewers found no blocking issue in the
source inventory/licences, verification, CMake/link behavior, version identity or
handoff docs. The clean-checkout reviewer independently reproduced and tried to
falsify those claims. The lead retained all implementation writes.

## Tools, classification and limitations

Tested host: native Windows 11 AMD64 (OS build 26200), VS Build Tools 2022
17.14.40, MSVC 19.44.35228.0 (toolset 14.44.35207), SDK 10.0.26100.0,
CMake/CTest 4.4.3, Git 2.55.0.windows.5, Python 3.14.7; publication CLI
GitHub CLI 2.100.0. No privileged host/tool/driver changes were made.

Evidence: **source-inspected, imported, host-built and host-tested**. No
CUDA 11/native `sm_37`, K80/GK210, acquired-model, inference, Q1/basis correctness,
performance or quality evidence. Linux/macOS/Windows ARM and byte-identical
binaries were not tested. The 19 upstream GGUF tokenizer fixtures have zero
tensors; no model weights were acquired or added.

The unmodified upstream build emitted CMake CMP0194 and MSVC warnings
MSB8027/C4297/C4244/C4834. MSB8027 names `src/llama.cpp` and `src/models/llama.cpp`;
the latter is included through the upstream unity build. These warnings are
retained in the transcript; the bounded compilation/link/initialization passed,
without establishing model execution. K04 owns future native build diagnostics.
Raw ggml Git metadata reports outer commit `cd24a03`; the separate root provenance
correctly reports the full upstream and implementation identities. Reconfigure
after source/commit changes. Ordinary clone history containing import A is required.

## Blockers, next action and tasks unblocked

No K03 technical blocker remains. Maintainer review/acceptance of the focused PR
is the remaining gate; no self-merge or issue closure was performed. The next
verification command from an ordinary clean checkout is
`python scripts/verify_upstream.py`, followed by the exact host commands above.
To inspect future vendor patches, compare against import A and extend verification
with an accepted patch ledger while retaining the immutable baseline (IMPORT).

Once K03 is accepted, **K04, K05 and K06 may start independently** using this
snapshot/lock. K04 owes native CUDA 11/sm_37 build/artifact evidence; K05 the Q1
and approximation oracle; K06 exact acquired model identity/metadata/template
artifacts. K08 additionally needs K05. Hardware/model gates remain open and no
K04/K05/K06 implementation was started in this run.

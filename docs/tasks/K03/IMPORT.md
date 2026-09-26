# K03 pinned runtime import

The complete selected source is tracked at `vendor/llama.cpp/`. Validation and
acceptance state: [STATUS.md](STATUS.md). This is a host workspace; no CUDA
compatibility changes, model acquisition, inference or approximation implementation.

## Identity and accepted prerequisite

- Upstream: `https://github.com/ggml-org/llama.cpp`.
- Commit: `56381e407c0ccfb3a6f71e668a27a901001d22ce`; observed branch context `master`.
- Git tree: `24d31963e64a61fc390ecbdd12f7245a9feb73f9`.
- K80NSAI main/base at claim: `58a100dba15f9bca08c80a892224a342c8819f75`.
- Pristine import commit: `25ef5ba867cdc6bc0089d9cb05e7d89090f71560`.
- Canonical lock: [vendor/llama.cpp.lock.json](../../../vendor/llama.cpp.lock.json).

[PR #24](https://github.com/techrote/k80nsai/pull/24) merged at this base.
[Maintainer K01 signoff](https://github.com/techrote/k80nsai/issues/2#issuecomment-5845335994)
explicitly accepted the source choice and unblocked K03. Issue #2 was closed,
issue #3 had no claim and no PRs were open when checked. The
[K03 claim](https://github.com/techrote/k80nsai/issues/3#issuecomment-5845361945)
records ownership. Predecessors: K01 [source map](../K01/SOURCE_MAP.md),
[handoff](../K01/IMPLEMENTATION_HANDOFF.md), [work order](../K01/WORK_ORDERS.md#k03--issue-3)
and [import investigation](../K01/research/candidate-comparison-and-import.md)
as accepted in the base. Reviewed research revision:
`181cdcc7343a0bdb316c2c99621007c7f5f7d009`; final publication ledger:
`1481eb016c93e633ad173d68adb4cb563d53cdf1`.

## Import method, contents and licences

The lead fetched the exact SHA into a bare acquisition repository outside the
checkout, transferred Git objects and imported the full tree with `read-tree`.
No archive was used for import; no archive checksum is claimed as K03 evidence.
These commands executed with exit 0. Paths are public workspace-relative notation;
working directory is the K03 root unless `-C` supplies another directory:

```text
git init --bare ../source-cache/k03-upstream.git
git -C ../source-cache/k03-upstream.git fetch --depth=1 https://github.com/ggml-org/llama.cpp.git 56381e407c0ccfb3a6f71e668a27a901001d22ce
git -C ../source-cache/k03-upstream.git rev-parse "FETCH_HEAD^{commit}" "FETCH_HEAD^{tree}"
git fetch ../source-cache/k03-upstream.git 56381e407c0ccfb3a6f71e668a27a901001d22ce
git -c core.autocrlf=false read-tree --prefix=vendor/llama.cpp/ -u "56381e407c0ccfb3a6f71e668a27a901001d22ce^{tree}"
git commit -m "vendor: import llama.cpp at 56381e407c0ccfb3a6f71e668a27a901001d22ce"
git rev-parse HEAD:vendor/llama.cpp
```

The local fetch warned that shallow roots could not update refs; objects were
transferred and subtree equality independently proved the result. The upstream
commit is not a parent of K80NSAI history. Import commit A contains only 3,583
vendor files: 3,458 regular and 125 executable, 171,774,604 bytes, no symlinks or
gitlinks. Its subtree exactly equals the upstream tree. Integration is separate.

**Exclusions: none.** Git administration is not part of an upstream source tree.
The empty upstream `.gitmodules` is retained; no submodule initialization or
untracked source tree is needed. Upstream CI stays under the vendor prefix,
without promotion to root `.github/workflows`.

Retained: root [LICENSE](../../../vendor/llama.cpp/LICENSE) (MIT), all of
[licenses/](../../../vendor/llama.cpp/licenses/), `gguf-py/LICENSE`, third-party
notices under `vendor/`, UI dependency `LICENSE`/`LICENCE.md` files, and embedded
source notices. The lock lists all standalone licence files. The root LICENSE
SHA-256 was independently calculated as
`94f29bbed6a22c35b992c5c6ebf0e7c92f13b836b90f36f461c9cf2f0f1d010d`.
The 19 upstream `models/ggml-vocab-*.gguf` tokenizer fixtures have zero tensors
(checked only at header level). They are retained source test data, not model
weights or K06 acquired-model metadata evidence. No models were downloaded.

## Source integrity

From the checkout root:

```text
python scripts/verify_upstream.py
```

This offline check compares HEAD's vendor subtree and the import commit's subtree
with the lock, compares index path/blob/mode records, hashes every actual working
file in Git blob format, checks the inventory including ignored extra files,
and verifies notices/counts. On Windows executable modes are verified through
Git; on POSIX filesystem executable bits are checked too. Missing, added, altered
or staged vendor files fail nonzero. Use an ordinary clone containing the import
commit; preserve the separate import commit when merging the PR. A depth-one
clone may need to fetch that commit before verification.

To independently resolve the upstream commit/tree, use the exact-SHA acquisition
commands above, then run (also executed, exit 0):

```text
python scripts/verify_upstream.py --upstream-repo ../source-cache/k03-upstream.git
```

A read-only provenance reviewer independently compared every cached source blob
to GitHub's tree and reconstructed the Git tree hash, with no differences. Build
and offline verification do not access that cache or any upstream remote.

Root `.gitattributes` disables vendor text conversion, preserving bytes even with
Windows `core.autocrlf=true`, and marks the prefix vendored. It disables vendor
whitespace diagnostics: before this policy, pristine `git diff --cached --check`
returned 2 for inherited upstream whitespace (20,660 diagnostic/output lines).
Those bytes were preserved. Root integration retains ordinary whitespace checks;
exact tree/blob verification protects the snapshot. No upstream cleanup is hidden.

## Root build layout

Root [CMakeLists.txt](../../../CMakeLists.txt) verifies integrity before adding the
tracked source as a subproject. Requirements: Git, Python 3.10+, CMake 3.19+,
C++17 host compiler. The profile explicitly selects static bundled llama/ggml and
CPU. It rejects incompatible cached options and disables system ggml, dynamic
backends, non-CPU backends, optional x86 instruction extensions/native tuning,
optional CPU libraries/downloads, common/tools/tests/examples/server/app/UI and
standalone MTMD. There is no separate MTP CMake option at this pin; no speculative
or MTP execution occurs. All source remains available for downstream tasks.

The [host smoke](../../../tests/host-smoke.cpp) links llama and bundled ggml,
initializes/frees the libraries and CPU backend, and requires exactly one CPU
backend/device. CTest runs source integrity and host initialization. No model,
arithmetic oracle, external llama/ggml library or dependency source fetch is used.
Ordinary compiler/SDK libraries remain prerequisites.

Native Windows commands from the root (actual clean-run commands/exit codes are
in the evidence linked by STATUS):

```text
python scripts/verify_upstream.py
cmake -S . -B ../build-k03 -G "Visual Studio 17 2022" -A x64
cmake --build ../build-k03 --config Release --target k80nsai-host-smoke --parallel 2
ctest --test-dir ../build-k03 -C Release --output-on-failure
```

Keep products outside source or in ignored `build/`; use a fresh build directory
when switching profiles/toolchains. Other generators may be selected, but
Linux/macOS/Windows ARM builds were not tested. No OS migration or WSL GPU claim.

## Provenance and version-string trap

Reproduced: `git -C vendor/llama.cpp rev-parse HEAD` found the outer K80NSAI commit.
Initial generated `llama-version.h` and `ggml-version.h` both contained `25ef5ba`
(the K80NSAI import commit), not the runtime pin. ggml repeats its own Git probe
even if `LLAMA_BUILD_COMMIT` is set. Upstream source/probes are unchanged.
`llama_version()` reports semantic version `0.4.0-dev`; `ggml_commit()` demonstrates
the outer Git hash. Neither raw version string proves source provenance.

Root integration generates `k80nsai-provenance.json` and a header in the build
directory using the verified lock and the full K80NSAI commit/dirty state at
configure time. The smoke prints separately labelled upstream runtime/tree,
`k80nsai_configure_commit`, `k80nsai_configure_dirty`, and raw runtime metadata.
Reconfigure after switching commits or editing files; configure-time metadata
cannot identify subsequent edits. This establishes source/workspace reproduction,
not byte-identical binaries. No cosmetic build metadata override or source patch.

## Local modification strategy and remaining gates

Pristine import history remains immutable. K03 integration is entirely outside
`vendor/llama.cpp/`; `local_modifications` in the lock is empty. Audit later changes:

```text
git diff 25ef5ba867cdc6bc0089d9cb05e7d89090f71560 -- vendor/llama.cpp
```

The default verifier intentionally requires pristine source. Before accepting
vendor patches, their owner must coordinate with K03, retain original commit/tree,
record owned paths and patch commits/reasons, and extend verification to distinguish
the original baseline from reviewed modifications. Do not silently skip checking or
replace the upstream identity with a patched tree. K04 owns compatibility patches
and its native build lane; K08 owns shared runtime interfaces. Coordinate shared
root edits. Genuine import impossibility returns to K01/K03 with its exact
reproducer and coordinated repin procedure; this import needed no repin.

After K03 maintainer acceptance, K04/K05/K06 can work independently from this lock.
K04 still owes CUDA 11/native `sm_37` compilation and artifact inspection; K05 the
independent Q1/basis host oracle; K06 acquired model checksums, GGUF metadata and
templates. K08 also requires K05. K07/later retain real K80/model execution gates.
None is implemented or claimed here. Clean-checkout commands, exit codes, versions
and limitations are in [STATUS.md](STATUS.md) and its linked transcript.

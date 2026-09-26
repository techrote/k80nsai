# K04 CUDA 11 / native sm_37 build lane

**Implemented build preparation; compilation-pending.** No CUDA compiler or
backend artifact exists in this run. K04 acceptance remains open. See
[STATUS.md](STATUS.md) for evidence classification and revision accounting.

## Accepted source and ownership

Checked 2026-09-26 before edits: fetched `origin/main` was accepted K03 merge
`fcf54b8671035b3959e851d842bbf5c8e7ba2fe7`; issue #3 was closed **completed** and
[PR #25](https://github.com/techrote/k80nsai/pull/25) was merged. Issue #4 had no
comments, #8 had no comments, no PR was open, and the repository-wide issue
comment check found no competing build/runtime claim. The
[K04 claim](https://github.com/techrote/k80nsai/issues/4#issuecomment-5845631837)
bounds ownership to build integration, validation and this evidence. Branch:
`work/K04-sm37-compat`, separate worktree; lead performed all writes.

- Upstream: `https://github.com/ggml-org/llama.cpp`.
- Immutable commit: `56381e407c0ccfb3a6f71e668a27a901001d22ce`.
- Immutable tree: `24d31963e64a61fc390ecbdd12f7245a9feb73f9`.
- Pristine import: `25ef5ba867cdc6bc0089d9cb05e7d89090f71560`.
- Build infrastructure: `09f620d64d2403d84aba4c327e9afec3aaffa56b`.
- Predecessor artifacts: [K03 import](../K03/IMPORT.md), [K03 status](../K03/STATUS.md),
  [lock](../../../vendor/llama.cpp.lock.json), [K01 work order](../K01/WORK_ORDERS.md#k04--issue-4).

The pin, import history, licences, verifier, workflow manifest and K08 dispatch/API
are unchanged. No models or build binaries are added. K07/K09 were not started.

## Actual installed environment and missing prerequisite

[Ordinary command/version output and exits](evidence/environment.txt) is the
record, not the driver's advertised CUDA support. Native Windows 11 Pro
`10.0.26200`, AMD64; CMake/CTest `4.4.3`, Python `3.14.7`, Git
`2.55.0.windows.5`. CMake is `C:\Program Files\CMake\bin\cmake.exe` and Python is
`C:\Python314\python.exe`. Installed GPU: GTX 1650 SUPER, cc7.5, driver `616.92`;
`nvidia-smi.exe` is in `C:\Windows\System32`. No K80 is present; no GPU test ran.

| Tool | Actual path/version or absence |
|---|---|
| nvcc / cuobjdump | Neither found on PATH; no CUDA environment variables, standard toolkit directory, or CUDA registry key |
| CUDA root / toolkit version | **Unavailable**; `C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA` does not exist |
| Candidate CUDA host compiler | `C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Tools\MSVC\14.29.30133\bin\Hostx64\x64\cl.exe`, full version **19.29.30159.0** |
| Candidate Ninja | VS2019 `Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe`, **1.10.2** |
| MSBuild | VS2019 `MSBuild\Current\Bin\MSBuild.exe`, **16.11.6.22506** |
| K03 host regression compiler | VS2022 `VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\cl.exe`, **19.44.35228.0**; SDK **10.0.26100.0** |
| Other existing build tools | VS2022 MSBuild **17.14.51.32402**, Ninja **1.12.1** |

The compiler paths above start at the corresponding VS BuildTools installation.
`cl /Bv` returned 2 with D8003 (no source filename) after printing full versions;
that version query is not a source compilation failure. No installation, driver,
security, firmware, clock or power change was attempted. WSL was not used.

**Next prerequisite:** an operator-supplied complete CUDA **11.8** toolkit,
including nvcc, headers, cudart/cuBLAS libraries and matching cuobjdump. The
installed VS2019 compiler belongs to NVIDIA's supported MSVC 192x family for
CUDA 11.8; the exact combination remains uncompiled here. VS2022 19.44 is only
host-smoke evidence. [NVIDIA's archived Windows support table](https://docs.nvidia.com/cuda/archive/11.8.0/cuda-installation-guide-microsoft-windows/index.html#system-requirements).
No compiler/toolkit installation is implicitly authorized by these commands.

## Build lane and actual changes

Default `cmake -S .` still uses the original CPU-only K03 profile. Opt in with
`-DK80NSAI_CUDA_SM37=ON` in a **different build directory**. The root delegates
to [cmake/k04-cuda.cmake](../../../cmake/k04-cuda.cmake) before host configuration.
This first lane uses native 64-bit Ninja, explicit absolute C/C++/CUDA/host/Ninja
paths, CUDA 11.x, and exactly `37-real`. It rejects incompatible overrides and
requires nvcc to come from the specified toolkit. It does not choose a toolkit
from PATH or bypass nvcc's supported-host-compiler checks.

The lane sets the requested first baseline: CUDA ON; native CPU tuning, graphs,
FA, NCCL and CUB3.2 OFF; NO_VMM ON; tools/common ON; server/tests initially OFF.
Shared bundled llama/ggml/CPU/CUDA libraries are linked directly; system ggml and
dynamic backend discovery are OFF. Optional dependencies/downloads are disabled.
Tools ON configures upstream MTMD targets too; the bounded explicit build targets
do not build them. This is not a source-filtering or feature-removal patch.

The target checks require `ggml-cuda`, `llama-completion` and bundled libraries to
originate under the tracked vendor tree, and check the backend's architecture
property. Enabling `LLAMA_BUILD_TESTS` additionally requires `test-backend-ops`.
K03's offline verifier runs before adding vendor sources. `k04-build.txt` records
upstream versus outer implementation identities, tools and actual target paths;
`k04-cuda-sources.txt` lists the backend's selected sources. Raw upstream version
Git probes still report outer K80NSAI metadata, not the upstream source identity.
Reconfigure after a commit/source change to refresh this record.

`k04-inspect-native` depends on `ggml-cuda` and passes its **TARGET_FILE** to
[scripts/inspect_sm37.py](../../../scripts/inspect_sm37.py): the DLL/shared object,
not the Windows import library or completion executable. The script records
cuobjdump stdout/stderr, exact argument vectors/cwd/exits, backend SHA-256 and
function names; it requires an sm_37 ELF listing and instruction-bearing sm_37
SASS bodies. PTX is supplemental. The evidence directory must not already exist,
so a failed rebuild cannot overwrite an earlier record. The artifact is hashed
before and after inspection. Synthetic parser tests are separate from real
native-code evidence; actual CUDA11 tool-output integration remains untested.

## Executed checks and first failure

Commands below ran at the K04 checkout root. Logs substitute `<CHECKOUT>` and
`<HOST_BUILD>` for workspace paths; command arguments and exits are retained.

| Check | Result |
|---|---|
| `python scripts/verify_upstream.py` | 0; full immutable tree, import, index and working bytes pass |
| Fresh `cmake -S . -B C:/K80nsai/build-k04-missing-toolkit -G Ninja -DK80NSAI_CUDA_SM37=ON` | **1**, missing CUDA root, before any compiler invocation |
| Fresh PTX-only / system ggml / CUDA OFF configurations | All expected exit 1 |
| `python tests/test_inspect_sm37.py` | 0; 7 synthetic parser/collector rejection tests |
| K03 VS2022 configure/build/direct smoke | All 0 |
| K03 CTest | 0; **2/2 passed** |
| `python scripts/check_workflow.py` and `--live` | Both 0 |
| Vendor diff from pristine import | 0; empty |

Actual first K04 output:

```text
CMake Error at cmake/k04-cuda.cmake:48 (message):
  K04 requires an existing absolute CUDAToolkit_ROOT for CUDA 11.x; no
  toolkit is installed by this build
-- Configuring incomplete, errors occurred!
```

Classification: missing external prerequisite, **not** toolkit/header, host
compiler, architecture, intrinsic, template or library compilation diagnostics.
No CUDA translation unit was attempted, so no CUDA compatibility failure or
successful backend compilation can be reported. Review found and corrected a
CMake regex escape before commit; it is not a vendor/compiler finding.

[Preflight/tests](evidence/preflight.txt), [host regression](evidence/host-regression.txt),
[workflow](evidence/workflow.txt). Host warnings inherited from K03 remain visible
(CMP0194, MSB8027, C4297, C4244, C4834); no vendor changes suppress them.

## Patch ledger and integrity

**Accepted local vendor patches: zero.** `local_modifications` in the unchanged
lock is empty. The default verifier still requires the exact upstream tree, so
there is no weakened bypass or unrecorded vendor change.

| Path | Reason / motivating diagnostic | Scope | Patch commit | Validation |
|---|---|---|---|---|
| None | No CUDA compiler diagnostic exists to justify a vendor patch | None | None | Pristine tree verifier passes |

This empty ledger is reproducible directly from the preserved import. Audit:

```text
python scripts/verify_upstream.py
git diff --exit-code 25ef5ba867cdc6bc0089d9cb05e7d89090f71560 -- vendor/llama.cpp
```

Before accepting any future vendor modification, extend provenance to verify the
original immutable import plus deterministic ordered patches, exact changed
paths, patch commits, reasons/diagnostics, architecture/toolkit scope and validation.
Reject any unrecorded diff. Do not edit the locked upstream identity to describe
a patched tree or disable the current verifier to get a build through. Coordinate
shared dispatch edits with K08's live owner.

## Exact CUDA continuation commands (not executed)

Use native Windows `cmd.exe` from a clean checkout root after the missing toolkit
is supplied. The CUDA path below is the expected operator-supplied 11.8 path,
**not an observed installation**. Preserve stdout/stderr and `%ERRORLEVEL%`
immediately after each command; stop on nonzero. Use empty build/evidence paths.

```bat
set "K04_VS=C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools"
set "K04_CUDA=C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8"
set "K04_CL=%K04_VS%\VC\Tools\MSVC\14.29.30133\bin\Hostx64\x64\cl.exe"
set "K04_NINJA=%K04_VS%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"
set "K04_CMAKE=C:\Program Files\CMake\bin\cmake.exe"
call "%K04_VS%\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.29
"%K04_CUDA%\bin\nvcc.exe" --version
"%K04_CL%" /Bv
"%K04_CUDA%\bin\cuobjdump.exe" --version
python scripts\verify_upstream.py
"%K04_CMAKE%" -S . -B build/k04-sm37 -G Ninja ^
  -DK80NSAI_CUDA_SM37=ON -DCMAKE_BUILD_TYPE=Release ^
  "-DCMAKE_MAKE_PROGRAM=%K04_NINJA%" ^
  "-DCUDAToolkit_ROOT=%K04_CUDA%" ^
  "-DCMAKE_CUDA_COMPILER=%K04_CUDA%\bin\nvcc.exe" ^
  "-DCMAKE_CUDA_HOST_COMPILER=%K04_CL%" ^
  "-DCMAKE_C_COMPILER=%K04_CL%" "-DCMAKE_CXX_COMPILER=%K04_CL%" ^
  -DCMAKE_CUDA_ARCHITECTURES=37-real -DLLAMA_BUILD_TESTS=OFF
"%K04_CMAKE%" --build build/k04-sm37 --target ggml-cuda llama-completion --parallel 2 --verbose
"%K04_CMAKE%" --build build/k04-sm37 --target k04-inspect-native --verbose
"%K04_CMAKE%" -S . -B build/k04-sm37 -DLLAMA_BUILD_TESTS=ON
"%K04_CMAKE%" --build build/k04-sm37 --target test-backend-ops --parallel 2 --verbose
```

The `/Bv` version-only exit 2 is expected; all actual configure/build/inspection
commands require exit 0. Do not build the default `all` target or run the upstream
CTest suite for this bounded build check. Building operator tests does not run them.
After any narrow fix, first rebuild the failing target with `--verbose`, then
repeat the full backend/completion/operator build. For a new inspection, retain
the old evidence and select a new directory before invoking the target:

```bat
"%K04_CMAKE%" -S . -B build/k04-sm37 "-DK80NSAI_SM37_EVIDENCE_DIR=%CD%/build/k04-sm37/sm37-evidence-rebuild-1"
"%K04_CMAKE%" --build build/k04-sm37 --target k04-inspect-native --verbose
```

Prefer an absolute evidence path when overriding it. Audit `CMakeCache.txt`,
`compile_commands.json`, the verbose log and `k04-cuda-sources.txt`: explicit
CUDA11 nvcc/host compiler, `compute_37`/`sm_37` native generation, all actual
translation units and only this build's bundled ggml/llama dependencies. Save
CMake's configure log at the **first** real failure before changing any flags.
Classify its file/symbol/template, make one narrow justified fix, record it in
the patch ledger, rebuild that stage and repeat the full build and K03 regression.

The collector executes the following equivalent direct commands against the
backend path printed in `k04-build.txt`:

```text
<CUDA11>/bin/cuobjdump --list-elf <actual-ggml-cuda-backend>
<CUDA11>/bin/cuobjdump --gpu-architecture sm_37 --dump-sass <actual-ggml-cuda-backend>
<CUDA11>/bin/cuobjdump --list-ptx <actual-ggml-cuda-backend>
```

[CUDA11.8 utility format/options](https://docs.nvidia.com/cuda/archive/11.8.0/cuda-binary-utilities/index.html#cuobjdump).
**Current artifact path/hash, native output and operator-test build result: none / pending.**
Once produced, retain `native-artifact.json`, text outputs and a concise native
Q1/conversion kernel excerpt; do not commit binaries or megabytes of SASS. Match
candidate symbols back to `mmvq.cu` and `convert.cu` (inlined helper names may not
survive). An arbitrary unrelated sm_37 function is not the full backend proof.

## Source findings and unresolved runtime risks

These are inspected conditions, not compilation results or new source selection:

- CUDA CMake already accepts explicit 37-real. Its root globs and tracked generated
  template instances select 143 CUDA TUs at the default FA type combinations.
  FA OFF defines `GGML_CUDA_NO_FA`; it does not remove those TUs. Graphs OFF omits
  its definition; NO_VMM ON selects the existing allocation route; NCCL/CUB3.2 OFF
  avoid those optional dependencies. No template generation was run.
- `ggml-cuda/ggml-cuda.cu:1622-1663` selects F32 for automatic quantized/F16 on
  cc370, but automatic BF16 and forced F16/BF16 remain risks. `prefer_f32_output`
  leaves BF16 inputs intact. Preserve storage/conversion and modern-device behavior;
  unsupported forced modes must fail visibly when this is repaired and tested.
- `common.cuh:714-752` supplies scalar DP4A emulation below cc610. Existing guards
  for FP16/MMA/async paths and BF16/FP16 headers still require real TU compilation.
- `ggml-cuda.cu:1543-1612` uses typed SGEMM for single F32 matrices, but Ex calls
  remain for strided and pointer/broadcast batches even in F32. NVIDIA's documented
  cc constraints are a separate K80 library execution risk. Forcing F32 alone
  does not solve it. A later demonstrated fallback must preserve conversion,
  dimension-three strides, `i02=i12/r2`, `i03=i13/r3`, transpose/leading dimensions,
  destination stride and stream. No uncompiled fallback was added.
- Runtime graphs already disable below Volta; this lane leaves graphs OFF.
  The unchanged fused GDN/27B routes, BF16 conversions and batched library calls
  have no K80 evidence. Small-model success would not prove 27B compatibility.

Exact semantic analysis/primary links remain in the accepted
[K01 CUDA note](../K01/research/cuda11-sm37-compatibility.md),
[activation note](../K01/research/q1-abi-and-activation-path.md) and
[operator note](../K01/research/small-and-27b-operator-paths.md). No inference,
quality, throughput, model loading or cuBLAS runtime compatibility is claimed.

## Host regression and downstream consumption

From a clean repository root, the original K03 command sequence remains:

```text
python scripts/verify_upstream.py
cmake -S . -B ../build-k04-host -G "Visual Studio 17 2022" -A x64
cmake --build ../build-k04-host --config Release --target k80nsai-host-smoke --parallel 2
ctest --test-dir ../build-k04-host -C Release --output-on-failure
python scripts/check_workflow.py
python scripts/check_workflow.py --live
git diff --check
git status --porcelain
```

K07/K09 consume the same checkout/lock and successful native build directory only
**after K04 acceptance**. At that point use `k04-build.txt` for actual binary paths,
`native-artifact.json` for the backend hash and inspection, plus the complete build
logs/cache and accepted patch ledger. Re-run the above configure/build commands
in a fresh directory when integrating downstream sources; never mix a system or
another pin's backend. Keep dependent DLLs with that build's executables.

On the eventual authorized K80 host, the owning gate resolves its backend name
and device first. Candidate K07 commands are `test-backend-ops test -o
MUL_MAT,CPY -b <selected-K80-backend>` for conversion/Q1/batched routes and
`test-backend-ops test -o GATED_DELTA_NET,SSM_CONV -b <selected-K80-backend>` for
relevant hybrid operators, using actual K06 shapes and checking executed/skipped
counts. These commands were not run; K07 owns model/device setup and execution.
K09 also needs K05/K08's accepted oracle/interface. Neither downstream gate is
unblocked by this preparation PR. No model command can honestly be frozen before
those predecessor identities exist.

# CUDA 11 and native sm_37 compatibility

Research date: 2026-09-26. Read-only source investigation; no compiler invocation, artifact inspection, K80 execution, model run or implementation was performed. Consumers: K03/K04/K07. Runtime decision: [SOURCE_MAP.md](../SOURCE_MAP.md).

## Identities and evidence classes

Inspected immutable snapshots supplied by the lead:

- Mainline: `ggml-org/llama.cpp@56381e407c0ccfb3a6f71e668a27a901001d22ce`.
- Prism: `PrismML-Eng/llama.cpp@d8f26eec76da6d09bb708bcba51ef64b8cd868a3`.
- Kepler reference: `babal35/llamacpp-kepler@b367989573b1a97e37b001cbf6718365844b94ca`.
- Kepler documented upstream base: `ggml-org/llama.cpp@a95a11e5b834057e684712963f90bbb730f4745c`.

`S`: directly inspected source; `P`: NVIDIA/CMake primary documentation; `I`: inference/proposed implementation consequence; `T`: third-party execution claim; `U`: unresolved by compilation/execution. All findings below are S/P/I/T/U, never `sm37-compiled` or `k80-tested`.

Repository baseline: `8deeac521f7a76e92b54bdf2c95d31377b5a4f65`. Product/build scope remains in the shared RAG contracts.

## Exact historical diff, and why it is insufficient

An exact directory comparison with `git -c core.autocrlf=false diff --no-index --stat` finds four changed paths: new `KEPLER.md`, rewritten `README.md`, and only two code/build files. Diff exit 1 means differences, not a build failure.

1. `ggml/src/ggml-cuda/CMakeLists.txt`: five added lines inside the default, non-native architecture branch append `35-virtual 37-virtual` when toolkit version is less than 12. [Kepler lines 27–40](https://github.com/babal35/llamacpp-kepler/blob/b367989573b1a97e37b001cbf6718365844b94ca/ggml/src/ggml-cuda/CMakeLists.txt#L27-L40).
2. `ggml/src/ggml-cuda/ggml-cuda.cu`: replaces the NVIDIA portion of `supports_bf16` with `GGML_CUDA_CC_IS_NVIDIA(cc) && cc >= GGML_CUDA_CC_AMPERE`; AMD/MTHREADS portions remain. [Kepler lines 1492–1504](https://github.com/babal35/llamacpp-kepler/blob/b367989573b1a97e37b001cbf6718365844b94ca/ggml/src/ggml-cuda/ggml-cuda.cu#L1492-L1504). Base comparison: [upstream lines 1492–1504](https://github.com/ggml-org/llama.cpp/blob/a95a11e5b834057e684712963f90bbb730f4745c/ggml/src/ggml-cuda/ggml-cuda.cu#L1492-L1504).

The fork reports CUDA 11.4, Linux, driver 470.256.02, and execution of different models across both GPUs of one K80 board. Its prose claims and throughput are T, not our evidence; the dual-device model experiments do not validate this project's single-device Bonsai graph. [Pinned KEPLER.md](https://github.com/babal35/llamacpp-kepler/blob/b367989573b1a97e37b001cbf6718365844b94ca/KEPLER.md).

## Architecture path: explicit settings already exist in both candidates

Both candidate CMake files enclose automatic defaults in `if (NOT DEFINED CMAKE_CUDA_ARCHITECTURES)` at line 8. Their normal non-native defaults begin at 50, but explicit `-DCMAKE_CUDA_ARCHITECTURES=37-real` bypasses that list. Their later architecture normalization only rewrites 12X, so it leaves 37 intact. `GGML_NATIVE=OFF` is sensible for a reproducible portable build, but explicit architecture selection is the relevant override; the fork's default-list patch is unnecessary for this bounded build. S/I.

- [Mainline architecture handling, lines 1–100](https://github.com/ggml-org/llama.cpp/blob/56381e407c0ccfb3a6f71e668a27a901001d22ce/ggml/src/ggml-cuda/CMakeLists.txt#L1-L100).
- [Prism architecture handling, lines 1–100](https://github.com/PrismML-Eng/llama.cpp/blob/d8f26eec76da6d09bb708bcba51ef64b8cd868a3/ggml/src/ggml-cuda/CMakeLists.txt#L1-L100).

CMake 3.18 introduced the architecture property. `37` requests both real code and PTX; `37-real` requests native device code only; `37-virtual` requests PTX only. `native` was introduced in 3.24 and targets GPUs on the build host. Therefore neither a configure message nor `37-virtual` meets the native-sm_37 acceptance requirement. Prefer `37-real` for the first build, particularly if CUDA 11.8 output must run on an older R470 driver. [CMake primary documentation](https://cmake.org/cmake/help/v3.30/prop_tgt/CUDA_ARCHITECTURES.html). P/I.

## Compile surface is larger than runtime dispatch

| Layer | Mainline and Prism evidence | Consequence |
|---|---|---|
| Toolkit headers | Both `vendors/cuda.h` lines 3–7 include runtime, driver, cuBLAS, BF16 and FP16 headers unconditionally. FP8 header is guarded by `CUDART_VERSION >= 11080`; FP4 by `>= 12080`; pre-11.2 aliases are separate. | BF16 runtime avoidance does not remove BF16 types/header requirements. Do not remove BF16/FP16 headers globally; preserve conversions and storage. Probe actual toolkit headers first. |
| Broad translation units | Both CMake files glob every root `*.cu`, and template instances for fattn-tile, fattn-mma, mmq and mmf (lines 102–113). | A model that never runs an operation can still fail compilation of that operation's TU. |
| FA instance selection | Mainline calls `ggml_cuda_fattn_vec_instances`; Prism either globs all vec instances or lists f16, q4_0, q8_0 and bf16 instances explicitly. | Mainline's configurable instance helper is a build difference, not established Kepler support. |
| FA disable | `GGML_CUDA_FA=OFF` adds `GGML_CUDA_NO_FA`; it does not remove these source files from CMake. `FLASH_ATTN_AVAILABLE` is controlled in common.cuh and removes selected bodies. | Includes, function declarations, host dispatch, and code outside those body guards still compile. |
| Architecture guards | `FP16_AVAILABLE` requires NVIDIA CC>=600; `TURING_MMA_AVAILABLE` >=750; `AMPERE_MMA_AVAILABLE` and `CP_ASYNC_AVAILABLE` >=800. | These are useful existing barriers, not a whole-tree proof. Half storage/conversion still differs from native half arithmetic. |
| Toolkit-sensitive support files | `softmax.cu` includes cooperative_groups and reduce headers; mean/sum/ssm-scan include CUB. `lightning-indexer.cu` gates `<mma.h>` behind TURING_MMA_AVAILABLE. | Verify the exact 11.x installation rather than asserting all versions have identical headers and APIs. |
| Prism extra path | `mmq-hopper-q1.cu` is still globbed, but `<cuda_pipeline.h>`/CuTe and main implementation are behind `GGML_USE_HOPPER_Q1`; CMake option defaults OFF. | Keep the option OFF on Kepler; no CUTLASS dependency is needed merely because the file exists. |

Pinned source anchors:

- [Mainline vendor headers](https://github.com/ggml-org/llama.cpp/blob/56381e407c0ccfb3a6f71e668a27a901001d22ce/ggml/src/ggml-cuda/vendors/cuda.h#L1-L28), [Prism vendor headers](https://github.com/PrismML-Eng/llama.cpp/blob/d8f26eec76da6d09bb708bcba51ef64b8cd868a3/ggml/src/ggml-cuda/vendors/cuda.h#L1-L28).
- [Mainline globs/options](https://github.com/ggml-org/llama.cpp/blob/56381e407c0ccfb3a6f71e668a27a901001d22ce/ggml/src/ggml-cuda/CMakeLists.txt#L102-L141), [Prism globs/options](https://github.com/PrismML-Eng/llama.cpp/blob/d8f26eec76da6d09bb708bcba51ef64b8cd868a3/ggml/src/ggml-cuda/CMakeLists.txt#L102-L164).
- [Mainline instance helper](https://github.com/ggml-org/llama.cpp/blob/56381e407c0ccfb3a6f71e668a27a901001d22ce/ggml/cmake/common.cmake#L52-L115), [BF16 FA instance](https://github.com/ggml-org/llama.cpp/blob/56381e407c0ccfb3a6f71e668a27a901001d22ce/ggml/src/ggml-cuda/template-instances/fattn-vec-instance-bf16-bf16.cu#L1-L7), [FA body guard](https://github.com/ggml-org/llama.cpp/blob/56381e407c0ccfb3a6f71e668a27a901001d22ce/ggml/src/ggml-cuda/fattn-vec.cuh#L19-L45).
- [Mainline capabilities](https://github.com/ggml-org/llama.cpp/blob/56381e407c0ccfb3a6f71e668a27a901001d22ce/ggml/src/ggml-cuda/common.cuh#L263-L335), [Prism capabilities](https://github.com/PrismML-Eng/llama.cpp/blob/d8f26eec76da6d09bb708bcba51ef64b8cd868a3/ggml/src/ggml-cuda/common.cuh#L253-L325).
- [Softmax includes](https://github.com/ggml-org/llama.cpp/blob/56381e407c0ccfb3a6f71e668a27a901001d22ce/ggml/src/ggml-cuda/softmax.cu#L1-L10), [lightning include guard](https://github.com/ggml-org/llama.cpp/blob/56381e407c0ccfb3a6f71e668a27a901001d22ce/ggml/src/ggml-cuda/lightning-indexer.cu#L1-L16), [Prism Hopper guard](https://github.com/PrismML-Eng/llama.cpp/blob/d8f26eec76da6d09bb708bcba51ef64b8cd868a3/ggml/src/ggml-cuda/mmq-hopper-q1.cu#L1-L19).

No compilation was performed, so none of these headers/TUs is recorded as failing or passing on CUDA 11.4/11.8. A narrow failed-header or failed-instantiation fix must be justified by a real diagnostic; wholesale deletion of modern code is not justified by this inspection.

## Semantic BF16 port and a separate batched-cuBLAS risk

Both candidate pins use `ggml_cuda_mul_mat_cublas` and templated `ggml_cuda_mul_mat_cublas_impl<compute_type>`. The old `supports_bf16` predicate is absent at the relevant selection point.

- Quantized weights select F16 only when `fast_fp16_hardware_available` is true, otherwise F32; native F16 weights similarly fall back to F32. On GK210, this helper is false. Native BF16 weights are not covered by that fallback. `GGML_PREC_F32` can override type, but then `GGML_CUDA_CUBLAS_COMPUTE_TYPE` can override it again to F16 or BF16. All three template specializations remain referenced. S.
- The BF16 specialization uses BF16 A/B types with FP32 computation. `prefer_f32_output` changes the output type and scalars; it does not convert BF16 A/B to F32. Thus changing only the output precision cannot repair unsupported BF16 input execution. S/I.
- A candidate K04 fix should validate the final compute type after environment selection, choose a supported F32 conversion path for automatic BF16 on Kepler, and explicitly reject or visibly handle unsupported forced F16/BF16 settings. Reuse the existing capability helpers where semantically correct and preserve AMD/MUSA behavior. This is a proposed work order, not implemented code. I.

[Mainline traits/implementation, 1362–1619](https://github.com/ggml-org/llama.cpp/blob/56381e407c0ccfb3a6f71e668a27a901001d22ce/ggml/src/ggml-cuda/ggml-cuda.cu#L1362-L1619), [Mainline selection, 1622–1663](https://github.com/ggml-org/llama.cpp/blob/56381e407c0ccfb3a6f71e668a27a901001d22ce/ggml/src/ggml-cuda/ggml-cuda.cu#L1622-L1663). Prism equivalent is exactly one line later in these sections: [traits/implementation, 1363–1620](https://github.com/PrismML-Eng/llama.cpp/blob/d8f26eec76da6d09bb708bcba51ef64b8cd868a3/ggml/src/ggml-cuda/ggml-cuda.cu#L1363-L1620), [selection, 1623–1664](https://github.com/PrismML-Eng/llama.cpp/blob/d8f26eec76da6d09bb708bcba51ef64b8cd868a3/ggml/src/ggml-cuda/ggml-cuda.cu#L1623-L1664).

**Separate blocker/risk:** the NVIDIA 11.8 cuBLAS reference requires CC>=5.0 for `cublasGemmEx` and `cublasGemmBatchedEx`. Both candidates choose typed `cublasSgemm` only for F32 and `ne12 == ne13 == 1`; batched/broadcast paths still call `cublasGemmStridedBatchedEx` or `cublasGemmBatchedEx`. Forcing F32 alone does not route those calls to typed SGEMM. The StridedBatchedEx documentation's error table repeats the BatchedEx name, so preserve that wording rather than claiming a separately verified runtime failure. The bounded probe must exercise both batch layouts; a potential repair uses typed `cublasSgemmStridedBatched`/`cublasSgemmBatched` or an explicit same-stream SGEMM loop with preserved strides and broadcasts. Exact graph reachability and execution remain U. [CUDA 11.8 cuBLAS §§2.8.12–14](https://docs.nvidia.com/cuda/archive/11.8.0/cublas/index.html#cublas-GemmEx), [BatchedEx](https://docs.nvidia.com/cuda/archive/11.8.0/cublas/index.html#cublas-GemmBatchedEx), [StridedBatchedEx](https://docs.nvidia.com/cuda/archive/11.8.0/cublas/index.html#cublas-GemmStridedBatchedEx).

Graphs: the mainline candidate already disables graph use below Volta at `ggml_cuda_graph_set_enabled`; this is stronger evidence about this runtime than the fork's assertion that CUDA Graphs universally require sm60. Use `GGML_CUDA_GRAPHS=OFF` initially to reduce the tested surface, without turning the fork's prose into a hardware rule. [Mainline lines 4395–4410](https://github.com/ggml-org/llama.cpp/blob/56381e407c0ccfb3a6f71e668a27a901001d22ce/ggml/src/ggml-cuda/ggml-cuda.cu#L4395-L4410).

## Toolchain/host/driver options from primary sources

These are later probe options, not certification of an available host. No installation or driver change is authorized here.

| Option | Primary constraint and use | Remaining check |
|---|---|---|
| CUDA 11.4 Update 4, native Linux x86_64, e.g. Ubuntu 20.04.3 + GCC 9.3.0 + glibc 2.31 | Exact archived NVIDIA table lists that distribution/toolchain; it also lists GCC 11 support. The fork's blanket “CUDA11 forbids GCC11” is inaccurate. | Installed version, OS/kernel/driver product compatibility, target compiler diagnostics. |
| CUDA 11.8, native Linux x86_64, e.g. Ubuntu 22.04 + GCC 11.2.0 + glibc 2.35, or listed Ubuntu 20.04 variants + GCC 9.3 | Newer 11.x headers can avoid some older-header issues. CUDA12 removes Kepler compiler/library support. | R470 host support for the exact Linux kernel; minor-compatibility feature/PTX limitations; actual binary execution. |
| CUDA 11.4 Update 4, native Windows x64 | NVIDIA table lists Windows 10/Server 2016,2019,2022 and MSVC 192x VS2019/191x VS2017. | Actual K80-supporting Windows driver package/product list and host OS. |
| CUDA 11.8, native Windows x64 | NVIDIA table lists Windows10/11 and Server2016/2019/2022; MSVC193x VS2022 17.0, 192x VS2019, 191x VS2017. A currently installed newer compiler must not be assumed compatible. | Pin actual compiler build and K80 driver support independently. Native Windows is a possible build route, not validated delivery. |
| CUDA12+, arbitrary current NVIDIA driver, WSL GPU path | CUDA12 explicitly removes Kepler; NVIDIA identifies R470 as the last Kepler branch and removal from R495. WSL documentation targets Pascal-or-later and excludes Tesla/TCC paths described there. | Not a supported K80 validation route for this assignment. A working TITAN/V100 installation does not settle any K80 condition. |

Primary URLs:

- [CUDA11.4.4 Linux supported distribution/compiler table](https://docs.nvidia.com/cuda/archive/11.4.4/cuda-installation-guide-linux/index.html#system-requirements).
- [CUDA11.8 Linux supported distribution/compiler table](https://docs.nvidia.com/cuda/archive/11.8.0/cuda-installation-guide-linux/index.html#system-requirements).
- [CUDA11.4.4 Windows table](https://docs.nvidia.com/cuda/archive/11.4.4/cuda-installation-guide-microsoft-windows/index.html#system-requirements).
- [CUDA11.8 Windows table](https://docs.nvidia.com/cuda/archive/11.8.0/cuda-installation-guide-microsoft-windows/index.html#system-requirements).
- [NVIDIA Kepler R470/R495 statement](https://developer.nvidia.com/blog/revealing-new-features-in-the-cuda-11-5-toolkit/).
- [CUDA12.0 removal](https://docs.nvidia.com/cuda/archive/12.0.0/cuda-toolkit-release-notes/index.html#deprecated-features).
- [CUDA on WSL constraints](https://docs.nvidia.com/cuda/wsl-user-guide/index.html#constraints).

Driver distinctions: NVIDIA's 11.8 release notes list CUDA11.x minor-compatibility floors of Linux 450.80.02 / Windows 452.39, separately from the CUDA11.8 bundled development-driver versions 520.61.05 / 522.06 and 11.4 Update4 versions 470.82.01 / 472.50. A numerical floor does not imply that every later driver supports K80. Do not install the 11.8 bundled driver on that assumption. [Release notes Tables2–3](https://docs.nvidia.com/cuda/archive/11.8.0/cuda-toolkit-release-notes/index.html#cuda-major-component-versions). P.

Minor compatibility has feature limitations and does not make new PTX run on older drivers; explicitly target native architecture and inspect the artifact. A container supplies userspace, not missing host device/driver support. [Minor-version compatibility caveats](https://docs.nvidia.com/deploy/cuda-compatibility/minor-version-compatibility.html#application-considerations-for-minor-version-compatibility). P/I.

## Blocker/options matrix and bounded K04 sequence

| Gate | Present evidence | Bounded next action / outcome |
|---|---|---|
| Appropriate toolkit/compiler exists | U; no environment claim here | Record versions/paths once; select one supported combination already available or operator-supplied. Missing tools are an external blocker; no environment-doctor app. |
| Header compatibility | Includes and guards inspected, no compiler run | Compile one temporary TU including candidate common.cuh and FP16/BF16 conversions using exactly the selected toolkit and host compiler; preserve first diagnostic. |
| Full TU/template compilation | Both candidates glob broad source sets | Configure fresh build with explicit 37-real and conservative feature options; compile actual ggml-cuda target, retain verbose command and first diagnostic. No “runtime disabled” substitute for compiling. |
| BF16/forced compute selection | Concrete source gap in both pins | Port semantics into final compute selection; test auto and forced selections without bypassing unsupported types silently. Keep source and run evidence separate. |
| Batched/broadcast cuBLAS | CC>=5 documentation conflicts with relevant Ex calls | Test F32 single, strided-batch and pointer/broadcast cases on K80; implement narrowly scoped typed F32 alternative if needed. Not settled by compilation. |
| Native artifact | No artifact exists | Inspect final backend DLL/SO, not only CLI executable or CMake cache; require sm_37 cubin/SASS for relevant kernels. |
| K80 library/kernel execution | No authorized K80 available in this run | Later single-device backend operation tests including conversions, Q1 matrix-vector, softmax, batched math and recurrent kernels that the inspected graph actually needs. |
| Bonsai correctness/fit | Outside this investigator's measurements | Separate small and 27B runs, model IDs/checksums and actual graph coverage; preserve failure as evidence. |

Later compile plan (not run):

1. Record `cmake --version`, `nvcc --version`, selected compiler version (`cl` or `g++ --version`), exact CUDA root, sanitized OS/kernel, and driver/device information supplied by the actual K80 host. `nvidia-smi`'s CUDA field is not nvcc/toolkit identity. Choose one toolkit first; a second 11.x version is justified only by a specific incompatible header/API diagnosis.
2. Create an out-of-tree build against K03's imported, lead-selected immutable source. Build header probe and then real backend. A possible baseline CMake invocation is:

```text
cmake -S vendor/llama.cpp -B build/k04-sm37 -G Ninja -DCMAKE_BUILD_TYPE=Release -DGGML_CUDA=ON -DCMAKE_CUDA_ARCHITECTURES=37-real -DGGML_NATIVE=OFF -DGGML_CUDA_GRAPHS=OFF -DGGML_CUDA_FA=OFF -DGGML_CUDA_NO_VMM=ON -DGGML_CUDA_NCCL=OFF -DGGML_CUDA_CUB_3DOT2=OFF -DLLAMA_BUILD_TESTS=OFF -DLLAMA_BUILD_SERVER=OFF -DLLAMA_BUILD_TOOLS=ON
cmake --build build/k04-sm37 --target ggml-cuda llama-completion --parallel 2 --verbose
```

The selected pin places `llama-cli` inside `if (LLAMA_BUILD_SERVER)` ([tools/CMakeLists.txt:20–28](https://github.com/ggml-org/llama.cpp/blob/56381e407c0ccfb3a6f71e668a27a901001d22ce/tools/CMakeLists.txt#L20-L28)). The command therefore builds the existing in-process `llama-completion` target, not the absent server-backed CLI; this is a reference smoke executable, not the future K13 chat implementation.

Supply explicit `CMAKE_CUDA_COMPILER`, `CUDAToolkit_ROOT`, and, for Ninja where needed, `CMAKE_CUDA_HOST_COMPILER` pointing at the recorded installed tools; do not let PATH silently select CUDA12 or an unsupported host compiler. Prism additionally keeps `GGML_CUDA_HOPPER_Q1=OFF`. `GGML_CUDA_NO_VMM=ON` intentionally chooses the existing allocation route for the initial bounded probe; later broadening is optional and evidence-driven. Save CMake cache, full command/exit code, build log and artifact hashes. A source-proven missing guard gets one targeted fix and rebuild; do not rotate arbitrary flags/toolchains without recording the failed hypothesis.

3. Inspect the emitted backend artifact with the matching toolkit utilities (actual path is chosen from the build output):

```text
cuobjdump --list-elf <ggml-cuda.dll-or-libggml-cuda.so>
cuobjdump --gpu-architecture sm_37 --dump-sass <ggml-cuda.dll-or-libggml-cuda.so>
cuobjdump --list-ptx <ggml-cuda.dll-or-libggml-cuda.so>
```

Retain non-empty sm_37 cubin/SASS evidence and verify representative conversion/Q1 kernels are present. PTX listing is supplemental; a PTX-only listing fails the native criterion. `cuobjdump` accepts host executable/object/library and embedded fatbins; `nvdisasm` is for extracted cubins. [CUDA11.8 binary utilities](https://docs.nvidia.com/cuda/archive/11.8.0/cuda-binary-utilities/index.html#cuobjdump).

4. Build the existing operator-test executable before the K07 probes (the initial compile isolation above deliberately left tests off). Reconfigure the same recorded toolchain/cache with `cmake -S vendor/llama.cpp -B build/k04-sm37 -DLLAMA_BUILD_TESTS=ON`, then run `cmake --build build/k04-sm37 --target test-backend-ops --parallel 2 --verbose`. Save both exit codes; resolve the executable path from that build. The operator commands in the model note require this artifact. Retain the original compiler/architecture/options in the cache and verify they did not change.

5. A successful build and inspection yields `sm37-compiled`, not `k80-tested`. On an authorized K80 later, first run bounded backend tests for (a) FP16/BF16 storage-to-F32 conversion, (b) Q1 single-vector reference, (c) F32 single/strided/pointer-batch cuBLAS routes and broadcast/permuted inputs, (d) the required non-linear/recurrent operations. Record actual selected device and no hidden CPU fallback. Then K07 performs real small and 27B reference runs under the agreed graph/model identity contract. K04 must not claim these from the fork's measurements.

## Conclusions for lead synthesis

The same core CUDA11/sm37 risks affect both candidates. Existing explicit architecture selection avoids a needless default-list fork. Mainline's FA instance selector differs from Prism's static list, and Prism has optional Hopper-only Q1 code, but neither difference proves compatibility or incompatibility. Both need the newer BF16 selection examined, and both have a separate batched Ex issue requiring a supported route or actual targeted evidence. There is no source-only justification for saying two historical patches suffice, nor for selecting the Kepler fork as the Bonsai runtime.

Unknowns left deliberately explicit: installed toolchain availability; exact Windows/Linux K80 driver/OS tuple; which new TUs/templates fail on the chosen CUDA11 release; whether selected source compiles native sm37; exact graph reachability of batched Ex paths; K80 operation outputs; single-GK210 memory fit and model output. These are bounded later implementation/build/execution gates, not reasons to expand this research into runtime implementation.

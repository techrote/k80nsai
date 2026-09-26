# Opt-in lane; the default root K03 CPU-only profile remains unchanged.
# Preflight runs before project() so CMake cannot discover a different compiler.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "K04 requires an out-of-source build")
endif()
if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
    message(FATAL_ERROR "The bounded K04 lane requires -G Ninja and explicit tool paths")
endif()

function(k04_bool name value)
    if(DEFINED ${name})
        if(value AND NOT ${name} OR NOT value AND ${name})
            message(FATAL_ERROR "K04 requires ${name}=${value}; use a fresh build directory")
        endif()
    endif()
    set(${name} ${value} CACHE BOOL "K04 bounded CUDA lane" FORCE)
endfunction()

k04_bool(BUILD_SHARED_LIBS ON)
k04_bool(GGML_CPU ON)
k04_bool(GGML_CUDA ON)
k04_bool(GGML_CUDA_NO_VMM ON)
k04_bool(LLAMA_BUILD_COMMON ON)
k04_bool(LLAMA_BUILD_TOOLS ON)
k04_bool(FETCHCONTENT_FULLY_DISCONNECTED ON)
foreach(name IN ITEMS
        LLAMA_USE_SYSTEM_GGML GGML_BACKEND_DL GGML_NATIVE GGML_STATIC
        GGML_CUDA_GRAPHS GGML_CUDA_FA GGML_CUDA_NCCL GGML_CUDA_CUB_3DOT2
        GGML_CUDA_FORCE_MMQ GGML_CUDA_FORCE_CUBLAS
        LLAMA_BUILD_SERVER LLAMA_BUILD_APP LLAMA_BUILD_UI LLAMA_USE_PREBUILT_UI
        LLAMA_BUILD_EXAMPLES LLAMA_BUILD_MTMD LLAMA_OPENSSL LLAMA_SUBPROCESS
        LLAMA_LLGUIDANCE LLAMA_TOOLS_INSTALL LLAMA_TESTS_INSTALL
        GGML_CCACHE GGML_BUILD_TESTS GGML_BUILD_EXAMPLES GGML_CPU_ALL_VARIANTS
        GGML_BLAS GGML_ACCELERATE GGML_LLAMAFILE GGML_OPENMP GGML_OPENMP_FETCH
        GGML_CPU_KLEIDIAI GGML_CPU_HBM
        GGML_HIP GGML_MUSA GGML_METAL GGML_VULKAN GGML_WEBGPU GGML_RPC
        GGML_CANN GGML_SYCL GGML_OPENCL GGML_HEXAGON GGML_ZDNN GGML_ZENDNN
        GGML_OPENVINO GGML_ET GGML_VIRTGPU GGML_VIRTGPU_BACKEND)
    k04_bool(${name} OFF)
endforeach()
option(LLAMA_BUILD_TESTS "Compile upstream operator harness after the initial K04 build" OFF)
if(DEFINED CMAKE_CUDA_ARCHITECTURES AND NOT CMAKE_CUDA_ARCHITECTURES STREQUAL "37-real")
    message(FATAL_ERROR "K04 requires CMAKE_CUDA_ARCHITECTURES=37-real; PTX alone is insufficient")
endif()
set(CMAKE_CUDA_ARCHITECTURES "37-real" CACHE STRING "Native GK210 code only" FORCE)

if(NOT IS_ABSOLUTE "${CUDAToolkit_ROOT}" OR NOT IS_DIRECTORY "${CUDAToolkit_ROOT}")
    message(FATAL_ERROR "K04 requires an existing absolute CUDAToolkit_ROOT for CUDA 11.x; no toolkit is installed by this build")
endif()
foreach(name IN ITEMS CMAKE_CUDA_COMPILER CMAKE_CUDA_HOST_COMPILER
        CMAKE_C_COMPILER CMAKE_CXX_COMPILER CMAKE_MAKE_PROGRAM)
    if(NOT IS_ABSOLUTE "${${name}}" OR NOT EXISTS "${${name}}" OR IS_DIRECTORY "${${name}}")
        message(FATAL_ERROR "K04 requires an existing absolute ${name}; do not rely on PATH compiler selection")
    endif()
endforeach()
if(CMAKE_HOST_WIN32)
    set(k04_exe ".exe")
else()
    set(k04_exe "")
endif()
file(REAL_PATH "${CUDAToolkit_ROOT}/bin/nvcc${k04_exe}" k04_nvcc)
file(REAL_PATH "${CMAKE_CUDA_COMPILER}" k04_selected_nvcc)
file(REAL_PATH "${CMAKE_CUDA_HOST_COMPILER}" k04_host)
file(REAL_PATH "${CMAKE_CXX_COMPILER}" k04_cxx)
if(NOT k04_nvcc STREQUAL k04_selected_nvcc)
    message(FATAL_ERROR "CMAKE_CUDA_COMPILER must be CUDAToolkit_ROOT/bin/nvcc${k04_exe}")
endif()
if(NOT k04_host STREQUAL k04_cxx)
    message(FATAL_ERROR "K04 requires the same CXX and CUDA host compiler")
endif()
execute_process(COMMAND "${k04_nvcc}" --version
    RESULT_VARIABLE k04_nvcc_status OUTPUT_VARIABLE k04_nvcc_version ERROR_VARIABLE k04_nvcc_error)
if(NOT k04_nvcc_status EQUAL 0 OR NOT k04_nvcc_version MATCHES "release 11[.][0-9]+,")
    message(FATAL_ERROR "K04 requires NVIDIA CUDA 11.x nvcc, received:\n${k04_nvcc_version}${k04_nvcc_error}")
endif()
set(k04_cuobjdump "${CUDAToolkit_ROOT}/bin/cuobjdump${k04_exe}")
if(NOT EXISTS "${k04_cuobjdump}")
    message(FATAL_ERROR "K04 requires the matching toolkit utility: ${k04_cuobjdump}")
endif()

project(k80nsai-k04 LANGUAGES C CXX CUDA)
if(CMAKE_CROSSCOMPILING OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "K04 requires a native 64-bit host build")
endif()
find_package(Git REQUIRED)
find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)
execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/scripts/verify_upstream.py"
    RESULT_VARIABLE k04_integrity OUTPUT_VARIABLE k04_integrity_output ERROR_VARIABLE k04_integrity_error)
if(NOT k04_integrity EQUAL 0)
    message(FATAL_ERROR "Pinned source integrity check failed:\n${k04_integrity_output}${k04_integrity_error}")
endif()
message(STATUS "${k04_integrity_output}")
find_package(CUDAToolkit 11 REQUIRED)
file(REAL_PATH "${CUDAToolkit_NVCC_EXECUTABLE}" k04_found_nvcc)
if(NOT CUDAToolkit_VERSION VERSION_LESS "12" OR NOT k04_found_nvcc STREQUAL k04_nvcc)
    message(FATAL_ERROR "CUDAToolkit discovery disagrees with the explicit CUDA 11 compiler")
endif()
if(NOT CMAKE_CUDA_COMPILER_ID STREQUAL "NVIDIA" OR
        CMAKE_CUDA_COMPILER_VERSION VERSION_LESS "11" OR
        NOT CMAKE_CUDA_COMPILER_VERSION VERSION_LESS "12")
    message(FATAL_ERROR "The enabled CUDA language must use NVIDIA CUDA 11.x")
endif()
if(TARGET llama OR TARGET ggml OR TARGET ggml-cuda)
    message(FATAL_ERROR "K04 must build its own tracked llama/ggml targets")
endif()

# All TUs and tracked template instances selected by upstream CMake remain intact.
add_subdirectory(vendor/llama.cpp)
foreach(name IN ITEMS llama ggml ggml-base ggml-cpu ggml-cuda llama-completion)
    if(NOT TARGET ${name})
        message(FATAL_ERROR "Missing pinned-source target: ${name}")
    endif()
    get_target_property(k04_imported ${name} IMPORTED)
    get_target_property(k04_source ${name} SOURCE_DIR)
    string(FIND "${k04_source}/" "${CMAKE_SOURCE_DIR}/vendor/llama.cpp/" k04_prefix)
    if(k04_imported OR NOT k04_prefix EQUAL 0)
        message(FATAL_ERROR "${name} must come from the tracked vendor tree")
    endif()
endforeach()
if(LLAMA_BUILD_TESTS AND NOT TARGET test-backend-ops)
    message(FATAL_ERROR "The pinned operator-test target was not generated")
endif()
get_target_property(k04_arch ggml-cuda CUDA_ARCHITECTURES)
if(NOT k04_arch STREQUAL "37-real")
    message(FATAL_ERROR "ggml-cuda target architecture is ${k04_arch}, expected 37-real")
endif()

execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE k04_commit OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${GIT_EXECUTABLE}" status --porcelain WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE k04_dirty COMMAND_ERROR_IS_FATAL ANY)
file(READ "${CMAKE_SOURCE_DIR}/vendor/llama.cpp.lock.json" k04_lock)
string(JSON k04_upstream GET "${k04_lock}" commit)
string(JSON k04_tree GET "${k04_lock}" tree)
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/k04-build.txt" CONTENT
"profile=K04 CUDA 11 native sm_37 (compilation and inspection required)
upstream_commit=${k04_upstream}
upstream_tree=${k04_tree}
implementation_commit=${k04_commit}
configure_changes=${k04_dirty}
cmake=${CMAKE_VERSION}
generator=${CMAKE_GENERATOR}
cuda_root=${CUDAToolkit_ROOT}
cuda_version=${CUDAToolkit_VERSION}
nvcc=${CMAKE_CUDA_COMPILER}
cuda_host=${CMAKE_CUDA_HOST_COMPILER}
cxx=${CMAKE_CXX_COMPILER}
cxx_version=${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}
architectures=${k04_arch}
operator_tests=${LLAMA_BUILD_TESTS}
backend=$<TARGET_FILE:ggml-cuda>
completion=$<TARGET_FILE:llama-completion>
runtime_version_caveat=upstream Git probes report outer K80NSAI metadata
")
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/k04-cuda-sources.txt"
    CONTENT "$<JOIN:$<TARGET_PROPERTY:ggml-cuda,SOURCES>,\n>\n")
set(K80NSAI_SM37_EVIDENCE_DIR "${CMAKE_BINARY_DIR}/sm37-evidence" CACHE PATH
    "New/empty directory for native backend inspection (change for each rebuild)")
add_custom_target(k04-inspect-native
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/scripts/inspect_sm37.py"
        --artifact "$<TARGET_FILE:ggml-cuda>" --cuobjdump "${k04_cuobjdump}"
        --output "${K80NSAI_SM37_EVIDENCE_DIR}"
    DEPENDS ggml-cuda VERBATIM)
message(STATUS "K04 configured; build ggml-cuda and llama-completion, then k04-inspect-native. Configuration is not native-code evidence.")

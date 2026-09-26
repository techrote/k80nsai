cmake_minimum_required(VERSION 3.19)
option(K80NSAI_CUDA_SM37 "Build the separate K04 CUDA 11/native sm_37 lane" OFF)
if(K80NSAI_CUDA_SM37)
    include(cmake/k04-cuda.cmake)
    return()
endif()

project(k80nsai LANGUAGES C CXX)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Use an out-of-source build: cmake -S . -B <build-directory>")
endif()

find_package(Git REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/verify_upstream.py"
    RESULT_VARIABLE integrity_status OUTPUT_VARIABLE integrity_output ERROR_VARIABLE integrity_error)
if(NOT integrity_status EQUAL 0)
    message(FATAL_ERROR "Pinned source integrity check failed:\n${integrity_output}${integrity_error}")
endif()
message(STATUS "${integrity_output}")

# This root entry point is the bounded K03 host smoke. K04 owns a native CUDA lane.
# Reject incompatible overrides so a cached/system backend cannot silently participate.
set(host_disabled
    BUILD_SHARED_LIBS LLAMA_USE_SYSTEM_GGML
    LLAMA_BUILD_COMMON LLAMA_BUILD_TESTS LLAMA_BUILD_TOOLS LLAMA_BUILD_EXAMPLES
    LLAMA_BUILD_SERVER LLAMA_BUILD_APP LLAMA_BUILD_UI LLAMA_BUILD_MTMD
    LLAMA_USE_PREBUILT_UI LLAMA_OPENSSL LLAMA_SUBPROCESS LLAMA_LLGUIDANCE
    LLAMA_TOOLS_INSTALL LLAMA_TESTS_INSTALL
    GGML_BACKEND_DL GGML_CPU_ALL_VARIANTS GGML_NATIVE GGML_BLAS GGML_ACCELERATE
    GGML_LLAMAFILE GGML_OPENMP GGML_OPENMP_FETCH GGML_CPU_KLEIDIAI GGML_CPU_HBM
    GGML_CCACHE GGML_BUILD_TESTS GGML_BUILD_EXAMPLES
    GGML_CUDA GGML_HIP GGML_MUSA GGML_METAL GGML_VULKAN GGML_WEBGPU GGML_RPC
    GGML_CANN GGML_SYCL GGML_OPENCL GGML_HEXAGON GGML_ZDNN GGML_ZENDNN
    GGML_OPENVINO GGML_ET GGML_VIRTGPU GGML_VIRTGPU_BACKEND
    GGML_SSE42 GGML_AVX GGML_AVX2 GGML_BMI2 GGML_AVX_VNNI GGML_FMA GGML_F16C
    GGML_AVX512 GGML_AVX512_VBMI GGML_AVX512_VNNI GGML_AVX512_BF16
    GGML_AMX_TILE GGML_AMX_INT8 GGML_AMX_BF16)
foreach(option_name IN LISTS host_disabled)
    if(${option_name})
        message(FATAL_ERROR "${option_name}=ON is outside the K03 host smoke configuration")
    endif()
    set(${option_name} OFF CACHE BOOL "Disabled for the K03 host smoke" FORCE)
endforeach()
if(DEFINED GGML_CPU AND NOT GGML_CPU)
    message(FATAL_ERROR "The K03 host smoke requires GGML_CPU=ON")
endif()
set(GGML_CPU ON CACHE BOOL "Build the bundled CPU backend" FORCE)
set(FETCHCONTENT_FULLY_DISCONNECTED ON CACHE BOOL "No dependency downloads in the host smoke" FORCE)
if(TARGET llama OR TARGET ggml)
    message(FATAL_ERROR "The host smoke must build llama and ggml from the pinned vendor tree")
endif()

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/vendor/llama.cpp.lock.json" upstream_lock)
string(JSON K80NSAI_UPSTREAM_REPOSITORY GET "${upstream_lock}" repository)
string(JSON K80NSAI_UPSTREAM_COMMIT GET "${upstream_lock}" commit)
string(JSON K80NSAI_UPSTREAM_TREE GET "${upstream_lock}" tree)
execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE K80NSAI_IMPLEMENTATION_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${GIT_EXECUTABLE}" status --porcelain
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE implementation_changes
    COMMAND_ERROR_IS_FATAL ANY)
set(K80NSAI_IMPLEMENTATION_DIRTY "false")
if(implementation_changes)
    set(K80NSAI_IMPLEMENTATION_DIRTY "true")
endif()

# Upstream's own version Git probes intentionally remain unchanged. They see the
# outer repository. Our separately labelled lock/configure IDs are authoritative.
add_subdirectory(vendor/llama.cpp)
foreach(library IN ITEMS llama ggml ggml-base ggml-cpu)
    get_target_property(library_imported ${library} IMPORTED)
    get_target_property(library_source ${library} SOURCE_DIR)
    string(FIND "${library_source}/" "${CMAKE_CURRENT_SOURCE_DIR}/vendor/llama.cpp/" vendor_prefix)
    if(library_imported OR NOT vendor_prefix EQUAL 0)
        message(FATAL_ERROR "${library} must come from the tracked vendor tree")
    endif()
endforeach()

configure_file(cmake/provenance.h.in generated/k80nsai-provenance.h @ONLY)
configure_file(cmake/provenance.json.in k80nsai-provenance.json @ONLY)
add_executable(k80nsai-host-smoke tests/host-smoke.cpp)
target_compile_features(k80nsai-host-smoke PRIVATE cxx_std_17)
target_include_directories(k80nsai-host-smoke PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(k80nsai-host-smoke PRIVATE llama ggml)
enable_testing()
add_test(NAME source-integrity COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/verify_upstream.py")
add_test(NAME host-smoke COMMAND k80nsai-host-smoke)
set_tests_properties(source-integrity host-smoke PROPERTIES TIMEOUT 60)

add_library(k80nsai-q1-reference STATIC q1_reference.cpp)
target_compile_features(k80nsai-q1-reference PUBLIC cxx_std_17)
target_include_directories(k80nsai-q1-reference PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
# The oracle specifies separate, sequential FP32 operations (no contraction).
if(MSVC)
    target_compile_options(k80nsai-q1-reference PUBLIC /fp:strict)
else()
    target_compile_options(k80nsai-q1-reference PUBLIC -fno-fast-math -ffp-contract=off)
endif()

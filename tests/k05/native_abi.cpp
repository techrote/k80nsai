#define GGML_COMMON_DECL_CPP
#include "ggml-common.h"
#include <cstddef>
#include <iostream>
#include <type_traits>

static_assert(QK1_0 == 128);
static_assert(std::is_same<ggml_half, std::uint16_t>::value);
static_assert(sizeof(ggml_half) == 2);
static_assert(sizeof(block_q1_0) == 18);
static_assert(offsetof(block_q1_0, d) == 0);
static_assert(offsetof(block_q1_0, qs) == 2);
static_assert(sizeof(block_q1_0::qs) == 16);
int main() {
    std::cout << "PASS imported ABI: QK1_0=128 d=0 qs=2 signs=16 sizeof=18\n";
}
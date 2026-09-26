#include "k80nsai-provenance.h"
#include "llama.h"
#include "ggml.h"
#include "ggml-backend.h"
#include "ggml-cpu.h"

#include <cstdio>

int main() {
    std::printf("upstream_runtime=%s@%s\nupstream_tree=%s\n",
                K80NSAI_UPSTREAM_REPOSITORY, K80NSAI_UPSTREAM_COMMIT, K80NSAI_UPSTREAM_TREE);
    std::printf("k80nsai_configure_commit=%s\nk80nsai_configure_dirty=%s\n",
                K80NSAI_IMPLEMENTATION_COMMIT, K80NSAI_IMPLEMENTATION_DIRTY);
    std::printf("raw_llama_version=%s\nraw_ggml_commit=%s (outer build metadata)\n",
                llama_version(), ggml_commit());
    llama_backend_init();
    ggml_backend_t cpu = ggml_backend_cpu_init();
    const bool ok = cpu && ggml_backend_is_cpu(cpu)
        && ggml_backend_reg_count() == 1 && ggml_backend_dev_count() == 1
        && ggml_backend_dev_type(ggml_backend_get_device(cpu)) == GGML_BACKEND_DEVICE_TYPE_CPU;
    if (cpu) {
        std::printf("backend=%s\n", ggml_backend_name(cpu));
        ggml_backend_free(cpu);
    }
    llama_backend_free();
    if (!ok) {
        std::fprintf(stderr, "FAIL: expected exactly one bundled CPU backend/device\n");
        return 1;
    }
    std::puts("PASS: llama/ggml host initialization; no model loaded");
    return 0;
}

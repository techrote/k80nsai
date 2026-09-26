# k80nsai

Real-model, command-line experiments in approximate Bonsai Q1 inference on NVIDIA Tesla K80.

**Current state: implementation workflow deployed, not an implemented inference engine. No K80 speedup or model-quality result is claimed.**

Start with the [task atlas and milestones](OVERVIEW.md), [master issue #1](https://github.com/techrote/k80nsai/issues/1), and [agent instructions](AGENTS.md). The [shared reference bundle](docs/rag/INDEX.md) supplies the contracts used by the implementation issues.

## What gets built

One implementation phase: a pinned Bonsai-capable llama.cpp runtime, simple terminal chat, and `reference`, `b1`, `b2`, `b3`, `adaptive` modes on one GK210 device. Real-model generation happens early. A small binary Bonsai model is a plumbing fixture; binary 27B is the main evaluation target.

Only eligible linear operations are approximated. Attention, normalization, model loading, tokenization and sampling retain their reference implementation. Unsupported operations are reported, not silently described as accelerated. Output variation is intentional; invalid tensor interpretation and stale state are not.

## Execution queue

There are **23 published issues: one master tracker, 20 required tasks and two deferred extensions**. Begin with [K01/#2](https://github.com/techrote/k80nsai/issues/2), then [K03/#3](https://github.com/techrote/k80nsai/issues/3). Once the source is imported, native build compatibility, host reference tests and model acquisition can run concurrently. Later the CLI, telemetry and encoder can proceed independently against the shared interface.

Task ID K02 was retired before publication; there is no missing required issue. Ordinary build prerequisites remain in K04, without a separate automatic environment-doctor application. Follow the atlas rather than assuming issue numbers equal task IDs.

Milestones are evidence checkpoints within the single phase. Dependencies and concurrency rules are documented and machine-readable, not assumed to be enforced by GitHub's server.

## Workflow integrity

Run `python scripts/check_workflow.py` from the repository root. Optional `--live` checks public issue IDs and titles. See the [deployment audit](docs/DEPLOYMENT_AUDIT.md) for what was and was not validated during publication.

The bounded host workspace is described below. Chat commands remain downstream implementation work.

## Pinned host workspace

K03 imports `ggml-org/llama.cpp@56381e407c0ccfb3a6f71e668a27a901001d22ce`.
See the [import/licence/reproduction record](docs/tasks/K03/IMPORT.md) and
[K03 status](docs/tasks/K03/STATUS.md). This verifies host integration; CUDA/K80,
model execution and experimental chat remain downstream work.

From the repository root with Git, Python 3.10+, CMake 3.19+ and C++17 tools:

```text
python scripts/verify_upstream.py
cmake -S . -B build/host -G "Visual Studio 17 2022" -A x64
cmake --build build/host --config Release --target k80nsai-host-smoke --parallel 2
ctest --test-dir build/host -C Release --output-on-failure
```

These are native Windows x64 host commands. Other generators are selectable;
Linux/macOS builds are untested. The root profile rejects CUDA/system-ggml
overrides. Products stay in ignored `build/` or outside source. Reconfigure after
switching commits to refresh the implementation identity. Planned experimental
flags in the reference bundle are not already available binaries.

# k80nsai

Real-model, command-line experiments in approximate Bonsai Q1 inference on NVIDIA Tesla K80.

**Current state: planning/workflow deployed, not an implemented inference engine. No K80 speedup or model-quality result is claimed.**

Start with the [task atlas and milestones](OVERVIEW.md), the [master issue](https://github.com/techrote/k80nsai/issues/1), and [agent operating instructions](AGENTS.md). The [shared reference bundle](docs/rag/INDEX.md) supplies the contracts used by every implementation issue.

## What gets built

One implementation phase: a pinned Bonsai-capable llama.cpp runtime, a simple terminal chat, and `reference`, `b1`, `b2`, `b3`, and `adaptive` modes on one GK210 device. Real model generation happens early. Bonsai 1.7B is a plumbing fixture; the binary 27B model is the main target. Unsupported operations retain the reference path and are reported, not silently described as accelerated.

The experiment changes selected linear operations during decoding. It does not replace attention, tokenization, sampling, normalization, or the model architecture. Output changes are intentional; bad indexing, corrupted state, or silent CPU execution are not.

## Execution queue

The first independent tasks are [K01: source/compatibility map](https://github.com/techrote/k80nsai/issues/2) and [K02: environment/preflight](https://github.com/techrote/k80nsai/issues/3). Follow dependencies rather than numerical issue order. The atlas permits concurrent development but reserves shared runtime interfaces and K80 measurements to one owner at a time.

The repository initially contains 21 required implementation tasks, one master tracker, and two explicitly deferred extensions. Milestones in the atlas are evidence checkpoints inside the single phase, not separate approval rounds.

## Documentation integrity

Run `python scripts/check_workflow.py` from the repository root. It validates the local task graph and reference-bundle links without requiring CUDA or GitHub credentials. Live issue status remains authoritative; the JSON graph is a dependency manifest, not a claim that tasks are complete.

Build/chat commands will be added and tested by the implementation tasks. Do not confuse planned CLI flags in the reference bundle with already available binaries.

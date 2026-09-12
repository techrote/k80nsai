# k80nsai

A real-model, command-line proof of concept for approximate Bonsai Q1 inference on NVIDIA Tesla K80.

**Status:** implementation workflow being published; no inference implementation or K80 performance result is claimed by this repository bootstrap.

The core experiment is `reference`, `b1`, `b2`, `b3`, and `adaptive` decode, using one GK210 device at a time. The runtime, tokenizer, attention and model loading remain based on a pinned Bonsai-capable llama.cpp revision. This is one implementation phase with incremental, real-model checkpoints—not a prerequisite synthetic research programme.

The task atlas will live in [OVERVIEW.md](OVERVIEW.md), agent instructions in [AGENTS.md](AGENTS.md), and the shared reference bundle in [docs/rag/INDEX.md](docs/rag/INDEX.md). GitHub issues are the execution queue.

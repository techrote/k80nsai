#!/usr/bin/env python3
"""Inspect the built ggml-cuda library with its CUDA 11 toolkit (no GPU execution)."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys


def native_functions(elf: str, sass: str) -> list[str]:
    """Require an sm_37 ELF and real instruction bodies in sm_37 SASS sections."""
    if not re.search(r"(?m)^\s*ELF file\s+\d+:.*[.]sm_37[.]cubin\s*$", elf):
        raise ValueError("No native sm_37 cubin in backend ELF listing")
    arch = function = None
    functions = set()
    for line in sass.splitlines():
        match = re.match(r"\s*(?:arch\s*=|code for)\s+(sm_\w+)\s*$", line)
        if match:
            arch, function = match[1], None
        match = re.match(r"\s*Function\s*:\s*(\S+)", line)
        if match:
            function = match[1]
        if arch == "sm_37" and function and re.search(
            r"/\*[0-9a-fA-F]+\*/\s+[^/]*;\s*/\*\s*0x[0-9a-fA-F]+\s*\*/", line
        ):
            functions.add(function)
    if not functions:
        raise ValueError("No non-empty native sm_37 SASS function bodies in backend")
    return sorted(functions)


def inspect(artifact: Path, cuobjdump: Path, output: Path) -> None:
    artifact, cuobjdump, output = artifact.resolve(), cuobjdump.resolve(), output.resolve()
    if not artifact.is_file() or not re.fullmatch(
        r"(?:ggml-cuda\.dll|libggml-cuda\.so(?:\.[0-9]+)*)", artifact.name
    ):
        raise ValueError("Expected the built ggml-cuda DLL/shared library, not a CLI or import library")
    if not cuobjdump.is_file():
        raise ValueError("Missing matching CUDA 11 cuobjdump executable")
    # Never overwrite an earlier record or leave its success attached to a failed rebuild.
    output.mkdir(parents=True, exist_ok=False)
    before = hashlib.sha256(artifact.read_bytes()).hexdigest()
    records = []

    def run(label: str, *args: str) -> str:
        command = [str(cuobjdump), *args]
        result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False)
        (output / f"{label}.stdout.txt").write_bytes(result.stdout)
        (output / f"{label}.stderr.txt").write_bytes(result.stderr)
        records.append({"command": command, "cwd": str(Path.cwd()), "exit_code": result.returncode})
        (output / "commands.json").write_text(json.dumps(records, indent=2) + "\n", encoding="utf-8")
        if result.returncode:
            raise ValueError(f"cuobjdump {label} failed with exit {result.returncode}; see {output}")
        return result.stdout.decode("utf-8", errors="replace")

    version = run("version", "--version")
    if not re.search(r"release 11\.\d+,", version):
        raise ValueError("Artifact inspection requires CUDA 11.x cuobjdump")
    elf = run("list-elf", "--list-elf", str(artifact))
    sass = run("sm37-sass", "--gpu-architecture", "sm_37", "--dump-sass", str(artifact))
    run("list-ptx", "--list-ptx", str(artifact))
    functions = native_functions(elf, sass)
    after = hashlib.sha256(artifact.read_bytes()).hexdigest()
    if before != after:
        raise ValueError("Backend changed during inspection; repeat after the build finishes")
    (output / "sm37-functions.txt").write_text("\n".join(functions) + "\n", encoding="utf-8")
    summary = {
        "artifact": str(artifact), "sha256": after, "bytes": artifact.stat().st_size,
        "cuobjdump": str(cuobjdump), "cuobjdump_version": version.strip(),
        "native_architecture": "sm_37", "native_function_count": len(functions),
        "q1_conversion_candidates": [f for f in functions if "q1_0" in f.lower()],
        "mmvq_candidates": [f for f in functions if "mul_mat_vec_q" in f.lower()],
        "representative_kernel_review": "Manually map candidates to pinned source; helper names may be inlined",
        "k80_tested": False, "model_tested": False,
    }
    (output / "native-artifact.json").write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
    print(f"PASS: {len(functions)} native sm_37 functions in {artifact.name}; sha256={after}")
    print(f"Evidence: {output}; no GPU or model execution performed")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--artifact", required=True, type=Path)
    parser.add_argument("--cuobjdump", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path, help="new directory; never overwrite earlier evidence")
    args = parser.parse_args()
    try:
        inspect(args.artifact, args.cuobjdump, args.output)
    except (OSError, ValueError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

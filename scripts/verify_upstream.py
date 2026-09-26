#!/usr/bin/env python3
"""Verify the pristine llama.cpp import, index, and actual checkout without a network."""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
LOCK = ROOT / "vendor/llama.cpp.lock.json"


def git(*args: str, cwd: Path = ROOT) -> bytes:
    return subprocess.check_output(["git", *args], cwd=cwd)


def verify(upstream_repo: Path | None = None) -> None:
    lock = json.loads(LOCK.read_text(encoding="utf-8"))
    prefix = lock["destination"]
    source = ROOT / prefix
    expected = lock["tree"]
    if lock["exclusions"]:
        raise ValueError("This verifier requires a complete, unfiltered upstream tree")
    if upstream_repo is not None:
        for suffix, value in (("commit", lock["commit"]), ("tree", expected)):
            actual = git("rev-parse", f'{lock["commit"]}^{{{suffix}}}', cwd=upstream_repo).decode().strip()
            if actual != value:
                raise ValueError(f"Upstream {suffix} does not match the lock: {actual}")
    for ref in ("HEAD", lock["import_commit"]):
        actual = git("rev-parse", f"{ref}:{prefix}").decode().strip()
        if actual != expected:
            raise ValueError(f"{ref}:{prefix} is {actual}, expected {expected}")

    entries = {}
    for record in git("ls-tree", "-r", "-z", f"HEAD:{prefix}").split(b"\0"):
        if not record:
            continue
        metadata, name = record.split(b"\t", 1)
        mode, kind, oid = metadata.decode().split()
        if kind != "blob" or mode not in ("100644", "100755"):
            raise ValueError(f"Unexpected upstream entry type: {record!r}")
        entries[name.decode("utf-8")] = (mode, oid)

    index = {}
    for record in git("ls-files", "--stage", "-z", "--", prefix).split(b"\0"):
        if record:
            metadata, name = record.split(b"\t", 1)
            mode, oid, stage = metadata.decode().split()
            if stage != "0":
                raise ValueError("Unmerged vendor index entry")
            index[name.decode("utf-8")[len(prefix) + 1:]] = (mode, oid)
    if index != entries:
        raise ValueError("Vendor index differs from the locked tree (including file modes)")

    present = {p.relative_to(source).as_posix() for p in source.rglob("*")
               if p.is_file() or p.is_symlink()}
    missing, extra = entries.keys() - present, present - entries.keys()
    if missing or extra:
        raise ValueError(f"Vendor file inventory differs: missing={sorted(missing)}, extra={sorted(extra)}")
    total = 0
    for name, (mode, oid) in entries.items():
        path = source / name
        if path.is_symlink():
            raise ValueError(f"Unexpected symlink: {name}")
        data = path.read_bytes()
        total += len(data)
        actual = hashlib.sha1(b"blob " + str(len(data)).encode() + b"\0" + data).hexdigest()
        if actual != oid:
            raise ValueError(f"Modified vendor file: {name}")
        if os.name != "nt" and bool(path.stat().st_mode & 0o111) != (mode == "100755"):
            raise ValueError(f"Modified executable mode: {name}")
    if len(entries) != lock["file_count"] or total != lock["total_bytes"]:
        raise ValueError("Vendor file/byte counts do not match the lock")
    for name in lock["notice_files"]:
        if name not in entries:
            raise ValueError(f"Missing retained notice: {name}")
    print(f'PASS: {lock["repository"]}@{lock["commit"]}')
    print(f"tree={expected}; files={len(entries)}; bytes={total}; exclusions=0")
    print("Committed tree, import commit, index modes, working bytes and notices verified.")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--upstream-repo", type=Path,
                        help="also resolve commit/tree in an independently fetched upstream Git repository")
    args = parser.parse_args()
    try:
        verify(args.upstream_repo)
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

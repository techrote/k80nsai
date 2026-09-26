"""Synthetic parser/collector negative controls; never target compilation evidence."""
import importlib.util
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

SPEC = importlib.util.spec_from_file_location("inspect_sm37", Path(__file__).resolve().parents[1] / "scripts/inspect_sm37.py")
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)
ELF = "ELF file    1: backend.sm_37.cubin\n"
# Synthetic example in the documented cuobjdump format, not actual CUDA output.
SASS = "arch = sm_37\ncode for sm_37\nFunction : sample\n/*0008*/ EXIT ; /* 0x8000000000001de7 */\n"


class NativeEvidenceTests(unittest.TestCase):
    def test_native_body(self):
        self.assertEqual(MODULE.native_functions(ELF, SASS), ["sample"])

    def test_ptx_only(self):
        with self.assertRaises(ValueError):
            MODULE.native_functions("PTX file 1: backend.sm_37.ptx", ".target sm_37\n.entry sample() {}")

    def test_headers_are_not_instructions(self):
        with self.assertRaises(ValueError):
            MODULE.native_functions(ELF, "arch = sm_37\nFunction : sample\n")

    def test_other_arch_instructions_do_not_count(self):
        with self.assertRaises(ValueError):
            MODULE.native_functions(ELF, "arch = sm_37\n" + SASS.replace("sm_37", "sm_75"))

    def test_unscoped_instruction_does_not_count(self):
        with self.assertRaises(ValueError):
            MODULE.native_functions(ELF, "Function : sample\n/*0008*/ EXIT ; /* 0x1234 */\n")

    def test_cli_and_import_library_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name in ("llama-completion.exe", "ggml-cuda.lib"):
                artifact = root / name
                artifact.touch()
                with self.assertRaises(ValueError):
                    MODULE.inspect(artifact, root / "cuobjdump.exe", root / "output")

    def test_existing_evidence_never_overwritten(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            artifact, tool, output = root / "ggml-cuda.dll", root / "cuobjdump.exe", root / "output"
            artifact.touch()
            tool.touch()
            output.mkdir()
            marker = output / "native-artifact.json"
            marker.write_text("earlier evidence", encoding="utf-8")
            with patch.object(MODULE.subprocess, "run") as run:
                with self.assertRaises(FileExistsError):
                    MODULE.inspect(artifact, tool, output)
                run.assert_not_called()
            self.assertEqual(marker.read_text(encoding="utf-8"), "earlier evidence")


if __name__ == "__main__":
    unittest.main()

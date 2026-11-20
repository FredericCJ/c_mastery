"""Offline regressions for the guest launcher. No observed guest evidence is produced here."""
import hashlib
import importlib.util
import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

LAB = Path(__file__).resolve().parents[1] / "labs/linux_supervision"
SPEC = importlib.util.spec_from_file_location("supervision_guest", LAB / "guest.py")
guest = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(guest)

BOOT = "LS_BOOT kernel=7.0.0-43-generic controllers=cpu pids"
END = "LS_BOOT_END status=0"
ROWS = [
    "LS_INNER_FIX name=namespace variant=view pid=1",
    "LS_OUTER name=namespace variant=view child_pid=77",
    "LS_CLEAN name=namespace variant=view pids_current=0 removed=1 empty_procs=1",
    "LS_CASE_END name=namespace variant=view status=0",
]


def classify(lines, code=0):
    return guest.classify(lines, ["namespace"], {"qemu_returncode": code, "cases": []})


class LaunchTests(unittest.TestCase):
    def test_same_grading_directory_preserves_both_attempts_and_prediction(self):
        # Exercise two ordinary submissions using the same CTest output path.
        # An empty cache makes each attempt fast and deliberately unavailable.
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            lab = root / "workspace"
            shutil.copytree(LAB, lab, ignore=shutil.ignore_patterns("build", "__pycache__", ".cmastery", "answers"))
            (lab / "answers").mkdir()
            answer = lab / "answers/namespace.json"
            answer.write_text(json.dumps({"prediction": "The outer PID differs.", "explanation": "Join the two views."}))
            for folder in ("reader", "worker"):
                helper = lab / "src" / folder / "state.h"
                helper.parent.mkdir()
                helper.write_text(f"/* {folder} state */\n")
            env = {**os.environ, "CMASTERY_LINUX_GUEST_CACHE": str(root / "missing-cache")}
            target = root / "target"
            cmd = [sys.executable, lab / "submission.py", "check", "namespace", "--program", sys.executable,
                   "--out", target]
            for _ in range(2):
                result = subprocess.run(cmd, capture_output=True, text=True, env=env, timeout=15)
                self.assertEqual(result.returncode, 77, result.stderr)
            records = sorted(target.glob("attempt-*/prediction.json"))
            self.assertEqual(len(records), 2)
            self.assertEqual(records[0].read_bytes(), records[1].read_bytes())
            sources = json.loads(records[0].read_text())["implementation"]
            self.assertNotEqual(sources["reader/state.h"], sources["worker/state.h"])
            answer.write_text(json.dumps({"prediction": "Changed after observation.", "explanation": "Same source."}))
            result = subprocess.run(cmd, capture_output=True, text=True, env=env, timeout=15)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("original prediction", result.stderr)

    def test_complete_launch_and_program_failure(self):
        self.assertEqual(classify([BOOT, *ROWS, END]), "completed")
        failed = "LS_CASE_END name=namespace variant=view status=1"
        self.assertEqual(classify([BOOT, *ROWS[:-1], failed, "LS_BOOT_END status=1"]),
                         "case-failed")

    def test_partial_duplicate_or_out_of_order_serial(self):
        for lines in (
            [BOOT, *ROWS],
            [BOOT, END],
            [BOOT, ROWS[-1], END],
            [BOOT, *ROWS, ROWS[-1], END],
            [BOOT, *ROWS, END, END],
            [BOOT, BOOT, *ROWS, END],
            [BOOT, END, *ROWS],
        ):
            with self.subTest(lines=lines), self.assertRaises(ValueError):
                classify(lines)
        with self.assertRaises(ValueError):
            classify([BOOT, *ROWS, END], code=1)

    def test_explicit_guest_capability_failure(self):
        with self.assertRaises(guest.Unavailable):
            classify(["LS_BOOT_END status=77"])
        with self.assertRaises(ValueError):
            classify(["LS_BOOT_END status=77"], code=1)

    def test_missing_live_cache_records_unavailable(self):
        with tempfile.TemporaryDirectory() as tmp:
            out = Path(tmp) / "observations"
            cmd = [sys.executable, str(LAB / "guest.py"), "smoke", "--out", out,
                   "--cache", Path(tmp) / "absent"]
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 77, result.stderr)
            record = json.loads((out / "launch.json").read_text())
            self.assertEqual(record["status"], "unavailable")
            self.assertIsNone(record["qemu_returncode"])
            self.assertEqual(record["cases"], [])
            self.assertEqual(record["sha256"]["serial"], guest.digest(out / "serial.txt"))

    def test_missing_and_changed_package_are_distinct(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            spec = {"archive": "pool/main/x/fixture.deb",
                    "sha256": hashlib.sha256(b"pinned bytes").hexdigest()}
            with self.assertRaises(guest.Unavailable):
                guest.verify_package(root, spec)
            path = root / "fixture.deb"
            path.write_bytes(b"pinned bytes")
            self.assertEqual(guest.verify_package(root, spec), path)
            path.write_bytes(b"changed bytes")
            with self.assertRaises(ValueError):
                guest.verify_package(root, spec)

    def test_setup_rejects_changed_download_before_replacing_cache(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            archive = root / "fixture.deb"
            archive.write_bytes(b"original")
            spec = {"archive": archive.name, "sha256": guest.digest(archive)}
            cache = root / "cache"
            with patch.object(guest, "PACKAGES", {"fixture": spec}), \
                    patch.object(guest, "SNAPSHOT", root.as_uri() + "/"):
                guest.setup(cache)
                local = guest.verify_package(cache, spec)
                local.write_bytes(b"unchanged on failure")
                archive.write_bytes(b"wrong download")
                with self.assertRaises(ValueError):
                    guest.setup(cache)
                self.assertEqual(local.read_bytes(), b"unchanged on failure")

    @unittest.skipUnless(shutil.which("cc") and shutil.which("readelf"), "requires native ELF tools")
    def test_real_x86_64_static_executable(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = root / "main.c"
            source.write_text("int main(void) { return 0; }\n")
            for name, flags in (("static", ["-static"]), ("dynamic", []), ("object", ["-c"])):
                path = root / name
                subprocess.run(["cc", *flags, source, "-o", path], check=True)
                if name == "static":
                    guest.checked_static(path)
                else:
                    with self.subTest(name=name), self.assertRaises(ValueError):
                        guest.checked_static(path)


if __name__ == "__main__":
    unittest.main()

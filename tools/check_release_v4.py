"""Exercise v4 receipt and promotion guards in an isolated temporary sandbox.

No real acceptance, archive, historical receipt, or target runner is modified.
The wrapper tests substitute subprocesses; they do not execute target campaigns.
"""
from contextlib import ExitStack, redirect_stdout
import io
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

import accept_v4
import package_v4
import run_targets_v4
import run_legacy_matrix_v4
from common import ROOT, dump, read_json, sha


class ReleaseGuards(unittest.TestCase):
    RECEIPTS = "reviews/v4"
    MODULES = (accept_v4, package_v4, run_targets_v4, run_legacy_matrix_v4)
    ACCEPT = accept_v4
    PACKAGE = package_v4
    TARGETS = run_targets_v4
    LEGACY = run_legacy_matrix_v4

    def setUp(self):
        parent = ROOT / "build/release_guard_checks_v4"
        if parent.resolve() != ROOT.resolve() / "build/release_guard_checks_v4":
            raise ValueError("Unsafe release-test directory")
        parent.mkdir(parents=True, exist_ok=True)
        temporary = tempfile.TemporaryDirectory(prefix="case_", dir=parent)
        sandbox = Path(temporary.name).resolve()
        if sandbox.parent != parent.resolve():
            raise ValueError("Unsafe release-test sandbox")
        self.addCleanup(temporary.cleanup)
        self.sandbox = sandbox
        self.root = sandbox / "checkout"
        self.root.mkdir()
        self.stack = ExitStack()
        self.addCleanup(self.stack.close)
        for module in self.MODULES:
            self.stack.enter_context(patch.object(module, "ROOT", self.root))
        self.stack.enter_context(redirect_stdout(io.StringIO()))
        self.write("curriculum/example.md", "accepted teaching\n")
        self.receipt = {"passed": True, "inspected_paths": {
            "curriculum/example.md": sha(self.root / "curriculum/example.md")}, "findings": []}
        dump(self.root / self.RECEIPTS / "check.json", self.receipt)
        self.dist = self.root / "dist"
        self.dist.mkdir()
        self.candidate = self.root / "build/release_candidates/new"
        self.candidate.mkdir(parents=True)

    def write(self, relative, text):
        path = self.root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8", newline="\n")
        return path

    def change_receipt(self, **changes):
        record = dict(self.receipt)
        record.update(changes)
        dump(self.root / self.RECEIPTS / "check.json", record)

    def symlink(self, path, target, directory=False):
        try:
            path.symlink_to(target, target_is_directory=directory)
        except (OSError, NotImplementedError) as error:
            self.skipTest("Symlink unavailable on this platform: " + str(error))

    def staged(self, existing=(0, 1, 2, 3)):
        archives = []
        before = {}
        for index, name in enumerate(self.PACKAGE.ARCHIVES):
            source = self.candidate / name
            source.write_bytes(("new:" + name).encode())
            archives.append(source)
            if index in existing:
                destination = self.dist / name
                destination.write_bytes(("old:" + name).encode())
                before[name] = destination.read_bytes()
        return archives, before

    def test_current_pinned_receipt_returns_receipt_digest(self):
        self.assertEqual(self.ACCEPT.pinned_receipt("check.json"),
                         sha(self.root / self.RECEIPTS / "check.json"))

    def test_missing_receipt_rejects(self):
        with self.assertRaises(FileNotFoundError):
            self.ACCEPT.pinned_receipt("missing.json")

    def test_failed_receipt_rejects_even_with_current_source(self):
        self.change_receipt(passed=False)
        with self.assertRaisesRegex(ValueError, "Failed or absent"):
            self.ACCEPT.pinned_receipt("check.json")

    def test_unpinned_receipt_rejects(self):
        self.change_receipt(inspected_paths={})
        with self.assertRaisesRegex(ValueError, "no source pins"):
            self.ACCEPT.pinned_receipt("check.json")

    def test_stale_source_rejects(self):
        self.write("curriculum/example.md", "changed after execution\n")
        with self.assertRaisesRegex(ValueError, "Stale receipt"):
            self.ACCEPT.pinned_receipt("check.json")

    def test_missing_pinned_source_rejects(self):
        (self.root / "curriculum/example.md").unlink()
        with self.assertRaisesRegex(ValueError, "Stale receipt"):
            self.ACCEPT.pinned_receipt("check.json")

    def test_escaping_pin_rejects_even_when_external_hash_matches(self):
        outside = self.sandbox / "outside.md"
        outside.write_text("outside\n", encoding="utf-8")
        self.change_receipt(inspected_paths={"../outside.md": sha(outside)})
        with self.assertRaisesRegex(ValueError, "Stale receipt"):
            self.ACCEPT.pinned_receipt("check.json")

    def test_redirected_source_pin_rejects(self):
        outside = self.sandbox / "outside.md"
        outside.write_text("outside\n", encoding="utf-8")
        link = self.root / "curriculum/linked.md"
        self.symlink(link, outside)
        self.change_receipt(inspected_paths={"curriculum/linked.md": sha(outside)})
        with self.assertRaisesRegex(ValueError, "Stale receipt"):
            self.ACCEPT.pinned_receipt("check.json")

    def test_open_medium_finding_rejects(self):
        self.change_receipt(findings=[{"severity": "medium", "status": "open"}])
        with self.assertRaisesRegex(ValueError, "Unresolved finding"):
            self.ACCEPT.pinned_receipt("check.json")

    def test_resolved_medium_finding_is_permitted(self):
        self.change_receipt(findings=[{"severity": "medium", "status": "resolved"}])
        self.ACCEPT.pinned_receipt("check.json")

    def test_closed_high_finding_is_permitted(self):
        self.change_receipt(findings=[{"severity": "high", "status": "closed"}])
        self.ACCEPT.pinned_receipt("check.json")

    def test_relative_release_directory_is_created_inside_checkout(self):
        expected = self.root / "build/another_candidate"
        self.assertEqual(self.PACKAGE.bounded_directory(Path("build/another_candidate")), expected)
        self.assertTrue(expected.is_dir())

    def test_parent_traversal_release_directory_rejects(self):
        with self.assertRaises(ValueError):
            self.PACKAGE.bounded_directory(Path("../escaped"))
        self.assertFalse((self.sandbox / "escaped").exists())

    def test_absolute_release_directory_rejects(self):
        outside = self.sandbox / "escaped"
        with self.assertRaises(ValueError):
            self.PACKAGE.bounded_directory(outside)
        self.assertFalse(outside.exists())

    def test_redirected_release_directory_rejects(self):
        outside = self.sandbox / "outside"
        outside.mkdir()
        self.symlink(self.root / "redirected", outside, directory=True)
        with self.assertRaisesRegex(ValueError, "Unsafe release directory"):
            self.PACKAGE.bounded_directory(Path("redirected"))

    def test_promotion_replaces_all_four_candidates(self):
        archives, before = self.staged()
        destinations = self.PACKAGE.promote(archives, self.dist, self.candidate)
        self.assertEqual(len(destinations), 4)
        for destination in destinations:
            self.assertEqual(destination.read_bytes(), ("new:" + destination.name).encode())
            self.assertNotEqual(destination.read_bytes(), before[destination.name])

    def test_promotion_failure_restores_old_files_and_removes_new_ones(self):
        archives, before = self.staged(existing=(0, 2, 3))
        original = Path.replace

        def injected_failure(path, destination):
            if path == archives[2]:
                raise OSError("injected third promotion failure")
            return original(path, destination)

        with patch.object(Path, "replace", injected_failure):
            with self.assertRaisesRegex(OSError, "injected third"):
                self.PACKAGE.promote(archives, self.dist, self.candidate)
        for name, content in before.items():
            self.assertEqual((self.dist / name).read_bytes(), content)
        self.assertFalse((self.dist / self.PACKAGE.ARCHIVES[1]).exists())
        self.assertTrue(archives[2].exists())

    def test_directory_destination_rejects_before_any_replacement(self):
        archives, before = self.staged(existing=(0, 1, 3))
        (self.dist / self.PACKAGE.ARCHIVES[2]).mkdir()
        with self.assertRaisesRegex(ValueError, "not a file"):
            self.PACKAGE.promote(archives, self.dist, self.candidate)
        for name, content in before.items():
            self.assertEqual((self.dist / name).read_bytes(), content)
        self.assertTrue(all(path.is_file() for path in archives))

    def test_escaped_final_archive_rejects_before_any_replacement(self):
        archives, before = self.staged(existing=(0, 1, 3))
        outside = self.sandbox / "outside.zip"
        outside.write_bytes(b"untouched outside archive")
        self.symlink(self.dist / self.PACKAGE.ARCHIVES[2], outside)
        with self.assertRaisesRegex(ValueError, "Unsafe final archive"):
            self.PACKAGE.promote(archives, self.dist, self.candidate)
        self.assertEqual(outside.read_bytes(), b"untouched outside archive")
        for name, content in before.items():
            self.assertEqual((self.dist / name).read_bytes(), content)

    def prepare_wrapper(self):
        self.write("reviews/v3/untouched.json", '{"historical": "keep bytes exactly"}\n')
        for name in run_targets_v4.OUTPUTS:
            self.write("reviews/v3/" + name, '{"status":"passed","passed":true,"old":true}\n')
        identity = {"sha256": "fixture-source", "files": self.receipt["inspected_paths"]}
        self.stack.enter_context(patch.object(run_targets_v4, "source_identity", return_value=identity))
        return run_targets_v4.historical_files(self.root / "reviews/v3")

    def successful_runner(self, command, **kwargs):
        if command[3].endswith("run_target_matrix.py"):
            for name in run_targets_v4.TARGET_RECEIPTS:
                dump(self.root / "reviews/v3" / name, {"status": "passed", "current": True})
            dump(self.root / "reviews/v3/target_matrix.json", {"status": "passed", "passed": True})
        else:
            dump(self.root / "reviews/v3/os_comparison.json", {"status": "passed", "passed": True})
        return subprocess.CompletedProcess(command, 0, "synthetic wrapper test only", "")

    def test_target_adapter_preserves_every_historical_byte(self):
        before = self.prepare_wrapper()
        with patch.object(run_targets_v4.subprocess, "run", side_effect=self.successful_runner):
            self.assertEqual(self.TARGETS.run(native=True), 0)
        self.assertEqual(run_targets_v4.historical_files(self.root / "reviews/v3"), before)
        record = read_json(self.root / self.RECEIPTS / "target_matrix.json")
        self.assertTrue(record["passed"])
        self.assertEqual(record["inspected_paths"], self.receipt["inspected_paths"])
        self.assertNotIn("old", record)

    def test_target_adapter_does_not_relabel_historical_success(self):
        before = self.prepare_wrapper()
        failure = subprocess.CompletedProcess([], 9, "failed before writing a receipt", "")
        with patch.object(run_targets_v4.subprocess, "run", return_value=failure):
            self.assertEqual(self.TARGETS.run(native=True), 1)
        self.assertEqual(run_targets_v4.historical_files(self.root / "reviews/v3"), before)
        for name in ("target_matrix.json", "os_comparison.json"):
            record = read_json(self.root / self.RECEIPTS / name)
            self.assertFalse(record["passed"])
            self.assertIn("no new receipt", record["reason"])

    def test_source_change_prevents_target_receipt_promotion(self):
        before = self.prepare_wrapper()
        initial = {"sha256": "before", "files": self.receipt["inspected_paths"]}
        changed = {"sha256": "after", "files": self.receipt["inspected_paths"]}
        with patch.object(run_targets_v4, "source_identity", side_effect=[initial, changed]):
            with patch.object(run_targets_v4.subprocess, "run", side_effect=self.successful_runner):
                self.assertEqual(self.TARGETS.run(native=True), 1)
        self.assertEqual(run_targets_v4.historical_files(self.root / "reviews/v3"), before)
        for name in ("target_matrix.json", "os_comparison.json"):
            record = read_json(self.root / self.RECEIPTS / name)
            self.assertFalse(record["passed"])
            self.assertTrue(record["source_changed_during_wrapper"])

    def test_target_adapter_restores_history_on_unexpected_runner_exception(self):
        before = self.prepare_wrapper()

        def failure(command, **kwargs):
            dump(self.root / "reviews/v3/new_partial.json", {"incomplete": True})
            raise OSError("runner launch failed")

        with patch.object(run_targets_v4.subprocess, "run", side_effect=failure):
            with self.assertRaisesRegex(OSError, "runner launch failed"):
                self.TARGETS.run(native=True)
        self.assertEqual(run_targets_v4.historical_files(self.root / "reviews/v3"), before)
        self.assertFalse(read_json(self.root / self.RECEIPTS / "target_matrix.json")["passed"])


    def prepare_legacy_wrapper(self):
        self.write("reviews/v3/untouched.json", '{"historical": "retain these bytes"}\n')
        names = ["lab_matrix_" + "_".join(run_legacy_matrix_v4.FAMILIES) + ".json"]
        names += ["lab_" + family + ".json" for family in run_legacy_matrix_v4.FAMILIES]
        for name in names:
            self.write("reviews/v3/" + name, '{"passed":true,"status":"passed","old":true}\n')
        for name in ["cmastery/cli.py", "cmastery/learning.py", "tools/run_lab_matrix.py",
                     "tools/run_legacy_matrix_v4.py"]:
            self.write(name, "# Synthetic source-pin fixture; never executed.\n")
        for family in run_legacy_matrix_v4.FAMILIES:
            self.write("labs/" + family + "/fixture.c", "/* synthetic source pin */\n")
        self.stack.enter_context(patch.object(run_legacy_matrix_v4, "source_identity",
                                             side_effect=self.legacy_identity))
        return run_targets_v4.historical_files(self.root / "reviews/v3")

    def legacy_identity(self):
        digest = sha(self.root / "curriculum/example.md")
        return {"sha256": digest, "files": {"curriculum/example.md": digest}}

    def fresh_legacy_runner(self, command, **kwargs):
        suffix = "lab_matrix_" + "_".join(run_legacy_matrix_v4.FAMILIES) + ".json"
        dump(self.root / "reviews/v3" / suffix,
             {"passed": True, "status": "passed", "input_sha256": self.legacy_identity()["sha256"],
              "families": [{"family": family, "passed": True}
                           for family in run_legacy_matrix_v4.FAMILIES]})
        for family in run_legacy_matrix_v4.FAMILIES:
            dump(self.root / "reviews/v3" / ("lab_" + family + ".json"),
                 {"passed": True, "status": "passed", "fresh": True})
        return subprocess.CompletedProcess(command, 0)

    def assert_failed_legacy_and_preserved(self, before):
        record = read_json(self.root / self.RECEIPTS / "legacy_matrix.json")
        self.assertEqual(record["status"], "failed")
        self.assertIs(record["passed"], False)
        self.assertNotIn("details", record)
        self.assertEqual(run_targets_v4.historical_files(self.root / "reviews/v3"), before)

    def test_legacy_nonzero_exit_cannot_relabel_historical_success(self):
        before = self.prepare_legacy_wrapper()
        with patch.object(run_legacy_matrix_v4.subprocess, "run",
                          return_value=subprocess.CompletedProcess([], 9)):
            with self.assertRaisesRegex(ValueError, "runner exited 9"):
                self.LEGACY.main()
        self.assert_failed_legacy_and_preserved(before)

    def test_legacy_zero_exit_without_new_receipts_rejects(self):
        before = self.prepare_legacy_wrapper()
        with patch.object(run_legacy_matrix_v4.subprocess, "run",
                          return_value=subprocess.CompletedProcess([], 0)):
            with self.assertRaisesRegex(ValueError, "omitted new receipts"):
                self.LEGACY.main()
        self.assert_failed_legacy_and_preserved(before)

    def test_legacy_source_mutation_prevents_success_receipt(self):
        before = self.prepare_legacy_wrapper()

        def mutate(command, **kwargs):
            result = self.fresh_legacy_runner(command, **kwargs)
            self.write("curriculum/example.md", "changed while runner executed\n")
            return result

        with patch.object(run_legacy_matrix_v4.subprocess, "run", side_effect=mutate):
            with self.assertRaisesRegex(ValueError, "Source changed"):
                self.LEGACY.main()
        self.assert_failed_legacy_and_preserved(before)

    def test_legacy_fresh_success_is_pinned_and_preserves_history(self):
        before = self.prepare_legacy_wrapper()
        with patch.object(run_legacy_matrix_v4.subprocess, "run", side_effect=self.fresh_legacy_runner):
            self.LEGACY.main()
        self.assertEqual(run_targets_v4.historical_files(self.root / "reviews/v3"), before)
        record = read_json(self.root / self.RECEIPTS / "legacy_matrix.json")
        self.assertIs(record["passed"], True)
        self.assertEqual(record["input_sha256"], self.legacy_identity()["sha256"])
        self.assertEqual(set(record["details"]), set(run_legacy_matrix_v4.FAMILIES))
        self.assertTrue(all(detail["fresh"] for detail in record["details"].values()))
        self.assertNotIn("old", record)
        for relative, digest in record["inspected_paths"].items():
            self.assertEqual(sha(self.root / relative), digest)


def main(case=ReleaseGuards):
    result = unittest.TextTestRunner(verbosity=2).run(
        unittest.defaultTestLoader.loadTestsFromTestCase(case))
    if not result.wasSuccessful():
        raise SystemExit(1)
    print(f"{result.testsRun} isolated v4 release guards passed; {len(result.skipped)} platform skips.")


if __name__ == "__main__":
    main()

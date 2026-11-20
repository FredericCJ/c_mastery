"""Regression checks for the counting-evidence boundary. No measurement is relabelled."""
import copy
import json
from pathlib import Path
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "labs/binary_inspection"))
import inspect_lab
import inspection_checks as checks


class CountTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.path = ROOT / "reviews/v5/expansion/main_integration/pmu/layout-count.json"
        cls.capture = json.loads(cls.path.read_text())

    def test_original_test_run_has_an_intact_raw_stream(self):
        self.assertEqual(self.capture["provenance"], "TEST IMPLEMENTATION")
        checks.verify_count_stream(self.capture)

    def test_homogeneous_kernel_default_allows_unpinned_affinity(self):
        synthetic = copy.deepcopy(self.capture)
        synthetic["host"]["affinity"] = [2, 4]
        synthetic["pmu_domain"] = {"cpu": None, "name": "kernel generic hardware PMU", "type": None,
                                   "generic_event_encoding": "kernel default"}
        checks.verify_count_stream(synthetic)

    def test_test_capture_is_refused_as_learner_work(self):
        row = {"report": self.path.relative_to(ROOT), "report_sha256": inspect_lab.digest(self.path),
               "pairs": 31, "interpretation": "supported"}
        with patch.object(inspect_lab, "LAB", ROOT), patch.object(checks, "LAB", ROOT):
            with self.assertRaisesRegex(ValueError, "provenance"):
                checks.pmu_result(row, None, live=True)

    def test_counters_must_match_the_output_of_this_invocation(self):
        damaged = copy.deepcopy(self.capture)
        damaged["rows"][-1]["raw_instructions"] += 1
        with self.assertRaisesRegex(ValueError, "original collector output"):
            checks.verify_count_stream(damaged)
        damaged = copy.deepcopy(self.capture)
        damaged["commands"][0]["argv"][3] = 1
        with self.assertRaisesRegex(ValueError, "argv"):
            checks.verify_count_stream(damaged)

    def test_hardware_scope_and_migration_are_checked_even_during_calibration(self):
        for field, value in (("scope", "software"), ("cpu_after", 17), ("cycle_id", 0)):
            with self.subTest(field=field):
                damaged = copy.deepcopy(self.capture)
                trial = damaged["calibration"]["attempts"][0]["trials"][0]
                trial[field] = value
                damaged["commands"][0]["stdout"] = json.dumps(trial)
                with self.assertRaises(ValueError):
                    checks.verify_count_stream(damaged)


class ManifestTests(unittest.TestCase):
    def test_current_workspace_sources_and_image_are_bound(self):
        # Synthetic manifest-only fixture. No host report or assessor implementation
        # is involved in this check. The original TEST capture is only read above.
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            labs = root / "labs"
            pmu = labs / "performance_followthrough"
            pmu.mkdir(parents=True)
            adapter = pmu / "adapter.c"
            header = pmu / "pmu_domain.h"
            for f in (adapter, header):
                f.write_text("/* unit fixture */\n")
            learner, image = root / "own.c", root / "collector"
            learner.write_text("/* authored test component */\n")
            image.write_bytes(b"unit fixture")
            supplied = [{"file": f.relative_to(labs).as_posix(), "sha256": inspect_lab.digest(f), "label": "SUPPLIED"}
                        for f in (adapter, header)]
            build = {"binary_sha256": inspect_lab.digest(image), "flags": ["-O2"], "components": [
                {"role": "pmu", "path": str(learner), "label": "LEARNER", "sha256": inspect_lab.digest(learner)}]}
            timestamp = "2026-11-20T00:00:00+00:00"
            manifest = {"kind": "native-build", "instrumented": False, "provenance": "LEARNER", **build,
                        "binary": str(image), "supplied_adapters": supplied, "created_utc": timestamp,
                        "families": [{"family": "layout", "available": True}]}
            (root / "build.json").write_text(json.dumps(manifest))
            doc = {"build": build, "request": {"family": "layout"}, "created_utc": timestamp}
            native = SimpleNamespace(LABS=labs, HERE=pmu, SUPPLIED_COLLECTOR=[adapter], instrumented=lambda _: [])
            with patch.object(inspect_lab, "LAB", root), patch.object(checks, "LAB", root):
                checks.verify_live_build(doc, image, native)
                adapter.write_text("/* changed adapter */\n")
                with self.assertRaisesRegex(ValueError, "sha256"):
                    checks.verify_live_build(doc, image, native)
                adapter.write_text("/* unit fixture */\n")
                learner.write_text("/* changed component */\n")
                with self.assertRaisesRegex(ValueError, "sha256"):
                    checks.verify_live_build(doc, image, native)


if __name__ == "__main__":
    unittest.main()

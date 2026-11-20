"""Author-only regression checks for the full Linux supervision evidence gate."""
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

import check_expansion_campaigns as campaigns
from common import ROOT, read_json


class SupervisionAcceptance(unittest.TestCase):
    def test_family_runner_reports_missing_guest_capability(self):
        with tempfile.TemporaryDirectory() as cache:
            result = subprocess.run(
                [sys.executable, ROOT / "tools/run_unit_campaigns_v5.py",
                 "family", "linux_supervision", "--native"], cwd=ROOT, capture_output=True,
                text=True, env={**os.environ, "CMASTERY_LINUX_GUEST_CACHE": cache}, timeout=90)
        self.assertEqual(result.returncode, 77, result.stdout + result.stderr)

    def test_aggregate_rejects_portable_stale_and_unbound_raw_observations(self):
        family = "linux_supervision"
        meta = read_json(ROOT / "labs" / family / "family.json")
        canonical = ROOT / "reviews/v5/expansion/linux_supervision/current.json"
        self.assertEqual(campaigns.candidates(family, meta), [canonical])
        with tempfile.TemporaryDirectory(dir=ROOT) as folder:
            path = Path(folder) / "current.json"
            for change, expected in (
                    ({"profile": "portable"}, "full"),
                    ({"passed": "failed"}, "successful"),
                    ({"independent_learner": "assessed"}, "assessment"),
                    ({"inputs": {}}, "stale"),
                    ({"guest_runs": {}}, "guest runs")):
                with self.subTest(change=change):
                    report = read_json(canonical)
                    if "guest_runs" in change:
                        from check_linux_supervision import inputs
                        report["inputs"] = inputs(True)
                    report.update(change)
                    path.write_text(json.dumps(report))
                    with patch.object(campaigns, "candidates", return_value=[path]):
                        row = campaigns.inspect_family(family)
                    self.assertFalse(row["passed"])
                    self.assertIn(expected, " ".join(row["errors"]))


if __name__ == "__main__":
    unittest.main()

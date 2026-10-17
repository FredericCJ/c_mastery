"""Negative controls for immutable graph rows and route-prefix preservation.

All generation occurs under temporary roots; no accepted catalog is rewritten.
"""
from contextlib import redirect_stdout
from copy import deepcopy
from io import StringIO
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import integrate_v2 as legacy
import integrate_v5 as current
import v5_common


class GraphPreservation(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="c mastery graph preservation ")
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / "curriculum").mkdir()
        (self.root / "labs").mkdir()
        (self.root / "curriculum/integration_families.json").write_text('{"families": []}', encoding="utf-8")
        self.baseline = self.generate(["P1", "P2"])
        self.preserved = {row["id"]: deepcopy(row) for row in self.baseline["projects"]}
        self.before = (self.root / "curriculum/project_graph.json").read_bytes()

    def loaded(self, ids):
        taxonomy = [{"id": "T01", "subtopics": [{"id": "T01.01", "objectives": [{"id": "T01.01.a"}]}]}]
        modules = {"T01": {"meta": {"projects": list(ids), "prerequisites": []}}}
        projects = {key: {"meta": {"module": "T01", "topics": ["T01.01"], "requires_projects": [],
                    "title": key, "status": "core", "hours": 1, "fixtures": []}} for key in ids}
        return taxonomy, modules, projects, []

    def generate(self, ids, *, loaded=None, preserved=None, order=None):
        with patch.object(legacy, "ROOT", self.root), patch.object(legacy, "load", return_value=loaded or self.loaded(ids)), redirect_stdout(StringIO()):
            legacy.graph(preserved=preserved, preserved_order=order)
        return json.loads((self.root / "curriculum/project_graph.json").read_text(encoding="utf-8"))

    def assert_unchanged(self):
        self.assertEqual((self.root / "curriculum/project_graph.json").read_bytes(), self.before)

    def test_new_laboratory_discovery_restores_exact_accepted_rows(self):
        folder = self.root / "labs/new_family"
        folder.mkdir()
        (folder / "family.json").write_text(json.dumps({"projects": [{"id": "P1", "tests": ["new.test"]}]}), encoding="utf-8")
        graph = self.generate(["P1", "P2"], preserved=self.preserved, order=self.baseline["order"])
        self.assertEqual(graph["projects"], self.baseline["projects"])
        self.assertEqual(graph["order"], self.baseline["order"])
        directory = (self.root / "curriculum/guides/executable-evidence.md").read_text(encoding="utf-8")
        self.assertNotIn("new_family", directory)

    def test_evidence_directory_retains_new_project_adapters(self):
        folder = self.root / "labs/new_family"
        folder.mkdir()
        (folder / "family.json").write_text(json.dumps({"projects": [{"id": "A0", "tests": ["new.test"]}, {"id": "P1", "tests": ["old.reassigned"]}]}), encoding="utf-8")
        self.generate(["A0", "P1", "P2"], preserved=self.preserved, order=self.baseline["order"])
        directory = (self.root / "curriculum/guides/executable-evidence.md").read_text(encoding="utf-8")
        self.assertIn("new.test", directory)
        self.assertNotIn("old.reassigned", directory)

    def test_missing_trailing_accepted_project_rejected_before_write(self):
        with self.assertRaisesRegex(ValueError, "disappeared"):
            self.generate(["P1"], preserved=self.preserved, order=self.baseline["order"])
        self.assert_unchanged()

    def test_missing_nontrailing_accepted_project_rejected_before_write(self):
        with self.assertRaisesRegex(ValueError, "disappeared"):
            self.generate(["P2"], preserved=self.preserved, order=self.baseline["order"])
        self.assert_unchanged()

    def test_nonlaboratory_semantic_drift_rejected_before_write(self):
        for field, changed in (("title", "Rewritten accepted title"), ("hours", 999), ("requires_projects", ["P2"])):
            loaded = self.loaded(["P1", "P2"])
            loaded[2]["P1"]["meta"][field] = changed
            with self.subTest(field=field), self.assertRaisesRegex(ValueError, "semantics changed"):
                self.generate(["P1", "P2"], loaded=loaded, preserved=self.preserved, order=self.baseline["order"])
            self.assert_unchanged()

    def test_missing_none_valued_contract_field_is_not_equal(self):
        preserved = deepcopy(self.preserved)
        preserved["P1"]["accepted_nullable_field"] = None
        with self.assertRaisesRegex(ValueError, "semantics changed"):
            self.generate(["P1", "P2"], preserved=preserved, order=self.baseline["order"])
        self.assert_unchanged()

    def test_new_project_is_appended_after_complete_accepted_prefix(self):
        graph = self.generate(["A0", "P1", "P2"], preserved=self.preserved, order=self.baseline["order"])
        self.assertEqual(graph["order"], ["P1", "P2", "A0"])
        self.assertEqual({row["id"]: row for row in graph["projects"] if row["id"] in self.preserved}, self.preserved)

    def test_duplicate_or_incomplete_preserved_route_rejected(self):
        for order in (["P1", "P1"], ["P1"], ["P1", "P2", "UNKNOWN"]):
            with self.subTest(order=order), self.assertRaisesRegex(ValueError, "exactly once"):
                self.generate(["P1", "P2"], preserved=self.preserved, order=order)
            self.assert_unchanged()

    def test_unready_preserved_route_rejected(self):
        loaded = self.loaded(["P1", "P2"])
        loaded[2]["P2"]["meta"]["requires_projects"] = ["P1"]
        generated = self.generate(["P1", "P2"], loaded=loaded)
        preserved = {row["id"]: row for row in generated["projects"]}
        before = (self.root / "curriculum/project_graph.json").read_bytes()
        with self.assertRaisesRegex(ValueError, "no longer ready"):
            self.generate(["P1", "P2"], loaded=loaded, preserved=preserved, order=["P2", "P1"])
        self.assertEqual((self.root / "curriculum/project_graph.json").read_bytes(), before)

    def test_v5_input_hash_guard_rejects_drift_before_generation(self):
        accepted = {"project_graph": {key: v5_common.digest(row) for key, row in self.preserved.items()}, "project_order": self.baseline["order"]}
        for mutation in ("changed", "missing"):
            incoming = deepcopy(self.baseline)
            if mutation == "changed":
                incoming["projects"][0]["lab_family"] = "unauthorized-reassignment"
            else:
                incoming["projects"].pop()
            with patch.object(current, "read_json", return_value=incoming), patch.object(v5_common, "baseline", return_value=accepted), patch.object(current, "generate_graph") as generator:
                with self.subTest(mutation=mutation), self.assertRaisesRegex(ValueError, "input changed"):
                    current.graph()
                generator.assert_not_called()

    def test_v5_verified_input_passes_exact_records_and_order(self):
        accepted = {"project_graph": {key: v5_common.digest(row) for key, row in self.preserved.items()}, "project_order": self.baseline["order"]}
        with patch.object(current, "read_json", return_value=self.baseline), patch.object(v5_common, "baseline", return_value=accepted), patch.object(current, "generate_graph") as generator:
            current.graph()
            generator.assert_called_once_with(preserved=self.preserved, preserved_order=self.baseline["order"])


if __name__ == "__main__":
    unittest.main(verbosity=2)

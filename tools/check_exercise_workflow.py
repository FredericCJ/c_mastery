"""Exercise workflow contract guards; isolated fixtures never claim learner mastery.

The compile round trip runs only where GCC, CMake and Ninja are available.
Run under Linux/WSL for the real stub rejection and successful implementation.
"""
from contextlib import redirect_stdout, redirect_stderr
from copy import deepcopy
from datetime import timedelta
from io import StringIO
import json
from pathlib import Path
import shutil
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from cmastery import cli, exercises as ex, learning


class ExerciseWorkflow(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix="c mastery exercise ")
        self.base = Path(self.tmp.name)
        self.root = self.base / "repository"
        self.profile = self.base / "study"
        self.workspace = self.base / "learner"
        self.patch = patch.object(cli, "ROOT", self.root)
        self.patch.start()
        self.addCleanup(self.patch.stop)
        self.addCleanup(self.tmp.cleanup)
        self.value = {
            "schema_version": 1, "id": "X-T01-001", "title": "One admission boundary",
            "project": "PR-T01-01", "domain": "T01", "objectives": ["O1"], "families": ["B", "C", "F", "J"],
            "prerequisite_exercises": [], "prerequisite_artifacts": [], "profile": "hosted",
            "environment": {"language": "C17", "target": "host", "toolchain": "GCC/Clang", "dependencies": "none"},
            "source_records": [{"id": "SRC-ONE", "path": "research/sources.json", "passages": ["Section 1"]}],
            "lab": {"family": "sample", "scaffold": "labs/sample", "tests": ["sample.boundary"]},
            "assessor_analysis": "assessor/sample/analysis.md", "required": True, "fixtures": [], "shared_contracts": [],
            "status": {"content": "authored", "execution": "not-run", "independent_review": "not-run", "limitations": ["Test fixture"]},
            "objective_coverage": [{"objective": "O1", "teaching": ["#problem"], "implementation": ["D1"],
                "boundary": {"applicable": True, "evidence": ["D1"]}, "independent_assessment": ["C1"]}],
            "rubric": [{"id": "C1", "objective": "O1", "criterion": "Explain and reject the boundary", "levels": {
                "0": "Absent", "1": "Example only", "2": "Invariant and boundary justified", "3": "Transfers under changed contract"}}]}
        cli.save(self.root / "curriculum/edition.json", {"schema_version": 5, "catalog_dir": "curriculum/v5"})
        cli.save(self.root / "curriculum/v5/units.json", {"units": [{"id": "U-ONE", "project": "PR-T01-01", "title": "Baseline", "hours": 1, "objectives": ["O1"], "requires": [], "recall_sets": []}], "order": ["U-ONE"]})
        cli.save(self.root / "curriculum/v5/recall.json", {"sets": [], "policy": {}})
        cli.save(self.root / "curriculum/v5/artifacts.json", {"artifacts": []})
        cli.save(self.root / "curriculum/project_graph.json", {"projects": [{"id": "PR-T01-01", "title": "Baseline", "module": "T01", "fixtures": []}]})
        cli.save(self.root / "research/sources.json", {"sources": [{"id": "SRC-ONE", "inspection": {"date": "2026-01-01", "method": "fixture", "scope": "Section 1", "limits": "Synthetic test record"}, "locators": ["Section 1"]}]})
        self.write("curriculum/projects/PR-T01-01.md", "# Baseline project\n")
        self.write("curriculum/guides/component-contracts.md", "# Component contracts\n")
        self.write("assessor/sample/analysis.md", "WITHHELD-SOLUTION-MARKER\n")
        self.write("labs/sample/src/answer.c", "int admission(int n) { (void)n; return 0; }\n")
        self.write("labs/sample/tests/test.c", "int admission(int); int main(void) { return admission(1) == 1 && admission(-1) == 0 ? 0 : 1; }\n")
        self.write("labs/sample/CMakeLists.txt", "cmake_minimum_required(VERSION 3.16)\nproject(sample C)\nenable_testing()\nadd_executable(check_boundary src/answer.c tests/test.c)\nadd_test(NAME sample.boundary COMMAND check_boundary)\n")
        cli.save(self.root / "labs/sample/family.json", {"family": "sample", "projects": [], "units": [], "tests": ["sample.boundary"], "exercises": [{"id": "X-T01-001", "project": "PR-T01-01", "tests": ["sample.boundary"]}]})
        self.publish()
        learning.study_init(self.profile)
        cli.instantiate(self.value["project"], self.workspace, exercise_id=self.value["id"])

    def write(self, name, text):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")

    def publish(self):
        body = "# One admission boundary\n\n" + "\n".join("## " + heading + "\n\nD1: Specific boundary, invariant, deliverable and evidence for this fixture.\n" for heading in ex.HEADINGS)
        if self.value.get("delayed_transfer"):
            body = body.replace("## Variants\n", "## Variants\n\n" + ", ".join(self.value["delayed_transfer"]["variant_ids"]) + "\n")
        self.write("curriculum/v5/exercises/" + self.value["id"] + ".md", "```json\n" + json.dumps(self.value, indent=2) + "\n```\n\n" + body)
        cli.save(self.root / "curriculum/v5/exercises.json", ex.build_catalog())

    def ready_assessment(self):
        value = ex.exercise(self.value["id"])
        assessment = ex.assessment_template(value)
        assessment.update(explanation="Admission precondition and rejected state explained", transfer="Negative input tested under changed admission contract", transfer_variant="changed-sign")
        assessment["independence"] = {"unaided": True, "assistance": []}
        assessment["dimensions"] = {key: 3 if i < 2 else 2 for i, key in enumerate(learning.DIMENSIONS)}
        (self.workspace / "evidence").mkdir(exist_ok=True)
        (self.workspace / "evidence/reasoning.md").write_text("Test fixture analysis; not a real learner assessment.\n", encoding="utf-8")
        assessment["evidence"] = ["evidence/reasoning.md"]
        for row in assessment["criteria"]:
            row.update(score=2, reason="Boundary argument and changed condition", evidence=["evidence/reasoning.md"])
        self.assessment_path = self.base / "assessment.json"
        cli.save(self.assessment_path, assessment)
        ex.studied(self.profile, value["id"])
        return assessment

    def fixture_receipt(self, **changes):
        """Synthetic runner record solely for negative guards, never reported as execution."""
        value = ex.exercise(self.value["id"])
        record = {"project": value["project"], "exercise": value["id"], "exercise_contract": learning.digest(value),
                  "exercise_harness": ex.harness(value), "source": cli.snapshot(self.workspace), "assessment": "learner",
                  "status": "passed", "environment": {"test_fixture": True}, "component_links": {},
                  "steps": [{"exit_code": 0} for _ in range(3)], "tests": [{"name": "sample.boundary", "failed": False, "skipped": False}]}
        record.update(changes)
        cli.save_check_receipt(self.workspace, record)
        return record

    def test_init_separates_answers_and_status(self):
        text = "\n".join(path.read_text(encoding="utf-8") for path in self.workspace.rglob("*") if path.is_file())
        self.assertNotIn("WITHHELD-SOLUTION-MARKER", text)
        self.assertNotIn("assessor", {part for path in self.workspace.rglob("*") for part in path.relative_to(self.workspace).parts})
        meta = cli.read(self.workspace / ".cmastery/project.json")
        self.assertEqual(meta["exercise"], self.value["id"])
        self.assertNotIn("unit", meta)
        state = learning.study_status(self.profile)
        self.assertFalse(state["expansion_completion"]["complete"])
        self.assertFalse(state["comprehensive_complete"])
        self.assertEqual(state["accepted"], [])

    def test_cli_sheet_and_study_next_round_trip(self):
        output, template = self.base / "sheet.md", self.base / "template.json"
        with redirect_stdout(StringIO()):
            self.assertEqual(cli.main(["study", "sheet", str(self.profile), str(output), "--exercise", self.value["id"], "--assessment-template", str(template)]), 0)
            self.assertEqual(cli.main(["study", "next", str(self.profile), "--studied-exercise", self.value["id"]]), 0)
        self.assertNotIn("WITHHELD-SOLUTION-MARKER", output.read_text(encoding="utf-8"))
        self.assertFalse(cli.read(template)["independence"]["unaided"])
        self.assertEqual(ex.progress(self.profile)["studied"], [self.value["id"]])
        with self.assertRaises(ValueError):
            ex.acceptance(self.profile, self.workspace, self.value["id"], template)

    def test_mutually_exclusive_selection(self):
        for args in (["init", self.value["project"], str(self.base / "another")], ["check", str(self.workspace)],
                     ["study", "sheet", str(self.profile), str(self.base / "sheet")], ["accept", str(self.profile), str(self.workspace), "--assessment", "anything"]):
            with redirect_stderr(StringIO()), self.assertRaises(SystemExit) as error:
                cli.main(args + ["--unit", "U-ONE", "--exercise", self.value["id"]])
            self.assertEqual(error.exception.code, 2)

    def test_changed_contract_and_stale_generation_rejected(self):
        path = self.root / "curriculum/v5/exercises" / (self.value["id"] + ".md")
        path.write_text(path.read_text(encoding="utf-8") + "\nNew obligation.\n", encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "stale"):
            ex.exercise(self.value["id"])
        self.value["title"] = "Changed contract"
        self.publish()
        with self.assertRaisesRegex(ValueError, "contract changed"):
            ex.workspace_contract(self.workspace)

    def test_learner_cannot_rewrite_trusted_public_harness(self):
        (self.workspace / "tests/test.c").write_text("int main(void) { return 0; }\n", encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "harness changed"):
            cli.run_checks(self.workspace, "gcc", "debug", exercise_id=self.value["id"])

    def test_path_leaks_and_cycles_rejected(self):
        for name in ("../outside", "/absolute", "C:/absolute", "assessor/sample/analysis.md", "labs/../assessor/sample/analysis.md"):
            with self.assertRaises(ValueError):
                ex.path_in_root(name)
        other = deepcopy(self.value)
        other["id"] = "X-T01-002"
        other["prerequisite_exercises"] = [self.value["id"]]
        first = dict(self.value, prerequisite_exercises=[other["id"]])
        with self.assertRaisesRegex(ValueError, "cycle"):
            ex.ordered([first, other])
        with self.assertRaisesRegex(ValueError, "Unknown prerequisite"):
            ex.ordered([first])

    def test_unassessed_leaf_and_missing_specification_rejected(self):
        value, body = ex.parse_brief(self.root / "curriculum/v5/exercises" / (self.value["id"] + ".md"))
        bad = deepcopy(value)
        bad["objective_coverage"][0]["independent_assessment"] = ["nonexistent"]
        with self.assertRaisesRegex(ValueError, "objective-specific"):
            ex.validate(bad, body)
        with self.assertRaisesRegex(ValueError, "Prerequisites"):
            ex.validate(value, body.replace("## Prerequisites", "## Removed"))

    def test_reference_ids_inspections_and_exact_passages_required(self):
        value, body = ex.parse_brief(self.root / "curriculum/v5/exercises" / (self.value["id"] + ".md"))
        for field, replacement, message in (("id", "UNREAD", "unique inspected"), ("passages", ["Uninspected section"], "outside registered")):
            bad = deepcopy(value)
            bad["source_records"][0][field] = replacement
            with self.assertRaisesRegex(ValueError, message):
                ex.validate(bad, body)
        cli.save(self.root / "research/sources.json", {"sources": [{"id": "SRC-ONE", "locators": ["Section 1"]}]})
        with self.assertRaisesRegex(ValueError, "unique inspected"):
            ex.validate(value, body)
        for inspection in ({"status": "unavailable"}, {"status": "not-read", "date": "2026-01-01", "method": "fixture", "scope": "Section 1", "limits": "Fixture"}):
            cli.save(self.root / "research/sources.json", {"sources": [{"id": "SRC-ONE", "locators": ["Section 1"], "inspection": inspection}]})
            with self.assertRaises(ValueError):
                ex.validate(value, body)

    def test_dangling_teaching_or_deliverable_mapping_rejected(self):
        value, body = ex.parse_brief(self.root / "curriculum/v5/exercises" / (self.value["id"] + ".md"))
        for key, replacement in (("teaching", ["#missing"]), ("implementation", ["D99"])):
            bad = deepcopy(value)
            bad["objective_coverage"][0][key] = replacement
            with self.assertRaisesRegex(ValueError, "missing teaching anchors or deliverables"):
                ex.validate(bad, body)

    def test_studied_baseline_unit_does_not_satisfy_accepted_prerequisite(self):
        self.value["prerequisite_units"] = ["U-ONE"]
        self.publish()
        self.workspace = self.base / "baseline-dependent-learner"
        cli.instantiate(self.value["project"], self.workspace, exercise_id=self.value["id"])
        self.ready_assessment()
        learning.mark_studied(self.profile, "U-ONE")
        self.fixture_receipt()
        with self.assertRaisesRegex(ValueError, "accepted baseline unit"):
            ex.acceptance(self.profile, self.workspace, self.value["id"], self.assessment_path)

    def test_delayed_variant_timing_and_retained_study_binding(self):
        self.value["delayed_transfer"] = {"minimum_days": 7, "variant_ids": ["DELAYED-ONE"]}
        self.publish()
        self.workspace = self.base / "delayed-learner"
        cli.instantiate(self.value["project"], self.workspace, exercise_id=self.value["id"])
        assessment = self.ready_assessment()
        self.fixture_receipt()
        with self.assertRaisesRegex(ValueError, "Delayed independent transfer pending"):
            ex.acceptance(self.profile, self.workspace, self.value["id"], self.assessment_path)
        later = learning.clock() + timedelta(days=8)
        with patch.object(learning, "clock", return_value=later):
            with self.assertRaisesRegex(ValueError, "declared delayed"):
                ex.acceptance(self.profile, self.workspace, self.value["id"], self.assessment_path)
            assessment["transfer_variant"] = "DELAYED-ONE"
            cli.save(self.assessment_path, assessment)
            self.assertEqual(ex.acceptance(self.profile, self.workspace, self.value["id"], self.assessment_path)["status"], "accepted")
            state = ex.accepted(self.profile)[self.value["id"]]
            self.assertTrue(state["historical_valid"])
            recent = ex.studied(self.profile, self.value["id"])
            forged = {key: value for key, value in state["record"].items() if key not in {"schema_version", "id", "at", "kind", "previous", "sha256"}}
            forged["study_event"] = recent["id"]
            learning.append(self.profile, "exercise-accepted", **forged)
            self.assertFalse(ex.accepted(self.profile)[self.value["id"]]["historical_valid"])

    def test_pending_batches_prevent_comprehensive_completion(self):
        self.ready_assessment()
        self.fixture_receipt()
        ex.acceptance(self.profile, self.workspace, self.value["id"], self.assessment_path)
        manifest = cli.read(self.root / "curriculum/edition.json")
        manifest["exercise_expansion"] = {"required_exercises": 1, "remaining_work": "curriculum/v5/expansion/remaining.json"}
        cli.save(self.root / "curriculum/edition.json", manifest)
        cli.save(self.root / "curriculum/v5/expansion/remaining.json", {"batches": [{"id": "B01", "status": "authored"}, {"id": "B02", "status": "planned-not-authored"}]})
        state = ex.progress(self.profile)
        self.assertTrue(state["current_exercises_complete"])
        self.assertFalse(state["complete"])
        self.assertEqual(state["pending_batches"], ["B02"])

    def test_test_receipt_alone_or_assessor_receipt_cannot_accept(self):
        assessment = self.ready_assessment()
        self.fixture_receipt(assessment="assessor-validation")
        with self.assertRaisesRegex(ValueError, "learner check"):
            ex.acceptance(self.profile, self.workspace, self.value["id"], self.assessment_path)
        self.fixture_receipt()
        assessment["criteria"] = []
        cli.save(self.assessment_path, assessment)
        with self.assertRaisesRegex(ValueError, "every objective-specific"):
            ex.acceptance(self.profile, self.workspace, self.value["id"], self.assessment_path)

    def test_forged_incomplete_receipt_and_escaping_evidence_rejected(self):
        assessment = self.ready_assessment()
        self.fixture_receipt(steps=[], tests=[])
        with self.assertRaisesRegex(ValueError, "learner check"):
            ex.acceptance(self.profile, self.workspace, self.value["id"], self.assessment_path)
        self.fixture_receipt()
        assessment["criteria"][0]["evidence"] = ["../assessment.json"]
        cli.save(self.assessment_path, assessment)
        with self.assertRaisesRegex(ValueError, "within"):
            ex.acceptance(self.profile, self.workspace, self.value["id"], self.assessment_path)

    def test_source_change_stales_receipt(self):
        self.ready_assessment()
        self.fixture_receipt()
        (self.workspace / "src/answer.c").write_text("int admission(int n) { return n > 0; }\n", encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "learner check"):
            ex.acceptance(self.profile, self.workspace, self.value["id"], self.assessment_path)

    def test_missing_prerequisite_and_artifact_rejected(self):
        self.value["prerequisite_exercises"] = ["X-T01-002"]
        with self.assertRaisesRegex(ValueError, "Unknown prerequisite"):
            self.publish()
        self.value["prerequisite_exercises"] = []
        self.value["prerequisite_artifacts"] = ["missing.component"]
        with self.assertRaisesRegex(ValueError, "Unknown prerequisite artifact"):
            self.publish()

    @unittest.skipUnless(all(shutil.which(x) for x in ("gcc", "cmake", "ninja")), "Linux/WSL GCC+CMake+Ninja needed for executed round trip")
    def test_real_stub_rejection_implementation_acceptance_and_history(self):
        self.ready_assessment()
        failed = cli.run_checks(self.workspace, "gcc", "debug", exercise_id=self.value["id"])
        self.assertEqual(failed["status"], "failed", failed)
        self.assertEqual(len(failed["steps"]), 3, "Stub must fail semantically, after successful compilation")
        self.assertEqual(failed["steps"][1]["exit_code"], 0)
        with self.assertRaisesRegex(ValueError, "learner check"):
            ex.acceptance(self.profile, self.workspace, self.value["id"], self.assessment_path)
        (self.workspace / "src/answer.c").write_text("int admission(int n) { return n > 0; }\n", encoding="utf-8")
        passed = cli.run_checks(self.workspace, "gcc", "release", exercise_id=self.value["id"])
        self.assertEqual(passed["status"], "passed", passed)
        result = ex.acceptance(self.profile, self.workspace, self.value["id"], self.assessment_path)
        self.assertEqual(result["status"], "accepted")
        self.assertTrue(ex.progress(self.profile)["complete"])
        self.assertFalse(learning.study_status(self.profile)["comprehensive_complete"])
        self.assertEqual(learning.study_status(self.profile)["accepted"], [])
        (self.workspace / "src/answer.c").write_text("int admission(int n) { return n >= 0; }\n", encoding="utf-8")
        state = ex.accepted(self.profile)[self.value["id"]]
        self.assertTrue(state["historical_valid"])
        self.assertFalse(state["valid"])
        record = dict(state["record"])
        for key in ("schema_version", "id", "at", "kind", "previous", "sha256"):
            record.pop(key)
        record["assessment"] = dict(record["assessment"], criteria=[])
        learning.append(self.profile, "exercise-accepted", **record)
        self.assertFalse(ex.accepted(self.profile)[self.value["id"]]["historical_valid"])


if __name__ == "__main__":
    unittest.main(verbosity=2)

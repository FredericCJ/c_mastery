"""Behavioral checks for the v4 offline learning and component workflow.

Fixtures are deliberately small; no learner progress or repository catalog is
modified. Run under Linux/WSL as well to compile a relocated exported component.
"""
from datetime import datetime, timedelta, timezone
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from cmastery import cli, learning as learn


class LearningContracts(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="c mastery learning ")
        self.base = Path(self.temp.name)
        self.root = self.base / "catalog"
        self.profile = self.base / "study"
        self.workspace = self.base / "learner"
        self.workspace.mkdir()
        self.time = datetime(2026, 1, 1, tzinfo=timezone.utc)
        self.root_patch = patch.object(cli, "ROOT", self.root)
        self.clock_patch = patch.object(learn, "clock", side_effect=lambda: self.time)
        self.root_patch.start(); self.clock_patch.start()
        self.units = [{"id": "U-ONE", "project": "PR-T01-01", "title": "Checked bytes", "hours": 2,
                       "objectives": ["O1"], "requires": [], "imports": [], "exports": [],
                       "recall_sets": ["TOPIC-1"], "critical_objectives": ["O1"], "family": "sample",
                       "tests": [], "checks": ["Boundary rejected"], "claims": [{"id": "bounded", "scope": "host"}],
                       "target_scope": "host", "milestone": False, "decision_required": False},
                      {"id": "U-TWO", "project": "PR-T01-01", "title": "Consume bytes", "hours": 2,
                       "objectives": ["O2"], "requires": ["U-ONE"], "imports": [], "exports": [],
                       "recall_sets": ["TOPIC-2"], "critical_objectives": [], "family": "sample",
                       "tests": [], "checks": [], "claims": [], "milestone": False}]
        sets = [{"id": "TOPIC-" + str(i), "objectives": ["O" + str(i)], "resource_ids": [],
                 "variants": [{"id": "V" + str(j), "prompt": "Reconstruct mechanism " + str(j),
                               "objectives": ["O" + str(i)], "checklist": ["Boundary explained"]}
                              for j in range(1, 4)]} for i in (1, 2)]
        cli.save(self.root / "curriculum/v4/recall.json", {"sets": sets,
                 "policy": {"gaps_days": [1, 3, 7, 14, 30], "freshness_days": 14, "session_minutes": 15}})
        self.artifacts = [{"id": "bytes.checked", "producer_unit": "U-ONE", "contract_version": "1",
                           "public_contract": "Checked addition, unchanged output on overflow", "critical_objectives": ["O1"],
                           "profiles": ["hosted", "freestanding-local"], "targets": ["host"], "consumers": ["U-TWO"]}]
        self.write_catalog()
        cli.save(self.workspace / ".cmastery/project.json", {"project": "PR-T01-01", "family": "sample",
                 "unit": "U-ONE", "source_catalog": {}})
        cli.save(self.workspace / "family.json", {"family": "sample", "projects": [{"id": "PR-T01-01", "tests": ["sample.boundary"]}]})
        (self.workspace / "src").mkdir()
        (self.workspace / "include").mkdir()
        (self.workspace / "src/bytes.c").write_text('#include "bytes.h"\nint checked_add(int a,int b){return a+b;}\n')
        (self.workspace / "include/bytes.h").write_text('int checked_add(int a,int b);\n')
        (self.workspace / "CMakeLists.txt").write_text("# test adapter\n")
        (self.workspace / "evidence").mkdir()
        (self.workspace / "evidence/unit.md").write_text("Observed rejecting boundary and changed capacity.\n")
        self.assessment = {"explanation": "Representation invariant and reject-before-write mechanism",
            "transfer": "Repeated under changed capacity", "independence": {"unaided": True, "assistance": []},
            "checks": [{"check": "Boundary rejected", "met": True}], "evidence": ["evidence/unit.md"],
            "claims": [{"id": "bounded", "scope": "host", "evidence": ["evidence/unit.md"]}],
            "dimensions": {k: 3 if i < 2 else 2 for i, k in enumerate(learn.DIMENSIONS)}}
        self.assessment_file = self.base / "assessment.json"
        self.answer = self.base / "answer.txt"
        self.answer.write_text("My unaided original reasoning")
        self.grade = self.base / "grade.json"
        self.declaration = self.base / "export.json"
        cli.save(self.declaration, {"contract_version": "1", "cmake_target": "checked_bytes",
            "profiles": ["hosted"], "targets": ["host"], "sources": ["src/bytes.c"],
            "headers": ["include/bytes.h"], "include_dirs": ["include"], "dependencies": []})
        learn.study_init(self.profile)

    def tearDown(self):
        self.root_patch.stop(); self.clock_patch.stop(); self.temp.cleanup()

    def write_catalog(self):
        cli.save(self.root / "curriculum/v4/units.json", {"units": self.units, "order": [u["id"] for u in self.units]})
        cli.save(self.root / "curriculum/v4/artifacts.json", {"artifacts": self.artifacts})

    def review(self, key="TOPIC-1", score=2, assistance=None, contract=None):
        attempt = learn.recall_start(self.profile, key, contract)["attempt"]
        submitted = learn.recall_submit(self.profile, attempt, self.answer, assistance)
        cli.save(self.grade, {"scores": {o: score for o in submitted["objectives"]}, "feedback": "Reasoning checked; corrected the boundary example"})
        learn.recall_grade(self.profile, attempt, self.grade)
        return attempt

    def accept_one(self, export=False):
        if export:
            self.units[0]["exports"] = ["bytes.checked"]
            self.write_catalog()
        learn.mark_studied(self.profile, "U-ONE")
        self.review()
        cli.save(self.assessment_file, self.assessment)
        return learn.acceptance(self.profile, self.workspace, "U-ONE", self.assessment_file)

    def export_one(self):
        self.accept_one(export=True)
        bundle = self.base / "bundle"
        learn.artifact_export(self.profile, self.workspace, "bytes.checked", bundle, self.declaration)
        return bundle

    def test_no_recall_activated_before_study(self):
        self.assertEqual(learn.recall_states(self.profile), {})
        with self.assertRaisesRegex(ValueError, "Study"):
            learn.recall_start(self.profile, "TOPIC-1")
        learn.mark_studied(self.profile, "U-ONE")
        learn.mark_studied(self.profile, "U-ONE")
        self.assertEqual(len(learn.recall_states(self.profile)), 1)
        self.assertEqual(learn.due(self.profile), [])

    def test_gaps_and_maintenance_no_perpetual_queue(self):
        learn.mark_studied(self.profile, "U-ONE")
        for elapsed in (1, 3, 7, 14, 30):
            self.time += timedelta(days=elapsed)
            self.assertEqual(len(learn.due(self.profile)), 1)
            self.review()
        self.assertIsNone(learn.recall_states(self.profile)["TOPIC-1"]["due"])
        self.time += timedelta(days=100)
        self.assertEqual(learn.due(self.profile), [])
        self.assertEqual(learn.freshness(self.profile, ["O1"]), ["O1"])

    def test_early_repetitions_do_not_collapse_spacing(self):
        learn.mark_studied(self.profile, "U-ONE")
        self.review(); self.review(); self.review()
        self.assertEqual(learn.recall_states(self.profile)["TOPIC-1"]["stage"], 0)
        self.assertEqual(learn.freshness(self.profile, ["O1"]), [])

    def test_missed_days_produce_one_opportunity_and_bounded_session(self):
        learn.mark_studied(self.profile, "U-ONE")
        learn.mark_studied(self.profile, "U-TWO")
        self.time += timedelta(days=80)
        self.assertEqual(len(learn.due(self.profile)), 2)
        self.assertEqual(len(learn.study_next(self.profile)["recall"]), 2)
        self.assertEqual(learn.study_next(self.profile)["review_minutes"], 15)

    def test_variants_rotate_and_feedback_is_hidden_until_submission(self):
        learn.mark_studied(self.profile, "U-ONE")
        first = learn.recall_start(self.profile, "TOPIC-1")
        self.assertNotIn("checklist", first)
        with self.assertRaisesRegex(ValueError, "Finish"):
            learn.recall_start(self.profile, "TOPIC-1")
        result = learn.recall_submit(self.profile, first["attempt"], self.answer)
        self.assertIn("checklist", result)
        with self.assertRaisesRegex(ValueError, "original answer"):
            learn.recall_submit(self.profile, first["attempt"], self.answer)
        cli.save(self.grade, {"scores": {"O1": 2}, "feedback": "Reviewed invariant"})
        learn.recall_grade(self.profile, first["attempt"], self.grade)
        self.assertEqual(learn.recall_start(self.profile, "TOPIC-1")["variant"], "V2")

    def test_assisted_success_requires_independent_later_attempt(self):
        learn.mark_studied(self.profile, "U-ONE")
        self.review(score=3, assistance=["Looked at source"])
        state = learn.recall_states(self.profile)["TOPIC-1"]
        self.assertEqual(state["last_result"], "correction-and-delayed-retry")
        self.assertEqual(learn.freshness(self.profile, ["O1"]), ["O1"])
        self.assertEqual(learn.instant(state["due"]), self.time + timedelta(days=1))

    def test_failed_recall_reopens_only_affected_objective(self):
        learn.mark_studied(self.profile, "U-ONE"); learn.mark_studied(self.profile, "U-TWO")
        self.review(); self.review("TOPIC-2")
        self.review(score=1)
        self.assertEqual(learn.freshness(self.profile, ["O1", "O2"]), ["O1"])

    def test_same_session_repair_does_not_establish_delayed_retention(self):
        learn.mark_studied(self.profile, "U-ONE")
        self.review(score=1)
        self.review(score=3)
        self.assertEqual(learn.freshness(self.profile, ["O1"]), ["O1"])
        self.time += timedelta(days=1)
        self.review(score=2)
        self.assertEqual(learn.freshness(self.profile, ["O1"]), [])

    def test_distinct_consumed_contracts_do_not_erase_one_another(self):
        learn.mark_studied(self.profile, "U-ONE")
        self.review(contract="bytes.checked@1")
        self.review(contract="bytes.cursor@1")
        self.assertEqual(learn.freshness(self.profile, ["O1"], "bytes.checked@1"), [])
        self.assertEqual(learn.freshness(self.profile, ["O1"], "bytes.cursor@1"), [])
        self.review(score=1)
        self.assertEqual(learn.freshness(self.profile, ["O1"], "bytes.checked@1"), ["O1"])

    def test_grade_only_prompted_objectives_and_retain_superseded_evaluation(self):
        learn.mark_studied(self.profile, "U-ONE")
        attempt = self.review()
        cli.save(self.grade, {"scores": {"O1": 3, "O2": 3}, "feedback": "No basis for O2"})
        with self.assertRaisesRegex(ValueError, "exactly"):
            learn.recall_grade(self.profile, attempt, self.grade)
        cli.save(self.grade, {"scores": {"O1": 1}, "feedback": "Corrected my overgenerous grade"})
        learn.recall_grade(self.profile, attempt, self.grade)
        evaluations = [e for e in learn.events(self.profile) if e["kind"] == "recall-graded"]
        self.assertEqual(len(evaluations), 2)
        self.assertEqual(evaluations[1]["supersedes"], evaluations[0]["id"])
        self.assertEqual(learn.freshness(self.profile, ["O1"]), ["O1"])

    def test_clock_rollback_and_edited_ledger_rejected(self):
        self.time -= timedelta(days=1)
        with self.assertRaisesRegex(ValueError, "future|backwards"):
            learn.mark_studied(self.profile, "U-ONE")
        self.time += timedelta(days=1)
        path = learn.area(self.profile) / "ledger.jsonl"
        path.write_text(path.read_text().replace("profile-created", "forged-record"))
        with self.assertRaisesRegex(ValueError, "hash chain"):
            learn.events(self.profile)

    def test_missing_recall_blocks_acceptance_but_not_study(self):
        learn.mark_studied(self.profile, "U-ONE")
        cli.save(self.assessment_file, self.assessment)
        with self.assertRaisesRegex(ValueError, "Critical recall"):
            learn.acceptance(self.profile, self.workspace, "U-ONE", self.assessment_file)
        self.assertEqual(learn.study_status(self.profile)["studied"], ["U-ONE"])

    def test_unit_acceptance_does_not_require_full_rubric(self):
        self.assessment.pop("dimensions")
        self.assertEqual(self.accept_one()["status"], "accepted")

    def test_milestone_requires_all_dimensions_and_two_transfer_scores(self):
        self.units[0]["milestone"] = True; self.write_catalog()
        self.assessment["dimensions"] = {k: 2 for k in learn.DIMENSIONS}
        with self.assertRaisesRegex(ValueError, "Milestone rubric"):
            self.accept_one()
        self.assessment["dimensions"]["mechanism"] = 3
        self.assessment["dimensions"]["communication_transfer"] = 3
        cli.save(self.assessment_file, self.assessment)
        self.assertEqual(learn.acceptance(self.profile, self.workspace, "U-ONE", self.assessment_file)["status"], "accepted")

    def test_assessor_receipt_cannot_establish_learner_acceptance(self):
        self.units[0]["tests"] = ["sample.boundary"]; self.write_catalog()
        learn.mark_studied(self.profile, "U-ONE"); self.review()
        cli.save(self.assessment_file, self.assessment)
        cli.save(self.workspace / "evidence/latest.json", {"status": "passed", "unit": "U-ONE",
            "unit_contract": learn.digest(self.units[0]), "assessment": "assessor-validation", "project": "PR-T01-01",
            "source": cli.snapshot(self.workspace), "tests": [{"name": "sample.boundary"}]})
        with self.assertRaisesRegex(ValueError, "assessor validation"):
            learn.acceptance(self.profile, self.workspace, "U-ONE", self.assessment_file)

    def test_claim_scope_and_physical_capture_are_not_interchangeable(self):
        self.units[0]["claims"] = [{"id": "bounded", "scope": "physical"}]; self.write_catalog()
        with self.assertRaisesRegex(ValueError, "scope differs"):
            self.accept_one()
        self.assessment["claims"][0]["scope"] = "physical"
        cli.save(self.assessment_file, self.assessment)
        with self.assertRaisesRegex(ValueError, "physical evidence"):
            learn.acceptance(self.profile, self.workspace, "U-ONE", self.assessment_file)

    def test_history_survives_later_work_but_claim_source_change_reopens(self):
        self.assessment["source_files"] = ["src/bytes.c"]
        self.accept_one()
        (self.workspace / "unrelated.c").write_text("/* new work */")
        self.assertEqual(learn.study_status(self.profile)["accepted"], ["U-ONE"])
        (self.workspace / "src/bytes.c").write_text("/* changed source */")
        self.assertIn("U-ONE", learn.study_status(self.profile)["reopened"])

    def test_new_contract_reopens_affected_unit_only(self):
        self.accept_one()
        self.units[1]["hours"] = 3; self.write_catalog()
        self.assertEqual(learn.study_status(self.profile)["accepted"], ["U-ONE"])
        self.units[0]["checks"].append("New boundary"); self.write_catalog()
        self.assertIn("U-ONE", learn.study_status(self.profile)["reopened"])

    def test_design_seal_needs_alternatives_experiment_and_subsequent_implementation(self):
        self.units[0]["decision_required"] = True; self.write_catalog()
        design = self.base / "decision.json"
        data = {"requirements": ["Bounded memory"], "predictions": ["Fixed storage avoids allocation"],
                "alternatives": ["caller storage", "owned storage"], "choice": "caller storage",
                "rejected_reason": "No allocator at this boundary", "reversal_trigger": "Variable sizes dominate",
                "spike_evidence": ["evidence/unit.md"]}
        cli.save(design, data)
        learn.decision_seal(self.profile, self.workspace, "U-ONE", design)
        with self.assertRaisesRegex(ValueError, "No implementation change"):
            self.accept_one()
        self.time += timedelta(minutes=10)
        (self.workspace / "src/bytes.c").write_text("/* implementation after experiment */")
        cli.save(self.assessment_file, self.assessment)
        self.assertEqual(learn.acceptance(self.profile, self.workspace, "U-ONE", self.assessment_file)["status"], "accepted")

    def test_artifact_export_is_pinned_relocatable_and_has_cmake_interface(self):
        bundle = self.export_one()
        moved = self.base / "moved"; shutil.move(str(bundle), moved)
        manifest = learn.verify_bundle(moved)
        self.assertEqual(manifest["cmake_target"], "checked_bytes")
        cmake = (moved / "CMakeLists.txt").read_text()
        self.assertNotIn(str(self.workspace), cmake)
        self.assertIn("${CMAKE_CURRENT_SOURCE_DIR}/include", cmake)
        compiler = shutil.which("cc") or shutil.which("gcc")
        if shutil.which("cmake") and compiler:
            build = self.base / "compiled"
            subprocess.run(["cmake", "-S", str(moved), "-B", str(build), "-DCMAKE_C_COMPILER=" + compiler], check=True, capture_output=True)
            subprocess.run(["cmake", "--build", str(build)], check=True, capture_output=True)

    def test_export_requires_current_producer_sources(self):
        self.accept_one(export=True)
        (self.workspace / "src/bytes.c").write_text("/* changed after accepted result */")
        with self.assertRaisesRegex(ValueError, "Producer source changed"):
            learn.artifact_export(self.profile, self.workspace, "bytes.checked", self.base / "bundle", self.declaration)

    def test_export_and_link_reject_tampered_bundle_or_unsupported_target(self):
        bundle = self.export_one()
        consumer = self.base / "consumer"; consumer.mkdir()
        cli.save(consumer / ".cmastery/project.json", {"project": "PR-T01-01", "family": "sample"})
        with self.assertRaisesRegex(ValueError, "requested"):
            learn.artifact_link(self.profile, consumer, bundle, "hosted", "physical")
        learn.artifact_link(self.profile, consumer, bundle, "hosted", "host")
        self.assertEqual(learn.linked_state(consumer)["bytes.checked"]["source_identity"], cli.snapshot(self.workspace)["sha256"])
        (bundle / "src/bytes.c").write_text("/* tampered */")
        with self.assertRaisesRegex(ValueError, "changed"):
            learn.artifact_link(self.profile, consumer, bundle, "hosted", "host")

    def test_export_path_traversal_and_assessor_paths_rejected(self):
        for value in ("../outside.c", "C:/outside.c", "assessor/solution.c", "include\\secret.h"):
            with self.assertRaises(ValueError):
                learn.safe_relative(value)
        self.accept_one(export=True)
        declaration = cli.read(self.declaration)
        declaration["sources"] = ["../answer.txt"]
        cli.save(self.declaration, declaration)
        with self.assertRaises(ValueError):
            learn.artifact_export(self.profile, self.workspace, "bytes.checked", self.base / "bundle", self.declaration)
        self.assertFalse((self.base / "bundle").exists())

    def test_consumption_requires_contract_specific_fresh_recall(self):
        bundle = self.export_one()
        self.units[1]["imports"] = ["bytes.checked"]; self.write_catalog()
        consumer = self.base / "consumer"; consumer.mkdir()
        cli.save(consumer / ".cmastery/project.json", {"project": "PR-T01-01", "family": "sample", "unit": "U-TWO", "source_catalog": {}})
        (consumer / "evidence").mkdir(); (consumer / "evidence/unit.md").write_text("Adapter observations")
        learn.artifact_link(self.profile, consumer, bundle, "hosted", "host")
        learn.mark_studied(self.profile, "U-TWO")
        assessment = dict(self.assessment, checks=[], claims=[])
        cli.save(self.assessment_file, assessment)
        with self.assertRaisesRegex(ValueError, "Consumption preflight"):
            learn.acceptance(self.profile, consumer, "U-TWO", self.assessment_file)
        self.review(contract="bytes.checked@1")
        self.assertEqual(learn.acceptance(self.profile, consumer, "U-TWO", self.assessment_file)["status"], "accepted")
        (consumer / next(iter(learn.linked(consumer).values()))["path"] / "src/bytes.c").write_text("/* altered import */")
        self.assertIn("U-TWO", learn.study_status(self.profile)["reopened"])

    def test_sheet_shares_ids_and_excludes_feedback(self):
        learn.mark_studied(self.profile, "U-ONE"); self.time += timedelta(days=1)
        output = self.base / "sheet.md"
        learn.study_sheet(self.profile, output, "U-ONE")
        text = output.read_text(encoding="utf-8")
        self.assertIn("TOPIC-1 / V1", text)
        self.assertNotIn("Boundary explained", text)
        self.assertIn("U-ONE", text)

    def test_recursive_dependency_export_builds_after_relocation(self):
        base_bundle = self.export_one()
        self.units[1]["imports"] = ["bytes.checked"]
        self.units[1]["exports"] = ["codec.records"]
        self.artifacts.append({"id": "codec.records", "producer_unit": "U-TWO", "contract_version": "1",
            "public_contract": "Record through checked bytes", "critical_objectives": [],
            "profiles": ["hosted"], "targets": ["host"], "consumers": []})
        self.write_catalog()
        consumer = self.base / "codec"; consumer.mkdir()
        cli.save(consumer / ".cmastery/project.json", {"project": "PR-T01-01", "family": "sample", "unit": "U-TWO", "source_catalog": {}})
        for directory in ("evidence", "include", "src"):
            (consumer / directory).mkdir()
        (consumer / "evidence/unit.md").write_text("Independent linked boundary experiment")
        (consumer / "include/codec.h").write_text("int encode_record(int value);\n")
        (consumer / "src/codec.c").write_text('#include "codec.h"\n#include "bytes.h"\nint encode_record(int value){return checked_add(value,1);}\n')
        learn.artifact_link(self.profile, consumer, base_bundle, "hosted", "host")
        learn.mark_studied(self.profile, "U-TWO")
        self.review(contract="bytes.checked@1")
        cli.save(self.assessment_file, dict(self.assessment, checks=[], claims=[]))
        learn.acceptance(self.profile, consumer, "U-TWO", self.assessment_file)
        declaration = {"contract_version": "1", "cmake_target": "codec_records", "profiles": ["hosted"],
            "targets": ["host"], "sources": ["src/codec.c"], "headers": ["include/codec.h"],
            "include_dirs": ["include"], "dependencies": []}
        cli.save(self.declaration, declaration)
        with self.assertRaisesRegex(ValueError, "complete imported"):
            learn.artifact_export(self.profile, consumer, "codec.records", self.base / "closure", self.declaration)
        declaration["dependencies"] = ["bytes.checked"]; cli.save(self.declaration, declaration)
        learn.artifact_export(self.profile, consumer, "codec.records", self.base / "closure", self.declaration)
        relocated = self.base / "relocated-closure"; shutil.move(str(self.base / "closure"), relocated)
        self.assertIn("bytes.checked", learn.verify_bundle(relocated)["dependencies"])
        compiler = shutil.which("cc") or shutil.which("gcc")
        if shutil.which("cmake") and compiler:
            build = self.base / "compiled-closure"
            subprocess.run(["cmake", "-S", str(relocated), "-B", str(build), "-DCMAKE_C_COMPILER=" + compiler], check=True, capture_output=True)
            subprocess.run(["cmake", "--build", str(build)], check=True, capture_output=True)
        (relocated / "dependencies/bytes.checked/src/bytes.c").write_text("/* changed dependency */")
        with self.assertRaises(ValueError):
            learn.verify_bundle(relocated)

    def test_unlisted_bundle_files_and_dependency_path_escape_are_rejected(self):
        bundle = self.export_one()
        (bundle / "extra.c").write_text("/* unlisted */")
        with self.assertRaisesRegex(ValueError, "outside"):
            learn.verify_bundle(bundle)
        (bundle / "extra.c").unlink()
        manifest = cli.read(bundle / "artifact.json")
        manifest["dependencies"] = {"../outside": {"sha256": "0" * 64}}
        manifest.pop("sha256"); manifest["sha256"] = learn.digest(manifest)
        cli.save(bundle / "artifact.json", manifest)
        with self.assertRaisesRegex(ValueError, "Invalid dependency"):
            learn.verify_bundle(bundle)

    def test_unit_check_runs_only_selected_adapter_and_records_contract(self):
        self.units[0]["tests"] = ["sample.boundary"]; self.write_catalog()
        env = {"capabilities": {k: {"available": True} for k in ("gcc", "cmake", "ninja")}}
        def invoke(command, **kwargs):
            if command[0] == "ctest":
                report = Path(command[command.index("--output-junit") + 1])
                report.parent.mkdir(parents=True, exist_ok=True)
                report.write_text('<testsuite><testcase name="sample.boundary" status="run"/></testsuite>')
            return {"command": command, "exit_code": 0, "stdout": "", "stderr": ""}
        with patch.object(cli, "doctor", return_value=env), patch.object(cli, "invoke", side_effect=invoke):
            result = cli.run_checks(self.workspace, "gcc", "debug", unit_id="U-ONE")
        self.assertEqual(result["status"], "passed")
        self.assertEqual(result["unit"], "U-ONE")
        self.assertEqual(result["unit_contract"], learn.digest(self.units[0]))
        self.assertIn("^(sample\\.boundary)$", result["steps"][-1]["command"])
        self.units[0]["checks"].append("New acceptance requirement"); self.write_catalog()
        self.assertTrue(cli.report(self.workspace)["evidence"]["stale"])

    def test_unit_selection_overrides_parent_target_tests_and_all_tests_requires_complete_scope(self):
        self.units[0]["tests"] = ["sample.boundary"]; self.write_catalog()
        family = cli.read(self.workspace / "family.json")
        family.update(default_target="model", tests=["sample.boundary", "sample.other"], targets={"model": {"tests": ["sample.boundary", "sample.other"]}})
        family["projects"][0]["target_tests"] = {"model": ["sample.parent"]}
        cli.save(self.workspace / "family.json", family)
        env = {"capabilities": {k: {"available": True} for k in ("gcc", "cmake", "ninja")}}
        def invoke(command, **kwargs):
            if command[0] == "ctest":
                path = Path(command[command.index("--output-junit") + 1]); path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text('<testsuite><testcase name="sample.boundary" status="run"/></testsuite>')
            return {"command": command, "exit_code": 0, "stdout": "", "stderr": ""}
        with patch.object(cli, "doctor", return_value=env), patch.object(cli, "invoke", side_effect=invoke):
            selected = cli.run_checks(self.workspace, "gcc", "debug", unit_id="U-ONE")
            self.assertEqual(selected["required_tests"], ["sample.boundary"])
            self.assertEqual(selected["status"], "passed")
            entire = cli.run_checks(self.workspace, "gcc", "debug", unit_id="U-ONE", all_tests=True)
            self.assertEqual(entire["status"], "unavailable")
            self.assertIn("sample.other", entire["reason"])

    def test_structured_dependency_contract_maps_real_source_and_include_paths(self):
        self.units[0]["tests"] = ["sample.boundary"]; self.write_catalog()
        dependency = self.base / "foundation-workspace"
        cli.save(dependency / "family.json", {"family": "foundations"})
        (dependency / "src").mkdir(); (dependency / "include").mkdir()
        family = cli.read(self.workspace / "family.json")
        family["dependencies"] = [{"family": "foundations", "implementation_variable": "CMASTERY_FOUNDATIONS_ROOT",
                                   "include_variable": "CMASTERY_FOUNDATIONS_INCLUDE", "required_for": ["sample.boundary"]}]
        cli.save(self.workspace / "family.json", family)
        env = {"capabilities": {k: {"available": True} for k in ("gcc", "cmake", "ninja")}}
        def invoke(command, **kwargs):
            if command[0] == "ctest":
                path = Path(command[command.index("--output-junit") + 1]); path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text('<testsuite><testcase name="sample.boundary" status="run"/></testsuite>')
            return {"command": command, "exit_code": 0, "stdout": "", "stderr": ""}
        with patch.object(cli, "doctor", return_value=env), patch.object(cli, "invoke", side_effect=invoke):
            result = cli.run_checks(self.workspace, "gcc", "debug", unit_id="U-ONE", dependencies={"foundations": dependency})
        self.assertEqual(result["status"], "passed")
        self.assertIn("-DCMASTERY_FOUNDATIONS_ROOT=" + str(dependency / "src"), result["steps"][0]["command"])
        self.assertIn("-DCMASTERY_FOUNDATIONS_INCLUDE=" + str(dependency / "include"), result["steps"][0]["command"])
        self.assertNotIn("assessor_source", result)

    def test_unit_target_tests_select_scoped_guest_without_future_family_tests(self):
        self.units[0]["tests"] = ["host.scheduler"]
        self.units[0]["target_tests"] = {"cortex-m4-qemu": ["execution.image_cortex-m4-qemu", "execution.cortex_m4_preemptive"]}
        self.write_catalog()
        family = cli.read(self.workspace / "family.json")
        family["targets"] = {"cortex-m4-qemu": {"evidence_kind": "emulator", "tests": ["future.vm"]}}
        family["projects"][0]["target_tests"] = {"cortex-m4-qemu": ["future.vm"]}
        cli.save(self.workspace / "family.json", family)
        env = {"capabilities": {k: {"available": True} for k in ("gcc", "cmake", "ninja")}}
        def invoke(command, **kwargs):
            if command[0] == "ctest":
                path = Path(command[command.index("--output-junit") + 1]); path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text('<testsuite><testcase name="execution.image_cortex-m4-qemu" status="run"/><testcase name="execution.cortex_m4_preemptive" status="run"/></testsuite>')
            return {"command": command, "exit_code": 0, "stdout": "", "stderr": ""}
        with patch.object(cli, "doctor", return_value=env), patch.object(cli, "invoke", side_effect=invoke):
            result = cli.run_checks(self.workspace, "gcc", "debug", unit_id="U-ONE", target="cortex-m4-qemu")
        self.assertEqual(result["status"], "passed")
        self.assertEqual(result["required_tests"], self.units[0]["target_tests"]["cortex-m4-qemu"])
        self.assertEqual(result["emulator_execution_tests"], ["execution.cortex_m4_preemptive"])

    def test_only_actual_guest_runners_count_as_emulator_observations(self):
        guest = ["kernel_boot_smoke", "kernel_runtime", "kernel_integration", "kernel_guest", "kernel_smp_guest_boot",
                 "execution.cortex_m4", "execution.cortex_m4_boot", "execution.cortex_m4_cooperative",
                 "execution.cortex_m4_preemptive", "execution.cortex_m4_fp"]
        record = {"tests": [{"name": n} for n in guest + ["kernel_archive", "execution.image_cortex-m4-qemu", "host.qemu_model"]]}
        self.assertEqual(cli.emulator_test_names(record), guest)
        self.prepare_claim({"id": "bounded", "scope": "emulator", "target": "cortex-m4-qemu"})
        record.update(project="PR-T01-01", unit="U-ONE", unit_contract=learn.digest(self.units[0]),
                      source=cli.snapshot(self.workspace), status="passed", assessment="learner", evidence_kind="emulator",
                      target="cortex-m4-qemu", tests=[{"name": "execution.image_cortex-m4-qemu"}])
        cli.save_check_receipt(self.workspace, record)
        with self.assertRaisesRegex(ValueError, "Required emulator evidence"):
            learn.acceptance(self.profile, self.workspace, "U-ONE", self.assessment_file)
        record["tests"] = [{"name": "execution.cortex_m4_boot"}]
        cli.save_check_receipt(self.workspace, record)
        self.assertEqual(learn.acceptance(self.profile, self.workspace, "U-ONE", self.assessment_file)["status"], "accepted")

    def test_generated_component_imports_do_not_expand_path_expressions(self):
        value = '/tmp/a $ENV{HOME}; message(FATAL_ERROR "bad") ]=]/component'
        literal = learn.cmake_literal(value)
        self.assertTrue(literal.startswith("[==["))
        self.assertTrue(literal.endswith("]==]"))
        self.assertEqual(literal[4:-4], value)

    def test_assessment_sheet_template_cannot_silently_accept(self):
        output = self.base / "sheet.md"
        template = self.base / "template.json"
        learn.study_sheet(self.profile, output, "U-ONE", template)
        generated = cli.read(template)
        self.assertFalse(generated["independence"]["unaided"])
        self.assertTrue(all(not c["met"] for c in generated["checks"]))
        self.assertNotIn("dimensions", generated)

    def test_new_family_unit_selection_and_same_project_advance_preserve_code(self):
        cli.save(self.root / "curriculum/project_graph.json", {"projects": [{"id": "PR-T01-01", "title": "Fixture", "module": "T01", "lab_family": None}]})
        brief = self.root / "curriculum/projects/PR-T01-01.md"
        brief.parent.mkdir(parents=True); brief.write_text("# Fixture project\n")
        guide = self.root / "curriculum/guides/component-contracts.md"
        guide.parent.mkdir(parents=True); guide.write_text("# Fixture contracts\n")
        cli.save(self.root / "labs/sample/family.json", {"family": "sample", "projects": [],
            "units": [{"id": u["id"], "project": u["project"], "tests": []} for u in self.units]})
        new = self.base / "new-unit-lab"
        cli.instantiate("PR-T01-01", new, unit_id="U-ONE")
        (new / "original.c").write_text("/* learner source */")
        cli.advance(new, "PR-T01-01", unit_id="U-TWO")
        self.assertEqual((new / "original.c").read_text(), "/* learner source */")
        self.assertEqual(cli.read(new / ".cmastery/project.json")["unit"], "U-TWO")
        self.assertTrue(list((new / "evidence/history").glob("*/UNIT.md")))

    def test_family_contracts_file_is_not_overwritten_by_generic_guide(self):
        cli.save(self.root / "curriculum/project_graph.json", {"projects": [{"id": "PR-T01-01", "title": "Fixture", "module": "T01", "lab_family": None}]})
        brief = self.root / "curriculum/projects/PR-T01-01.md"
        brief.parent.mkdir(parents=True); brief.write_text("# Fixture project\n\nSee [contracts](../guides/component-contracts.md).\n")
        guide = self.root / "curriculum/guides/component-contracts.md"
        guide.parent.mkdir(parents=True); guide.write_text("# Generic contracts guide\n")
        cli.save(self.root / "labs/sample/family.json", {"family": "sample", "projects": [],
            "units": [{"id": u["id"], "project": u["project"], "tests": []} for u in self.units]})
        (self.root / "labs/sample/CONTRACTS.md").write_text("# Family ownership contracts\n")
        shipped = self.base / "shipped-contracts-lab"
        cli.instantiate("PR-T01-01", shipped, unit_id="U-ONE")
        self.assertEqual((shipped / "CONTRACTS.md").read_text(), "# Family ownership contracts\n")
        self.assertEqual((shipped / "COMPONENT-CONTRACTS.md").read_text(), "# Generic contracts guide\n")
        self.assertIn("(COMPONENT-CONTRACTS.md)", (shipped / "PROJECT.md").read_text())
        cli.advance(shipped, "PR-T01-01", unit_id="U-TWO")
        self.assertEqual((shipped / "CONTRACTS.md").read_text(), "# Family ownership contracts\n")
        self.assertIn("(COMPONENT-CONTRACTS.md)", (shipped / "PROJECT.md").read_text())
        (self.root / "labs/sample/CONTRACTS.md").unlink()
        plain = self.base / "plain-lab"
        cli.instantiate("PR-T01-01", plain, unit_id="U-ONE")
        self.assertEqual((plain / "CONTRACTS.md").read_text(), "# Generic contracts guide\n")
        self.assertFalse((plain / "COMPONENT-CONTRACTS.md").exists())
        self.assertIn("(CONTRACTS.md)", (plain / "PROJECT.md").read_text())

    def test_physical_capture_closure_is_pinned_by_acceptance(self):
        self.units[0]["claims"] = [{"id": "bounded", "scope": "physical"}]
        self.write_catalog()
        family = cli.read(self.workspace / "family.json")
        family["targets"] = {"board": {"physical": True}}
        cli.save(self.workspace / "family.json", family)
        (self.workspace / "evidence/image.elf").write_bytes(b"demonstration-image")
        (self.workspace / "evidence/trace.csv").write_text("time,value\n0,1\n")
        physical = {key: "documented" for key in ("board_revision", "silicon_revision", "clocks", "cache_mpu", "instrument",
                    "instrumentation_overhead", "trace_overruns", "observations")}
        physical.update(target="board", source_sha256=cli.snapshot(self.workspace)["sha256"],
                        images=["image.elf"], captures=["trace.csv"])
        cli.save(self.workspace / "evidence/manifest.json", physical)
        cli.record_physical(self.workspace, self.workspace / "evidence/manifest.json")
        self.assessment["claims"][0]["scope"] = "physical"
        self.accept_one()
        capture = next((self.workspace / "evidence/physical").glob("*/*_captures_*"))
        capture.write_text("changed capture")
        self.assertIn("U-ONE", learn.study_status(self.profile)["reopened"])

    def capture(self, target, interfaces=None):
        (self.workspace / "evidence/image.elf").write_bytes(b"author-image")
        (self.workspace / "evidence/trace.csv").write_text("time,value\n0,1\n")
        physical = {key: "documented" for key in ("board_revision", "silicon_revision", "clocks", "cache_mpu", "instrument",
                    "instrumentation_overhead", "trace_overruns", "observations")}
        physical.update(target=target, source_sha256=cli.snapshot(self.workspace)["sha256"], images=["image.elf"], captures=["trace.csv"])
        if interfaces is not None:
            physical["interfaces"] = interfaces
        cli.save(self.workspace / "evidence/manifest.json", physical)
        return cli.record_physical(self.workspace, self.workspace / "evidence/manifest.json")

    def prepare_claim(self, required):
        self.units[0]["claims"] = [required]; self.write_catalog()
        self.assessment["claims"][0].update(scope=required["scope"])
        cli.save(self.assessment_file, self.assessment)
        learn.mark_studied(self.profile, "U-ONE"); self.review()

    def emulator(self, target, count):
        record = {"project": "PR-T01-01", "unit": "U-ONE", "unit_contract": learn.digest(self.units[0]),
                  "source": cli.snapshot(self.workspace), "status": "passed", "assessment": "learner",
                  "evidence_kind": "emulator", "target": target, "cpu_count": count, "tests": [{"name": "kernel_smp_guest_boot"}]}
        cli.save_check_receipt(self.workspace, record)

    def test_physical_claim_requires_exact_board_and_cannot_weaken_catalog_target(self):
        self.units[0]["capture_targets"] = {name: {"physical": True, "manual_only": True} for name in ("stm32f439", "stm32h745")}
        self.prepare_claim({"id": "bounded", "scope": "physical", "target": "stm32h745"})
        self.capture("stm32f439")
        self.assessment["claims"][0]["target"] = "stm32f439"
        cli.save(self.assessment_file, self.assessment)
        with self.assertRaisesRegex(ValueError, "stm32h745"):
            learn.acceptance(self.profile, self.workspace, "U-ONE", self.assessment_file)
        self.capture("stm32h745")
        self.assertEqual(learn.acceptance(self.profile, self.workspace, "U-ONE", self.assessment_file)["status"], "accepted")

    def test_manual_interfaces_require_real_declared_capture_without_pretend_build_target(self):
        self.units[0]["capture_targets"] = {"interface-bench": {"physical": True, "manual_only": True, "interfaces": ["UART", "SPI"]}}
        self.prepare_claim({"id": "bounded", "scope": "physical", "target": "interface-bench", "interfaces": ["UART", "SPI"]})
        with self.assertRaisesRegex(ValueError, "actually measured interfaces"):
            self.capture("interface-bench", ["CAN"])
        self.capture("interface-bench", ["UART"])
        with self.assertRaisesRegex(ValueError, "interfaces=SPI,UART"):
            learn.acceptance(self.profile, self.workspace, "U-ONE", self.assessment_file)
        self.capture("interface-bench", ["SPI"])
        self.assertEqual(learn.acceptance(self.profile, self.workspace, "U-ONE", self.assessment_file)["status"], "accepted")
        with self.assertRaisesRegex(ValueError, "Unsupported target"):
            cli.run_checks(self.workspace, "gcc", "debug", target="interface-bench")

    def test_emulator_target_and_all_cpu_counts_are_required(self):
        self.prepare_claim({"id": "bounded", "scope": "emulator", "target": "x86-smp", "cpus": [1, 2, 4]})
        self.emulator("cortex-m4-qemu", 4)
        with self.assertRaisesRegex(ValueError, "x86-smp"):
            learn.acceptance(self.profile, self.workspace, "U-ONE", self.assessment_file)
        self.emulator("x86-smp", 1); self.emulator("x86-smp", 2)
        with self.assertRaisesRegex(ValueError, "cpus=4"):
            learn.acceptance(self.profile, self.workspace, "U-ONE", self.assessment_file)
        self.emulator("x86-smp", 4)
        self.assertEqual(learn.acceptance(self.profile, self.workspace, "U-ONE", self.assessment_file)["status"], "accepted")
        self.assertEqual(len(list((self.workspace / "evidence/checks").glob("*.json"))), 4)

    def test_emulator_target_name_does_not_turn_host_test_into_guest_evidence(self):
        self.prepare_claim({"id": "bounded", "scope": "emulator", "target": "x86-smp", "cpus": 4})
        record = {"project": "PR-T01-01", "unit": "U-ONE", "unit_contract": learn.digest(self.units[0]),
                  "source": cli.snapshot(self.workspace), "status": "passed", "assessment": "learner",
                  "evidence_kind": "emulator", "target": "x86-smp", "cpu_count": 4, "tests": [{"name": "host.archive"}]}
        cli.save_check_receipt(self.workspace, record)
        with self.assertRaisesRegex(ValueError, "Required emulator evidence"):
            learn.acceptance(self.profile, self.workspace, "U-ONE", self.assessment_file)

    def test_same_source_file_evolution_preserves_prerequisite_but_stales_export(self):
        self.assessment["source_files"] = ["src/bytes.c"]
        self.accept_one(export=True)
        with (self.workspace / "src/bytes.c").open("a") as stream:
            stream.write("int independent_next_unit(void){return 7;}\n")
        state = learn.accepted(self.profile)["U-ONE"]
        self.assertFalse(state["valid"]); self.assertTrue(state["historical_valid"])
        learn.mark_studied(self.profile, "U-TWO")
        cli.save(self.assessment_file, dict(self.assessment, checks=[], claims=[]))
        self.assertEqual(learn.acceptance(self.profile, self.workspace, "U-TWO", self.assessment_file)["status"], "accepted")
        with self.assertRaisesRegex(ValueError, "current learner acceptance"):
            learn.artifact_export(self.profile, self.workspace, "bytes.checked", self.base / "stale-export", self.declaration)
        retained_hash = learn.accepted(self.profile)["U-ONE"]["record"]["revision"]["source"]["src/bytes.c"]
        retained = learn.area(self.profile) / "objects" / retained_hash
        retained.write_bytes(b"changed accepted object")
        self.assertFalse(learn.accepted(self.profile)["U-ONE"]["historical_valid"])

    def test_portfolio_and_project_completion_require_prior_physical_scope(self):
        self.units[0]["capture_targets"] = {"interface-bench": {"physical": True, "manual_only": True, "interfaces": ["UART"]}}
        self.units[0]["project_completion"] = False
        self.prepare_claim({"id": "bounded", "scope": "physical", "target": "interface-bench", "interfaces": ["UART"]})
        self.capture("interface-bench", ["UART"])
        learn.acceptance(self.profile, self.workspace, "U-ONE", self.assessment_file)
        self.units[1]["project_completion"] = True
        self.units[1]["claims"] = [{"id": "portfolio", "scope": "physical", "target": "interface-bench",
                                      "interfaces": ["UART", "SPI"], "source_units": ["U-ONE"]}]
        self.write_catalog(); learn.mark_studied(self.profile, "U-TWO")
        consumer = self.base / "portfolio"
        cli.save(consumer / ".cmastery/project.json", {"project": "PR-T01-01", "family": "sample", "unit": "U-TWO", "source_catalog": {}})
        (consumer / "evidence").mkdir(); (consumer / "evidence/unit.md").write_text("Portfolio interpretation of original physical observations")
        assessment = dict(self.assessment, checks=[], claims=[{"id": "portfolio", "scope": "physical", "evidence": ["evidence/unit.md"]}])
        cli.save(self.assessment_file, assessment)
        with self.assertRaisesRegex(ValueError, "interfaces=SPI,UART"):
            learn.acceptance(self.profile, consumer, "U-TWO", self.assessment_file)
        self.assertFalse(learn.study_status(self.profile)["completion"]["core_route_complete"])
        self.units[1]["claims"][0]["interfaces"] = ["UART"]; self.write_catalog()
        learn.acceptance(self.profile, consumer, "U-TWO", self.assessment_file)
        self.assertTrue(learn.study_status(self.profile)["completion"]["core_route_complete"])
        state = learn.accepted(self.profile)["U-ONE"]
        capture_hash = next(h for n,h in state["record"]["revision"]["evidence"].items() if "captures" in n)
        (learn.area(self.profile) / "objects" / capture_hash).write_bytes(b"altered retained physical observation")
        self.assertFalse(learn.study_status(self.profile)["completion"]["core_route_complete"])

    def test_conflicting_monolithic_provider_snapshots_are_rejected(self):
        source = self.workspace / "src/bytes.c"
        source.write_text(source.read_text() + "int next_parser(void){return 0;} /* learner stub */\n")
        bundle = self.export_one()
        self.artifacts.append(dict(self.artifacts[0], id="parser.integer")); self.write_catalog()
        source.write_text(source.read_text().replace("return 0;} /* learner stub */", "return 42;} /* implemented parser */"))
        cli.save(self.assessment_file, self.assessment)
        learn.acceptance(self.profile, self.workspace, "U-ONE", self.assessment_file)
        declaration = cli.read(self.declaration); declaration["cmake_target"] = "later_parser"
        cli.save(self.declaration, declaration)
        later = self.base / "later-provider"
        learn.artifact_export(self.profile, self.workspace, "parser.integer", later, self.declaration)
        consumer = self.base / "consumer"
        cli.save(consumer / ".cmastery/project.json", {"project": "PR-T01-01", "family": "sample"})
        learn.artifact_link(self.profile, consumer, bundle, "hosted", "host")
        learn.artifact_link(self.profile, consumer, later, "hosted", "host")
        with self.assertRaisesRegex(ValueError, "revalidate and re-export"):
            learn.prepare_component_build(consumer, consumer / "build/control")
        declaration["cmake_target"] = "revalidated_bytes"; cli.save(self.declaration, declaration)
        coherent = self.base / "coherent-provider"
        learn.artifact_export(self.profile, self.workspace, "bytes.checked", coherent, self.declaration)
        learn.artifact_link(self.profile, consumer, coherent, "hosted", "host")
        self.assertTrue(learn.prepare_component_build(consumer, consumer / "build/coherent")["arguments"])


if __name__ == "__main__":
    unittest.main(verbosity=2)

"""Isolated ownership/schema regressions; no physical observations or receipts."""
from pathlib import Path
import copy
import json
import tempfile
import unittest

import verify_embedded_fixtures as oracle


class EmbeddedFixtureContracts(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="cmastery-embedded-fixtures-")
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.original_templates = {}
        for name, (kind, owners) in oracle.CAPTURE_CONTRACTS.items():
            relative = "fixtures/embedded/" + name
            data = json.loads((oracle.ROOT / relative).read_text(encoding="utf-8"))
            self.original_templates[name] = data
            self.put(relative, data)
            guide = f"curriculum/guides/interrupt-{kind}-board-protocol.md"
            checker = f"tools/check_interrupt_{kind}_capture.py"
            self.put(guide, "# Original synthetic ownership-test protocol\n")
            self.put(checker, "# Synthetic ownership-test checker placeholder\n")
            for owner in owners:
                self.put(f"curriculum/projects/{owner}.md", "```json\n" + json.dumps(
                    dict(id=owner, module="T29", fixtures=[])) + "\n```\n")
                self.put(f"curriculum/v5/expansion/legacy_assessments/{owner}.json", dict(
                    project=owner, domain="T29", required=True,
                    supplied_artifacts=[relative, guide, checker], shared_contracts=[guide]))
        self.milestone = "fixtures/embedded/PR-T29-02.json"
        self.put(self.milestone, dict(id="PR-T29-02-fixture", description="Saved mask restoration",
                                    input=dict(kind="mask_restore", prior=[0, 1]), expected=[0, 1],
                                    verification_method="Independent original finite model"))
        self.project = "curriculum/projects/PR-T29-02.md"
        self.put(self.project, "```json\n" + json.dumps(dict(
            id="PR-T29-02", module="T29", fixtures=[self.milestone])) + "\n```\n")

    def put(self, relative, data):
        path = self.root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(data if isinstance(data, str) else json.dumps(data), encoding="utf-8")

    def card(self, owner="PR-T29-02"):
        relative = f"curriculum/v5/expansion/legacy_assessments/{owner}.json"
        return relative, json.loads((self.root / relative).read_text(encoding="utf-8"))

    def rejected(self, contains=None):
        result = oracle.verify(self.root)
        self.assertTrue(result["failures"], result)
        self.assertEqual(result["physical_observations"], 0)
        if contains:
            self.assertIn(contains, json.dumps(result["failures"]))
        return result

    def test_bound_templates_and_independent_milestone(self):
        self.assertEqual(oracle.verify(self.root), dict(fixtures=4, checks=1,
            capture_template_checks=3, physical_observations=0, failures=[]))

    def test_current_registry_preserves_every_expected_output(self):
        result = oracle.verify()
        self.assertFalse(result["failures"], result)
        self.assertGreater(result["checks"], 0)
        self.assertEqual(result["capture_template_checks"], len(oracle.CAPTURE_CONTRACTS))
        self.assertEqual(result["fixtures"], result["checks"] + result["capture_template_checks"])
        self.assertEqual(result["physical_observations"], 0)

    def test_changed_milestone_expected_value_rejected(self):
        path = self.root / self.milestone
        data = json.loads(path.read_text())
        data["expected"] = [0, 0]
        self.put(self.milestone, data)
        self.rejected('"actual": [0, 1]')

    def test_unreferenced_milestone_stays_rejected(self):
        self.put("fixtures/embedded/extra.json", dict(id="extra-fixture", description="extra",
            input=dict(kind="mask_restore", prior=[0]), expected=[0], verification_method="test"))
        self.rejected("Unreferenced fixture")

    def test_missing_owned_milestone_stays_rejected(self):
        (self.root / self.milestone).unlink()
        self.rejected("Missing referenced fixture")

    def test_unknown_nested_and_non_json_files_rejected(self):
        for relative in ("fixtures/embedded/unknown_capture.json", "fixtures/embedded/nested/interrupt_time_capture.json",
                         "fixtures/embedded/unknown.txt"):
            with self.subTest(relative=relative):
                self.put(relative, self.original_templates["interrupt_time_capture.json"])
                self.rejected()
                (self.root / relative).unlink()

    def test_every_template_field_and_type_checked(self):
        for name, original in self.original_templates.items():
            for field, value in original.items():
                for mode in ("missing", "wrong-type"):
                    with self.subTest(template=name, field=field, mode=mode):
                        data = copy.deepcopy(original)
                        if mode == "missing":
                            del data[field]
                        else:
                            data[field] = False if type(value) is int else 1
                        self.assertTrue(oracle.capture_template_errors(name, data))
            data = copy.deepcopy(original)
            data["physical_observed"] = True
            self.assertTrue(oracle.capture_template_errors(name, data))

    def test_populated_or_fabricated_capture_is_not_a_blank_form(self):
        for name, original in self.original_templates.items():
            for status in ("observed", "captured", "passed"):
                with self.subTest(template=name, status=status):
                    data = copy.deepcopy(original)
                    data["status"] = status
                    self.put("fixtures/embedded/" + name, data)
                    self.rejected("authored unobserved template")
                    self.put("fixtures/embedded/" + name, original)
        changes = {
            "interrupt_handoff_capture.json": ("samples", [{"entry": 1}]),
            "interrupt_lease_capture.json": ("source_sha256", {"image": "a" * 64}),
            "interrupt_time_capture.json": ("rows", [{"begin_tick": 1}]),
        }
        for name, (key, value) in changes.items():
            data = copy.deepcopy(self.original_templates[name]); data[key] = value
            self.assertTrue(oracle.capture_template_errors(name, data))

    def test_nested_field_type_and_lease_schedule_contract(self):
        time = copy.deepcopy(self.original_templates["interrupt_time_capture.json"])
        time["calibration"]["anchor_tick"] = False
        self.assertTrue(oracle.capture_template_errors("interrupt_time_capture.json", time))
        lease = copy.deepcopy(self.original_templates["interrupt_lease_capture.json"])
        lease["required_schedule_ids"][0] = "invented"
        self.assertTrue(oracle.capture_template_errors("interrupt_lease_capture.json", lease))

    def test_missing_or_wrong_each_parent_identity(self):
        for name, (_, owners) in oracle.CAPTURE_CONTRACTS.items():
            for owner in owners:
                relative, original = self.card(owner)
                for field, value in (("project", "PR-T29-99"), ("domain", "T35"), ("required", False)):
                    with self.subTest(owner=owner, field=field):
                        data = copy.deepcopy(original); data[field] = value; self.put(relative, data)
                        self.rejected("Wrong capture assessment identity")
                        self.put(relative, original)
                (self.root / relative).unlink()
                self.rejected("Missing public artifact")
                self.put(relative, original)

    def test_missing_duplicate_or_private_artifact_binding(self):
        relative, original = self.card()
        for path in original["supplied_artifacts"]:
            for mode in ("missing", "duplicate", "private"):
                with self.subTest(path=path, mode=mode):
                    data = copy.deepcopy(original)
                    if mode == "missing": data["supplied_artifacts"].remove(path)
                    elif mode == "duplicate": data["supplied_artifacts"].append(path)
                    else: data["supplied_artifacts"][data["supplied_artifacts"].index(path)] = "assessor/private.py"
                    self.put(relative, data)
                    self.rejected("Unbound or duplicate capture artifact")
                    self.put(relative, original)
        data = copy.deepcopy(original); data["shared_contracts"] = []
        self.put(relative, data); self.rejected("Unbound capture protocol")

    def test_missing_public_template_protocol_checker(self):
        _, card = self.card()
        for relative in card["supplied_artifacts"]:
            with self.subTest(relative=relative):
                path = self.root / relative; content = path.read_bytes(); path.unlink()
                self.rejected("Missing public artifact")
                path.write_bytes(content)

    def test_capture_cannot_be_registered_as_milestone(self):
        self.put(self.project, "```json\n" + json.dumps(dict(id="PR-T29-02", module="T29",
            fixtures=[self.milestone, "fixtures/embedded/interrupt_handoff_capture.json"])) + "\n```\n")
        self.rejected("Capture template bound as milestone output")

    def test_duplicate_or_escape_milestone_binding(self):
        for fixtures, message in (([self.milestone, self.milestone], "Duplicate fixture binding"),
                                  (["../outside.json"], "Fixture escapes owned directory")):
            self.put(self.project, "```json\n" + json.dumps(dict(id="PR-T29-02", module="T29", fixtures=fixtures)) + "\n```\n")
            self.rejected(message)

    def test_duplicate_json_fields_rejected(self):
        self.put("fixtures/embedded/interrupt_time_capture.json", '{"status":"observed","status":"unobserved"}')
        self.rejected("Duplicate JSON key")

    def test_safe_public_path_rejects_traversal_private_and_absolute(self):
        for path in ("../outside", "assessor/answer.py", str(self.root / "absolute"), "tools\\checker.py"):
            with self.subTest(path=path), self.assertRaises(ValueError):
                oracle.public_file(self.root, path)


if __name__ == "__main__":
    unittest.main()

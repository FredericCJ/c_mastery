"""Compile the real P1 integration through accepted, exported learner bundles.

This is an author plumbing test in disposable profiles. Reference sources are
explicitly copied into fixture workspaces to provide known implementations; these
temporary self-assessments make no claim about any learner's accomplishments.
Run in Linux/WSL with GCC, CMake and Ninja. No assessor fallback is requested from
the CLI at any point in the handoff.
"""
from pathlib import Path
import shutil
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from cmastery import cli, learning

SOURCE = Path(__file__).resolve().parents[1]


@unittest.skipUnless(sys.platform.startswith("linux") and all(shutil.which(x) for x in ("gcc", "cmake", "ninja")),
                     "The real hosted handoff needs Linux/WSL, GCC, CMake and Ninja")
class ActualIntegrationHandoff(unittest.TestCase):
    def test_p1_accept_export_relocate_link_and_run(self):
        with tempfile.TemporaryDirectory(prefix="cmastery-author-handoff-") as temporary:
            base = Path(temporary)
            root = base / "course-fixture"
            study = base / "author-fixture-not-learner-progress"
            unit_data = [
                ("AUTHOR-FOUNDATIONS", "PR-T03-02", "foundations", ["foundations.cursor", "foundations.parse"], ["bytes.cursor", "parser.integer"]),
                ("AUTHOR-ALGORITHMS", "PR-T14-01", "algorithms", ["algorithms.ring", "algorithms.pool"], ["containers.ring", "memory.pool"]),
                ("LAB-IN-01", "PR-P1-02", "integration", ["integration.p1"], [])]
            units = []
            artifacts = []
            graph = []
            for identifier, project, family, tests, exports in unit_data:
                shutil.copytree(SOURCE / "labs" / family, root / "labs" / family)
                units.append({"id": identifier, "project": project, "title": "Author-only handoff fixture", "hours": 1,
                    "objectives": [], "requires": [], "imports": [], "exports": exports, "recall_sets": [],
                    "critical_objectives": [], "family": family, "tests": tests, "checks": [], "claims": [],
                    "target_scope": "host", "milestone": bool(exports), "decision_required": False})
                graph.append({"id": project, "title": "Author-only fixture", "module": "fixture", "lab_family": family})
                path = root / "curriculum/projects" / (project + ".md")
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text("# Author-only integration plumbing fixture\n", encoding="utf-8")
                for artifact in exports:
                    artifacts.append({"id": artifact, "producer_unit": identifier, "contract_version": "1",
                        "public_contract": "Use the actual laboratory API and its semantic tests", "critical_objectives": [],
                        "profiles": ["A"], "targets": ["host"], "consumers": ["LAB-IN-01"]})
            units[-1]["requires"] = [u["id"] for u in units[:2]]
            units[-1]["imports"] = [a["id"] for a in artifacts]
            cli.save(root / "curriculum/project_graph.json", {"projects": graph})
            cli.save(root / "curriculum/v4/units.json", {"units": units, "order": [u["id"] for u in units]})
            cli.save(root / "curriculum/v4/artifacts.json", {"artifacts": artifacts})
            cli.save(root / "curriculum/v4/recall.json", {"sets": [], "policy": {"gaps_days": [1, 3, 7, 14, 30], "freshness_days": 14, "session_minutes": 15}})
            guide = root / "curriculum/guides/component-contracts.md"
            guide.parent.mkdir(parents=True); guide.write_text("# Explicit author fixture contracts\n", encoding="utf-8")
            with patch.object(cli, "ROOT", root):
                learning.study_init(study)
                exports = []
                for item in units[:2]:
                    family = item["family"]
                    workspace = base / (family + "-author-implementation")
                    cli.instantiate(item["project"], workspace, unit_id=item["id"])
                    # Explicit author fixture seeding, never CLI fallback.
                    for source in (SOURCE / "assessor" / family).iterdir():
                        if source.is_file() and source.suffix in (".c", ".S"):
                            shutil.copy2(source, workspace / "src" / source.name)
                    receipt = cli.run_checks(workspace, "gcc", "debug", unit_id=item["id"])
                    self.assertEqual(receipt["status"], "passed", receipt.get("steps"))
                    self.assertEqual(receipt["assessment"], "learner")
                    self.assertNotIn("assessor_source", receipt)
                    evidence = workspace / "evidence/author-fixture.md"
                    evidence.write_text("Author-only known source plumbing test; no learner mastery claim.\n", encoding="utf-8")
                    assessment = {"explanation": "Author fixture validates actual import plumbing", "transfer": "Relocate the exported bundle",
                        "independence": {"unaided": True, "assistance": []}, "checks": [], "claims": [],
                        "evidence": ["evidence/author-fixture.md"],
                        "dimensions": {key: 3 if i < 2 else 2 for i, key in enumerate(learning.DIMENSIONS)}}
                    assessment_path = base / (family + "-assessment.json"); cli.save(assessment_path, assessment)
                    learning.mark_studied(study, item["id"])
                    learning.acceptance(study, workspace, item["id"], assessment_path)
                    for artifact in item["exports"]:
                        sources = ["src/foundation.c", "src/v4_foundation.c"] if family == "foundations" else ["src/algo.c", "src/v4_algo.c"]
                        headers = [p.relative_to(workspace).as_posix() for p in (workspace / "include").glob("*.h")]
                        declaration = {"contract_version": "1", "cmake_target": "fixture_" + artifact.replace(".", "_"),
                            "profiles": ["A"], "targets": ["host"], "sources": sources, "headers": headers,
                            "include_dirs": ["include"], "dependencies": []}
                        declaration_path = base / (artifact + ".json"); cli.save(declaration_path, declaration)
                        bundle = base / "exports" / artifact
                        learning.artifact_export(study, workspace, artifact, bundle, declaration_path)
                        moved = base / "relocated" / artifact; moved.parent.mkdir(exist_ok=True)
                        shutil.move(str(bundle), moved)
                        exports.append(moved)
                consumer = base / "integration-author-implementation"
                cli.instantiate("PR-P1-02", consumer, unit_id="LAB-IN-01")
                for bundle in exports:
                    learning.artifact_link(study, consumer, bundle, "A", "host")
                # Unimplemented integration remains a rejecting learner check.
                rejected = cli.run_checks(consumer, "gcc", "debug", unit_id="LAB-IN-01")
                self.assertEqual(rejected["status"], "failed", rejected.get("steps"))
                shutil.copy2(SOURCE / "assessor/integration/integration.c", consumer / "src/integration.c")
                # The runner must regenerate imports; edited local convenience
                # CMake metadata must not execute during a controlled check.
                (consumer / ".cmastery/components.cmake").write_text('message(FATAL_ERROR "EDITED IMPORT EXECUTED")\n', encoding="utf-8")
                receipt = cli.run_checks(consumer, "gcc", "debug", unit_id="LAB-IN-01")
                self.assertEqual(receipt["status"], "passed", receipt.get("steps"))
                self.assertEqual(receipt["assessment"], "learner")
                self.assertEqual(set(receipt["component_build"]["roles"]), {"foundations", "algorithms"})
                self.assertEqual(len(receipt["component_links"]), 4)
                self.assertNotIn("assessor_source", receipt)
                self.assertEqual([test["name"] for test in receipt["tests"]], ["integration.p1"])
                self.assertTrue(all("CMASTERY_ASSESSOR_ROOT" not in arg for arg in receipt["steps"][0]["command"]))
                # All-tests must not claim missing firmware/VM/storage scenarios.
                unavailable = cli.run_checks(consumer, "gcc", "debug", unit_id="LAB-IN-01", all_tests=True)
                self.assertEqual(unavailable["status"], "unavailable")
                self.assertIn("Required tests not registered", unavailable["reason"])
                link = learning.linked(consumer)["bytes.cursor"]
                (consumer / link["path"] / "src/foundation.c").write_text("/* changed pinned import */\n", encoding="utf-8")
                self.assertTrue(cli.report(consumer)["evidence"]["stale"])
                with self.assertRaises(ValueError):
                    cli.run_checks(consumer, "gcc", "debug", unit_id="LAB-IN-01")


if __name__ == "__main__":
    unittest.main(verbosity=2)

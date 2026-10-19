"""Exercise exported learner scaffolds without creating learner acceptance."""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys
import tempfile

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from cmastery import cli, exercises


def inspect(run_stub=False):
    data = exercises.catalog()
    records, errors = [], []
    expected = cli.read(cli.ROOT / "curriculum/edition.json").get("exercise_expansion", {}).get("required_exercises")
    if expected is not None and len(data["exercises"]) != expected:
        errors.append("Actual exercise catalog differs from manifest count")
    executions = []
    checked_families = set()
    with tempfile.TemporaryDirectory(prefix="c mastery real exercise ") as directory:
        temporary = Path(directory).resolve()
        if temporary.is_relative_to(cli.ROOT.resolve()):
            raise ValueError("Integration workspaces must be outside the repository")
        for identifier in data["order"]:
            value = exercises.exercise(identifier)
            workspace = temporary / identifier
            try:
                cli.instantiate(value["project"], workspace, exercise_id=identifier)
                selected = exercises.workspace_contract(workspace, identifier)
                names = [p.relative_to(workspace).as_posix() for p in workspace.rglob("*") if p.is_file()]
                if any("assessor" in Path(name).parts or Path(name).name == "semantic_answers.inc" for name in names):
                    raise ValueError("Assessor material copied into learner workspace")
                body = (workspace / "EXERCISE.md").read_text(encoding="utf-8")
                if any(row["criterion"] not in body or any(level not in body for level in row["levels"].values()) for row in value["rubric"]):
                    raise ValueError("Learner brief omitted an objective-specific rubric anchor")
                scaffold = exercises.path_in_root(value["lab"]["scaffold"])
                learner_sources = {name: identity for name, identity in cli.snapshot(scaffold)["files"].items() if name.startswith("src/")}
                if not learner_sources or any(exercises.sha(workspace / name) != identity for name, identity in learner_sources.items()):
                    raise ValueError("Initialized learner source differs from the canonical unsolved scaffold")
                for name in learner_sources:
                    answer = cli.ROOT / "assessor" / value["lab"]["family"] / Path(name).name
                    if answer.is_file() and exercises.sha(answer) == exercises.sha(workspace / name):
                        raise ValueError("Completed assessor implementation shipped as learner source")
                records.append({"exercise": identifier, "project": value["project"], "status": "passed",
                    "contract_sha256": cli.read(workspace / ".cmastery/project.json")["exercise_contract"],
                    "criteria_present": len(value["rubric"]), "copied_fixtures": value["fixtures"],
                    "copied_shared_contracts": value["shared_contracts"], "learner_sources": learner_sources,
                    "assessor_files_absent": True, "canonical_workspace_contract": selected["id"]})
                family = value['lab']['family']
                if run_stub and family not in checked_families:
                    execution = check_exported_stub(workspace, temporary, identifier)
                    executions.append(execution)
                    checked_families.add(family)
                    if execution['status'] != 'passed':
                        errors.append(family + ': exported stub did not compile and reach its intended assertion')
            except (OSError, ValueError, KeyError) as error:
                records.append({"exercise": identifier, "status": "failed", "error": str(error)})
                errors.append(identifier + ": " + str(error))
        if run_stub:
            edition = cli.read(cli.ROOT / 'curriculum/edition.json')
            for family in sorted(set(edition['new_families']) - checked_families):
                meta = cli.read(cli.ROOT / 'labs' / family / 'family.json')
                project = meta['projects'][0]
                project = project['id'] if isinstance(project, dict) else project
                workspace = temporary / ('project-' + family)
                try:
                    cli.instantiate(project, workspace, family_name=family)
                    execution = check_exported_stub(workspace, temporary)
                    executions.append(execution)
                    checked_families.add(family)
                    if execution['status'] != 'passed':
                        errors.append(family + ': exported project stub failed before its semantic assertion')
                except (OSError, ValueError, KeyError) as error:
                    errors.append(family + ': ' + str(error))
    if run_stub and not executions:
        errors.append('Requested exported workspace executions did not occur')
    return {"schema_version": 1, "status": "failed" if errors else "passed", "errors": errors,
            "exercise_count": len(records), "workspaces": records,
            "stub_executions": executions, "executed_families": sorted(checked_families),
            "limits": "Temporary actual-catalog integration; no learner progress, assessment, independent human review or physical validation recorded."}


def assertion_marker(meta):
    """Select the declared assertion, or a known specialized harness contract."""
    family = meta.get('family', meta.get('id'))
    campaign = meta.get('campaign')
    campaign = campaign if isinstance(campaign, dict) else {}
    markers = {'foundation_depth': 'FD_CHECK:', 'b04_depth': 'DEPTH_CHECK:',
               'b05_depth': 'DEPTH_CHECK:', 'b045_systems': 'DEPTH_CHECK:',
               'systems_certificates': 'SC_CHECK:', 'serial_depth': 'SD_CHECK:',
               'ethernet_depth': 'ED_CHECK:', 'edu7_depth': 'E7_CHECK:',
               'http_depth': 'HD_CHECK:', 'service_depth': 'SD_CHECK:'}
    b06 = {'architecture_ports', 'firmware_lifecycle', 'kernel_lifecycle', 'linux_io_depth',
           'memory_defense', 'numeric_simd', 'subset_compiler', 'verification_fuzzing'}
    marker = campaign.get('assertion_marker', markers.get(family, 'B06_CHECK' if family in b06 else None))
    if not isinstance(marker, str) or not marker.strip():
        raise ValueError('Family has no valid semantic assertion marker: ' + str(family))
    return marker


def check_exported_stub(workspace, temporary, identifier=None):
    meta = cli.read(workspace / 'family.json')
    family = meta.get('family', meta.get('id'))
    marker = assertion_marker(meta)
    dependencies = {}
    for name in cli.dependency_contracts(meta, {}).keys():
        destination = temporary / ('dependency-' + name)
        if not destination.exists():
            dependency = cli.read(cli.ROOT / 'labs' / name / 'family.json')
            project = dependency['projects'][0]
            project = project['id'] if isinstance(project, dict) else project
            cli.instantiate(project, destination, family_name=name)
        dependencies[name] = destination
    receipt = cli.run_checks(workspace, 'gcc', 'debug', exercise_id=identifier, dependencies=dependencies)
    steps = receipt.get('steps', [])
    semantic_failure = (receipt.get('status') == 'failed' and len(steps) == 3
        and steps[0].get('exit_code') == 0 and steps[1].get('exit_code') == 0
        and steps[2].get('exit_code') not in (None, 0) and marker in steps[2].get('stdout', '')
        and any(test.get('failed') for test in receipt.get('tests', [])))
    return dict(family=family, status='passed' if semantic_failure else 'failed', receipt=receipt,
        assertion_marker=marker,
        claim='Exported unsolved learner source compiled and reached the intended semantic assertion.',
        dependency_workspaces=sorted(dependencies),
        limits='One build per family checks export paths. Unsolved dependencies are explicit harness inputs; their presence does not prove component integration or mastery.')


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--run-stub", action="store_true", help="Require actual GCC configure/build/semantic rejection under Linux/WSL")
    args = parser.parse_args(argv)
    try:
        report = inspect(args.run_stub)
    except (OSError, ValueError, KeyError) as error:
        report = {"status": "failed", "errors": [str(error)]}
    print(json.dumps(report, indent=2, ensure_ascii=False))
    return int(report["status"] != "passed")


if __name__ == "__main__":
    raise SystemExit(main())

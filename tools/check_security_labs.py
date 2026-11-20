#!/usr/bin/env python3
"""Validate the declared model, evidence and remediation of family-E exercises."""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from cmastery.exercises import parse_brief, path_in_root

FIELDS = ("target", "adversary", "violated_property", "boundary", "expected_evidence",
          "impact_class", "remediation", "limitations")
IMPACT_CLASSES = {"crash", "sanitizer-finding", "undefined-behaviour",
                  "demonstrated-invariant-violation", "proved-impact"}


def problems_for(value):
    lab = value.get("security_lab")
    if not isinstance(lab, dict):
        return ["missing security_lab"]
    issues = []
    for field in FIELDS:
        if not isinstance(lab.get(field), str) or not lab[field].strip():
            issues.append(f"missing {field}")
    if issues:
        return issues
    if lab["impact_class"].split()[0].rstrip(".,:;") not in IMPACT_CLASSES:
        issues.append("unknown impact class")
    try:
        target = path_in_root(lab["target"])
        scaffold = path_in_root(value["lab"]["scaffold"])
        if not target.is_relative_to(scaffold):
            issues.append("target outside the declared learner scaffold")
    except (ValueError, KeyError, TypeError) as error:
        issues.append(f"invalid target or scaffold ({error})")
    return issues


def check(directory, allow_missing=False):
    results, invalid, inputs = {}, {}, {}
    for path in sorted(directory.glob("X-*.md")):
        try:
            value, _ = parse_brief(path)
            if not isinstance(value, dict) or value.get("id") != path.stem:
                raise ValueError("exercise ID differs from its filename")
            if not isinstance(value.get("families"), list):
                raise ValueError("missing exercise family list")
            if "E" in value["families"]:
                results[path.stem] = problems_for(value)
                inputs[path.name] = hashlib.sha256(path.read_bytes()).hexdigest()
        except (OSError, ValueError) as error:
            invalid[path.stem] = [str(error)]
    missing = [name for name, issues in results.items() if issues == ["missing security_lab"]]
    failing = {name: issues for name, issues in results.items()
               if issues and not (allow_missing and name in missing)}
    if not results:
        failing["catalog"] = ["no family-E exercises found"]
    failing.update(invalid)
    return dict(schema_version=1, generated_at=datetime.now(timezone.utc).isoformat(),
                exercises=len(results), complete=sum(not issues for issues in results.values()),
                missing=missing, failing=failing, passed=not failing,
                validator_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
                family_e_brief_sha256=inputs,
                scope="Structure and public target binding of authored contracts. Learner performance and model outcomes require separate observations.")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--report", type=Path)
    parser.add_argument("--allow-missing", action="store_true",
                        help="report missing contracts while checking the fields of existing contracts")
    args = parser.parse_args()
    report = check(ROOT / "curriculum/v5/exercises", args.allow_missing)
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps({k: report[k] for k in ("exercises", "complete", "missing", "failing", "passed")}))
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Check that every adversarial (family E) exercise states its security contract.

Each family-E brief in curriculum/v5/exercises/ must carry a `security_lab`
object in its JSON header with: target, adversary, violated_property,
boundary, expected_evidence, impact_class, remediation and limitations. The
impact class separates a crash, a sanitizer finding or undefined behaviour
from a demonstrated invariant violation or a proved impact, so a crash is
never presented as a proven exploit.

    python3 tools/check_security_labs.py [--report PATH] [--allow-missing]
"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
FIELDS = ("target", "adversary", "violated_property", "boundary", "expected_evidence",
          "impact_class", "remediation", "limitations")
IMPACT_CLASSES = {"crash", "sanitizer-finding", "undefined-behaviour",
                  "demonstrated-invariant-violation", "proved-impact"}
HEADER = re.compile(r"^```json\n(.*?)\n```", re.S)


def header(path):
    match = HEADER.match(path.read_text(encoding="utf-8-sig"))
    return json.loads(match.group(1)) if match else None


def problems_for(value):
    problems = []
    lab = value.get("security_lab")
    if not isinstance(lab, dict):
        return ["missing security_lab"]
    for field in FIELDS:
        text = lab.get(field)
        if not isinstance(text, str) or len(text.strip()) < 12:
            problems.append(f"{field}: missing or too short to be specific")
    impact = lab.get("impact_class", "")
    if impact.split()[0].rstrip(":;,.") not in IMPACT_CLASSES if impact else True:
        problems.append("impact_class must start with one of " + ", ".join(sorted(IMPACT_CLASSES)))
    targets = re.findall(r"(?:labs|assessor|fixtures)/[\w./-]+", lab.get("target", ""))
    for target in targets:
        if not (ROOT / target.rstrip(".,;")).exists():
            problems.append(f"target path does not exist: {target}")
    return problems


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--report", type=Path)
    parser.add_argument("--allow-missing", action="store_true",
                        help="report but do not fail on exercises without security_lab")
    args = parser.parse_args()
    results = {}
    for path in sorted((ROOT / "curriculum/v5/exercises").glob("X-*.md")):
        value = header(path)
        if not value or "E" not in value.get("families", []):
            continue
        results[value["id"]] = problems_for(value)
    missing = [k for k, v in results.items() if v == ["missing security_lab"]]
    failing = {k: v for k, v in results.items() if v and not (args.allow_missing and k in missing)}
    report = dict(schema_version=1, generated_at=datetime.now(timezone.utc).isoformat(),
                  exercises=len(results), complete=sum(not v for v in results.values()),
                  missing=missing, failing=failing, passed=not failing,
                  scope="Presence and shape of the stated security contract; not a review of its truth.")
    if args.report:
        args.report.write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps({k: report[k] for k in ("exercises", "complete", "passed")} | {"missing": len(missing)}))
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())

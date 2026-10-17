"""Source-bound structural validation of the eight new capstone supplements."""
import hashlib
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
from check_legacy_assessments import validate
from common import load, read_json


def main():
    taxonomy, _, projects, _ = load()
    objectives = {o["id"] for d in taxonomy for t in d["subtopics"] for o in t["objectives"]}
    units = {u["id"]: u for u in read_json(ROOT / "curriculum/v5/units.json")["units"]}
    base = ROOT / "curriculum/v5/expansion/legacy_assessments"
    schema = read_json(base / "schema.json")
    bound = {base / "schema.json", ROOT / "tools/check_legacy_assessments.py", Path(__file__).resolve()}
    errors, rows = [], []
    for path in sorted(base.glob("PR-CAP-*.json")):
        data = read_json(path)
        errors.extend(validate(data, schema, projects, objectives, units))
        rows.append({"project": data["project"], "criteria": len(data["criteria"])})
        bound.update([path, ROOT / data["assessor_analysis"]])
        bound.update(ROOT / p for p in data["shared_contracts"] + data["supplied_artifacts"])
        bound.update(ROOT / r["path"] for r in data["source_records"])
    if len(rows) != 8:
        errors.append("Expected all eight capstone supplements")
    report = {"status": "passed" if not errors else "failed", "projects": rows, "errors": errors,
              "command": ".conda/python.exe assessor/legacy_depth/validate_capstone_scope.py",
              "source_sha256": {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
                                for p in sorted(bound)},
              "limitations": ["Structural/source-binding checks only; full service, firmware, independent learner and physical gates remain separate."]}
    dest = ROOT / "reviews/v5/expansion/legacy_capstones/structure.json"
    dest.parent.mkdir(parents=True, exist_ok=True)
    dest.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"status": report["status"], "projects": len(rows),
                      "criteria": sum(r["criteria"] for r in rows), "errors": errors}))
    return 0 if not errors else 1


if __name__ == "__main__":
    raise SystemExit(main())

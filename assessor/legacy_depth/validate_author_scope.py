"""Validate this author's retained-domain supplements without rewriting global catalogs."""
import hashlib
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
from check_legacy_assessments import validate
from common import load, read_json

DOMAINS = ["T01", "T02", "T03", "T04", "T05", "T06", "T12", "T13", "T14",
           "T15", "T16", "T17", "T30", "T31", "T32", "T33", "T34", "T35"]


def main():
    taxonomy, _, projects, _ = load()
    objectives = {o["id"] for d in taxonomy for t in d["subtopics"] for o in t["objectives"]}
    units = {u["id"]: u for u in read_json(ROOT / "curriculum/v5/units.json")["units"]}
    schema_path = ROOT / "curriculum/v5/expansion/legacy_assessments/schema.json"
    schema = read_json(schema_path)
    bound = {schema_path, ROOT / "tools/check_legacy_assessments.py", Path(__file__).resolve()}
    results, errors = [], []
    for domain in DOMAINS:
        files = sorted((ROOT / "curriculum/v5/expansion/legacy_assessments").glob("PR-" + domain + "-*.json"))
        count, seen = 0, set()
        for path in files:
            data = read_json(path)
            errors.extend(validate(data, schema, projects, objectives, units))
            count += len(data["criteria"])
            seen.update(c["objective"] for c in data["criteria"])
            bound.update([path, ROOT / data["assessor_analysis"]])
            bound.update(ROOT / x for x in data["shared_contracts"] + data["supplied_artifacts"])
            bound.update(ROOT / x["path"] for x in data["source_records"])
        expected = {o for o in objectives if o.startswith(domain + ".")}
        missing = sorted(expected - seen)
        if missing:
            errors.append(domain + " missing leaf assessments: " + ", ".join(missing))
        results.append({"domain": domain, "projects": len(files), "criteria": count,
                        "unique_objectives": len(seen), "missing_objectives": missing})
    report = {"schema_version": 1, "status": "passed" if not errors else "failed",
              "command": ".conda/python.exe assessor/legacy_depth/validate_author_scope.py",
              "domains": results, "errors": errors,
              "source_sha256": {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
                                for p in sorted(bound)},
              "limitations": ["Structural and source-binding validation only; no independent human review, learner assessment, physical validation, or execution of every authored task.",
                              "Retained historical acceptance evidence is unchanged and not represented as a fresh execution."]}
    output = ROOT / "reviews/v5/expansion/legacy_depth/author_scope_validation.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"status": report["status"], "projects": sum(r["projects"] for r in results),
                      "criteria": sum(r["criteria"] for r in results), "errors": errors}))
    return 0 if not errors else 1


if __name__ == "__main__":
    raise SystemExit(main())

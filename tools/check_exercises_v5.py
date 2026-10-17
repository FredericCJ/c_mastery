"""Check specification integrity without promoting authored work to acceptance."""
import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from cmastery import cli, exercises, learning


def inspect():
    expected = exercises.build_catalog()
    actual = cli.read(learning.catalog_dir() / "exercises.json")
    errors = [] if actual == expected else ["Generated exercise catalog is stale"]
    manifest = cli.read(cli.ROOT / "curriculum/edition.json")
    declared = manifest.get("exercise_expansion", {}).get("required_exercises")
    required = sum(value["required"] for value in expected["exercises"])
    if declared is not None and required != declared:
        errors.append("Manifest required_exercises differs from actual required exercise count")
    if not expected["exercises"]:
        errors.append("No substantive exercise expansion authored")
    return {"status": "failed" if errors else "passed", "errors": errors,
            "exercise_count": len(expected["exercises"]), "required_exercise_count": required, "order": expected["order"],
            "limits": "Structure and traceability checks do not establish technical correctness or independent assessment."}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", help="Write a new check receipt; accepted historical receipts are never overwritten")
    args = parser.parse_args(argv)
    try:
        report = inspect()
    except (OSError, ValueError, KeyError, TypeError) as error:
        report = {"status": "failed", "errors": [str(error)]}
    if args.output:
        output = Path(args.output)
        if output.exists():
            raise ValueError("Choose a new validation receipt path")
        cli.save(output, report)
    import json
    print(json.dumps(report, indent=2))
    return int(report["status"] != "passed")


if __name__ == "__main__":
    raise SystemExit(main())

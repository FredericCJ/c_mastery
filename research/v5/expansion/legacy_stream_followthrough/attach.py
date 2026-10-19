"""Preserve the original assessment and bind its newly concrete stream producer."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
SUPPORT = "research/v5/modern/stream_producer.json"
VARIANT_SUPPORT = "research/v5/expansion/legacy_stream_followthrough/support.json"
PROJECT = "PR-T19-04"
IDS = ["AX-PR-T19-04-T19.08." + suffix for suffix in "abc"]
PUBLIC = ["labs/api_depth/stream/" + name
          for name in ("stream.h", "stream.c", "tests.c", "CONTRACTS.md",
                       "variants.h", "variants.c", "variants_test.c", "VARIANTS.md", "run_variants.py")]


def attach(record):
    if record["project"] != PROJECT:
        return record
    for criterion in record["criteria"]:
        if criterion["id"] in IDS:
            criterion["successor_support"] = {"path": VARIANT_SUPPORT, "id": criterion["id"]}
    additions = record.setdefault("additional_successor_support", [])
    additions[:] = [row for row in additions if row.get("path") not in (SUPPORT, VARIANT_SUPPORT)]
    additions.append({
        "path": SUPPORT, "project": PROJECT, "criterion_ids": IDS,
        "scope": "Concrete bounded io.stream frame, copied payload, short-write progress, "
                 "cancellation and live-owner generations, with actual T72 consumer. "
                 "This original producer record retains its narrower observation scope."
    })
    additions.append({"path": VARIANT_SUPPORT, "project": PROJECT, "criterion_ids": IDS,
                      "scope": "Original status-plus-count, scatter/gather and two-frame "
                               "changed conditions; consult source-bound variant evidence. "
                               "Author execution is separate from the delayed learner assessment."})
    for path in PUBLIC:
        if path not in record["supplied_artifacts"]:
            record["supplied_artifacts"].append(path)
    for name in ("CONTRACTS.md", "VARIANTS.md"):
        contract = "labs/api_depth/stream/" + name
        if contract not in record["shared_contracts"]:
            record["shared_contracts"].append(contract)
    return record


def main():
    path = ROOT / "curriculum/v5/expansion/legacy_assessments" / (PROJECT + ".json")
    record = attach(json.loads(path.read_text(encoding="utf-8")))
    path.write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8", newline="\n")
    print(json.dumps({"project": PROJECT, "criteria": IDS, "public_artifacts": PUBLIC}))


if __name__ == "__main__":
    main()

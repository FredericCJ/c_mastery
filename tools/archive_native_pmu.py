"""Archive a completed native PMU run without rewriting its raw evidence."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--destination", type=Path, required=True)
    args = parser.parse_args()
    source = args.source.resolve()
    destination = args.destination.resolve()
    if destination.exists() or not destination.is_relative_to(ROOT):
        parser.error("destination must be a fresh directory inside the checkout")
    original = json.loads(source.read_text())
    if not original.get("passed"):
        parser.error("source campaign did not complete")
    inputs = [(Path(row["log"]), row["sha256"]) for row in original["commands"]]
    inputs += [(ROOT / name, digest) for name, digest in original["source_sha256"].items()]
    inputs.append((Path(original["binary"]), original["binary_sha256"]))
    for path, expected in inputs:
        if not path.is_file() or sha(path) != expected:
            parser.error("missing or changed input: " + str(path))
    destination.mkdir(parents=True)
    logs = destination / "logs"
    logs.mkdir()
    retained = destination / "sources"
    retained.mkdir()
    shutil.copyfile(source, destination / "original-receipt.json")
    result = copy.deepcopy(original)
    names = set()
    for row in result["commands"]:
        src = Path(row["log"])
        if src.name in names:
            raise ValueError("duplicate log basename")
        names.add(src.name)
        target = logs / src.name
        shutil.copyfile(src, target)
        if sha(target) != row["sha256"]:
            raise ValueError("copy changed log")
        row["original_log"] = row["log"]
        row["log"] = str(target.relative_to(ROOT))
    for name, digest in original["source_sha256"].items():
        target = retained / name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(ROOT / name, target)
        if sha(target) != digest:
            raise ValueError("copy changed source")
    binary = destination / "native"
    shutil.copyfile(original["binary"], binary)
    if sha(binary) != original["binary_sha256"]:
        raise ValueError("copy changed binary")
    result["original_binary"] = result["binary"]
    result["binary"] = str(binary.relative_to(ROOT))
    result["archive"] = {
        "original_receipt": str((destination / "original-receipt.json").relative_to(ROOT)),
        "original_receipt_sha256": sha(source),
        "source_snapshot": str(retained.relative_to(ROOT)),
        "raw_files_unchanged": True,
        "recorded_commands": "Original argv retained as execution provenance; only evidence locations remapped.",
    }
    (destination / "campaign.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps({"archive": str(destination.relative_to(ROOT)), "logs": len(names),
                      "source_files": len(original["source_sha256"]), "original_sha256": sha(source)}))


if __name__ == "__main__":
    main()

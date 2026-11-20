"""Derive the public layout replay input from one archived regime.

Run at the repository root with --check to verify the shipped bytes. No host
observations or summarised interpretation enter this extract.
"""
import argparse
import hashlib
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[3]
LAB = ROOT / "labs/performance_followthrough"
sys.path.insert(0, str(LAB / "dossiers"))
from run_dossier import gate_row

ARCHIVE = ROOT / "reviews/v5/expansion/polish_revision/dossiers/representation_locality"
PUBLIC = LAB / "dossiers/replay/layout.json"
REGIME = "n=2048: soa (0) vs aos64 (1)"
GROUP = "fills"


def extract():
    def read(name):
        return json.loads((ARCHIVE / name).read_text())

    spec, events, build = read("spec.json"), read("events.json")["events"], read("build.json")
    stream = (ARCHIVE / "rows.jsonl").read_bytes()
    rows, trials = [], {}
    for line in stream.splitlines():
        entry = json.loads(line)
        if (entry["regime"], entry["group"]) != (REGIME, GROUP):
            continue
        driver = entry["driver"]
        window = driver["window"]
        row = gate_row(spec["row"], driver)
        row.update(index=entry["index"], pair=entry["pair"], order=entry["order"], variant=entry["variant"],
                   raw=window["raw"], ids=window["ids"], raw_instructions=window["raw"][0],
                   raw_cycles=window["raw"][1], cpu_before=window["cpu_start"], cpu_after=window["cpu_end"])
        if entry["phase"] == "calibration":
            trials.setdefault(entry["repetitions"], []).append(row)
        else:
            rows.append(row)
    return {"schema_version": 2, "kind": "native-run", "provenance": "ARCHIVED AUTHOR REPLAY",
            "notice": "Historical author hardware observation supplied for analysis.",
            "source": {"path": str(ARCHIVE.relative_to(ROOT) / "rows.jsonl"),
                       "sha256": hashlib.sha256(stream).hexdigest(), "regime": REGIME, "group": GROUP},
            "build": {"binary_sha256": build["binary_sha256"], "flags": build["flags"]},
            "host": read("environment.json")["host"], "request": {"family": "layout", "n": 2048, "pairs": 31},
            "variants": ["soa", "aos64"],
            "events": [{"name": name, "encoding": events[name]["encoding"], "config": events[name]["config"]}
                       for name in spec["groups"][GROUP]],
            "calibration": {"attempts": [{"repetitions": rep, "trials": items} for rep, items in trials.items()]},
            "rows": rows}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    data = json.dumps(extract(), separators=(",", ":"), ensure_ascii=False) + "\n"
    if args.check:
        sys.exit(0 if PUBLIC.read_text() == data else "layout replay differs from source rows")
    PUBLIC.parent.mkdir(parents=True, exist_ok=True)
    PUBLIC.write_text(data)

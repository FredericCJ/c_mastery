"""Verify archived raw PMU evidence and derive bounded descriptive summaries."""
import argparse
from collections import Counter
import csv
from fractions import Fraction
import hashlib
import json
from pathlib import Path
import re
import statistics
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def sample_attribution(data, archive):
    """Resolve executable samples through recorded maps and ELF LOAD segments."""
    commands = {r["label"]: r for r in data["commands"]}
    records = []
    for line in (ROOT / commands["sampled-records"]["log"]).read_text().splitlines():
        if line.startswith("{"):
            records.append(json.loads(line))
    binary = ROOT / data["binary"]
    command = ["readelf", "-lW", str(binary)]
    elf = subprocess.run(command, text=True, capture_output=True, check=True).stdout
    evidence = archive / "attribution-elf-loads.txt"
    evidence.write_text(elf)
    loads = []
    for line in elf.splitlines():
        fields = line.split()
        if fields and fields[0] == "LOAD":
            loads.append(tuple(int(fields[i], 16) for i in (1, 2, 4)))
    maps = []
    for row in records:
        if row.get("kind") != "mapping":
            continue
        fields = row["line"].split()
        if len(fields) >= 6 and fields[-1] == data["original_binary"] and "x" in fields[1]:
            start, end = (int(v, 16) for v in fields[0].split("-"))
            maps.append((start, end, int(fields[2], 16)))
    instructions = {}
    function = None
    for line in (ROOT / commands["disassembly"]["log"]).read_text().splitlines():
        symbol = re.match(r"^[0-9a-f]+ <([^>]+)>:$", line)
        if symbol:
            function = symbol[1]
        instruction = re.match(r"^\s*([0-9a-f]+):\s+", line)
        if instruction:
            instructions[int(instruction[1], 16)] = (function, line.strip())
    counts = Counter()
    unmapped = 0
    for row in records:
        if row.get("kind") != "sample":
            continue
        matches = [offset + row["ip"] - start for start, end, offset in maps if start <= row["ip"] < end]
        addresses = [virtual + matches[0] - offset for offset, virtual, size in loads
                     if matches and offset <= matches[0] < offset + size]
        if len(addresses) != 1 or addresses[0] not in instructions:
            unmapped += 1
            continue
        counts[addresses[0]] += 1
    return {"method": "recorded mapping file offset, then matching ELF LOAD virtual address, then exact disassembly address",
            "load_evidence": {"command": command, "log": str(evidence.relative_to(ROOT)), "sha256": sha(evidence)},
            "main_binary_samples": sum(counts.values()), "unattributed_samples": unmapped,
            "sites": [{"elf_address": hex(address), "samples": count,
                       "symbol": instructions[address][0], "disassembly": instructions[address][1]}
                      for address, count in counts.most_common()],
            "limit": "precise_ip=0 permits skid; a sampled instruction is not proof of the causal instruction or stall reason."}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--archive", type=Path, required=True)
    args = parser.parse_args()
    archive = args.archive.resolve()
    report = archive / "campaign.json"
    data = json.loads(report.read_text())
    for row in data["commands"]:
        if sha(ROOT / row["log"]) != row["sha256"]:
            raise ValueError("log no longer matches: " + row["label"])
    original = ROOT / data["archive"]["original_receipt"]
    if sha(original) != data["archive"]["original_receipt_sha256"]:
        raise ValueError("original receipt changed")
    snapshot = ROOT / data["archive"]["source_snapshot"]
    for name, expected in data["source_sha256"].items():
        if sha(snapshot / name) != expected:
            raise ValueError("source snapshot changed: " + name)
    if sha(ROOT / data["binary"]) != data["binary_sha256"]:
        raise ValueError("binary changed")
    rows = [r for r in data["observations"] if "pair" in r]
    summaries = []
    for family, n in sorted({(r["family"], r["n"]) for r in rows}):
        group = [r for r in rows if r["family"] == family and r["n"] == n]
        paired = {}
        for row in group:
            if row["status"] != "measured" or not 0 < row["running"] <= row["enabled"]:
                raise ValueError("invalid observation in measured campaign")
            paired.setdefault(row["pair"], {})[row["variant"]] = row
        if set(paired) != set(range(31)) or any(set(v) != {0, 1} for v in paired.values()):
            raise ValueError("full comparison requires 31 complete pairs")
        calibration = [r for r in data["calibration"] if r["family"] == family and r["n"] == n][-1]
        entry = {"family": family, "n": n, "pairs": len(paired),
                 "calibration_sufficient": calibration["sufficient"],
                 "calibration_target_ns": calibration["target_ns"],
                 "endpoint_migrations": sum(r["cpu_before"] != r["cpu_after"] for r in group),
                 "minimum_running_fraction": str(min(Fraction(r["running"], r["enabled"]) for r in group))}
        for variant in range(2):
            samples = [r for r in group if r["variant"] == variant]
            entry[f"v{variant}_median_ns"] = statistics.median(r["elapsed_ns"] / r["repetitions"] for r in samples)
            for name in ("instructions", "cycles"):
                entry[f"v{variant}_median_{name}"] = statistics.median(
                    float(Fraction(r["raw_" + name] * r["enabled"], r["running"] * r["repetitions"])) for r in samples)
            entry[f"v{variant}_median_cpi"] = statistics.median(
                r["raw_cycles"] / r["raw_instructions"] for r in samples if r["raw_instructions"])
        differences = [pair[1]["elapsed_ns"] / pair[1]["repetitions"] -
                       pair[0]["elapsed_ns"] / pair[0]["repetitions"] for pair in paired.values()]
        entry["median_paired_delta_ns"] = statistics.median(differences)
        entry["v1_faster_pairs"] = sum(d < 0 for d in differences)
        summaries.append(entry)
    sampled = next(r for r in data["observations"] if r.get("kind") == "sampling")
    standard = data["environment"].get("standard_perf_comparison", {})
    analysis = {
        "schema_version": 1, "campaign_sha256": sha(report), "producer_sha256": sha(Path(__file__)),
        "raw_integrity_verified": True, "sampled_records": sampled,
        "sample_attribution": sample_attribution(data, archive),
        "standard_comparison_valid": bool(standard.get("parsed_counters")),
        "standard_comparison": standard,
        "summaries": summaries,
        "diagnosis": "Author interpretation is in interpretation.md; original campaign gates remain immutable.",
        "independent_assessment": "not-assessed",
    }
    (archive / "analysis.json").write_text(json.dumps(analysis, indent=2) + "\n")
    with (archive / "summary.csv").open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(summaries[0]))
        writer.writeheader()
        writer.writerows(summaries)
    print(json.dumps({"archive": str(archive.relative_to(ROOT)), "paired_regimes": len(summaries),
                      "hardware_rows": len(rows), "raw_integrity_verified": True,
                      "standard_comparison_valid": analysis["standard_comparison_valid"],
                      "calibration_insufficient": [(r["family"], r["n"]) for r in summaries if not r["calibration_sufficient"]]}))


if __name__ == "__main__":
    main()

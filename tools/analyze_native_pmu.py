"""Verify recorded raw PMU evidence and derive bounded, gated summaries.

Reads either an archived historical campaign (schema 1, with an `archive`
section naming its source snapshot) or a campaign written by the current
tools/run_native_pmu.py (schema 2, with `source_snapshot`). Integrity is
checked first: every log, the source snapshot and the binary must still match
their recorded hashes.

Every paired regime is then judged by tools/measurement_gates.py: acquisition,
scheduling, timing adequacy, comparability and interpretation. Rankings and
per-variant medians are produced only for `supported` regimes; inconclusive or
refused regimes keep their gate states and reasons, and their raw rows stay in
the campaign. Old receipts are read, never rewritten: the analysis goes to a
fresh --output directory (default: the archive directory, only when it holds
no analysis yet).
"""
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

import measurement_gates as gates

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def resolve(name):
    return ROOT / name  # absolute paths stay absolute


def sample_attribution(data, output):
    """Resolve executable samples through recorded maps and ELF LOAD segments."""
    commands = {r["label"]: r for r in data["commands"]}
    records = []
    for line in resolve(commands["sampled-records"]["log"]).read_text().splitlines():
        if line.startswith("{"):
            records.append(json.loads(line))
    binary = resolve(data["binary"])
    original = data.get("original_binary", data["binary"])
    command = ["readelf", "-lW", str(binary)]
    elf = subprocess.run(command, text=True, capture_output=True, check=True).stdout
    evidence = output / "attribution-elf-loads.txt"
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
        if len(fields) >= 6 and fields[-1] == original and "x" in fields[1]:
            start, end = (int(v, 16) for v in fields[0].split("-"))
            maps.append((start, end, int(fields[2], 16)))
    instructions = {}
    function = None
    for line in resolve(commands["disassembly"]["log"]).read_text().splitlines():
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
            "load_evidence": {"command": command, "log": str(evidence), "sha256": sha(evidence)},
            "main_binary_samples": sum(counts.values()), "unattributed_samples": unmapped,
            "sites": [{"elf_address": hex(address), "samples": count,
                       "symbol": instructions[address][0], "disassembly": instructions[address][1]}
                      for address, count in counts.most_common()],
            "limit": "precise_ip=0 permits skid; a sampled instruction is not proof of the causal instruction or stall reason."}


def verify_integrity(data, report):
    for row in data["commands"]:
        if sha(resolve(row["log"])) != row["sha256"]:
            raise ValueError("log no longer matches: " + row["label"])
    if "archive" in data:
        original = resolve(data["archive"]["original_receipt"])
        if sha(original) != data["archive"]["original_receipt_sha256"]:
            raise ValueError("original receipt changed")
        snapshot = resolve(data["archive"]["source_snapshot"])
    elif data.get("source_snapshot"):
        snapshot = resolve(data["source_snapshot"])
    else:
        raise ValueError("campaign has no source snapshot to verify against")
    for name, expected in data["source_sha256"].items():
        if sha(snapshot / name) != expected:
            raise ValueError("source snapshot changed: " + name)
    if sha(resolve(data["binary"])) != data["binary_sha256"]:
        raise ValueError("binary changed")
    return {"campaign_sha256": sha(report), "logs": len(data["commands"]), "source_snapshot": str(snapshot)}


def calibration_for(data, family, n):
    """Re-derive a regime's calibration from its raw trials with the current
    gate rules. Schema 2 stores one entry per regime with its attempts;
    schema 1 stored one entry per attempt. Stored targets and verdicts are
    never trusted: they may come from an older, weaker rule."""
    entries = [r for r in data["calibration"] if r["family"] == family and r["n"] == n]
    if not entries:
        raise ValueError(f"no calibration for {family} {n}")
    attempts = entries[-1]["attempts"] if "attempts" in entries[-1] else entries
    if not all("trials" in a for a in attempts):
        # The sweep's placeholder when hardware was blocked: nothing was
        # calibrated, so nothing can be re-derived and timing cannot pass.
        return dict(gates.calibration_state([dict(a, min_elapsed_ns=0) for a in attempts], max_repetitions=None),
                    sufficient=False, rederived=False)
    state = gates.rebuild_calibration([{"repetitions": a["repetitions"], "trials": a["trials"]} for a in attempts],
                                      max_repetitions=gates.MAX_REPETITIONS)
    return dict(state, rederived=True)


def descriptive(group):
    """Per-variant medians: computed only for supported regimes."""
    entry = {"minimum_running_fraction": str(min(Fraction(r["running"], r["enabled"]) for r in group))}
    for variant in range(2):
        samples = [r for r in group if r["variant"] == variant]
        entry[f"v{variant}_median_ns"] = statistics.median(r["elapsed_ns"] / r["repetitions"] for r in samples)
        for name in ("instructions", "cycles"):
            entry[f"v{variant}_median_{name}"] = statistics.median(
                float(Fraction(r["raw_" + name] * r["enabled"], r["running"] * r["repetitions"])) for r in samples)
        cpis = [r["raw_cycles"] / r["raw_instructions"] for r in samples if r["raw_instructions"]]
        entry[f"v{variant}_median_cpi"] = statistics.median(cpis) if cpis else None
    return entry


def summarize(data):
    rows = [r for r in data["observations"] if "pair" in r and r.get("kind") == "count"]
    summaries = []
    for family, n in sorted({(r["family"], r["n"]) for r in rows}):
        group = [r for r in rows if r["family"] == family and r["n"] == n]
        calibration = calibration_for(data, family, n)
        evaluation = gates.evaluate_regime(group, calibration, required_pairs=gates.REQUIRED_PAIRS)
        entry = {"family": family, "n": n, "pairs": evaluation["pairs"],
                 "repetitions": calibration["repetitions"],
                 "calibration_sufficient": calibration["sufficient"],
                 "calibration_exhausted": calibration["exhausted"],
                 "calibration_target_ns": calibration["target_ns"],
                 "endpoint_migrations": sum(r["cpu_before"] != r["cpu_after"] for r in group),
                 "gates": evaluation["gates"], "reasons": evaluation["reasons"]}
        if "ranking" in evaluation:
            entry["ranking"] = evaluation["ranking"]
            entry["descriptive"] = descriptive(group)
        summaries.append(entry)
    return summaries


def csv_rows(summaries):
    for s in summaries:
        ranking = s.get("ranking", {})
        yield {"family": s["family"], "n": s["n"], "pairs": s["pairs"], "repetitions": s["repetitions"],
               **{"gate_" + k: v for k, v in s["gates"].items()},
               "median_paired_delta_ns": ranking.get("median_paired_delta_ns", ""),
               "v1_faster_pairs": ranking.get("v1_faster_pairs", "")}


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--archive", type=Path, required=True, help="directory holding campaign.json")
    parser.add_argument("--output", type=Path, help="fresh directory for analysis.json (default: --archive)")
    args = parser.parse_args()
    archive = args.archive.resolve()
    output = (args.output or archive).resolve()
    if (output / "analysis.json").exists():
        parser.error("an analysis already exists there; receipts are immutable, choose a fresh --output")
    report = archive / "campaign.json"
    data = json.loads(report.read_text())
    integrity = verify_integrity(data, report)
    output.mkdir(parents=True, exist_ok=True)
    summaries = summarize(data)
    sampled = next(r for r in data["observations"] if r.get("kind") == "sampling")
    standard = data["environment"].get("standard_perf_comparison", {})
    interpretation = Counter(s["gates"]["interpretation"] for s in summaries)
    analysis = {
        "schema_version": 2, "campaign_schema_version": data.get("schema_version", 1),
        "campaign_sha256": integrity["campaign_sha256"], "producer_sha256": sha(Path(__file__)),
        "measurement_gates_sha256": sha(gates.SOURCE),
        "raw_integrity_verified": True, "integrity": integrity, "sampled_records": sampled,
        "sample_attribution": sample_attribution(data, output),
        "standard_comparison_valid": bool(standard.get("parsed_counters")),
        "standard_comparison": standard,
        "saturation": data.get("saturation"),
        "summaries": summaries,
        "interpretation_counts": dict(interpretation),
        "diagnosis": "Author interpretation belongs in a separate dossier; campaign gates remain immutable.",
        "independent_assessment": "not-assessed",
    }
    (output / "analysis.json").write_text(json.dumps(analysis, indent=2) + "\n")
    rows = list(csv_rows(summaries))
    with (output / "summary.csv").open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]) if rows else ["family"])
        writer.writeheader()
        writer.writerows(rows)
    print(json.dumps({"analysis": str(output / "analysis.json"), "paired_regimes": len(summaries),
                      "interpretation": dict(interpretation), "raw_integrity_verified": True,
                      "standard_comparison_valid": analysis["standard_comparison_valid"],
                      "calibration_insufficient": [(s["family"], s["n"]) for s in summaries
                                                   if not s["calibration_sufficient"]]}))


if __name__ == "__main__":
    main()

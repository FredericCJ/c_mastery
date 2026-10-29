"""Public deterministic protocol checks and separately gated native PMU observations.

No kernel policy, capability, governor or system affinity changes. --hardware-only
uses an already reviewed binary; it performs no compilation or package install.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys
import statistics
from fractions import Fraction
from datetime import datetime, timezone

ROOT = Path(__file__).resolve().parents[1]
LAB = ROOT / "labs/performance_followthrough"
REF = ROOT / "assessor/performance_followthrough"


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def preflight():
    paths = ["/proc/sys/kernel/perf_event_paranoid", "/proc/self/status",
             "/proc/self/cgroup", "/proc/sys/kernel/perf_event_mlock_kb",
             "/sys/devices/system/cpu/online"]
    result = {"platform": platform.platform(), "uid": os.getuid(),
              "affinity": sorted(os.sched_getaffinity(0)), "files": {}, "pmus": {}}
    for name in paths:
        try:
            result["files"][name] = Path(name).read_text()
        except OSError as error:
            result["files"][name] = {"unavailable": str(error)}
    for pmu in sorted(Path("/sys/bus/event_source/devices").glob("*")):
        fields = {}
        for path in [pmu / "type", pmu / "cpumask", *pmu.glob("events/*"),
                     *pmu.glob("format/*"), *pmu.glob("caps/*")]:
            try:
                fields[str(path.relative_to(pmu))] = path.read_text().strip()
            except OSError:
                pass
        result["pmus"][pmu.name] = fields
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--report", type=Path, required=True)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--protocol-only", action="store_true")
    mode.add_argument("--hardware-only", action="store_true")
    parser.add_argument("--binary", type=Path)
    parser.add_argument("--sanitizers", action="store_true")
    parser.add_argument("--pairs", type=int, default=31)
    args = parser.parse_args()
    if not 1 <= args.pairs <= 31:
        parser.error("pairs must be 1..31; fewer than 31 is a smoke observation")
    if args.hardware_only and (not args.binary or not args.binary.is_file()):
        parser.error("hardware-only requires an existing --binary")
    report = args.report.resolve()
    if report.exists():
        parser.error("report exists; use a fresh immutable receipt path")
    out = report.with_suffix("")
    out.mkdir(parents=True, exist_ok=False)
    if args.binary and not args.hardware_only:
        args.binary.resolve().parent.mkdir(parents=True, exist_ok=True)
    commands = []
    checks = []
    observations = []
    calibration = []
    sources = [Path(__file__), *LAB.glob("pmu*"), LAB / "publication.h",
               LAB / "publication.c", REF / "pmu.c",
               ROOT / "assessor/algorithms/algo.c", ROOT / "labs/algorithms/include/algo.h",
               ROOT / "labs/corpus_events/events.h", ROOT / "assessor/corpus_events/events.c",
               ROOT / "labs/hosted_systems/timer_depth/collection.h",
               ROOT / "labs/hosted_systems/timer_depth/timer.h",
               ROOT / "assessor/hosted_systems/timer_depth/collection.c",
               ROOT / "assessor/hosted_systems/timer_depth/timer.c"]
    source_hashes = {str(p.relative_to(ROOT)): sha(p) for p in sources if p.is_file()}
    gates = {"protocol": "not-run", "hardware_counting": "not-run",
             "hardware_sampling": "not-run", "microarchitectural_diagnosis": "not-assessed",
             "independent_learner": "not-assessed"}

    def run(label, command, expected=0, timeout=60):
        command = list(map(str, command))
        result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=timeout)
        log = out / (label + ".txt")
        log.write_text(result.stdout + result.stderr)
        commands.append({"label": label, "command": command, "exit_code": result.returncode,
                         "expected_exit_code": expected, "log": str(log), "sha256": sha(log)})
        if expected is not None and result.returncode != expected:
            raise RuntimeError(f"{label}: expected {expected}, got {result.returncode}: {result.stderr[-1000:]}")
        return result.stdout, result.stderr

    def check(label, condition):
        checks.append({"name": label, "passed": bool(condition)})
        if not condition:
            raise AssertionError(label)

    error = None
    binary = args.binary.resolve() if args.binary else out / "native"
    common = ["-std=c17", "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-I" + str(LAB)]
    environment = {}
    try:
        if not args.hardware_only:
            profiles = [("debug", ["-O0", "-g"]), ("optimized", ["-O2", "-DNDEBUG"])]
            if args.sanitizers:
                profiles.append(("asan-ubsan", ["-O1", "-g", "-fsanitize=address,undefined", "-fno-omit-frame-pointer"]))
            for cc in ("gcc", "clang"):
                run(cc + "-version", [cc, "--version"])
                for name, flags in profiles:
                    exe = out / (cc + "-" + name)
                    run(exe.name + "-build", [cc, *common, *flags, LAB / "pmu_tests.c", REF / "pmu.c", LAB / "publication.c", "-o", exe])
                    stdout, _ = run(exe.name, [exe])
                    check(exe.name + "-protocol", "PF_PROTOCOL_PASS" in stdout)
            exe = out / "learner"
            run("learner-build", ["gcc", *common, "-O0", LAB / "pmu_tests.c", LAB / "pmu.c", LAB / "publication.c", "-o", exe])
            stdout, stderr = run("learner-unsolved", [exe], expected=1)
            check("learner-semantic-rejection", "PF_PROTOCOL_ASSERT:" in stderr and "PF_PROTOCOL_PASS" not in stdout)
            reference = (REF / "pmu.c").read_text()
            mutations = {
                "never-ran-as-measured": ("p->state = p->running ? PF_PMU_MEASURED : PF_PMU_NEVER_RAN;", "p->state = PF_PMU_MEASURED;"),
                "lost-accounting": ("stats->lost += lost;", "stats->lost += 0;"),
                "duplicate-id": ("j == p->count || (seen & (1u << j))", "j == p->count"),
            }
            for name, (old, new) in mutations.items():
                check(name + "-anchor", reference.count(old) == 1)
                mutated = out / (name + ".c")
                mutated.write_text(reference.replace(old, new))
                exe = out / name
                run(name + "-build", ["gcc", *common, "-O0", LAB / "pmu_tests.c", mutated, LAB / "publication.c", "-o", exe])
                stdout, stderr = run(name, [exe], expected=1)
                check(name + "-semantic-rejection", "PF_PROTOCOL_ASSERT:" in stderr and "PF_PROTOCOL_PASS" not in stdout)
            gates["protocol"] = "passed"
            if not args.protocol_only:
                run("native-build", ["gcc", *common, "-O3", "-g", "-I" + str(ROOT / "labs/algorithms/include"),
                    LAB / "pmu_native.c", LAB / "pmu_linux.c", REF / "pmu.c",
                    ROOT / "assessor/algorithms/algo.c", LAB / "pmu_integrations.c",
                    "-I" + str(ROOT / "labs/corpus_events"), "-I" + str(ROOT / "labs/hosted_systems/timer_depth"),
                    ROOT / "assessor/corpus_events/events.c",
                    ROOT / "assessor/hosted_systems/timer_depth/collection.c",
                    ROOT / "assessor/hosted_systems/timer_depth/timer.c", "-pthread", "-lm", "-o", binary])
                binary.with_suffix(".build.json").write_text(json.dumps({
                    "binary_sha256": sha(binary), "source_sha256": source_hashes,
                    "build_command": commands[-1]["command"],
                    "compiler": shutil.which("gcc")}, indent=2) + "\n")
        if not args.protocol_only:
            build_record = json.loads(binary.with_suffix(".build.json").read_text())
            check("binary-matches-build", build_record["binary_sha256"] == sha(binary))
            check("binary-source-matches", build_record["source_sha256"] == source_hashes)
            environment = preflight()
            environment["build"] = build_record
            header = Path("/usr/include/linux/perf_event.h")
            environment["perf_event_header_sha256"] = sha(header) if header.is_file() else None
            for label, command in [("cpu", ["lscpu"]), ("virtualization", ["systemd-detect-virt"]),
                                   ("perf-version", ["perf", "--version"]), ("events", ["perf", "list", "--no-desc"]),
                                   ("symbols", ["nm", "-n", binary]), ("build-id", ["readelf", "-n", binary]),
                                   ("disassembly", ["objdump", "-d", binary])]:
                run(label, command, expected=None)
            run("mappings", ["cat", "/proc/self/maps"])
            raw, _ = run("capability", [binary, "count", "numeric", "1024", "0", "8"])
            capability = json.loads(raw)
            observations.append(capability)
            available = capability["status"] == "measured" and capability["running"] > 0
            gates["hardware_counting"] = "passed" if available else "blocked:" + capability["status"]
            sizes = {"search": [16, 1024, 65536], "layout": [256, 8192, 262144],
                     "numeric": [256, 8192, 1048576], "dispatch": [256, 8192, 262144],
                     "allocation": [16, 256, 4096], "sort": [16, 128, 1024],
                     "concurrency": [1024, 16384, 65536], "service": [8, 64, 1024]}
            pairs = args.pairs if available else 1
            for family, regimes in sizes.items():
                for n in regimes if available else regimes[:1]:
                    repetitions = 8
                    if available:
                        repetitions = 1
                        while True:
                            trials = []
                            for variant in range(2):
                                raw, _ = run(f"calibrate-{family}-{n}-{repetitions}-{variant}",
                                             [binary, "count", family, n, variant, repetitions])
                                row = json.loads(raw)
                                check(f"calibrate-{family}-{n}-{repetitions}-{variant}-valid", row["status"] == "measured" and row["running"] > 0)
                                trials.append(row)
                            target = max(50000, *(100 * r["clock_overhead_max_ns"] for r in trials),
                                         *(1000 * r["clock_resolution_ns"] for r in trials))
                            sufficient = min(r["elapsed_ns"] for r in trials) >= target
                            calibration.append({"family": family, "n": n, "repetitions": repetitions,
                                                "target_ns": target, "sufficient": sufficient, "trials": trials})
                            if sufficient or repetitions == 1024:
                                break
                            repetitions *= 2
                    for pair in range(pairs):
                        checksums = []
                        for order in range(2):
                            variant = order ^ (pair & 1)
                            label = f"{family}-{n}-{pair}-{variant}"
                            raw, _ = run(label, [binary, "count", family, n, variant, repetitions])
                            row = json.loads(raw)
                            row.update(pair=pair, order=order)
                            observations.append(row)
                            checksums.append(row["checksum"])
                            if available:
                                check(label + "-scheduled", row["status"] == "measured" and 0 < row["running"] <= row["enabled"] and row["instruction_id"] != row["cycle_id"] and not row["cleanup_errno"])
                        check(family + str(n) + str(pair) + "-equal-work", checksums[0] == checksums[1])
            raw, _ = run("sampled-records", [binary, "sample", "numeric", "65536", "0", "8"])
            records = [json.loads(line) for line in raw.splitlines()]
            sampling = records[-1]
            observations.append(sampling)
            gates["hardware_sampling"] = "passed" if sampling["status"] == "measured" else "blocked:" + sampling["status"]
            if available:
                # --delay=-1 creates disabled events. Work mode enables/disables
                # its task events around the same batch; separate runs may differ.
                raw, perf = run("standard-perf", ["perf", "stat", "--delay=-1", "-x,", "-e", "{instructions:u,cycles:u}", "--",
                                             binary, "work", "numeric", "65536", "0", "8"], expected=None)
                environment["standard_perf_comparison"] = {"output": raw, "counters": perf,
                    "same_workload_and_scope": True, "equality_required": False,
                    "interpretation": "Compare orders and ratios with variation; inspect scheduling percentages and verify nonzero events."}
            if args.pairs < 31 and available:
                gates["hardware_counting"] = "smoke-observed"
        check("source-stable", source_hashes == {str(p.relative_to(ROOT)): sha(p) for p in sources if p.is_file()})
    except (OSError, ValueError, AssertionError, RuntimeError, subprocess.TimeoutExpired) as exc:
        error = str(exc)
    summaries = []
    for family, n in sorted({(r["family"], r["n"]) for r in observations if "pair" in r}):
        rows = [r for r in observations if r.get("family") == family and r.get("n") == n and "pair" in r]
        result = {"family": family, "n": n, "pairs": len(rows) // 2,
                  "hardware_valid": all(r["status"] == "measured" and r["running"] > 0 for r in rows),
                  "interpretation": "Observation only; causal diagnosis requires the hypothesis dossier and disassembly review."}
        for variant in range(2):
            selected = [r for r in rows if r["variant"] == variant]
            if not selected:
                continue
            result[str(variant)] = {"elapsed_ns_per_repetition_median": statistics.median(
                r["elapsed_ns"] / r["repetitions"] for r in selected)}
            if result["hardware_valid"]:
                for row in selected:
                    row["derived"] = {name: str(Fraction(row["raw_" + name] * row["enabled"], row["running"])) for name in ("cycles", "instructions")}
                    row["derived"]["cpi"] = str(Fraction(row["raw_cycles"], row["raw_instructions"])) if row["raw_instructions"] else None
                    row["derived"]["running_fraction"] = str(Fraction(row["running"], row["enabled"]))
        paired = {r["pair"]: {} for r in rows}
        for row in rows:
            paired[row["pair"]][row["variant"]] = row["elapsed_ns"] / row["repetitions"]
        differences = [v[1] - v[0] for v in paired.values() if len(v) == 2]
        result["median_paired_elapsed_difference_ns"] = statistics.median(differences) if differences else None
        summaries.append(result)
    payload = {"schema_version": 1, "created_utc": datetime.now(timezone.utc).isoformat(),
               "passed": error is None, "error": error, "gates": gates, "checks": checks,
               "source_sha256": source_hashes, "commands": commands, "environment": environment,
               "observations": observations, "binary": str(binary) if binary.is_file() else None,
               "summaries": summaries, "calibration": calibration,
               "binary_sha256": sha(binary) if binary.is_file() else None,
               "host_policy_changed": False,
               "scope": "Author protocol checks and separately labelled hardware observations; no educational effectiveness claim",
               "sources": ["https://docs.kernel.org/userspace-api/perf_ring_buffer.html",
                           "https://docs.kernel.org/admin-guide/perf-security.html",
                           "https://man7.org/linux/man-pages/man2/perf_event_open.2.html"]}
    report.write_text(json.dumps(payload, indent=2) + "\n")
    print(json.dumps({"passed": payload["passed"], "error": error, "report": str(report), "gates": gates, "binary": payload["binary"]}))
    return 0 if error is None else 1


if __name__ == "__main__":
    raise SystemExit(main())

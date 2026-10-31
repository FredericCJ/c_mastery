#!/usr/bin/env python3
"""Run a diagnostic dossier: build, test, inspect, measure and analyze.

Supplied course infrastructure (dossiers/README.md). A dossier is described by
a spec file (for example concurrent_state/spec.json): the driver and test
sources, the symbols to disassemble, the placement premises, the events (by
`perf list` name and expected encoding), the counter groups, the variants and
the paired regimes, and the hypotheses written before measurement.

    python3 run_dossier.py test    --spec SPEC [--pmu-source PMU] [--sanitizers] --out FRESH_DIR
    python3 run_dossier.py run     --spec SPEC --pmu-source PMU [--test-implementation]
                                   --out FRESH_DIR [--sanitizers] [--pairs 31]
                                   [--quiet-marker PATH] [--max-load 1.5] [--max-wait-minutes 60]
    python3 run_dossier.py analyze --dir RECEIPT_DIR --output FRESH_JSON

`run` refuses to measure unless every correctness test passed, the build is
uninstrumented and every placement premise holds. Each row is one driver
process; each regime is calibrated and then measured in alternating A/B
pairs, and is judged by labs/support/measurement_gates.py (the same rules as
native.py and the author campaigns). `analyze` re-derives every gate and
estimate from the raw rows; a stored verdict is never trusted.

Only paths given on the command line are read: nothing here opens tools/ or
assessor/ by itself. No command changes host policy (perf_event_paranoid,
governors, frequency, isolation or interrupt affinity); placement is per thread
within the allowed CPU set.

Exit status: 0 completed (a completed run can still be inconclusive); 1 a
correctness test or a driver's contract failed; 2 invalid request, build
failure, refused build, unmet premise or host failure.
"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import json
import math
import os
from pathlib import Path
import platform
import re
import shutil
import signal
import statistics
import subprocess
import sys
import tempfile
import time

HERE = Path(__file__).resolve().parent
LAB = HERE.parent
SUPPORT = LAB.parent / "support"
sys.path.insert(0, str(SUPPORT))
sys.path.insert(0, str(LAB))
import measurement_gates as gates  # noqa: E402  (labs/support)
import native  # noqa: E402  (labs/performance_followthrough/native.py)

EXIT_CONTRACT = 1
EXIT_INVALID = 2
WARNINGS = ["-std=c17", "-Wall", "-Wextra", "-Wpedantic", "-Werror"]
COMMON_SOURCES = [HERE / "dossier_common.c", SUPPORT / "cm_parse.c", SUPPORT / "cm_clock.c",
                  LAB / "pmu_linux.c"]
COMMON_HEADERS = [HERE / "dossier_common.h", SUPPORT / "cm_parse.h", SUPPORT / "cm_clock.h", LAB / "pmu.h"]
ASAN = ["-O1", "-g", "-fsanitize=address,undefined", "-fno-sanitize-recover=all", "-fno-omit-frame-pointer"]
TSAN = ["-O1", "-g", "-fsanitize=thread"]
PMU_SYSFS = Path("/sys/bus/event_source/devices/cpu")
PERF_TYPE_RAW = 4
FULL_COVERAGE = 0.99  # share of the measured window the peer must cover
INSTRUCTIONS, CYCLES = "ex_ret_instr", "ls_not_halted_cyc"


class Refusal(Exception):
    """A premise of the run does not hold; carries the exit status."""

    def __init__(self, message, status=EXIT_INVALID):
        super().__init__(message)
        self.status = status


# ---------------------------------------------------------------------------
# pure functions (test_run_dossier.py)


def parse_encoding(text):
    """`cpu/event=0x43,umask=2/` -> {"event": 0x43, "umask": 2}."""
    match = re.fullmatch(r"cpu/([a-z]+=(?:0x[0-9a-fA-F]+|[0-9]+)(?:,[a-z]+=(?:0x[0-9a-fA-F]+|[0-9]+))*)/", text or "")
    if not match:
        raise ValueError(f"not a core-PMU encoding: {text!r}")
    fields = {}
    for item in match.group(1).split(","):
        name, value = item.split("=")
        if name in fields:
            raise ValueError(f"field {name} repeated in {text!r}")
        fields[name] = int(value, 0)
    return fields


def same_encoding(expected, fields):
    """Does the spec's expected encoding (`event=0x43,umask=0x02`) name
    exactly the fields perf list reports?"""
    return parse_encoding(f"cpu/{expected}/") == fields


def _format_bits(spec):
    """`config:0-7,32-35` -> [0..7, 32..35] (the PMU's sysfs format)."""
    match = re.fullmatch(r"config:([0-9,\-]+)", spec)
    if not match:
        raise ValueError(f"unsupported format {spec!r}")
    bits = []
    for part in match.group(1).split(","):
        low, _, high = part.partition("-")
        bits.extend(range(int(low), int(high or low) + 1))
    return bits


def config_from_fields(fields, formats):
    """Place each field's value into perf_event_attr.config as the PMU's
    sysfs format prescribes, low value bits first."""
    config = 0
    for name, value in fields.items():
        if name not in formats:
            raise ValueError(f"field {name!r} is not in the PMU format")
        bits = _format_bits(formats[name])
        if value >> len(bits):
            raise ValueError(f"{name}={value:#x} does not fit {formats[name]}")
        for index, bit in enumerate(bits):
            if value >> index & 1:
                config |= 1 << bit
    return config


def nearest_rank(values, quantile):
    ordered = sorted(values)
    if not ordered:
        raise ValueError("no values")
    return ordered[max(0, math.ceil(quantile * len(ordered)) - 1)]


def describe(values):
    values = list(values)
    if not values:
        return {"n": 0}
    return {"n": len(values), "median": statistics.median(values), "q1": nearest_rank(values, 0.25),
            "q3": nearest_rank(values, 0.75), "min": min(values), "max": max(values)}


def sign_test_interval(values, alpha=0.05):
    """Distribution-free interval for the median of `values` from order
    statistics: [x(k), x(n-k+1)] with the largest k whose two-sided binomial
    tail stays within alpha. Assumes independent observations."""
    ordered = sorted(values)
    n = len(ordered)
    tail, k = 0.0, 0
    for i in range(n + 1):
        tail_next = tail + math.comb(n, i) / 2 ** n
        if 2 * tail_next > alpha:
            break
        tail, k = tail_next, i + 1
    if k == 0:
        return {"low": None, "high": None, "coverage": None, "n": n,
                "note": "too few observations for a sign-test interval at this level"}
    return {"low": ordered[k - 1], "high": ordered[n - k], "coverage": 1 - 2 * tail, "n": n,
            "ranks": [k, n - k + 1]}


def _lookup(document, dotted):
    for key in dotted.split("."):
        document = document[key]
    return document


def gate_row(row_spec, driver):
    """The fields measurement_gates judges, from one driver line. Every
    active window must be measured; the least-scheduled window supplies
    running/enabled, so multiplexing in any thread is visible."""
    measured = _lookup(driver, row_spec["measured_window"])
    windows = []
    for path in row_spec["windows"]:
        owner = _lookup(driver, path.rsplit(".", 1)[0]) if "." in path else driver
        if owner.get("active", True):
            windows.append((path, _lookup(driver, path)))
    status = "measured"
    for path, w in windows:
        if w["state"] != "measured":
            status = w["state"] if w is measured else "peer:" + w["state"]
            break
    worst = min((w for _, w in windows), key=lambda w: (w["running"] / w["enabled"]) if w["enabled"] else -1)
    units = driver[row_spec["units"]] * (driver["repetitions"] if row_spec["units_times_repetitions"] else 1)
    cleanup = next((w["cleanup_errno"] for _, w in windows if w.get("cleanup_errno")), 0)
    ids = measured.get("ids") or []
    return {"status": status, "running": worst["running"], "enabled": worst["enabled"], "cleanup_errno": cleanup,
            "instruction_id": ids[0] if len(ids) > 0 else None, "cycle_id": ids[1] if len(ids) > 1 else None,
            "repetitions": driver["repetitions"], "elapsed_ns": measured["elapsed_ns"],
            "checksum": _lookup(driver, row_spec["checksum"]), "clock_resolution_ns": driver["clock_resolution_ns"],
            "clock_brackets_ns": driver["clock_brackets_ns"], "units": units}


def placement_holds(premise, topology, allowed):
    """Check one placement premise against sysfs topology and the allowed set."""
    cpus, relation = premise["cpus"], premise["relation"]
    if relation not in ("single", "smt-siblings", "different-core"):
        raise ValueError(f"unknown placement relation {relation!r}")
    if not all(c in allowed and c in topology for c in cpus):
        return False
    if relation == "single":
        return len(cpus) == 1
    if len(cpus) != 2 or cpus[0] == cpus[1]:
        return False
    siblings = cpus[1] in topology[cpus[0]]["siblings"]
    return siblings if relation == "smt-siblings" else not siblings


def coverage(measured, peer, tail_end):
    """Fraction of the measured window during which the peer was active
    (from its window start to the end of its tail)."""
    length = measured["end_ns"] - measured["start_ns"]
    if length <= 0:
        return None
    overlap = min(measured["end_ns"], tail_end) - max(measured["start_ns"], peer["start_ns"])
    return max(0, overlap) / length


def one_minute_load(text):
    fields = text.split()
    if not fields:
        raise ValueError("empty /proc/loadavg")
    return float(fields[0])


def busy_fractions(before, after):
    """Per-CPU busy fraction between two /proc/stat snapshots."""
    result = {}
    for cpu, old in before.items():
        new = after.get(cpu)
        if new is None:
            continue
        delta = [b - a for a, b in zip(old, new)]
        total = sum(delta)
        idle = delta[3] + (delta[4] if len(delta) > 4 else 0)
        result[cpu] = (total - idle) / total if total > 0 else None
    return result


# ---------------------------------------------------------------------------
# host facts (read only)


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def read(path):
    try:
        return Path(path).read_text().strip()
    except OSError as error:
        return f"unavailable: {error.strerror}"


def command_text(argv, timeout=60):
    if not shutil.which(argv[0]):
        return {"argv": argv, "unavailable": "not installed"}
    result = subprocess.run(argv, capture_output=True, text=True, timeout=timeout)
    return {"argv": argv, "exit_code": result.returncode, "stdout": result.stdout, "stderr": result.stderr[-4000:]}


def proc_stat():
    snapshot = {}
    for line in read("/proc/stat").splitlines():
        if re.match(r"cpu\d+ ", line):
            name, *values = line.split()
            snapshot[name] = [int(v) for v in values]
    return snapshot


def load_snapshot():
    return {**native.load_snapshot(), "stat": proc_stat()}


def topology():
    result = {}
    for cpu_dir in sorted(Path("/sys/devices/system/cpu").glob("cpu[0-9]*"), key=lambda p: int(p.name[3:])):
        siblings = read(cpu_dir / "topology/thread_siblings_list")
        if siblings.startswith("unavailable"):
            continue
        result[int(cpu_dir.name[3:])] = {"siblings": _cpu_list(siblings),
                                         "core_id": read(cpu_dir / "topology/core_id")}
    return result


def _cpu_list(text):
    cpus = set()
    for part in text.split(","):
        low, _, high = part.partition("-")
        cpus.update(range(int(low), int(high or low) + 1))
    return cpus


def environment():
    """Machine identity and measurement conditions, read only."""
    cpus = {}
    for cpu_dir in sorted(Path("/sys/devices/system/cpu").glob("cpu[0-9]*"), key=lambda p: int(p.name[3:])):
        entry = {name: read(cpu_dir / path) for name, path in (
            ("core_id", "topology/core_id"), ("siblings", "topology/thread_siblings_list"),
            ("scaling_driver", "cpufreq/scaling_driver"), ("scaling_governor", "cpufreq/scaling_governor"),
            ("energy_performance_preference", "cpufreq/energy_performance_preference"),
            ("cpuinfo_min_khz", "cpufreq/cpuinfo_min_freq"), ("cpuinfo_max_khz", "cpufreq/cpuinfo_max_freq"),
            ("scaling_min_khz", "cpufreq/scaling_min_freq"), ("scaling_max_khz", "cpufreq/scaling_max_freq"),
            ("scaling_cur_khz", "cpufreq/scaling_cur_freq"), ("microcode", "microcode/version"))}
        entry["caches"] = [{name: read(index / name) for name in
                            ("level", "type", "size", "coherency_line_size", "ways_of_associativity",
                             "shared_cpu_list")} for index in sorted(cpu_dir.glob("cache/index*"))]
        cpus[cpu_dir.name] = entry
    cpuinfo = read("/proc/cpuinfo").split("\n\n")[0]
    pmu = {str(p.relative_to(PMU_SYSFS)): read(p) for pattern in ("type", "events/*", "format/*", "caps/*",
                                                                  "perf_event_mux_interval_ms")
           for p in sorted(PMU_SYSFS.glob(pattern)) if p.is_file()}
    kernel_log = command_text(["journalctl", "-k", "-b", "--no-pager", "-o", "short-monotonic"])
    if "stdout" in kernel_log:
        kernel_log["stdout"] = "\n".join(line for line in kernel_log["stdout"].splitlines()
                                         if re.search(r"tsc|clocksource|Performance Events|perf:|microcode",
                                                      line, re.I))
    return {
        "utc": datetime.now(timezone.utc).isoformat(), "host": native.host_facts(),
        "platform": platform.platform(), "uid": os.getuid(), "allowed_cpus": sorted(os.sched_getaffinity(0)),
        "files": {path: read(path) for path in (
            "/proc/sys/kernel/perf_event_paranoid", "/proc/sys/kernel/nmi_watchdog",
            "/proc/sys/kernel/perf_event_max_sample_rate", "/proc/sys/kernel/perf_event_mlock_kb",
            "/sys/devices/system/clocksource/clocksource0/current_clocksource",
            "/sys/devices/system/clocksource/clocksource0/available_clocksource",
            "/sys/devices/system/cpu/amd_pstate/status", "/sys/devices/system/cpu/cpufreq/boost",
            "/sys/devices/system/cpu/smt/control", "/sys/kernel/mm/transparent_hugepage/enabled",
            "/sys/kernel/mm/transparent_hugepage/defrag", "/proc/cmdline", "/proc/version",
            "/etc/os-release")},
        "vulnerabilities": {p.name: read(p) for p in sorted(Path("/sys/devices/system/cpu/vulnerabilities").glob("*"))},
        "cpuinfo_cpu0": cpuinfo, "cpus": cpus, "core_pmu_sysfs": pmu,
        "lscpu": command_text(["lscpu"]), "lscpu_json": command_text(["lscpu", "-J"]),
        "uname": command_text(["uname", "-a"]), "kernel_log_clock_and_pmu": kernel_log,
        "tools": {name: command_text(argv) for name, argv in (
            ("gcc", ["gcc", "--version"]), ("clang", ["clang", "--version"]),
            ("objdump", ["objdump", "--version"]), ("perf", ["perf", "--version"]),
            ("ld", ["ld", "--version"]), ("glibc", ["ldd", "--version"]),
            ("gcc_configuration", ["gcc", "-v"]))},
        "python": sys.version,
    }


# ---------------------------------------------------------------------------
# events


def perf_list():
    if not shutil.which("perf"):
        return None
    result = subprocess.run(["perf", "list", "-j", "--details"], capture_output=True, text=True, timeout=120)
    if result.returncode:
        return None
    return {entry["EventName"]: entry for entry in json.loads(result.stdout)
            if entry.get("Unit") == "cpu" and "EventName" in entry}


def resolve_events(spec, listing):
    """Each spec event, resolved through `perf list` and the PMU's sysfs
    format. An absent event, a different encoding or a missing format is
    `unavailable`: never substituted by another event."""
    formats = {p.name: read(p) for p in sorted((PMU_SYSFS / "format").glob("*"))}
    pmu_type = read(PMU_SYSFS / "type")
    resolved = {}
    for name, event in spec["events"].items():
        entry = {"name": name, **event, "pmu_type": pmu_type}
        try:
            if pmu_type != str(PERF_TYPE_RAW):
                raise ValueError(f"core PMU type {pmu_type} is not PERF_TYPE_RAW")
            if listing is None:
                raise ValueError("perf list is unavailable on this host")
            if name not in listing:
                raise ValueError("perf list does not name this event for this CPU")
            fields = parse_encoding(listing[name]["Encoding"])
            if not same_encoding(event["encoding"], fields):
                raise ValueError(f"perf list encodes it as {listing[name]['Encoding']}, the spec expects "
                                 f"{event['encoding']}")
            entry.update(state="available", config=config_from_fields(fields, formats),
                         perf_list=listing[name], sysfs_format={k: formats[k] for k in fields})
        except ValueError as error:
            entry.update(state="unavailable", reason=str(error))
        resolved[name] = entry
    return resolved


def group_configs(spec, events, group):
    names = spec["groups"][group]
    if names[:2] != [INSTRUCTIONS, CYCLES]:
        raise Refusal(f"group {group} must begin with {INSTRUCTIONS} and {CYCLES}")
    if any(events[n]["state"] != "available" for n in names):
        return None
    return [events[n]["config"] for n in names]


# ---------------------------------------------------------------------------
# build, tests, disassembly


class Recorder:
    """Every command, its exit status and a hashed log file."""

    def __init__(self, out):
        self.out = out
        (out / "logs").mkdir(parents=True, exist_ok=True)
        # Test binaries are scratch: only the measured driver is kept.
        self.work = Path(tempfile.mkdtemp(prefix="dossier-tests-"))
        self.commands = []

    def run(self, label, argv, timeout=600, expected=0):
        argv = [str(a) for a in argv]
        started = time.monotonic()
        try:
            result = subprocess.run(argv, capture_output=True, text=True, timeout=timeout)
            code, stdout, stderr = result.returncode, result.stdout, result.stderr
        except subprocess.TimeoutExpired as expired:
            code, stdout, stderr = "timeout", str(expired.stdout or ""), str(expired.stderr or "")
        log = self.out / "logs" / f"{label}.txt"
        log.write_text(f"$ {' '.join(argv)}\n--- stdout\n{stdout}\n--- stderr\n{stderr}")
        self.commands.append({"label": label, "argv": argv, "exit_code": code, "expected_exit_code": expected,
                              "seconds": round(time.monotonic() - started, 3),
                              "log": str(log.relative_to(self.out)), "log_sha256": sha256(log)})
        return code, stdout, stderr

    def close(self):
        shutil.rmtree(self.work, ignore_errors=True)


def spec_paths(spec_path, spec, key):
    return [(spec_path.parent / name).resolve() for name in spec[key]["sources"]]


def includes(spec_path, pmu_source):
    return [f"-I{spec_path.parent}", f"-I{HERE}", f"-I{LAB}", f"-I{SUPPORT}"]


def build_driver(recorder, spec_path, spec, pmu_source, provenance):
    sources = [*spec_paths(spec_path, spec, "driver"), *COMMON_SOURCES, pmu_source]
    binary = recorder.out / "build" / spec["driver"]["name"]
    binary.parent.mkdir(parents=True, exist_ok=True)
    flags = [*WARNINGS, *gates.MEASUREMENT_CFLAGS, "-pthread"]
    argv = ["gcc", *flags, *includes(spec_path, pmu_source), *sources, "-o", binary]
    code, _, stderr = recorder.run("build-driver", argv)
    if code != 0:
        raise Refusal("driver build failed: " + stderr[-2000:])
    found = native.instrumented(binary)
    if found:
        raise Refusal("refused: the driver links a sanitizer runtime " + ", ".join(found))
    _, producer, _ = recorder.run("dwarf-producer", ["readelf", "--debug-dump=info", binary])
    producers = sorted(set(re.findall(r"DW_AT_producer\s*:\s*(?:\(indirect string, offset: 0x[0-9a-f]+\):\s*)?(.+)",
                                      producer)))
    headers = sorted({*COMMON_HEADERS, *(spec_path.parent / h for h in spec["driver"].get("headers", []))})
    return {"binary": str(binary.relative_to(recorder.out)), "binary_sha256": sha256(binary), "argv": [str(a) for a in argv],
            "flags": flags, "instrumented": False, "provenance": provenance, "dwarf_producers": producers,
            "sources": [{"path": str(p), "sha256": sha256(p)} for p in [*sources, *headers]]}


def run_tests(recorder, spec_path, spec, pmu_source, sanitizers):
    test = spec["tests"]
    sources = [*spec_paths(spec_path, spec, "tests"), *(COMMON_SOURCES if test.get("needs_support") else []),
               *([pmu_source] if test.get("needs_support") else [])]
    profiles = [("debug", ["-O0", "-g"]), ("optimized", ["-O2", "-g"])]
    if sanitizers:
        profiles.append(("asan-ubsan", ASAN))
        if test.get("threads"):
            profiles.append(("tsan", TSAN))
    results = []
    for compiler in ("gcc", "clang"):
        for profile, flags in profiles:
            label = f"test-{compiler}-{profile}"
            binary = recorder.work / label
            argv = [compiler, *WARNINGS, *flags, "-pthread", *includes(spec_path, pmu_source), *sources, "-o", binary]
            code, _, stderr = recorder.run(label + "-build", argv)
            if code != 0:
                results.append({"label": label, "outcome": "build-error", "exit_code": code})
                continue
            code, stdout, stderr = recorder.run(label, [binary], timeout=900)
            passed = code == 0 and test["pass_marker"] in stdout
            results.append({"label": label, "outcome": "passed" if passed else "failed", "exit_code": code,
                            "stdout": stdout[-1000:], "stderr": stderr[-1000:]})
    return {"sources": [{"path": str(p), "sha256": sha256(p)} for p in sources], "results": results,
            "passed": bool(results) and all(r["outcome"] == "passed" for r in results)}


def disassemble(recorder, binary, symbols):
    text = []
    for symbol in symbols:
        _, stdout, _ = recorder.run(f"objdump-{symbol}", ["objdump", "-d", "--no-show-raw-insn", "-l",
                                                          f"--disassemble={symbol}", binary])
        body = stdout.split("Disassembly of section .text:", 1)[-1].strip()
        text.append(f"=== {symbol}\n{body}\n")
    path = recorder.out / "disassembly.txt"
    path.write_text("\n".join(text))
    return {"file": "disassembly.txt", "sha256": sha256(path), "symbols": symbols}


# ---------------------------------------------------------------------------
# quiet window and measurement


class QuietWindow:
    """Create the marker, wait until the 1-minute load is below the limit,
    and remove the marker when the batch ends, also on error or signal."""

    def __init__(self, marker, max_load, max_wait_s, poll_s=30):
        self.marker, self.max_load, self.max_wait_s, self.poll_s = marker, max_load, max_wait_s, poll_s
        self.record = {"marker": str(marker) if marker else None, "max_load": max_load, "polls": []}
        self._previous = {}

    def _raise(self, signum, frame):
        raise SystemExit(f"signal {signum} during the quiet window")

    def __enter__(self):
        for signum in (signal.SIGTERM, signal.SIGHUP, signal.SIGINT):
            self._previous[signum] = signal.signal(signum, self._raise)
        self.record["marker_created_utc"] = datetime.now(timezone.utc).isoformat()
        if self.marker:
            Path(self.marker).parent.mkdir(parents=True, exist_ok=True)
            Path(self.marker).write_text(f"run_dossier.py pid {os.getpid()}\n")
        started = time.monotonic()
        while True:
            load = one_minute_load(read("/proc/loadavg"))
            self.record["polls"].append({"utc": datetime.now(timezone.utc).isoformat(), "load_1min": load})
            if load < self.max_load:
                break
            if time.monotonic() - started > self.max_wait_s:
                raise Refusal(f"load stayed at or above {self.max_load} for {self.max_wait_s} s")
            time.sleep(self.poll_s)
        self.record["waited_s"] = round(time.monotonic() - started, 1)
        self.record["measurement_start_utc"] = datetime.now(timezone.utc).isoformat()
        self._measure_start = time.monotonic()
        return self

    def __exit__(self, kind, value, trace):
        self.record["measurement_s"] = round(time.monotonic() - getattr(self, "_measure_start", time.monotonic()), 1)
        if self.marker:
            try:
                Path(self.marker).unlink()
            except FileNotFoundError:
                pass
        self.record["marker_removed_utc"] = datetime.now(timezone.utc).isoformat()
        self.record["ended_by"] = "completion" if kind is None else f"{kind.__name__}: {value}"
        for signum, handler in self._previous.items():
            signal.signal(signum, handler)
        return False


class Measurer:
    def __init__(self, binary, spec, rows_file):
        self.binary, self.spec, self.rows_file = binary, spec, rows_file
        self.count = 0

    def row(self, regime, group, configs, variant_name, variant, repetitions, phase, pair=None, order=None):
        argv = [str(self.binary), "measure", *self.spec["variants"][variant_name], str(repetitions),
                *map(str, configs)]
        before = load_snapshot()
        result = subprocess.run(argv, capture_output=True, text=True, timeout=600)
        after = load_snapshot()
        record = {"index": self.count, "regime": regime, "group": group, "phase": phase, "pair": pair,
                  "order": order, "variant": variant, "variant_name": variant_name, "repetitions": repetitions,
                  "argv": argv[1:], "exit_code": result.returncode, "stderr": result.stderr[-2000:],
                  "load_before": before, "load_after": after}
        self.count += 1
        lines = [line for line in result.stdout.splitlines() if line.startswith("{")]
        record["driver"] = json.loads(lines[-1]) if lines else None
        with self.rows_file.open("a") as stream:
            stream.write(json.dumps(record, separators=(",", ":")) + "\n")
        if result.returncode == EXIT_CONTRACT:
            raise Refusal(f"{variant_name}: the driver's correctness contract failed: {result.stderr[-400:]}",
                          EXIT_CONTRACT)
        if result.returncode != 0 or record["driver"] is None:
            raise Refusal(f"{variant_name}: driver exit {result.returncode}: {result.stderr[-400:]}")
        return dict(gate_row(self.spec["row"], record["driver"]), pair=pair, variant=variant, order=order)

    def regime(self, regime, group, configs, pairs):
        names = regime["variants"]

        def measure(repetitions):
            return [self.row(regime["name"], group, configs, names[v], v, repetitions, "calibration")
                    for v in (0, 1)]

        calibration = gates.calibrate(measure)
        if calibration.get("aborted"):
            return calibration
        for pair in range(pairs):
            for order in range(2):
                variant = order ^ (pair & 1)  # alternate which variant runs first
                self.row(regime["name"], group, configs, names[variant], variant, calibration["repetitions"],
                         "pair", pair, order)
        return calibration


# ---------------------------------------------------------------------------
# analysis (re-derived from raw rows)


def _per_unit(rows, index):
    return [r["driver_window"]["raw"][index] / r["units"] for r in rows]


def _row_view(spec, record):
    driver = record["driver"]
    row = dict(gate_row(spec["row"], driver), pair=record["pair"], variant=record["variant"], order=record["order"])
    row["driver_window"] = _lookup(driver, spec["row"]["measured_window"])
    row["load_1min_before"] = one_minute_load(record["load_before"]["loadavg"])
    row["load_1min_after"] = one_minute_load(record["load_after"]["loadavg"])
    row["busy"] = busy_fractions(record["load_before"]["stat"], record["load_after"]["stat"])
    windows = [_lookup(driver, p) for p in spec["row"]["windows"]]
    row["involuntary_switches"] = sum(max(0, w.get("involuntary_switches", 0)) for w in windows
                                      if w["state"] != "empty")
    row["migrated"] = any(w["cpu_start"] != w["cpu_end"] for w in windows if w["state"] != "empty")
    if spec["row"].get("peer_window") and _lookup(driver, spec["row"]["peer_window"].rsplit(".", 1)[0])["active"]:
        row["peer_window"] = _lookup(driver, spec["row"]["peer_window"])
    if "thread1" in driver and driver["thread1"]["active"]:
        row["peer_coverage"] = coverage(row["driver_window"], driver["thread1"]["window"],
                                        driver["thread1"]["tail_end_ns"])
        life = driver["lifecycle"]
        row["lifecycle_ns"] = life["join_return_ns"] - life["create_begin_ns"]
        row["thread_start_ns"] = life["peer_entry_ns"] - life["create_begin_ns"]
    elif "lifecycle" in driver:
        life = driver["lifecycle"]
        row["lifecycle_ns"] = life["join_return_ns"] - life["create_begin_ns"]
        row["thread_start_ns"] = life["peer_entry_ns"] - life["create_begin_ns"]
    if "setup_ns" in driver:
        row["setup_ns"] = driver["setup_ns"]
    return row


def carryover(spec, regime, records, previous):
    """Per variant, per-unit time and cycles of pair rows split by the
    variant of the row that ran just before (a different process). The
    pairing alternates order, so each variant follows itself in about half
    its rows; a difference between the halves is a carry-over effect through
    state the processes share (caches, DRAM, frequency), which the
    independence assumption of the intervals does not allow for."""
    result = {}
    for v in (0, 1):
        split = {"after_same_variant": [], "after_other_variant": [], "after_other_row": []}
        for record in records:
            if record["phase"] != "pair" or record["variant"] != v:
                continue
            before = previous.get(record["index"])
            if before is None or before["regime"] != record["regime"] or before["group"] != record["group"]:
                key = "after_other_row"
            else:
                key = "after_same_variant" if before["variant_name"] == record["variant_name"] else "after_other_variant"
            row = gate_row(spec["row"], record["driver"])
            window = _lookup(record["driver"], spec["row"]["measured_window"])
            split[key].append((row["elapsed_ns"] / row["units"],
                               window["raw"][1] / row["units"] if len(window.get("raw") or []) > 1 else None))
        result[regime["variants"][v]] = {
            key: {"rows": len(values), "elapsed_ns_per_unit": describe(t for t, _ in values),
                  "cycles_per_unit": describe(c for _, c in values if c is not None)}
            for key, values in split.items() if values}
    return result


def analyze_regime(spec, events, regime, group, records, previous=None):
    names = spec["groups"][group]
    attempts = {}
    for record in records:
        if record["phase"] == "calibration":
            attempts.setdefault(record["repetitions"], []).append(
                dict(gate_row(spec["row"], record["driver"]), pair=None, variant=record["variant"]))
    calibration = gates.rebuild_calibration([{"repetitions": reps, "trials": trials}
                                             for reps, trials in sorted(attempts.items())])
    rows = [_row_view(spec, r) for r in records if r["phase"] == "pair"]
    evaluation = gates.evaluate_regime(rows, calibration, required_pairs=gates.REQUIRED_PAIRS)
    result = {"regime": regime["name"], "group": group, "variants": regime["variants"], "events": names,
              "calibration": {k: v for k, v in calibration.items() if k != "attempts"},
              "gates": evaluation["gates"], "reasons": evaluation["reasons"], "pairs": evaluation["pairs"],
              "ranking": evaluation.get("ranking")}
    if not rows:
        return result
    result["host"] = {
        "cpu_busy_fraction_over_regime": busy_fractions(records[0]["load_before"]["stat"],
                                                        records[-1]["load_after"]["stat"]),
        "load_1min_max": max(max(r["load_1min_before"], r["load_1min_after"]) for r in rows),
        "seconds": None,
        "note": "Busy fractions include this runner and the measured driver; CPUs outside the placement show "
                "other host work. /proc/stat advances in 10 ms ticks."}
    try:
        first = datetime.fromisoformat(records[0]["load_before"]["utc"])
        last = datetime.fromisoformat(records[-1]["load_after"]["utc"])
        result["host"]["seconds"] = round((last - first).total_seconds(), 3)
    except (KeyError, ValueError):
        pass
    acquisition_valid = evaluation["gates"]["acquisition"] == "valid"
    if previous is not None and acquisition_valid:
        result["carryover"] = carryover(spec, regime, records, previous)
    per_variant = {}
    for v in (0, 1):
        mine = [r for r in rows if r["variant"] == v]
        summary = {"variant": regime["variants"][v], "rows": len(mine),
                   "elapsed_ns_per_unit": describe(r["elapsed_ns"] / r["units"] for r in mine),
                   "involuntary_switch_rows": sum(r["involuntary_switches"] > 0 for r in mine),
                   "migrated_rows": sum(r["migrated"] for r in mine),
                   "load_1min_before": describe(r["load_1min_before"] for r in mine)}
        if acquisition_valid:
            summary["events_per_unit"] = {name: describe(_per_unit(mine, i)) for i, name in enumerate(names)}
            peers = [r["peer_window"] for r in mine if r.get("peer_window")]
            if peers and len(peers) == len(mine):
                summary["peer_events_per_unit"] = {
                    name: describe(w["raw"][i] / r["units"] for w, r in zip(peers, mine))
                    for i, name in enumerate(names)}
            summary["user_cycles_per_ns"] = describe(r["driver_window"]["raw"][1] / r["elapsed_ns"] for r in mine)
            summary["instructions_per_cycle"] = describe(r["driver_window"]["raw"][0] / r["driver_window"]["raw"][1]
                                                         for r in mine)
        covered = [r["peer_coverage"] for r in mine if r.get("peer_coverage") is not None]
        if covered:
            summary["rows_peer_coverage_below_full"] = sum(c < FULL_COVERAGE for c in covered)
        for key in ("peer_coverage", "lifecycle_ns", "thread_start_ns", "setup_ns"):
            values = [r[key] for r in mine if r.get(key) is not None]
            if values:
                summary[key] = describe(values)
        per_variant[str(v)] = summary
    result["per_variant"] = per_variant
    paired = {}
    for r in rows:
        paired.setdefault(r["pair"], {})[r["variant"]] = r
    complete = [p for p in sorted(paired) if set(paired[p]) == {0, 1}]
    if acquisition_valid and evaluation["gates"]["scheduling"] == "full":
        result["counter_differences_v1_minus_v0_per_unit"] = {
            name: {"median": statistics.median(d), "interval": sign_test_interval(d)}
            for i, name in enumerate(names)
            for d in [[paired[p][1]["driver_window"]["raw"][i] / paired[p][1]["units"] -
                       paired[p][0]["driver_window"]["raw"][i] / paired[p][0]["units"] for p in complete]]}
        result["counter_differences_limit"] = ("Paired counter differences are observations of this host and build; "
                                               "they are not a timing ranking and correlated counters do not "
                                               "establish a cause.")
    if evaluation["gates"]["interpretation"] == "supported":
        differences = [paired[p][1]["elapsed_ns"] / paired[p][1]["units"] -
                       paired[p][0]["elapsed_ns"] / paired[p][0]["units"] for p in complete]
        def undisturbed(row):
            covered = row.get("peer_coverage")
            return row["involuntary_switches"] == 0 and (covered is None or covered >= FULL_COVERAGE)

        quiet = [p for p in complete if undisturbed(paired[p][0]) and undisturbed(paired[p][1])]
        result["timing_v1_minus_v0_ns_per_unit"] = {
            "median": statistics.median(differences), "interval": sign_test_interval(differences),
            "v1_faster_pairs": sum(d < 0 for d in differences), "pairs": len(differences),
            "without_disturbed_pairs": {
                "rule": f"both rows free of in-window involuntary switches and, with a peer, its activity "
                        f"covering at least {FULL_COVERAGE} of the measured window",
                "pairs": len(quiet),
                "median": statistics.median([differences[complete.index(p)] for p in quiet]) if quiet else None},
            "limit": "Descriptive for this host, build and placement; the interval assumes independent pairs. "
                     "No population, universal-ordering or WCET claim."}
    return result


def analyze(receipt):
    spec = json.loads((receipt / "spec.json").read_text())
    events = json.loads((receipt / "events.json").read_text())["events"]
    records = [json.loads(line) for line in (receipt / "rows.jsonl").read_text().splitlines() if line.strip()]
    previous = {record["index"]: before for before, record in zip(records, records[1:])}
    regimes = []
    for regime in spec["regimes"]:
        for group in regime["groups"]:
            mine = [r for r in records if r["regime"] == regime["name"] and r["group"] == group]
            if mine:
                regimes.append(analyze_regime(spec, events, regime, group, mine, previous))
    return {"schema_version": 1, "kind": "dossier-analysis", "dossier": spec["dossier"],
            "created_utc": datetime.now(timezone.utc).isoformat(),
            "rules": {"gates": str(Path(gates.SOURCE).resolve()), "gates_sha256": sha256(gates.SOURCE),
                      "required_pairs": gates.REQUIRED_PAIRS, "runner_sha256": sha256(__file__)},
            "rows": len(records), "regimes": regimes,
            "hypotheses": spec.get("hypotheses", []),
            "limits": ["Raw rows are the evidence; every value here is derived from rows.jsonl by this script.",
                       "Calling-thread user-space counts only (exclude_kernel, exclude_hv); peers count their own.",
                       "Rankings exist only for regimes whose five gates all passed.",
                       "Sign-test intervals assume independent pairs; the carry-over section shows where "
                       "consecutive processes were not independent."]}


# ---------------------------------------------------------------------------
# commands


def fresh_dir(path):
    out = Path(path).resolve()
    if out.exists() and any(out.iterdir()):
        raise Refusal(f"{out} is not empty; receipts are never overwritten")
    out.mkdir(parents=True, exist_ok=True)
    return out


def write_json(path, value):
    Path(path).write_text(json.dumps(value, indent=1, sort_keys=False) + "\n")


def command_test(args):
    spec_path = Path(args.spec).resolve()
    spec = json.loads(spec_path.read_text())
    out = fresh_dir(args.out)
    recorder = Recorder(out)
    try:
        tests = run_tests(recorder, spec_path, spec, Path(args.pmu_source).resolve(), args.sanitizers)
    finally:
        recorder.close()
    write_json(out / "tests.json", {**tests, "commands": recorder.commands})
    print(json.dumps({"passed": tests["passed"], "results": [(r["label"], r["outcome"]) for r in tests["results"]]}))
    return 0 if tests["passed"] else EXIT_CONTRACT


def command_run(args):
    out = fresh_dir(args.out)
    recorder = Recorder(out)
    try:
        return run_campaign(args, out, recorder)
    finally:
        recorder.close()


def run_campaign(args, out, recorder):
    spec_path = Path(args.spec).resolve()
    spec = json.loads(spec_path.read_text())
    pmu_source = Path(args.pmu_source).resolve()
    provenance = native.require_label([pmu_source, *spec_paths(spec_path, spec, "driver")],
                                      test_implementation=args.test_implementation)
    started = datetime.now(timezone.utc).isoformat()
    write_json(out / "spec.json", spec)
    write_json(out / "environment.json", environment())
    listing = perf_list()
    events = resolve_events(spec, listing)
    write_json(out / "events.json", {"perf_version": command_text(["perf", "--version"]), "events": events})
    tests = run_tests(recorder, spec_path, spec, pmu_source, args.sanitizers)
    write_json(out / "tests.json", tests)
    if not tests["passed"]:
        raise Refusal("correctness tests failed; nothing is measured", EXIT_CONTRACT)
    build = build_driver(recorder, spec_path, spec, pmu_source, provenance)
    write_json(out / "build.json", build)
    binary = out / build["binary"]
    disassembly = disassemble(recorder, binary, spec["disassemble"])
    topo, allowed = topology(), set(os.sched_getaffinity(0))
    premises = [{**p, "holds": placement_holds(p, topo, allowed)} for p in spec["placements"]]
    if not all(p["holds"] for p in premises):
        raise Refusal("placement premise does not hold: " + json.dumps(premises))
    plan = []
    for regime in spec["regimes"]:
        if args.regime and regime["name"] not in args.regime:
            continue
        for group in regime["groups"]:
            configs = group_configs(spec, events, group)
            plan.append((regime, group, configs))
    measurer = Measurer(binary, spec, out / "rows.jsonl")
    outcomes = []
    with QuietWindow(args.quiet_marker, args.max_load, args.max_wait_minutes * 60) as quiet:
        for regime, group, configs in plan:
            if configs is None:
                outcomes.append({"regime": regime["name"], "group": group, "state": "unavailable-events"})
                continue
            calibration = measurer.regime(regime, group, configs, args.pairs)
            outcomes.append({"regime": regime["name"], "group": group,
                             "state": "acquisition-aborted" if calibration.get("aborted") else "measured",
                             "repetitions": calibration["repetitions"]})
    analysis = analyze(out)
    write_json(out / "analysis.json", analysis)
    files = {str(p.relative_to(out)): sha256(p) for p in sorted(out.rglob("*"))
             if p.is_file() and p.name != "campaign.json"}
    write_json(out / "campaign.json", {
        "schema_version": 1, "kind": "dossier-campaign", "dossier": spec["dossier"], "provenance": provenance,
        "notice": native.TEST_NOTICE if provenance == native.PROVENANCE_TEST else None,
        "started_utc": started, "finished_utc": datetime.now(timezone.utc).isoformat(),
        "runner": {"path": str(Path(__file__).resolve()), "sha256": sha256(__file__), "argv": sys.argv},
        "spec": {"path": str(spec_path), "sha256": sha256(spec_path)}, "pmu_source": {
            "path": str(pmu_source), "sha256": sha256(pmu_source)},
        "binary_sha256": build["binary_sha256"], "disassembly": disassembly, "placements": premises,
        "pairs_requested": args.pairs, "quiet_window": quiet.record, "outcomes": outcomes,
        "commands": recorder.commands, "files_sha256": files})
    print(json.dumps({"receipt": str(out), "provenance": provenance, "quiet_window": quiet.record,
                      "regimes": [(r["regime"], r["group"], r["gates"]["interpretation"])
                                  for r in analysis["regimes"]]}, indent=1))
    return 0


def command_analyze(args):
    receipt = Path(args.dir).resolve()
    output = Path(args.output).resolve()
    if output.exists():
        raise Refusal(f"{output} exists; choose a fresh path")
    write_json(output, analyze(receipt))
    return 0


def parser():
    top = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = top.add_subparsers(dest="command", required=True)
    p = sub.add_parser("test", help="build and run the correctness tests only")
    p.add_argument("--spec", required=True)
    p.add_argument("--pmu-source", default=str(LAB / "pmu.c"))
    p.add_argument("--sanitizers", action="store_true")
    p.add_argument("--out", required=True)
    p = sub.add_parser("run", help="test, build, inspect and measure every regime")
    p.add_argument("--spec", required=True)
    p.add_argument("--pmu-source", required=True, help="your pmu.c")
    p.add_argument("--test-implementation", action="store_true",
                   help="label the run TEST IMPLEMENTATION (required for an assessor source)")
    p.add_argument("--sanitizers", action="store_true", help="also run the tests under sanitizers")
    p.add_argument("--pairs", type=native.decimal, default=gates.REQUIRED_PAIRS)
    p.add_argument("--regime", action="append", help="measure only this regime (repeatable)")
    p.add_argument("--quiet-marker", help="file that exists while measuring (shared-host protocol)")
    p.add_argument("--max-load", type=float, default=1.5, help="1-minute load required before measuring")
    p.add_argument("--max-wait-minutes", type=native.decimal, default=60)
    p.add_argument("--out", required=True, help="fresh receipt directory")
    p = sub.add_parser("analyze", help="re-derive gates and estimates from a receipt's raw rows")
    p.add_argument("--dir", required=True)
    p.add_argument("--output", required=True)
    return top


def main():
    args = parser().parse_args()
    try:
        return {"test": command_test, "run": command_run, "analyze": command_analyze}[args.command](args)
    except Refusal as refusal:
        print(f"run_dossier.py: {refusal}", file=sys.stderr)
        return refusal.status


if __name__ == "__main__":
    raise SystemExit(main())

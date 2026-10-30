"""Public deterministic protocol checks and separately gated native PMU observations.

No kernel policy, capability, governor or system affinity changes. --hardware-only
uses an already reviewed binary; it performs no compilation or package install.

Stages (each recorded with exact commands, exit codes and log hashes):

protocol   GCC and Clang builds of the protocol, loss, sampling-ring, parser and
           clock tests (debug and optimized; --sanitizers adds ASan/UBSan for
           all of them and a ThreadSanitizer build of the concurrent ring test),
           semantic rejection of the unfinished learner scaffold, mutation
           checks, and the collector's argument-parsing exit-status tests.
hardware   an optimized unsanitized collector, environment preflight, a
           calibrated paired counting sweep judged by tools/measurement_gates.py
           (rankings only for supported regimes), sampled records, a bounded
           ring-saturation experiment (final versus concurrent drain) and a
           standard perf comparison.
"""
import argparse
import csv
import hashlib
import io
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys
from fractions import Fraction
from datetime import datetime, timezone

from c_source import find_all, replace_unique
import measurement_gates as gates

ROOT = Path(__file__).resolve().parents[1]
LAB = ROOT / "labs/performance_followthrough"
SUPPORT = ROOT / "labs/support"
REF = ROOT / "assessor/performance_followthrough"

# Protocol constants and the measurement build flags come from the gates
# module (labs/support/measurement_gates.py), shared with the learner runner.
MAX_REPETITIONS = gates.MAX_REPETITIONS
REQUIRED_PAIRS = gates.REQUIRED_PAIRS
WARNINGS = ["-std=c17", "-Wall", "-Wextra", "-Wpedantic", "-Werror"]
INCLUDES = ["-I" + str(LAB), "-I" + str(SUPPORT)]
COMPONENT_INCLUDES = ["-I" + str(ROOT / "labs/algorithms/include"), "-I" + str(ROOT / "labs/corpus_events"),
                      "-I" + str(ROOT / "labs/hosted_systems/timer_depth")]
COLLECTOR = [LAB / "pmu_native.c", LAB / "pmu_workloads.c", LAB / "pmu_report.c", LAB / "pmu_sampling.c",
             LAB / "pmu_loss.c", LAB / "pmu_control.c", LAB / "pmu_linux.c", LAB / "pmu_integrations.c",
             SUPPORT / "cm_parse.c", SUPPORT / "cm_clock.c", REF / "pmu.c"]
COMPONENTS = [ROOT / "assessor/algorithms/algo.c", ROOT / "assessor/corpus_events/events.c",
              ROOT / "assessor/hosted_systems/timer_depth/collection.c",
              ROOT / "assessor/hosted_systems/timer_depth/timer.c"]
WITH_COMPONENTS = ["-DPF_WITH_SORT", "-DPF_WITH_SERVICE", *COMPONENT_INCLUDES]

# Deterministic test programs: (name, sources, extra flags, pass marker).
UNIT_TESTS = [
    ("protocol", [LAB / "pmu_tests.c", REF / "pmu.c", LAB / "publication.c"], [], "PF_PROTOCOL_PASS"),
    ("loss", [LAB / "pmu_loss_tests.c", LAB / "pmu_loss.c", REF / "pmu.c"], [], "PF_LOSS_PASS"),
    ("sampling", [LAB / "pmu_sampling_tests.c", LAB / "pmu_sampling.c", LAB / "pmu_loss.c", REF / "pmu.c"],
     ["-pthread"], "PF_SAMPLING_PASS"),
    ("parse", [SUPPORT / "cm_parse_tests.c", SUPPORT / "cm_parse.c"], [], "CM_PARSE_PASS"),
    ("clock", [SUPPORT / "cm_clock_tests.c", SUPPORT / "cm_clock.c"], [], "CM_CLOCK_PASS"),
]
ASAN = ["-O1", "-g", "-fsanitize=address,undefined", "-fno-sanitize-recover=all", "-fno-omit-frame-pointer"]
TSAN = ["-O1", "-g", "-fsanitize=thread"]

# Each mutant reintroduces a defect class the named tests must reject.
MUTATIONS = {
    "never-ran-as-measured": (REF / "pmu.c", "protocol",
                              "p->state = p->running ? PF_PMU_MEASURED : PF_PMU_NEVER_RAN;",
                              "p->state = PF_PMU_MEASURED;"),
    "lost-accounting": (REF / "pmu.c", "protocol", "*total += lost;", "*total += 0;"),
    "duplicate-id": (REF / "pmu.c", "protocol", "j == p->count || (seen & (1u << j))", "j == p->count"),
    "loss-double-count": (LAB / "pmu_loss.c", "loss", "account->total_lost = inputs->kernel_lost;",
                          "account->total_lost = inputs->kernel_lost + inputs->inband_lost;"),
    "loss-unknown-as-zero": (LAB / "pmu_loss.c", "loss", "if (!inputs->kernel_count_known) { return; }",
                             "if (!inputs->kernel_count_known) { account->accounting = PF_LOSS_COMPLETE;"
                             " account->lossless = inputs->inband_lost == 0u; return; }"),
    "early-loss-unknown": (LAB / "pmu_sampling.c", "sampling",
                           "pf_loss_reconcile(&nothing_observed, &result->loss);", "(void)nothing_observed;"),
    "classify-ring-parsed-forced": (LAB / "pmu_sampling.c", "sampling",
                                    ".ring_parsed = result->parser_ran && (result->parser_status == PF_RING_OK),",
                                    ".ring_parsed = true,"),
    "classify-lossless-unobserved": (LAB / "pmu_sampling.c", "sampling",
                                     'if (strcmp(result->status, "measured") != 0) '
                                     "{ result->loss.lossless = false; }", "(void)0;"),
    "loss-unparsed-ring-trusted": (LAB / "pmu_loss.c", "loss", "if (!inputs->ring_parsed) { return; }", "(void)0;"),
    "loss-lossless-ignores-throttle": (
        LAB / "pmu_loss.c", "loss",
        "account->lossless = (inputs->kernel_lost == 0u) && (inputs->hardware_lost == 0u) && "
        "(inputs->throttle_records == 0u) && (inputs->unknown_records == 0u);",
        "account->lossless = (inputs->kernel_lost == 0u) && (inputs->hardware_lost == 0u) && "
        "(inputs->unknown_records == 0u);"),
    "loss-lossless-ignores-unknown": (
        LAB / "pmu_loss.c", "loss",
        "account->lossless = (inputs->kernel_lost == 0u) && (inputs->hardware_lost == 0u) && "
        "(inputs->throttle_records == 0u) && (inputs->unknown_records == 0u);",
        "account->lossless = (inputs->kernel_lost == 0u) && (inputs->hardware_lost == 0u) && "
        "(inputs->throttle_records == 0u);"),
    "parse-narrow-then-check": (
        SUPPORT / "cm_parse.c", "parse",
        "unsigned max, unsigned *value) { uintmax_t parsed = 0;"
        " enum cm_parse_status status = cm_parse_umax(text, min, max, &parsed);",
        "unsigned max, unsigned *value) { uintmax_t parsed = 0;"
        " enum cm_parse_status status = cm_parse_umax(text, 0, UINTMAX_MAX, &parsed);"
        " if (status == CM_PARSE_OK && ((unsigned)parsed < min || (unsigned)parsed > max))"
        " { status = CM_PARSE_RANGE; }"),
}
ASSERT_MARKERS = {"protocol": "PF_PROTOCOL_ASSERT:", "loss": "PF_LOSS_ASSERT:",
                  "sampling": "PF_SAMPLING_ASSERT:", "parse": "CM_PARSE_ASSERT:"}

# Ordering mutants of the real ring consumer: each weakens one half of the
# acquire/release contract with the kernel (pmu_sampling.h). Only
# ThreadSanitizer can see them: the sampling test (model producer, the real
# drainer thread of pf_sampling_observe) must exit 66 with a data race report.
TSAN_EXIT = 66
ORDERING_MUTATIONS = {
    "relaxed-data-head": ("uint64_t head = __atomic_load_n(view->head, __ATOMIC_ACQUIRE);",
                          "uint64_t head = __atomic_load_n(view->head, __ATOMIC_RELAXED);"),
    "relaxed-data-tail": ("__atomic_store_n(view->tail, tail, __ATOMIC_RELEASE);",
                          "__atomic_store_n(view->tail, tail, __ATOMIC_RELAXED);"),
}

# Python unit tests of the shared gates and the learner runner.
PYTHON_TESTS = [("measurement-gates", ROOT / "tools/test_measurement_gates.py"),
                ("learner-runner", LAB / "test_native.py")]

SWEEP = {"search": [16, 1024, 65536], "layout": [256, 8192, 262144],
         "numeric": [256, 8192, 1048576], "dispatch": [256, 8192, 262144],
         "allocation": [16, 256, 4096], "sort": [16, 128, 1024],
         "concurrency": [1024, 16384, 65536], "service": [8, 64, 1024]}

# Bounded ring-saturation experiment: one data page holds 102 40-byte sample
# records. The period is 200000 cycles, chosen with about twice the throttle
# floor (maximum frequency / perf_event_max_sample_rate) as margin when it was
# set. The kernel may lower that rate by itself, so each row records the floor
# read at its start, the summary records the margin, and the throttle records
# decide: a throttled interval produces no sample and no loss count, so any
# throttle record makes the outcome `throttled`.
SATURATION = dict(family="numeric", n=1048576, variant=0, repetitions=64, precision=0,
                  pages=1, period=200000, trials=3)


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def read_text(path):
    try:
        return Path(path).read_text()
    except OSError as error:
        return {"unavailable": str(error)}


def load_snapshot():
    """Host load at one moment. The machine is shared: other work perturbs
    timing and sampling, so hardware receipts record it before and after."""
    meminfo = read_text("/proc/meminfo")
    available = next((line.split()[1] for line in meminfo.splitlines() if line.startswith("MemAvailable:")),
                     None) if isinstance(meminfo, str) else None
    return {"utc": datetime.now(timezone.utc).isoformat(), "loadavg": read_text("/proc/loadavg"),
            "mem_available_kb": int(available) if available else None}


def preflight():
    paths = ["/proc/sys/kernel/perf_event_paranoid", "/proc/self/status", "/proc/loadavg", "/proc/meminfo",
             "/proc/self/cgroup", "/proc/sys/kernel/perf_event_mlock_kb",
             "/proc/sys/kernel/perf_event_max_sample_rate",
             "/sys/devices/system/cpu/online",
             "/sys/devices/system/clocksource/clocksource0/current_clocksource",
             "/sys/devices/system/clocksource/clocksource0/available_clocksource"]
    result = {"platform": platform.platform(), "uid": os.getuid(),
              "affinity": sorted(os.sched_getaffinity(0)), "files": {}, "pmus": {}}
    for name in paths:
        result["files"][name] = read_text(name)
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


def source_files():
    return [Path(__file__), ROOT / "tools/check_perf_control.py", ROOT / "tools/measurement_gates.py",
            *sorted(LAB.glob("pmu*")), LAB / "check_native_args.py", LAB / "native.py", LAB / "test_native.py",
            ROOT / "tools/test_measurement_gates.py", LAB / "publication.h",
            LAB / "publication.c", *sorted(SUPPORT.glob("cm_*")), SUPPORT / "measurement_gates.py",
            REF / "pmu.c", ROOT / "labs/algorithms/include/algo.h", ROOT / "labs/corpus_events/events.h",
            ROOT / "labs/hosted_systems/timer_depth/collection.h",
            ROOT / "labs/hosted_systems/timer_depth/timer.h", *COMPONENTS]


def source_hashes():
    return {str(p.relative_to(ROOT)): sha(p) for p in source_files() if p.is_file()}


def derived_estimates(rows):
    """Checked rational estimates, kept apart from the raw rows they come from."""
    derived = []
    for row in rows:
        entry = {"pair": row["pair"], "variant": row["variant"],
                 "running_fraction": str(Fraction(row["running"], row["enabled"]))}
        for name in ("cycles", "instructions"):
            entry["scaled_" + name] = str(Fraction(row["raw_" + name] * row["enabled"], row["running"]))
        entry["cpi"] = str(Fraction(row["raw_cycles"], row["raw_instructions"])) if row["raw_instructions"] else None
        derived.append(entry)
    return derived


def summarize_regime(family, n, rows, calibration):
    evaluation = gates.evaluate_regime(rows, calibration, required_pairs=REQUIRED_PAIRS)
    summary = {"family": family, "n": n, "repetitions": calibration["repetitions"],
               "calibration": {k: v for k, v in calibration.items() if k != "attempts"},
               "endpoint_migrations": sum(r["cpu_before"] != r["cpu_after"] for r in rows),
               **evaluation,
               "interpretation_limit": "Observation only; causal diagnosis requires the hypothesis "
                                       "dossier and disassembly review."}
    if evaluation["gates"]["acquisition"] == "valid":
        summary["derived"] = derived_estimates(rows)
    return summary


def summarize_saturation(rows, rate_before, rate_after):
    modes = {}
    for mode in ("final", "concurrent"):
        selected = [r for r in rows if r["drain_mode"] == mode]
        modes[mode] = {key: [r.get(key) for r in selected] for key in
                       ("status", "samples", "inband_lost", "hardware_lost", "kernel_lost", "total_lost",
                        "unreported_lost", "loss_accounting", "lossless", "throttled", "throttle_records",
                        "unknown", "drains", "running", "throttle_bound")}
    measured = bool(rows) and all(r["status"] == "measured" for r in rows)
    complete = measured and all(r["loss_accounting"] == "complete" for r in rows)
    throttled = [r for r in rows if r.get("throttled") or r.get("throttle_records")]
    if not measured:
        outcome = "not-measured"
    elif throttled:
        # Skipped samples are in no count: neither saturation nor its
        # absence can be claimed for this configuration.
        outcome = (f"throttled in {len(throttled)} of {len(rows)} runs: loss counts exclude the "
                   "throttled intervals; choose a longer period")
    elif not complete:
        outcome = "unknown: kernel loss count unavailable; in-band totals are lower bounds only"
    elif any(r["total_lost"] or r["hardware_lost"] for r in rows):
        outcome = "saturated"
    else:
        outcome = "saturation not reached"
    lost = {mode: [v for v in values["total_lost"] if v is not None] for mode, values in modes.items()}
    return {"configuration": SATURATION, "outcome": outcome, "by_drain_mode": modes,
            "final_drain_lost_any": bool(lost["final"]) and any(lost["final"]),
            "concurrent_drain_lost_any": bool(lost["concurrent"]) and any(lost["concurrent"]),
            "perf_event_max_sample_rate": {"before": rate_before, "after": rate_after},
            "throttle_records": sum(r.get("throttle_records", 0) for r in rows),
            "period_floor_cycles": rows[0].get("period_floor_cycles") if rows else None,
            "period_margin": (str(Fraction(SATURATION["period"], rows[0]["period_floor_cycles"]))
                              if rows and rows[0].get("period_floor_cycles") else None),
            "limit": "Descriptive for this host and binary. Loss depends on period, ring size, run length "
                     "and when the drainer thread is scheduled; the concurrent drainer competes for the "
                     "machine. Throttled intervals produce neither samples nor loss records."}


class Campaign:
    def __init__(self, out):
        self.out = out
        self.commands = []
        self.checks = []

    def run(self, label, command, expected=0, timeout=120, env=None, pass_fds=()):
        command = list(map(str, command))
        result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=timeout,
                                env=env, pass_fds=pass_fds)
        log = self.out / (label + ".txt")
        log.write_text(result.stdout + result.stderr)
        self.commands.append({"label": label, "command": command, "exit_code": result.returncode,
                              "expected_exit_code": expected, "log": str(log), "sha256": sha(log)})
        if expected is not None and result.returncode != expected:
            raise RuntimeError(f"{label}: expected {expected}, got {result.returncode}: {result.stderr[-1000:]}")
        return result.stdout, result.stderr

    def check(self, label, condition):
        self.checks.append({"name": label, "passed": bool(condition)})
        if not condition:
            raise AssertionError(label)

    def build(self, label, compiler, flags, sources, output):
        self.run(label + "-build", [compiler, *WARNINGS, *INCLUDES, *flags, *sources, "-o", output])

    def unit_tests(self, compiler, profile, flags, tests):
        for name, sources, extra, marker in tests:
            exe = self.out / f"{compiler}-{profile}-{name}"
            self.build(exe.name, compiler, [*flags, *extra], sources, exe)
            stdout, _ = self.run(exe.name, [exe])
            self.check(exe.name + "-pass", marker in stdout)

    def protocol(self, sanitizers):
        profiles = [("debug", ["-O0", "-g"]), ("optimized", ["-O2", "-DNDEBUG"])]
        if sanitizers:
            profiles.append(("asan-ubsan", ASAN))
        for cc in ("gcc", "clang"):
            self.run(cc + "-version", [cc, "--version"])
            for name, flags in profiles:
                self.unit_tests(cc, name, flags, UNIT_TESTS)
            if sanitizers:
                threaded = [t for t in UNIT_TESTS if t[0] == "sampling"]
                self.unit_tests(cc, "tsan", TSAN, threaded)
        exe = self.out / "learner"
        self.build("learner", "gcc", ["-O0"], [LAB / "pmu_tests.c", LAB / "pmu.c", LAB / "publication.c"], exe)
        stdout, stderr = self.run("learner-unsolved", [exe], expected=1)
        self.check("learner-semantic-rejection", "PF_PROTOCOL_ASSERT:" in stderr and "PF_PROTOCOL_PASS" not in stdout)
        self.mutations()
        if sanitizers:
            self.ordering_mutations()
        self.argument_tests(sanitizers)
        for label, script in PYTHON_TESTS:
            _, stderr = self.run(label + "-unittest", [sys.executable, script])
            self.check(label + "-unittest", stderr.rstrip().endswith("OK"))

    def mutations(self):
        tests = {name: (sources, extra) for name, sources, extra, _ in UNIT_TESTS}
        for name, (target, test, old, new) in MUTATIONS.items():
            source = target.read_text()
            self.check(name + "-anchor", len(find_all(source, old)) == 1)
            mutated = self.out / (name + "-" + target.name)
            mutated.write_text(replace_unique(source, old, new))
            sources, extra = tests[test]
            exe = self.out / name
            self.build(name, "gcc", ["-O0", *extra], [mutated if s == target else s for s in sources], exe)
            stdout, stderr = self.run(name, [exe], expected=1)
            self.check(name + "-semantic-rejection", ASSERT_MARKERS[test] in stderr and "PASS" not in stdout)

    def ordering_mutations(self):
        sources, extra = next((s, e) for name, s, e, _ in UNIT_TESTS if name == "sampling")
        target = LAB / "pmu_sampling.c"
        source = target.read_text()
        for name, (old, new) in ORDERING_MUTATIONS.items():
            self.check(name + "-anchor", len(find_all(source, old)) == 1)
            mutated = self.out / (name + "-" + target.name)
            mutated.write_text(replace_unique(source, old, new))
            for cc in ("gcc", "clang"):
                exe = self.out / f"{name}-{cc}"
                self.build(exe.name, cc, [*TSAN, *extra], [mutated if s == target else s for s in sources], exe)
                _, stderr = self.run(exe.name, [exe], expected=TSAN_EXIT, timeout=300)
                self.check(exe.name + "-race-detected", "ThreadSanitizer: data race" in stderr)

    def argument_tests(self, sanitizers):
        """Exit statuses of the collector's command line, with and without the
        optional components; never opens a perf event (check mode)."""
        variants = [("args-minimal", ["-O1", "-g"], []), ("args-full", ["-O1", "-g", *WITH_COMPONENTS], COMPONENTS)]
        if sanitizers:
            variants.append(("args-asan-ubsan", [*ASAN, *WITH_COMPONENTS], COMPONENTS))
        for label, flags, components in variants:
            exe = self.out / label
            self.build(label, "gcc", [*flags, "-pthread"], [*COLLECTOR, *components, "-lm"], exe)
            stdout, _ = self.run(label + "-exit-status", [sys.executable, LAB / "check_native_args.py",
                                                          "--binary", exe], timeout=600)
            self.check(label + "-exit-status", "PF_ARGS_PASS:" in stdout)


def count_row(campaign, label, binary, family, n, variant, repetitions):
    raw, _ = campaign.run(label, [binary, "count", family, n, variant, repetitions])
    return json.loads(raw)


def calibrate(campaign, binary, family, n):
    """The shared calibration loop (measurement_gates.calibrate); an
    acquisition failure during calibration fails the campaign check."""
    def measure(repetitions):
        return [count_row(campaign, f"calibrate-{family}-{n}-{repetitions}-{v}", binary, family, n, v, repetitions)
                for v in range(2)]

    state = gates.calibrate(measure)
    campaign.check(f"calibrate-{family}-{n}-acquired", not state.get("aborted"))
    return state


def sweep(campaign, binary, available, pairs, crosscheck_only, observations, calibration, summaries):
    sizes = {} if crosscheck_only else SWEEP
    for family, regimes in sizes.items():
        for n in (regimes if available else regimes[:1]):
            if available:
                state = calibrate(campaign, binary, family, n)
            else:  # exercises the path only; nothing is interpretable
                state = dict(gates.calibration_state([{"repetitions": 8, "min_elapsed_ns": 0, "target_ns": 1}],
                                                     max_repetitions=None), calibrated=False)
            calibration.append({"family": family, "n": n, **state})
            rows = []
            for pair in range(pairs if available else 1):
                checksums = []
                for order in range(2):
                    variant = order ^ (pair & 1)  # alternate A/B order
                    row = count_row(campaign, f"{family}-{n}-{pair}-{variant}", binary, family, n, variant,
                                    state["repetitions"])
                    row.update(pair=pair, order=order)
                    observations.append(row)
                    rows.append(row)
                    checksums.append(row["checksum"])
                campaign.check(f"{family}{n}{pair}-equal-work", checksums[0] == checksums[1])
            summaries.append(summarize_regime(family, n, rows, state))


def saturation(campaign, binary):
    rate = "/proc/sys/kernel/perf_event_max_sample_rate"
    before = read_text(rate)
    rows = []
    s = SATURATION
    for trial in range(s["trials"]):
        order = ("final", "concurrent") if trial % 2 == 0 else ("concurrent", "final")
        for position, mode in enumerate(order):
            raw, _ = campaign.run(f"saturation-{mode}-{trial}", [
                binary, "sample", s["family"], s["n"], s["variant"], s["repetitions"], s["precision"],
                f"--drain={mode}", f"--pages={s['pages']}", f"--period={s['period']}", "--retain=0"])
            row = json.loads(raw.splitlines()[-1])
            row.update(trial=trial, order=position)
            rows.append(row)
    return rows, summarize_saturation(rows, before, read_text(rate))


def standard_comparison(campaign, binary, environment):
    local_raw, _ = campaign.run("standard-local", [binary, "count", "numeric", "65536", "0", "8"])
    local = json.loads(local_raw)
    control_read, control_write = os.pipe()
    ack_read, ack_write = os.pipe()
    child_env = dict(os.environ, PF_PERF_CONTROL_FD=str(control_write), PF_PERF_ACK_FD=str(ack_read))
    try:
        raw, perf = campaign.run("standard-perf", [
            "perf", "stat", "--delay=-1", "--control=fd:" + str(control_read) + "," + str(ack_write),
            "-x,", "-e", "{instructions:u,cycles:u}", "--", binary, "work", "numeric", "65536", "0", "8"],
            env=child_env, pass_fds=(control_read, control_write, ack_read, ack_write))
    finally:
        for descriptor in (control_read, control_write, ack_read, ack_write):
            os.close(descriptor)
    standard = {}
    for row in csv.reader(io.StringIO(perf)):
        if len(row) >= 5 and row[2] in ("instructions:u", "cycles:u"):
            standard[row[2]] = {"raw": int(row[0]), "running_ns": int(row[3]), "running_percent": float(row[4])}
    campaign.check("standard-perf-counted", set(standard) == {"instructions:u", "cycles:u"} and
                   all(r["raw"] > 0 and r["running_ns"] > 0 and r["running_percent"] > 0 for r in standard.values()))
    campaign.check("standard-perf-matched-workload",
                   json.loads(raw)["checksum"] == local["checksum"] and local["status"] == "measured")
    environment["standard_perf_comparison"] = {
        "output": raw, "counters": perf, "parsed_counters": standard, "local_observation": local,
        "same_workload_and_scope": True, "equality_required": False,
        "control": "enable/ack then batch then disable/ack; user-space control return instructions are included",
        "interpretation": "Same user-thread batch and checksum across separate runs. Control overhead and run "
                          "variation preclude exact equality."}


def snapshot_sources(out, hashes):
    """Copy the exact inputs next to the receipt so it stays verifiable."""
    snapshot = out / "sources"
    for name in hashes:
        target = snapshot / name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(ROOT / name, target)
    return snapshot


def display(path):
    """Repository-relative when inside the checkout, absolute otherwise."""
    path = Path(path).resolve()
    return str(path.relative_to(ROOT)) if path.is_relative_to(ROOT) else str(path)


def comparison_counts(summaries):
    counts = {"supported": 0, "inconclusive": 0, "refused": 0}
    for summary in summaries:
        counts[summary["gates"]["interpretation"]] += 1
    return counts


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--report", type=Path, required=True)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--protocol-only", action="store_true")
    mode.add_argument("--hardware-only", action="store_true")
    parser.add_argument("--binary", type=Path)
    parser.add_argument("--sanitizers", action="store_true")
    parser.add_argument("--crosscheck-only", action="store_true",
                        help="Hardware-only focused standard-tool comparison; no workload sweep or sampling")
    parser.add_argument("--pairs", type=int, default=REQUIRED_PAIRS)
    args = parser.parse_args()
    if not 1 <= args.pairs <= REQUIRED_PAIRS:
        parser.error("pairs must be 1..31; fewer than 31 is a smoke observation")
    if args.hardware_only and (not args.binary or not args.binary.is_file()):
        parser.error("hardware-only requires an existing --binary")
    if args.crosscheck_only and not args.hardware_only:
        parser.error("crosscheck-only requires hardware-only")
    report = args.report.resolve()
    if report.exists():
        parser.error("report exists; use a fresh immutable receipt path")
    out = report.with_suffix("")
    out.mkdir(parents=True, exist_ok=False)
    if args.binary and not args.hardware_only:
        args.binary.resolve().parent.mkdir(parents=True, exist_ok=True)
    campaign = Campaign(out)
    observations, calibration, summaries = [], [], []
    hashes = source_hashes()
    gate_states = {"protocol": "not-run", "hardware_counting": "not-run", "hardware_sampling": "not-run",
                   "sampling_saturation": "not-run", "microarchitectural_diagnosis": "not-assessed",
                   "independent_learner": "not-assessed"}
    error = None
    saturation_summary = None
    snapshot = None
    binary = args.binary.resolve() if args.binary else out / "native"
    environment = {}
    try:
        if not args.hardware_only:
            campaign.protocol(args.sanitizers)
            gate_states["protocol"] = "passed-with-sanitizers" if args.sanitizers else "passed"
            if not args.protocol_only:
                campaign.build("native", "gcc", [*gates.MEASUREMENT_CFLAGS, *WITH_COMPONENTS, "-pthread"],
                               [*COLLECTOR, *COMPONENTS, "-lm"], binary)
                binary.with_suffix(".build.json").write_text(json.dumps({
                    "binary_sha256": sha(binary), "source_sha256": hashes,
                    "build_command": campaign.commands[-1]["command"],
                    "compiler": shutil.which("gcc")}, indent=2) + "\n")
                stdout, _ = campaign.run("perf-control-protocol", [sys.executable, ROOT / "tools/check_perf_control.py",
                                                                    "--binary", binary])
                campaign.check("perf-control-acknowledgements", "PERF_CONTROL_PASS:" in stdout)
        if not args.protocol_only:
            build_record = json.loads(binary.with_suffix(".build.json").read_text())
            campaign.check("binary-matches-build", build_record["binary_sha256"] == sha(binary))
            campaign.check("binary-source-matches", build_record["source_sha256"] == hashes)
            symbols, _ = campaign.run("symbols", ["nm", "-n", binary])
            campaign.check("performance-binary-uninstrumented",
                           not any(s in symbols for s in ("__asan_init", "__tsan_init", "__ubsan_handle")))
            snapshot = snapshot_sources(out, hashes)
            load_before = load_snapshot()
            environment = preflight()
            environment["load"] = {"before": load_before}
            environment["build"] = build_record
            header = Path("/usr/include/linux/perf_event.h")
            environment["perf_event_header_sha256"] = sha(header) if header.is_file() else None
            for label, command in [("cpu", ["lscpu"]), ("virtualization", ["systemd-detect-virt"]),
                                   ("perf-version", ["perf", "--version"]), ("events", ["perf", "list", "--no-desc"]),
                                   ("build-id", ["readelf", "-n", binary]), ("disassembly", ["objdump", "-d", binary]),
                                   ("families", [binary, "families"])]:
                campaign.run(label, command, expected=None)
            campaign.run("mappings", ["cat", "/proc/self/maps"])
            capability = count_row(campaign, "capability", binary, "numeric", "1024", "0", "8")
            observations.append(capability)
            available = capability["status"] == "measured" and capability["running"] > 0
            gate_states["hardware_counting"] = "passed" if available else "blocked:" + capability["status"]
            sweep(campaign, binary, available, args.pairs, args.crosscheck_only, observations, calibration, summaries)
            if not args.crosscheck_only:
                raw, _ = campaign.run("sampled-records", [binary, "sample", "numeric", "65536", "0", "256"])
                sampling = json.loads(raw.splitlines()[-1])
                observations.append(sampling)
                gate_states["hardware_sampling"] = ("passed" if sampling["status"] == "measured"
                                                    else "blocked:" + sampling["status"])
                if sampling["status"] == "measured":
                    rows, saturation_summary = saturation(campaign, binary)
                    observations.extend(rows)
                    gate_states["sampling_saturation"] = saturation_summary["outcome"]
            if available:
                standard_comparison(campaign, binary, environment)
                gate_states["standard_tool_comparison"] = "observed"
            if args.pairs < REQUIRED_PAIRS and available:
                gate_states["hardware_counting"] = "smoke-observed"
            if args.crosscheck_only and available:
                gate_states["hardware_counting"] = "crosscheck-observed"
            gate_states["comparisons"] = comparison_counts(summaries)
            environment["load"]["after"] = load_snapshot()
        campaign.check("source-stable", hashes == source_hashes())
    except (OSError, ValueError, AssertionError, RuntimeError, subprocess.TimeoutExpired) as exc:
        error = str(exc)
    payload = {"schema_version": 2, "created_utc": datetime.now(timezone.utc).isoformat(),
               "passed": error is None, "error": error, "gates": gate_states, "checks": campaign.checks,
               "source_sha256": hashes,
               "source_snapshot": display(snapshot) if snapshot else None,
               "commands": campaign.commands, "environment": environment,
               "observations": observations, "binary": str(binary) if binary.is_file() else None,
               "summaries": summaries, "calibration": calibration, "saturation": saturation_summary,
               "binary_sha256": sha(binary) if binary.is_file() else None,
               "host_policy_changed": False,
               "measurement_gates": str(Path(gates.SOURCE).resolve().relative_to(ROOT)),
               "scope": "Author protocol checks and separately labelled hardware observations; "
                        "no educational effectiveness claim",
               "sources": ["https://docs.kernel.org/userspace-api/perf_ring_buffer.html",
                           "https://docs.kernel.org/admin-guide/perf-security.html",
                           "https://man7.org/linux/man-pages/man2/perf_event_open.2.html"]}
    report.write_text(json.dumps(payload, indent=2) + "\n")
    print(json.dumps({"passed": payload["passed"], "error": error, "report": str(report), "gates": gate_states,
                      "binary": payload["binary"]}))
    return 0 if error is None else 1


if __name__ == "__main__":
    raise SystemExit(main())

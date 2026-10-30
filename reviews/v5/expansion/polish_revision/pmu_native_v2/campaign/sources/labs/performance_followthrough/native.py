#!/usr/bin/env python3
"""Learner workflow for the native PMU laboratory (PMU_PROTOCOL.md).

Runs from a learner workspace or a source checkout: every path is relative to
this file, and nothing here reads `tools/` or `assessor/`. Your own component
is `pmu.c`; files marked SUPPLIED are course adapters, not your work.

    python3 native.py doctor     # what this machine allows, and why
    python3 native.py protocol   # deterministic protocol, loss and ring tests
    python3 native.py build      # optimized, uninstrumented collector
    python3 native.py run        # bounded paired counting comparison (gated)
    python3 native.py sample     # sampling / ring-saturation observation
    python3 native.py report     # summarize run and sample reports

Exit status: 0 completed; 1 a contract or workload check rejected your
component; 2 invalid request, build error or refused build; 3 the requested
workload family is not built in. A completed `run` can still be inconclusive
or refused: read its gates, not just the exit status.

Provenance labels (LEARNER, TEST IMPLEMENTATION) are self-asserted by the
person running the build; this tool only refuses the obvious mislabel of a
source inside an `assessor` directory. They are not proof of authorship.

No command changes host policy (perf_event_paranoid, capabilities, CPU
governors). When access is missing, `doctor` names the exact reason.
"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import errno
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import shutil
import subprocess
import sys
import tempfile

HERE = Path(__file__).resolve().parent
LABS = HERE.parent
SUPPORT = LABS / "support"

WARNINGS = ["-std=c17", "-Wall", "-Wextra", "-Wpedantic", "-Werror"]
EXIT_SEMANTIC = 1  # a contract assertion or component check failed
EXIT_INVALID = 2
EXIT_UNAVAILABLE = 3
ASAN = ["-O1", "-g", "-fsanitize=address,undefined", "-fno-sanitize-recover=all", "-fno-omit-frame-pointer"]
TSAN = ["-O1", "-g", "-fsanitize=thread"]
SANITIZER_RUNTIME = (b"__asan_init", b"__tsan_init", b"__msan_init", b"__ubsan_handle", b"__lsan_init")
PARANOID_USER_LIMIT = 2  # at or below this, user-space events of your own process are allowed

# Supplied course adapters linked into the collector. Your component is pmu.c.
SUPPLIED_COLLECTOR = [HERE / name for name in (
    "pmu_native.c", "pmu_workloads.c", "pmu_report.c", "pmu_sampling.c", "pmu_loss.c",
    "pmu_control.c", "pmu_linux.c", "pmu_integrations.c")] + [SUPPORT / "cm_parse.c", SUPPORT / "cm_clock.c"]

# Optional components you may supply to enable two workload families.
COMPONENTS = {
    "algorithms": {"option": "--algorithms-source", "file": "algo.c", "family": "sort",
                   "include": [LABS / "algorithms/include"]},
    "events": {"option": "--events-source", "file": "events.c", "family": "service",
               "include": [LABS / "corpus_events"]},
    "collection": {"option": "--collection-source", "file": "collection.c", "family": "service",
                   "include": [LABS / "hosted_systems/timer_depth"]},
    "timer": {"option": "--timer-source", "file": "timer.c", "family": "service",
              "include": [LABS / "hosted_systems/timer_depth"]},
}
SERVICE_PARTS = ("events", "collection", "timer")
PROVENANCE_LEARNER = "LEARNER"
PROVENANCE_TEST = "TEST IMPLEMENTATION"
TEST_NOTICE = ("TEST IMPLEMENTATION: the component sources are a supplied test or reference implementation "
               "used to exercise this workflow. Nothing produced from them is evidence of your own work.")
RING_STATUS = {1: "PF_RING_PARTIAL", 2: "PF_RING_INVALID", 3: "PF_RING_OVERFLOW", 4: "PF_RING_UNIMPLEMENTED"}


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def read(path):
    try:
        return Path(path).read_text().strip()
    except OSError as error:
        return f"unavailable: {error.strerror}"


def emit(value):
    print(json.dumps(value, indent=2))


def fail(message, status=EXIT_INVALID):
    print("native.py: " + message, file=sys.stderr)
    return status


def decimal(text):
    """argparse type: plain ASCII decimal digits only (no sign, space, `_`)."""
    if not re.fullmatch(r"[0-9]+", text):
        raise argparse.ArgumentTypeError(f"{text!r} is not a plain decimal number")
    return int(text)


_GATES = None


def load_gates():
    """The same gate rules and constants the author campaigns use, shipped
    in labs/support (never tools/)."""
    global _GATES
    if _GATES is None:
        if not (SUPPORT / "measurement_gates.py").is_file():
            raise SystemExit(fail("labs/support/measurement_gates.py is missing from this workspace"))
        sys.path.insert(0, str(SUPPORT))
        import measurement_gates  # noqa: E402  (labs/support)
        _GATES = measurement_gates
    return _GATES


def fresh_report(path):
    report = Path(path).resolve()
    if report.exists():
        raise SystemExit(fail(f"{report} exists; choose a fresh report path (reports are not overwritten)"))
    report.parent.mkdir(parents=True, exist_ok=True)
    return report


# ---------------------------------------------------------------------------
# provenance


def assessor_sources(paths):
    """Sources whose resolved path (symlinks followed) has an `assessor`
    directory component."""
    return [p for p in paths if "assessor" in Path(p).resolve().parts]


def require_label(paths, *, test_implementation):
    """Provenance for a build or protocol run. A source from an assessor
    directory is never your own work: it must be labelled TEST
    IMPLEMENTATION. The label is otherwise self-asserted."""
    found = assessor_sources(paths)
    if found and not test_implementation:
        raise SystemExit(fail("refused: " + ", ".join(map(str, found)) + " is in an assessor directory; "
                              "pass --test-implementation to label the run TEST IMPLEMENTATION"))
    return PROVENANCE_TEST if test_implementation else PROVENANCE_LEARNER


# ---------------------------------------------------------------------------
# doctor


def process_security(status_text):
    fields = {}
    for key, name in (("Seccomp", "seccomp"), ("NoNewPrivs", "no_new_privs")):
        match = re.search(rf"^{key}:\s*(\S+)", status_text, re.M)
        fields[name] = match.group(1) if match else "unknown"
    return fields


def detect(argv):
    if not shutil.which(argv[0]):
        return "unknown"
    result = subprocess.run(argv, capture_output=True, text=True)
    return result.stdout.strip() or "none"


def host_facts():
    cpu = re.search(r"^model name\s*:\s*(.+)$", read("/proc/cpuinfo"), re.M)
    lsm = read("/proc/self/attr/current")
    return {"machine": platform.machine(), "kernel": platform.release(),
            "cpu_model": cpu.group(1) if cpu else "unknown",
            "perf_event_paranoid": read("/proc/sys/kernel/perf_event_paranoid"),
            "perf_event_max_sample_rate": read("/proc/sys/kernel/perf_event_max_sample_rate"),
            "perf_event_mlock_kb": read("/proc/sys/kernel/perf_event_mlock_kb"),
            "clocksource": read("/sys/devices/system/clocksource/clocksource0/current_clocksource"),
            **process_security(read("/proc/self/status")),
            "lsm": lsm.replace("\x00", "") if lsm else "unknown",
            "lsm_stack": read("/sys/kernel/security/lsm"),
            "container": detect(["systemd-detect-virt", "--container"]),
            "virtualization": detect(["systemd-detect-virt"]),
            "affinity": sorted(os.sched_getaffinity(0))}


def capability_effective():
    """Decode CapEff from /proc/self/status: CAP_PERFMON is bit 38, CAP_SYS_ADMIN 21."""
    match = re.search(r"^CapEff:\s*([0-9a-f]+)", read("/proc/self/status"), re.M)
    if not match:
        return {"available": False}
    value = int(match.group(1), 16)
    return {"available": True, "cap_perfmon": bool(value >> 38 & 1),
            "cap_sys_admin": bool(value >> 21 & 1), "raw": match.group(1)}


def paranoid_meaning(value):
    meanings = {
        "-1": "no restriction for unprivileged users",
        "0": "unprivileged users may profile user and kernel code (no raw tracepoints)",
        "1": "unprivileged users may profile user and kernel code of their own processes",
        "2": "unprivileged users may profile user-space code of their own processes only",
        "3": "Ubuntu/Debian patch: unprivileged perf_event_open denied",
        "4": "Ubuntu/Debian patch: unprivileged perf_event_open denied",
    }
    return meanings.get(value, "unknown value")


def explain_denial(error, facts):
    paranoid_text = facts.get("perf_event_paranoid", "unknown")
    try:
        paranoid = int(paranoid_text)
    except ValueError:
        paranoid = None
    security = f"Seccomp: {facts.get('seccomp', 'unknown')}, NoNewPrivs: {facts.get('no_new_privs', 'unknown')}"
    if paranoid is not None and paranoid > PARANOID_USER_LIMIT:
        return (f"perf_event_open denied (errno {error}) by perf_event_paranoid={paranoid_text} "
                f"({paranoid_meaning(paranoid_text)}). An administrator can set it to "
                f"{PARANOID_USER_LIMIT} or less, or run the collector with CAP_PERFMON. "
                "This tool changes neither.")
    known = "" if paranoid is not None else "perf_event_paranoid is unreadable; "
    prefix = (f"perf_event_open denied (errno {error}) although {known}perf_event_paranoid={paranoid_text} "
              "allows user-space events of your own process, so another policy refuses it: ")
    if facts.get("seccomp") in ("1", "2"):
        return (prefix + f"a seccomp filter is active ({security}). Sandboxes and container runtimes "
                "commonly block perf_event_open; run outside it or have its profile allow the syscall.")
    return (prefix + f"no seccomp filter ({security}). Check the Linux security module (security context "
            f"{facts.get('lsm', 'unknown')}; stack {facts.get('lsm_stack', 'unknown')}) and the "
            f"container {facts.get('container', 'unknown')}, whose policy can deny perf_event_open.")


def explain_errno(error, facts):
    """The exact reason for a failed perf_event_open."""
    if error in (errno.EACCES, errno.EPERM):
        return explain_denial(error, facts)
    if error in (errno.ENOENT, errno.ENODEV, errno.EOPNOTSUPP):
        return (f"hardware event unavailable (errno {error}): no PMU exposed (virtualization "
                f"{facts.get('virtualization', 'unknown')}) or the event or precision is unsupported "
                "on this processor.")
    if error == errno.ENOSYS:
        return ("perf_event_open returned ENOSYS: the kernel was built without CONFIG_PERF_EVENTS, or a "
                f"seccomp filter answers ENOSYS (Seccomp: {facts.get('seccomp', 'unknown')}).")
    return (f"perf_event_open failed with errno {error} ({os.strerror(error) if error else 'none'}), "
            "which is neither a permission nor an availability refusal; see perf_event_open(2).")


def explain(probe, facts):
    """Every missing capability; empty only when the strict counting group
    opened and sampling works."""
    group = probe.get("counting_group")
    if group is None:
        detail = probe.get("reason") or probe.get("output") or f"exit {probe.get('exit_code')}"
        return [f"the PMU probe did not run ({probe.get('status')}): {detail}. Install a C compiler "
                "(gcc or clang) so pmu_probe.c can ask perf_event_open directly."]
    reasons = []
    if group.get("status") != "opened":
        reasons.append(explain_errno(group.get("errno", 0), facts))
    sampling = probe.get("sampling", {})
    if sampling.get("status") not in (None, "opened") and group.get("status") == "opened":
        reasons.append(f"sampling unavailable (errno {sampling.get('errno')}); counting still works: "
                       + explain_errno(sampling.get("errno", 0), facts))
    return reasons


def access_state(probe, reasons):
    group = probe.get("counting_group")
    if group is None:
        return "unknown"
    return "available" if group.get("status") == "opened" and not reasons else "blocked"


def probe_events():
    """Try a strict hardware group (instructions + cycles) and a sampling event
    with a tiny C probe, so the answer comes from perf_event_open itself."""
    source = HERE / "pmu_probe.c"
    compiler = shutil.which("cc") or shutil.which("gcc") or shutil.which("clang")
    if not source.is_file() or not compiler:
        return {"status": "not-run", "reason": "pmu_probe.c or a C compiler is missing"}
    with tempfile.TemporaryDirectory(prefix="pf-probe-") as work:
        binary = Path(work) / "probe"
        build = subprocess.run([compiler, *WARNINGS[:1], "-O1", str(source), "-o", str(binary)],
                               capture_output=True, text=True)
        if build.returncode:
            return {"status": "build-error", "output": build.stderr[-2000:]}
        run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
        try:
            return json.loads(run.stdout)
        except ValueError:
            return {"status": "probe-error", "exit_code": run.returncode, "output": run.stderr[-2000:]}


def doctor(args):
    pmus = {}
    for device in sorted(Path("/sys/bus/event_source/devices").glob("*")):
        pmus[device.name] = {"type": read(device / "type"),
                             "events": sorted(p.name for p in (device / "events").glob("*"))[:40]}
    facts = host_facts()
    probe = probe_events()
    missing = explain(probe, facts)
    emit({"schema_version": 2, **facts, "capabilities": capability_effective(), "pmus": pmus,
          "probe": probe, "missing": missing, "hardware_access": access_state(probe, missing),
          "limits": "A successful open shows access, not that an event means what you think: check the "
                    "processor's event documentation before interpreting counts. The clock source sets "
                    "the smallest interval `run` can time."})
    return 0


# ---------------------------------------------------------------------------
# protocol


def compile_and_run(compiler, flags, sources, label, work):
    binary = Path(work) / label
    command = [compiler, *WARNINGS, *flags, f"-I{HERE}", f"-I{SUPPORT}", *map(str, sources),
               "-o", str(binary)]
    build = subprocess.run(command, capture_output=True, text=True)
    if build.returncode:
        return {"label": label, "outcome": "build-error", "command": command,
                "output": build.stderr[-4000:]}
    try:
        run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=120)
    except subprocess.TimeoutExpired:
        return {"label": label, "outcome": "timeout", "command": command}
    if run.returncode == 0:
        outcome = "passed"
    elif run.returncode == EXIT_SEMANTIC and "_ASSERT:" in run.stderr:
        outcome = "semantic-rejection"  # a contract assertion failed: fix pmu.c
    elif run.returncode < 0:
        outcome = f"crash-signal-{-run.returncode}"
    else:
        outcome = f"unexpected-exit-{run.returncode}"
    return {"label": label, "outcome": outcome, "command": command,
            "stdout": run.stdout[-4000:], "stderr": run.stderr[-4000:]}


def protocol(args):
    pmu_source = Path(args.pmu_source).resolve()
    provenance = require_label([pmu_source], test_implementation=args.test_implementation)
    compilers = [c for c in ("gcc", "clang") if shutil.which(c)]
    profiles = [("debug", ["-O0", "-g"])]
    if args.sanitizers:
        profiles.append(("asan-ubsan", ASAN))
    tests = [("protocol", [HERE / "pmu_tests.c", pmu_source, HERE / "publication.c"], []),
             ("loss", [HERE / "pmu_loss_tests.c", HERE / "pmu_loss.c", pmu_source], []),
             ("sampling-ring", [HERE / "pmu_sampling_tests.c", HERE / "pmu_sampling.c", HERE / "pmu_loss.c",
                                pmu_source], ["-pthread"])]
    results = []
    with tempfile.TemporaryDirectory(prefix="pf-protocol-") as work:
        for compiler in compilers:
            for name, flags in profiles:
                for test, sources, extra in tests:
                    results.append(compile_and_run(compiler, [*flags, *extra], sources,
                                                   f"{compiler}-{name}-{test}", work))
            if args.sanitizers:  # the ring consumer and drainer are the concurrent part
                test, sources, extra = tests[2]
                results.append(compile_and_run(compiler, [*TSAN, *extra], sources,
                                               f"{compiler}-tsan-{test}", work))
    summary = {"provenance": provenance, "notice": TEST_NOTICE if provenance == PROVENANCE_TEST else None,
               "pmu_source": str(pmu_source), "pmu_source_sha256": sha256(pmu_source),
               "results": results,
               "passed": bool(results) and all(r["outcome"] == "passed" for r in results),
               "scope": "Deterministic protocol, loss-accounting and ring-consumer/drainer tests only; "
                        "no hardware event is opened."}
    emit(summary)
    if summary["passed"]:
        return 0
    return EXIT_INVALID if any(r["outcome"] == "build-error" for r in results) else EXIT_SEMANTIC


# ---------------------------------------------------------------------------
# build


def instrumented(binary):
    """Sanitizer runtimes linked in: such a binary measures the sanitizer,
    not your code, so it is refused for every measurement."""
    data = Path(binary).read_bytes()
    return [marker.decode() for marker in SANITIZER_RUNTIME if marker in data]


def selected_components(args):
    chosen = {}
    for role, spec in COMPONENTS.items():
        value = getattr(args, role + "_source")
        if value:
            path = Path(value).resolve()
            if not path.is_file():
                raise SystemExit(fail(f"{spec['option']} {path} does not exist"))
            chosen[role] = path
    service = [part for part in SERVICE_PARTS if part in chosen]
    if service and len(service) != len(SERVICE_PARTS):
        missing = [COMPONENTS[p]["option"] for p in SERVICE_PARTS if p not in chosen]
        raise SystemExit(fail("the service family needs events, collection and timer together; missing "
                              + ", ".join(missing)))
    return chosen


def include_flags(chosen):
    defines, includes = [], [f"-I{HERE}", f"-I{SUPPORT}"]
    for role in chosen:
        for directory in COMPONENTS[role]["include"]:
            if not directory.is_dir():
                raise SystemExit(fail(f"{role} headers missing from this workspace: {directory}"))
            if f"-I{directory}" not in includes:
                includes.append(f"-I{directory}")
    if "algorithms" in chosen:
        defines.append("-DPF_WITH_SORT")
    if "events" in chosen:
        defines.append("-DPF_WITH_SERVICE")
    return defines, includes


def build(args):
    output = Path(args.output).resolve()
    if output.exists() and any(output.iterdir()):
        return fail(f"{output} is not empty; builds are not overwritten")
    pmu_source = Path(args.pmu_source).resolve()
    if not pmu_source.is_file():
        return fail(f"--pmu-source {pmu_source} does not exist")
    compiler = shutil.which(args.cc)
    if not compiler:
        return fail(f"compiler {args.cc!r} not found")
    chosen = selected_components(args)
    provenance = require_label([pmu_source, *chosen.values()], test_implementation=args.test_implementation)
    missing = [str(p) for p in SUPPLIED_COLLECTOR if not p.is_file()]
    if missing:
        return fail("supplied adapters missing from this workspace: " + ", ".join(missing))
    defines, includes = include_flags(chosen)
    flags = [*WARNINGS, *load_gates().MEASUREMENT_CFLAGS, "-pthread", *defines]
    output.mkdir(parents=True, exist_ok=True)
    binary = output / "pmu_native"
    command = [compiler, *flags, *includes, *map(str, [*SUPPLIED_COLLECTOR, pmu_source, *chosen.values()]),
               "-lm", "-o", str(binary)]
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode:
        print(result.stderr[-6000:], file=sys.stderr)
        return fail("build failed (compiler output above)")
    found = instrumented(binary)
    if found:
        binary.unlink()
        return fail("refused: the collector links a sanitizer runtime (" + ", ".join(found) +
                    "); measurement builds must be uninstrumented")
    families = subprocess.run([str(binary), "families"], capture_output=True, text=True, timeout=30)
    version = subprocess.run([compiler, "--version"], capture_output=True, text=True).stdout.splitlines()
    manifest = {
        "schema_version": 2, "kind": "native-build", "provenance": provenance,
        "provenance_basis": "self-asserted by the person who ran the build; assessor-directory sources refused "
                            "unless labelled TEST IMPLEMENTATION",
        "notice": TEST_NOTICE if provenance == PROVENANCE_TEST else None,
        "created_utc": datetime.now(timezone.utc).isoformat(),
        "binary": str(binary), "binary_sha256": sha256(binary),
        "compiler": compiler, "compiler_version": version[0] if version else "unknown",
        "flags": flags, "command": command, "instrumented": False,
        "components": [{"role": "pmu", "path": str(pmu_source), "sha256": sha256(pmu_source),
                        "label": provenance}] +
                      [{"role": role, "path": str(path), "sha256": sha256(path), "label": provenance}
                       for role, path in chosen.items()],
        "supplied_adapters": [{"file": str(p.relative_to(LABS)), "sha256": sha256(p), "label": "SUPPLIED"}
                              for p in SUPPLIED_COLLECTOR],
        "families": [json.loads(line) for line in families.stdout.splitlines() if line.startswith("{")],
    }
    (output / "build.json").write_text(json.dumps(manifest, indent=2) + "\n")
    emit(manifest)
    return 0


def load_build(directory):
    directory = Path(directory).resolve()
    try:
        manifest = json.loads((directory / "build.json").read_text())
    except (OSError, ValueError):
        raise SystemExit(fail(f"{directory}/build.json missing or unreadable; run `native.py build` first"))
    binary = Path(manifest["binary"])
    if not binary.is_file() or sha256(binary) != manifest["binary_sha256"]:
        raise SystemExit(fail("the collector no longer matches build.json; rebuild it"))
    found = instrumented(binary)
    if found or manifest.get("instrumented"):
        raise SystemExit(fail("refused: instrumented collector (" + ", ".join(found) + ") cannot measure"))
    return manifest, binary


def require_family(manifest, family):
    row = next((r for r in manifest["families"] if r["family"] == family), None)
    if row is None:
        raise SystemExit(fail(f"unknown family {family!r}"))
    if not row["available"]:
        option = {"sort": "--algorithms-source", "service": "--events-source, --collection-source "
                  "and --timer-source"}[family]
        raise SystemExit(fail(f"family {family} is not built in: it needs the {row['requires']}; "
                              f"rebuild with {option}", EXIT_UNAVAILABLE))


def collector(binary, argv, timeout=120):
    result = subprocess.run([str(binary), *map(str, argv)], capture_output=True, text=True, timeout=timeout)
    return result.returncode, result.stdout, result.stderr


def build_notice(manifest):
    return manifest.get("notice") or (TEST_NOTICE if manifest.get("provenance") == PROVENANCE_TEST else None)


# ---------------------------------------------------------------------------
# run


def count_once(binary, family, n, variant, repetitions, log):
    argv = ["count", family, n, variant, repetitions]
    code, stdout, stderr = collector(binary, argv)
    log.append({"argv": argv, "exit_code": code, "stdout": stdout, "stderr": stderr[-2000:]})
    if code == EXIT_SEMANTIC:
        raise SystemExit(fail("a workload or checksum check failed: " + stderr.strip()[-400:], EXIT_SEMANTIC))
    if code != 0:
        raise SystemExit(fail(f"collector exit {code}: {stderr.strip()[-400:]}", code))
    return json.loads(stdout)


def explain_open(status, error, facts):
    if status == "unimplemented":
        return "your pmu.c reports PF_PMU_UNIMPLEMENTED: finish and pass `native.py protocol` first."
    if error:
        return explain_errno(error, facts)
    return f"acquisition state {status!r} with no errno: see PMU_PROTOCOL.md, strict grouped acquisition."


def run(args):
    gates = load_gates()
    manifest, binary = load_build(args.build)
    require_family(manifest, args.family)
    if not 1 <= args.pairs <= gates.REQUIRED_PAIRS:
        return fail(f"--pairs must be 1..{gates.REQUIRED_PAIRS} (fewer is a smoke observation)")
    report_path = fresh_report(args.report)
    log = []

    def measure(repetitions):
        return [count_once(binary, args.family, args.n, v, repetitions, log) for v in (0, 1)]

    calibration = gates.calibrate(measure)
    failed = None
    if calibration.get("aborted"):
        trials = calibration["attempts"][-1]["trials"]
        failed = next(t for t in trials if not gates.acquired(t))
        rows = [dict(t, pair=0, order=v) for v, t in enumerate(trials)]
    else:
        rows = []
        for pair in range(args.pairs):
            for order in range(2):
                variant = order ^ (pair & 1)  # alternate which variant runs first
                row = count_once(binary, args.family, args.n, variant, calibration["repetitions"], log)
                rows.append(dict(row, pair=pair, order=order))
    evaluation = gates.evaluate_regime(rows, calibration, required_pairs=gates.REQUIRED_PAIRS)
    facts = host_facts()
    result = {
        "schema_version": 2, "kind": "native-run", "provenance": manifest["provenance"],
        "notice": build_notice(manifest), "created_utc": datetime.now(timezone.utc).isoformat(),
        "build": {"binary_sha256": manifest["binary_sha256"], "components": manifest["components"],
                  "flags": manifest["flags"]},
        "request": {"family": args.family, "n": args.n, "pairs": args.pairs},
        "host": facts, "calibration": calibration, "rows": rows, "evaluation": evaluation,
        "explanation": explain_open(failed["status"], failed["errno"], facts) if failed else None,
        "commands": log,
        "limits": "Calling-thread user-space counts on this host and build. A supported ranking is "
                  "descriptive, not a population, universal-ordering or WCET claim; causes need the "
                  "hypothesis dossier.",
    }
    report_path.write_text(json.dumps(result, indent=2) + "\n")
    emit({"report": str(report_path), "provenance": manifest["provenance"], "gates": evaluation["gates"],
          "reasons": evaluation["reasons"], "ranking": evaluation.get("ranking"),
          "explanation": result["explanation"]})
    return 0


# ---------------------------------------------------------------------------
# sample


def loss_reading(summary):
    if summary.get("status") != "measured":
        return f"no sampling observation: status {summary.get('status')}; nothing is claimed about loss"
    notes = []
    if summary.get("throttled"):
        notes.append(f"throttled: {summary['throttle_records']} throttle records, so the kernel skipped "
                     "samples that no loss count reports")
    if summary.get("throttle_bound"):
        notes.append(f"the period is below this host's throttle floor ({summary['period_floor_cycles']} "
                     "cycles at the maximum frequency)")
    accounting = summary.get("loss_accounting")
    if accounting == "complete":
        if summary["lossless"]:
            text = "lossless: the kernel's lost count is 0, with no hardware drop, throttling or unknown record"
        else:
            text = (f"{summary['total_lost']} ring writes failed by the kernel's count; "
                    f"{summary['unreported_lost']} of them without an in-band PERF_RECORD_LOST; "
                    f"{summary['hardware_lost']} hardware drops")
    elif accounting == "unknown":
        text = (f"loss unknown (lost_support={summary['lost_support']}): in-band records give only a "
                f"lower bound of {summary['lost_lower_bound']}")
    else:
        text = f"loss accounting {accounting}: in-band exceeds the kernel count; do not pick a number"
    return "; ".join([text, *notes])


def explain_sample(summary, facts):
    stage = summary.get("stage", "complete")
    if stage in ("complete",) and summary["status"] in ("measured", "no-samples", "never-scheduled"):
        return None
    if stage == "parser":
        status = summary.get("parser_status")
        if status == 4:
            return "your pf_ring_drain reports PF_RING_UNIMPLEMENTED: finish and pass `native.py protocol` first."
        return (f"your pf_ring_drain stopped on the kernel's records with {RING_STATUS.get(status, status)}; "
                "rerun `native.py protocol` and compare the record layout in PMU_PROTOCOL.md.")
    if stage == "open":
        return explain_errno(summary.get("errno", 0), facts)
    if stage == "map":
        return (f"the ring could not be mapped (errno {summary.get('errno')}): reduce --pages or check "
                f"perf_event_mlock_kb ({facts['perf_event_mlock_kb']} KiB).")
    return (f"the host or a supplied adapter failed at stage {stage!r} (not your pf_ring_drain); "
            f"control errno {summary.get('control_errno')}.")


def sample(args):
    manifest, binary = load_build(args.build)
    require_family(manifest, args.family)
    report_path = fresh_report(args.report)
    argv = ["sample", args.family, args.n, args.variant, args.repetitions, args.precision,
            f"--drain={args.drain}", f"--pages={args.pages}", f"--period={args.period}", f"--retain={args.retain}"]
    code, stdout, stderr = collector(binary, argv, timeout=300)
    lines = [json.loads(line) for line in stdout.splitlines() if line.startswith("{")]
    summary = lines[-1] if lines and lines[-1].get("kind") == "sampling" else None
    if summary is None:
        return fail(f"collector exit {code} without a sampling line: {stderr.strip()[-400:]}",
                    code if code in (EXIT_SEMANTIC, EXIT_UNAVAILABLE) else EXIT_INVALID)
    facts = host_facts()
    result = {
        "schema_version": 2, "kind": "native-sample", "provenance": manifest["provenance"],
        "notice": build_notice(manifest), "created_utc": datetime.now(timezone.utc).isoformat(),
        "build": {"binary_sha256": manifest["binary_sha256"], "components": manifest["components"],
                  "flags": manifest["flags"]},
        "argv": argv, "exit_code": code, "host": facts, "summary": summary,
        "loss": loss_reading(summary), "explanation": explain_sample(summary, facts),
        "mappings": [r["line"] for r in lines if r.get("kind") == "mapping"],
        "samples": [r for r in lines if r.get("kind") == "sample"],
        "stderr": stderr[-2000:],
        "limits": "precise_ip=0 permits skid; a sampled address is not proof of the causal instruction. "
                  "A final-only drain biases kept samples toward the start of the run.",
    }
    report_path.write_text(json.dumps(result, indent=2) + "\n")
    emit({"report": str(report_path), "provenance": manifest["provenance"], "status": summary["status"],
          "stage": summary.get("stage"), "drain_mode": summary["drain_mode"], "samples": summary["samples"],
          "loss": result["loss"], "explanation": result["explanation"]})
    return code if code in (0, EXIT_SEMANTIC, EXIT_INVALID, EXIT_UNAVAILABLE) else EXIT_INVALID


# ---------------------------------------------------------------------------
# report


def recheck(document):
    """Re-derive a run's calibration and gates from its raw rows: a stored
    verdict is never trusted, it may come from an older rule or be edited."""
    gates = load_gates()
    calibration = gates.rebuild_calibration(document["calibration"]["attempts"])
    return calibration, gates.evaluate_regime(document["rows"], calibration, required_pairs=gates.REQUIRED_PAIRS)


def describe(document):
    kind = document.get("kind")
    lines = [f"[{document.get('provenance', 'unknown provenance')}] {kind}"]
    if document.get("notice"):
        lines.append("  " + document["notice"])
    if kind == "native-run":
        calibration, evaluation = recheck(document)
        request = document["request"]
        lines.append(f"  {request['family']} n={request['n']}: {evaluation['pairs']} complete pairs at "
                     f"{calibration['repetitions']} repetitions (timing target {calibration['target_ns']} ns)")
        lines.append("  gates: " + ", ".join(f"{k}={v}" for k, v in evaluation["gates"].items()))
        stored = document.get("evaluation", {}).get("gates", {}).get("interpretation")
        if stored != evaluation["gates"]["interpretation"]:
            lines.append(f"  note: the recheck ({evaluation['gates']['interpretation']}) differs from the stored "
                         f"interpretation ({stored}); the recheck governs")
        lines += ["  reason: " + reason for reason in evaluation["reasons"][:8]]
        ranking = evaluation.get("ranking")
        if ranking:
            lines.append(f"  ranking: median paired difference (v1 - v0) {ranking['median_paired_delta_ns']:.1f} "
                         f"ns per repetition; variant 1 faster in {ranking['v1_faster_pairs']}/"
                         f"{ranking['pairs']} pairs. {ranking['limit']}")
        else:
            lines.append("  ranking: none (only supported comparisons are ranked)")
    elif kind == "native-sample":
        s = document["summary"]
        lines.append(f"  {s['family']} n={s['n']} drain={s['drain_mode']} pages={s['data_pages']} "
                     f"period={s['sample_period']} precise_ip={s['precise_ip']}: status {s['status']}, "
                     f"{s['samples']} samples, {s['drains']} drains")
        lines.append("  loss: " + loss_reading(s))
    else:
        raise ValueError(f"unknown report kind {kind!r}")
    if document.get("explanation"):
        lines.append("  why: " + document["explanation"])
    return "\n".join(lines)


def report(args):
    status = 0
    for path in args.reports:
        try:
            print(describe(json.loads(Path(path).read_text())))
        except (OSError, ValueError, KeyError) as error:
            print(f"{path}: unreadable report ({error})", file=sys.stderr)
            status = EXIT_INVALID
    return status


# ---------------------------------------------------------------------------


def parser():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    sub.add_parser("doctor", help="report PMU access and the exact missing capability")
    label_help = ("label the run TEST IMPLEMENTATION: the sources are a supplied test or reference "
                  "implementation, so nothing it produces is evidence of your own work (required for any "
                  "source in an assessor directory)")

    p = sub.add_parser("protocol", help="build and run the deterministic tests against your pmu.c")
    p.add_argument("--pmu-source", default=str(HERE / "pmu.c"), help="your pmu.c (default: ./pmu.c)")
    p.add_argument("--sanitizers", action="store_true", help="also run ASan/UBSan and ThreadSanitizer builds")
    p.add_argument("--test-implementation", action="store_true", help=label_help)

    p = sub.add_parser("build", help="build the optimized, uninstrumented collector")
    p.add_argument("--pmu-source", required=True, help="the pmu.c to measure with (yours)")
    for role, spec in COMPONENTS.items():
        p.add_argument(spec["option"], dest=role + "_source",
                       help=f"{spec['file']} for the {spec['family']} family (optional)")
    p.add_argument("--test-implementation", action="store_true", help=label_help)
    p.add_argument("--cc", default="gcc", help="C compiler (default: gcc)")
    p.add_argument("--output", required=True, help="new or empty directory for the collector")

    p = sub.add_parser("run", help="bounded paired counting comparison, judged by the timing gates")
    p.add_argument("--build", required=True, help="directory written by `build`")
    p.add_argument("--family", required=True)
    p.add_argument("--n", type=decimal, required=True)
    p.add_argument("--pairs", type=decimal, default=31, help="1..31; fewer than 31 cannot be supported")
    p.add_argument("--report", required=True, help="fresh JSON report path")

    p = sub.add_parser("sample", help="cycle sampling with explicit ring size, period and drain mode")
    p.add_argument("--build", required=True)
    p.add_argument("--family", required=True)
    p.add_argument("--n", type=decimal, required=True)
    p.add_argument("--variant", type=decimal, default=0)
    p.add_argument("--repetitions", type=decimal, default=16)
    p.add_argument("--drain", choices=("final", "concurrent"), default="final")
    p.add_argument("--pages", type=decimal, default=8, help="data pages, a power of two (1..256)")
    p.add_argument("--period", type=decimal, default=200000,
                   help="cycles per sample (1000..4294967295); below the host's throttle floor the run is "
                        "flagged and never lossless")
    p.add_argument("--precision", type=decimal, default=0,
                   help="precise_ip 0..3; a rejected request stays rejected")
    p.add_argument("--retain", type=decimal, default=65536, help="samples kept in the report (0..1048576)")
    p.add_argument("--report", required=True, help="fresh JSON report path")

    p = sub.add_parser("report", help="summarize run and sample reports (gates are rechecked)")
    p.add_argument("reports", nargs="+")
    return parser


def main():
    args = parser().parse_args()
    commands = {"doctor": doctor, "protocol": protocol, "build": build, "run": run, "sample": sample,
                "report": report}
    return commands[args.command](args)


if __name__ == "__main__":
    raise SystemExit(main())

"""Learner native PMU workflow, end to end, from a copy of labs/ only.

Each learner command runs under bubblewrap with the author checkout covered by
an empty tmpfs, so native.py can reach nothing but the copied labs/ tree and
the copied test implementation. Writes a receipt directory (fresh path).
"""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import shutil
import subprocess

ROOT = Path("/home/fred/dev/c_mastery")
MOUNT = Path("/mnt/learner")


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def load_snapshot():
    meminfo = Path("/proc/meminfo").read_text()
    available = next((line.split()[1] for line in meminfo.splitlines() if line.startswith("MemAvailable:")), None)
    return {"utc": datetime.now(timezone.utc).isoformat(), "loadavg": Path("/proc/loadavg").read_text().strip(),
            "mem_available_kb": int(available) if available else None}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--work", type=Path, required=True)
    parser.add_argument("--receipt", type=Path, required=True)
    args = parser.parse_args()
    work, receipt = args.work.resolve(), args.receipt.resolve()
    if receipt.exists() or work.exists():
        raise SystemExit("fresh paths required")
    work.mkdir(parents=True)
    receipt.mkdir(parents=True)
    shutil.copytree(ROOT / "labs", work / "labs", ignore=shutil.ignore_patterns("__pycache__", "build"))
    test = work / "test-implementation"
    test.mkdir()
    for source, name in ((ROOT / "assessor/performance_followthrough/pmu.c", "pmu.c"),
                         (ROOT / "assessor/algorithms/algo.c", "algo.c")):
        shutil.copyfile(source, test / name)
    # A learner workspace has no assessor/ tree. A learner's own directory
    # that merely has the name `assessor` must not be refused.
    (work / "mywork/assessor").mkdir(parents=True)
    shutil.copyfile(work / "labs/performance_followthrough/pmu.c", work / "mywork/assessor/pmu.c")
    (test / "README.txt").write_text(
        "TEST IMPLEMENTATION: author reference sources copied in only to exercise the learner "
        "workflow end to end. Nothing produced from them is learner evidence.\n")
    steps = []

    def learner(label, argv, expected):
        command = ["bwrap", "--dev-bind", "/", "/", "--tmpfs", "/mnt", "--bind", str(work), str(MOUNT),
                   "--tmpfs", str(ROOT), "--chdir", str(MOUNT / "labs/performance_followthrough"),
                   "--", "python3", "native.py", *argv]
        result = subprocess.run(command, capture_output=True, text=True, timeout=900)
        log = receipt / (label + ".txt")
        log.write_text(result.stdout + "\n--- stderr ---\n" + result.stderr)
        steps.append({"label": label, "argv": ["python3", "native.py", *argv], "isolation": command[:9],
                      "exit_code": result.returncode, "expected_exit_code": expected,
                      "log": str(log.relative_to(ROOT)), "log_sha256": sha(log)})
        if result.returncode != expected:
            raise RuntimeError(f"{label}: exit {result.returncode}, expected {expected}: {result.stderr[-800:]}")
        return result.stdout

    def checkout(label, work_dir):
        """Inside the author checkout (not isolated): an unlabelled copy of an
        assessor file outside assessor/ is still refused, by its hash."""
        copy = work_dir.parent / "assessor-copy/pmu.c"
        copy.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(ROOT / "assessor/performance_followthrough/pmu.c", copy)
        command = ["python3", str(ROOT / "labs/performance_followthrough/native.py"), "protocol",
                   "--pmu-source", str(copy)]
        result = subprocess.run(command, capture_output=True, text=True, timeout=300, cwd=ROOT)
        log = receipt / (label + ".txt")
        log.write_text(result.stdout + "\n--- stderr ---\n" + result.stderr)
        steps.append({"label": label, "argv": command, "isolation": "none: runs in the author checkout",
                      "exit_code": result.returncode, "expected_exit_code": 2,
                      "log": str(log.relative_to(ROOT)), "log_sha256": sha(log)})
        if result.returncode != 2 or "assessor implementation" not in result.stderr:
            raise RuntimeError(f"{label}: exit {result.returncode}: {result.stderr[-400:]}")

    error = None
    load_before = load_snapshot()
    impl = str(MOUNT / "test-implementation")
    build = str(MOUNT / "build/native")
    runs = MOUNT / "runs"
    try:
        doctor = json.loads(learner("doctor", ["doctor"], 0))
        scaffold = json.loads(learner("protocol-scaffold", ["protocol"], 1))
        reference = json.loads(learner("protocol-test-implementation",
                                       ["protocol", "--sanitizers", "--test-implementation",
                                        "--pmu-source", impl + "/pmu.c"], 0))
        if reference.get("provenance") != "TEST IMPLEMENTATION":
            raise RuntimeError("protocol output lacks the TEST IMPLEMENTATION label")
        # The learner's own scaffold in a directory named `assessor`: not
        # refused (exit 1 is the scaffold's semantic rejection), labelled LEARNER.
        own = json.loads(learner("protocol-own-directory-named-assessor",
                                 ["protocol", "--pmu-source", str(MOUNT / "mywork/assessor/pmu.c")], 1))
        if own.get("provenance") != "LEARNER":
            raise RuntimeError("a learner directory named assessor was mislabelled")
        checkout("checkout-copy-of-assessor-refused", work)
        manifest = json.loads(learner("build-test-implementation", [
            "build", "--pmu-source", impl + "/pmu.c", "--algorithms-source", impl + "/algo.c",
            "--test-implementation", "--output", build], 0))
        learner("run-numeric-8192", ["run", "--build", build, "--family", "numeric", "--n", "8192",
                                     "--report", str(runs / "run-numeric-8192.json")], 0)
        learner("run-sort-128", ["run", "--build", build, "--family", "sort", "--n", "128",
                                 "--report", str(runs / "run-sort-128.json")], 0)
        learner("run-service-unavailable", ["run", "--build", build, "--family", "service", "--n", "64",
                                            "--report", str(runs / "run-service-64.json")], 3)
        for drain in ("final", "concurrent"):
            learner(f"sample-saturation-{drain}", [
                "sample", "--build", build, "--family", "numeric", "--n", "1048576", "--repetitions", "64",
                "--pages", "1", "--period", "200000", "--drain", drain, "--retain", "0",
                "--report", str(runs / f"sample-saturation-{drain}.json")], 0)
        learner("sample-default", ["sample", "--build", build, "--family", "numeric", "--n", "65536",
                                   "--repetitions", "256", "--report", str(runs / "sample-default.json")], 0)
        learner("sample-precision-3", ["sample", "--build", build, "--family", "numeric", "--n", "65536",
                                       "--precision", "3", "--report", str(runs / "sample-precision-3.json")], 0)
        learner("sample-throttled-period-20000", [
            "sample", "--build", build, "--family", "numeric", "--n", "1048576", "--repetitions", "16",
            "--period", "20000", "--retain", "0", "--report", str(runs / "sample-throttled-20000.json")], 0)
        throttled = json.loads((work / "runs/sample-throttled-20000.json").read_text())["summary"]
        if throttled["lossless"] or not throttled["throttle_bound"]:
            raise RuntimeError("a throttle-bound period was reported lossless or unflagged")
        reports = sorted(str(p).replace(str(work), str(MOUNT)) for p in (work / "runs").glob("*.json"))
        learner("report", ["report", *reports], 0)
    except (RuntimeError, ValueError, subprocess.TimeoutExpired) as exc:
        error = str(exc)
    copies = {}
    for path in sorted((work / "runs").glob("*.json")) + [work / "build/native/build.json"]:
        if path.is_file():
            target = receipt / ("build.json" if path.name == "build.json" else path.name)
            shutil.copyfile(path, target)
            copies[str(target.relative_to(ROOT))] = sha(target)
    summary = {
        "schema_version": 1, "created_utc": datetime.now(timezone.utc).isoformat(),
        "passed": error is None, "error": error,
        "provenance": "TEST IMPLEMENTATION",
        "round": "final review round; supersedes reviews/v5/expansion/polish_revision/learner_native_v2 "
                 "(which saturated at period 100000 while the documented period is 200000)",
        "load": {"before": load_before, "after": load_snapshot()},
        "notice": "Author reference pmu.c and algo.c copied into the scratch workspace and labelled TEST "
                  "IMPLEMENTATION to exercise the learner workflow. Not learner evidence, not an "
                  "independent assessment, and not the extracted-learner-archive test (route packaging is "
                  "separate).",
        "workspace": str(work), "workspace_view": str(MOUNT),
        "isolation": "bubblewrap: the author checkout path is covered by an empty tmpfs for every step; "
                     "only the copied labs/ tree and test-implementation/ are reachable",
        "copied_inputs_sha256": {"labs/performance_followthrough/native.py": sha(work / "labs/performance_followthrough/native.py"),
                                 "labs/support/measurement_gates.py": sha(work / "labs/support/measurement_gates.py"),
                                 "test-implementation/pmu.c": sha(test / "pmu.c"),
                                 "test-implementation/algo.c": sha(test / "algo.c")},
        "steps": steps, "report_copies_sha256": copies,
        "perf_event_paranoid": Path("/proc/sys/kernel/perf_event_paranoid").read_text().strip(),
        "host_policy_changed": False,
    }
    (receipt / "receipt.json").write_text(json.dumps(summary, indent=2) + "\n")
    print(json.dumps({"passed": summary["passed"], "error": error, "steps": len(steps),
                      "receipt": str(receipt / "receipt.json")}))
    return 0 if error is None else 1


if __name__ == "__main__":
    raise SystemExit(main())

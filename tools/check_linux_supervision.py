"""Check the hosted C17 supervision lab offline or run the full QEMU author campaign."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[1]
LAB = ROOT / "labs/linux_supervision"
PRIVATE = ROOT / "assessor/linux_supervision"
REPORT = ROOT / "reviews/v5/expansion/linux_supervision/current.json"
CASES = ("namespace", "pids", "shutdown", "network", "telemetry")
sys.path.insert(0, str(LAB))
from observations import verify


class Unavailable(RuntimeError):
    pass


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def inputs(full):
    directories = [LAB, ROOT / "fixtures/linux_supervision"]
    if full:
        directories.append(PRIVATE)
    files = {Path(__file__).resolve(), ROOT / "tools/test_linux_supervision_guest.py",
             ROOT / "research/v5/expansion/linux_supervision/resources.json"}
    if full:
        files.update((ROOT / "cmastery").glob("*.py"))
        for name in ("tools/check_expansion_campaigns.py", "tools/run_unit_campaigns_v5.py",
                     "tools/accept_v5.py", "tools/public_exports.py", "tools/public_validation.py",
                     "curriculum/edition.json", "curriculum/v5/exercises.json",
                     "curriculum/v5/routes/contextual-v3.json",
                     "curriculum/v5/routes/contextual-v3.source.json"):
            files.add(ROOT / name)
    for directory in directories:
        for path in directory.rglob("*"):
            parts = path.relative_to(directory).parts
            if path.is_file() and not {"build", "__pycache__"}.intersection(parts):
                files.add(path)
    mapping = json.loads((LAB / "family.json").read_text())
    for exercise in mapping["exercises"]:
        files.add(ROOT / "curriculum/v5/exercises" / (exercise["id"] + ".md"))
    return {p.relative_to(ROOT).as_posix(): sha(p) for p in sorted(files)}


def run(argv, expected=0):
    cmd = [str(word) for word in argv]
    result = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True, timeout=300)
    if result.returncode != expected:
        raise ValueError(f"{cmd} exited {result.returncode}\n{result.stdout}\n{result.stderr}")
    return (result.stdout + result.stderr).strip()


def reject(case, serial):
    try:
        verify(case, serial)
    except ValueError as exc:
        return str(exc)
    raise ValueError(f"{case} oracle accepted a negative control")


def normalized(value, work):
    return str(value).replace(str(work), "<campaign>").replace(str(ROOT), "<checkout>")


def build(cc, role, work, implementation=None):
    folder = work / f"{cc}-{role}"
    configure = ["cmake", "-S", LAB, "-B", folder, f"-DCMAKE_C_COMPILER={cc}",
                 "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"]
    if implementation is not None:
        configure.append(f"-DCMASTERY_ASSESSOR_ROOT={implementation}")
    run(configure)
    compile_cmd = ["cmake", "--build", folder, "--parallel", "2"]
    run(compile_cmd)
    program = folder / "supervise"
    guard = run([program, "network", "A"], expected=1)
    if "through the Linux supervision guest fixture" not in guard:
        raise ValueError(f"{cc}-{role}: wrong failure from ordinary host invocation")
    commands = json.loads((folder / "compile_commands.json").read_text())
    return program, {
        "configure": [normalized(word, work) for word in configure],
        "build": [normalized(word, work) for word in compile_cmd],
        "compile_commands": [normalized(row["command"], work) for row in commands],
        "host_guard": guard,
        "program_sha256": sha(program),
    }


def host_unit(cc, work):
    exe = work / (cc + "-pipe-test")
    cmd = [cc, "-std=c17", "-Wall", "-Wextra", "-Werror", "-pedantic"]
    if cc == "clang":
        cmd += ["-O1", "-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
    else:
        cmd.append("-O2")
    cmd += [PRIVATE / "support.c", PRIVATE / "tests/support_test.c", "-o", exe]
    run(cmd)
    output = run([exe])
    if "private host pipe checks passed" not in output:
        raise ValueError("host pipe test did not complete")
    return {"command": [normalized(x, work) for x in cmd], "output": output}


def boot(program, role, work, cache):
    label = program.parent.name
    out = work / (label + "-guest")
    cmd = [sys.executable, LAB / "guest.py", "run", "--program", program, "--out", out]
    if cache:
        cmd += ["--cache", cache]
    proc = subprocess.run(cmd, cwd=ROOT, text=True, capture_output=True, timeout=300)
    launch = json.loads((out / "launch.json").read_text())
    if proc.returncode == 77 and launch["status"] == "unavailable":
        raise Unavailable(launch["reason"])
    expected_status = "case-failed" if role == "learner" else "completed"
    expected_code = 1 if role == "learner" else 0
    if proc.returncode != expected_code or launch["status"] != expected_status:
        raise ValueError(f"{label}: {proc.stdout} {proc.stderr} {launch.get('reason', '')}")
    if sha(program) != launch["sha256"]["program"] or sha(out / "serial.txt") != launch["sha256"]["serial"]:
        raise ValueError(f"{label}: unstable payload or serial")
    serial = (out / "serial.txt").read_text()
    observations = {}
    replay_checks = {}
    for case in CASES:
        if role == "reference":
            observations[case] = {"verified": verify(case, serial)}
            prefix = f"LS_OUTER name={case} "
            rows = serial.splitlines()
            missing = next(i for i, row in enumerate(rows) if row.startswith(prefix))
            replay_checks[case] = reject(case, "\n".join(rows[:missing] + rows[missing + 1:]))
        else:
            reason = reject(case, serial)
            if role == "mutated":
                from mutants import FAILURE_PREFIX
                if not reason.startswith(FAILURE_PREFIX[case]):
                    raise ValueError(f"{case} mutant was rejected for an unrelated reason: {reason}")
            observations[case] = {"rejected": reason}
    if role == "learner":
        for cell in launch["cases"]:
            message = f"{cell['name']}/{cell['variant']} requires a learner implementation"
            if cell["status"] != 1 or message not in serial:
                raise ValueError(f"{label}: stub failed for an unexpected reason in {cell}")
    info = {
        "role": role, "launch_status": launch["status"], "observations": observations,
        "replay_missing_outer_rejections": replay_checks,
        "cases": launch["cases"], "sha256": launch["sha256"],
        "guest_packages": launch["packages"], "guest_snapshot": launch["snapshot"],
        "tools": launch["tool_versions"], "qemu_returncode": launch["qemu_returncode"],
        "qemu_command": launch["command"], "timeout_seconds": launch.get("timeout_seconds"),
        "serial_limit_bytes": launch.get("serial_limit_bytes"),
    }
    return info, out / "serial.txt"


def empty_cache_check(program, work):
    folder = work / "unavailable-control"
    cmd = [sys.executable, LAB / "guest.py", "run", "--program", program,
           "--case", "namespace", "--cache", work / "empty-cache", "--out", folder]
    run(cmd, expected=77)
    launch = json.loads((folder / "launch.json").read_text())
    if launch["status"] != "unavailable" or launch["cases"] or launch["qemu_returncode"] is not None:
        raise ValueError("unavailable control incorrectly recorded a guest outcome")
    return {"status": launch["status"], "case_outcomes": launch["cases"],
            "reason": normalized(launch["reason"], work)}


def fixture_reasoning():
    """Recompute the public byte oracle without launching or imitating a guest."""
    workloads = json.loads((ROOT / "fixtures/linux_supervision/workloads.json").read_text())
    request = bytes(workloads["network"]["request"])
    wire = sum(request).to_bytes(4, "big")
    workload = workloads["telemetry"]
    frames, width = workload["frames"], workload["request_octets"]
    total = sum(sum((i + k) % 256 for k in range(width)) for i in range(frames))
    if len(request) != 8 or wire.hex() != "00000283" or total != 8355840:
        raise ValueError("public workload no longer matches the T24 checksum contract")
    return {"network_request_hex": request.hex(), "network_reply_hex": wire.hex(),
            "telemetry_checksum": total, "input_bytes": frames * width,
            "reply_bytes": frames * workload["reply_octets"],
            "scope": "Offline arithmetic for the supplied workload."}


def campaign(args, work):
    original = inputs(args.full)
    result = {
        "schema_version": 1, "profile": "full" if args.full else "portable",
        "recorded_at": datetime.now(timezone.utc).isoformat(),
        "scope": "Author tests of compiled hosted C and the Linux guest fixture.",
        "independent_learner": "not-assessed", "native_hardware_observation": "not-run",
        "host": {"kernel": platform.release(), "machine": platform.machine(),
                 "python": platform.python_version(), "cmake": run(["cmake", "--version"]).splitlines()[0]},
        "inputs": original, "builds": {}, "guest_runs": {}, "portable_rejections": {},
        "fixture_reasoning": fixture_reasoning(),
        "launcher_regressions": run([sys.executable, ROOT / "tools/test_linux_supervision_guest.py"]),
    }
    raw = {}
    for case in CASES:
        result["portable_rejections"][case] = {
            "empty": reject(case, ""),
            "unavailable": reject(case, "LS_BOOT_END status=77\n"),
            "missing_fixture":
            reject(case, f"LS {case} variant=unverified\nLS_BOOT_END status=0\n"),
        }
    for cc in ("gcc", "clang"):
        result["host"][cc] = run([cc, "--version"]).splitlines()[0]
        learner, record = build(cc, "learner", work)
        result["builds"][learner.parent.name] = record
        if cc == "gcc":
            result["unavailable_control"] = empty_cache_check(learner, work)
        if not args.full:
            continue
        record["host_pipe_test"] = host_unit(cc, work)
        reference, record = build(cc, "reference", work, PRIVATE)
        result["builds"][reference.parent.name] = record
        for role, program in (("reference", reference), ("learner", learner)):
            label = program.parent.name
            result["guest_runs"][label], raw[label] = boot(program, role, work, args.cache)
    if args.full:
        sys.path.insert(0, str(PRIVATE))
        result["trace_mutations"] = changed_transcript_checks(raw["gcc-reference"].read_text())
        sources = work / "mutated"
        run([sys.executable, PRIVATE / "mutants.py", "--output", sources])
        mutant, record = build("gcc", "mutated", work, sources)
        record["mutated_sources"] = {p.name: sha(p) for p in sorted(sources.iterdir())}
        label = mutant.parent.name
        result["builds"][label] = record
        result["guest_runs"][label], raw[label] = boot(mutant, "mutated", work, args.cache)
        from archive_test import verify_archive
        result["exported_workspaces"], extracted = verify_archive(work, args.cache)
        raw.update(extracted)
    if inputs(args.full) != original:
        raise ValueError("bound inputs changed during the campaign; rerun with the final sources")
    result["passed"] = True
    return result, raw


def publish(result, raw, path):
    path.parent.mkdir(parents=True, exist_ok=True)
    for label, source in raw.items():
        relative = Path("raw") / (label + "-" + sha(source)[:16] + ".txt")
        log = path.parent / relative
        log.parent.mkdir(exist_ok=True)
        if not log.exists():
            shutil.copyfile(source, log)
        if label.startswith("exported-"):
            _, case, role = label.split("-")
            record = result["exported_workspaces"][case]["runs"][role]
        else:
            record = result["guest_runs"][label]
        if sha(log) != record["sha256"]["serial"]:
            raise ValueError(f"archive does not match fresh serial: {log}")
        record["serial"] = relative.as_posix()
    with tempfile.NamedTemporaryFile(dir=path.parent, mode="w", delete=False) as tmp:
        tmp.write(json.dumps(result, indent=2) + "\n")
    os.replace(tmp.name, path)


def changed_transcript_checks(serial):
    sys.path.insert(0, str(PRIVATE))
    from mutants import transcript_mutations
    results = {}
    for label, (case, altered, expected) in transcript_mutations(serial).items():
        reason = reject(case, altered)
        if not reason.startswith(expected):
            raise ValueError(f"{label} rejected for an unrelated reason: {reason}")
        results[label] = reason
    return results


def validate_cells(serial, cases, role, status, rows):
    from guest import classify
    parsed = {"qemu_returncode": 0, "cases": []}
    actual = classify(serial.splitlines(), cases, parsed)
    if actual != status or parsed["cases"] != rows:
        raise ValueError("archived raw terminal states disagree with launch metadata")
    if role == "learner":
        for cell in rows:
            if f"{cell['name']}/{cell['variant']} requires a learner implementation" not in serial:
                raise ValueError("archived rejection did not execute the expected starter")


def verify_report(path):
    receipt = json.loads(path.read_text())
    if receipt.get("profile") not in {"full", "portable"} or receipt.get("schema_version") != 1:
        raise ValueError("unrecognized supervision receipt")
    full = receipt["profile"] == "full"
    if not receipt["passed"] or receipt["inputs"] != inputs(full):
        raise ValueError("receipt has stale source bindings or a failed result")
    labels = {f"{cc}-{role}" for cc in ("gcc", "clang") for role in ("reference", "learner")}
    labels.add("gcc-mutated")
    if set(receipt["guest_runs"]) != (labels if full else set()):
        raise ValueError("wrong set of guest runs")
    for label, record in receipt["guest_runs"].items():
        from guest import CASES as variants, PACKAGES, SNAPSHOT
        status = 1 if label.endswith("-learner") else 0
        expected = [{"name": name, "variant": variant, "status": status}
                    for name in CASES for variant in variants[name]]
        if (record["cases"] != expected or record["qemu_returncode"] != 0
                or record["guest_packages"] != PACKAGES or record["guest_snapshot"] != SNAPSHOT
                or record["sha256"]["runner"] != sha(LAB / "guest.py")
                or record["sha256"]["fixture_source"] != sha(LAB / "guest_fixture.c")):
            raise ValueError(f"{label}: recorded run identity or variant coverage differs")
        raw = (path.parent / record["serial"]).resolve()
        if not raw.is_relative_to(path.parent.resolve()) or sha(raw) != record["sha256"]["serial"]:
            raise ValueError(f"{label}: archived guest serial changed")
        if record["sha256"]["program"] != receipt["builds"][label]["program_sha256"]:
            raise ValueError(f"{label}: different executable reached the guest")
        transcript = raw.read_text()
        validate_cells(transcript, list(CASES), record["role"], record["launch_status"], record["cases"])
        if label == "gcc-reference" and receipt["trace_mutations"] != changed_transcript_checks(transcript):
            raise ValueError("retained transcript mutation rejections changed")
        observed = {}
        for case in CASES:
            if label.endswith("-reference"):
                observed[case] = {"verified": verify(case, transcript)}
            else:
                reason = reject(case, transcript)
                if label.endswith("-mutated"):
                    sys.path.insert(0, str(PRIVATE))
                    from mutants import FAILURE_PREFIX
                    if not reason.startswith(FAILURE_PREFIX[case]):
                        raise ValueError(f"{case}: archived mutant failed for an unrelated reason")
                observed[case] = {"rejected": reason}
        if observed != record["observations"]:
            raise ValueError(f"{label}: recorded verdict disagrees with the serial")
    if full:
        workspaces = receipt["exported_workspaces"]
        if set(workspaces) != set(CASES):
            raise ValueError("incomplete selection of exported learner workspaces")
        for case, workspace in workspaces.items():
            if set(workspace["runs"]) != {"reference", "learner"}:
                raise ValueError(f"{case}: incomplete exported runs")
            for role, record in workspace["runs"].items():
                raw = (path.parent / record["serial"]).resolve()
                if (not raw.is_relative_to(path.parent.resolve()) or sha(raw) != record["sha256"]["serial"]
                        or record["program_sha256"] != record["sha256"]["program"]
                        or record["sha256"]["runner"] != workspace["entries"]["guest.py"]
                        or record["sha256"]["fixture_source"] != workspace["entries"]["guest_fixture.c"]):
                    raise ValueError(f"{case}: archived workspace proof changed")
                serial = raw.read_text()
                validate_cells(serial, [case], role, record["status"], record["cases"])
                if role == "reference" and verify(case, serial) != record["facts"]:
                    raise ValueError(f"{case}: archived public replay diverged")
                if role == "learner":
                    reject(case, serial)
    print(f"{receipt['profile']} receipt matches source and archived observations")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--full", action="store_true", help="execute all assessor, stub and mutation guest runs")
    parser.add_argument("--cache", type=Path, help="use restored guest.py packages in this directory")
    parser.add_argument("--report", type=Path, nargs="?", const=REPORT,
                        help="archive evidence; plain --report selects the canonical full receipt")
    parser.add_argument("--verify-report", type=Path, metavar="PATH",
                        help="recheck a saved receipt, current source hashes and archived serial")
    args = parser.parse_args()
    if args.report and args.report.resolve() == REPORT and not args.full:
        parser.error("use --full to write the canonical full receipt")
    try:
        if args.verify_report:
            verify_report(args.verify_report)
            return 0
        with tempfile.TemporaryDirectory(prefix="cmastery-linux-supervision-") as directory:
            result, raw = campaign(args, Path(directory))
            if args.report:
                publish(result, raw, args.report)
        print(f"{result['profile']} passed; {len(result['builds'])} builds, "
              f"{len(result['guest_runs'])} guest runs")
        return 0
    except Unavailable as exc:
        print(f"guest unavailable: {exc}", file=sys.stderr)
        return 77
    except (ValueError, KeyError, OSError, subprocess.SubprocessError) as exc:
        print(f"campaign failed: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())

"""Run B01 author evidence without writing historical acceptance receipts.

Run inside Linux/WSL. A successful report means finite author controls passed;
it never denotes learner mastery, independent review or physical validation.
"""
from __future__ import annotations
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
from v5_common import identity as curriculum_identity

ROOT = Path(__file__).resolve().parents[1]
CASES = ("semantic", "extent", "cursor", "codec", "lifecycle", "overlap", "handles", "defense")

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--report", type=Path, default=ROOT / "build/foundation_depth/report.json")
    parser.add_argument("--compiler", action="append", choices=("gcc", "clang"))
    parser.add_argument("--skip-target", action="store_true", help="record target audit unavailable; overall campaign remains incomplete")
    args = parser.parse_args()
    report_path = args.report.resolve()
    build = ROOT / "build/foundation_depth/campaign"
    build.mkdir(parents=True, exist_ok=True)
    def source_hashes() -> dict[str, str]:
        paths = []
        for directory in ("labs/foundation_depth", "assessor/foundation_depth", "fixtures/foundation_depth"):
            paths.extend(p for p in (ROOT / directory).rglob("*") if p.is_file() and "__pycache__" not in p.parts)
        paths.extend((Path(__file__).resolve(), ROOT / "tools/verify_foundation_depth_fixtures.py"))
        return {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(paths)}
    hashes = source_hashes()
    curriculum_input = curriculum_identity()
    report = {
        "schema_version": 5, "family_report_schema_version": 1, "batch": "B01", "family": "foundation_depth",
        "started_utc": datetime.now(timezone.utc).isoformat(), "status": "running", "passed": False,
        "inspected_paths": hashes, "source_hashes": hashes,
        "input_sha256": curriculum_input["sha256"],
        "family_input_sha256": hashlib.sha256(json.dumps(hashes, sort_keys=True).encode()).hexdigest(),
        "commands": [], "profiles": [], "learner_stub_rejections": [],
        "independent_assessment": "not-performed", "physical_validation": "not-performed",
        "limitations": ["Finite same-author evidence is not absolute mastery or independent review.",
                        "Target audit compiles/links and inspects artifacts; no target execution or hardware observation."],
    }
    def run(argv: list[str], label: str, allowed: tuple[int, ...] = (0,)) -> subprocess.CompletedProcess:
        proc = subprocess.run(argv, cwd=ROOT, text=True, capture_output=True, timeout=180,
                              env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:halt_on_error=1", "UBSAN_OPTIONS": "halt_on_error=1:print_stacktrace=1"})
        log = build / f"{len(report['commands']):03d}-{label}.log"
        log.write_text("$ " + " ".join(argv) + "\n" + proc.stdout + "\nSTDERR\n" + proc.stderr, encoding="utf-8")
        report["commands"].append({"label": label, "argv": argv, "cwd": str(ROOT), "exit_code": proc.returncode,
                                   "stdout": proc.stdout, "stderr": proc.stderr, "log": str(log.relative_to(ROOT))})
        if proc.returncode not in allowed:
            raise RuntimeError(f"{label}: exit {proc.returncode}; see {log}")
        return proc
    failure = None
    try:
        compilers = args.compiler or ["gcc", "clang"]
        for executable in ["cmake", "ctest", *compilers]:
            if not shutil.which(executable):
                raise RuntimeError(f"required executable unavailable: {executable}")
            run([executable, "--version"], "version-" + executable)
        run([sys.executable, "tools/verify_foundation_depth_fixtures.py"], "fixtures")
        for cc in compilers:
            # The ISO requirement is a diagnostic; strict rejection is our compiler policy.
            constraint = run([cc, "-std=c17", "-pedantic-errors", "-fsyntax-only",
                              "fixtures/foundation_depth/semantic/pointer_constraint.c"],
                             cc + "-constraint", allowed=(1,))
            if not constraint.stderr.strip():
                raise RuntimeError("constraint control failed without a diagnostic")
            for safe in ("mixed_sign", "unsigned_wrap", "short_circuit", "call_order"):
                output = build / (cc + "-" + safe)
                run([cc, "-std=c17", "-pedantic-errors", "fixtures/foundation_depth/semantic/" + safe + ".c", "-o", str(output)], cc + "-compile-" + safe)
                observed = run([str(output)], cc + "-run-" + safe)
                if safe == "call_order" and observed.stdout not in ("AB", "BA"):
                    raise RuntimeError("defined function-call control produced an unexpected output")
            for profile, kind, sanitize in (("debug", "Debug", "OFF"), ("release", "Release", "OFF"), ("sanitize", "Debug", "ON")):
                name = cc + "-" + profile
                assessor_build = build / (name + "-assessor")
                common = ["-DCMAKE_C_COMPILER=" + cc, "-DCMAKE_BUILD_TYPE=" + kind, "-DCMASTERY_SANITIZE=" + sanitize]
                run(["cmake", "-S", "labs/foundation_depth", "-B", str(assessor_build), *common,
                     "-DCMASTERY_ASSESSOR_ROOT=" + str(ROOT / "assessor/foundation_depth")], name + "-assessor-configure")
                run(["cmake", "--build", str(assessor_build), "--parallel", "4"], name + "-assessor-build")
                inventory = run(["ctest", "--test-dir", str(assessor_build), "--show-only=json-v1"], name + "-assessor-inventory")
                expected_tests = {"foundation_depth." + case for case in CASES} | {"foundation_depth.mutation." + case for case in CASES} | {"foundation_depth.authorization_observation"}
                if {test["name"] for test in json.loads(inventory.stdout)["tests"]} != expected_tests:
                    raise RuntimeError("assessor campaign did not run the required 8 anchors, 8 mutation controls and observation")
                run(["ctest", "--test-dir", str(assessor_build), "--output-on-failure"], name + "-assessor-test")
                run([sys.executable, "tools/verify_foundation_depth_fixtures.py", "--driver", str(assessor_build / "depth-oracle-driver")], name + "-oracle")
                learner_build = build / (name + "-learner")
                run(["cmake", "-S", "labs/foundation_depth", "-B", str(learner_build), *common,
                     "-DCMASTERY_ASSESSOR_ROOT="], name + "-learner-configure")
                run(["cmake", "--build", str(learner_build), "--parallel", "4"], name + "-learner-build")
                for case in CASES:
                    rejected = run([str(learner_build / "depth-tests"), case], name + "-learner-" + case, allowed=(1,))
                    if "FD_CHECK:" not in rejected.stderr:
                        raise RuntimeError("learner rejection was not an identified semantic assertion")
                    report["learner_stub_rejections"].append({"compiler": cc, "profile": profile, "case": case, "status": "rejected"})
                report["profiles"].append({"compiler": cc, "profile": profile, "assessor_tests": 17,
                                            "compiled_mutants_rejected": 8, "learner_stubs_rejected": 8,
                                            "oracle_cases": 2048, "status": "passed"})
        if args.skip_target:
            report["target_audit"] = {"status": "unavailable", "reason": "explicit --skip-target", "execution": "not-run"}
        else:
            prefix = "arm-none-eabi-"
            for name in ("gcc", "nm", "objdump", "readelf", "size"):
                if not shutil.which(prefix + name):
                    raise RuntimeError("required target audit tool unavailable: " + prefix + name)
            run([prefix + "gcc", "--version"], "target-version")
            target = build / "cortex-m4"
            target.mkdir(exist_ok=True)
            flags = ["-std=c17", "-O2", "-mcpu=cortex-m4", "-mthumb", "-ffreestanding", "-fno-builtin",
                     "-ffunction-sections", "-fdata-sections", "-fno-unwind-tables", "-fno-asynchronous-unwind-tables",
                     "-fstack-usage", "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-Ilabs/foundation_depth/include"]
            for source, basename in (("assessor/foundation_depth/depth.c", "depth"), ("labs/foundation_depth/freestanding/entry.c", "entry")):
                run([prefix + "gcc", *flags, "-c", source, "-o", str(target / (basename + ".o"))], "target-compile-" + basename)
                undefined = run([prefix + "nm", "-u", str(target / (basename + ".o"))], "target-undefined-" + basename)
                if basename == "depth" and undefined.stdout.strip():
                    raise RuntimeError("portable object has implicit runtime/helper dependencies")
            image = target / "depth.elf"
            run([prefix + "gcc", "-mcpu=cortex-m4", "-mthumb", "-nostdlib", "-nodefaultlibs", "-nostartfiles",
                 str(target / "depth.o"), str(target / "entry.o"), "-T", "labs/foundation_depth/freestanding/audit.ld",
                 "-Wl,--no-undefined,--gc-sections,-Map=" + str(target / "depth.map"), "-o", str(image)], "target-link")
            undefined = run([prefix + "nm", "-u", str(image)], "target-undefined-final")
            if undefined.stdout.strip():
                raise RuntimeError("strict final image has unresolved symbols")
            symbols = run([prefix + "nm", "--defined-only", str(image)], "target-symbol-ownership")
            if re.search(r"\b(?:memcpy|memmove|memset|malloc|free|__aeabi_\w+|__atomic_\w+)\b", symbols.stdout):
                raise RuntimeError("strict image contains an undeclared runtime implementation")
            linkmap = (target / "depth.map").read_text()
            if re.search(r"\.a(?:\b|\()|\.so(?:\b|\()", linkmap):
                raise RuntimeError("strict link unexpectedly included a library")
            run([prefix + "objdump", "-d", str(image)], "target-disassembly")
            run([prefix + "readelf", "-h", "-S", "-l", str(image)], "target-elf")
            run([prefix + "size", str(image)], "target-size")
            report["target_audit"] = {"status": "passed", "target": "Cortex-M4 Thumb", "execution": "not-run",
                "startup": "link-audit entry only, no reset/vector startup", "undefined_symbols": [],
                "linked_objects": ["depth.o", "entry.o"], "libraries": [], "implicit_helpers": [],
                "allocation_free_reachable_core": ["extent", "cursor", "codec", "overlap", "handles", "defense"],
                "artifacts": {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(target.iterdir()) if p.is_file()}}
        report["source_stable"] = source_hashes() == hashes
        report["canonical_source_stable"] = curriculum_identity()["sha256"] == curriculum_input["sha256"]
        report["source_changed_during_run"] = not report["canonical_source_stable"]
        if not report["source_stable"] or not report["canonical_source_stable"]:
            raise RuntimeError("campaign sources changed during execution; retain observations and rerun against stable sources")
        report["passed"] = not args.skip_target
        report["status"] = "passed" if report["passed"] else "incomplete"
    except (RuntimeError, subprocess.TimeoutExpired, OSError) as error:
        failure = str(error)
        report["status"] = "failed"
        report["failure"] = failure
    finally:
        report["finished_utc"] = datetime.now(timezone.utc).isoformat()
        report_path.parent.mkdir(parents=True, exist_ok=True)
        report_path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"status": report["status"], "passed": report["passed"], "profiles": len(report["profiles"]),
                      "learner_stubs_rejected": len(report["learner_stub_rejections"]), "report": str(report_path), "failure": failure}, indent=2))
    return 0 if report["passed"] else 1

if __name__ == "__main__":
    raise SystemExit(main())

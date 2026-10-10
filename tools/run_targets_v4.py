"""Run current target/comparison campaigns without changing historical v3 receipts.

Run this after canonical sources and the learner workflow are frozen. The old
runners still emit v3 paths; this adapter snapshots and restores every historical
file byte, discards preexisting output names before execution, and publishes only
new observations under reviews/v4 with source pins. No hardware run is inferred.
"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
from pathlib import Path
import subprocess
import sys

from common import ROOT, dump, read_json, sha
from v2_common import source_identity

TARGET_RECEIPTS = (
    "target_cortex-m4-qemu_default.json", "target_stm32f439_default.json",
    "target_stm32h745_default.json", "target_x86-smp_1.json",
    "target_x86-smp_2.json", "target_x86-smp_4.json",
)
OUTPUTS = (*TARGET_RECEIPTS, "target_matrix.json", "os_comparison.json")


def safe_folder(relative):
    expected = ROOT.resolve() / relative
    folder = ROOT / relative
    if folder.resolve() != expected:
        raise ValueError("Redirected receipt directory: " + relative)
    folder.mkdir(parents=True, exist_ok=True)
    return folder


def historical_files(folder):
    files = {}
    for path in folder.rglob("*"):
        if path.is_file():
            if path.is_symlink() or not path.resolve().is_relative_to(folder.resolve()):
                raise ValueError("Historical receipt escapes its directory: " + str(path))
            files[path.relative_to(folder).as_posix()] = path.read_bytes()
    return files


def restore_history(folder, before):
    # Delete only individually checked newly created receipt files, never a tree.
    for path in folder.rglob("*"):
        if path.is_file() and path.relative_to(folder).as_posix() not in before:
            if not path.resolve().is_relative_to(folder.resolve()):
                raise ValueError("Refusing escaped historical cleanup: " + str(path))
            path.unlink()
    for relative, content in before.items():
        path = folder / relative
        if not path.resolve().is_relative_to(folder.resolve()):
            raise ValueError("Refusing escaped historical restoration: " + relative)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(content)
    if historical_files(folder) != before:
        raise ValueError("Historical v3 receipt bytes were not restored")


def run(freertos_root=None, native=False):
    historical = safe_folder("reviews/v3")
    current = safe_folder("reviews/v4")
    before = historical_files(historical)
    identity = source_identity()
    captured = {}
    executions = []
    for name in ("target_matrix.json", "os_comparison.json", "targets_wrapper.json"):
        dump(current / name, {"schema_version": 4, "status": "pending", "passed": False,
                              "input_sha256": identity["sha256"],
                              "inspected_paths": identity["files"]})
    try:
        # Prevent a runner that fails before writing from reusing a stale success.
        for name in OUTPUTS:
            path = historical / name
            if path.exists():
                path.unlink()
        commands = [("run_target_matrix.py", "target_matrix.json"),
                    ("run_os_comparison.py", "os_comparison.json")]
        for script, receipt in commands:
            command = [sys.executable, "-X", "utf8", str(ROOT / "tools" / script)]
            if native:
                command.append("--native")
            if script == "run_os_comparison.py" and freertos_root is not None:
                command += ["--freertos-root", str(freertos_root)]
            print("Running " + script + "; current receipts will be written to reviews/v4", flush=True)
            try:
                completed = subprocess.run(command, cwd=ROOT, capture_output=True,
                                           text=True, encoding="utf-8", errors="replace",
                                           timeout=7200)
                exit_code = completed.returncode
                output = completed.stdout + "\n" + completed.stderr
            except subprocess.TimeoutExpired as error:
                exit_code = 124
                output = "Campaign exceeded the wrapper timeout: " + str(error)
            log = current / (script.removesuffix(".py") + ".log")
            log.write_text(output, encoding="utf-8", newline="\n")
            executions.append({"runner": script, "receipt": receipt, "command": command,
                               "exit_code": exit_code, "log": log.relative_to(ROOT).as_posix(),
                               "log_sha256": sha(log)})
        for name in OUTPUTS:
            path = historical / name
            if path.is_file():
                captured[name] = read_json(path)
    finally:
        restore_history(historical, before)

    changed = source_identity()["sha256"] != identity["sha256"]
    recorded_at = datetime.now(timezone.utc).isoformat()
    for name, record in captured.items():
        record = dict(record)
        record.update(schema_version=4, input_sha256=identity["sha256"],
                      recorded_at=recorded_at, inspected_paths=identity["files"],
                      source_changed_during_wrapper=changed,
                      historical_v3_bytes_preserved=True)
        if changed:
            record.update(status="failed", passed=False)
        dump(current / name, record)

    successful = True
    for execution in executions:
        name = execution["receipt"]
        record = dict(captured.get(name, {}))
        passed = (execution["exit_code"] == 0 and record.get("passed") is True
                  and record.get("status") == "passed" and not changed)
        if name == "target_matrix.json":
            passed = passed and all(captured.get(n, {}).get("status") == "passed"
                                    for n in TARGET_RECEIPTS)
        record.update(schema_version=4, status="passed" if passed else "failed",
                      passed=passed, input_sha256=identity["sha256"],
                      inspected_paths=identity["files"], recorded_at=recorded_at,
                      execution=execution, source_changed_during_wrapper=changed,
                      historical_v3_bytes_preserved=True,
                      physical_observations="not-run")
        if name not in captured:
            record["reason"] = "Runner produced no new receipt; historical success was not reused."
        if changed:
            record["reason"] = "Canonical sources changed during execution."
        dump(current / name, record)
        successful = successful and passed
    summary = {"schema_version": 4, "passed": successful,
               "status": "passed" if successful else "failed", "recorded_at": recorded_at,
               "input_sha256": identity["sha256"], "inspected_paths": identity["files"],
               "historical_v3_bytes_preserved": True, "campaigns": executions,
               "source_changed_during_run": changed, "physical_observations": "not-run",
               "scope": "Six actual retained target configurations and the pinned hosted Linux/FreeRTOS comparison."}
    dump(current / "targets_wrapper.json", summary)
    print("V4 target campaigns " + summary["status"] + "; historical v3 bytes preserved.")
    return 0 if successful else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--native", action="store_true")
    parser.add_argument("--freertos-root", type=Path)
    args = parser.parse_args()
    return run(args.freertos_root, args.native)


if __name__ == "__main__":
    raise SystemExit(main())

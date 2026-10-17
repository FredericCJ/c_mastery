"""Execute the bounded original format-authority example; never change host policy.

Run on Linux (including WSL) from any directory. Only the tiny supplied teaching
target and withheld repair are compiled. Failed builds/runs remain failures, not
claimed demonstrations. Receipt source hashes bind this exact evidence scope.
"""
import argparse
import datetime
import hashlib
import json
import pathlib
import subprocess

ROOT = pathlib.Path(__file__).resolve().parents[2]
SOURCES = [
    "fixtures/legacy_depth/format_authority.c",
    "fixtures/legacy_depth/format_authority.json",
    "assessor/legacy_depth/security_format.c",
    "assessor/legacy_depth/security_format_test.c",
    "assessor/legacy_depth/verify_format_demo.py",
]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", default="reviews/v5/expansion/legacy_depth/format_demo.json")
    args = parser.parse_args()
    build = ROOT / "build/legacy_format_demo"
    build.mkdir(parents=True, exist_ok=True)
    commands = []

    def run(argv):
        completed = subprocess.run(argv, cwd=ROOT, text=True, capture_output=True, timeout=60)
        commands.append({"argv": [str(a) for a in argv], "exit_code": completed.returncode,
                         "stdout": completed.stdout, "stderr": completed.stderr})
        return completed.returncode == 0

    passed = True
    profiles = [("gcc-o2", "gcc", ["-O2"]), ("clang-o2", "clang", ["-O2"]),
                ("gcc-asan-ubsan", "gcc", ["-O1", "-g", "-fsanitize=address,undefined",
                                           "-fno-omit-frame-pointer"])]
    for name, compiler, flags in profiles:
        passed = run([compiler, "--version"]) and passed
        for variant, source, flaw in [("flawed", SOURCES[0], 1), ("repaired", SOURCES[2], 0)]:
            exe = build / (name + "-" + variant)
            built = run([compiler, "-std=c17", "-Wall", "-Wextra", "-Werror", *flags,
                         "-DEXPECT_FLAW=" + str(flaw), str(ROOT / source),
                         str(ROOT / SOURCES[3]), "-o", str(exe)])
            executed = run([str(exe)]) if built else False
            passed = built and executed and passed
    report = {
        "schema_version": 1, "status": "passed" if passed else "failed",
        "observed_at_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "scope": "Six original-target runs across GCC/Clang optimized and GCC ASan/UBSan profiles; no learner assessment or other vulnerability claim.",
        "source_sha256": {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in SOURCES},
        "commands": commands,
        "limitations": ["The pointer argument is deliberately valid and typed; this is a formatting-authority integrity violation, not arbitrary-address write.",
                        "No hardening setting was changed. Other libraries, targets and writable-format policies were not tested.",
                        "Output truncation checks apply to the repaired literal renderer; arbitrary input formats were not executed."],
    }
    output = ROOT / args.output
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"status": report["status"], "commands": len(commands), "receipt": str(output)}))
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())

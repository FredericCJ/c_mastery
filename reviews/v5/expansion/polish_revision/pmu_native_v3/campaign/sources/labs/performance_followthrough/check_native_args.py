#!/usr/bin/env python3
"""Exit-status tests for the native collector's command line (pmu_native.c).

Every numeric argument must be ASCII decimal digits inside its documented range
(labs/support/cm_parse.h). Anything else is an invalid request: exit status 2,
never a smaller, valid-looking request. The tests use the `check` mode, which
validates a request and the workload oracle without opening any perf event, so
they need no PMU access; invalid requests are also sent to `count` and
`sample`, which must reject them before touching hardware.

    python3 check_native_args.py --binary PATH

Prints PF_ARGS_PASS:<cases> and exits 0, or prints PF_ARGS_ASSERT lines and
exits 1. Supplied course check; it reads no file outside this directory.
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys

EXIT_OK = 0
EXIT_INVALID = 2
EXIT_UNAVAILABLE = 3

# Text that must never become a number: empty, signs, whitespace, trailing
# text, a base prefix, a value that wraps a 32-bit unsigned to 1, and one that
# overflows uintmax_t.
MALFORMED = ["", "+1", "-1", " 1", "1 ", "1x", "0x10", "1.0",
             "4294967297", "18446744073709551616"]

# (name, accepted minimum, accepted maximum, extra validity rule)
POSITIONAL = {
    "N": (1, 4194304),
    "VARIANT": (0, 1),
    "REPETITIONS": (1, 1024),
    "PRECISION": (0, 3),
}
OPTIONS = {
    "--pages": (1, 256),
    "--period": (1000, 4294967295),
    "--retain": (0, 1048576),
}
DEFAULT = {"family": "numeric", "N": "16", "VARIANT": "0", "REPETITIONS": "4",
           "PRECISION": "0"}


class Runner:
    def __init__(self, binary):
        self.binary = binary
        self.cases = 0
        self.failures = []

    def call(self, argv):
        result = subprocess.run([self.binary, *argv], capture_output=True, text=True, timeout=120)
        return result.returncode, result.stdout, result.stderr

    def expect(self, argv, status, label, check_output=None):
        self.cases += 1
        code, stdout, stderr = self.call(argv)
        problem = None
        if code != status:
            problem = f"exit {code}, expected {status}"
        elif check_output is not None:
            problem = check_output(stdout)
        if problem:
            self.failures.append(f"{label}: {argv!r}: {problem}: {stderr.strip()[-200:]}")


def positional(mode, **override):
    values = dict(DEFAULT, **override)
    argv = [mode, values["family"], values["N"], values["VARIANT"], values["REPETITIONS"]]
    if mode in ("sample", "check"):
        argv.append(values["PRECISION"])
    return argv


def echoed(field, expected):
    """The accepted value must reach the request unchanged (no narrowing)."""
    def check(stdout):
        try:
            row = json.loads(stdout.splitlines()[-1])
        except (ValueError, IndexError):
            return "no JSON check line"
        if row.get(field) != expected:
            return f"{field} echoed as {row.get(field)!r}, expected {expected!r}"
        return None
    return check


def test_positional(runner):
    fields = {"N": "n", "VARIANT": "variant", "REPETITIONS": "repetitions",
              "PRECISION": "precise_ip"}
    for name, (low, high) in POSITIONAL.items():
        modes = ["check", "sample"] if name == "PRECISION" else ["check", "count", "sample"]
        for mode in modes:
            for text in MALFORMED + [str(high + 1)] + ([str(low - 1)] if low > 0 else []):
                runner.expect(positional(mode, **{name: text}), EXIT_INVALID, f"{name} rejected")
        overrides = {name: str(high)}
        if name == "N":
            overrides["REPETITIONS"] = "1"
        runner.expect(positional("check", **overrides), EXIT_OK, f"{name} maximum accepted",
                      echoed(fields[name], high))
        runner.expect(positional("check", **{name: str(low)}), EXIT_OK, f"{name} minimum accepted",
                      echoed(fields[name], low))


def test_family_bounds(runner):
    # A family with a smaller bound rejects its maximum + 1 instead of clamping.
    runner.expect(positional("check", family="allocation", N="65536", REPETITIONS="1"), EXIT_OK,
                  "allocation maximum accepted", echoed("n", 65536))
    runner.expect(positional("check", family="allocation", N="65537"), EXIT_INVALID,
                  "allocation maximum + 1 rejected")
    for family in ("sort", "service"):
        code, stdout, _ = runner.call(["families"])
        rows = [json.loads(line) for line in stdout.splitlines() if line.startswith("{")]
        available = {r["family"]: r["available"] for r in rows}.get(family)
        runner.expect(positional("check", family=family, N="1025"), EXIT_INVALID,
                      f"{family} maximum + 1 rejected")
        expected = EXIT_OK if available else EXIT_UNAVAILABLE
        runner.expect(positional("check", family=family, N="1024", REPETITIONS="1"), expected,
                      f"{family} maximum {'accepted' if available else 'reported unavailable'}")


def test_options(runner):
    for name, (low, high) in OPTIONS.items():
        for mode in ("check", "sample"):
            for text in MALFORMED + [str(high + 1)] + ([str(low - 1)] if low > 0 else []):
                runner.expect(positional(mode) + [f"{name}={text}"], EXIT_INVALID, f"{name} rejected")
        field = name[2:].replace("pages", "data_pages").replace("period", "sample_period")
        runner.expect(positional("check") + [f"{name}={high}"], EXIT_OK, f"{name} maximum accepted",
                      echoed(field, high))
        runner.expect(positional("check") + [f"{name}={low}"], EXIT_OK, f"{name} minimum accepted",
                      echoed(field, low))
    # The ring needs a power-of-two number of data pages: 3 is not rounded.
    runner.expect(positional("check") + ["--pages=3"], EXIT_INVALID, "pages power of two")
    for drain in ("final", "concurrent"):
        runner.expect(positional("check") + [f"--drain={drain}"], EXIT_OK, "drain accepted",
                      echoed("drain_mode", drain))
    for bad in ("--drain=", "--drain=both", "--drain", "--pages", "--unknown=1", "-pages=1"):
        runner.expect(positional("check") + [bad], EXIT_INVALID, "malformed option rejected")
    runner.expect(positional("check") + ["--pages=1", "--pages=1"], EXIT_INVALID,
                  "duplicate option rejected")
    # Sampling options belong to sampling requests only.
    runner.expect(positional("count") + ["--pages=1"], EXIT_INVALID, "count rejects sample option")


def test_shape(runner):
    for argv in ([], ["count"], ["count", "numeric", "16"], ["measure", "numeric", "16", "0"],
                 ["count", "bogus", "16", "0"], ["count", "numeric", "16", "0", "4", "0"],
                 ["sample", "numeric", "16", "0", "4", "0", "extra"], ["families", "x"]):
        runner.expect(argv, EXIT_INVALID, "malformed request rejected")
    runner.expect(["families"], EXIT_OK, "families listed")


def test_control_descriptors(runner):
    """`work` takes perf stat's descriptors from the environment; they obey
    the same grammar (0..INT_MAX), so a signed or padded number never names a
    descriptor (EINVAL) and nothing is written to it."""
    for text in ("", "+1", " 1", "1x", "-1", "2147483648", "4294967297"):
        for name in ("PF_PERF_CONTROL_FD", "PF_PERF_ACK_FD"):
            env = dict(os.environ, PF_PERF_CONTROL_FD="1", PF_PERF_ACK_FD="0")
            env[name] = text
            runner.cases += 1
            result = subprocess.run([runner.binary, "work", "numeric", "16", "0", "1"], env=env,
                                    stdin=subprocess.DEVNULL, capture_output=True, text=True, timeout=60)
            if name == "PF_PERF_CONTROL_FD" and result.stdout:
                runner.failures.append(f"{name}={text!r}: wrote to a descriptor: {result.stdout[:80]!r}")
            elif result.returncode != EXIT_INVALID or "Invalid argument" not in result.stderr:
                runner.failures.append(f"{name}={text!r}: exit {result.returncode}: {result.stderr.strip()[-120:]}")


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--binary", required=True)
    args = parser.parse_args()
    runner = Runner(args.binary)
    for test in (test_shape, test_positional, test_family_bounds, test_options, test_control_descriptors):
        test(runner)
    for failure in runner.failures:
        print("PF_ARGS_ASSERT:" + failure, file=sys.stderr)
    if runner.failures:
        print(f"PF_ARGS_FAIL:{len(runner.failures)} of {runner.cases}")
        return 1
    print(f"PF_ARGS_PASS:{runner.cases}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

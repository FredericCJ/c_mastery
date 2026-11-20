"""Materialize the five private, single-defect alternatives in a fresh directory."""
import argparse
from pathlib import Path
import shutil


HERE = Path(__file__).resolve().parent
# Each alternative preserves a compiling C program and exits through its
# regular observation path. The guest fixture and all observations stay active.
MUTATIONS = {
    "namespace": (
        'namespace_inode("mnt"), marker',
        'namespace_inode("pid"), marker',
    ),
    "pids": (
        "pid_t next = held_worker(release, deadline);",
        """/* Release and reap the first worker before attempting the probe. */
    if (kill(first, SIGKILL)) fail_sys("release before probe");
    (void)collect(first, deadline);
    first = -1;
    pid_t next = held_worker(release, deadline);""",
    ),
    "shutdown": (
        "if (waited != child) {\n        status = collect",
        'if (waited != child && strcmp(variant, "early")) {\n        status = collect',
    ),
    "network": (
        "write_full(connection, variant, 1, deadline);",
        'write_full(connection, "A", 1, deadline);',
    ),
    "telemetry": (
        "pause_reader = stalled;",
        "pause_reader = 0;",
    ),
}


FAILURE_PREFIX = {
    "namespace": "mount_ns: expected ",
    "pids": "fork_errno: expected ",
    "shutdown": "exit: expected ",
    "network": "marker: expected ",
    "telemetry": "Inconclusive observation: the discriminating CPU or pipe signal",
}


def transcript_mutations(serial):
    """Derive all altered records from one successful live run."""
    rows = serial.splitlines()

    def index(prefix):
        matches = [i for i, row in enumerate(rows) if row.startswith(prefix)]
        if len(matches) != 1:
            raise ValueError(f"expected a unique mutation source row for {prefix}")
        return matches[0]

    def change(prefix, key, value):
        i = index(prefix)
        updated = rows.copy()
        tokens = rows[i].split()
        old = [j for j, token in enumerate(tokens) if token.startswith(key + "=")]
        if len(old) != 1:
            raise ValueError(f"missing field {key}")
        tokens[old[0]] = f"{key}={value}"
        updated[i] = " ".join(tokens)
        return "\n".join(updated)

    baseline = "LS telemetry variant=baseline "
    measured = dict(token.split("=", 1) for token in rows[index(baseline)].split()[2:])
    misplaced = rows.copy()
    barrier = misplaced.pop(index("LS_NET_OVERLAP "))
    misplaced.insert(misplaced.index("LS_BOOT_END status=0"), barrier)
    return {
        "barrier_after_replies": ("network", "\n".join(misplaced), "Unordered network/"),
        "different_kernel": ("namespace", change("LS_BOOT ", "kernel", "unrecognized-guest"),
                             "Incomplete or unexpected guest boot"),
        "instant_escalation": ("shutdown", change("LS shutdown variant=stubborn ", "grace_ms", 0),
                               "Missing bounded TERM-to-KILL grace"),
        "one_byte_pipe": ("telemetry", change(baseline, "pipe_size", 1), "pipe_size: expected "),
        "one_usec_workload": ("telemetry", change(baseline, "usage_after", int(measured["usage_before"]) + 1),
                              "CPU usage outside the observed leaf interval"),
    }


def materialize(output):
    output.mkdir(parents=True, exist_ok=False)
    for case, (old, new) in MUTATIONS.items():
        source = (HERE / f"{case}.c").read_text()
        if source.count(old) != 1:
            raise ValueError(f"mutation site in {case}.c is stale")
        if case == "pids":
            source = source.replace("#include <errno.h>", "#include <errno.h>\n#include <signal.h>")
        (output / f"{case}.c").write_text(source.replace(old, new))
    for name in ("support.c", "support.h"):
        shutil.copyfile(HERE / name, output / name)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    materialize(parser.parse_args().output)

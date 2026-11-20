"""Reconcile supervisor measurements with independently sampled guest fixture fields."""
import errno
import signal

from guest import BOOT_IDENTITY, CASES as VARIANTS


def require(condition, message):
    if not condition:
        raise ValueError(message)


def read_record(line):
    fields = {}
    for token in line.split():
        key, separator, value = token.partition("=")
        require(separator and key and value and key not in fields, "Malformed or repeated trace field: " + token)
        fields[key] = value
    return fields


def expect(row, **fields):
    for key, value in fields.items():
        require(row.get(key) == str(value), f"{key}: expected {value}, observed {row.get(key)}")


def number(row, key):
    try:
        return int(row[key], 10)
    except (KeyError, TypeError, ValueError) as exc:
        raise ValueError("Missing or malformed integer: " + key) from exc


def unique(records, event, **selectors):
    matches = [row for name, row in records if name == event and
               all(row.get(k) == str(v) for k, v in selectors.items())]
    require(len(matches) == 1, f"Expected one {event} {selectors}, observed {len(matches)}")
    return matches[0]


def witnesses(records, case, variant):
    key = {"name": case, "variant": variant}
    before = unique(records, "LS_OUTER", **key)
    inside = unique(records, "LS_INNER_FIX", **key)
    after = unique(records, "LS_CLEAN", **key)
    expect(unique(records, "LS_CASE_END", **key), status=0)
    group = "/lab/" + case + "-" + variant
    expect(before, pids_current=1, child_group=group, observer_group="/", marker="outer")
    expect(inside, pid=1, uid=1000, gid=1000, marker="inner", group=group)
    child = number(before, "child_pid")
    require(child > 1 and child != number(before, "observer_pid"), "Outer child/observer PID identity")
    for key in ("pid_ns", "mnt_ns", "net_ns"):
        expect(inside, **{key: number(before, "child_"+key)})
        require(number(inside, key) > 0, "Invalid namespace inode")
        if key != "net_ns" or case in {"network", "telemetry"}:
            require(number(inside, key) != number(before, key), "Shared namespace: " + key)
    expect(after, child_pid=child, pids_current=0, empty_procs=1, removed=1, marker="outer",
           observer_group="/", child_group=group, pids_max=before["pids_max"],
           cpu_quota=before["cpu_quota"], cpu_period=number(before, "cpu_period"))
    return before, inside, after


def process_view(row, before, inside, after):
    expect(row, inner_pid=1, proc_pid=1, report_ppid=1, marker=inside["marker"],
           pid_ns=inside["pid_ns"], mount_ns=inside["mnt_ns"], cgroup=before["child_group"],
           pids_max=before["pids_max"], report_pid=number(row, "worker_pid"))
    require(number(row, "worker_pid") > 1, "Worker PID must differ from namespace init")
    return {"outer_child_pid": number(before, "child_pid"), "inner_pid": 1,
            "mount_ns": inside["mnt_ns"], "pids_max": number(after, "pids_max")}


def task_limit(row, before, inside, after):
    refused = row["variant"] == "two"
    limit = 2 if refused else 3
    expect(before, pids_max=limit)
    expect(row, limit=limit, before=1, occupied=2, peak=limit, remaining=1, reaped=limit-1,
           fork_errno=errno.EAGAIN if refused else 0, cgroup=inside["group"])
    pid = number(row, "attempted_pid")
    require(pid == -1 if refused else pid > 1, "Fork result disagrees with admission")
    hits = number(row, "event_after") - number(row, "event_before")
    require(number(row, "event_before") >= 0 and hits == int(refused), "Task-limit event delta")
    expect(after, pids_hits=number(row, "event_after"))
    return {"admitted": limit-1, "limit_event_delta": hits}


def shutdown(row, before, inside, after):
    variant = row["variant"]
    # Readiness, forwarded TERM, escalation, wait disposition and startup errno
    # describe different transitions. Only startfail has a second wait obligation.
    expected = {
        "normal": (1, 1, 0, 0, 0, 0), "early": (0, 0, 0, 23, 0, 0),
        "startfail": (0, 0, 0, 127, 0, errno.ENOENT),
        "stubborn": (1, 1, 1, -1, int(signal.SIGKILL), 0),
    }
    ready, term, escalation, status, sig, startup = expected[variant]
    peer = int(variant == "startfail")
    expect(row, ready=ready, term=term, escalated=escalation, exit=status, signal=sig,
           startup_errno=startup, eof=1, children_errno=errno.ECHILD, reaped=1+peer,
           fd_after=number(row, "fd_before"), peer_started=peer, peer_eof=peer, peer_exit=0 if peer else -1)
    require(number(row, "fd_before") >= 3 and 0 <= number(row, "elapsed_ms") <= 5000,
            "Descriptor baseline or shutdown deadline")
    grace = number(row, "grace_ms")
    if escalation:
        require(100 <= grace <= min(1000, number(row, "elapsed_ms")), "Missing bounded TERM-to-KILL grace")
    else:
        expect(row, grace_ms=0)
    return {"reaped": number(row, "reaped"), "exit": status, "signal": sig}


def loopback(row, before, inside, after):
    expect(row, net_ns=inside["net_ns"], port=39073, marker=ord(row["variant"]),
           duplicate_errno=errno.EADDRINUSE, reply=sum((0, 1, 2, 3, 128, 254, 255, 0)))
    return {"outer_pid": number(before, "child_pid"), "network_namespace": inside["net_ns"],
            "marker": number(row, "marker")}


def telemetry(row, before, inside, after, connection, finished):
    variant = row["variant"]
    quota = "20000" if variant == "quota" else "max"
    expect(before, cpu_quota=quota, cpu_period=100000)
    expect(connection, port=39073)
    client_port = number(connection, "client_port")
    server_inode = number(connection, "server_inode")
    require(0 < client_port <= 65535 and client_port != 39073 and server_inode > 0,
            "Missing established TCP peer")
    expect(row, quota=quota, period=100000, cgroup=inside["group"], frames=8192, bytes=65536,
           checksum=8355840, digest=3940692756, peer="127.0.0.1", port=39073, client_port=client_port,
           server_inode=server_inode, tcp_reply_bytes=32768, pipe_size=4096)
    start, end = number(row, "usage_before"), number(row, "usage_after")
    require(0 <= start <= number(connection, "usage_usec") < number(finished, "usage_usec")
            <= end <= number(after, "usage_usec"),
            "CPU usage outside the observed leaf interval")
    start, end = number(row, "throttle_before"), number(row, "throttle_after")
    first = number(connection, "nr_throttled")
    last = number(finished, "nr_throttled")
    require(0 <= start <= first <= last <= end <= number(after, "nr_throttled"),
            "Throttle counters outside leaf interval")
    throttle = end-start
    refused, resumed = number(row, "eagain"), number(row, "resumed")
    require(refused >= 0 and resumed == refused, "Downstream progress accounting")
    require(number(row, "elapsed_ms") >= 0, "Negative elapsed time")
    require((throttle > 0) == (variant == "quota") and (last > first) == (variant == "quota")
            and (refused > 0) == (variant == "stalled"),
            "Inconclusive observation: the discriminating CPU or pipe signal is absent or mixed")
    cause = "cpu-throttling" if throttle else "pipe-backpressure" if refused else "none-observed"
    return {"cause": cause, "throttled_periods": throttle, "write_refusals": refused,
            "successful_resumes": resumed, "frames": number(row, "frames")}


CHECKS = {"namespace": process_view, "pids": task_limit, "shutdown": shutdown, "network": loopback}


def verify(case, serial):
    """Offline evidence from one selected case, also usable on multi-case author captures."""
    require(case in VARIANTS, "Unknown supervision case")
    lines = [line.strip() for line in serial.splitlines() if line.startswith("LS")]
    require(lines and lines[0] == BOOT_IDENTITY and lines[-1] == "LS_BOOT_END status=0",
            "Incomplete or unexpected guest boot or shutdown")
    require(sum(line.startswith("LS_BOOT ") for line in lines) == 1 and
            sum(line.startswith("LS_BOOT_END ") for line in lines) == 1, "Concatenated guest runs")
    records = []
    for line in lines[1:-1]:
        if line.startswith("LS "):
            line = line[3:]
        event, _, payload = line.partition(" ")
        records.append((event, read_record(payload)))
    require(all(name != "LS_CONTROL_FAIL" for name, _ in records), "Guest control failed")
    variants = VARIANTS[case]
    event = "view" if case == "namespace" else case
    require([row.get("variant") for name, row in records if name == event] == list(variants)
            or (case == "network" and sorted(row.get("variant", "") for name, row in records if name==event) == sorted(variants)),
            "Missing, duplicate or unexpected learner observations")
    facts = {}
    overlap = unique(records, "LS_NET_OVERLAP") if case == "network" else None
    for name in ("LS_INNER_FIX", "LS_OUTER", "LS_CLEAN", "LS_CASE_END"):
        observed = [row.get("variant", "") for event_name, row in records
                    if event_name == name and row.get("name") == case]
        require(sorted(observed) == sorted(variants), f"Unexpected or missing {case} {name} witness")
    for variant in variants:
        before, inside, after = witnesses(records, case, variant)
        row = unique(records, event, variant=variant)
        sequence = [("LS_INNER_FIX", inside), ("LS_OUTER", before)]
        if overlap is not None:
            sequence.append(("LS_NET_OVERLAP", overlap))
        if case == "telemetry":
            connection = unique(records, "LS_TCP", name=case, variant=variant)
            finished = unique(records, "LS_CPU_END", name=case, variant=variant)
            sequence.extend([("LS_TCP", connection), ("LS_CPU_END", finished)])
            facts[variant] = telemetry(row, before, inside, after, connection, finished)
        else:
            facts[variant] = CHECKS[case](row, before, inside, after)
        sequence.extend([(event, row), ("LS_CLEAN", after),
                         ("LS_CASE_END", unique(records, "LS_CASE_END", name=case, variant=variant))])
        positions = [records.index(item) for item in sequence]
        require(positions == sorted(positions), f"Unordered {case}/{variant} observations")
    if case == "network":
        expect(overlap, ready=2, port=39073, a_pid=facts["A"]["outer_pid"], b_pid=facts["B"]["outer_pid"])
        require(facts["A"]["network_namespace"] != facts["B"]["network_namespace"], "Shared network namespace")
        sockets = [number(overlap, k) for k in ("a_socket","b_socket")]
        require(min(sockets) > 0 and sockets[0] != sockets[1], "Missing distinct live listeners")
    return facts

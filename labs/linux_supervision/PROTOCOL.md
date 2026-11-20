# Linux supervision guest protocol

The guest calls `/supervise CASE VARIANT` once per cell. Each invocation runs
as UID/GID 1000 and PID 1 in its own PID and mount namespaces, with a procfs
mounted for that PID namespace. Read cgroup membership from
`/proc/self/cgroup`. In this unified hierarchy the third field gives the
leaf path under `/sys/fs/cgroup`. The outer fixture owns the controllers,
the namespace setup and the independent observations. The network and telemetry
exercises each receive a private network namespace with loopback up.

Write one line to stdout in the form `LS EVENT variant=VARIANT key=value ...`.
Use decimal integers and single-token strings. Indicate setup, I/O and
deadline failures on stderr and return nonzero. The fixture records its own
observations on serial. Join the supervisor's fields to the outer observer's PID, membership, mount
and socket observations.

## PID and mount views

For `namespace view`, use event `view`. A child sends its own PID and its
parent's PID through a pipe. Use `worker_pid` for the parent's fork result,
`report_pid` and `report_ppid` for the received values, `inner_pid` for the
supervisor's PID and `proc_pid` for the target of the `/proc/self` link.
Collect the worker. Read the inode numbers of `/proc/self/ns/pid` and
`/proc/self/ns/mnt` into `pid_ns` and `mount_ns`. Read the first line of
`/run/supervision-marker` into `marker`. Record `cgroup` and the leaf's
`pids_max`. The outer fixture reads the same marker path and inspects the
child's namespaces from its own procfs. Identify which two identities denote
the same process in different views.

## Admission under a task limit

Run `pids two` and `pids three`. Use event `pids` and retain the same live
process tree up to the probe. Record `cgroup`, the controller's actual
`limit`, and the following observations.

| Field | Sampling point |
| --- | --- |
| `before`, `occupied` | `pids.current` with init alone, then with the first worker held live |
| `event_before`, `event_after` | The `max` counter in `pids.events` on either side of the next fork |
| `attempted_pid`, `fork_errno` | Fork result in the parent and the errno saved immediately on failure, or zero on success |
| `peak` | `pids.current` while every admitted worker is still held |
| `remaining`, `reaped` | Population after releasing and collecting workers, and the number actually collected |

A held reader and an outside observer consume different task budgets. Mark
all open pipe ends in your process diagram before permitting either worker
to run to completion.

## Shutdown and startup failure

Use event `shutdown`. The `normal` worker acknowledges readiness and requests
shutdown by sending TERM to the supervisor. PID 1 establishes a TERM
disposition and handles the request, forwards TERM and reaps a clean exit.
The `stubborn` worker makes the same request and ignores the forwarded TERM.
Use a bounded grace period and escalate. The outer fixture timeout protects
the guest and a timeout there fails the case.

An `early` worker exits with status 23 before reporting readiness. In
`startfail`, establish a separate live worker and wait for its readiness
before a second child attempts an exec of a missing image. Report the actual
launch errno to the parent before exiting with status 127. Release the first
worker by EOF, observe its acknowledgment, and collect both statuses.

| Fields | Meaning |
| --- | --- |
| `ready`, `startup_errno` | Primary worker's readiness and exec error |
| `term`, `escalated` | TERM forwarded and KILL sent, each 0 or 1 |
| `exit`, `signal` | Primary child's exit code, or -1 if signaled, and its terminating signal, or 0 |
| `eof`, `reaped`, `children_errno` | EOF on the primary status pipe, total collected, and errno from the final empty child-set probe |
| `fd_before`, `fd_after` | Count of owned descriptors before construction and after cleanup, excluding the enumeration descriptor |
| `peer_started`, `peer_eof`, `peer_exit` | Earlier worker acknowledged readiness, acknowledged EOF, and exited with the recorded status. Use 0, 0, -1 when absent |
| `grace_ms`, `elapsed_ms` | Monotonic TERM-to-KILL interval in the stubborn trial (100–1000 ms), zero for other trials, and total elapsed milliseconds |

## Concurrent loopback listeners

Run `network A` and `network B` as a pair. Use event `network`. Each program
listens on TCP `127.0.0.1:39073`. Write the one-byte `R` to inherited
`SUPERVISION_BARRIER_FD` after bind and listen, then wait for the one-byte `G`
before exchanging traffic. The fixture releases both after observing two
live listeners. In each namespace try a second bind to the identical
address while the first listener remains live, using a fresh socket with
no reuse options.

Each server first sends its one-octet greeting, ASCII A or B. The client
then sends the existing T24 request `00 01 02 03 80 fe ff 00`. The server
returns the unsigned sum of those eight octets as four big-endian octets.
Record `net_ns`, `port`, the greeting actually received as decimal `marker`,
decoded `reply`, and `duplicate_errno`. After a short stream read, advance the cursor and read the remaining octets.

## Elective telemetry service

Run `telemetry baseline`, `quota` and `stalled` on identical data. Use event
`telemetry`. Frame numbers are 0 through 8191. Octet k of frame i is the
low eight bits of i+k, for k from 0 to 7. The client sends the request stream to the worker on TCP
`127.0.0.1:39073`. The worker returns the four-byte big-endian reply for
every frame on that TCP connection. The client forwards those replies to a
downstream reader through a nonblocking pipe. That reader verifies the full
sequence. For a finite and reproducible CPU load, the worker maintains an
unsigned 32-bit digest seeded with 2166136261. For each frame XOR in its sum,
then perform 4096 rounds of `d = (d ^ (d >> 13)) * 16777619`, modulo 2³².
Use the same work in every cell.

After accepting the connection, keep both endpoints live. Notify the fixture
by sampling `usage_before` and `throttle_before`, then sending `R` on fd 3.
Wait for `G` before the first request. At that barrier the fixture samples the
established server socket and both endpoint ports through the process's
network namespace, then reads CPU counters from the experiment leaf. After
all replies have been delivered and all children reaped, send `D` on fd 3.
The fixture samples the leaf again and sends `F`. Now read `usage_after`
and `throttle_after`. The two fixture samples bound the work interval
independently of the supervisor's own observations.

Use batches of 64 replies and request 4096 bytes of downstream pipe capacity.
Record the capacity the kernel grants. In baseline and quota, drain each
batch before requesting another. In stalled, pause the reader until an
actual nonblocking pipe write returns EAGAIN. Drain the outstanding replies,
resume the writer and verify the pending batch was delivered exactly once.
The quota cell changes `cpu.max` to `20000 100000`. Both other cells run with
an unlimited leaf quota. Read `usage_usec` and `nr_throttled` from the
leaf's `cpu.stat` around the full workload.

| Fields | Meaning |
| --- | --- |
| `cgroup`, `quota`, `period` | Membership and both space-separated fields of `cpu.max`, reported separately |
| `frames`, `bytes`, `checksum`, `digest` | Reply count, corresponding input octets, sum of decoded replies and final service digest |
| `usage_before`, `usage_after`, `throttle_before`, `throttle_after` | Raw CPU usage (µs) and number of throttled periods |
| `eagain`, `resumed` | Number of observed output refusals and count of refused batches subsequently accepted |
| `peer`, `port`, `client_port`, `server_inode`, `tcp_reply_bytes` | Peer IPv4 address, service and client ports, accepted socket inode, and payload octets actually received from TCP |
| `pipe_size`, `elapsed_ms` | Actual pipe capacity in bytes and monotonic elapsed time |

Explain a quota effect using the counters and a downstream stall using the
writer's refusal and subsequent progress. Timing depends on guest scheduling.

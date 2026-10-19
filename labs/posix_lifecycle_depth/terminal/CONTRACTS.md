# Owned terminals, full pipelines and parser transport boundaries

## Stable identifiers and exercise families

This required successor serves `AX-PR-T23-04-T23.11.a`, `.11.b`, `.11.c`,
`AX-PR-T24-06-T24.10.a` and `.10.b`, and original gap `LRUN-07`. Families
B/C/D/F/G/I/J assess construction, limits, diagnosis, falsification, recovery
and transfer. The original criterion text, delayed IDs, units and receipts stay
intact. T24.10.c's SocketCAN/frame adapter and physical two-node work remain
separate; this PTY exercise does not implement CAN.

## Sources and objectives

Read the exact registered Linux man-pages6.19 `termios(3)`, `pty(7)`, `pipe(7)`
and `shutdown(2)` passages, and the imported checkpoint parser contract. Derive
descriptor ownership, terminal state restoration, first-byte versus interbyte
waiting, byte-stream segmentation and connection-epoch boundaries. C17 governs
objects and arithmetic. POSIX/Linux, the terminal line discipline and selected
ABI govern calls and observations; they are not ISO C guarantees. `pipe2`,
`F_GETPIPE_SZ`, `/proc/PID/syscall` and `ptsname_r` are selected Linux facilities.

## Prerequisites and learner artifacts

Bring the completed exact-child launcher and descriptor maps from `PR-T23-01`,
the byte cursor from `PR-T23-02`, and the profile-C parser artifact from
`PR-P3-01`/`PR-P3-02` with `checkpoint_depth.h`. Review CRC framing, bounded
queues, terminal attributes, forked-process resource ownership and monotonic
deadlines. The parser is an explicit input: compile the learner's completed
parser once and link that same object into each transport experiment. Never
silently import an assessor implementation. Replace `terminal_stub.c` and
`stream_stub.c`; retain intentionally flawed controls in the assessor campaign.

## Environment and dependencies

Hosted C17 on Linux, libc/POSIX, owned UNIX98 PTYs, pipes, local socketpairs and
owned children. No controlling terminal is opened. GCC/Clang debug/release and
ASan/UBSan profiles apply to the hosted layer. The imported parser uses
`-ffreestanding -fno-builtin`; audit `nm -u` for implicit dependencies. The
reference campaign intentionally leaves that parser object uninstrumented for
the closure audit; hosted sanitizer results do not cover its internals. Parser
validation remains a prerequisite. `/proc` access to the exact owned child's
syscall establishes blocking-read enrollment; denied access is an environmental
failure, never permission to change host ptrace/security settings.

## Problem, supplied inputs and outputs

Implement `terminal.h`. Save a PTY slave's actual attributes, apply the named
raw/canonical policy, set symbolic B9600 and CS8/no parity/one stop-bit, read back
the result and restore the saved state. Exercise `A`, newline and NUL under
canonical/raw modes. Implement a producer/copier/sink pipeline that transfers
`4 * discovered_pipe_capacity + 17` bytes, each byte `offset % 251`; report
exact byte count, unsigned checksum, child IDs/statuses and reaped mask.
Add a sink that closes after at most512 bytes, pipe/fork acquisition failures
at each of three ordinals, zero input and a forced deadline. Discover capacity;
do not assume65536. The local workload bound is1MiB, not a universal pipe limit.

Implement `stream.h` around the existing parser. The supplied corpus includes
noise, wrong version, oversized length, bad checksum and three valid frames.
Two independently generated CRC vectors encode sequences0x1234/0x5678 with
payloads `41 00 0a` and `a5 ff`. Run identical input and all59 chunk bounds
through byte arrays, a raw PTY and an AF_UNIX socketpair. Deliver three exact
messages in order. Disconnect after eight bytes of the first frame, open a new
connection, supply its old suffix then the second frame: only the second frame
may be delivered. Also discard a queued prior-connection message on close.

## Mechanisms and misconceptions

Pipe EOF means all writer references closed, not merely that the producer
exited. Waiting for a producer before starting its consumer deadlocks once the
bounded pipe fills. A successful `tcsetattr` may apply only part of the request;
verify the named fields. A returned terminal timeout zero is not transport EOF
under every mode. VMIN>0/VTIME>0 starts its timer after the first byte; it cannot
bound a no-first-byte wait. A read is an arbitrary byte segment, not one frame.
A reconnect must not accidentally finish a previous peer's partial frame.

## Representation, invariants, failure and costs

`PTGuard` borrows a live, exclusively configured PTY descriptor; no other actor
may change its attributes. It owns a saved `termios` value and restoration duty,
not the descriptor. Compare named flags, every `c_cc`, and symbolic speeds,
never padding via `memcmp`. Set `restore_pending` before attempting an attribute
change, because failure can leave restoration necessary. A failed get before
the snapshot creates no duty. Return positive errno, keep a failed restoration
pending, and permit retry. An inactive restore is idempotent. Device loss can
make restoration impossible; report it and close the owned resource without
claiming that the old state was restored. No dynamic allocation is needed.

`pt_pipeline` accepts0..1MiB and a1..10000ms deadline. Every acquired descriptor
has exactly one owner in each process; close unused duplicates promptly. Signal
policy changes occur only in owned children. Require ordinary waitable SIGCHLD
policy and no concurrent external reaper for these children. On acquisition/deadline failure,
close descriptors, terminate only unreaped owned children and reap each exact
PID. The output preserves created IDs and observed statuses. Numeric checksum
fits uint32 under the workload bound. The result pipe carries a same-process
ABI structure; this is not a portable file/network representation. Work is
O(bytes), storage is bounded512-byte child buffers plus fixed ownership tables.
The deadline is a laboratory containment policy, not a real-time guarantee.

`PTSession` owns its parser/queue, saturating reject count and monotonically
increasing uint64 connection epoch. Its API is externally serialized. Open on
an already connected session or at epoch exhaustion fails unchanged. Close
discards partial and queued data by explicit service policy. Feed borrows live
input for one call; `used` counts exactly the consumed prefix. `CP_FULL` retains
the completed parser frame and unread input ownership; drain a message then
resume from `used`. A rejection consumes no imaginary suffix. Invalid calls
leave `used` unchanged. Use the imported parser operations; no alternate parser,
transport-specific wire format or hidden allocator. Work is bounded by the
fixed candidate size per input byte; queue capacity remains2.

## Deliverables

Submit both implementations, complete descriptor graphs at fork/close/reap,
terminal desired/observed/restored tables, all four VMIN/VTIME timelines,
first-byte enrollment evidence and exact pipeline content/status results.
Include a parser object hash/undefined-symbol audit, three-adapter message
traces, consumed-prefix/backpressure table and reconnect discard policy. Explain
why the checksum vectors are an independent oracle and why read chunk bounds
do not prescribe actual kernel segmentation.

## Verification and pass/fail

Run `guard`, `bytes`, `timers`, `pipeline`, `disconnect`. Correct implementations
exit0 with `PT_PASS`. Guard tests inject initial-get, set, partial-success,
post-set-get and restore errors and verify saved actual fields. Pipeline tests
check exact-child reaping independently with `waitpid`; merely observing a
zombie with WNOWAIT does not count. Timing tests use owned child syscall
enrollment, observe no-first-byte blocking, and report actual elapsed values.
They use broad laboratory bounds, not a claim of precise scheduler latency.
SIGPIPE default/EPIPE policy runs only in children; PTY disconnect records
EOF/EIO and restoration failure; socket half-close still permits the response.

For explicit parser reuse, run `python terminal/run_stream.py --parser-source
PATH --parser-include DIRECTORY --implementation PATH`. Both segmentation and
reconnect must exit0 with `PS_PASS`; the unsolved adapter must fail. The author
campaign compiles every control successfully and requires a named `PL_CHECK:`
assertion, not a crash/sanitizer/compiler failure. Preserve all commands,
environment, source/object/log hashes and failed attempts.

## Objective rubric

Level0 is absent/unsafe evidence. Level1 is a happy-path output without causal
ownership/timing reasoning. For level2, `.11.a` requires above-capacity transfer,
early-stage exit and exact reaping under all six acquisition faults; `.11.b`
requires restoration and four timing cases; `.11.c` separates SIGPIPE, EPIPE,
PTY disconnect and half-close. `.10.a` requires readback plus restored actual
fields and explicitly excludes electrical/baud proof. `.10.b` requires one
parser object, identical corruption/segmentation corpus and reset/queue policy.
Level3 additionally requires the original delayed unaided changed-condition
attempt with its argument; author reference execution cannot award that level.

## Graduated hints and assessor separation

First draw the writer-reference count before reasoning about EOF. Next separate
saved state, desired state and restoration duty. For timers, distinguish
"child says ready" from "child is blocked in read". For parser reuse, retain
the unconsumed suffix and test reconnect before a frame checksum is available.
The implementation and control explanations reside only in
`assessor/posix_lifecycle_depth/terminal/ANALYSIS.md`, outside learner exports.

## Changed conditions and integration

The original delayed variants are supplied as executable reference support:
third-stage early exit; all VMIN/VTIME combinations; local socket half-close;
no-first-byte versus interbyte waiting; and reconnect mid-frame. Change compiler
and optimization, input segmentation and actual pipe capacity. A physical UART
adapter additionally needs dedicated hardware, baud measurement, voltage/flow
control/error evidence and an explicit disconnect policy. SocketCAN and the
two-node bus/termination/bit-timing task remain required separate work.

## Security containment

Only original teaching processes and synthetic bytes are used. The local
attacker can choose malformed input, close its endpoint or reconnect; successful
demonstrations are a bounded hang caught by deadline, an unwanted cross-session
frame, or lost valid messages. No code execution/privilege escalation is claimed.
Repair closes redundant endpoints, reaps exact children, verifies terminal
state and resets parser ownership. Regression tests compare all messages and
ownership outcomes. These repairs do not authenticate peers, guarantee physical
serial integrity or prove every schedule. No external targets, user terminals,
deployed devices, real credentials or host configuration changes are allowed.

## Validation status and limits

Learner stubs remain unsolved. The source-bound campaign records actual author
execution separately from learner attempts, external review and physical
observations. No receipt alone proves mastery. PTY settings/readback establish
the selected software interface, never real baud, RS485 turnaround, wire levels,
CAN arbitration or real-device behavior.

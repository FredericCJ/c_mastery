# T24.07: notification is not ownership or completion

Use the inspected `epoll`, `epoll_ctl`, `epoll_wait` and prerequisite `poll`
passages in the source register. These OS interfaces describe readiness; the
application must still preserve partial operations and handle changed state.
The local examples below are original teaching programs, not production peers.

## T24.07.a — the readiness-to-read gap

Implement `struct rd_operation` and a nonblocking adapter with the exact sequence:
read-ready, read two octets, EAGAIN, read-ready, read one octet, EOF. The requested
extent is three octets `[7,0,9]`. Maintain the cursor at two across would-block,
publish completion only after the third octet, then record EOF separately.
EOF after only one or two octets must preserve partial progress with an explicit
terminal classification. A zero-length operation is rejected by this API.

Make read results drive state, not the earlier readiness indication. Show the
smallest counterexample to replacing a requested extent by the returned count
or converting EAGAIN into a terminal error. Predict every cursor, result and
buffer value before running. Check that a transmit operation never accidentally
uses the receive cursor or overwrites its unsent bytes.

For the delayed task, duplicate the owned read descriptor, observe readiness,
and let a second consumer drain the available two octets first. The original
read now gets EAGAIN. Retain the actual poll/read results and explain why the
notification did not reserve data for one consumer. This ordered interleaving
does not need a sleep or a nondeterministic race to demonstrate the mechanism.
The shared open-file-description lifetime must include both descriptors.

Hints: identify the fact established at the notification instant; next write the
cursor invariant `0 <= done <= goal`; finally separate transport end from
application-extent completion.

## T24.07.b — independent directions and retained operations

Represent RX and TX separately for one connection. Fill its owned nonblocking
outgoing socket buffer until EAGAIN within a fixed prefill cap. An unsuccessful
send leaves the TX cursor and bytes unchanged. Drain the prefill, alternate
one-octet receive and transmit progress, complete three RX octets and cancel TX
after two of its three octets. Verify exact received/sent prefixes and that the
unsent octet did not escape. Cancellation cannot retract the two already sent
octets or imply a remote rollback.

The changed-condition task replaces synchronous callbacks by queued completion
events. Derive identity before implementing the adapter: connection slot and
generation distinguish connection incarnations; direction and operation sequence
distinguish different retained operations in one incarnation. A stale completion
from sequence 1 must not advance a newly pending sequence 2 on the same connection.
Reject stale identity before accessing operation payload. A reused descriptor
number is insufficient identity.

Submit both RX and TX, request close, and show that a cancellation acknowledgment
does not release either target operation. Deliver their target completions in both
orders while a callback still borrows the connection. Reuse is forbidden until
all three obligations—RX, TX and callback borrow—have settled. Show a duplicate
completion, wrong direction, wrong connection generation and exhausted operation
sequence. The provided event adapter is serialized; implementing an OS completion
backend requires its distinct target/buffer-retention contract.

Hints: draw one box for each lifetime obligation; then inject an old completion
after a new submission; finally ask which event proves the last possible access.

## T24.07.c — bound work as well as successful I/O

Schedule an always-ready producer and a single-byte participant with two successful
operations per turn. Record service order under rotating start position. Contrast
with a deliberately bounded demonstration of unlimited draining which spends its
entire chosen work horizon on the producer; do not run an infinite hang as a test.
Retrying EAGAIN without changing readiness likewise consumes work without progress.

For the delayed task, callbacks may do CPU work while producing zero bytes.
A success-only budget no longer bounds the turn. Add a callback budget, count each
invocation even when success count does not advance, and refuse nested turns.
With three participants and a positive callback budget, rotation places each
continuously ready participant first within three turns, provided all callbacks
return and turns keep running. Derive this bound; single-threaded execution alone
does not imply fairness or bounded wall-clock latency.

Use the public deterministic callback traces, then compare generated schedules to
a separate ordered-list/quota oracle. Name the independent model assumptions:
ready mask, start position, callback budget and per-participant result type.
Include always-progress, CPU-only, would-block and terminal callbacks. A model
whose expected output calls `rd_turn` would be circular.

Hints: count invocations independently from bytes; try a callback returning zero
forever; finally make the next turn's starting position part of persistent state.

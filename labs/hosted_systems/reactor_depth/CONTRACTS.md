# PR-T24-04: readiness, lifetime and bounded shutdown

This required successor implements the original T24.07.a–c, T24.08.a–c and
T24.11.a–c criteria and their exact changed-condition tasks. The original IDs,
assessment bodies and seven-day delay remain in `ASSESSMENT.json`. Read
`READINESS.md`, `LIFETIME.md` and `SHUTDOWN.md`; implement the deliberately
incomplete `reactor.c` and `waiters.c` against the supplied headers. Public tests
are feedback; separate assessor implementations, native probes, oracle, mutation
drivers and answer keys are withheld.

Use hosted C17, profile A, Linux nonblocking pipes/local sockets and epoll, POSIX
mutexes and condition variables. Core state machines use fixed objects and C
operations, with no event-loop, queue, coroutine or cancellation library doing
the assigned work. `struct rd_service` must import the actual T24.05 `struct rf_outbox` API rather
than reimplementing a narrower queue. Supply your prerequisite `stream.c` from
PR-T24-03. Author runs use the named assessor stream implementation; that evidence
does not establish that an eventual learner provider meets the same contract.

Prerequisites: T18 ownership and failure contracts, T23 descriptor lifetime,
T24.03 absolute deadlines, T24.05 framing/queue ownership and the relevant T25
mutex/predicate knowledge for the blocking port. Complete the pure state machines
and synchronous native loop before the completion-event and threaded variants.
Bring descriptor and message ledgers, byte encoders and prediction traces.
The queue/provider dependency must build before its service consumer.

All core operations are serialized by one event-loop owner. All object pointers
must refer to initialized, live, aligned, appropriately sized and non-overlapping
storage. The registry and its metadata remain alive while any cached token could
arrive. A valid generation is not a general validator for arbitrary addresses.
The caller may not mutate an owned message, reinitialize a pending operation, or
retain a peeked range past advance/disconnect. C enforces neither these ownership
rules nor callback termination. Runtime checks reject specified invalid tokens,
bounds and transitions; tests and external reasoning cover additional obligations.

`struct rd_operation` owns up to eight bytes and separate goal/done/terminal fields.
Receive and transmit states are independent. Positive progress records actual
committed bytes; -2 means would-block, -1 terminal error, zero EOF. A terminal
operation never becomes active without explicit fresh initialization. Transport
EOF after the requested extent has completed is a separate channel observation.

`struct rd_registry` has four stable slots. Event identity is `(slot, connection generation)`;
completion identity additionally includes direction and a nonreused operation
sequence. Connection generation and operation sequence have declared finite
limits. Exhaustion retires the relevant reuse path; it never silently wraps.
Closing forbids new borrows/submissions and delays reuse until all callback
borrowers and submitted target operations settle. A cancellation acknowledgment
alone does not settle a target operation. The completion adapter here is an
executable serialized event protocol; the kernel-retained I/O backend belongs to
T24.12 and must be validated independently.

`struct rd_scheduler` has three participants. Each turn snapshots ready work, permits at
most two successful operations per participant, at most eight successes globally,
and a separately selected callback budget of 1–8. CPU-only work consumes a
callback. Ready publication during a callback joins the next turn; nested turns
are rejected. Fairness arguments require finite callbacks and repeated turns.

`struct rd_service` imports four connections and eight frames per connection, a global
payload limit 0–256, and a separate 64-entry lifetime terminal ledger. An epoch
accepts at most 64 distinct IDs, including previously completed ones; exhaustion
rejects a caller-owned message. Derive memory as the sum of queue/message objects,
ledger, payload/wire representation, stack and kernel buffers, not payload alone.
Admission runs outside callbacks. Callbacks may request stop reentrantly; a running
service rejects admission while borrowers exist so disconnect policy cannot release
another callback's message. Stop rejects new work immediately, can only shorten
an existing absolute deadline, and retains pending message ownership until the
last borrower returns. Caller code closes its actual descriptors after release.

`struct rd_port` is a separate capacity-one producer/consumer adapter using a single
mutex and predicate conditions. It supports cooperative stop and peer-gone events,
not `pthread_cancel`. Condition variables use CLOCK_MONOTONIC. On reacquiring the
mutex, an observed stop, peer event or available transfer takes precedence over
a concurrent timeout return; the selected terminal result is latched. This is
an explicit arbitration policy, not a strict physical completion deadline.
Destroy the port only after joining every user; never destroy a live condition.

For every leaf submit: prediction before execution; implementation and adapter;
pre/postconditions, state/ownership invariants and progress argument; raw commands,
versions and source identities; positive and deliberately incorrect cases;
independent expected results; retained storage and work cost; and an unfamiliar
changed-condition analysis. A matched trace without the leaf's causal explanation
does not pass. Seven-day variants require a separate learner attempt.

All native tests use original owned pipes, sockets, processes and synthetic bytes.
No external peers, host security changes or deployed devices are involved. Stale
event demonstrations may expose an incorrect synthetic recipient/operation, never
access real data. The separate intentionally racy specimen is only a diagnostic
control under ThreadSanitizer; no portable C output is assigned to its undefined
behavior. Passing sanitizers or finite histories does not prove absolute mastery.

Public builds (substitute the learner's completed prerequisites):

```sh
cc -std=c17 -O2 -Wall -Wextra -Werror \
  -I labs/hosted_systems/reactor_depth -I labs/hosted_systems/stream_depth \
  labs/hosted_systems/reactor_depth/tests.c \
  labs/hosted_systems/reactor_depth/reactor.c \
  labs/hosted_systems/stream_depth/stream.c -o reactor-tests
cc -D_GNU_SOURCE -std=c17 -O2 -Wall -Wextra -Werror -pthread \
  -I labs/hosted_systems/reactor_depth \
  labs/hosted_systems/reactor_depth/waiters_tests.c \
  labs/hosted_systems/reactor_depth/waiters.c -o waiter-tests
```

No C23 feature is required. epoll is Linux-specific; pthread rules have separately
inspected POSIX Issue-8 authority. ISO C17 by itself supplies neither interface.
Exact published C17/C23 standard access is not newly claimed. Host Python oracles,
sanitizers and publishing tools are not dependencies of a freestanding target.

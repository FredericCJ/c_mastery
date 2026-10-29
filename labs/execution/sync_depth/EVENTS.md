# Event predicates, consuming order and finite occurrence sequences

This required successor supplies concrete implementation and experiment support
for all three original PR-T51-08 criteria and their exact delayed variants. Keep
the parent T51.08 teaching, project, fixture, rubric, prerequisites and fourteen
fields. Prerequisite artifacts are the learner's actual PR-T51-01 common wait
kernel and PR-T51-07 completion/ownership analysis. Import completed CMX explicitly;
the public `events.c` remains an unsolved compiling scaffold. `events.h`,
`events_tests.c`, `events_m4.c` and `run_events.py` are supplied learner artifacts;
the worked implementation and causal analysis remain assessor-only.

## Representation and ownership

Use C17 with `uint32_t`/`uint64_t`, the retained sixteen-task CMX kernel and caller
storage. `struct cd_events` owns an actual `cmx_event` and one result slot per logical task;
it borrows its live kernel. Each result slot is idle, pending or done. All access
is externally serialized, including CMX ticks, cancellation and task transitions.
Do not move an event/kernel or reap/reuse a participating task until its pending
operation is completed and collected. Raw CMX task slots are not generation-safe
handles. Initializing fresh storage requires no retained old waiters or result
references. Inputs/outputs/control objects are live, valid and disjoint; ordinary
clients cannot edit public state except the named valid boundary fixtures.

The mechanism uses actual `cmx_event_wait`, `cmx_event_set` and `cmx_event_close`.
Do not replace them with a host event library or implement a second scheduler.
The adapter makes terminal-result ownership explicit. There is no allocation or
target libc. Host compiler, Python and emulator orchestration are verification
dependencies, separate from audited target helpers. These serialized schedules
do not establish native thread or interrupt safety.

## Event operations and selected policy

`cde_wait` admits one operation for a callable task with a nonzero mask and Boolean
`all`/`consume` values. Invalid arguments, noncallable task or an uncollected result
reject INVALID without changing bits or creating a result. After valid admission,
even immediate CLOSED, TIMEOUT or FULL creates a result to collect. A successful
match observes `bits & mask`; any requires a nonempty intersection, all requires
every requested bit. Consuming success clears exactly those observed bits.
Nonconsuming success leaves the group unchanged. Already-matching state wins
before a deadline test, as in the retained CMX API.

`cde_result` returns BLOCKED while the actual CMX operation remains waiting.
After completion it returns exactly that kernel status and retires the result.
Only OK writes the observed-mask output. Error and pending results preserve the
output; a second collection rejects. Callers may not start another unrelated
operation on the same logical task before collecting this result. A tick or
cancel that selects a terminal result cannot be undone by a later set.

`cde_set` ORs the supplied bits, including a harmless zero set. It then uses the
retained FIFO *eligible-waiter* policy. The oldest currently eligible waiter
observes and possibly consumes; eligibility is recomputed after each wake.
An older unsatisfied all-waiter does not block a younger satisfied any-waiter.
This is sequential consumption, not snapshot broadcast. Task priority does not
replace event registration order. A nonconsuming observer placed before a
consumer can see the same bits; placed after that consumer it may remain waiting.

`cde_clear` clears only its mask and does not wake anyone. `cde_close` closes the
group and selects CLOSED for all current waiters. Completed successes remain
successful. Later sets/clears reject CLOSED. `cde_destroy` refuses while any
uncollected pending or completed result remains; after collection it detaches the
kernel. Destruction is not implicit cancellation. Stop-requested is a persistent,
nonconsumed bit included in every relevant wait-any predicate, or shutdown uses
the explicit close path. A consuming stop waiter can deprive another waiter of
that stop indication; do not silently use that policy for broadcast shutdown.

The retained enrollment sequence cannot wrap: an attempt that would enroll after
`kernel.sequence == UINT32_MAX` returns FULL. This is different from an immediate
matching predicate, which needs no new wait node. Deadlines retain CMX's explicit
half-range modular-time assumptions. No lifetime or finite-counter exhaustion
rule is relaxed by the adapter.

## Changed representation: occurrence sequence

Three identical sets performed before observation produce one level bit, whereas
three actual `cmx_notify` operations retain three counted notifications. For the
required sequence variant, `struct cde_sequence` stores `(epoch, serial)`, both uint32_t.
The logical number is `(uint64_t)epoch << 32 | serial`. Initial state is zero.
Each publication increments the serial; at UINT32_MAX it increments the epoch
and sets serial to zero. At the maximal pair it refuses FULL without mutation.
There is no terminal wrap, silent reuse or indefinite uniqueness claim. Named
near-limit fixtures are constructed directly rather than pretending billions
of publications ran.

Each observer owns a separate `struct cde_cursor` from the same sequence lifetime.
`cde_sequence_read` refuses a future cursor INVALID; equal cursor returns BLOCKED;
both preserve cursor, delta and missed outputs. For a changed sequence it returns
OK, writes the exact unsigned delta and `missed = delta - 1`, then advances that
observer's cursor. Missed means intervening publications not separately observed
by this cursor, not an assertion that a network packet or payload was lost. The
sequence retains multiplicity, not each event's payload or a processing receipt.
It is an observation API, not an additional blocking wait queue. A separately
composed notification may be a hint, but the cursor comparison remains the source
of truth. Reset requires retiring all cursors or another lifetime namespace;
this API does not accept stale cursors across reinitialization.

No C atomicity follows from a uint64_t representation, especially on Cortex-M4.
The common serialization premise covers publication and cursor reads. ISO C
enforces unsigned arithmetic and object representation rules, not exclusivity,
borrow lifetimes, correct registration, observer identity or progress. Compare
language-level events/channels without claiming they automatically count levels,
preserve every payload or authenticate consumers.

## Required experiments and independent expected results

D1 derives any/all truth tables, eligibility and consumption order, result-slot
invariants, storage lifetime, finite counters and shutdown. D2 implements the
actual CMX adapter and the sequence variant from the permitted primitives. D3
retains commands, raw traces, source/image hashes, output sentinels, independent
expectations and the exact first divergence of each faulty control. D4 completes
the original changed conditions: sequence/missed-event behavior, reverse
registration order, and bit31 in a full uint32_t mask. Author references do not
constitute the learner's delayed unaided assessment.

The immediate Boolean oracle enumerates all eight three-bit states, seven
nonzero masks, two any/all modes and two consumption modes: 224 cases. It derives
matches by inspecting individual named bits, independently of the implementation's
mask predicate. Failed immediate matches use an already-due deadline and preserve
the sentinel. Successful consuming waits preserve unrelated bits. Repeat with
`UINT32_C(1) << 31`, including a blocked high-bit wait, so signed shifts and
accidental 31-bit truncation have visible consequences.

The two-waiter oracle enumerates both masks1..7, four any/all combinations, four
consumption combinations, both registration orders and all eight set values:
12,544 histories. Both waiters start on a clear group. Independently calculate
eligibility and remaining bits in registration order; compare each actual result,
observed mask and group state. Cancel and collect every remaining waiter before
destruction. The matrix includes observer-before-consumer and consumer-before-
observer, all versus any, and consuming overlap. It does not enumerate arbitrary
numbers of tasks, native interleavings or all machine-width states.

For the original masks3(all)/1(any), set1 first: the any-waiter succeeds while the
all-waiter stays pending. Then set3 and collect the all-waiter. Separately enroll
both then set3: reversing enrollment changes which consuming waiter succeeds.
Explain that this difference is the API policy, not a scheduler defect.

Check zero masks, non-Boolean mode, unused task, and a duplicate begin without
altering bits. At deadline5 select timeout, then set the matching bit before
collection: collect TIMEOUT unchanged and retain the bit for a subsequent wait.
Use nonconsuming data-ready/stop predicates for two waiters; set STOP and collect
both. Compare close waking both data-only waiters, refusal to destroy before
their results are collected, and checked wait-sequence exhaustion.

For multiplicity, perform three bit4 sets, three actual counted notifications
and three sequence publications. Observe one consuming event match, three
notification takes and sequence delta3/missed2. Read again and preserve sentinels.
Cross `(7, UINT32_MAX-1)` through two publications to `(8,0)` and obtain delta2.
Reject a future cursor unchanged. At `(UINT32_MAX,UINT32_MAX)` refuse publication;
from cursor zero the exact remaining difference is UINT64_MAX.

## Assessment, controls and resource evidence

Level1 reproduces an ordinary event match. Level2 supports every named predicate,
ownership, order, terminal, finite-width and multiplicity claim with actual
artifacts and a counterexample to the stated misconception. Level3, after the
required delay, independently derives and implements the changed representation,
order and width assumptions. Passing a finite oracle is not absolute mastery or
proof of all possible executions. Hints: count distinct bits separately from
posts; recompute eligibility after each consuming wake; find the last owner of a
pending result; distinguish a low-word wrap from identity reuse; a cursor gap
does not store the missing payloads. Separate worked analysis remains private.

Eight semantic controls use any for all, erase unrelated bits, consume an
observer, reverse FIFO eligibility order, omit the wrap epoch, collapse a sequence
gap, truncate bit31, or close without waking waiters. Each must compile and fail
its intended assertion. The unsolved adapter and actual CMX producer stub must
also compile and be rejected. Compiler failures, process crashes, sanitizers and
harness timeouts are not successful negative evidence. Report six GCC/Clang
host profiles separately from actual selected Cortex-M4 emulator execution and
its explicit helper/symbol/link-map audit. No board execution is implied.

Storage is one CMX group, sixteen bounded result slots and the borrowed kernel;
each sequence/cursor is two uint32_t fields. Immediate predicate/clear and cursor
arithmetic have bounded constant work. The retained event setter repeatedly scans
the sixteen-task table to select eligible waiters, giving O(N squared) worst-case
selection work when bounds vary, not an asserted constant-time scheduler or WCET.
The supplied experiment tests logical outcomes, not interrupt latency. Record
actual ABI layouts and target stack/helper costs before making resource claims.

The only targets are these original local teaching programs and dedicated QEMU
images. Controlled ordering and finite-state inputs can demonstrate an omitted
shutdown wake, wrong consumer result or lost count. No external service, deployed
firmware or real data is involved. Runtime exclusions, trusted uncorrupted control
state and finite counter/lifetime limits remain explicit. New evidence is author
validation only; independent review, delayed learner performance and physical
observations remain separate, unmanufactured obligations.

# Counted notifications: occurrence ownership, overflow and execution contexts

## Identity, objectives and families

This required successor implements PR-T51-09 / T51.09.a–c and the three original
`AX-PR-T51-09-T51.09.{a,b,c}-DELAYED` variants. Families are construction,
boundary/diagnosis, failure recovery, verification, measurement and changed
conditions. Historical project fixtures and acceptance records remain unchanged.
The old clamp fixture and the retained CMX all-or-error implementation express
different contracts: compare both explicitly; do not silently equate them.

## Prerequisites and inspected references

Import a completed PR-T51-01 CMX common wait protocol and task lifecycle. Reuse the
actual `cmx_notify`, `cmx_notify_take`, timeout/cancel, task generation and scheduler
operations. The notification field must have a single adapter owner. Review
PR-T51-08 coalesced events and PR-T51-07 terminal result ownership before starting.
Actual bounded course and primary source inspections are registered in
`research/v5/expansion/legacy_concurrency_notifications/resources.json`: POSIX
Issue 8 mutex and memory synchronization, Linux affinity/current-CPU APIs,
CMSIS Cortex-M4 registers and QEMU 10.2.1 SysTick implementation. Existing original
criterion references remain applicable; no unseen external text is claimed read.

## Environment and dependencies

Core: ISO C17, the actual CMX representation, uint32/uint64 types and no allocation,
library synchronization, or hidden runtime. C11/C17 atomic operations are relevant
only in the hosted comparison: `volatile` does not create synchronization. C23
does not make a compound load/store increment atomic either. Target: Cortex-M4
soft-float ABI, original explicit memory helpers, owned QEMU `mps2-an386`, no
hardware flashing. Hosted variant: Linux pthread mutexes/affinity and a monotonic
clock, at least two effective allowed CPUs. These dependencies are not target
imports. Source, assembly, undefined symbols and linker map are separate evidence.

## Problem, supplied inputs and outputs

Complete `notifications.c`, `notifications_native.c` and `notifications_irq.c`
against the supplied public headers, tests and target harness. They are unsolved
learner files. Supply explicit completed imports to the public runners; assessor
implementations are not copied into a learner workspace. Inputs include post3,
post2,take4; stored7+post2 at maximum8; post3 to an empty blocked recipient;
UINT32_MAX; selected timeout/cancellation; stale task incarnation; and the exact
near-exhaustion ledger states in `notifications_tests.c`.

Expected observable results include remain1, all-or-error refusal of2 with stored7
unchanged, direct claim1 plus stored2, and unchanged result outputs on errors.
Also construct the clamp alternative for analysis:7+2 accepts1 and records the
other refused occurrence. Do not substitute that alternative into this API.
The independent integer oracle enumerates1024 five-operation histories/5120
transitions. Explain its finite operation alphabet and omissions.

## Representation, invariants and ownership

`CDNotify` borrows a live, stable CMX kernel and one task incarnation; `CDNHandle`
contains its owner and full task generation. All outputs are live, writable and
disjoint from control/kernel objects. `cdn_init` requires live nonnull pointers
and fresh exclusive storage. Initialization may not reset a live control to
evade stale-handle checks. Attach accepts READY/RUNNING with zero raw credits,
positive maximum, no existing attachment. After detach the same control can bind
only the same slot with a different task generation. Archive its ledger first.
The CMX generation never wraps: final generation cannot be reaped for reuse.

Outside a result's internal update, claimed is zero or one and:

`accepted = consumed + cancelled + stored + claimed`.

Stored is at most maximum; the reserved claim is separate. Accepted/consumed and
cancelled count admitted occurrences. Rejected counts explicitly reported refused
post attempts; retrying an occurrence may add multiple refused attempts. It is
not a unique source-event count. Invalid/stale/closed/fault-backpressured attempts
remain entirely with the caller and do not increment rejected. No payload is
carried. A count cannot reconstruct lost event data or authenticate a producer.

## Operations and failure semantics

Post is all-or-error. Test `count > maximum-stored` before mutation; no overflowing
addition. On FULL publish a single exact fault record(sequence,count,FULL), keep
accepted/stored unchanged and leave CDNPost unchanged. While that record is
unacknowledged, refuse new posts with CDN_FAULT_PENDING. Fault reading does not
acknowledge; only the matching sequence does. Caller must preserve, retry or
explicitly discard the refused occurrence and account for that decision.

Reserve accepted and rejected uint64 capacity and a nonterminal fault sequence
before accepting a post. CDN_RETIRED leaves state and output unchanged; this
finite lifetime deliberately refuses even a currently fitting success if a
future failure cannot be reported within the reserved counters. Cleanup remains
legal. It is not an unbounded exact-event guarantee.

Wait reserves one credit immediately or through the actual CMX wakeup. One
collectable result belongs to the adapter until `cdn_result`; reentering wait
before collection is invalid. A successful result atomically takes up to the
requested N at the **result collection boundary**, including posts admitted
before collection. It does not wait for all N. Request UINT32_MAX for consume-all
subject to that representable maximum; if a reserved claim plus stored exceeds
UINT32_MAX, one or more credits remain. BLOCKED and terminal errors preserve the
count output; a completed success wins over later cancellation. Selected timeout
before post leaves the new credits for a later operation. Close cancels a pending
wait and moves stored credits to cancelled; an already reserved successful claim
remains collectable. Detach requires closed, no outstanding result and fault ack.
Close/detach and producer quiescence must precede task exit/reap or owner release.

## Required construction and reasoning

Derive the conservation equation and argue every transition including overflow,
close-after-success and timeout-before-post. Prove subtraction bounds and that
consumed/cancelled cannot exceed accepted. State control/kernel lifetime and
nonreentrancy assumptions; direct raw notification or unrelated task operations
while a result is outstanding violate this exclusive adapter contract. A stale
handle check is not permission to dereference freed owner storage.

Core operations are constant in count and do not loop N times; a post has one
recipient lookup. CMX scheduler/tick costs are separate and may scan bounded task
arrays. On 32-bit targets, 64-bit accounting is not inherently atomic. Core calls
need external exclusion; one compiler optimization is not a synchronization
proof. Supply annotated assembly/helper closure and representation costs.

## Interrupt experiment and measurement

Implement saved/restored PRIMASK exclusion, preserving nested saved state. The
post wrapper allows thread/PendSV or an external IRQ with configured priority at
least0x80; reject NMI/fault/unsupported handlers and more urgent IRQs. This is the
course kernel-call policy, not an ISO C or universal Arm rule. Thread wait rejects
handler context and nonzero PRIMASK/BASEPRI/FAULTMASK. Post may pend PendSV when it
wakes a task; it must not call the scheduler from IRQ0.

Run the supplied real vector table: IRQ0 priority0x80, PendSV0xff, BASEPRI0xc0
temporarily holding PendSV. Four separate IRQs post3,2,3,4. Expect accepted8,
claimed1,stored7,refused4, one pending scheduling exception, then consumed8 after
release. Test an urgent priority0 IRQ refusal, nested mask restoration and stale
task generation. A single pending bit is not an event counter.

Collect four interrupt-entry intervals, four protected-post intervals and one
deferred scheduling interval from the24-bit downcounting SysTick register. Use
modular subtraction with the explicit less-than-half-period observation window;
the harness rejects intervals outside it. QEMU models virtual timer time, not a
physical instruction-cycle counter. Report raw values, resolution (including
zero if observed), tool versions and instrumentation. Entry includes pending-write,
exception entry and probes; post includes wrapper/probes; deferred includes
scheduler/probes. Do not add them into an unsupported WCET claim or infer board
interrupt latency. No physical timing has been measured.

## Cross-CPU changed condition

Implement the native wrapper using one initialized pthread mutex around the
entire CMX/core transition. Its address stays stable. No raw concurrent core call
is permitted. Join all callers before destroy; no concurrent destroy/rebind.
Hosted take uses a deadline already due for a nonblocking try. FULL copies the
fault to the caller and acknowledges under exclusion; retry is an explicit caller
choice. Unexpected clock/CPU-query/mutex errors can occur after mutation: treat
them as port-fatal/unknown effect, stop and inspect, do not blindly retry.

Pin two producers to one effective allowed CPU and the consumer to a distinct
allowed CPU, checking effective affinity and sampled CPU. Run4000 accepted
occurrences with capacity8 and consumer up-to7. Final accepted=consumed4000,
pending=cancelled0; total refused attempts must match explicit fault reports.
Missing two allowed CPUs is a failed environment requirement, not a passing skip.
Retain31 raw wait/protected-work samples per worker outside the lock. Lock-wait
includes sampling and mutex acquisition; protected work includes CPU query, core,
fault copying/ack and final sampling, excludes unlock. Contention and scheduler
effects are part of these observations; no starvation or real-time bound follows.

Provide a race-free counterexample in which two atomic loads both read0 and two
atomic stores write1. It must fail expected2 without relying on undefined data
races; compare atomic fetch_add yielding2. This is a compound-operation example,
not a weak-memory model checker or evidence that arbitrary lock-free code works.

## Verification and acceptance

Require actual completed CMX import in every positive core/native/target build.
Run ledger/handoff/lifetime/histories under GCC/Clang debug,optimized and ASan/UBSan.
Run native variants additionally under GCC/Clang TSan with real instrumentation.
Run GCC O0/O3 and Clang O3 M4 images; audit linked helpers/no undeclared archives.
Every reference must meet exact counts, outputs, task identity and mask/exception
assertions. Every semantic mutant and unsolved consumer/producer must compile
then fail the intended assertion, not crash, time out or merely trip a sanitizer.
Keep raw commands/logs/source hashes. Falsify clamp, dropped surplus, ignored
generation, Boolean collapse, consume-all-for-up-to, hidden overflow, mask loss,
IRQ scheduling and forbidden-context variants.

## Deliverables, rubric and hints

Deliver completed three implementations, transition/invariant derivations,
unchanged-output and fault ownership table, original fixture contract comparison,
history oracle explanation, source/assembly/map audit and raw measurement report.
For T51.09.a assess conservation, overflow/fault capacity and finite-counter
cleanup; for b assess one-shot handoff, context/identity, up-to-N snapshot and
partial outcomes; for c assess burst accounting, selected terminal order,
cost separation, actual CPU synchronization and atomic counterexample.
Score0: missing/unsafe;1: ordinary trace passes but assumptions/error ownership
missing;2: all relevant boundaries and executable evidence explained with their
limits;3: independently derive the changed policy and diagnose an unseen mutant
after the original minimum7-day delay. Author runs are not a learner's score3.

Hints, in order: (1) separate stored and reserved credit; (2) draw post/timeout/
collect orders before coding; (3) reserve arithmetic/fault capacity before state
publication; (4) count refused attempts separately from admitted occurrences;
(5) put native exclusion around the whole operation, not individual fields.
Separate assessor analysis and implementations are withheld under
`assessor/legacy_concurrency_sync/`.

## Security scope, variants and evidence limits

Targets are original course programs in local processes and disposable QEMU.
Synthetic counts/IDs only; no external service, host security-setting change or
physical board operation. A stale-incarnation or lost-count demonstration is a
local property violation, not arbitrary code execution. Repair checks preserve
generation/exclusion/overflow invariants; resource exhaustion and caller misuse
remain explicit limits. The three delayed variants are exact original tasks:
observable occurrences with fault channel; wait-up-to-N partial semantics;
different-CPU synchronized producers. Consume-all is also required by the
original project. Do not relabel an authored or author-executed variant as a
delayed unaided attempt, independent review or physical measurement. Source-bound
campaigns and successor support record actual validation status; no finite suite
proves absolute mastery or all possible schedules.

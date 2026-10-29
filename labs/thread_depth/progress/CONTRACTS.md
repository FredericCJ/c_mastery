# Progress, costs and concurrency evidence

This is the required implementation successor for the nine original criteria
`AX-PR-T25-04-T25.10.a` through `AX-PR-T25-04-T25.11.c` and
`AX-PR-T25-05-T25.12.a` through `.c`, including their exact `-DELAYED` variants.
The original project/criterion tasks, objective mappings, four-level rubrics,
seven-day intervals and historical acceptance records remain authoritative.
This contract supplies concrete inputs, interfaces and executable obligations;
passing its finite tests does not establish mastery or all possible schedules.

## Prerequisites and environment

Retain the T25.01 lifetime/rollback artifact, T25.03–.05 `struct tw_queue` and lock-order
artifact, and T25.06–.09 publication/reuse argument. Import the actual
`labs/thread_depth/waiting/waiting.h` interface and its
`labs/thread_depth/waiting/CONTRACTS.md` ownership rules. Submit the implementation
you produced for that earlier artifact; its exported `learner.c` is deliberately
unfinished. Do not replace the queue with a library container. Use the retained
T25 module and original criterion source records alongside the newly inspected
records in `research/v5/expansion/legacy_thread_progress/resources.json`.

Profile A: hosted ISO C17, POSIX threads on the selected Linux host; GCC and
Clang with strict diagnostics, ordinary optimization, AddressSanitizer plus
UndefinedBehaviorSanitizer, and separately ThreadSanitizer. `pthread_*`, clocks,
thread creation, ordinary allocation for test storage and host-side Python
verification are permitted. Implement donation, admission, aggregation and
history search from the supplied primitives. Linux cache-line discovery and
affinity inspection are implementation interfaces, not ISO C properties.
Do not change scheduling privileges, affinity, host security or device state.
No kernel or physical interrupt execution is part of these hosted observations.
The exact final published C17/C23 source gap is retained; inspected C11/C23
draft passages and compiler modes are identified separately.

Live nonnull control/output pointers, correctly sized disjoint input/output
objects, initialized synchronization objects and retained storage until all
users finish are preconditions unless an interface explicitly accepts null.
Callbacks, arbitrary forged pointers and concurrent destruction are outside
this contract. Initialize/destroy while externally quiescent. Thread errors
are checked; internal impossible mutex/condition errors terminate the original
teaching process rather than pretending rollback succeeded. Cancellation is
deferred only; no asynchronous-cancellation guarantee is supplied.

## T25.10: progress and conditional bounds

**D10a — Four hazards and donation (families A, B, D, F, I).** Supply four
distinct traces: two participants owning opposite locks and each requesting
the other; both releasing and repeating the same retry phase without useful
work; a contender running and losing a lock while another repeatedly completes;
and a low-priority owner delaying a high-priority waiter while an unrelated
medium-priority participant runs. Mark control state, ownership, blocked/runnable
state and completed work for every participant. A finite trace is a prefix;
give the repeating-state argument and scheduling assumptions for an infinite
extension. Repair the first trace by increasing stable lock keys, including
callback self-acquisition; demonstrate the actual earlier `order` laboratory.

Implement `tp_donate(base, owner, n, out)`: `1 <= n <= 4`, priorities 0–7,
`owner[i]` either -1 or a thread index. Return each thread's maximum base
priority among participants with a wait-chain reaching it. Cycles are allowed
as an input graph; donated priority does not dissolve a deadlock. Recompute
from base priorities after removal of an edge. Invalid inputs return
`TP_INVALID` and preserve the output. Input arrays and output are disjoint.
For base `[1,2,3,7]`, owners `[-1,0,1,2]`, all effective priorities are 7;
removing thread 3's wait yields `[3,3,3,7]`. Derive a terminating least-fixed-point
algorithm and compare with an independently written reachability oracle over
160,000 priority/graph combinations. The delayed original task changes the
wait graph and requires transitive donation plus withdrawal, not just boosting
the immediate owner. Explain what actual POSIX priority inheritance guarantees
and what the abstract bounded scheduler additionally assumes.

**D10b — Exclusion is not fairness (A, C, F, I).** Give a legal mutex-counter
schedule in which the excluded contender repeatedly runs but never acquires.
Identify the extra lock-admission fairness assumption; CPU scheduling fairness
alone does not provide it. For the delayed deadline task implement
`tp_bound(cost, n, known, deadline, out)`, 1–8 nonnegative `uint64_t` terms.
The caller must justify that every term is an upper bound in the same time unit,
including a bound on predecessors, owner work, preemption and interference.
`known` is 0 or 1; unknown returns `TP_UNSUPPORTED`, checked-sum overflow
returns `TP_OVERFLOW`, and a sum greater than the deadline returns
`TP_UNSUPPORTED`. Failures preserve output. With two FIFO predecessors at 3,
own work 2 and interference 4, admit deadline 12 and reject 11. No such FIFO
or interference bound is asserted for a default host mutex. Label measured
quantiles separately; never pass an observed maximum as `known` without an
independent bound argument.

**D10c — Whole-path effects (A, D, H, I).** Analyze a path with lock body 50
units, allocation 200, first-touch fault 300, logging 400 and preemption 500
as illustrative observations. Their sum is not a worst-case bound when any
term is unbounded. Trace each dependency, including allocation inside logging
and implicit library calls. In the delayed nonsleepable context implement
`tp_nonsleepable`: zero effects is admitted; any known bit `TP_ALLOC`,
`TP_PAGEFAULT`, `TP_LOG`, `TP_BLOCK`, `TP_PREEMPT` or `TP_UNAUDITED` rejects
with `TP_UNSUPPORTED`; unknown bits return `TP_INVALID`. The mask is a declared
effect argument, not a static verifier or an interrupt-safe certificate.
Audit a generated object for explicit allocator, mutex and output imports;
explain why symbol absence cannot exclude page faults or preemption. Derive a
changed design using bounded preallocated storage and deferred logging, then
list the selected target/context proof still needed. Do not execute unsafe
host work in an actual interrupt handler.

## T25.11: resource and measurement contracts

**D11a — Layout and false sharing (B, C, H, I).** `tp_separated(base, stride,
count, extent, line)` returns whether the occupied byte intervals of at most
four cells occupy disjoint modeled cache lines; `line` must be a power of two,
`1 <= extent <= line`, stride positive. Check address arithmetic before use;
return -1 on invalid/overflow, 0 on shared lines, otherwise 1. This is address
arithmetic, not dereferencing arbitrary integer addresses. The actual experiment
allocates two single-owner atomic counters, compares adjacent and line-strided
placements with the same update count, and records actual addresses, stride,
allocation alignment, reported line size, compiler flags and unchanged allowed
CPU mask. Atomic updates prevent a language data race; distinct values can
still contend for coherence resources. A 64-byte stride can share a 128-byte
line. Changing base alignment can split a 64-byte hot region across lines;
do not assert that a naturally aligned four-byte counter at stride 64 must
share a 64-byte line solely because the base is not line aligned.

Run seven repeats per placement; repeat zero is labeled warm-up. Later repeats
perform 100,000 updates per worker, two workers. Validate every final count.
The timed interval includes controller barrier release and thread joins.
Record distribution and uncertainty; a slower separated run is evidence to
investigate, not a failed correctness test. Address layout plus elapsed time
does not by itself measure coherence traffic. No hardware performance-counter
or cache-line guarantee is inferred from Linux's reported value.

**D11b — Aggregation and current readers (B, F, H, J).** `tp_accumulate` processes
one strided partition of at most 65,536 immutable 32-bit unsigned values;
1–4 workers, `begin < step`, mode 0 private, 1 batched, 2 per-item shared atomic,
batch 1–65,536, intensity 0–64. `tp_mix` repeats the specified modulo-32-bit
recurrence `(x*1664525+1013904223) XOR (old_x >> 7)`. Return sum, checksum,
items and shared update count. Invalid inputs preserve output and shared state;
`data == NULL` is allowed only for zero length. Initialize the shared total
to zero and cover each input once across disjoint partitions. These bounds
fit the 64-bit sums; no shared reset occurs before joins. Test 20,000 inputs
`i%97+1`, two workers, batch 32, intensities 0 and 32. Expected shared update
counts are 0, 626 and 20,000. Independently derive the exact sum and checksum.
Private mode needs a final merge and O(workers) result storage; batch mode adds
partial local totals and publishes only completed batches; per-item updates
have more shared traffic. Do not compare their intermediate freshness as equal.

For the delayed reader requirement implement `struct tp_total`: a mutex protects both
`value` and `items`; each successful addition linearizes under that mutex,
each read returns a current committed pair, overflow preserves both fields.
No lock-free or wait-free claim. Join writers/readers before destruction.
Test two writers and a concurrent reader, then quiescent overflow injection.
The reader's current-total contract requires added synchronization; an old
batch total is not an equivalent repair.

**D11c — Empty/useful/composed cost (C, F, H, I).** Measure 20,000 empty
lock/unlock iterations, the useful recurrence alone, and the same recurrence
inside the lock, with verified lock counts/checksums. Change intensity 1 to 32.
Retain 32 back-to-back monotonic-clock differences, clock resolution, seven
repeats including warm-up, raw values and compiler disassembly showing retained
work/calls. Use median, quartiles, range and explicitly labeled matched ratios;
do not assume composed cost equals the sum of two separately measured costs.
Aggregation timings include create/join and merge, whereas layout starts after
worker creation; do not compare them as identical intervals. Instrumented
profiles validate behavior; use ordinary optimized profiles for the reported
performance comparison. Explain migration, interference, thermal and timer
uncertainty. The changed-intensity task must predict and then evaluate which
cost dominates; no universal speedup or worst-case latency follows.

## T25.12: complementary evidence and changed shutdown

**D12a — Queue campaigns (D, F, G, I).** Reuse the actual capacity-three
`struct tw_queue`, force two consumers to enroll before starting two producers, and
transfer 2,000 distinct copied integer values. Fixed per-producer seeds control
selected yields, but do not make OS scheduling deterministic. Check each value
exactly once, then close, join, retire and destroy in that order. Repeat the
earlier forced queue/wake/deadline cases. Execute an original known-race process
under TSan and a separate race-free logical queue mutant; a silent detector or
unavailable runtime must not be reported as a pass. For the delayed weak-order
variant derive the release/acquire message-passing and reverse-retirement edges,
use the earlier restricted MP/SB model and one/two-reader handoff model, and
exhibit the early-retirement witness. Those bounded models are explicitly the
selected changed condition; no weakly ordered physical execution is claimed.

**D12b — History checker (A, B, C, F).** Implement `tp_history` for an initially
empty capacity-three FIFO and at most six operations. Invocation/response
numbers are distinct positive event positions; response zero means pending.
Thread IDs are 0–3 and one thread cannot overlap its own calls. Complete
responses follow invocations. Enqueue succeeds unless full; dequeue yields
the oldest value or EMPTY. Validate input before modifying `struct tp_witness`.
`TP_REQUIRE_COMPLETE` reports `TP_INCOMPLETE` without changing output when a
call is pending. The delayed `TP_ALLOW_EXTENSION` policy may append responses
to a subset of pending calls, then remove remaining incomplete calls; it must
retain every completed call and every completed-before-invoked relation.
Return the selected order, synthesized pending results/values and completion
mask, or `TP_NONLINEAR`. This is existential finite safety search, not liveness.

Required examples include enqueues 1 then 2 followed by dequeues 2 then 1
(race-free but not linearizable with those nonoverlapping timestamps), duplicate
removal, full/empty boundaries, and pending enqueue 7 preceding a completed
dequeue of 7. Silently deleting that pending enqueue incorrectly rejects a
permitted completion. Moving its invocation after the completed dequeue makes
the history invalid as a queue explanation. Compare a separate permutation
oracle over 22,680 complete histories and twelve pending-policy cases; also
capture and check twenty actual four-operation queue histories per native
profile. The event recorder is synchronized and influences schedules.

Submit an evidence matrix: TSan observes executed races, ASan/UBSan detect
selected executed memory/language faults, history search checks a stated finite
sequential specification, and the ownership/order argument supplies assumptions
and edges. None substitutes for the other or proves unexecuted behavior.

**D12c — Lifetime and shutdown transfer (D, F, G, J).** Minimize the supplied
publication-before-initialization trace to its first violated dependency;
repair source ordering plus matching publication synchronization, and separately
justify last-read-to-reuse ordering. Do not add sleeps. For the original delayed
shutdown task implement `struct tp_end_queue`, composing the actual earlier queue:
close admission, let already admitted calls settle under drain-close semantics,
then discard residual copied integers once and retire the underlying queue.
This differs from drain-to-consumers and from instantaneous cancellation/drop.

At most eight calls are admitted, values are 0–1,000,000 and the conservative
accepted-plus-inflight ceiling is 16,000. Reject excess calls with EAGAIN,
headroom exhaustion with EOVERFLOW, invalid values with EINVAL, and new calls
after closing with ECANCELED. A single shutdown controller calls abort;
repeated abort returns EALREADY. Hold the wrapper admission root until all
external callers join. Disable deferred cancellation across admitted operation
settlement and controller cleanup; restoring it may deliver pending cancellation
after the operation committed but before its caller sees a result. The copied
integer has no external destructor; callers must not infer noncommit from a
canceled return path. This protocol does not bound shutdown under an unfair
scheduler or an owner that never runs. Unexpected internal synchronization
errors terminate the confined teaching process.

Prove the quiescent identity `accepted = delivered + discarded`, maintain
discarded-value sum, preserve failed-pop output, and test full/empty blocked
calls plus a pending canceled producer. Destroy wrapper synchronization only
after join; an inflight count of zero alone does not prove the caller stopped
touching the wrapper. New admissions never touch the retired underlying queue.

## Deliverables, verification and assessment

Deliver D10a–D12c as separate objective-labeled packets: source, derivation,
invariants/ownership timeline, counterexample, failure ledger, exact commands
and raw outputs, independent expected results, relevant cost model, changed
assumption analysis and limitations. Keep analysis-only undefined cases out of
positive executions. Public tests compile unfinished implementations and reject
them at a named `TP_ASSERT`; a build failure is not a semantic rejection.
Use private implementations/oracles only on the assessor side. The author
campaign is `tools/run_thread_progress.py`; its source-bound receipt is author
evidence and does not submit or pass a learner's work.

For each original objective retain its existing four-level rubric. Score 0 for
missing/incorrect contracts; 1 for passing ordinary examples without the stated
proof/counterexample; 2 only when the corresponding D packet demonstrates all
ordinary invariants, failure tests and independently expected outcomes; 3 only
after the original named delayed variant is independently attempted after the
minimum interval and defended under changed assumptions. Donation must assess
withdrawal, bounds must reject unproved inputs, layout must assess active byte
extent, aggregation must assess freshness, costs must assess equal correctness
and interval limits, history must assess pending extension, and shutdown must
assess admission/root lifetime separately from payload settlement. Shared tests
may support multiple packets; award each criterion only on its own argument.

## Graduated hints and changed conditions

Hint 1: mark the first state transition that makes the desired conclusion
possible or impossible. A completed call and a pending call impose different
real-time constraints; a stopped admission gate and a dead worker are different.
Hint 2: recompute donation from base priorities, treat whole-path cost terms as
conditional facts, and write the queue's sequential operation before search.
Hint 3: use graph reachability, checked partial sums, byte-interval intersection,
and a small enumerated history before scaling an experiment. Put every terminal
queue path into an ownership table. Detailed answers belong in the separate
assessor analysis, not this learner contract.

The original nine `-DELAYED` tasks remain required, not replaced by changing
constants. Reassign wait chains; withdraw donation; deny a deadline assumption;
change to a nonsleepable context; change cache line and active extent; require
current intermediate totals; change useful-work intensity; use a weak-order
model; retain pending operations by extension; change queue shutdown ownership.
The existing original criterion mapping selects the applicable changes.

## Security and evidence limits

All adversarial scheduling, race and mutant demonstrations use original teaching
processes, copied synthetic integers and local temporary files. No external
service, real credential, host policy change or deployed device is involved.
The observable failures are a reported race, wrong FIFO/count, unproved bound,
or violated ownership invariant; none establishes arbitrary code execution.
Compare the flawed and corrected implementation under the same checks, and
retain remaining schedule/model/target limitations. Author implementation,
executed profiles, independent review, learner delay and physical observation
are distinct receipt dimensions. No claim is made for unexecuted architectures,
all histories, general scheduler fairness, or absolute mastery.

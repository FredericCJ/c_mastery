# Assessor analysis: T25.10–.12

The learner contract and original nine criterion objects are required. This
file and the implementations/oracles remain outside learner exports. The
source-bound validation receipt reports actual observations; the solutions
below are author work, not an independent review or learner assessment.

## T25.10.a: hazards, transitive inheritance and withdrawal

The AB/BA graph has a directed cycle with both owners waiting. Ordered stable
keys remove that cycle only if every acquisition, including callback paths,
obeys the rule. The actual earlier `order` test demonstrates the opposite
try-lock state and repaired order without permanently hanging the process.
`hazards.py` also executes finite lock-state traces. Livelock returns to the
same acquisition/backoff control state with neither operation committed;
starvation lets a denied participant run while another commits; inversion
lets medium work delay the low owner needed by high. They are different
properties; a thousand steps alone proves none is infinite. Repetition under
the explicit adversarial scheduler gives the extension argument.

The donation implementation propagates maxima along wait edges from BASE
priorities. At most n-1 distinct edges are needed for a reachable donor, so
n relaxation passes suffice even with cycles. The independent oracle follows
each donor's reachability path rather than repeating the same relaxation code.
For withdrawal, remove the edge and recompute; carrying previous effective
priorities would retain stale donations. Delayed oral counterexample: break
the final edge of the 1→2→3→7 donor chain, then another middle edge. A solution
that boosts one direct owner or never lowers priority fails the original rubric.
The POSIX inspected protocol describes recursive inheritance; the synthetic
scheduler's fixed-priority choice and bounded owner work remain extra premises.

## T25.10.b: exclusion, fairness, response bound

Acquisition uniqueness implies neither fair admission nor eventual acquisition.
A runnable loser can be scheduled only when the winner owns the mutex, then
lose again on release. Weak fairness of runnable threads does not force a lock
grant. The declared FIFO bound is two predecessors×3 plus own2 plus
interference4 =12. Refusing unknown interference is the correct delayed answer,
even if every measured sample is below12. Checked addition must happen before
comparison; wrapping the sum would fabricate admission. No default pthread
mutex guarantee supplies the assumed predecessor count. Classify the result as
a conditional derivation, not a measured worst-case execution time.

## T25.10.c: effects and nonsleepable change

The short critical section says nothing about hidden allocator contention,
faults, output blocking or scheduling. The object import probe deliberately
retains malloc/free, pthread lock and write dependencies; it never runs an
unsafe handler. Its admission mask rejects every such dependency and unaudited
calls. Absence of those symbols is not proof against faults or preemption.
For the changed context, preallocate/touch permitted memory, move output to a
bounded deferred queue, and establish target priority/interrupt/memory rules.
If those rules are unavailable, refusal is the correct answer. The pure mask
function is a policy checker whose input assertions still require evidence.

## T25.11.a: actual extent and locality

Map inclusive first/last byte addresses to line numbers after overflow checks;
ordered nonoverlapping cells share a line if next first <= previous last.
Stride64 separates four-byte counters for a line64 model but not line128.
Misaligning a full64-byte active region by4 splits it across adjacent lines;
claiming the same failure for a single aligned four-byte active counter would
be an incorrect overgeneralization. Record actual addresses and alignment.
Two atomic counters have separate modification orders even when on one cache
line. Their physical coherence interaction is distinct from ISO data races.
Elapsed differences with unchanged affinity are observations with migration
and interference confounders, not direct cache-traffic measurements. Timing
order alternates placement within each repeat; it is not randomized/blinded.

## T25.11.b: equal final results versus freshness

The input partitions are disjoint and cover the array. Each local sum and the
total fit64 bits under the n/width bound. The batch scheme flushes a final
partial batch, giving 2×ceil(10000/32)=626 shared updates. The private scheme
has zero updates and needs a final merge; per-item has20000. Checksums use an
independent arithmetic recurrence with division by128 to express logical
right shift for uint32 values. The shared relaxed sum is meaningful as a final
result after joins; it is not publication of other state. Delayed current
readers need a new linearization contract. `TPTotal` updates value and items
under one mutex; readers observe one committed pair. Overflow preserves both.
It sacrifices nonblocking progress and adds read synchronization explicitly.

## T25.11.c: timing interpretation

All three loops execute20000 iterations, and useful/composed checksums match.
The empty loop has no checksum contribution and is the synchronization control,
not an equivalent application by itself. Two intensities change compute per
critical section. Raw repeated data, timer resolution and clock-pair overhead
are retained; report ordinary optimized distributions separately from
instrumented correctness runs. Compiler disassembly must retain mutex calls
and the recurrence. No claim that costs add exactly is justified by isolated
measurements, and a bottleneck change on one host does not predict all hosts.
The three aggregation modes have equal final sum/checksum contracts, but their
intermediate freshness and timer boundaries are explicitly different.

## T25.12.a: deterministic enrollment, nondeterministic schedules

Actual condition enrollment precedes producer creation; no sleep guesses that
a worker waited. The stress seeds control yield requests only, not the kernel
schedule. All2000 distinct copied values appear once across the two private
consumer ledgers. The earlier waiting tests cover surplus wake, deadline and
cancel/close paths. A dedicated original race must be reported by TSan; a
logical mutant changes a popped value while retaining locking and must fail
the semantic oracle. TSan silence does not establish FIFO or linearizability.
The delayed model imports the actual earlier MP/SB checker and handoff state
enumerator: relaxed MP may see ready1/data0; publication plus matching acquire
forbids it in the stated fragment. Early retirement permits overwrite while a
reader still owns data. Neither model enumerates every ISO C program or runs
an actual weak-memory CPU.

## T25.12.b: finite histories and pending extension

The C search checks every candidate next operation against outstanding
completed-before-invoked predecessors, simulates capacity3 FIFO, and backtracks.
For pending operations it enumerates completion subsets; omitted calls are
explicitly incomplete after extension, never completed calls silently removed.
No pending response constrains another operation to occur after it in the
recorded history; appended responses occur at the end. Well-formed per-thread
intervals prevent a second call after an unresolved same-thread invocation.
The worst search is bounded by subset enumeration times six-operation
permutations; fixed automatic storage and at most six recursion levels suffice.
Output publishes only a complete witness. The Python oracle enumerates event
orders/permutations independently. Its22680 complete cases and12 pending cases
are a finite differential corpus, not proof of arbitrary search input.

The completed sequence enqueue1, enqueue2, dequeue2, dequeue1 violates FIFO
without any data race. Removing real-time constraints falsely admits it and is
the intentional history mutant. Pending enqueue7 before a completed dequeue7
must be completed in an explaining extension; ignoring it falsely rejects.
If its invocation follows that dequeue's response, no extension repairs the
ordering. Twenty actual four-call histories are recorded under a separate
clock mutex and checked after join; instrumentation changes the schedule and
cannot make all histories observed. The evidence matrix must distinguish this
finite safety check from race detection, memory checking and a proof argument.

## T25.12.c: first broken edge and alternative shutdown

The minimal operational witness is publish→read while data is uninitialized;
write-before-release and reads-from-acquire establish forward publication.
Retirement needs the reverse edge from the last reader to reuse. The tiny
trace minimizer does not model weak visibility; the imported memory model and
handoff argument supply that separate obligation. Do not reward inserting a
delay that only hides the witness.

`TPEndQueue` closes its own admission root before closing the underlying
drain-close queue. Admitted calls finish, decrement inflight and account
committed copies before cancellation is restored. The controller waits for
zero inflight, drains residual copies to its discard ledger, releases the
underlying final reference and retires it. New calls examine only the surviving
wrapper gate. External join is still required before destroying that gate:
zero inflight is insufficient because a departing thread can still restore
cancellation or return through caller-owned state. The pending canceled
producer test confirms no stranded inflight credit; cancellation delivery does
not prove whether an admitted operation committed. The implementation bounds
counts/headroom and the discarded sum; it never rolls a generation counter.

The final conservation equation is accepted=delivered+discarded. During work,
additional terms include buffered copies and operations between inner commit
and wrapper accounting; do not assert the quiescent equation in that interval.
The wrapper deliberately settles admitted operations before discarding the
residue. This is an independently specified delayed policy, not instantaneous
abort, and requires scheduler/operation progress for termination. No imported
frozen queue source is changed. The shutdown mutant erases discard accounting
and fails even though the underlying locks remain race-free.

## Rubric and withheld transfer administration

Use each original criterion's exact ID and task. Require its corresponding
D packet and the ordinary/changed evidence above; passing another packet earns
no implicit coverage. Level2 needs the contract, mechanism argument, concrete
counterexample and intended negative control. Level3 additionally needs the
actual original delayed interval and an independent learner attempt without
this analysis. Select a changed graph with multiple donors; an unknown
interference term despite reassuring samples; a hot extent different from the
field size; a concurrent current reader; an intensity regime dominated by work;
a pending enqueue required by a returned value; or a shutdown with blocked
producer plus pending cancellation. Ask for a counterexample to any blanket
fairness, race-free-implies-linearizable, padding-always-faster, or zero-inflight-
means-dead claim. The author campaign never awards level3 or certifies mastery.

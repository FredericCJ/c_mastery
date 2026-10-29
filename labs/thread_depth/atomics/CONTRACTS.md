# Publication, atomic objects and CAS — required T25 successor, version1

This contract implements the twelve unchanged criteria
`AX-PR-T25-03-T25.{06,07,08,09}.{a,b,c}` and their original `-DELAYED`
tasks. The exact prompts, objectives, prerequisite units, graduated hints and
four-level criteria remain in
`curriculum/v5/expansion/legacy_assessments/PR-T25-03.json`; its
`shared-contract.md` applies. The additional experiments below are required.
No historical hours, units, acceptance receipt or learner result changes.
Reasoning, construction, boundary/diagnosis, falsification and changed-condition
families are selected because each separates a different failure mechanism.

## Prerequisites and environment

Bring the assessed T25.01 thread-lifetime and T25.03 predicate/ownership
arguments, checked unsigned arithmetic, object lifetime and function-pointer
contracts. Keep the parent U-T25-03-01 through -04 bindings. A thread is joined
before its argument storage, lease, control object or synchronization object
is destroyed. Every pointer supplied below denotes a live, correctly aligned
object; arrays cover the stated extents. Objects stay at stable addresses.
Control, output and borrowed payload objects are disjoint. ISO C cannot check
these preconditions or linear ownership. Invalid scalar/count admission returns
before publication; arbitrary invalid addresses are not a supported fault.

The core remains profile C, ISO C17, freestanding primitives and caller storage.
Do not import a queue, snapshot, graph or reclamation implementation. The target
closure contains `cas.c`, `publication.c`, `models.c` and the public header.
The learner implements these in `learner.c`. `_Atomic` is a language primitive,
not permission to link an undeclared helper. The host harness uses Linux
pthread, allocation for ownership probes, Python models, GCC/Clang, sanitizers,
object tools and the explicitly linked host `libatomic` aggregate provider.
Those are verification dependencies, never automatically target dependencies.
The header requires exact uint8/16/32/64 types for the width experiments.

Run separate GCC/Clang debug, optimized, ASan/UBSan and TSan configurations.
The selected cross targets are Cortex-M4 Thumb and Cortex-M0 Thumb1, separately
from native x86-64 and actual hosted i386 execution. No generated target image
is executed or flashed in this package. A successful helper-free image does
not prove ISR latency, preemption safety, fairness or board behavior.

## Publication and compound state — T25.06

For .06a draw event graphs with distinct sequenced-before, reads-from and
synchronizes-with edges. Name `W(payload=42)`, release `W(ready=1)`, acquire
`R(ready=1)`, and `R(payload)`. Then make ready's load read its initial zero:
the failed branch does not read payload. No publication edge can be inferred
from elapsed time or an unrelated acquire. For the delayed reuse task, include
the receiver's last payload read, its release retirement and the writer's
acquire of that retirement before overwrite. `ta_closure` computes reachability
for1..8 events, represented by one unsigned successor mask per event; reject
out-of-range bits unchanged. Closure alone is not a C execution validator.

For .06b annotate actual source expressions before supplying the binary access
descriptors to `ta_data_race`: same memory location, which accesses write,
whether both are atomic and whether a happens-before relation orders them.
Distinguish disjoint array elements from adjacent nonzero-width bit-fields;
a zero-width field separates the example memory locations. Mixing ordinary
representation access with an atomic operation does not make both atomic.
Do not run the undefined source expressions. The classifier returns-1 for a
nonbinary descriptor; its seven supplied cases are evidence of classification,
not a dynamic detector for arbitrary programs.

For .06c reproduce the actual two-atomic-balance history: start50/50, pause the
writer after storing40 to the first account, observe total90, then permit the
second store60. Compare with one mutex covering both stores and both reads;
the observer cannot enter the partial transaction. Implement `struct ta_snapshot` with
one writer, two atomic integer fields and an unsigned sequence. Values range
0..100 and sum100. A write publishes odd sequence, both fields, then next even
sequence using SC operations; reject before wrap. `ta_snapshot_try` accepts
1..16 attempts, publishes only an equal-even observation, and preserves output
on `TA_LIMIT` or invalid admission. Each attempt is bounded; system-level
success or wait freedom is not promised. Do not replace atomic fields with
ordinary fields and claim the version test erases a data race.

## Actual atomic operations and target dependencies — T25.07

For .07a implement `struct ta_counter`: initialize before sharing, relaxed fetch-add
returns the old value, relaxed load returns its observed value, unsigned wrap
is modulo. The UINT_MAX +2 probe distinguishes old versus new return values.
The changed aggregate is `_Atomic(struct ta_pair)` with two uint64 fields. Initialize,
load, store and initialize a separate destination from the loaded semantic
value; never memcpy a live atomic representation, infer padding equality or
assume a lock-free sixteen-byte operation. Host aggregate helpers are explicit.

For .07b report actual object size, type alignment, address alignment and
`atomic_is_lock_free` results for8/16/32/64-bit objects, a valid64-byte-aligned
64-bit object and the pair. Report relevant `ATOMIC_*_LOCK_FREE` macros without
pretending they are per-object timing measurements. Repeat on the actual i386
ABI. Never form a deliberately misaligned atomic pointer to test alignment.
`ta_lockfree_admit(query,unresolved_helpers,context_authorized)` is a local
admission policy: allow only query1, helper count0 and explicit context
authorization1; nonbinary query/authorization is invalid. Authorization is an
external reviewed assumption, not something the query establishes.

For .07c inspect generated instructions, every undefined symbol and the final
link map. Link the complete core with no hosted library on M4. Separately
compile `target.c` with function sections and isolate `target_native`,
`target_wide`, and `target_pair`. Record the first additional atomic helper on
widening and the M0 native-width helper. The required negative link excludes
providers; a missing-helper diagnostic is the expected rejection, never a
successful target implementation. Do not fix it by silently adding libatomic.
If proposing a provider, its storage, locks, reentrancy and interrupt/preemption
rules require a separate audit before any ISR use. Host Python is not in that
closure. The historical fixture is only an abstract dependency reachability
case; retain it without claiming it already checked emitted atomic helpers.

## Restricted execution models and reusable handoff — T25.08

For .08a implement `ta_litmus` for the following exact model. There are two
atomic integer locations x,y initially0, each written once with constant1;
two threads, no fences/RMWs/dependencies, no out-of-thin-air calculation. MP is
`T0: x=1; y=1`, `T1: a=y; b=x`. SB is `T0: x=1; a=y`,
`T1: y=1; b=x`. Relaxed means all operations relaxed; RA means release stores
and acquire loads; SC means all operations SC; mixed means SC stores and
relaxed loads. Enumerate all32 kind/order/outcome combinations. Return1 for a
permitted pair,0 for forbidden,-1 outside this restricted domain. Submit RF,
modification-order and HB arguments, not only a lookup table. The independent
SC oracle enumerates total orders; applying that oracle to RA would wrongly
exclude store-buffering00. Mixed loads do not become members of the SC order.

For .08b `struct ta_slot` has one writer, one or two fixed reader identities, ordinary
payload and an atomic pending bit mask. Initialization is quiescent. Publish
requires mask0 acquired, writes payload, then releases the full reader mask.
Each reader acquires its own bit, copies payload and clears only that bit with
an acquire-release RMW. Reuse waits for the final retirement. A reader identity
must never have two simultaneous callers. Every reader consumes every
generation; departure or cancellation requires a separate protocol and is not
silently treated as retirement. Nonready operations return `TA_LIMIT`; outputs
remain unchanged. Analyze both the forward publication and reverse RMW chain.
Test2,000 generations for each reader count. Separately enumerate every
transition boundary of two publications and fixed readers; the early-clear
variant must supply a concrete overwritten-read trace. This model checks the
state machine under its ordering obligations, not all weak-memory executions.

For .08c compare actual inspected draft passages and selected compiler behavior.
The direct consume-loaded pointer feeds its field load; the control-only
variant chooses whether to read an unrelated pointer and does not carry that
dependency. `ta_dependency` classifies the supplied value-use/control-only/
kill descriptors, not arbitrary C expressions. Demonstrate one actual
release/consume pointer publication and one release-fence, relaxed flag,
acquire-fence communication. A fence requires the specified intervening atomic
reads-from relation; an isolated fence is not a universal publication operation.
Inspect consume/acquire/control and thread/signal-fence lowering on each target.
TSan does not model this fence synchronization: the fence execution is omitted
only in TSan profiles and remains mandatory in debug/O3/ASan/UBSan profiles.
Compiler success and acquire-like consume lowering do not prove that the
control variant satisfies ISO dependency rules. Exact final C17 and published
C23 source comparison remains an explicitly uncompleted source obligation.

## CAS and ownership — T25.09

For .09a classify all36 named success/failure pairs before executing valid
ones. The declared conservative contract forbids release/acq_rel failure and
failure stronger than success. A successful CAS is a read-modify-write;
failure is a load that updates expected. The changed release-only success uses
relaxed failure. Invalid pairs are analysis-only, never run as experiments.

For .09b `ta_increment` accepts a supplied CAS callback, initial expected,
1..16 attempts and a one-attempt flag. Each retry recomputes desired from the
new expected. Script spurious failure with unchanged10, mismatch updating20,
then success with21. The one-attempt mode requests strong CAS; ordinary mode
requests weak. Limit exhaustion publishes a report but no success claim;
invalid admission preserves the report. A weak implementation may choose
strong behavior, but the converse does not justify one-attempt weak success.
Callbacks return normally, update expected on mismatch, and never retain its
address. Constant work per attempt excludes callback internals.

For .09c `ta_publish` owns a typed heap node until actual callback success.
Failures preserve caller ownership; they never free the newly observed node.
The changed desired-value computation accepts at most4 explicit borrowed-node
leases held by the coordinator until return. Membership compares pointers
only; it does not acquire or validate lifetime protection. Before dereferencing
expected, require nonnull membership or return `TA_UNPROTECTED`. Recompute
after mismatch. The owned node is distinct from every borrowed node. Its value
may change during an unsuccessful derivation; ownership still stays with the
caller. A successful receiver can free the published node before callback
return, so the sender must not inspect it afterwards. The tests include actual
C pointer CAS plus immediate terminal receiver free. This is no general hazard
pointer or ABA solution. Reclamation is required separately in T26.

## Deliverables, verification and assessment

Submit D1 annotated event/source tables and exact edition/implementation labels;
D2 compatible core implementation, declared operations and ownership diagrams;
D3 ordinary and changed-condition execution logs plus independent finite
oracles/counterexample traces; D4 compiler, ABI, alignment, helper/link maps and
resource tables; D5 limitations, failed attempts and residual source gaps.
Costs include O(n^3)/O(n) graph closure for n<=8, constant atomic operations per
slot attempt, at most16 snapshot/CAS attempts, at most4 lease comparisons per
CAS attempt, and fixed storage. Lock-free atomic operations do not make the
whole retry protocol wait-free or bound the host scheduler.

Use public tests independently of private assessor sources. The author command
`python3 tools/run_thread_atomics.py` records source-bound GCC/Clang profiles,
intended mutant failures, compiled learner-stub rejection, model enumeration,
actual i386 queries and separate target audits. A source-only export contains
this contract, public header/tests/probes and unsolved learner implementation;
assessor implementations and finite-model solution are withheld.

The unchanged objective-specific rubric is authoritative. For each leaf,
level0 fails its named contract or omits reasoning; level1 supplies the ordinary
output without explaining its mechanism; level2 supplies D1–D4 with an
independent expected result and the relevant failure/boundary evidence;
level3 additionally succeeds on the exact original changed condition after
the required minimum seven-day learner interval and defends the argument under
questioning. An author running that packet does not award level3 to a learner.
Hints progress from identifying the shared location/ownership edge, to naming
the relevant RF/SW or failure transition, to isolating the smallest offending
history; the separate assessor analysis supplies solutions and limits.

## Security and validation limits

All targets are original local teaching programs using synthetic numbers and
fresh process memory. No external service, credential, kernel policy or board
is touched. The race sensitivity control is a separate intentionally racy
process with no numeric oracle; it establishes selected TSan sensitivity only.
Undefined semantic examples and invalid CAS-order pairs are never executed.
Overwrite/ownership counterexamples establish the stated property violation,
not code execution or privilege escalation. Repair and regression evidence
must preserve the stated correctness contract. Compiler/sanitizer/model results
are bounded author evidence; external review, independent learner assessment,
physical observations and exact unavailable edition passages remain separate.

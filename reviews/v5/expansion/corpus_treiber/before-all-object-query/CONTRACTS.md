# Treiber stack, protected snapshots and bounded reclamation

## Representation and admitted calls
C17, `unsigned` at least32 bits, lock-free atomic unsigned required at initialization.
`TsStack` has eight physical nodes, runtime capacity1..8, four fixed participant
hazards and one atomic index head (`UINT_MAX` means empty). A node holds copied
uint32 data and a next index. This is a bounded index representation of Treiber's
single-head linked stack, not the retained Michael-Scott queue or work-stealing
deque. The state word is `generation*4+phase`: FREE0, CLAIMED/LIVE1, RETIRED2.
Initial generation0 is issuable. `TS_GEN_MAX` is `UINT_MAX/4` normally and3 in the
required changed translation. A MAX-generation node may be pushed/popped once;
its final retirement is permanent. No tag wraps, and a retired slot does not
return to the free pool without a matching-generation scan/reclaim.

All controls/backing/output objects are live, correctly aligned and disjoint;
only the same stack is intentionally shared. Init requires fresh storage or
quiescent storage after all operations, protected views and collector tickets
have ended and all callers have joined. Do not copy/move an initialized stack.
Do not mutate public fields except explicit single-threaded white-box tests.
Participant0..3 is exclusively owned by one caller at a time, through the whole
begin/commit/cancel interval; registration/reassignment requires joined old users.
Push and collect require no participant. There is no asynchronous cancellation,
unregistration or destructor that magically revokes another thread's hazard.
No allocator, thread API, callback, container library or atomic helper shim is
permitted inside the target implementation. Pthreads and Python are host harness
dependencies only. Init refuses unsupported atomic lock freedom; the initialized
object may then be discarded/reinitialized quiescently, never operated on.

## Operations and publication
`push` copies its value into an exclusively claimed free node and initializes
next before the successful head CAS, its linearization point. CAS failure updates
the observed head and rewrites the still-private link. There is no exposed node
handle and no operation to reinsert a removed live node. FULL means this finite
claim scan reserved no node; concurrent freeing may occur afterward, and FULL
does not assert a linearizable snapshot of logical element count. It includes
claimed-but-not-yet-published nodes, protected retirements and exhausted slots.
Push performs no implicit collection; retry policy belongs to the caller.

`begin(stack,p,fresh_view)` repeatedly reads head, publishes a hazard and
revalidates head BEFORE reading the ordinary payload/link. Empty returns EMPTY
and leaves the fresh output unchanged. A successful view is a single, noncopied
authority with an immutable snapshot and a live hazard. A delayed view may retain
a removed node; it is not a lock on the head and need not eventually succeed.
`commit(view,out)` does one strong head CAS. On success it copies the snapshot,
retires the old head, clears the hazard, invalidates the view and publishes out.
On contention it clears/invalidates the view, returns RETRY and preserves out.
Null output admission returns INVALID and leaves the live view/hazard intact so
cancel remains possible. `cancel` clears/invalidate without removing an element.
`pop` repeats begin/commit on RETRY; EMPTY preserves out and linearizes at an
empty head observation. Only successful commit publishes a value. Required null
arguments and out-of-range participants return INVALID. A view passed to commit
or cancel must be either a current live view or an explicitly invalidated/zero
view, not indeterminate storage. All atomics are deliberately SC; this is a
sufficient order, not a weakest-order claim.

## Reclamation and failure
`scan(stack,index,fresh_ticket)` reads a RETIRED generation and scans ALL hazards.
If none protects that index and its generation is below MAX, it stages a ticket;
otherwise RETRY preserves output. `reclaim(ticket)` consumes the ticket on either
OK or RETRY and CASes that exact retirement to next-generation FREE. A collector
delayed across another collector's reclaim/reuse/retire cycle cannot free the new
incarnation. `collect` visits each admitted slot once and counts successful
reclaims. A scan ticket does not pin a node; its generation CAS is essential.
The single-node stalled-reader case must return FULL while retaining memory and
resume only after hazard release. No failure authorizes rewriting protected data.
Strong CAS failures imply interference for the selected atomic semantics; there
is no wait-free per-caller retry bound. Bounded scan/reclaim return safely even
when useful allocation cannot progress. A stopped pusher can retain its claimed
slot; a stopped reader retains one slot; crashed callers cannot be silently
reassigned. The finite memory policy is not unbounded allocation progress.

## Proof obligations and changed assumptions
Prove acyclic head-to-next reachability, unique claim, immutable published data,
and conservation of free/claimed/live/retired nodes (claimed and live share a
phase but are distinguished by reachability). For SC protection, reason about
the total order of hazard publication, head revalidation, successful removal,
retirement observation and collector scan. If removal precedes successful
revalidation of the old incarnation, the equality cannot persist through reuse
while its hazard is continuously live. An index newly reused BEFORE protection
may safely be observed as a new node; no old ordinary fields were read yet.
Explain why a protected head's successor cannot be independently popped while
that head remains reachable, and why a failed CAS must discard the successor.

V1: delay a pop view while other participants remove/reclaim/push; also delay a
collector ticket across reuse and a second protected retirement. Run the finite
generation3 translation through complete exhaustion without obstructing cleanup.
V2: four competing native participants plus actual two-participant histories;
compare a list oracle under real-time constraints and run the actual implementation
under GenMC RC11. Remove release publication and head revalidation separately.
Compare with the original all-atomic ABA specimen, which reinserts a live index:
even never-freed storage or garbage collection alone does not prevent its stale
CAS from restoring a logically removed node. Production API deliberately excludes
that operation. A tagged head alone would still not justify dereferencing freed
memory. The existing `cmx_hp_*` queue and identity pool supply prerequisite learner
arguments, not an automatically interchangeable stack implementation.

## Costs, language comparison and evidence limits
Space O(N+P), fixed arrays, zero allocation and recursion. Claim scan O(N),
collection O(NP), uncontended head work O(1); retry work depends on interference.
All callers contend on head; node-state/hazard traffic and cache placement matter.
Report actual object size, instructions/helper dependencies and counts before
making an equivalent-contract timing comparison. ISO C defines atomic ordering,
not lock-free support for every type/target, scheduling fairness or physical
latency. C++ atomic syntax does not add automatic reclamation; garbage-collected
languages can prevent premature freeing but still admit logical reinsertion ABA.
Ownership/noncopying, participant exclusivity and shutdown quiescence are caller
discipline here, not C type-system enforcement. Runtime index/null checks do not
validate arbitrary forged control fields.

Native, sanitizer, independent finite traces, finite RC11 models, and strict M4
object/link inspection are separate evidence. No hardware execution, silicon
latency, arbitrary-history proof or independent learner mastery is inferred.
The original IBM1986 report remains inaccessible; inspected author-hosted modern
Treiber pseudocode and Michael2004 reclamation clauses are the algorithmic sources.
The retained N1570 is a C11 draft, not the published C17/C23 standard. Target C17
is the implementation baseline; atomic facilities originated in C11. No reliance
on deprecated/later-edition atomic initialization changes is silently assumed.

## Security laboratory
Only original in-process synthetic values/nodes are targets. The adversary controls
operation timing, stalled participants, full pools and the original reinsertion
specimen. Harmless success is wrong synthetic membership/value, premature reuse or
lost ownership. The all-atomic specimen demonstrates logical ABA without undefined
behavior; model-checker data-race findings establish a violated memory-safety
premise, not executable arbitrary code or privilege escalation. Repair by the
exact protection/publication/generation rules, retain causal regression controls,
and explain remaining finite-resource denial of service from stalled callers.

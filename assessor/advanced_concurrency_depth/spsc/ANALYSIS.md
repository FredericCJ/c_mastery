# SPSC assessor analysis

The six original criterion instances share one mechanism but retain their
specific PR-T26-01 prediction and PR-T26-04 construction duties. Historical
acceptance is not rewritten. No learner receives this file or implementation.

## T26.03.a

The retained CMX API initializes valid scalar queues and returns a lock-free
boolean. Its invalid0 is not the enum constant; the additive AS API makes the
enum failure explicit. A power-of-two capacity1 can be full because monotonically
advancing counters distinguish distance1 from0. Ordinary owner-local active
flags reject nested callbacks in the same execution domain. They do not make
the code safe for two concurrent producer threads: both could read/write the
ordinary flag and slot. The symbolic double-head snapshot yields one final
head advance and loses A; stronger atomic ordering on head cannot restore the
missing reservation ownership. Two consumers similarly duplicate a claim.
The delayed repair is admission rejection for same-domain reentry, with changed
status semantics stated explicitly; a true MPMC design remains a separate task.

## T26.03.b

Forward release/acquire publishes all four ordinary words. Reverse release/
acquire protects the consumer's last read from the next writer. A scalar load
being naturally indivisible would not establish either ISO happens-before edge.
Fixed storage avoids free but not conflicting access. The hook gates force the
last prepublication/prereuse boundary without relying on scheduler timing.
The early-index mutants fail these particular stronger hook-placement contracts;
the synchronization in the forced test prevents their synthetic failure from
becoming a native race. Missing-order variants are assessed in the relation
model as ordinary-access race candidates, never given a reliable native value.
No-hook concurrent tests transfer16000 multiword records across four capacities.
Instrumented clean runs remain finite evidence. Hooks are external effects and
must be bounded/noncanceling under any end-to-end progress claim.

## T26.03.c

Let unbounded logical producer/consumer positions be P,C. The invariant is
0<=P−C<=capacity. Producer observation T cannot regress in modification order;
it only advances P while P−T<capacity. Therefore observed distance stays within
the declared modulus interval, including wrap. The consumer has the dual
monotone observation H and only advances C while C<H. An old coherent observation
can deny an opportunity, not invent one. This is why an arbitrary stale integer
unrelated to the participant's earlier observations is not a permitted model
state. The separate reduced-state enumerator includes coherent stale reads
across two wraps and does not assert every ISO execution has been enumerated.

For modulus64 and cap1,2,4,16, reachable fresh states are64×(cap+1) and transition
edges128×cap: totals1728 and2944. The native state projection maps tail near
UINT_MAX, head=tail+occupancy, and exercises matching slot indices. N−1 storage
has N² normalized states and2N(N−1) edges; N1 has no usable slot. Full tests
compare next(head)==tail rather than unsigned distance==capacity.

Inspect target objects rather than calling C atomics intrinsically lock-free.
M4 and M0 compiler lowering may differ even for aligned32-bit load/store, and
the frozen retained CMX object includes unrelated CAS helpers unless functions
are sectioned and the selected image is garbage-collected. Report selected-image
closure and full-object imports separately. A link audit with no undefined
symbols does not establish physical ordering, cache coherence or ISR reentrancy.
The64-byte layout is a selected representation; measure offsets without claiming
every cache uses that size or that separation guarantees a speedup.

## Assessment

Fail claims that relaxed payloads are safe because storage is fixed, that
N−1 and full-capacity counters use the same proof, that enum INVALID equals
boolean0, or that retrying forever is bounded per-request progress. Require each
original task's construction, derivation and changed-condition defense. The
exact delayed IDs and interval are preserved in canonical metadata; source-bound
author execution does not constitute independent level3 learner evidence.

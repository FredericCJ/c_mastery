# T53.11 assessor analysis

The shared contract is `labs/smp_depth/pool/CONTRACTS.md`; the public learner
implementations remain deliberately unsuccessful. The reference fixes the
capacity at3 and ledger at16 to make every ownership transition inspectable.

## `.a`: acceptance, claim, cancellation and nested work

The trace accepts10/20/30/40 and refuses50 because the queued sequence20/30/40
already fills three positions while10 is RUNNING. A claim removes a queue
position but leaves a live job slot, so a zero queue length is not a drain
criterion. Completion is the protected state transition, not callback entry.
Running cancellation sets a request; the owner publishes the terminal state.
Slot collection cannot race away a running owner. IDs bind world+slot+generation;
the externally distinct world lifetime is necessary because initialization resets
per-slot generations. The terminal maximum generation can still be collected.

The replay oracle keeps a dictionary keyed by logical accepted IDs, rather than
the C slot array, and computes every short operation history independently.
Some histories deliberately abandon an older running claim after claiming another;
the expected ledger retains that running job and does not invent liveness.

Three native parents wait until the controller fills the shared queue with three
children. A second barrier prevents any parent from draining a child before every
parent tries submission. All three return FULL, preserve the output ID, finish
their own parent and drain the existing children. This demonstrates an explicit
refusal policy, not successful acceptance of every recursively requested child.
The single-worker helper test shows another legal response, outside the lock.

## `.b`: two masks and the degraded state

An allocation bit is set before create; a started bit is set only after successful
create. Workers are held at gate0 so failed prefixes can publish gate2 without
running user work. Reaping visits all allocations: unstarted allocations release
directly, started allocations release only after join0. A join failure leaves
both bits and the pointer intact. Reaping continues other slots and exposes
DEGRADED if any remain. The next shutdown retries only retained handles. No new
launch is permitted during degradation. This is a safety/containment guarantee,
not bounded completion of arbitrary callbacks or a promise that join will recover.

Mutants7/8 are intentionally evaluated only by mock callbacks that never actually
free stack memory. Mock assertions expose premature release or ignored failed
join. Mutant9 leaves the failed-startup gate at WAIT; the mock detects a join
without STOP. Native controls inject errors before real backend calls and then
actually join successful threads. The target's requested fourth child reaches
the retained kernel's actual process-thread limit. Its64KiB initial stack arena
is recycled after join and remains mapped until process lifetime ends: there is
no CMK free operation to claim otherwise.

## `.c`: equivalent work, deques and limits

Every native range is disjoint and covers the same global integer interval.
The per-index mix and aggregate sum use specified unsigned modular arithmetic,
so repartitioning preserves the exact result. Host Python recomputes ranges with
unbounded integers and explicit64-bit masking. Static/dynamic policy comparisons
share job count/cost/checksum; the chunk variant preserves the logical interval
while changing range boundaries. Record launch and idle overhead honestly. A
sampled queue high-water mark is a lower bound on its actual peak. Admission
attempt time is not exact enqueue time; it is an upper bound on residence under
the successful attempt. No cache topology or speedup result is assumed.

The work-stealing variant is deliberately a fixed-array coarse-lock design.
Owner newest and thief oldest removal both linearize under the same lock, so
there is no reader holding an old resized array. A proof for this variant does
not establish Chase–Lev memory ordering or reclamation, which needs a different
representation. Callback work never runs while holding the queue lock. Counter
capacity is consumed only on admission, with terminal counters bounded by prior
admissions, so cleanup cannot become unavailable at the maximum accepted count.

## Assessment separation

All three original delayed variants have concrete author support. The learner
must still produce a new ownership/progress argument and changed execution after
the required delay. The actual CMK1/2/4CPU guest is distinct from physical hardware.
Native TSan observations are bounded detection evidence, not a C-model proof.
The canonical support records exact checks and preserves earlier historical
execution fields; it must not silently promote an author campaign to independent
learner or human-reviewed evidence.

# T53.05 assessor analysis

This is withheld author support, not a learner handout. Follow the public
contract's exact ownership, no-cancellation and saved-state premises.

For criterion a, the outstanding admission counter is claimed before ticket
fetch-add. At most15 live reservations occupy a16-value namespace, including
the owner. Pausing before ticket allocation cannot create a duplicate live
ticket because no more than15 grants exist. Both unsigned backing-counter wrap
and the public low four bits preserve the sequence because16 divides the
unsigned modulus. The admission bound deliberately avoids a full-modulus queue;
it is not a heuristic for processor count. The independent17-value trace
exposes ticket0=ticket16 if that bound is removed. That arithmetic alone is
insufficient: a new participant can otherwise acquire-load the initial serving0
after wrap without acquiring the previous owner's writes. The bounded RC11
wrap litmus actually exposed that pre-freeze defect. Retirement now releases
the ownership history by using sequentially consistent admission, allocation
and serving operations. Their common order plus the15-live bound puts at least
one appropriate serving advancement before a reused ticket's readiness load;
the old equal initial value is not the latest eligible SC store. A successful
read acquires the actual releasing owner's publication. An early admitted
participant may pause before allocating a ticket, so admission-only reasoning
would be incomplete. This sufficient policy is not a weakest-order proof.
An intermediate acquire/release candidate did not finish its exploratory
full-turn model within the allotted tool runs; it is retained as unvalidated,
not the reference or evidence for a minimal-order claim. Keep the original
relaxed path as mutant6. Mutant4
removes both release paths so the ordinary publication failure remains distinct.

MCS tail exchange publishes the current node. A predecessor can observe next
NULL even after a successor exchanged the tail but before it linked next.
Failed tail CAS therefore requires waiting for the linkage, not retirement.
The predecessor's release write to successor.waiting hands protected state to
the successor's acquire read. The uncontended path uses release tail-CAS and
the next owner's acquire exchange. Clear own active only after all own-node
references are complete; no predecessor accesses a successor node after its
wakeup store. Node reuse while active is independently refused. Per-CPU storage
does not solve nested-interrupt lifetime unless the IRQ mask prevents reentry
until that node is retired; two nested locks still require two nodes.

For criterion b, save/disable occurs before admission and the private context
is visible only to its current CPU with interrupts masked. Rank is checked
against the active top and strictly increases. An invalid release cannot pop
the stack. The successful release caches saved state, releases the lock, pops
the private stack and restores IF last, with no subsequent guard access. Inner
saved state is0; outer saved state can be0 or1. The actual raw-owner diagnostic
observes its self-IPI in the suspended-owner state and exits before entering an
infinite wait. This demonstrates the cycle's premises and safe diagnostic
containment; do not claim it measured an actual infinite wait. A second CPU's
progress while BSP IF0 distinguishes local masking from global exclusion.

For criterion c, the identical protected counter and16-add workload supplies
a common correctness contract. The selected RMW/wait counters differ in
meaning from physical coherence traffic: ticket admission and node-active CAS
are intentionally outside them. Do not rank algorithms using raw counter units
as though each were an equal-cost instruction. Per-thread waits, gaps and
counts are observations; finite completion does not prove starvation freedom.
The independent two-thread/two-lock scheduler has an opposite-order deadlock
state and no increasing-order deadlock within its finite state space.

The actual GenMC program reads/writes an ordinary protected counter. Its missing
release mutant leaves atomic ownership control but removes protected-data
happens-before; require the reported non-atomic counter race, not a generic
tool failure. Record blocked executions and spin assumptions. RC11 is the
selected tool model, not a claim that all C editions or scheduler liveness were
verified. Native mutants never execute this intentional C data race.

The NUMA guest actually observes node membership and pins four participants
across two nodes. It does not move pages or isolate nodes on cache lines. The
correct changed-condition conclusion can be that the virtual measurements do
not answer the physical remote-cache question. A learner who claims locality
from affinity alone has not satisfied c. Physical results require explicit
page policy, machine topology, equivalent workload, sampling and new evidence.

All three original criteria and all three delayed variants require their own
reasoning and deliverables; a passing final counter alone is level1. Retain
original delayed interval and learner evidence semantics. External human review,
independent learner evidence and physical observations remain separate.

# T53.09 technical analysis

The original generation9 example requires target1 even if its ACK is nonzero.
The equality is part of the authority check; an acquire load alone does not
associate a value with the correct request. The reference fixes membership
under the publication lock and refuses entry during a pending request. A
different admission protocol can be correct, but needs its own proof and tests.
Entry after completion flushes before use. GLOBAL includes all online CPUs:
being absent from this address space is insufficient to exclude a cached global
translation. Offline deletion is not provided; missing completion retains the
frame indefinitely.

The request fields are atomic because a delayed handler may overlap a later
request. It validates the full tuple, invalidates, validates again, then updates
its ACK monotonically. An old handler cannot lower an ACK, overwrite a new
generation, or authorize a request whose generation differs. Invalidation may
be repeated harmlessly. Publication and handler completion never depend on an
ordinary kernel lock in the interrupt handler. The tests invoke the local
handler synchronously while the membership lock is held to discriminate that
deadlock-prone design. No remote callback may pretend to invalidate another
physical CPU; the guest asserts local identity.

Timeout does not change pending state or produce a permit. Notification failure
returns SIGNAL after committed publication; retry only sends missing targets.
Generation exhaustion refuses new requests but does not block cleanup. The
frame-owner proof is deliberately separate: the two-space permission experiment
retains the frame after either shootdown because live mappings still own it.

The original deterministic cache model keeps every C object live and uses array
indices. Its stale777 write is a modeled permission failure, not C undefined
behavior. The guest uses actual original page tables and delays CPU1's IPI with
IF clear. In the defective condition, that CPU observes synthetic999 through
the old mapping after frameA reuse; the corrected condition waits for INVLPG
and then sees frameB's200. The observation is a bounded property violation.
It does not establish arbitrary code execution or privilege escalation.

For global pages, INVLPG targets the selected global page even with PCIDE=0;
CR3 reload alone lacks that guarantee. The context adapter uses PGE disable and
restore for a stronger flush. The ISA permits over-invalidation, so a negative
CR3-only experiment that happens to pass would not prove correctness. No
PCID/INVPCID implementation or weakest-order proof is claimed.

Permission testing uses CR0.WP and two distinct roots, with alternating CPUs
in each address space. After spaceA becomes read-only, only its CPUs fault on
the dedicated write probe. After spaceB also changes, all fault. The handler
accepts only the known RIP, CR2 and present/write error bits; its return skips
that original probe and returns a failure status. Other faults terminate the
experiment. This is supervised teaching instrumentation, not an exception
recovery policy for arbitrary application faults.

One four-page request and four single-page requests perform equal page work.
They differ in request/notification overhead, not correctness. Compare actual
counts first and retain each local TSC sample; no physical speedup follows
from one virtual run. Native stress establishes selected concurrent membership
executions under sanitizers, not hardware invalidation or all C executions.

Assess `.a`, `.b` and `.c` independently using the public criterion rubric.
Require the changed-condition learner to derive target masks and frame owners
without the private reference. The author campaign cannot substitute for the
original delayed independent learner assessment. Physical hardware and external
human review remain separate evidence dimensions.

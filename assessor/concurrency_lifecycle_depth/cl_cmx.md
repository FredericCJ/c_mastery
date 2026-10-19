# Queue/lifetime adapter technical analysis

The reference composes the original CL byte and lease mechanisms with actual
CMX latch waits. A per-task slot records the borrowed sender bytes only while
admitted and pending; immediate acceptance copies through `cl_send`. Progress
settles CMX results before examining queue capacity. Reversing that order lets
a timed-out sender's still-visible pointer commit after its caller may reuse
the buffer. The terminal case deliberately creates that exact order.

A condition slot differs from a queue slot: it remains a lifetime reference
through mutex reacquisition and through result collection, until explicit
unlock. CMX's intermediate mutex waiter stores its chosen outcome in `pending`.
Cache this before close/cancel; calling raw cancellation again would overwrite
the selected outcome. Serial/object/task/lease generations reject different
stale identities and cannot replace storage-lifetime preconditions. Exhausted
admission returns RETIRED; cleanup remains available without a new serial.

The IRQ wrapper protects the complete enrollment hook and operation with the
saved mask. Its test pends IRQ0 inside enrollment and proves it runs only after
restoration; nested masking stays masked. The callback lease survives close.
The dedicated software source has no peripheral/DMA state: join disables its
NVIC entry, clears pending, reads active/enabled/pending, then drops the lease.
The source must not be repended afterward. Treating this as a generic device
join is an overstatement. The two controls independently omit disable and
pending clear; retaining only the enable check would miss one of them.

The task veneer mirrors retained blocking admission: current RUNNING task,
matching machine, thread mode, all relevant masks clear. It locks, enrolls,
pends PendSV when blocked, restores the saved mask and collects after actual
resumption. Imported SVC/PendSV code switches separate PSP stacks; callers do
not call their entry again to simulate suspension. The tests sample those
stacks, then require task exit and coordinator reap before reuse. The copy
scenario suppresses periodic ticks after its actual handler check so missing
PendSV cannot be rescued accidentally. Timeout and condition scenarios retain
SysTick. An initial campaign incorrectly expected the no-dispatch mutant to
fail even where a coincident tick could dispatch it; that failed test attempt
is retained, and the strengthened cooperative case is the discriminator.

The close-open mutant is discriminated by the implicit port sender after
close. An ordinary full-queue sender may already fail through the lifetime
registry, concealing the missing byte-queue close. The host campaign's initial
wrong case association is retained as a failed test attempt. The corrected
case checks the actual missing mechanism, rather than accepting any failure.

Task runtime accounting uses 60 unique producer/sequence identities. Consumer
logging is later than dequeue and can be preempted, so that runtime ledger does
not assert its logging order equals queue linearization order. A separate
externally serialized actual-CMX dispatch trace checks per-producer dequeue
order. Abort preserves `accepted = delivered + discarded + count`; application
acknowledgement remains the separate original volatile acknowledgement variant.

Do not award the robust owner-death criterion from CMX planned exit. Its
unchanged producer refuses exit while synchronization is owned. The prior
actual pthread robust-record experiment supplies unexpected owner/repairer
death and bounded repair, with explicitly different platform contracts.

All target helpers are explicit original support sources. Generated structure
copies may call byte helpers; map and symbol audit allow only that exact set,
not an undeclared compiler runtime. Finite O(N²) progress and real emulator
execution do not provide WCET, SMP memory-model correctness, physical IRQ
latency, arbitrary fault containment, external review or learner mastery.

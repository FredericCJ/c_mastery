# T53.05: lock ownership, interrupt state and measured contention

## 1. Identity and families

This required successor supplies executable author support for
`AX-PR-T53-05-T53.05.a`, `.b`, `.c` and their existing `-DELAYED` variants.
Families A/B/C/D/F/H/I/J apply to the distinct obligations below. It preserves
the accepted PR-T53-05 identity, descriptive hours and historical evidence.
The learner C and guest/assembly files remain unsolved.

## 2. Objectives and inspected references

Criterion a derives TAS, finite ticket and MCS representations and node lifetime.
Criterion b implements saved local interrupt state, two nested ranked locks and
the protected-data publication chain. Criterion c compares equivalent workloads,
falsifies missing release ordering and distinguishes observed contention from
fairness or physical cache-traffic claims. Exact inspected passages, editions,
hashes and access limitations are registered in
`research/v5/expansion/legacy_smp_locks/resources.json` and `source_access.json`.
The Rochester authors' pseudocode page is an inspected algorithm source, not an
assertion that the entire 1991 paper was read. Linux implementation details are
not ISO C semantics. The inspected atomic text is the N1570 C11 draft; exact
published C17 text remains unavailable and no C23 equivalence is inferred.

## 3. Prerequisites and learner artifacts

Supply completed PR-T53-02 startup and PR-T53-03 private table/stack work, the
retained `kernel_smp` implementation, and PR-T53-04 interrupt/EOI reasoning.
Supply an explanation of atomic reads-from/happens-before, object lifetime,
ordinary data races, unsigned wrap and the selected AMD64 interrupt ABI.
The guest composer takes completed folders explicitly and verifies a precise
startup source hook. Port that hook deliberately for a differently structured
learner implementation; it never locates or copies assessor solutions.

## 4. Environment and dependencies

Implement the core as ISO C17 using fixed caller storage, `<stdint.h>`,
`<stddef.h>` and `<stdatomic.h>`. No allocation, OS call or target runtime helper
is permitted. Require lock-free integer/pointer atomic operations in the selected
guest and audit generated instructions and final undefined symbols. Hosted
pthread, affinity, clock, sanitizers, Python and GenMC are verification tools,
not freestanding dependencies. The AMD64 QEMU TCG guest uses real local IF,
interrupt gates, xAPIC IPIs and 1/2/4 logical CPUs, with sparse IDs 0/1/2/4.
Changed compiler profiles compile the adapted mechanism; the retained
GCC-specific kernel prerequisite and GNU assembler remain explicitly identified.

## 5. Concrete problem, inputs and outputs

Implement `locks.h` in `locks.c`. Each `struct sl_lock` has an immutable kind and positive
global rank. `sl_init` runs exclusively before sharing; ticket start is 0..15,
and other kinds require start0. It validates before mutation. A guard and
per-CPU `struct sl_context` start zero, are exclusively owned, live and neither copied
nor moved while active. Node initialization requires live exclusive storage.
Inputs must not overlap unrelated lock/control objects. Stats are optional,
owner-private and initialized to zero; use workloads whose counters fit64 bits.

TAS uses exchange-acquire admission and release unlock. A ticket lock reserves
from an actual four-bit ticket namespace: start14 yields14,15,0. The selected
finite ticket implementation uses sequentially consistent admission, allocation
and serving atomics, which include the required acquire/release semantics.
Admission is
bounded to15 outstanding guards, strictly below modulus16. The16th reservation
returns `SL_FULL` without changing its guard. `sl_ticket_reserve` returns a
queued phase1 guard; `sl_ticket_ready` returns `SL_PENDING` until its ticket is
served, then publishes phase2 ownership. There is no cancellation or abandoned
reservation. `sl_enter` waits for an admitted ticket; only its owner releases it.

MCS uses a caller-owned node with atomic next/waiting/active fields. Claim the
node before publishing it to the tail. A node already queued or owned returns
`SL_BUSY`, including when reused for a different lock. A node remains live and
stable until its owner completes `sl_leave`; a successor may still be linking
after an unlocker's empty-next observation. No queued node may be force-cleared,
moved, timed out or reclaimed. One node per CPU is sufficient only when a
same-CPU interrupt cannot enqueue that same node before the prior owner retires.
Two simultaneously nested locks require two distinct nodes per CPU.

`sl_leave` accepts phase2 only, releases the protected object and returns the
guard to phase0. Invalid operations do not authorize ownership or cleanup.
Reinitializing a shared lock, forged guards, recursive admission and asynchronous
termination while owning/queued are outside the admitted contract.

`sl_irq_enter` saves and disables the local interrupt state before admission.
`struct sl_port` save/restore callbacks are live, non-reentrant and restore the exact
saved state; they return normally. The selected port saves IF as0/1, not all
RFLAGS. Enter and leave must use the same live callbacks and port context;
switching ports while a guard is active is outside the admitted contract.
A context can hold at most four guards in strictly increasing global
rank; wrong order/depth returns `SL_ORDER` after restoring entry state. Release
must match the exact top guard pointer and is LIFO. A rejected wrong-order
release leaves the held locks and interrupt state unchanged. Callers may not
manually enable interrupts, migrate or switch context while a guard is held.
NMI and synchronous faults are not masked by this API.

## 6. Mechanisms and misconceptions

An atomic flag is not a lifetime proof. A finite ticket counter is not safe under
arbitrarily many outstanding requests. An MCS queue node cannot be reused because
its owner merely stopped spinning. Disabling local interrupts neither excludes
another CPU nor gives a protected ordinary object a C happens-before edge.
Unconditionally enabling interrupts on unlock breaks nesting and callers that
entered with IF already clear. FIFO admission does not imply scheduler fairness
or bounded wall-clock latency.

## 7. Required arguments and invariants

Derive one protected owner per lock and the direct acquire/release chain for
ordinary payload accesses. Bound distinct live ticket values and explain why
reservation admission precedes ticket allocation even if a reserving CPU pauses.
The selected sequentially consistent order also ties admission, allocation and
serving observations together. Occupancy alone does not prevent a first-time
entrant from reading an old equal serving value after wrap when the other
operations are relaxed: show why the selected order excludes that value, not
just a distinct-live-tickets argument. This is a sufficient ordering policy,
not a claim to use the weakest possible atomic orders.
For MCS, separate publication of a node, successor linkage, tail-CAS retirement
and successor wakeup; identify the last access to each retired node. Explain the
local-handler/paused-owner cycle for same-node recursive use.

Prove increasing ranks exclude a cycle only when every participating acquisition
obeys the same global order; no claim is made for raw unranked calls. Derive
LIFO saved-state restoration for both initial IF0 and IF1. Give O(1) storage per
lock, O(participants) live MCS nodes and O(nesting) private context storage.
Waiting duration depends on holders, scheduling and interrupts, not just the
number of C instructions. Ticket admission may refuse despite free processor
capacity; refusal and indefinite waiting are different outcomes.

## 8. Implementations and experiments

Implement all three locks, saved-state wrappers and the guest/assembly adapter.
First reserve15 tickets starting14, reject the16th, observe the second pending,
then release in order. Attempt one active MCS node on a different lock; require
BUSY and unchanged second tail. Test both initial interrupt states, nested
rank1/rank2 locks, reversed release and a different lower-rank lock.

The guest executes200 task and200 real interrupt acquisitions per CPU, with
both nested locks protecting each increment. A self-directed IPI is issued
inside the masked region; it must run only after the owner has retired both
nodes. Require per-CPU depth0, nodes0, interrupts200 and total work400*CPU count.
While BSP IF is clear, a second CPU must independently publish progress. Inject
an always-enable restore port and a raw unmasked owner. The latter deliberately
detects the delivered handler's suspended-owner dependency before spinning:
this is a finite observed deadlock premise, not an artificially hung machine
misreported as a successful diagnosis.

Run four pinned native threads, each2000 acquisitions and exactly16 additions
0..15 per acquisition for every algorithm. Require count8000 and sum960000.
Record each participant's observed CPU/NUMA node, successful acquisitions,
selected algorithm RMW attempts/failures, waiting loads, clock wait sum/max,
maximum intervening acquisition count and shared elapsed interval. Counters
cover the TAS exchange, ticket allocation and MCS tail operations, not node-
validation/admission CAS, all generated atomic instructions, cache misses or bus
traffic. Instrumentation and clock calls have cost. No performance threshold or
universal winner is prescribed; compare distributions under equal contracts.

For the changed placement, boot the supplied private Linux VM with two observed
virtual NUMA nodes, CPUs0–1 and2–3, distance matrix10/30. Execute the same static
probe pinned across those nodes and record sysfs topology and affinity results.
CPU affinity does not establish local page placement: these objects may be
first-touched by the main thread, and nodes are not promised isolated cache
lines. This is executable changed-topology evidence, not physical NUMA latency,
bandwidth, cache coherence or page-policy validation.

## 9. Verification and pass/fail

Run `tests.c` in GCC/Clang debug, optimized and ASan/UBSan profiles. Run `native.c`
additionally with both GCC and Clang TSan. Compiler success is mandatory before
an intended assertion rejection can count. The public stub must compile and
fail semantically; it is not a learner success. Check independent finite lock-
order schedules: opposite orders admit a two-owner cycle; common increasing
order does not in this bounded model. Enumerate16 starts and17 reservations to
show ticket aliasing and the admitted bound.

Use actual GenMC RC11 on `publication_model.c`, with the completed core on its
explicit include path, for TAS/ticket/MCS. Two threads increment an ordinary
protected counter. Each reference must report no error; changing unlock to
relaxed must report the actual non-atomic counter race. Keep the exact version,
flags, explored/blocked executions and counterexample. Do not execute that
deliberate data race as an ordinary native correctness test. A bounded model
result is neither arbitrary-program verification nor a liveness proof.

Also run `wrap_model.c`: one thread acquires16 successive tickets; a first-time
entrant observes allocation16 with a relaxed fetch-add-zero and then reserves
ticket0. The diagnostic RMW changes no value and provides no acquire ordering.
This observation constrains the schedule without adding a synchronizes-with
edge. Probe readiness before any join. If pending, retain the live guard, join
the first thread and finish the guard; the join only supplies cleanup progress
after the tested probe. The version with relaxed admission/allocation/retirement admits an old serving0 and
an ordinary counter race. The repair must exclude that execution. Use the actual
GenMC header's assume macro; the installed v0.19 runtime does not implement the
manual's standalone declaration as an external callable symbol.

Build the guest with explicit completed prerequisites:

```sh
python3 labs/smp_depth/locks/guest.py --lab labs/kernel_smp \
  --kernel COMPLETED_KERNEL --startup-public labs/smp_depth/startup \
  --startup-completed COMPLETED_STARTUP --implementation COMPLETED_LOCKS \
  --work NEW_PRIVATE_DIRECTORY --kind 2 --cpus 4
```

Require final zero undefined symbols and inspect actual atomic/CLI/STI/IRETQ
instructions. Correct guest exit33 must accompany all exact per-CPU records;
each intended control requires exit35 and its own diagnosis. Generic failures,
timeouts, crashes or sanitizer faults do not count. The source-bound author
campaign retains raw commands/logs and independent oracle output. Re-run changed
conditions with new source identities; never relabel an earlier receipt.

## 10. Per-objective rubric

Score a/b/c independently. Level0: broken ownership/order, unsafe containment or
invented execution. Level1: ordinary mutual exclusion without the respective
counterexample or lifetime argument. Level2: a derives finite admission and MCS
retirement with actual boundary cases; b demonstrates exact nested state and
remote progress plus ordering/link evidence; c gives equal-work raw results,
actual missing-release model counterexamples, lock-order analysis and measured
limitations. Level3 additionally solves that criterion's named changed condition
unaided after seven days, revisiting assumptions and explaining conflicting
results. Author reference execution is not learner acceptance or independent
human assessment.

## 11. Graduated hints and assessor separation

1. Write which agent may still read each node after the owner sees next=NULL.
2. Draw publication, ownership and retirement as separate events; annotate
   their reads-from edges before choosing memory orders.
3. Save entry IF before masking, and simulate inner release with entry IF0.

The completed implementation and technical analysis remain under
`assessor/legacy_smp/`; learner exports contain no completed source.

## 12. Changed-condition variants

a-DELAYED: reuse one MCS node per CPU across task/interrupt phases; explain and
detect nested interrupt reuse before admission. Use separate nodes for two
simultaneously held locks. b-DELAYED: implement two ranked IRQ-save locks,
LIFO state restoration and explicit order rejection under initial IF0 and IF1.
c-DELAYED: run equal-work comparisons across the two virtual NUMA nodes, retain
node lifetime, and name the physical measurement and page-placement assumptions
that require new evidence. These are authored executable opportunities; actual
delayed learner performances remain unassessed.

## 13. Isolation and security scope

All flawed variants target original small curriculum programs or disposable
teaching guests with synthetic counters. No network, host-disk passthrough,
credentials, deployed kernels, hardware flashing or host security changes.
Attacker-like scheduling controls only the laboratory participants and interrupt
delivery. The violated properties are protected-data publication, state
restoration and progress; no crash/race is described as arbitrary code execution.
Repair the exact defect, re-run its discriminator and retain residual scheduler,
NMI, physical timing and finite-model limitations.

## 14. Status and evidence limits

Current support records author executions separately from external review,
independent learner assessment and physical observation. GenMC is a selected
RC11 bounded program analysis; TSan/native stress and virtual guests establish
only their observed conditions. The executable NUMA transfer does not establish
physical placement or performance. No finite collection proves absolute mastery.

# Concurrent-system implementation and evidence contracts

These contracts extend the shared component cards. Each project names the stage
and supported target; a later feature is not an unstated early prerequisite.

Declare the numerical priority convention in every trace. The finite inheritance
and ceiling fixtures use larger numbers for higher urgency; the original
`execution` runtime uses smaller numbers for higher urgency. Translate urgency
when moving an example between them. Numerical order alone is not a portable
scheduling contract.

## State, lifetime and authority

Keep process address spaces/handles separate from thread context, stack, wait and
join state; keep CPU-local current-task and interrupt state separate from both.
Every ready/wait/timer-list membership has one owner. Task states have explicit
transitions among NEW, READY, RUNNING, BLOCKED and EXITED. Reaping requires that
no CPU, wait object or callback can still use the task's state or stack.

The advanced OS uses bounded admission: eight process slots, thirty-two total
thread slots and four threads per process. Treat these as tested limits and reject
one beyond each. Retain the original six-slot T48 compatibility workload. Static
ELF spawn selects a validated boot-archive image; it is not a general exec/dynamic-
linking API. COW clone admits exactly one thread and no outstanding operation,
gives the child only explicitly permitted handles, and rolls back partial failure.

Use generation identities for revocable handles. Shared-memory messages carry
validated offsets/lengths, not pointers meaningful only in another address space.
Pin a mapping/frame while copying or using it across a blocking operation; a
successful earlier range check does not protect against a concurrent unmap.
TLB invalidation acknowledgments precede frame reuse. A failed acknowledgment
quarantines affected state; it must not be treated as a successful remote flush.

## Waiting and synchronization

Wait registration, protected-state release and blocking form one serialized
transition. Grant and timeout race for one result. A timeout cannot consume a
permit or message that the caller was not told it acquired. Absolute monotonic
deadlines avoid extending waits after every notification. State rollover bounds
explicitly and use a half-range argument only within its valid interval.

| Primitive | Required contract and failure experiment |
|---|---|
| Semaphore | Checked maximum/count; permits have no mutex ownership; reject overflow |
| Mutex | Nonrecursive, owner checked; ISR acquisition forbidden; test inversion and nested donation withdrawal |
| Condition | Predicate loop under the matching mutex; atomic release/wait; reacquire before returning |
| Message channel | Fixed capacity and explicit copy/borrow ownership; normal close drains accepted messages; abort discards and wakes waiters |
| Events | Wait-any/all and clear-on-consume defined; bits coalesce rather than count occurrences |
| Notifications | Count accumulated occurrences with explicit overflow policy |
| Barrier/once | Generation and initialization failure/lifetime defined; no early reuse while old participants remain |
| Reader/writer lock | Declared fairness policy; exercise writer starvation and interrupted acquisition |

Cancellation is cooperative. Owner failure poisons affected mutexes and wakes
waiters with failure; protected state is not silently declared consistent. A
recursive lock attempt, stale handle and wrong-owner unlock must produce an
explicit result. In fixed-priority variants, donation follows dependency chains
and is removed on timeout, cancellation and unlock. A ceiling protocol states its
task/resource assumptions; it is not interchangeable with ordinary inheritance.

ISR-safe APIs are separate bounded entry points: no blocking, allocation, remote-
core waiting or arbitrary callbacks. They report whether deferred scheduling is
needed. Interrupt masking only excludes the relevant local contexts. A spinlock
must account for local interrupts and cannot be held across blocking operations.

## Machine and compiler boundary

Use the strict E/F variant: original reset/entry/vector/trap/context code and
required runtime helpers; no linked libc, libgcc, compiler-rt, libatomic, HAL,
OpenAMP or existing scheduler. Compiler headers and inspected inline intrinsics
are permitted. Audit final link inputs and instructions as well as undefined
symbols: static linkage can hide a dependency from `nm -u`.

Save the context required by the ABI and architecture, including alignment,
exception-return metadata and any enabled FP state. Begin with integer-only
contexts, then test FP preservation and the selected lazy-stacking policy.
Distinguish compiler ordering, C synchronization, hardware barriers, MMIO and
cache/DMA ownership. `volatile` does not establish a happens-before relation.

## SMP, AMP and progress

The x86 SMP route starts with a coarse kernel-entry lock and deferred rescheduling,
then separates locks after correctness. Timer preemption, concurrent kernel entry
and a fully preemptible kernel are different claims. Per-CPU ownership, remote
wakeup, migration and shootdown protocols each need independent adversarial tests.

The H745 route has separately linked M7/M4 images and static peripheral/task
ownership. Both linker scripts reserve identical common SRAM, excluded from
ordinary `.bss` clearing. M7 initializes shared state before publishing an epoch-
qualified ready handshake. Shared queue state is authoritative; HSEM doorbells
may coalesce, requiring drain/recheck before sleeping. A peer timeout does not
authorize reusing state it may still own.

Start with noncacheable shared control and SPSC rings using aligned 32-bit indices
and inspected acquire/release publication. Cacheable payloads use cache-line-
exclusive ownership: clean/complete before publication, invalidate/complete before
consumption. Include unrelated dirty-byte loss and late-DMA counterexamples.
Do not assume generic inter-core CAS support on this target.

Specify lock-free progress for the whole admitted operation, including allocation,
reclamation and callbacks. Identify a linearization point and record overlapping
operation histories. Stalling one participant is an essential test, not proof of
all schedules. Hazard protect/validate/retire and epoch quiescence are lifetime
protocols; neither follows merely from making a pointer atomic.

## Real-time and physical observations

Separate task WCET assumptions, release jitter, blocking, interrupt interference,
scheduler overhead and communication-server interference. Derive response-time or
demand bounds for the declared model; compare model traces with actual traces.
Keep fixed-priority ceilings and EDF resource rules distinct. A utilization bound
for independent uniprocessor tasks does not certify a shared-resource multicore
application. Overload handling must name drop, defer, coalesce or fail behavior.

Physical H745 records include both image hashes/boot epochs, overlapping work
pulses on one analyzer timebase, a shared peripheral timer for common timestamps,
per-core DWT deltas, fixed clock settings, silicon revision, cache/MPU regions,
trace overflow and instrument overhead. Never subtract unrelated M7/M4 cycle
counters as if they shared a synchronized origin. Run cold-reset trials detached
from the debugger. Mark simulated, emulated, instrumented and physical records
separately, preserving missing observations as not-run.

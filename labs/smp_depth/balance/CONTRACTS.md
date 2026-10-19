# Affinity, stopped-context migration and bounded balancing

## Identity, objectives and families

Required successor to PR-T53-08, AX-PR-T53-08-T53.08.a/b/c and their original seven-day `-DELAYED` variants. Families A/B/C/D/F/G/H/I/J assess ownership derivation, construction, failures, falsification, equivalent-work observation and transfer. Objective .a concerns effective versus requested affinity and the safe context boundary; .b concerns reservation/rollback and timer routing; .c concerns one execution owner, pinned placement and qualified resource measurements.

## Prerequisite knowledge and learner artifacts

Supply the completed PR-T53-05 locks, PR-T53-06 queues, PR-T53-07 wakeup and PR-T53-02 startup adapters explicitly. Preserve the required PR-T56-01 system substrate and its baseline contract. The target composition uses the actual retained CMK startup and saved-context switch. Bring a lock-order graph, queue ownership table, saved-stack representation and the wakeup port contract. Host OS affinity calls are permitted only in the measurement harness; they do not implement the C scheduler mechanism.

## Environment, editions and inspected references

The core uses fixed-storage ISO C17 and no allocation/library calls. C11 atomics are the relevant edition change; selected N1570 passages are inspected, while exact readable published C17 remains unavailable. Native tests use Linux/GNU `pthread_setaffinity_np` and `sched_getaffinity`, whose per-thread masks and kernel/cpuset intersection are separate from ISO C. Actual man-pages6.19 DESCRIPTION/ERRORS/NOTES passages were inspected. The teaching mask has exactly up to four declared CPUs and rejects out-of-domain bits rather than silently intersecting them.

The target is original x86-64 GNU C/assembly in a disposable QEMU TCG guest. It preserves general-register saved stacks in one address space, without FP/SIMD/TLS, preemptive migration, CPU hotplug or NUMA memory placement. The companion T53.07 adapter exercises actual idle IPIs; this balancing comparison deliberately polls so its measurements do not become idle-wakeup evidence. See the source register for exact inspected producer/source boundaries.

## Self-contained problem and inputs

Implement `balance.c` and the saved-context adapter against `balance.h`. Compose the actual queue and wakeup implementations. Four task slots initially occupy CPU0; task0 is pinned there, while the movable policy permits tasks1–3 on CPUs0/1. The fixed queue capacity is four. A bounded balance call performs at most one admissible READY migration. First repair a task whose current home is excluded; otherwise move only when source load exceeds an admissible destination load by more than one. Load includes reserved return/transfer capacity. Tie order is ascending task slot and destination index.

The concrete changed-affinity fixture runs task1 on CPU0, requests mask{1}, saves its context, and resumes it later on CPU1. Reject empty masks, bits outside the world, running-frame migration, stale task generations and already-consumed transfer tickets. Inject failure before reservation and after reservation/before commit. Timer tokens armed at CPU0 must remain valid while migration is in progress and resolve the authoritative new home afterward.

## Mechanism and misconception

Affinity is an admissible placement set, not an instruction that instantly stops an executing context. Copying or publishing a live frame would permit simultaneous execution. A copied old CPU number is not enduring callback routing authority. Source-first locking can deadlock two opposite moves. Equal queue lengths also say nothing by themselves about unequal task costs, migration overhead, cache state or response deadlines.

## Representation, invariants and ownership

`SbWorld` owns the actual `SwWorld`, one policy lock, per-task effective/pending masks, migration counters and one timer lease per task. All underlying queue operations go through this API. Lock order is policy→wakeup control→queue locks in ascending CPU index, with the actual queue producer enforcing the final order. One caller owns each actor CPU, no ISR calls occur, and port callbacks remain stable/nonreentrant. Inputs/outputs are disjoint from control storage. Initialization creates an externally distinct world lifetime and invalidates all previous handles, tickets and callback tokens; it is not a reusable same-world reset.

READY, RUNNING, BLOCKED and TRANSFER retain the exact queue invariants. An effective mask may exclude a READY/BLOCKED task's current home: that task cannot be dispatched there. Head-of-queue refusal is explicit; this small policy does not skip arbitrary ineligible heads or promise fairness. A mask excluding a RUNNING task's CPU becomes pending and returns DEFERRED; the effective mask remains unchanged until `sb_finish` after the real stack has been saved. Finish applies the pending mask before a subsequent dispatch/migration can acquire policy authority. A newer valid mask request replaces the earlier pending request.

## Operations, failures and cleanup

`sb_move_begin` only admits READY tasks, allowed destinations, available destination capacity and a reserved migration-counter increment. The queue reserves both source rollback and destination commit space. Injection1 refuses before mutation. Injection2 performs an actual reserve/rollback; task ownership and caller outputs are preserved, but queue revisions have advanced and a failed rollback notification may require retry. The diagnostic transfer state is never dispatchable. `sb_move_end(commit=0)` restores source ownership; commit publishes destination ownership, increments the counter once and notifies through the real wakeup port. SIGNAL means commit already happened: retry notification, never replay or roll back the consumed ticket.

At migration counterMAX, new moves are refused; dispatch/finish/exit and timer cancellation still work. Timer serialMAX is usable once, after which new arms are refused while cancellation remains legal. Slot creation/reuse requires whole-world quiescence and no live timer lease. Task generation changes on reuse; a stale copied token cannot gain authority over a replacement task. No counter wraps silently.

## Timer callback routing and lifetime

`SbTimer` stores a task generation and timer serial. Its `old_home` is diagnostic, not authority. Fire/cancel resolves the live token under the policy lock; fire uses the current task home. During TRANSFER, fire returns MOVING and retains the lease so the caller retries after resolution. Accepted wake, duplicate existing READY/RUNNING service or SIGNAL consumes the lease. Other refusals preserve it; cancellation is the cleanup route. The world must stay live for a caller presenting a token, and destroying it requires all callers quiescent. This is a generation-checked synchronous callback-entry protocol, not a claim to cancel arbitrary external code or a hardware timer ISR. The native/guest tests invoke the actual entry at explicitly controlled boundaries.

## Required implementation and experimental deliverables

Submit source and a table of every admitted/refused transition, including outputs and notification status; a no-duplicate/no-lost-context argument; the CPU-index lock proof; and a timer arm→moving refusal→new-home resolution→retirement trace. Explain why neither an old CPU field nor an IPI can replace ownership checks. Derive O(T*C+K) bounded selection/queue work and O(T+C*K) storage, plus callback and TAS contention costs; no wait-free or fairness bound is claimed.

Host tests cover deferred running affinity, disallowed READY dispatch, both injected failures, full destination, rollback capacity, timer retirement/reuse, failed notification, terminal counters, pinned selection and stale identity. The independent finite model places a wake before/during/after commit and rollback and constructs the opposite-lock cycle. It also gives an explicit equal-length counterexample: costs[100,1,1,1], two migrations costing10 each, pinned job0, placement[0,1,1,0]; maximum work101 plus20 overhead exceeds the original103.

## Verification and expected results

The actual guest executes four saved contexts for seven actions each, one block/wake boundary and56 context switches. In the pinned policy all run on CPU0; in the movable policy tasks1/2 execute first on CPU0 and later CPU1, task0 remains pinned, and precisely two transfers occur. An independently represented trace checker validates each generation/revision predecessor, exclusive execution transitions, complete source/target pairs and the actor/task causal DAG. Require64 events pinned or68 movable. Test two/four virtual CPUs and repeated runs with GCC/Clang target profiles. Reject the specific early-publication and omitted-durable-wake controls; a build failure, crash or timeout is not either demonstration.

Native actors are actually pinned to two CPUs selected from the process's allowed mask. Six trials alternate three pinned/movable pairs. Each completes four jobs,40 quanta,80 sweeps of32,768 words and checksum27,525,120. Record balance time, elapsed time, initial imbalance, migration count, and first/repeated sweep times under the identical work contract. Run both TSan builds plus debug/release/ASan+UBSan profiles. These durations include OS scheduling, virtualization and other effects: they do not isolate cache misses or guarantee a speedup. Physical cache warmup/PMU attribution remains unmeasured, and QEMU TSC values remain separate virtual observations. Do not replace that limitation with an invented cache result.

## Assessor rubric and pass/fail distinctions

For .a, level0 publishes a live context, level1 handles ordinary READY moves, level2 preserves effective/pending masks and actual saved-context authority, level3 derives the changed running-mask boundary and capacity refusal. For .b, level0 uses source-first lock order or copied CPU callback authority, level1 succeeds without rollback, level2 accounts for every reservation/notification/token lifetime, level3 reroutes a timer through TRANSFER and proves cancellation/reuse. For .c, level0 equates balanced lengths with speed, level1 reports a checksum/mean alone, level2 validates full ownership traces and equivalent work with observation scope, level3 pins the sensitive task, derives its admissible set and explains the measured/counterexample tradeoff. Passing a finite matrix is evidence, not absolute mastery.

## Hints and assessor separation

Hint1: separate requested mask, effective mask and current execution owner. Hint2: a stopped READY frame can wait at a disallowed home while dispatch is refused; determine how the bounded balancer repairs it. Hint3: preserve timer authority by task generation/lease, then resolve the home at admitted callback entry. The separate assessor implementation/analysis contains the completed mechanisms. Learner exports contain only public contracts, tests, tools and deliberately incomplete sources; completed prerequisite artifacts are supplied explicitly.

## Changed conditions, containment and validation status

Retain the original delayed variants: exclude a running task's CPU, migrate a task referenced by an old-CPU timer, and pin the latency-sensitive task while balancing others. These implementations provide assessable changed conditions; delayed unaided learner attempts and external human review remain unperformed.

Only original synthetic tasks/handles and private processes or disposable guests are targets. Defects are intentional original publication, rollback, old-route and pinning mutants. Harmless success is a named assertion or bounded wrong-state witness, followed by repair/regression; no third-party target, physical flashing, host security change or arbitrary-code-execution claim is involved. Current author execution is established only by the source-bound campaign and guard reports. Physical cache/timing, other ISA ports and independently assessed learner evidence remain distinct.

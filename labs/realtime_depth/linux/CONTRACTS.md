# Linux fair and real-time policy comparison

## Identity and exercise families

This required successor targets AX-PR-T52-12-T52.12.a/b/c, their original DELAYED variants and G-LCON-PR-T52-12. Families A/B/C/D/F/G/H/I/J apply through semantic derivation, an original bounded runner, invalid/denied admission, independent trace assessment, deliberate nontermination, observed resource distributions and changed CPU/interference assumptions. Do not turn a successful policy request or a small sampled latency into a hard application guarantee.

## Objectives, references and prerequisite artifacts

Read the registered primary kernel EEVDF/deadline documentation and Linux sched(7) passages in the successor resource register. Inspect the exact selected guest `kernel/sched/fair.c` eligibility/pick path and `deadline.c` depletion/parameter path, plus its configuration, rather than assuming every Linux release uses the same details. Require PR-T52-11 and PR-T49-09 artifacts, explicit time/overflow reasoning, process lifecycle cleanup, clock/error handling and ordinary original workload code. The finite snapshot selector is original course analysis of the stated rule, not a replacement for actual Linux execution.

## Environment and dependency profile

C17 hosted Linux; six GCC/Clang host profiles assess the pure policy arithmetic. Actual privileged class experiments run only inside an owned diskless/networkless QEMU guest using explicitly provided frozen kernel, configuration and static BusyBox. The selected image is7.0.14-cmastery-b06 with SMP, high-resolution timers and PREEMPT_DYNAMIC/LAZY; it is not PREEMPT_RT. No host scheduler, cgroup, security, resource-limit or boot setting changes occur. The public guest builder takes explicit completed probe inputs and never finds assessor implementations automatically.

## Concrete problem and supplied inputs

Implement the public unsolved `lc_select`, `lc_parameters` and guest probe. The teaching selector receives1..8 already-accounted `(signed lag, virtual deadline)` snapshots, admits lag>=0, chooses minimum deadline with stable index tie-breaking, and leaves selected output unchanged on invalid or no eligible entity. It deliberately excludes Linux buddy/slice protection, groups, placement, sleep decay and weighted-accounting updates. Compare the kernel's exact source branches before projecting this elementary rule onto an actual trace.

`lc_parameters(Q,D,P)` checks only mathematical0<Q<=D<=P in common integer units. Q3,D5,P10 gives utilization3/10; Q3,D2,P10 must fail before any policy syscall. Linux nanosecond min/max/flags, permission, root-domain affinity and admission constraints are additional predicates, not consequences of this test.

The actual equivalent-work probe launches20 absolute periodic jobs at10ms spacing with application deadline5ms. Each job performs exactly50,000 specified unsigned64 LCG updates from state1 using multiplier6364136223846793005 and increment1. The independent expected final state is13416517049422064561. Retain every release/start/finish/process-CPU observation and exact miss indicator; do not print inside the work loop. Requests cover OTHER, FIFO priority2, RR priority2 and DEADLINE runtime3ms/deadline5ms/period10ms. Read back actual class, flags, reservation, affinity and RR quantum.

## Necessary distinctions and counterexamples

An EEVDF virtual deadline controls fair-share ordering; it is not the job's5ms application deadline. FIFO has no round-robin quantum; RR rotates within equal static priority, and neither priority is an application deadline. A reservation parameter tuple can be mathematically valid and still be refused. The two-CPU guest deliberately attempts a DEADLINE mask containing only CPU1 and records EPERM under its wider root domain. A privilege-dropped original worker also records admission refusal. These cases remain unavailable experiments under those assumptions, not successful execution of the requested class.

## Ownership, failure and supervision reasoning

The supervisor owns each child PID until the exact waitpid result consumes it. It runs on guest CPU0; FIFO/RR workers and competing synthetic work use CPU1 in the two-CPU suite. DEADLINE keeps the full root-domain mask and a bounded reservation. Per-child CPU and RT CPU limits add containment; the supervisor has its own monotonic deadline and explicitly kills/reaps a deliberately infinite worker after250ms. Confirm actual kill and wait result, not just disappearance of output. One-CPU FIFO/RR infinite runs are refused because this placement would not protect the ordinary supervisor. A one-CPU DEADLINE experiment instead preserves reservation-based time for the supervisor.

All result storage is an inherited shared mapping. The parent reads the completed result only after reaping the worker. Background children have a readiness handshake, synthetic local storage and bounded CPU limits; they are terminated and reaped before return. Normal runs have a4-second supervisor bound; hitting it is failure, never timeout success. Output allocation/admission and syscall failures preserve explicit dispositions. Incomplete policy readback, cleanup or equivalent-work checks fail the exercise.

## Required implementation and measurements

Supply a separate original worker/probe and public builder. Record kernel image/config/source hashes, compiler/options, full command, CPU count, actual class/affinity, RT sysctl read-only observations and raw timing traces. Compare no interference, a same-CPU SCHED_BATCH CPU competitor, and same-CPU synthetic tmpfs read/write interference. Compare all-CPU and restricted ordinary affinity, plus the documented DEADLINE refusal and a separate one-CPU root domain guest. Do not silently compare unequal workloads or discard misses/outliers.

An explicit four-job overrun stress case consumes at least8ms of thread CPU per job under the3ms reservation and enables `SCHED_FLAG_DL_OVERRUN`. Record actual SIGXCPU overrun count, CPU/elapsed separation and misses. This intentionally changes the workload and is reported separately from the20-job checksum comparison. Signals plus the inspected depletion path support an observed reservation-overrun account; they do not measure physical WCET or enumerate every internal throttle interval.

## Verification and evidence-based acceptance

The independent Python oracle enumerates lag/deadline/parameter boundaries, predicts the checksum, checks every job index/release interval and miss indicator, verifies exact policy/parameter/affinity readback, and computes minimum/median/p95/maximum response from raw traces. It rejects a missing case, altered checksum, false policy, forged miss or denied mode labeled as available. Run all normal, refused, changed-interference, changed-CPU, overrun and infinite-worker cases. Mutation controls drop lag eligibility, admit Q>D and request OTHER while claiming a real-time mode. Public stubs must compile and fail the specific semantic gate. Compilation errors, kernel faults or outer QEMU timeouts do not satisfy negative tests.

## Per-objective rubric

For .a, level0 lacks a rule; level1 confuses virtual and application deadlines; level2 derives eligibility/selection and Q/D/P validity, with explicit kernel extras; level3 explains changed invalid Q>D and exact version-specific protection branches without overclaiming the miniature selector. For .b, level0 changes the host or lacks containment; level1 requests a class without readback; level2 executes bounded guest workers, retains refusals and proves cleanup; level3 deliberately runs the infinite worker and verifies the supervisor's placement, kill and exact reap. For .c, level0 asserts a guarantee from one number; level1 compares unmatched work or omits misses; level2 retains equivalent work and distributions with actual overrun/readback evidence; level3 changes affinity/CPU count/interference, predicts the changed assumptions and separates unavailable classes from observed ones. Independent learner transfer remains a separate assessment.

## Graduated hints

1. Mark fairness lag, virtual deadline, reservation deadline and application deadline as distinct quantities.
2. Compute the LCG result independently before selecting a scheduling class.
3. Fork while the parent remains ordinary; choose the child's affinity and class inside the guest, then verify actual readback.
4. The supervisor cannot rescue an unbounded higher-priority task if both monopolize its only CPU. Use the declared two-CPU placement or bounded DEADLINE reservation, and verify the child is actually reaped. Assessor implementation is withheld.

## Changed-condition variants

.a-DELAYED lowers D below Q and demonstrates pre-syscall rejection; it also explains why a valid tuple still needs Linux admission. .b-DELAYED makes an original worker loop forever under each admitted class and observes bounded supervisor kill/reap under the declared resource plan. .c-DELAYED changes ordinary affinity, switches CPU competitor to synthetic I/O, records the restricted-DEADLINE refusal and reruns DEADLINE in a separate one-CPU root domain. Attempt each independently after at least seven days; author execution is not a learner attempt.

## Security containment

Targets are only original synthetic processes in the dedicated owned guest; no credentials, external services or production data. The guest has no network or persistent disk and is discarded. The process marker and release check prevent accidental ordinary invocation, but are not a security boundary against a malicious privileged operator. Tools do not install kernels, change host policy or alter privileged host limits. A runaway guest still has an outer tool timeout, classified as failure rather than successful internal recovery.

## Validation status and limits

Authored executable successor with pilot observations; the final source-bound campaign determines which exact profiles and cases were executed. Physical timing/energy/WCET, independent human review and unaided delayed learner mastery are not asserted. The frozen debug/KASAN guest and TCG have instrumentation and virtualization costs; distributions characterize this workload/environment. The elementary selector does not simulate the whole current Linux fair scheduler. Successful request/readback and bounded observations are evidence, not a proof of all possible workloads or hard physical deadlines.

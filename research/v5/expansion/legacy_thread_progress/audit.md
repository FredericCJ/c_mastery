# T25.10–.12 required author implementation

Read both original project briefs and all nine criterion/changed-condition
objects before implementation. Scope is PR-T25-04 (progress/locality) and
PR-T25-05 (concurrency validation), retaining their exact original IDs and
minimum delayed interval. LRUN-17 is not closed by this inventory alone; its
completion envelope requires current campaign, export and negative guards.

Required mechanisms are four distinct schedule hazards and lock-order repair;
transitive priority donation with withdrawal; exclusion versus fairness and
conditional deadline bounds; whole-path effect budgeting and nonsleepable
admission; actual adjacent/separated address and timing probes with changed
line/alignment assumptions; private/batched/per-item aggregation and a current
reader contract; empty/useful/composed cost experiments at changed intensity;
actual prior queue forced schedules, fixed-seed stress and race sensitivity;
bounded history checking with explicit pending-operation extension policy;
and queue ordering/lifetime minimization plus changed shutdown policy.

Sources inspected so far: POSIX.1-2017 reprint
pthread_mutexattr_getprotocol DESCRIPTION31–108 and ERRORS117–129, including
recursive priority inheritance; Linux sched(7) policy/API sections86–182 and
SCHED_OTHER273–288 (not an assertion about this machine's scheduler internals);
Linux man-pages clock_gettime(3), release5.07, monotonic clock91–117 and return
value161–165. Herlihy/Wing1990 pages468–470 were directly read after the initial
web retrieval; exact history/completion passages and private hash are registered
in sources.md. The primary false-sharing article was directly read too.
Search results alone are not source inspection.

No host scheduling privileges, policy or security settings will be changed.
Bounds are conditional mathematical models, not measurements promoted to
worst-case response guarantees. Actual host timings remain environment-bound.
Existing frozen waiting/atomics producers will be consumed without mutation.

New code supplies donation with an independent160000-case reachability oracle,
conditional checked bounds/effect rejection, actual layout/aggregation/cost
probes and current mutex snapshots, finite FIFO history search with an
independent permutation oracle, and a real TWQueue settle-then-discard adapter.
Actual queue runs combine forced enrollment, fixed-seed yields, exact-once
accounting and recorded histories. Each mechanism has an intended semantic
mutant and compiled learner rejection. External evidence is not inferred from
author execution. The known race is isolated in an original teaching process.

# Assessor analysis: explicit jobs and scheduling premises

Use `jobs.c` only as the private reference; public jobs.c is deliberately
unsolved. Existing criterion IDs and original assessment bodies are retained.
The public contract supplies the14 required fields and objective-specific
rubric. Inspect each row separately, including the original delayed variant.

The raw CMX ordering producer is reused, not reimplemented. Prevalidation and
a temporary order prevent its prefix publication from leaking on a later
invalid task. The original simulator is unsuitable for D>T backlog: its
remaining/deadline arrays have one element per task and a release overwrites
them. The successor ledger instead has one element per input identity and
never reuses it. A bounded ready set and bounded historical ledger are
different resources; completed records do not consume active capacity.

At each boundary, checking deadlines before admitting new jobs prevents both
replacement and same-time completion ambiguity. Service over[t-1,t) has
already completed at t. Deadline t still READY means a miss; deadline at the
finite horizon is checked even though a release there is not admitted. Zero
cost completes without using a ready slot. A missed job remains serviceable;
a rejected or dropped job instead has a terminal policy result. These states
cannot be conflated in either safety or feasibility claims.

Fixed priority selection ranks tasks; EDF ranks absolute deadlines. Stable
task ties are producer order; ready ties are release then input index. Input
sequence identity is not a temporal ordering: supplied jitter can reorder
releases. The random oracle uses independent sorted ready lists and event
records, whereas C scans a fixed ledger; both must implement the published
boundary semantics. Neither is a proof for arbitrary real task arrivals.

T52.01.a: compare the exact sufficient-bound inequality after clearing the
common denominator. Raising positive sides to integer n preserves order.
LCM/power exhaustion is a representation refusal. The harmonic2/4+4/8 set
violates the sufficient bound yet has fixed-priority responses2 and8. It is
incorrect to call sufficient-test failure infeasibility. A zero-jitter bound
cannot be silently reused once delayed releases are admitted. The inspected
book passages establish the stated simple assumptions; the separate RTA
contract explicitly states its more general arrival-bound premise.

T52.01.b/c: DM order C,A,B with costs1,2,3 gives response1,3,6. Bcost4 gives
first exceeded7 againstD6. RM starts B, delaying both A and C beyond their
short relative deadlines. The public test generates two releases/task and
checks those outcomes, not only the order array. Transactional invalid input
checks the entire published result representation initialized to a sentinel.
Every scalar/fixed array result has defined initialization; no uninitialized
padding is interpreted as semantic data by the oracle.

T52.02.a/b: the A,B,C EDF trace is0,1,2,0,0. The constrained demand example
has demand2 at2 and4 at3; a horizon2 successful check is inconclusive beyond2.
The finite demand oracle separately enumerates releases and compares the
floor count. The all-interval processor-demand theorem is not reduced to
testing the selected finite horizon. Multiple C4/T2/D6 jobs finish at4 and8;
the third has2 remaining at10 and misses. With one ready slot, rejection
retains job0, rejects job1 and admits job2 after completion; drop policy instead
records the two interrupted histories. A protected nonpreemptible job is not
evicted; this is a declared policy, not a generic optimal overload theorem.

T52.01.c/02.c variants: low service3 at0 followed by highC1/D1 at1 is feasible
with free preemption. A nonpreemptible low section leaves two units of blocking
at arrival; high response3 exceedsD1. Charging one preemption tick without the
section yields completion3 rather than2. Overhead is charged on switching
away from unfinished work only. During the charge, releases/misses still
occur; selection is recomputed afterward. Dropping a running job is a policy
termination and is not counted as a normal preemption. These assumptions must
be matched before comparing implementation costs or using a response bound.

Finite complexity is O(n²+h*k+k²), storage O(n+k+h), no allocation. The
strict target audit permits only course byte helpers. Exact integer power
uses checked doubling/addition, avoiding a hidden uint64 divide helper.
Generated stack and text sizes include the selected harness/imports and
must not be presented as isolated abstraction overhead or physical WCET.

The11 mutant controls each falsify a different claim listed in the public
contract; compile failure/signal/sanitizer failure is never their expected
result. Additional controls replace each completed import and the learner
mechanism with its unsolved scaffold. Their successful compilation plus
named semantic assertion failure demonstrates that the tests reach actual
required behavior. A passing reference and failing mutants do not prove all
defects detected. No human independence or learner completion is inferred.

The first campaign retained150 passing host outcomes but failed the M4 build
on an enum comparison that was always false with the target's enum range.
Unsigned range checks retain invalid-enum rejection on both targets. A later
pilot found that the original byte helper did not export memcmp; the target
test harness now compares its own sentinel bytes explicitly. An incorrectly
typed pilot source directory also exposed a reused-object risk: target source
and linker paths must now be real files, old object outputs are removed before
compilation, and each new object must actually exist. That pilot is not used
as campaign evidence. Final campaigns use fresh private build directories.

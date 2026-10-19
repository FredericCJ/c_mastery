# T25.03–T25.05 source and depth audit

Inspection date:2026-10-17. Read the actual PR-T25-02 brief, its nine original
criterion and delayed-task objects, the capacity-three ring fixture, the
retained hosted queue implementation/header, and the relevant parent teaching
and concurrent-system contracts. The accepted `hs_queue` has capacity4 and
while-predicate waits/close-drain, but no deadline, cancellation cleanup,
admission-root retirement, explicit phase break or per-leaf changed experiment.
Its implementation is retained; the successor does not rewrite its historical
acceptance. New code uses capacity3 to execute the original mathematical fixture
and records the distinct API/status contract.

Primary passages actually read through direct page opens:

* POSIX.1-2017 reprint `pthread_cond_wait(3p)`: DESCRIPTION lines32–120,
  ERRORS150–167, RATIONALE193–217,219–237,244–285,325–357. The predicate and
  associated mutex govern progress; return alone grants no item. The
  cancellation and absolute-time passages support the narrow deferred-wait
  cleanup and clock contract. This page became accessible in this wave; earlier
  unavailable records remain historical.
* POSIX.1-2017 reprint `pthread_condattr_getclock(3p)`: DESCRIPTION32–47,
  RETURN VALUE49–57 and ERRORS60–65. The configured condition clock determines
  how the absolute time is interpreted. The selected experiment uses monotonic
  time; no wall-clock changes are made.
* POSIX.1-2017 reprint `pthread_mutex_destroy(3p)`: DESCRIPTION32–70 and
  RATIONALE106–183. A destroy call is not a safe means of invalidating active
  users. The root-lifetime protocol avoids those states rather than expecting
  a diagnostic for undefined use.
* Linux man-pages `pthread_setcancelstate(3)`: DESCRIPTION29–68,
  ATTRIBUTES89–96, NOTES110–148. Only the selected compute region permits
  asynchronous cancellation. Unsafe library work is masked; cleanup cannot
  generally fix interruption inside a resource operation.
* GNU libc manual36.2.5 POSIX Barriers: complete short section, including
  count, repeated batches, return distinction and destruction preconditions.
  The original custom barrier adds a documented broken state; this is not a
  claim that `pthread_barrier_wait` itself supplies failure detection.
* POSIX.1-2017 reprint `pthread_once(3p)`: DESCRIPTION30–50 and APPLICATION
  USAGE71–77. Completed initialization is not an application success status;
  the reference callback and failure-policy comparison remain distinct.
* POSIX.1-2017 reprint `pthread_rwlock_rdlock(3p)`: DESCRIPTION30–81 and
  ERRORS94–112. Scheduling-option and preference qualifications matter. The
  starvation models declare their own policies rather than silently treating
  every host rwlock as reader-preference.
* POSIX.1-2017 reprint `sem_trywait(3p)`: DESCRIPTION29–45, RETURN VALUE48–53
  and ERRORS56–68. Permit counting does not identify token ownership, participant
  arrival or a phase. `sem_wait(3p)` is only a redirect and is not independently
  credited as a substantive inspection.

These reprints identify IEEE/The Open Group2017 and Issue7/2018; formatting may
differ from the referee publication. `pthread_condattr_setclock.3.html` and the
GNU `Condition-Variables.html` URL were inaccessible; no inspection is claimed
for them. Exact published C17/C23 and current POSIX.1-2024 comparison remain
outside this source evidence. The active language baseline stays C17.

Author evidence must bind actual compiler/runtime commands. Finite schedules,
known-race detector sensitivity, models and source arguments each establish
different properties. No source list, matching fixture, compiler success or
reference test awards independent learner mastery.

The first source-bound campaign stopped after293 successful command checks
because the disassembly audit expected only the PLT spelling and encountered
Clang AddressSanitizer's `__interceptor_trampoline_pthread_setcancelstate`.
`attempt1_validation.json` preserves the original failure and scoped hashes;
its old build paths may be superseded by later runs and are not current evidence.
The retained disassembly copy is under `build/thread-waiting-pilot/`.
Read LLVM21.1.0 `sanitizer_common_interceptors.inc` lines6381–6400 directly:
the interceptor forwards to the real operation and checks the returned old
state. This is a bounded implementation comparison, not proof of all runtime
async-safety. The readiness publication now follows the state-change return,
and the sole controller cancels only after observing it. The audit retains
the exact selected symbol instead of silently normalizing arbitrary calls.

The second campaign reached336 successful command checks but the final Clang
ThreadSanitizer call audit rejected remaining function-entry/atomic/fence
instrumentation. `attempt2_validation.json` retains that failure. Read the
official Clang ThreadSanitizer documentation sections on `no_sanitize("thread")`
and `disable_sanitizer_instrumentation` (lines215–220): the former suppresses
ordinary access instrumentation only, whereas the latter suppresses all of it
for the selected function, with diagnostic limitations. The corrected compute
region uses the stronger attribute only under Clang and retains the full
generated-call audit. The first two reports are failed authoring attempts,
not accepted/current validation receipts.

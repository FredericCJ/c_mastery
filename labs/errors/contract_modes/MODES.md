# Diagnostic modes cannot disable memory admission

Required X-T65-008/R1-R4 and X-T65-008-DELAYED, after X-T65-007. Reuse actual
er_contract_counter. For the delayed threaded stage complete PR-T25-01 thread
creation/join and mutex ownership reasoning. Hosted C17 with actual C11 threads
and a separately selected ECM_POSIX_THREADS backend; neither supplies a new
first-principles mutex algorithm. The studied mechanism is the boundary between
diagnostics, memory admission and concurrent ownership of reports.

## Original policy and the changed operation

The original counter is initialized0..100 and amount is-100..100. OFF safely
clamps; OBSERVE reports and clamps; ENFORCE reports and refuses a violated
candidate. Report exhaustion refuses before changing counter or reports. A
mode switch does not make an otherwise invalid C operation legal. Retain this
total clamped model so its violated predicate can be observed without executing
undefined behavior.

EcmBuffer borrows length0..8 bytes; NULL is allowed only at length0. init takes
fresh descriptor storage and a live suitably sized byte object, with no allocation
or ownership transfer. Descriptor, data and outputs are disjoint and remain live.
Its reports field is size_t and may start at SIZE_MAX. In the serialized variant,
all callers and other accesses to the byte object are externally serialized.

ecm_write checks mode before anything else. Invalid mode/null control returns
INVALID unchanged. If index<length, all modes write exactly one byte and return
OK, even when reports is exhausted. The internal unchecked_write primitive has
the real precondition that index names an element of the supplied byte object.
If index>=length, every mode refuses without calling it. OFF returns CONTRACT
without a report; OBSERVE/ENFORCE increment reports then return CONTRACT, except
SIZE_MAX returns RANGE before changing reports. No clamping of the address is
allowed, and NULL+0 must not cause pointer arithmetic. This stronger admission
rule applies even when assert and diagnostic reporting are disabled.

## Concurrent report ownership and lifetime

EcmShared owns one nonrecursive mutex and borrows the same byte object. Fresh init
validates counter/length/pointers, creates the mutex and then initializes state.
If creation fails, IO returns and no live shared object was published; do not
destroy or use it. A successfully initialized object is noncopied/nonrelocated.
All shared write, counter and snapshot operations acquire the same mutex. Counter
calls the actual er_contract_counter with the same report count used by byte
admission. Snapshot stages counter/reports/length and all live bytes under that
lock, unlocks, then publishes to a caller-owned disjoint output. No raw data or
control access is allowed concurrently outside these operations.

Lock acquisition failure returns IO before effect. Unexpected unlock failure is
a fatal local-process policy, because state may already have committed; the adapter
calls abort instead of falsely claiming rollback. No robust-owner recovery,
asynchronous cancellation, reentry, signal use or mutex-failure injection is
claimed. Destroy is a quiescent caller action after all threads have joined and
no operation can start; it is not a concurrent close API. C types do not enforce
these ownership/lifetime premises. Live mutex byte-copy is forbidden.

The default backend uses mtx_init/lock/unlock/destroy and thrd_create/join. The
explicit POSIX variant changes both mutex and launcher APIs, preserving the
serialized state contract; neither mutex type is cast to the other. A four-worker
case makes1000 invalid byte attempts and1000 violating counter attempts per
worker, resulting in exactly8000shared reports, counter100 and unchanged byte7.
A subsequent fresh instance begins at SIZE_MAX-1: byte refusal consumes the last
report, and the counter refusal preserves90 rather than wrapping the count.

## Assert and evidence boundaries

Read N1570 draft7.2/7.2.1.1 and7.26.4.1-.3/.6, plus the inspected GNU mutex
documentation. Separate actual installed library behavior from draft authority.
Compile the original assert(++essential) probe with/without NDEBUG: the enabled
program observes1; disabled observes0. Essential updates do not belong in assert.
Main optimized builds define NDEBUG and still preserve memory admission because
the wrapper uses explicit checks. C11 threads remain optional in C17; this host
provides them. Exact published C17/C23 remain access gaps.

Ordinary/sanitized C11 tests, POSIX tests and TSan tests are separate. The installed
C11-thread TSan run aborts in __tsan_func_entry with a null+0x18 access; an original
minimal C11 mutex plus one protected write reproduces it. The same minimal program
runs normally without TSan. Retain both failed logs and the source-bound control;
this is not a clean C11 TSan result. The explicit POSIX backend is actually run
with both compiler TSan runtimes. Never disable host security or suppress findings
to manufacture a pass. Absence of a reported data race is not an all-schedules proof.

## Inputs, verification and assessment

D1 derives mode/report/effect tables and contrasts an ordinary violated predicate
with a bounds precondition. D2 implements the exact API and shared ownership,
including report exhaustion and init/destroy lifetime. D3 exhausts all101legal
counters,201amounts,3modes and report0/MAX-1/MAX:182709ordinary cases. Additional
invalid boundaries bring counter cases to182869;1188byte cases cover lengths0..8,
indices0..10, modes and report boundaries against an independent byte-array model.
D4 supplies joined real threads, exact shared counts, assert probes, backend
comparison and a sealed synchronization contract for the delayed exercise.

Five anchors cover the retained counter, bounds, exhaustion, admission and actual
concurrency. Seven compiled controls bypass OFF bounds, wrap report count, omit
reporting, reject valid writes, discard shared counter reports, or compile actual
ER_MUTANT15/16. All controls must reject through ER_CHECK after successful build;
no crash/sanitizer error counts as the intended semantic rejection. Compiling
learner stubs must fail every selected anchor.

R1 distinguishes diagnostics from language rules; R2 verifies commit/report order;
R3 demands independent exhaustive integer cases and actual NDEBUG probes; R4
requires safe memory admission in every mode and a new shared report owner. Nominal
reproduction scores1, complete bounded explanation/evidence2, independent changed
conditions with a counterexample3. Any invalid memory access, report wrap, hidden
concurrent raw access or claim that OFF removes C preconditions fails. Hints: admit
the address before the primitive, identify the report owner, retain exact failure
effects, and wait for users before destroying the lock. Separate assessor analysis
and delayed seven-day learner assessment are not replaced by author execution.

## Security laboratory

Target: only this original local byte-view/counter program, synthetic zero bytes,
no sockets or deployed objects. Attacker chooses mode/index/value or amount20
at counter90, not pointers, allocations, mutex state or host configuration.
Defect: confusing OFF reporting with permission to violate bounds, or committing
an enforced counter change. In the harmless compiled byte control, a real8byte
object advertises only3bytes; index5 writes a sentinel outside that declared view
but stays inside the actual object. Observing byte5=99 is the exact confined
property violation. This does not demonstrate arbitrary code execution or require
out-of-object memory access. The repaired wrapper keeps byte5unchanged in all
modes; regressions also test valid writes, empty view, SIZE_MAX index and exhausted
report state. Residual limits: dishonest extent/pointer, corrupted descriptor,
unjoined lifetime and unsynchronized external data access remain caller violations.

## Cost review (thirteen dimensions)

1. Representation: record buffer/shared/snapshot sizes and mutex backend separately.
2. Work: constant bounds/mode checks; snapshot copies at most8bytes.
3. Space: caller-owned descriptor, bytes and mutex; no growing report log.
4. Allocation: no mechanism heap allocation; thread/mutex/runtime internals separate.
5. Traffic: one admitted byte write or one report increment; snapshot reads under lock.
6. Dispatch: direct calls to actual er_contract_counter and selected mutex backend.
7. Code size: preserve emitted objects and linked executable observations.
8. Stack: retain compiler stack-use files; do not infer library stack maxima.
9. Timing: seven10000-run pure bounds workloads, checksum560000; do not compare
   that with threaded throughput as if contracts were equal.
10. Failure: bounds/report refusals, init/lock errors and fatal unlock policy distinct.
11. Scheduling: blocking mutex, no fairness or lock-free guarantee; joined destruction.
12. Builds: clean/incremental and actual include/link/runtime inputs retained.
13. Portability: C11 optional support, native64bit size_t oracle, POSIX variant and
   TSan runtime boundary separately observed; changed widths require new oracle bounds.

No physical evidence or independent human/learner assessment is claimed. Finite
execution and sanitizer observations support the bounded contract, not absolute mastery.

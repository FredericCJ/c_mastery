# Guarded cells, reentrant callbacks and actual concurrent callers

Required X-T66-005/R1-R4 and X-T66-005-DELAYED after X-T66-004. Compose actual
ow_borrow/ow_release and ow_cell_read/write/replace from the earlier C17 family.
The new OcgCell uses a POSIX pthread mutex around both the guard book and value.
It is hosted C17 on Linux, not a freestanding or lock-free implementation.
Python/diagnostic runtimes are host verification tools. No library implements
the studied borrowing or callback-publication protocol.

## Representation, operations and premises

OcgCell contains a fixed-address mutex, actual OwCell with extent1, ready,
closed and updating flags. OcgToken couples its exact live cell address with an
OwToken. Four slots retain the original finite serial and retirement policy.
Init receives fresh uninitialized storage or storage after successful destroy;
never copy or relocate an initialized mutex. It returns the actual pthread init
error code without pretending every failure is allocation exhaustion. Only
successful init permits other operations. Keep the cell alive until all callers
have returned, all guards have released and destruction succeeds.

All public operations serialize their guard/value checks with the same mutex.
Acquire returns a whole-cell reader or writer and preserves token output on
failure. Read/write validate the exact cell identity and original range/serial
checks. Read output is separate from cell, control, token and callback storage.
Writer-only mutation and replacement retain actual OwCell semantics. Replacement
with any active guard returns BUSY without changing value or old-output sentinel.
Release remains legal after close and at the last serial. Copying an existing
token does not create another permission; exactly one logical owner releases it.

Close is idempotent and stops new acquisitions, replacements and callback updates.
It does not revoke existing guards: their reads/writes/releases remain admitted
until release. This is draining, not asynchronous destruction. Destroy requires
external quiescence: no other caller is active, waiting to enter or able to start.
Join worker threads before calling it. With live logical guards it returns BUSY;
otherwise it closes and destroys the mutex, clearing ready only on success.
The no-guards observation alone is not permission to destroy a mutex that another
thread may be using. Reinitialization begins a new lifetime after all old tokens
have been abandoned; tokens are not safe to test against a destroyed/reused cell.

The selected host contract assumes an initialized default nonrobust mutex and
correct ownership on lock/unlock. No owner death, pthread cancellation, signal
handler API use, nonlocal callback exit or mutex backend failure during an admitted
operation is supported. Callback contexts and disjoint outputs outlive the call.
These are stated limits, not silent recovery guarantees. An initializer error is
returned; an unsupported backend failure is not modeled as successful cleanup.

## Callback transaction and reentry policy

ocg_update first checks open state, admits a real exclusive whole-cell guard,
reads the prior value, and marks updating under the mutex. It then releases the
mutex while retaining that guard. Invoke the callback exactly once with the old
value by copy and a pointer to a private candidate initialized to old. The
callback may call acquire/read-through-an-existing-valid-token/replace/update/
close on the cell, but may not init, destroy, edit control fields, retain candidate
storage or exit nonlocally. There cannot already be another valid guard when the
writer was acquired. Nested acquire/replace/update therefore reject BUSY without
deadlocking or exposing staged data. This explicitly chooses rejection, not a
snapshot-read API. The callback's own external effects are not rolled back.

After callback return, reacquire the mutex. If close was selected first, return
DEAD and retain old value and caller output even if the callback accepted its
candidate. Otherwise nonzero callback status returns INVALID and preserves both.
On success write the candidate using the retained actual writer token, then
publish old output. Always release the internal guard and clear updating before
return. A concurrent close linearizes when it sets closed under the mutex; an
update which committed earlier is not retroactively undone. A callback that
never returns can retain the guard indefinitely; no bounded-latency claim.

## Concrete work and evidence

D1 derives permission versus const qualification, escaped-pointer limits and the
mutex/guard lifetime invariant. A const access path does not make all aliases
immutable. Analyze an API returning an unchecked persistent raw pointer as a
counterexample; do not create a native data race or double ownership as a golden
test. The inspected RefCell contract supplies a runtime borrowing comparison,
while its safe-language lifetimes/destructors differ from C caller obligations.
D2 implements the exact public API, explicit close/drain and callback transaction.
D3 retains original empty-range, stale, readonly and replacement checks, all24
four-reader release orders, cross-cell tokens and final serial release. D4 uses
actual concurrent callers and explains why atomicizing only value leaves an
unsynchronized registry and authority race.

Concrete callback case: value9, output71; callback tries a reader and replacement,
observes BUSY, stages12, then succeeds. Commit yields old9/value12. A second
callback stages15 but rejects: value12/output71 stay, while callback count rises.
If it closes before returning success, no candidate publishes and cleanup drains.
The asynchronous test pauses a real worker callback with a separate test condition,
closes the cell from the main thread, resumes and joins the worker, then verifies
DEAD, untouched output/value and safe destruction. Four actual workers each
complete1000increment callbacks; final value4000. BUSY retries are counted, and a
finite harness attempt bound is an observation limit rather than a fairness theorem.

The independent56-case table covers reader counts0..6, writer/read modes,
callback failure and callback-close choices. It independently predicts slot and
callback admission, output and close state. This sequential table is not a
concurrency model checker. Run seven anchors with GCC/Clang debug, release and
ASan/UBSan; separately run real pthread workloads under each compiler's TSan.
Record unavailable diagnostics as gaps, never silently count absence as success.
Six compiled controls release the internal writer too early, publish a rejected
candidate, ignore callback-close, ignore cell identity, or enable original
OW_MUTANT5/15. They must fail exit1 at OW_CHECK, not by sanitizer abort. Compiling
learner stubs must fail each selected anchor.

R1 assesses enforcement/lifetime reasoning; R2 whole-operation locking and
publication; R3 complete original boundaries and independent outcomes; R4 reentry
and actual threaded transfer. Nominal completion earns1; complete bounded
justified evidence2; an independently attempted changed policy with explicit
counterexample3. Publication after selected close, guard bypass, data race or
destroy-before-quiescence is a hard failure. Hints: retain permission across the
unlocked callback; protect the book and value together; separate close from
destruction; return candidate failure without undoing external callback effects.
The delayed variant must be attempted after seven days without the worked code.
Assessor analysis remains private.

## Scope and thirteen cost dimensions

All flawed programs are original, local, synthetic-value teaching targets.
No external service, credential, privilege or host security setting is involved.
A separate ordinary-cell test deliberately takes &cell.value, retains a reader,
and writes88 through the escaped pointer in one thread. The harmless observation
is a changed value despite readonly logical permission. This is defined C storage
access in an intentionally bypassed teaching API, not an admitted guard operation
or a native data race. Repair the wrapper by exposing guarded copies only and
restricting direct representation access; the regression documents the remaining
trusted-client boundary rather than claiming C can block arbitrary raw aliases.
The controls demonstrate permission/publication defects only. A race detector
finding would not establish arbitrary code execution; correct tested interleavings
do not prove all schedules. The chosen caller contract excludes asynchronous
thread termination and corrupted native pointers.

1. Representation: actual sizeof cell/token, mutex plus four guard records.
2. Work: fixed four-slot scans and one callback; callback cost separately bounded.
3. Space: candidate scalar and token, no recursive owning copy.
4. Allocation: no explicit core heap calls; pthread implementation resources remain
   a separate dependency, so zero source-level calls is not zero system allocation.
5. Traffic: mutex/registry/value traffic, not just one atomic scalar.
6. Dispatch: one admitted user callback; rejected operations call none.
7. Code size: object/disassembly and actual imported symbols per compiler.
8. Stack: emitted stack records plus callback's separately declared requirements.
9. Timing: seven10000update samples, checksum2449965000; thread contention is a
   separate workload and no universal latency/fairness conclusion follows.
10. Failure: guard conflicts, stale, callback refusal, selected close and init error
    differ; output publication occurs only on accepted commit.
11. Scheduling: blocking pthread mutex and callback-held logical exclusion; joins
    establish actual worker completion before reclamation.
12. Build: source/header/link closure, all mutation/stub controls, clean/incremental
    work and separately labeled TSan binaries.
13. Portability: C17 core plus selected POSIX host contract, finite uint32 serials;
    a new target/backend requires its own synchronization/lifetime evidence.

The identified POSIX.1-2017 specification reproductions are used with their exact
edition attribution. Current official2024 endpoints returned403, and published
C17 remains unavailable. Author validation is not independent learner mastery.

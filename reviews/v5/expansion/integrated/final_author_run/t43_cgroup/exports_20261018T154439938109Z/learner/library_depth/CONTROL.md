# T12.15: signals, nonlocal control and publication

Required AX-PR-T12-04-T12.15.a/b/c and unchanged seven-day DELAYED variants;
families A/B/C/D/F/G/I/J. C17 hosted reference, explicit POSIX backend and
separate strict Arm object/link probes. Prerequisites: object lifetime, function
pointers, checked ownership, thread joins and release/acquire reasoning. Begin
with the learner's PR-T12-04 notebook and implement control.c against control.h.
The reference/GenMC model/transfer analysis remain assessor-only. Read inspected
records R-LLIB-N1570-CONTROL, R-LLIB-N1570-THREADS,
R-LLIB-N1570-ATOMIC, R-LLIB-POSIX2024-CONTROL and R-LLIB-GENMC019.
The WG14 citation is a C11 draft, not an inspected published C17 standard.

For .15a write a handler whose only shared action assigns1 to a static volatile
sig_atomic_t flag. A single-thread caller clears it, saves the previous SIGTERM
disposition, installs the handler, calls raise, inspects the flag after return
and restores the disposition. Return FAILURE on installation, delivery,
observation or restoration failure. Restoration failure means the process must
not assume its earlier disposition is active. There is no allocation or handler
I/O. The handler is constant work; delivery cost is platform-dependent. Neither
volatile nor sig_atomic_t is a general inter-thread synchronization mechanism.
Explain why arbitrary printf/malloc calls in an asynchronous handler are not
justified by their ordinary-call contracts, including interruption of internal
library invariants. Keep I/O in ordinary control flow.

The changed task takes the allowed POSIX branch. Use an original child process
to send one SIGUSR1 to its parent. The parent blocks it before fork, installs
sigaction, then uses sigsuspend with a temporary unblocked mask until the flag
is set. Restore action/mask and reap the child. Explain the lost-wakeup window
in testing the flag and then calling a separate unmask-and-sleep sequence.
sigsuspend atomically installs the temporary mask and waits; interrupted return
restores the earlier mask. This is a POSIX rule, not an ISO signal guarantee.
Use no signal handler allocation, locks, stdio or external target. This bounded
single-thread process experiment does not change host-wide signal policy.

For .15b derive the setjmp return paths before coding. Its use must obey the
allowed expression contexts (a switch controlling expression is used here).
Zero means initial passage; longjmp(env,0) resumes as1; a nonzero value is
returned as supplied. The destination function must still be active in the
same thread, with the relevant variably modified scope still valid. State is
not rolled back to the setjmp moment. An automatic nonvolatile local in the
destination function that changed since setjmp is indeterminate after longjmp;
do not execute a read and assign it a portable expected value.

lg_jump receives a caller-owned lg_owner whose allocation starts null, mode0..3
and a writable result pointer. The owner lives outside the destination frame
and no aliases modify it concurrently. Modes0,1,3 acquire8 bytes, then respectively
return normally, jump with2, or jump with0. Mode2 injects unavailable allocation.
Track acquired/released counts, free any acquired block once on every rejoining
path and clear the pointer. A volatile local changes from0 to7 before work; its
observed result is7 even after the valid jumps. Null arguments, nonnull incoming
ownership or invalid mode are ARGUMENT without effects. Acquisition failure is
FAILURE with no live allocation; other successful paths return OK. Counters have
caller-guaranteed headroom. Wrapper storage/work are constant plus malloc/free;
allocator cost is not implied to be constant. This protocol is explicit cleanup,
not language-enforced RAII or exception unwinding.

Supply diagrams for the nonvolatile-local defect and repairs, and classify the
cross-thread, returned-function and exited-VLA-scope variants without executing
undefined cases. Volatile addresses one local-value rule, not target lifetime,
heap cleanup or cross-thread validity. A normal error return with one cleanup
block is a legitimate alternative design; show which interfaces must change.

For .15c represent a one-shot publication by atomic_int ready and a plain int
payload. Initialize before starting either worker; one producer writes payload
then stores1 with release. A receiver loads ready with acquire; on0 it returns
PENDING without reading payload or writing output, and on1 returns the payload.
No writer changes payload after publication, no reset occurs while workers are
active, and the object remains alive until every started worker is joined. Derive the
edge: payload write is sequenced before release, the observing acquire
synchronizes with that store, and payload read follows it. A relaxed flag does
not obtain this argument merely because an x86 run looked correct.

Run32 independent publications with the actual C threads API and separately
with pthreads. Check create/join failures, use the matching join family and do
not reuse/free state until every started worker has terminated. C thread return
codes and pthread error conventions differ. Inspect optional-header support
and record atomic_is_lock_free on the actual object; lock freedom is an
implementation property distinct from ordering. Polling is bounded in this
fixture and provides no scheduler fairness guarantee. O(1) state/operation cost
excludes scheduling and any platform helper implementation; total polling work
depends on progress.

The host verifier explores a finite RC11 publication model with atomic payload
so removing either edge exposes a defined stale-read safety counterexample.
Relate that model to the real plain payload: the correct edge proves its
happens-before relation; missing it in the real program permits a data race,
so do not execute that version and assign a stale-value golden. Record model
bounds, memory model, verifier version and counterexample reads. Model checking
does not by itself prove unbounded liveness or validate an unrelated compiler.

Compile the actual publication functions for Cortex-M0 and Cortex-M4 with
freestanding/no-builtin flags, disassemble, inspect undefined symbols and link
the final image without libraries/helpers. The selected M0 backend imports
atomic helpers; expected rejection must name those missing helpers, not a
missing linker. The selected M4 image has empty unresolved closure. This does
not prove interrupt safety, board timing or all instruction/ABI requirements.
No libatomic, libc, compiler-rt or libgcc is silently admitted into this probe.

The original RTOS transfer requires an explicit adapter proof. Given a scheduler
without C threads, define spawn failure atomicity, handle ownership, start
publication, join/quiescence, cancellation cleanup, interrupt restrictions,
atomic implementation and helper closure. Reject a port that returns from join
while the worker still accesses state, starts a worker before initializing its
argument, maps failed spawn to a valid handle, or substitutes volatile flags.
State which platform source/target tests would establish each premise. The host
experiment is not evidence that an unspecified RTOS already provides them.

Deliver contract/layer table, source, ownership/control-flow diagrams, positive
and intended rejection traces, three finite model results, object/link audits
and changed-condition analyses. Six host profiles, both actual thread APIs and
the isolated POSIX signal path must pass. Three compiled semantic mutants and
the unsolved scaffold must fail at their named checks. Rubric:0 invalid lifetime
or synchronization;1 reproduces a trace;2 derives every ordinary invariant and
demonstrates meaningful counterexamples;3 independently transfers to the original
changed premises after7days with precise missing guarantees. Hints: identify
the still-live owner after a jump; identify the exact read-from edge; inspect
the generated imports before calling an atomic operation freestanding.
Author, external review, delayed learner and physical evidence stay separate.

# Withheld signal, jump and publication analysis

Assess all six ordinary/changed bindings of T12.15 independently. A successful
thread test does not assess invalid nonlocal destinations; an assembly listing
does not prove signal safety. References, public prerequisites and environments
are specified in CONTROL.md and the exact source register.

The ISO-facing handler only assigns the permitted flag and the caller runs
single-threaded. Its saved disposition is restored after raise even if delivery
fails. A failed restore remains a reported failure with uncertain disposition.
The POSIX variant blocks before fork and uses sigsuspend; a child send arriving
before the wait becomes pending, avoiding the test-then-sleep race. Only the
original process/owned child are targeted. Reject handler printf, hidden locks,
an unbounded external signal source or claims about another C edition's atomic
signal permissions without that edition's inspected wording. Choosing the
explicitly authorized POSIX branch satisfies the original disjunctive variant.

In lg_jump the owner object is in the caller, so its mutations do not turn it
into the changed automatic local of the destination function. The pointer
parameter itself is unchanged. The local result is assigned anew on every
switch path before use; changed is volatile. Heap allocation survives longjmp
until explicit cleanup. Modes1/3 exercise distinct return values2/1 and mode2
has no allocation. The cleanup mutant is rejected by live-owner counters; the
test then frees its deliberately leaked block so the intended check, not an
unrelated sanitizer leak, determines rejection. The unsolved scaffold must
fail its contract check while still compiling.

Reasoning-only counterexamples: (1) change an ordinary int local after setjmp,
jump back and try to print it: the changed local is indeterminate, not guaranteed
to be either old or new; (2) save env in a helper that returns, then jump to it:
destination function no longer active; (3) another thread jumps to this env:
same-thread requirement violated; (4) leave the variably modified scope active
at save before jumping back: violates that lifetime premise. None receives an
execution golden. Making the int volatile fixes only case1. Error codes plus a
single cleanup path are acceptable when all transitive callees propagate them;
do not describe either design as automatic C exception unwinding.

For publication, let W be payload=37, S the release ready=1, L the acquire that
reads1, and R the payload read. W is sequenced before S, S synchronizes with L,
and L is sequenced before R; therefore W happens before R. Initialization precedes
thread start and no later write exists, so the plain payload is race-free under
these premises. A release flag with a relaxed receiver or acquire receiver with
a relaxed publisher lacks the cross-thread edge. The GenMC model makes payload
atomic to observe stale0 as a defined assertion failure; it is a deliberately
different diagnostic model, not a claim that undefined plain-C execution has
that golden. Require both missing-edge counterexamples and actual verifier
exit42/Safety violation/read-from-INIT evidence. Tool absence or compiler failure
does not count as a semantic counterexample.

Each C threads/pthread execution uses its matching create/join family and shared
object lifetime. Return of the consumer before ready is not permission to free
state while a producer still runs. Native x86 success cannot establish weak-order
correctness. The M0 target's actual load/store helper imports must cause the
no-helper link rejection; M4's actual final image has no unresolved imports.
This is a target/compiler closure comparison, not a hardware measurement or an
argument that all atomic_int operations are always lock-free.

Delayed assessor packet, at minimum7days: supply (A) an RTOS spawn that can
allocate then fail, (B) join that only observes an exit-request flag, (C) a
single-core interrupt context where a helper takes a task mutex, and (D) a
properly documented task-start/join implementation plus the M0 import listing.
Require a contract table marking A invalid until cleanup/failure ownership is
specified; B invalid until accesses have quiesced; C potentially deadlocking,
not rescued by single-core scheduling; D still needs permitted atomic helpers
or a separately proved algorithm/runtime profile. Ask for a fresh adversarial
timeline and explicit platform evidence requirements. These are concrete
reasoning cases; no unspecified RTOS implementation is falsely claimed tested.
Alternative valid ports may use a documented scheduler primitive that establishes
the required edge and cleanup, provided the mechanism, costs and interrupt
restrictions are derived rather than borrowed from pthread names.

Award2 only when ordinary construction, failures and proofs are all correct;
award3 only on the independent changed-condition reasoning and evidence after
the actual delay. Parallel authoring and a finite RC11 no-error report establish
neither independent learner mastery nor unbounded liveness.

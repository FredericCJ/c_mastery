# Guard lifetime and reclamation: source comparison

This required reasoning companion, `SE-EPOCH-ENFORCEMENT-01`, extends
`X-T55-003` and objectives T55.07.a–c. Families A, D, F, I and J apply.
Prerequisites are the epoch/hazard representations in
[the modern bridge contract](../modern_bridge/CONTRACTS.md), a completed
three-node retirement trace, and C17 lifetime and synchronization reasoning.
Submit a trace table, enforcement matrix and repaired interface contract.
No third-party implementation is a dependency of a C target.

In Crossbeam epoch 0.9.18, ordinary guards keep a participant pinned; nested
guards defer unpinning until the last ends. Deferred functions need not run
within any promised interval. Flushing exposes deferred work to collectors;
it is not a completion barrier. `repin` and `repin_after` take an exclusive
guard borrow. Unsafe deferred destruction still requires removal from shared
reachability and a suitable destruction context. An unprotected guard does
not pin and executes deferred work immediately. These are library contracts,
not universal properties of Rust or of epoch reclamation.

The inspected `Atomic::load` signature connects the returned `Shared` lifetime
to the guard reference. Its implementation performs the requested atomic load;
the lifetime does not choose the memory order. Raw pointer conversion and
unsafe dereference retain validity obligations. The C bridge's participant
token carries no equivalent language lifetime constraint. Its explicit checks
can reject stale logical identities; they cannot revoke arbitrary raw pointers.
Sources: R-GUARD-CROSSBEAM-0918 and R-GUARD-ATOMIC-0918 in the accompanying
research register. Published C17 remains an identified source gap.

For the following original histories, assume two participants, three typed
nodes, sequentially consistent publication, and no allocation after startup.
All reasoning uses logical identities: do not execute a dangling dereference.

1. A enters at epoch 1, observes node N; W removes N, retires it at 1, advances
   to 2, then tries to advance again. A remains active. Give the retained-node
   and progress conclusions. Identify which claim changes if A is paused forever.
2. A enters twice through two API handles, releases one, and still reads N
   through the other. A proposed C adapter stores only one Boolean `active`
   and clears it at every release. Identify the first lost invariant; specify
   either rejection of nested acquisition or checked nesting with overflow
   failure and same-participant ownership. Do not silently change the original
   bridge's rule that a second `enter` returns BUSY.
3. A obtains a guard-bound reference, then attempts exclusive repinning while
   the reference is used afterwards. Compare the safe-language diagnostic
   obligation with C programmer discipline. Repeat with a raw pointer copied
   into an unrelated object. A source-level lifetime annotation cannot repair
   the latter's violated unsafe precondition.
4. W removes N and schedules its destructor. Explain why `flush` followed by
   freeing the destructor's context is insufficient. Design an explicit
   acknowledgement contract including owner, cancellation, callback failure,
   retained context and quiescence. Separate resource completion from progress.
5. Replace normal pinning with an unprotected guard while A may still read N.
   Identify the violated premise and the harmless logical observation that
   detects premature reclamation. A single-threaded teardown variant must
   prove exclusion of concurrent access before using this special case.

For each history, list preconditions, state after every operation, forbidden
transition, enforcement actor (C/Rust diagnostics, library runtime check,
external proof or caller discipline), ownership and worst-case retained space.
Distinguish a bounded three-node FULL result from unbounded retirement growth.
Derive the O(participants) scan and account separately for callback costs.
Supply one plausible incorrect explanation and its shortest counterexample.

Pass requires all five mechanisms justified and no inference of eventual
destructor completion, safe raw-pointer use or universal lock freedom.
Superficial completion merely names guards; competent work derives the
histories; independent mastery supplies a coherent changed contract and finds
its new failure modes without the assessor analysis. Scores remain unassigned
until a real learner submission is assessed. Assessor reasoning and graduated
hints are packaged separately in `assessor/guard_comparison/ANALYSIS.md`.

After at least seven days, assess an unseen participant-migration variant:
one worker suspends a task and another resumes it. Specify which state is
thread-local versus task-owned and how the original guard contract must be
revisited. A second variant replaces no-fail destructors with an external
resource whose close reports an uncertain outcome. Preserve its ownership
until a declared recovery policy resolves it. These are required transfer
assessments, not claims that delayed attempts have occurred.

Validation is author source inspection and worked logical analysis. No Rust
compiler, Crossbeam execution, whole-collector proof or physical observation
is asserted. Local synthetic traces are the complete security scope; no
external target, actual memory corruption or operational resource is needed.

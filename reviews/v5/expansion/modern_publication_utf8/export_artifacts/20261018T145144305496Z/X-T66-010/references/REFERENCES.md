# Allocated reference graphs and last-owner races

This required X-T66-006 investigation supplements `../CONTRACTS.md`. Use hosted
ISO C17, GCC and Clang, actual POSIX threads, and the actual `OwShared`,
`OwAtomicShared` and typed `OwAllocator` producer. The public compilation stub is
unsolved. `tests/model.c` and `tests/fence.c` are diagnostic clients, not target
dependencies. Reference counting itself may not be delegated to a library.

## Representation and ownership

Construct two distinct allocated `Node` controls and two separately allocated
`int32_t` payloads containing 11 and 12. `OrcMemory.take` returns suitably aligned
fresh allocated storage of the requested size, or NULL; its matching release
consumes that allocation. The actual typed allocator supplies the two one-cell
payloads. Allocator contexts, controls, output and payloads are live and disjoint.
Callbacks return normally, do not reenter the operation or mutate its world,
and do not retain borrows beyond the operation. Allocation failure preserves the
entire caller report and releases every successful prior acquisition.

Each fresh control starts strong=1, weak=1. The latter is the implicit weak
reference that survives until value destruction finishes. Each outgoing edge
owns exactly one strong or explicit weak reference to its target. Two audit
observers additionally own one explicit weak each; these retain the controls,
not values. No raw pointer creates ownership. All graph edits are under external
exclusion, with no concurrent traversal or graph mutation in this adapter.

`orc_cycle(memory,cells,weak_back_edge,out)` accepts only 0 or 1 for the flag.
After the first node gets a strong edge to the second, the second gets either a
strong or weak back edge. Drop both initial external strong owners. For a strong
cycle, the observation is two retained values and two retained controls. For a
weak back edge, it is zero retained values and two retained controls. Never infer
control leakage merely because the audit observers intentionally keep them live.

For the strong cycle, consume the existing first-to-second strong edge under
exclusive graph maintenance: acquire its replacement weak first, publish the
edge kind, then drop the former strong. This breaks the cycle. It is not tracing
garbage collection or a generally concurrent graph transformation.

A value destructor releases its own payload and detaches its outgoing edge,
then enqueues that edge's release. The caller drains the fixed two-entry queue
only after the reference-count operation returns. Each of two destructors
enqueues at most one edge and the drain removes an item before processing it;
capacity is therefore sufficient. This avoids transitive callbacks reentering
the same control while its implicit weak release is pending. Finally drop the
two observer weaks. Successful output has destructions=2, reclamations=2,
queued_releases=2, and repair_edges=1 for the strong cycle or 0 for the weak one.
Both raw allocations and typed payloads have been released. No pointer escapes.

## Atomic race and publication

Initialize an actual allocated control and payload 41 before publishing them
through `pthread_create`. Transfer the original strong to a dropper, one explicit
weak to an upgrader, and retain a separate observer weak in the joining thread.
A mutex/condition gate releases both contenders together. A successful upgrade
owns a strong before reading 41 and must release it; DEAD permits no value access.
The upgrader always releases its own weak. After both joins, exactly one
destruction and zero reclamations have occurred, strong=0 and weak=1. The final
observer release reclaims the actual allocated control once. Run 256 rounds;
neither scheduling outcome is required to appear in any particular finite run.

Separately run four actual workers, each initially owning a strong, each making
5000 retain/drop pairs and then releasing its transferred owner. Preserve an
observer weak through all joins. Count overflow tests use explicitly constructed
exclusive counter fixtures at UINT_MAX; they do not fabricate billions of real
references or perform an invalid concurrent double drop.

Explain the release sequence and final acquire fence, not just atomicity of a
counter. A successful upgrade cannot increment zero; it either precedes the
last decrement in strong's modification order or observes terminal zero. A read
while owning a strong precedes its release decrement. The last decrement and
acquire fence must make prior released accesses happen before destruction.
Concurrent ordinary payload mutation still needs separate synchronization.

## Verification and diagnostic limits

The independent Python model enumerates 12,787 legal owned-reference histories
of up to seven operations and checks both implementations. Six C17 build
profiles run the six anchors, three compiled semantic controls, failure injection
at each raw/typed acquisition, and compiling learner-stub rejection. Diagnose
an implicit-weak leak, a resurrection defect, and a falsely strong back edge.
Only intended `OW_CHECK` rejection counts; a diagnostic abort is not a semantic
mutant pass. Record actual atomic size/alignment, lock-free status, object imports,
disassembly, stack data and seven cost samples. Lock-free status is local to the
measured object/platform, and contention latency is not bounded by these samples.

Run TSan separately. The inspected GCC15.2 and LLVM21.1.8 runtime source bodies
do not implement vector-clock synchronization for an atomic fence. On the
observed installed builds, the original last-owner race emits warnings, as does
the minimal release-RMW/acquire-fence control. Preserve those warnings. Compare
the explicitly stronger acq_rel RMW translation: this changes the memory-order
contract, leaves the canonical producer untouched, and is expected to make those
observed diagnostic histories quiet. Do not claim a clean original TSan run or
use a suppression to obtain one. Installed binary identity and these controlled
experiments are separate from inspecting an upstream version's source.

The GenMC RC11 companion extracts the exact atomic provider functions because
processing the whole producer hit a tool memory-intrinsic transformation abort.
The exact weak-CAS retry client did not finish in its bounded run. A separately
labeled strong-CAS comparison removes spurious failure, and checks the two-owner
release/fence and last-owner/upgrade clients, with relaxed-release and resurrection
counterexamples. This comparison does not establish exhaustive correctness or
liveness of arbitrary weak-CAS retries, heap reclamation, or every C execution.
The native allocated test and manual memory-order argument remain distinct.

## Deliverables, rubric and changed conditions

D1/R1: give the value/control ownership graph, zero/overflow transitions,
initialization and release/fence argument; compare safe-language `Rc`/`Arc` rules
with C runtime checks and programmer discipline. D2/R2: implement both retained
APIs and the allocated graph, including deferred edge cleanup and failure unwind.
D3/R3: supply oracle, profile, diagnostic and lowering evidence and explain each
tool's actual scope. D4/R4 and X-T66-006-DELAYED: independently rebuild the raced
upgrade and allocated-cycle experiments without the previous solution after the
required delay, changing the weak owner transfer or graph cleanup ordering.
Show why removing the observer or reentering a control invalidates the argument.
The 13-dimension modern cost contract applies; distinguish O(1) count operations
from retry contention, reachable destructor work and strong-cycle retention.

Superficial completion reproduces one counter trace. Understanding requires
exact ownership and failure accounting plus a justified ordering argument.
Independent mastery evidence requires an unaided changed-condition attempt and
counterexample, separately recorded; no author campaign supplies that evidence.
Hints: draw implicit and explicit weak edges separately; defer outgoing-edge
release until callbacks return; retain an observer through joins before inspecting
the control. The separate private analysis supplies the assessor derivation.

Use only these original local programs, synthetic values and local diagnostic
processes. No external target, credentials or host security changes. A leak or
sanitizer report does not demonstrate code execution. Author execution status is
in source-bound reports; published C17 text remains inaccessible. No physical
execution, independent human review or learner assessment is asserted.

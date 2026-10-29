# T18 callback effects, quiescence and interface evolution

Apply [the shared contract](CONTRACTS.md). This packet implements the existing
criteria T18.04.a, T18.08.a–c and T18.10.a–c, including their original delayed
variants. Families A/B/C/D/F/G/H/I/J apply where indicated below. Prerequisites
are the T18 ownership packet, the actual `struct api_registry` removal contract and
`struct f_async` submit/cancel/complete contract. Bring the learner implementations and
their lifetime dossiers; reference validation imports the explicitly identified
assessor providers. It is not evidence that a learner integrated those artifacts.

## Why cancellation is not destruction

A registration is a reachability edge from an executor to a context. Removing
the edge prevents some future starts; it does not unwind a callback that already
started. Draw these separate events: owner creation, borrow, registration,
callback start, removal request, callback return, acknowledgment, owner release.
An acknowledgment must define its scope. If a callback enqueues a child, a promise
about the parent alone says nothing about that child. A scoped shutdown can
cancel queued children and join running children, or let all finish; it cannot
silently discard their ownership obligations.

Use the existing four-slot serialized registry. A parent holds one claim on a
one-byte heap payload whose value is 73. During its callback it creates a child
registration holding a second claim and removes itself. The child reads 73 and
removes itself. The registry snapshots registrations at dispatch entry, so the
child runs on a later dispatch. Execute three histories: let the child run;
destroy the registry between dispatches; request destruction inside the parent.
Record callback entry/exit, drops and final reference count. The parent removal
must return `API_PENDING`; no drop may occur before its callback returns. After
the first history's initial dispatch, one reference remains. At scope quiescence,
both claims have been released exactly once. Use real allocation/free, but never
read freed storage to obtain a demonstration. A bookkeeping assertion must catch
premature release before any potentially invalid access.

The context control block itself remains alive throughout the outer registry
call. Calling `api_registry_init` on a live registry is outside this protocol.
Generation validation does not make dangling registry storage safe. This is a
serialized executor experiment; parallel cancellation needs synchronization and
an acknowledgment tied to actual concurrent dispatch, addressed in T51/T73.

## Version 1: synchronous borrowed output

Implement `ac_transform_v1`: for 0–8 input bytes, XOR each with `0x20` and call
the exact `ac_borrowed_fn` signature once before returning. The output is a local
eight-byte array; it is valid only during that callback. Null input is allowed
only for count zero. Callback and `written` are required. Return 0 on success,
1 on invalid input; failure calls no callback and preserves `*written`. On
success, publish the count after the callback. The callback must return normally
and must not retain the borrowed pointer. The caller must keep all argument
objects alive and provide nonconflicting writable output storage.

No allocation, blocking or synchronization is used by this core. That statement
does not apply transitively to arbitrary callbacks. Implement one callback that
allocates a log record, copies the payload, then frees it, and another that
records a bounded numeric checksum. Count actual allocator invocations. Derive
the call graph and stack bound, including the callback. Compile the bounded
recorder and core for Cortex-M4, inspect relocations, stack-use output, final
linked dependencies and disassembly. A closed dependency graph and finite loop
bound are necessary inputs to an ISR assessment, not evidence of ISR safety or
measured latency on a board. Foreign object/link audits are not execution.

## Version 2: explicit owned result and stable operation

`struct ac_operation` is caller-owned live control storage initialized once before use.
Do not copy it while active, reinitialize it while active, destroy it from a
callback, or call it concurrently. All operations are serialized. `struct ac_allocate`
must return exclusive suitably aligned storage or null; release is infallible,
nonreentrant and consumes that allocation. Its context outlives every box.

`ac_submit_v2` validates all arguments and reserves counter headroom before
allocating one `struct ac_box`. It copies and transforms 0–8 input bytes. Input storage
may be reused when submit returns. Allocation failure leaves the operation and
ticket unchanged and never calls the receiver. Busy means an operation or
terminal callback is still active. Generation/release counter exhaustion rejects
before allocation; no modular reuse is permitted. The ticket is installed,
callback/context stored, and ownership established before inline completion is
possible. Submit success means accepted work, not necessarily a completed effect.

The callback receives either `ACX_OK` plus exactly one owned box, or
`ACX_CANCELLED`/`ACX_INDETERMINATE` plus null. An owned box can be read until
`ac_box_drop`; it may be immediately dropped inside the callback or retained.
The operation must not touch it after transfer. A caller retaining it takes the
release obligation and must preserve the allocator context. Completion publishes
`F_DONE`, clears internal ownership and increments the provider's release count
before calling user code. The reporting guard rejects reentrant use. Context may
be reclaimed after the terminal callback returns; operation control must survive
until the outer submit/finish call returns. These are separate lifetimes.

| Event | Running | Cancel requested | Terminal |
|---|---|---|---|
| Deadline expiry / cancel request | Request cancellation; pending | Idempotent request | Reject |
| Effect commit | Mark committed once | May win before acknowledgment | Reject |
| Normal completion | Success with owned box | Success may win the race | Reject |
| Cancel acknowledgment | Reject without request | Cancelled if uncommitted; indeterminate if committed | Reject |

The commit is an original synthetic effect marker, not external I/O. A cancel
request after commit reports indeterminate but still retains context until a
terminal event. A deadline is another request trigger, not a cancellation
acknowledgment. A late notification is rejected only while live control storage
still exists. Never send a late notification through a freed pointer.

Enumerate every event string of length 0–5 over C(cancel), T(timeout request),
K(commit), S(success completion), A(cancel acknowledgment): 3,906 histories.
Derive a separate state-transition oracle, compare every status and final state,
and check terminal call count, release count, allocations and frees. Drain any
unfinished operation afterward. This bound covers many duplicate/race orderings;
it is not a concurrency model or proof over unbounded histories. Separately test
stale tickets after reuse, busy admission, both counter limits, both completion
timings, allocation failure and immediate box release under sanitizers.

## Migration is a contract change

Keep `evolution_v1.h` independently installable. Version 2 adds new symbols and
retains the old synchronous implementation. An adapter that submits asynchronous
work and immediately returns cannot satisfy the old callback deadline; one that
allocates or blocks can violate the old effect contract. Preserve the old core
as the compatibility path. Source compatibility and successful linking alone
establish neither temporal nor ownership compatibility.

Compile `evolution_consumer.c` from a separate directory using only the installed
v1 header. Link its unchanged object against v1 and v2 archives, then relocate
the install prefix and link again. Test empty success, ordinary success and
failure preserving `written=71`. Change only failure initialization to zero in
a deliberately incompatible implementation: the old consumer must reject it.
The separate names `ac_transform_v1` and `ac_submit_v2` are the supported version
boundary. This package supports both for its entire edition; removal requires a
new contract/edition and explicit migration, not a silent implementation change.
No `v3` compatibility is promised.

Write an executable migrating client with stable receiver storage, allocation
failure handling, cancel-and-ack handling, success consumption and exactly one
release. Cache an owned snapshot across source mutation, and demonstrate that it
remains unchanged. A cached v1 borrowed pointer is an invalid design: diagnose
its lifetime from the timeline without dereferencing it after callback return.

## Evidence and assessment

Deliver the two implementations, consumer, descendant ownership trace, state
oracle, compatibility/effect matrices, allocator ledger, source/target audit,
and a falsification report. Run GCC/Clang O0/O2 and ASan/UBSan; predict before
running. All successful ordinary/changed relations above are required. Selected
semantic controls must compile and then reject early cancel-as-ack, rollback
after commit, failure-output mutation, early parent drop and leaked descendant
drop. Unsatisfied learner stubs must fail their checker without being replaced
by hidden reference code.

Rubric per mapped criterion: 0 incorrect/missing; 1 nominal output with omitted
lifetime/effect obligations; 2 correct implementation, causal argument and
discriminating evidence for the ordinary case; 3 independently derives and
implements the changed case, identifies enforcement boundaries and rejects an
overstated guarantee. A matching trace with a false lifetime argument fails.
After at least seven days, administer the original changed tasks with assessor
material withheld: descendant scope, strict target audit, inline completion,
postcommit cancellation, changed failure initialization, prefix relocation and
mutation-safe caching. Record real learner evidence separately.

Hints: (1) mark the earliest callback and latest legal use of each object;
(2) transfer each release obligation at one named event;
(3) publish terminal state before user code and preserve operation control
through the outer call. A boolean `cancelled` cannot express all the states.

Validation status is in the source-bound receipt linked by the shared contract.
This is original local synthetic work; no external service, credentials, host
security change, firmware flashing or arbitrary-code-execution claim is involved.

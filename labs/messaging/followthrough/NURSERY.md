# Nested nursery with real hosted imports

This required successor belongs to X-T73-001-R2/R4 and its delayed variant. It
imports the actual `struct msg_nursery` implementation plus `work.pool` (`hs_pool_sum`,
PR-T25-01/LAB-HS-10) and `io.deadline` (`hs_read_until`, PR-T24-05/LAB-HS-03).
The worker producer is a finite integer reducer, not a general task-submission
API. Each leaf may call it and returns only after its native workers have joined.
Testing-fault schedules here are explicit adapter inputs; no nonexistent fault
injection library is claimed. The original eight-child nursery remains required.

## Representation, ownership and operations

Use caller-owned stable `struct mfn_tree` storage, two group threads and two leaf slots per
group, an internal two-byte completion pipe, four task descriptors and a bounded
four-error result. `mfn_start` needs an uninitialized or fully finished tree, four
non-null functions and failure indices 0..2. Output storage, descriptor arrays,
callbacks' contexts and result storage must not overlap the tree or each other
in a way that modifies still-live input. Tasks and contexts must not access the
control representation except through documented operations. All callbacks
return normally; no asynchronous thread cancellation or nonlocal exits.

Invalid input or failure to create the internal close-on-exec pipe leaves the
tree and all context ownership unchanged. Once pipe setup succeeds, start returns
OK and takes all four context lifetimes, including contexts whose threads later
fail admission. A non-null drop callback receives its context exactly once after
every admitted descendant has joined. A null drop means the context is borrowed
through successful wait; the caller still may not release it earlier. The root
owner alone calls start/wait. A published active tree permits concurrent cancel;
it must remain alive until cancel callers also quiesce. No callback reenters wait,
start or destruction. Drops may not retain borrowed control/context storage.

Initialize both descendant cancellation flags before publishing any thread.
An explicit cancellation, any failed admission, or a nonzero leaf result requests
cancellation for both groups and the root. Cancellation does not prevent already
admitted leaves from running their callback; they observe their own descendant
flag and choose checkpoints. The root never detaches or forcibly terminates a
thread. It joins group threads first; a group joins its own descendants. The
root retries any unfinished descendant joins only after the group is joined,
so two threads never join the same thread concurrently.

`mfn_wait` borrows a valid cancellation pipe read end and an absolute
CLOCK_MONOTONIC nanosecond deadline. It uses the actual deadline-reader contract:
an already-expired deadline is observed before polling; otherwise cancellation
wins simultaneous descriptor readiness. A timeout or cancel request propagates
downward, then wait still joins upward. Thus a deadline bounds the start of
cancellation observation under the OS scheduling assumptions; it does **not**
bound return from a noncooperative child. A selected I/O error is separate from
deadline/cancel. The bounded internal writes cannot fill the pipe. Linux process
descriptor-table ownership excludes another thread closing or replacing these
private descriptors. There are no external network or privileged operations.

Join failure returns BUSY with output unchanged and all resources retained in the
caller-owned tree for retry. It is not quiescence and does not license disposal.
The harness does not claim to have induced an actual valid-thread pthread_join
failure. Successful wait closes its two descriptors, invokes all four drops,
publishes outcome and makes repeated wait CLOSED without changing output. Linux
close errors are not retried against a potentially reused descriptor.

## Error aggregation, expected results and experiments

Errors are reported by leaf index `group*2+slot`, with zero for unadmitted or
successful leaves and a separate admitted bitmap and spawn-failure bit. Primary
is the first nonzero error in creation order; it does not depend on completion
order. Two leaves reporting 11 and 22 in either forced temporal order retain both
and select 11. The four-slot vector is the bounded aggregate; no arbitrary-depth
exception-group equivalence is claimed. Failure injection selects each admission
before pthread_create, including no admitted group and partially admitted group.

Implement `src/nursery_follow.c`. Required evidence comprises the C tests
`messaging.nursery_nested` and `messaging.nursery_aggregate`, all 27 admission
schedules, actual pooled sums with one through four native workers, expired
deadline and ready cancellation, cancel-before-release barriers, unchanged
rejected output, exactly-once context drops, and the independent 2,187-case
aggregation oracle. The oracle derives admitted sets from the tree rather than
executing the C implementation's join algorithm. Reject four compiled mutants:
root-only cancellation, last-error selection, omitted context drops and swallowed
admission failure. Test under GCC/Clang debug, optimized, ASan/UBSan and separate
TSan. Sanitizers and finite schedules do not prove all interleavings or fairness.

For changed conditions, retain both nonzero results when either finishes first,
then explain why error order and temporal order are different contracts. Replace
finite pool leaves with cooperative long tasks and identify checkpoint intervals
in their cancellation argument. A task that never checks its flag is a valid
counterexample to a bounded join claim, not permission to free its context.

Hints: (1) cancellation flags are not join receipts; (2) child failure may happen
before the parent's next spawn; (3) initialize descendant controls before any
thread, and separate the owner of joining from the owner of cancellation.
The separate assessor analysis gives the ownership argument and fault matrix.
Native worker/thread stacks, scheduler latency, pthread storage, pipe buffers
and the finite reducer's arithmetic/data-model assumptions remain costs beyond
`sizeof(struct mfn_tree)`; do not call this a constant-time cancellation operation.

# Withheld T72 changed-condition analysis

Use the corresponding learner contract and original objective-specific rubric.
The following are assessor explanations, not learner hints or evidence of an
independent review. Compile the reference only through explicit assessor paths.

## X-T72-001

Consume-before-callback is observable when a callback fails. Moving publication
earlier leaks a partial owner or changes the output contract. Zip can avoid even
an extra first pull because both CFSource extents are known before either pull;
this proof fails for a general unknown-length iterator. The two map mutations
respectively omit failed-item cleanup and advance the first source when the
second is exhausted. Check allocations as well as values.

## X-T72-002

Filtering before grouping without preserving boundaries joins separated equal
keys incorrectly. The scan retains negative rows as unselected boundary markers.
CFBatch has distinct scanned and committed positions; a complete blocked group
is state, not a new read request. A negative-only group may advance committed
position without emitting. Ordering admits complete storage before writes and
is stable because equal keys are not shifted. Compare fused and unfused outputs
under equal selection/boundary semantics before measuring overhead.

## X-T72-003

Allocate the next owned item before releasing the previous one. Borrow, transfer
and clone have different validity intervals; only transfer clears current.
The self pointer detects an address change while storage is live, but cannot
authenticate stale aliases. The final admitted F46 does not need an unused
next-pair update. Check all counts and failed clones with old item canaries.

## X-T72-004

The mapping contains two inaccessible boundary pages and a writable middle.
Guard failure in a child proves that page's protection, not reliable handling
of arbitrary stack exhaustion. Nested fibers exercise distinct saved caller
contexts; resource cleanup occurs by callback cooperation before destruction.
Cancellation cannot safely jump out of arbitrary C call stacks. AArch64 paper
layout preserves x19..x29/SP and return continuation, low64 bits of v8..v15 and
the specified control state; x18 is platform-dependent.16-byte stack alignment
is necessary but insufficient for signing/shadow-stack/scalable-state support.
No executed backend for that ABI is present.

## X-T72-005

Tags encode descend-left, await-left, descend-right and combine. The left
temporary belongs to the continuation until combination/unwind. Cancellation
must traverse every owned temporary even if a child has failed. A complete
root result waiting on CEDeliver is a separate phase; repeated PENDING cannot
repeat evaluation or release the result's owner early. Forward edges bound
depth, while repeated DAG expansion may exceed the number of stored nodes.

## X-T72-006

This original cut commits a root attempt, not merely the nearest choice. For
ac, a successful cut('a') followed by missing b suppresses the ac alternative.
Without cut it succeeds. A prefix a with eof=0 is PENDING regardless of a
fallback that would accept a; absence of future bytes is not yet known. Keep
the furthest diagnostics independently from rolled-back cursor position. Four
stored expectations are the lowest distinct bytes; truncation discloses loss.

## X-T72-007

The seed starts as failure; the base digit grows it once, then the recursive
alternative reads that seed and extends through '+' digit. Accept only longer
endpoints. An in-progress failure bit without growth returns just the base.
Input revision and predicate environment are semantic key components, not
performance hints. This implementation has one fixed rule/position, so the
general mutual-recursion machinery and general complexity theorems are absent.

## X-T72-008

Under the mutex a terminal state is selected once. Tickets account for accepted
backend borrows, including notifications suppressed after cancel. Cancellation
does not erase those borrows. Inflight zero is necessary but not sufficient:
join/no-future-entry is a backend proof external to the frame. Only then ack
permits publication or exclusive destruction. Never test a stale ticket after
freeing its owner. The copied-channel adapter consumes local values before
notification and preserves the ticket when no value exists.

## X-T72-009

The chain is a borrowed lookup environment; the resource and frame are owned.
Clone allocates two objects, deep-copies bytes and repairs the continuation's
owner, rolling back on either failure. Sharing the handler chain is valid only
under its immutability/lifetime precondition. Clones can diverge after question1;
ordinary struct assignment instead aliases ownership and leaves the token tied
to the original. Koka multiple-resume/finalization semantics motivate the
resource question but are not implemented as a complete language runtime.

## Assessing explanations and transfer

Reject a successful test transcript without the state/ownership argument and
independent expected trace. Require why each mutation fails and which untested
inputs or schedules remain. Delayed work preserves exact X-T72-NNN-DELAYED
identity and seven-day policy; author variants are materials for that assessment,
not a learner's completed delayed assessment. Report compiler diagnostics,
sanitizer coverage, process/ABI observations and physical evidence separately.

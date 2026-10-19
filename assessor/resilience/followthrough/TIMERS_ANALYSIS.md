# Timer successor technical assessment

Assess X-T75-003 R1-R4 separately. The retained CMX is a fixed-array deadline scan;
do not relabel it as the missing list/heap transfer. Its modular uint32 comparison
and this adapter's nonwrapping uint64 contract overlap only for a stated bounded
set of observations. The test imports actual `cmx_init/task_add/sleep/cancel/tick`
and checks state membership across wrap; it does not invent a stable CMX event list.

The list orders by `(due,sequence)`, and its predecessor/successor repair preserves
bidirectional traversal. The heap stores inverse positions and repairs in the
direction determined by the replacement's parent comparison. Removing the final
element is a distinct boundary. The wheel uses full due times and delayed cascade:
at processed40, a future timer due107 shares the coarse bucket but must stay linked;
at104 it enters fine bucket3 and expires107. This is why modulo equality alone
cannot implement either cascade or expiry. Stable fine-bucket sorting is bounded
quadratic work, which must appear in the analysis and measured burst experiment.

The caller owns the container lifetime. Generation exhaustion retires only an
inactive slot; a live maximal-generation timer remains removable. Global serial
exhaustion stops new admission but does not stop cancellation or expiry. The raw
address in an identity is not a protection against reinitializing the entire
container at that address; that operation requires old identity retirement.

Advance stages output records, but mutates a valid container as it progresses.
All public refusal conditions are checked before this progression, so refusal is
transactional without a copied-container rollback. There are no fallible callbacks
or allocations inside the progression. The loop cannot increment past UINT64_MAX:
it increments only while `now < target`. `target-now <=1024` bounds per-call missed
ticks, while a series of calls may need proportionally more work.

The finite oracle uses a dictionary keyed by slot plus sorted absolute due tuples,
not links, heap positions or wheel arithmetic. Review equal-deadline order and
stale cancellation explicitly. Mutant31 leaves coarse timers pending, detected by
the due9 expectation; mutant32 cancels a live replacement with its predecessor's
identity. Inspect invariant checks after head/middle/tail removals and the actual
producer thread while processing is paused. The mutex is the synchronization
mechanism; the timer structures are not claimed lock-free.

Resource comparison is deliberately honest: `RztTimers` reserves all runtime
alternatives, so physical object size is equal. Logical maintenance counts expose
different work, not cache traffic or every C write. Timing builds disable those
counters. Report raw unsanitized GCC/Clang observations for the declared fixed
workloads; avoid generalizing N16 results to arbitrary capacities or a different
board. No throughput result can establish a deadline guarantee. Actual per-call
samples include clock-reading overhead and earlier output may perturb caches.
The supplied sealed author prediction is not a learner's unseen decision.

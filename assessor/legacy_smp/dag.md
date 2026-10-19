# T53.12 assessor analysis

The public `dag.h`/CONTRACTS describe the exact domain; learner `dag.c` and target
adapter remain unsuccessful stubs. The reference owns no heap allocation. Each
graph and its claims remain live until every caller has joined, even after the
sealed/terminal count says no task remains.

## `.a`: work, span and meaningful decomposition

Kahn validation on copied masks establishes an actual topological order. Work is
the sum of costs; span is max(cost+predecessor spans), with checked additions.
The four-node fixture has11/8 and ideal1/2/4CPU makespans11/8/8. Adding B→C makes
11/11; it also changes ordinary recursively summed values from1/3/4/11 to
1/3/7/14. Do not retain the old final value when this ordinary node function
depends on all predecessors. The performance task has separately declared node
programs, so a new ordering edge does not silently add a value twice.

Splitting C is justified because its bounded integer sum decomposes over disjoint
ranges. The split costs2+3 retain total work11, change span to6, and preserve the
exact result. Ideal two-CPU makespan remains8 because three branches totaling8
must fit after A and before D; four CPUs attain6. No general sequential recurrence
can be split this way without another proof. The initial native pilot correctly
refused a modular checksum whose partial sums overflowed the checked aggregator;
the final bounded low10-bit mix aligns workload and mechanism contracts.

## `.b`: validation, publication and dynamic append

Duplicate policy rejects before masks can silently deduplicate. All endpoints
are checked before shifts; capacities cap bit positions at15. Validation degrees
are temporary, leaving real remaining counts intact. An error never publishes
part of the new graph. Under the single graph lock, finish stores a result then
adds it to waiting successors and decrements their remaining counts. Only zero
changes state to READY; claim makes RUNNING under the same lock. The independent
oracle represents adjacency as Python lists, not the reference's bit masks, and
enumerates cyclic as well as acyclic graphs. It is still only a finite check.

Append locks the current world, stages all metadata and already-completed parent
values, and publishes one new index only after every check. Earlier parent
indices create a monotone extension order even if the original graph used
arbitrary forward/backward numeric indices. A parent either finishes before
append (its value is included immediately), or after append (the new successor
mask includes it). No third state loses or duplicates that dependency. Capacity,
duplicate, unknown-parent and arithmetic failures preserve output and graph.
This rule deliberately disallows changing old dependencies or adding future-node
references; calling it arbitrary graph mutation would be an overstatement.

## `.c`: failure, cancellation and measured causality

A terminal failed node causes every waiting dependent to become SKIPPED exactly
once. The propagation queue contains at most16 unique newly terminal nodes.
Already terminal/skipped nodes ignore later notifications. Input-sum overflow
produces a failed unexecuted node rather than a wrapped result. Independent
branches continue. Running cancellation retains the owner until finish and then
publishes CANCELLED; prematurely incrementing terminal count would permit
graph storage reuse while a worker still holds a claim.

The native diamond barrier forces both predecessor work bodies to finish before
their competing finish calls, stressing last-dependency publication without
holding the graph lock across the barrier. All logical task timestamps and owner
rows remain present. Readiness is an interval around actual predecessor finish
calls, not an exact guessed clock sample. A successor may run before its parent
records the after-call timestamp; the verifier allows that legitimate order.
The actual CMK guest imports the completed pool launcher and pins every worker,
including the coordinator. A registration barrier prevents shutdown from hiding
a child that has not yet entered the graph. Local TSC durations cannot establish
physical speedup or cross-core timing equivalence.

Nine normal semantic controls target separate contract errors; controls do not
deliberately perform invalid pointer access or free a live stack. The mutation
that distributes zero before storing a parent's result is diagnosed as a
publication defect even if the parent's own final result later looks correct.
Delayed tasks, external human review and physical evidence remain unattempted
dimensions unless separately recorded; author execution is not learner mastery.

The first complete campaign stopped at Clang release linking: aggregate copies
and initialization emitted `memcpy`/`memset`. The explicit target byte helpers
are a real dependency, not a weakened undefined-symbol audit. Their volatile
stores prevent recursive loop-idiom lowering; bounded guard-byte tests and the
strict final ELF audit check that selected dependency contract. The failed
attempt remains evidence of the discovered integration error.

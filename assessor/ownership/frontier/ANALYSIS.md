# T66.01 graph frontier: assessor analysis

Keep acquired and initialized distinct. At each loop head, count is the number
of successful takes, acquired is exactly their ID set, initialized is a subset,
and every initialized node's prerequisites are initialized. Prevalidation makes
every acquisition order topological and every count representable before effects.
After a take, storage belongs to the staged owner even if initialization fails.
Never call that node's destructor on a failed initialize return.

Reverse acquisition order preserves every dependency until its last dependent
has been destroyed. This remains true for A,B,D,C, so the mandatory failed-C
case releases C without destroyC and then destroys D,B,A. A count alone cannot
distinguish the four acquired nodes from the three constructed nodes. Mask and
callback-view removal before destruction make callback-observable lifetime exact.
The view does not authorize reentry or retention. No arbitrary foreign pointer
validation is attempted.

On success publication is one complete owner assignment. On failure only staged
state is consumed; the public destination remains empty. Final destroy clears
the owned fields and masks and is idempotent under the authentic-control premise.
The source still retains its allocator/context metadata after becoming empty;
reconstruction replaces it only after successful staging. Output preservation
does not erase allocation/init/destruction log effects.

The independent Python model uses prerequisite sets and an acquired ID list,
not the C mask implementation. It enumerates all24orders, the three valid orders,
every take/init failure and invalid/representable-but-backend-too-large counts.
Exact traces distinguish the FIFO, omitted destruction, leaked raw storage and
early publication controls. Mutants never need an invalid memory access as their
intended success criterion; assertion failure is the observed negative outcome.

The Rust specimen's Storage and Value destructors explain partial construction
with actual automatic local cleanup. It does not carry the C dependency graph in
the type system, and Drop cannot return an error. Ask the learner how field/local
ordering and external side effects can break an overstated equivalence.

R1 requires the subset/dependency argument and language/runtime/discipline matrix.
R2 requires actual V4Allocator/ow_allocator composition and unchanged failure.
R3 requires complete exact reverse traces, all acquisition and size boundaries,
two destruction calls and compiled controls. R4 requires an independent order and
failed initialization after acquisition, plus a new graph with a stated ownership
and destruction-order derivation. Sealed seven-day work remains separate from the
author's already executed changed case. Native timing and finite tests cannot
prove arbitrary callback code, physical execution or learner mastery.

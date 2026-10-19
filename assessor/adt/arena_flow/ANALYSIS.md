# Assessor: Whole validation and fixed-point premises

Reject a selected-root-only explanation. ad_eval checks every node even when
the chosen root is an early literal: an unreachable forward reference, invalid
kind or overflow invalidates the entire arena. adf_arena copies first, validates
the copy through actual ad_eval, and only then traverses prior indices without
rechecking. Its values coincide inductively with the validator's values, so
those additions are representable. The constructor stages a complete value and
publishes last. Changing original input after construction cannot invalidate
the owned copy; changing the public certificate itself remains caller-invalid.

A16-node doubling chain is a directed acyclic graph with shared children. Its
recursive expansion has2^16-1=65535 node visits, despite only16 stored nodes.
The measured cached-query workload includes one constructor and does not claim
equivalent mutation tolerance to repeated validation of a changing source.

In the fact solver each transfer is monotone under componentwise subset order.
Bottom is below every fixed point. By induction every worklist update stays
below every fixed point and cannot remove an already established fact. Every
node acquires at most8 facts; at most8E successor-enqueue events plus n initial
entries occur. A bitset can merge events but cannot create additional ones.
Every pop has a corresponding enqueue, giving n+8E<=520. At empty worklist, any
node whose predecessor changed since its last evaluation would be pending; all
equations therefore hold. Bottom iteration yields the least fixed point.

A budget is measured in popped nodes, not elapsed time or strict changes. A
zero-fact graph still needs each initial node processed to certify the equations.
The session retains partial progress when BUDGET is returned, while the result
object remains unchanged. Repeated calls with small positive budgets terminate
under the same bound. Original input mutation is harmless after the owned copy;
mutating session graph/value/pending fields invalidates the proof.

NO_FIXED_POINT is impossible for that admitted finite monotone model. The
separate truth-table solver allows nonmonotone functions and can prove absence
by checking every assignment. Self-NOT has no Boolean solution, but testing only
assignment0 cannot prove it. Mutual NOT has two solutions even while synchronous
00/11 iteration oscillates. This is the required counterexample to declaring
any nonconverging iteration unsatisfiable. Unbounded naturals with x+1 need a
different existence/convergence premise; machine wrap is not that model.

The independent arena oracle uses recursive values and mathematical overflow
checks. Flow final values come from synchronous rounds rather than reproducing
the worklist's exact trace. Boolean solutions are derived by independent tuples,
then compared to the least numeric satisfying assignment within budget. These
three models assess different claims and must not be credited interchangeably.

Mutation1 validates only the first copied node. Its public killing fixture has
an unreachable invalid kind with safe in-range fields, so rejection is a normal
semantic assertion rather than an out-of-bounds access. Other invalid-input
paths must never be used as portable expected-output oracles for a broken
implementation. The original forward-reference mutant uses initialized scratch,
making its wrong value deterministic while still semantically unacceptable.

Apply the four original criteria and thirteen cost rows individually. Native
compiler helpers, stack/binary sizes and raw timings are implementation evidence,
not ISO or freestanding guarantees. The informal primary dataflow passage does
not certify these original transfer functions or all analyses. Independent
learner assessment, non-author review, published C17/C23 text and physical
behavior remain separate evidence limits.

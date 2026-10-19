# Retained mechanisms and explicit law investigation

This required support keeps X-T76-001 through009's original implementations,
IDs and acceptance history. It recompiles the actual original C tests, eighteen
semantic controls, nine compiling learner-stub rejections and independent
mathematical/tagged-set/hash/byte oracle under current source pins. It does not
substitute original nominal tests for the separate durable, membership, delta,
sequence, Merkle, wire or replay successors. No new executable mechanism is
invented merely to increase an exercise count.

For X-T76-004-R1/R2/R3, implement the existing `RPPN`/`RPRegister` contracts first.
Derive the partial order over six unsigned components and the lexicographic
register event order. Enumerate every triple of the64 binary-component states,
six delivery permutations and self-retransmission. The independently expected
binary result is componentwise OR. Enumerate1,000 triples of ten compatible
register events and all six permutations. The event set is canonical initial
`(0,0,0)` plus each time1..3, replica0..2 with a distinct prescribed payload.
The selected event must be the lexicographic maximum, not the last arrival.
These finite enumerations supplement a general max-join proof; they do not prove
all uint32/uint64 executions. The Python oracle additionally covers actual
full-width state values, register extrema, invalid writer IDs and conflicts.

Required boundary reasoning and harmless original counterexamples:

- Two independent writers each incrementing replica0 from0 to1 merge to1,
  losing one intended event. Unique writer ownership is a premise; max is not
  broken by failing that premise.
- Adding a received state twice gives2 for a single increment; correct state
  merge remains1. An operation replay protocol needs a different identity ledger.
- An unchecked uint32 increment at MAX wraps to0 by defined unsigned arithmetic,
  violating inflation. The actual checked API refuses unchanged. Explain why
  unsigned defined behavior does not imply a correct distributed protocol.
- Event `(100,0,1)` can have occurred physically before `(2,1,2)` yet win the
  protocol order. No wall-clock recency is inferred from a total-order label.
- Equal identity `(1,0)` with payload10 versus20 conflicts. Joining a later
  `(2,0,30)` first masks the old conflicting pair; the API is not an associative
  total join over arbitrary incompatible histories. State precisely where the
  compatible unique-event assumption is required.

Inputs and outputs are the existing declared typed values; no external service
or real credentials are used. The target is the original local C program and
its separately compiled mutations. The teaching adversary controls only valid
synthetic event/state values and delivery order. Success is the listed wrong
counter/event result, never a crash or arbitrary code execution. Repair merge,
tie handling, overflow or writer admission as appropriate; retain regression
cases. Authentication/capability policy belongs to the membership successor and
does not retroactively make a forked writer honest.

Deliver the implementation, mathematical argument, ownership assumptions,
trace table, exact compiler/diagnostic evidence and cost table. Reuse the existing
artifact graph (`persist.hamt`, explicit allocator/failure model) for original
log/checkpoint tests. The original six `rp_checkpoint` failures must leave the
published root and arena used cursor unchanged, while unreachable scratch bytes
need not be identical. Preserve all original external-effect/transfer obligations.

Execution uses C17 hosted GCC/Clang; pthread tests explicitly serialize projection
updates. TSan checks executed schedules and is not a weak-memory proof. No
additional target library implements a studied mechanism; Python and sanitizers
are host verification dependencies. Learner builds use completed `replicated`
and DOD workspaces, with assessor overrides exclusive to author validation.

Hints: first name the state order; next separate event identity from value;
finally construct a conflicting pair hidden by a larger event. The private
analysis explains the max laws and incompatible-domain counterexample. Rubric
level1 is matching examples; level2 requires the complete law/premise/refusal
argument and independent evidence; level3 changes writer ownership, clock
ordering or delivery history and identifies which proof breaks. Apply each
original X-T76 rubric separately. An author campaign grants no learner level.

The delayed X-T76-004-DELAYED task retains an unseen writer replacement and
authenticated state policy from MEMBERSHIP.md, plus an incompatible register
history. At least seven days and an unaided attempt remain external evidence.
Seven CPU samples use the same100,000 correct PN/register updates; report native
sizes and actual stack/code outputs. Complete all13 modern cost dimensions using
the original and successor contracts: no allocation/I/O inside these joins,
O(number-of-components) PN work and O(1) register work under fixed-width arithmetic,
but growing identity/history storage in changed membership is separately charged.
ISO C supplies integer semantics, not monotonic clocks, unique writers, purity,
network delivery or distributed authorization. Physical execution, independent
human review and learner mastery remain unclaimed.

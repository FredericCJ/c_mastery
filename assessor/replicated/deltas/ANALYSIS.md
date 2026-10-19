# Durable delta assessor analysis

The two real RPSets provide24 add identities, with the second object's upper
writer slot unused. A local remove records exactly observed tags; copying known
frontiers into its delta permits structural validation before the matching add.
Each tag remains assigned to one value. Retaining both add and remove sets is
space bounded here, and FULL is semantically significant. The counterexample
that clears both masks but retains the counter shows why merely avoiding tag
reuse does not make tombstone deletion safe: an old saved add is still valid.

Sequence counts local changes and admitted received growth. A local empty
remove still advances the sequence; an already-subsumed received body does
not. The persistent set and sequence commit together. Volatile delta eviction
does not affect the set: a peer behind the retained prefix receives full state.
This is distinct from tombstone collection. A proof that all current peers
acknowledged a packet is insufficient if an old removed peer/message can return
under an unaccounted membership policy.

Durable epoch advances before a reopened writer returns; sequence and tag
frontiers never reset. Both packet and ACK contain source and target epochs.
A prior-session ACK cannot certify a current-session interval. The receiver
also refuses a delta with an unknown sender epoch or start beyond its joined
endpoint. The oracle retains actual sender prefix states and asserts the
receiver subsumes that base, making the causal check independent of a shared
numeric-counter mistake. The proof still depends on identified honest peers
and preservation of successfully committed state, exactly as documented.

Storage failure may leave the new redo commit durable even when the call
reports uncertainty. In-memory rollback is therefore not claimed. No ACK or
new caller output escapes on a failed call; recovery/open establishes new
authority. A failed epoch advance may consume an epoch, which is harmless.
Current-state comparison rejects an accidentally retained old control, but
does not authorize arbitrary concurrent accesses to one store.

The finite boundary fixtures install canonical durable state through the real
store, then exercise the last sequence/epoch advance and the refusal after it.
They do not pretend to generate 2^64 transitions. A duplicate body needs no
new sequence and remains acknowledgeable; a growing body at the limit cannot
publish an ACK. Compare actual callback counts as well as state/output bytes.

Controls1/2 test lost tombstones/unobserved removal;3/10 test digest/canonical
layout;4/7 test state/ACK before commit;6 tests missing epoch advance;5/8/9
test missing full fallback/causal gate/session ACK binding. Require the named
normal RP_CHECK failures. The first fault pilot's recovery-counter setup was
wrong and fixed before the final campaign; do not count its misleading mutant
passes as evidence. The final suite's ordinary controls must pass first.

Grade R4 as incomplete if a learner implements only an in-memory counter reset,
a full-state exchange renamed as an interval protocol, or a recovery path that
emits ACKs before durable admission. Grade every broad availability/Byzantine/
physical guarantee against its actual premises. A finite author campaign and
an agent code review are not a delayed independent learner assessment.

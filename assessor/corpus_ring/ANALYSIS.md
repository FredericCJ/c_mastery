# X-T76-010 assessor analysis

Keep this reference, oracle and delayed analysis out of learner packages.

## Core proof and concrete counterexamples

For key k, compare tokens by ((position-k) modulo65536,owner,ordinal). This
linear oracle is independent of the implementation's binary search over sorted
positions. At an equal position the lower owner wins; e.g. (100,2,0) precedes
(100,3,0). A strict-greater successor bug returns the wrong owner at100, and
reversing the collision tie makes ownership depend on a different contract.
Duplicate (owner,ordinal) identities at any positions must be rejected. This
prevents ambiguous membership edits; position collisions are a separate case.

With tokens (10,1,0),(30,2,0), insert (20,3,0),(40,3,1). Keys11..20 and31..40
move to owner3; all others stay. Thus exactly20 of65536 keys move, showing that
monotonicity alone does not imply a one-third balanced share. Removing3 restores
every original mapping. Public tests enumerate the entire key space for both
directions, and the independent model covers432 distinct finite views plus
1500 membership transitions. A valid alternative linear-scan implementation
may pass semantics, but cannot claim logarithmic lookup; assess its cost
argument separately rather than rejecting a correct slower representation.

Reseeding counterexample: old token positions10(owner1),30(owner2), key15 maps
to2. Change owner1's token to20 while adding owner3 at40; key15 now maps to old
owner1. Old-to-old movement violates the insertion monotonicity claim because
old tokens did not remain fixed. This is not a counterexample to the theorem's
actual premises. A byte-identical key can also move if digest interpretation
changes from big-endian to little-endian; make hash/serialization policy part
of view identity. A local uint32 epoch is not a globally unique identity.

For repeated virtual nodes (10,1,0),(20,1,1),(30,2,0),(40,3,0), wanted3 at key0
must return[1,2,3], not[1,1,2]. If owner1 is down, wanted2 returns[2,3], wanted3
fails with no output. Skipping unavailable nodes changes placement, not the
number of durable copies. Empty views, all-down masks, wanted16 and capacity
failure require separate reasoning; no wrap or shift uses an invalid width.

All output arrays are staged. `cr_plan` must not publish the first move then
discover capacity exhaustion. `cr_change` must not advance epoch before a cut.
Ordinary C assignment can tear under unsynchronized access; a concurrent port
would require a lifetime/publication protocol in addition to this value copy.
An immutable snapshot plus lock/RCU/hazard scheme is a possible new design,
not an unimplemented promise silently credited to this single-threaded one.

## Integration and enforcement

`sha_adapter.c` is a genuine direct call to the preexisting RphSha interface.
The campaign pins its exact header and implementation and checks513 byte-key
lengths0..512 against Python hashlib and the independent clockwise scan. The
core callback fixture instead targets failure propagation and byte order; it
does not stand in for real SHA integration. Initialization/export must preserve
an intentionally incomplete learner provider and require an explicitly selected
private reference. A function pointer table in C checks signature compatibility
only; compare it with a trait/interface's method contract, which also needs
semantic laws or verification. Neither a SHA library nor a language's map
type supplies this placement/view invariant automatically.

Compile controls1/2 change equality and tie order;3/4 publish an epoch before
abort and accept stale epochs;5/6 repeat physical owners and ignore the down
mask;7/8 ignore hash failure and reverse digest byte order. Each compiles, runs
its intended anchor and is rejected by a CR_CHECK assertion. Negative output
or a compilation error alone cannot count as a semantic kill. Control9 publishes an early migration row before capacity failure; the exact unchanged-array assertion rejects it. The oracle's
random seed and view bound are reproducible but do not prove all memberships.

## Graduated hints and rubric

Hint1: draw a circle containing0,65535 and two coincident tokens. Hint2: find
the first position >=key, then handle the end of the array. Hint3: keep token
identity, physical owner and key position distinct; build the complete new
view before publishing. Record assistance and failed hypotheses.

R1 needs the canonical representation, collision and monotonicity proof. R2
needs a bounded chosen-token load-capture demonstration and distinct replica
reasoning. R3 requires actual hashing composition, endian interpretation,
independent expected values and measured costs. R4 requires adjacent-epoch
plan composition, failure atomicity and the limits of handoff/anti-entropy
claims. Level1 is supplied-case execution; level2 satisfies the exact stated
contract with appropriate boundaries; level3 derives and checks an unseen
view/failure change without copying the reference and identifies broken
premises instead of overstating correctness.

## X-T76-010-DELAYED: degraded replicas and migration planning

Release at least seven days after ordinary work. Supply an admitted old view
[(10,1,0),(20,1,1),(30,2,0),(40,3,0)] at epoch7. Remove owner1 to create epoch8.
For keys[0,10,11,20,21,30,31,40,41], produce the exact migration plan using the
learner's earlier view and lookup component, then request two distinct replicas
for key0 with owner2 unavailable. Repeat with output capacity1, an old expected
epoch, and a view in which every token has position0. Require before/after
state, a no-publication argument, load and token-visit counts, and an explanation
of what would be needed to execute a durable handoff.

Expected moved keys are[0,10,11,20,41], each1→2; other keys retain2 or3.
Wanted2 with owner2 down fails because only3 remains. Capacity1 rejects the
five-move plan with unchanged rows/count. A lookup using epoch7 on the new view
returns STALE. In the all-zero view the smallest owner takes all keys, and a
replica scan can still find distinct owners at later coincident tokens. The
reference implements all these operations now; an independent learner attempt
after elapsed time remains pending. Do not claim learner integration from the
reference-provider execution.

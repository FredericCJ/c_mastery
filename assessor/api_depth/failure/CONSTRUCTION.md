# Private T21.03–04 analysis

The discriminator is the remaining dependency graph. Blind reverse cleanup is
insufficient when a child close retains ownership. In the count=3, order=0,1,2,
close-failure-mask=4, consume-mask=0 case, cleanup must retain mask 7. With
consume-mask=4 it must release all three, return DEVICE, and retain the close
cause. The independent oracle predicts held nodes and individual release events.
`AE_MUTANT_DEPENDENCY` and `AE_MUTANT_MISSED_RELEASE` compile and fail assertions;
an unsolved learner implementation also compiles and is rejected.

03.a assesses every prefix and unchanged output sentinel. 03.b assesses consumed
versus retained error, fourth-resource and exchanged-base order. 03.c assesses
failed init, repeated stop, duplicate ACTIVE init and three-generation retirement.
04.a assesses nested/labels/loop acquisition and the changed external effect.
04.b assesses resource/request identities, transfers and reused numeric slots.
04.c compares event traces and rejects rollback after commit. Grade each
criterion independently; passing the grid does not supply the causal explanation.

The three acquisition styles have three actual cleanup traversals: nested
unwinding, descending labels and reverse iteration. They share a close-at-position
primitive that checks live dependents and updates ownership from the close
postcondition. Require a worked control-flow derivation in the dossier; a label
that always closes a retained child's parent is wrong even if its no-error path
looks conventional. The graph oracle is independent of those traversals.

For the changed transfer, keep ledger 2 held after transferring it. Constructor
cleanup leaves base mask 3 and reports DEVICE. Only external close of 2 permits
base teardown. For postpublication failure, AMBIGUOUS does not undo the modeled
effect. The compensation input is trusted environmental evidence, not a proof
that a real remote operation was undone. An answer asserting a strong external
rollback guarantee fails irrespective of matching return codes.

Release the original changed packets after the minimum seven days. The executed
fourth-registration graph and changed base order require an independent cleanup
proof. Do not accept a copied nominal three-resource trace as transfer evidence.

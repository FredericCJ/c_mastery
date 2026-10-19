# Private query-index assessment

An index is redundant derived state. The reference stores row copies and builds
independent dense and sparse views so tests can expose disagreement after a
replacement. Clearing removed terms matters as much as setting new ones. A
complement ranges over live row identities, never all machine-word bits.
Rows are stable slots; compacting storage would require a new identity mapping
and consumer invalidation protocol, not an invisible optimization.

For scalar nullable logic the values are true, false and unknown. AND is false
if either side is false, true only if both are true; OR is true if either is
true, false only if both are false. The disjoint yes/no masks encode these
rules exactly; unknown is the live universe minus their union. On a null row,
NOT unknown remains unknown. The reference's NOT swaps the sets. A naive
complement of yes changes unknown into true and may include nonexistent rows.

Sorted document IDs let each merge discard the smaller unmatched ID because
it cannot occur later in the other sorted list. Candidate intersection alone
is insufficient: [1,0] contains both terms of [0,1] but not the phrase. Start
position s is legal exactly when every query[q] appears at s+q, hence shifted
position-set intersection. Repeated [1,1] requires two consecutive occurrences.
For proximity, enforce a positive distance even when both terms are identical;
otherwise every single occurrence incorrectly pairs with itself.

Zone soundness is an enclosure proof. If all current nonnull values are between
m and M, disjoint [m,M] and [low,high] imply no match. Overlap is only a necessary
condition: a group containing0 and10 has no value5. Lazy replacement must retain
the old enclosure and include the new value; dropping a former extreme is safe
but unnecessary for correctness. Deletion can leave a present conservative
group with no live values. Rebuild may then remove it. Empty, all-null, deleted
and partly filled groups need different state explanations even when all return
no matches for a selected query.

The concrete stale-maximum counterexample starts with values0..3 in the first
group, replaces row1 by100 and queries[100,100]. Failing to widen drops row1.
Deleting row1 lazily afterward gives a legitimate false-positive candidate
group but no result. Rebuild removes this needless scan. For equal-boundary
tests, a group minimum4 must survive query[4,4]; strict pruning at equality is
wrong for this inclusive API.

The implementation stages a full index before replacement. No fallible operation
occurs after publication. Alternatives can stage only affected rows/lists/zones,
but must prove rollback and avoid exposing inconsistent views. A sparse posting
implementation can replace the dense masks if the public result contract and
cost comparison remain explicit; it cannot substitute a library that performs
the mechanism. Original byte helpers close compiler lowering on the cross-build,
not a hidden allocator or indexing runtime.

The withheld transfer implementations are qi_compare/qi_logic, qi_near and lazy
qi_replace with qi_rebuild_zones/qi_search. Check actual changed-condition code,
not just a discussion. Alternative source-valid approaches include posting
cursor intersections and direct position scans after document intersection.
Require proof that repeated terms and nulls retain the same semantics.

Rubrics assess three independent dimensions per exercise: exact representation
and invariant, correct operation/failure behavior, and falsification/cost/transfer.
Fail a false-language claim even if expected examples happen to pass. A learner
who cannot derive the scalar truth table, positive-distance rule or conservative
enclosure proof has not demonstrated the corresponding criterion. Graduated
hints: define the complement universe; mark candidate starting positions; write
the logical implication that permits skipping a group; compare replacement
publication with a transaction. Reveal concrete counterexamples only at the
appropriate assessment stage. Author attempts are not delayed learner evidence.

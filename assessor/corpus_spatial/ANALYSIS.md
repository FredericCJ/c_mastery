# Spatial assessor reasoning

Keep this separate from learner packages. Review the exact selected contract,
not a remembered algorithm with different equality or ownership conventions.

## KD

Median partition includes duplicate coordinates using ID as the tie key; both
subtrees' boxes enclose their actual points. Summing independent per-axis distance
to a box yields a lower bound on every contained point's weighted squared distance.
All weights are positive, so pruning a strictly greater lower bound is sound.
Equal bounds require traversal because a smaller ID may win. Mutant1 changes that
boundary; mutant2 silently uses the unweighted metric. The weighted fixture
(2,0) versus (0,3), weights(4,1), picks ID1 at distance9, not ID0 at16.

## BVH

One leaf per object gives n leaves and n-1 binary internal nodes. A disjoint
parent bound excludes every descendant. Face contact is included; mutant3
changes that rule at a leaf. Refitting updates every union after children;
mutant4 leaves a stale ancestor and misses a box moved to(1024,1024).
Refit can preserve a poor partition; compare query counts before claiming speed.
Only box geometry is represented. A pair of enclosing boxes can overlap when
their represented physical shapes do not, so broad-phase output is not narrow-phase
collision evidence.
For a concrete counterexample, planar segment A from(0,0) to(2,2) and segment B
from(0,1) to(1,2) have overlapping boxes but lie on parallel distinct lines.
Their enclosing-box overlap does not establish segment intersection.

## Quad/octree

The 32-wide integer root halves five times. Every point belongs to exactly one
child because the high child starts at the midpoint and the low ends at midpoint-1.
Coincident IDs must stay together in a unit bucket. Mutant5 loses the low child;
mutant6 keeps one ID and drops coincident siblings. The tree invariant checks
disjoint leaf masks, union equal to the admitted live set, and point containment.
Bounding continuous coordinates without a minimum resolution does not prove
the same termination result. The inspected skip-quadtree paper is used only
for region/ownership background; no skip hierarchy is implemented here.

## R-tree

Sibling covering boxes may overlap, so descending only the first relevant
branch loses results (mutant7). Split must keep all five entries with at least
two on each side, recursively propagate overflow and tighten boxes. Delete
may remove an underfilled internal subtree; its descendant leaf IDs remain
live and must be reinserted (mutant8 removes that step). Reinsert after root
collapse, using fresh staged nodes. The reference deliberately flattens orphan
subtrees, unlike original level-preserving CondenseTree reinsertion. This trades
implementation complexity for more work without changing the stated exact query
contract. The bounded proof covers occupancy, equal depth and each ID once.
It does not claim original page-I/O complexity or persistence.

## Grid

For x<0 with a nonzero remainder, C x/width is one greater than mathematical
floor: mutant9 exposes this in the actual stored cell representation. A monotone
but wrong partition can accidentally preserve many final query results, which
is why testing only final overlaps is insufficient. Store full cell coordinates
despite hash collisions. Multiple shared cells do not create multiple candidate
identities; mutant10 increments the pair count repeatedly. The three-dimensional
closed extent count is a product of spans, bounded and computed before writes.
A large box with a small width can be valid geometry but inadmissible storage.

## Failure and evidence limits

Every public mutator stages into caller scratch and copies only after all steps
complete. Internal references contain indices, not borrowed pointers, so replacing
the index does not leave external aliases under the contract. The host invariant
checker inspects the actual representation rather than trusting its cached mask.
Python's brute-force oracle does not reuse tree, hash or C geometry helpers.
Target link auditing checks generated dependencies; it supplies no physical cache
or application latency observations. Compiler stack records are per function,
not a whole recursive path bound. Independently derive the n/depth bound and
add frame costs before claiming a target stack budget.

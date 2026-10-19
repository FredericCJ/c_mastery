# Strided borrow analysis — private assessor material

The abstraction is a finite set per live loan plus writer authority, not its
convex hull. The shape's last index is its maximum because strides are unsigned.
Validate nonempty offset below extent, then subtract offset from extent-1, prove
(rows-1)*row_stride fits by division, subtract it, and prove the column term.
This proves each later intermediate and final index is representable. Empty
shapes skip those operations. Distinctness still needs checking: a legal maximum
does not exclude row/column collisions. Local staging preserves output canaries.

Exact conflict is intersection combined with at-least-one-writer. The even/odd
example refutes necessity of nonoverlapping hulls. The current shape bound makes
O(64 squared) comparison inspectable; a GCD progression solver would need a new
proof for finite endpoints and multidimensional collisions, not a renamed loop.

Split's two acquired sets are disjoint subsets with union equal to the original.
Valid parent dimensions prove any nonempty right-child offset multiplication
fits. Empty right children need no next-row address: for padded final rows that
address could lie beyond the array. Retiring a MAX parent can remove the slot
from candidate capacity. Failure must restore every logical field, not padding
bytes whose values C structure assignment need not preserve. Tests compare
fields and output canaries. Child owner addresses must be rebound after commit.

The row/column test performs real guarded writes into int32_t[18], preserving
padding and all excluded cells. Numeric SIZE_MAX validation never dereferences
an imaginary array. Tokens are identities within a live serialized book, not
pointer validators, authentication or thread synchronization. Safe Rust RefCell
language/Drop integration is a stronger caller boundary than raw C controls;
this adapter separately supports disjoint subsets rather than one whole value.

Mutations1..6 isolate conservative false rejection, missed exclusion, self alias,
partial commit, stale reuse and padded offset. Actual OW_MUTANT4/14 retain the
original interval regression. Independent Python derives Cartesian-product sets
and split unions/capacity directly; bounded agreement is not universal proof.

Assess D1-D4 independently. A learner who merely copies this algorithm has not
demonstrated the delayed tile transfer or a new cardinality/cost tradeoff. A
future larger model may conservatively reject or use a richer solver, but must
state which acceptance behavior changed and compare equivalent contracts.

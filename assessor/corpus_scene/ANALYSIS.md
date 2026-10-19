# Assessor analysis

Keep this private when publishing learner work. Reference code is an assessor
artifact, not an imported answer for the learner's first-principles profile.

## Scene

The noncommuting anchor separates parent*local from local*parent: a translated
child's local X vector becomes world Y under a quarter-turn parent. Multiplying
in the other order gives(13,0,0), not(10,3,0). Mutant2 changes this causal step.
Mutant1 invalidates only the edited node; reading a previously clean descendant
then returns the old answer. A whole-frame traversal can hide that bug, so the
test directly requests the descendant. The independent oracle never reads caches.

The dirty chain terminates at a clean ancestor or sentinel. Ascending collects
indices; descending composes from that ancestor. Every result is staged before
cache publication, so a late range refusal cannot turn an ancestor falsely clean.
The finite slot owner and generation are distinct from storage address. Removing
a parent is refused while children exist; reparent walks the proposed ancestors
before changing a link. A forest cannot turn cyclic through an admitted operation.

Preserve-world is Lnew=inverse(WnewParent)*Wold. A signed permutation matrix has
inverse transpose, and inverse translation is minus that transpose times t.
This intentionally supports only a tractable exact inverse domain. Scale2's
inverse halves values and can leave the integer domain; singular transforms have
no inverse. Refusal is correct even if some particular child could be reparented
under a broader rational implementation. For V1, show a shear/rotation mismatch
and change a parent without preserving world; for V2 compare every world entry
before/after and snapshot the entire scene on unsupported inverse admission.

The fixed storage cost is not free: two matrices per node, the dirty-chain
array and staged matrices, and a full staged scene during keep-world. Recorded
work counts compare exact matrix multiplications, not latency or cache misses.
ISO C types do not enforce acyclicity, unique ownership or immutable handles.

## Corridor

CCW polygon interior lies left of each edge. Crossing that edge leaves the polygon
to its right; the destination-facing left endpoint is the edge's second vertex.
Swapping portal sides (mutant3) changes the funnel into a misleading exterior
constraint, despite valid local polygon bounds. The original determinant uses
ordinary counterclockwise-positive cross products; its inequalities are therefore
opposite to sources using bx*ay-ax*by. Do not copy signs without deriving them.

Apex rays bound feasible straight continuations through processed portals. A new
right ray inside tightens it; crossing the left support forces that left corner,
which every feasible taut path must negotiate. Restart from its portal retains
unprocessed constraints. The symmetric argument handles the other side. Collinear
supporting rays and shared endpoint starts need explicit equality rules. No
repeated point increases the path output. A complete staged result alone publishes.

Validate every nonadjacent polygon pair for disjoint interiors; overlap otherwise
can invalidate a corridor argument while simple shared-edge tests still pass.
Reject nonconsecutive shared edges because they introduce an alternative corridor
topology. A supplied channel is not an automatically globally optimal mesh route.
At vertex pinches, the claim is ordered-portal feasibility; do not silently assume
finite-width clearance. The visibility oracle's random family is a simple strip
channel, and its shortest-path theorem uses its own explicit finite polygon model.

For V1 repair CCW order under reflection and reverse the list with swapped start/end;
the shortest length is unchanged, while portal orientation must be reconstructed.
For V2 prove the copied session path remains readable after scratch overwrite,
then cancel and begin another request. Mutant4 omits generation comparison and
lets the old ticket read new authority. MAX still permits cancellation; begin
must refuse rather than obstruct cleanup or create ABA. All storage here is real
typed caller storage, not a character array cast justified merely by alignment.

Assessment must require the derivations above, expected oracle output, meaningful
refusal traces, raw cost evidence and changed-condition reasoning. A green finite
campaign supports the stated author scope only. No learner score is awarded by
the existence of this solution.

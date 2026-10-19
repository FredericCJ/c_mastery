# Spatial queries with explicit geometric and storage contracts

This required Profile C extension preserves the historical PR-T14-09 elective
record. X-T14-006 through X-T14-010 are required refinements of the existing
advanced-structure and verification objectives. They implement five different
mechanisms; a scalar ordered tree or ordinary hash table is not spatial-query
coverage. Bring checked arithmetic, actual typed-array storage, recursion/loop
invariants and your PR-T14 search/hash dossier. No previous private implementation
is silently imported. All studied sorting, geometry, allocation policy, tree
update and query operations must be implemented locally.

## Common representation and publication

The C17 core uses only stdint types and actual typed caller objects. All coordinates
are signed integers in [-1024,1024]. Dimensions are exactly 2 or 3; inactive
coordinates are zero. SpBox is **closed**: touching faces and zero-volume boxes
intersect. A point has equal endpoints. Unique IDs 0..31 define result-mask bits;
equal coordinates with different IDs remain distinct objects. This is a copied
value index, not a borrowing container, capability or concurrent structure.

SpIndex owns copied items, fixed node/reference arrays, index links and counters;
no heap allocator or geometry/container library is permitted. An initialized
index must remain unmodified except through the API. Calls, including queries,
are externally serialized. All pointers are live, correctly aligned and mutually
disjoint. Use a separate actual SpIndex scratch object; its contents may change
on refusal. Input arrays are immutable and may be NULL only when n=0. No pointer
into an index is a stable reference across edits. Successful rebuilds may replace
all node identities. Any valid old index may be replaced by a successful build;
it has no external resource requiring destruction.

sp_build admits kind1..5, dimension2/3, n0..32, unique IDs and valid geometry.
It rejects malformed input with INVALID, and valid requests exceeding the
declared budget with FULL. A zero budget admits an empty build. Output remains
byte-for-byte unchanged on any refusal. No implicit failure fallback to a scan
may masquerade as a successful indexed representation. All admitted operations
have bounded work because storage and coordinate domains are finite.

sp_edit action1 inserts a missing ID, action2 replaces an existing ID and
action3 removes an existing ID (only its ID matters). Duplicate insertion is
INVALID; missing replacement/removal is MISSING. Refusal preserves the previous
index. sp_refit is a distinct BVH-only moved-box operation with the same output
rule. Query/nearest/pair outputs publish only on success; malformed query
coordinates, wrong kind or absent output return INVALID. Empty nearest is MISSING.
The caller supplies trustworthy initialized control objects; this API does not
validate arbitrary hostile internal node graphs or forged pointers.

## KD: partition, lower bounds and deterministic ties

X-T14-006 stores points in a balanced binary k-dimensional tree. A level chooses
axis depth modulo dimension. Stable insertion sorting of IDs by coordinate then
ID and selection of the median gives a deterministic partition, including equal
coordinates. Each node owns one point and the exact bounding box of its subtree.
The build never delegates to qsort. Insert, move and delete use staged rebuilding:
this is a deliberate update-cost contract, not a logarithmic dynamic-update claim.

Range traversal rejects a subtree only when its conservative bound is disjoint.
Nearest uses the sum of weighted squared distances from query point to the box;
each active weight is 1..4. The smallest ID wins a distance tie. A lower bound
equal to the current best cannot be pruned solely by distance. The maximum
distance is 3*4*2048^2, representable in uint64_t. Widen before subtracting and
multiplying. Nearest traversal orders children by lower bound but correctness
does not depend on that heuristic. A balanced tree can still visit every point;
the primary paper's empirical average is not a universal worst-case guarantee.

## BVH: conservative enclosure, refit and rebuilding

X-T14-007 stores each box in exactly one leaf. Internal nodes partition objects,
not space, and sibling bounds may overlap. Split along the greatest bound extent,
sort by twice the box center (sum of endpoints) then ID and divide equal counts.
There are exactly 2n-1 nodes for n>0. Every internal box is the union of both
child boxes. Closed-boundary contact must survive pruning.

sp_refit changes one item, preserves all child links and recomputes bounds to
the root. It can make the hierarchy inefficient without making it incorrect.
sp_edit rebuilds; compare both on the same moved geometry with equal query
answers. Nearest returns exact point-to-box distance, not distance to a hidden
triangle/mesh. This original bounded box engine is not pbrt's ray tracer,
surface-area heuristic, or proof that bounding-box overlap means shape contact.

## Quad and octree: finite subdivision and duplicate buckets

X-T14-008 uses a point-region quadtree, changed to an octree by dimension3.
Root coordinates are [-16,16) in each active dimension, represented internally
by inclusive integer endpoints -16..15. Split simultaneously at each cell midpoint;
the high half owns the midpoint. Child bit k indicates the high half on axis k.
Empty children have no node. Stop at a singleton or side length1; the final unit
cell retains all distinct IDs at that coordinate. Thus duplicates cannot cause
infinite subdivision. Maximum depth is5, derived from the coordinate lattice,
not from the number of points. A budget can refuse a valid sparse build midway;
the old index still survives. Delete/move use staged rebuilding and empty-space
compression by absence, not compressed skip-tree edges.

Closed query boxes are tested against cell bounds and then actual points.
The continuous-source half-open ownership convention becomes exact integer
ownership here. Extending this to floating point requires a new termination,
NaN, rounding and split-boundary contract; changing a typedef does not suffice.

## R-tree: overlapping branches, quadratic split and deletion repair

X-T14-009 uses maximum4/minimum2 entries per nonroot node. Root is empty or
contains one or more leaf entries, or at least two child entries. All leaves
have equal depth. Entries contain IDs in leaves and node indices in internal
nodes; each covering box tightly encloses its current contents.

Insert chooses minimum volume enlargement, then smaller volume and earlier
entry. Overflow holds a fifth transient entry, chooses quadratic seeds by maximum
union-volume waste, then the entry with greatest preference between groups.
Assignment uses least enlargement, smaller volume, fewer entries and the first
group. Forced minimum occupancy overrides the preference. Splits propagate to
the root. A node-budget failure may occur after several staged splits; publication
still waits for full success.

Delete searches all potentially relevant records, removes the ID, removes
underfilled subtrees, collects their remaining leaf IDs, tightens ancestors,
collapses a one-child root, and reinserts collected items. This bounded original
variant flattens an orphan subtree into leaf records; Guttman's original procedure
reinserted internal entries at their level. Both preserve equal leaf depth, but
their work and grouping differ. Do not report identical update costs. Moving an
object is staged deletion plus insertion, not an in-place rectangle change that
leaves ancestors stale. In three dimensions the cost heuristic is volume; zero
volumes and heavily overlapping boxes remain valid and may defeat pruning.

## Spatial hash grid and the broader partition pattern

X-T14-010 maps each closed box to every integer cell from floor(lo/width) through
floor(hi/width), width1..16. Negative C integer division truncates; explicitly
correct the remainder to obtain floor. Check the complete reference count before
filling each item's references. At most256 references are admitted. The hash uses
three defined uint32 multiply/XOR operations modulo17; overflow is intentional
unsigned reduction. A hash collision is not equality of cell coordinates.

Pair enumeration traverses bucket chains, compares full cell coordinates,
deduplicates ID pairs shared by several cells, then applies exact box overlap.
Rows[i] bit j exists only for i<j. The candidate count is the number of unique
ID pairs sharing any cell, including disjoint boxes in one cell. Thus a candidate
is not a contact. Large objects and tiny cells may exhaust storage; failure
cannot drop objects or silently omit candidate pairs. Range queries scan occupied
references and filter cell coordinates, bounded by256 even for a huge query
window; this selected policy is not an O(number of query cells) claim.

The general spatial-partition corpus topic is assessed here and by the four
hierarchies, not inflated into a sixth duplicate implementation. Compare flat
grid duplication against BVH single ownership and point-subdivision empty cells.
Movement rebuilds the copied index atomically under external serialization.
No concurrent relocation or continuous collision detection is claimed.

## Work, verification and independent transfer

D1 is the invariant/pruning proof and a hand prediction from the fixture.
D2 supplies the actual APIs, update traces and representation diagrams. D3 includes
six C17 compiler profiles, all structural invariants after every mutation,
independent Python linear queries, compiled semantic controls and unchanged
refusal outputs. D4 supplies costs, source/target distinctions and delayed variants.
The exact five anchors each reject two different causal defects, not compiler
failure or sanitizer diagnostics. The public stub compiles and fails assertions.

The Python oracle examines6976 range/nearest/pair records from actual two/three
dimensional C runs. The C harness additionally tests five deterministic seeds,
full capacity, node/ref exhaustion, duplicates, min/max cell boundaries, delete
to empty, missing IDs, malformed inputs and weighted ties. A passing finite set
is evidence under this contract; it does not prove arbitrary memory corruption,
all distributions, mastery or universal asymptotic speed.

Record node/reference occupancy, visits, exact item tests, code size and compiler
stack records. For variable n, median sorting at each binary level costs O(n^2)
with this insertion-sort construction; queries are worst-case O(n), BVH refit
O(n), and rebuilding copies O(sizeof SpIndex). Quad work is O(nD+nodes*2^d)
with D=5. R-tree costs include splits and possibly O(n) reinsertions on deletion;
overlap may force visiting every node. Grid pair work is O(R^2), R<=256, and
range work O(R). The fixed backing and disjoint scratch are both resource costs.
Compiler-emitted helpers must be absent in the strict target object/link audit.
Host printf/strtoul/memcmp and Python remain verification dependencies only.

At least seven days later, independently reconstruct changed dimension and
weights (KD), topology-preserving refit versus rebuild (BVH), duplicate-heavy
octree plus exact budget (quad), root collapse and three-dimensional overlapping
updates (R-tree), and signed-cell width changes/replication exhaustion (grid).
Predict outputs and changed proof obligations before reading assessor analyses.
Each exercise's R1-R4 rubric separates representation/proof, implementation,
failure/falsification and changed-condition cost analysis. Level0 violates a
stated property; level1 passes one case without explaining why; level2 supplies
all scoped author evidence; level3 is independent delayed reconstruction.

Hints progress from drawing geometric ownership, to identifying a conservative
bound, to isolating the first stale/omitted node. Assessor reasoning is separate
under assessor/corpus_spatial. Current execution status belongs to source-bound
receipts; no physical deployment, independent learner assessment or human review
is inferred. Security scope is local synthetic coordinates only. Adversarial
distributions test resource refusal and false-negative claims, not external targets.

# Scene and corridor contracts

These are required first-principles C17 exercises, X-T14-011 and X-T16-004. The
component is Profile C: no allocator, geometry, matrix, container, memory-copy,
sorting or pathfinding library performs the studied mechanisms. Python rational
arithmetic, the host C library in tests, sanitizers and build tools are verification
dependencies only. C17 remains the active baseline; C11 draft evidence is not
relabeled as the published C17 standard. No C23, compiler, ABI or board guarantee
is inferred from a hosted result.

## Shared ownership and effects

All controls are initialized, live, noncopied and exclusively owned. Every pointer
argument denotes a distinct actual declared object with its full stated extent;
input arrays, output, controls, handles supplied by pointer and scratch do not
overlap. Operations are externally serialized. No raw internal mutation is allowed
outside explicitly marked quiescent finite-counter test seeding. Initialization
is only for a fresh object lifetime with no outstanding handles. Reinitializing
an existing identity is not a reset operation. These are programmer obligations,
not enforced linear types or borrow checking.

All fallible functions return a status. Null required arguments return INVALID
when their operation reaches argument admission; the session admission precedence
below is explicit.
Forged/expired/wrong-owner handles return STALE after admission checks. A refusal
preserves public outputs and owning control, including cached values, dirty bits,
and work counters. Scratch may change. Refusal does not authorize ignoring a
live borrow. No hidden effects, callbacks, allocation or partial output occur.

## Scene representation and operations

CsScene owns16 typed slots, each with local and cached world3x4 affine matrices,
one parent index or sentinel16, live/dirty flags and uint32 generation. Matrices
act on column vectors; the implicit fourth row is(0,0,0,1). Thus W(child) is
W(parent)*L(child), not the reverse. Roots use their local matrix. Local linear
entries are integers[-16,16] and translations[-1024,1024]; every published world
entry must fit[-1000000,1000000]. The exact integer profile includes singular
maps, shear and reflection. It is not a glTF loader: glTF local matrices must be
TRS-decomposable, whereas this original algebra exercise deliberately admits shear.
No continuous rotation, floating inversion, render API or GPU dependency is hidden.

cs_add copies local state into the lowest free nonretired slot and issues owner,
index and generation. Incrementing a generation to UINT32_MAX is permitted once;
after removal that slot retires forever. Exhaustion returns FULL without changing
the output handle. cs_remove accepts only a leaf and otherwise returns BUSY.
Sharing an application geometry identity across nodes does not create multiple
parents: instance transforms belong to distinct scene nodes. The tree is a forest;
each live node has at most one live parent, and cycles are rejected by reparent.

cs_set changes one local matrix and invalidates every descendant, including those
previously queried and clean. cs_world walks only the dirty ancestor chain, stages
each multiplication, checks its range, then commits every cache and output
together. A second read without edits performs zero matrix multiplications.
An ancestor edit must be observed even when the next request queries a deep leaf
directly rather than traversing from the root. No background invalidation or
unsynchronized reads are supported.

cs_reparent(...,preserve_world=0) retains local state. With preserve_world=1 it
stages the old world and new parent's world, requires the parent's linear map
to be a signed permutation, derives its exact transpose inverse and translation,
and admits the resulting local matrix only if it fits the local bounds. Other
invertible matrices, singular parents and unrepresentable translations return
RANGE with the entire original scene unchanged. This is an explicit supported
inverse domain, not a false claim that every invertible integer matrix is closed
under integer inversion. The world must be recomputed after structural publication.

Each world product uses signed64 intermediates: at most three terms bounded by
1000000*16 plus a translation, so no signed overflow is required. Work is O(depth)
per dirty query and O(nodes*depth) for descendant invalidation by parent scans;
the simple O(nodes*depth) choice is intentional. Copying a fixed16-node staging
scene for preserve-world also consumes charged storage. Compare a batch of
edits followed by reads with immediate full recomputation under identical queries.

## Convex corridor representation and funnel

nv_path accepts an **ordered corridor**, not an unordered navigation mesh:
1..8 polygons, each3..6 distinct integer vertices[-64,64] in strictly convex
counterclockwise order. Every non-edge vertex must be strictly left of each
directed edge. Polygons have disjoint interiors, tested by separating axes.
Consecutive polygons share exactly one complete reversed edge; nonconsecutive
polygons must not share an edge. Shared vertices are allowed. Start lies in the
first closed polygon and end in the last. Invalid winding, overlap, disconnection,
out-of-range coordinates or incomplete portals return INVALID. Mesh generation,
A* corridor selection and arbitrary terrain projection are separate mechanisms;
this exercise must not claim minimum path over every possible corridor.

Shared edges become ordered left/right portals viewed in traversal direction.
Degenerate start and end portals frame them. nv_path maintains an apex and two
supporting rays, narrows them with exact signed orientation, emits the opposite
corner when a ray crosses, and restarts at that corner's portal. Equality is
exact; there is no arbitrary floating epsilon. Cross products fit signed32
(differences at most128, determinant magnitude at most32768). Repeated output
points are suppressed, including start=end. The result is the taut point-agent
path through the ordered portals. It does not establish collision clearance for
a finite-radius robot, steering dynamics, shortest route through another corridor,
or arbitrary three-dimensional terrain.

NvScratch is an actual typed object containing10 left/right pairs and a staged
10-point path. portal_budget and point_budget are each0..10, model independently
available request storage, and are tested at exact need and one less. There are
n+1 required portals. The restarted funnel can revisit portals; use O(n*n) time
and O(n) scratch bounds for this implementation rather than citing a queue-based
linear algorithm. On capacity failure the previous caller path is unchanged;
no successful prefix masquerades as a complete route. Path publication copies
points, so scratch can immediately be reused or overwritten.

NvSession owns a copied path and monotonic uint32 request generation. nv_begin
is admitted only when inactive; it computes privately, then publishes path and
ticket atomically. After checking the session and ticket-output pointers,
nv_begin returns BUSY for a live request, or FULL for an exhausted inactive
session, before inspecting corridor, scratch or capacity arguments. Neither
refusal dereferences those deferred arguments or changes the prior ticket.
nv_copy validates owner, active
state and exact generation. nv_cancel revokes the ticket without reusing its
generation. A later begin issues a new one; MAX may issue once and cancellation
still works, but no subsequent begin wraps. This is synchronous cancellation
of stored output, not asynchronous device cancellation or quiescence. Caller
paths copied earlier remain independent values; ticket revocation cannot erase
copies already transferred to another owner.

This lifetime split is the T16 refinement: immutable corridor input lives across
requests, typed scratch belongs to one computation, and published path storage
belongs to a generation-scoped session. Charge all three regions. A declared
unsigned-char arena is not silently cast to typed polygon or path objects.

## Concrete workloads, variants and evidence

Scene anchor: a quarter-turn Z rotation plus translation(10,0,0), followed by
local translation(3,0,0), yields world translation(10,3,0). Editing its parent
to translation(20,0,0) yields(23,0,0). Query twice; the second work count is zero.
Exercise leaf removal, wrong owner, stale reuse, MAX retirement, cyclic reparent,
unsupported preserve-world scale, and a six-node scale16 chain that refuses
world range without changing any prior cache/output. The generated workload
has256 edits and4096 actual world observations compared with uncached Python
homogeneous products. Include shear/reflection and a deep direct leaf query.

Scene V1 changes the transform family and parent structure: reflect/shear across
three axes and reparent while preserving local state; rederive multiplication
order. V2 preserves world under a signed-permutation parent, then changes the
parent to scaling/singular and requires honest transactional refusal, stale
generation rejection and retired-slot cleanup. Neither is a constant substitution.

Corridor anchor is the three square L-shaped union with start(1,1), end(7,7)
and corner(4,4), plus a genuinely bent start(1,3) variant. Triangles, portal
endpoint start and start=end establish the equality rules. The generated workload
has160 differently bent channels, each rotated, reflected (winding repaired),
scaled by2, and traversed in both directions:1280 actual paths. An independent visibility graph uses rational
segment clipping against every polygon and Dijkstra over corridor vertices;
its Euclidean-length tolerance is1e-10. This is an independent bounded oracle,
not reuse of the funnel code or a formal real-arithmetic proof.

Corridor V1 transforms geometry by rotation/reflection/scaling and reverses
traversal; recompute winding/portal ownership rather than assuming point order
survives reflection. V2 reuses scratch across cancellation/new generations,
exhausts portal and path budgets, and verifies no stale ticket or partial route
becomes authoritative. Submit region-use diagrams and actual sizeof storage.

Run corpus_scene.1/.2, both distinct compiled mutants per anchor, the independent
oracle and six GCC/Clang C17 debug/release/ASan+UBSan configurations. A mutant
must compile and fail CS_CHECK exit41, without sanitizer failure. Target object
and linked-image audits must name implicit helpers and libraries; compilation
is not target execution. Passing these finite checks is neither an independent
learner assessment nor absolute mastery.

## Assessment and bounded security

For each criterion: level0 breaks the contract or overclaims evidence; level1
passes the supplied example without invariants; level2 supplies the implementation,
counterexample, independent checks and resource accounting; level3 independently
reconstructs both changed conditions at least7days later. Graduated hints: first
draw ownership and orientations; then compute one noncommuting product or ray
crossing by hand; finally stage a full operation and remove one invariant repair.
Assessor technical solutions remain in assessor/corpus_scene/ANALYSIS.md.

Adversarial geometry and stale tickets target only these original local arrays.
Attacker capabilities are bounded admitted coordinates, capacities and previously
issued tokens. The properties are correct geometric output, refusal preservation
and stale-authority exclusion. Harmless success is a wrong matrix/path or wrongly
accepted old ticket; repairs restore invalidation, order, orientation and identity.
No malformed pointer, external game server, real navigation device, arbitrary
code execution or physical safety claim is authorized. Caller-control corruption,
concurrent mutation and opaque mesh-file parsing are outside this API's premises.

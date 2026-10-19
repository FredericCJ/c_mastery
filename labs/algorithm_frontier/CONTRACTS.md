# Source-preserving graph frontiers: PR-T36-07

This required successor implements the exact `T36.09.a/b/c` ordinary criteria and
their original `AX-PR-T36-07-T36.09.*-V1` tasks. Preserve the original project,
unit/prerequisite identities, seven-day delay and objective rubric. Historical
hours are descriptive, not a limit on this work. The project's generic working
card mentions unrelated sorting kernels; the operative graph contract and this
explicit successor define the present work. No new assembly graph solver is
required, and no missing graph mechanism is credited to a sorting heading.

## Prerequisites and environment

Import completed learner T14 `CDCSR`, `CDRing`, `CDHeap` and `CDHeapHandle`, and
T15 `ad_bfs`/`ad_dijkstra`. Their exact public contracts are in
`labs/containers_depth/CONTRACTS.md` and `labs/algorithm_depth/CONTRACTS.md`.
Identify original function/header hashes and retain prior evidence. Adapt only
frontier operations, admission and certificates; do not rebuild a complete
solver, queue or heap. The author reference derives the selected actual T15
function bodies using asserted single-occurrence substitutions and links the
unchanged originals for comparison. A learner may adapt their own completed
producer through equally explicit, reviewable patches.

Use ISO C17 Profile C with actual typed objects. The selected Linux x86-64 and
Cortex-M4 targets have eight-bit bytes and an `int` that represents524279. Do
not delegate the assessed algorithms to a library. Python exact arithmetic/path
enumeration, hosted printing and sanitizers are host verification dependencies.
The Cortex image uses original reset/semihost code, caller storage and explicit
byte helpers for compiler-lowered aggregate copies, with no hosted runtime.
Audit object/final symbols, ELF headers, map and disassembly including implicit
helpers. Exact final C17/C23 text remains uninspected; no other edition's draft
is relabeled C17. ABI/layout observations are separate from ISO semantics.

## Inputs, storage and failure contract

`ag_run` accepts1..8 vertices,0..24 directed `CDEdge` records, a valid source,
parallel flag0/1, a live `AGWorkspace`, queue capacity0..8 and distinct result/
trace controls. Input, workspace and outputs are disjoint and remain live during
the synchronous call. There is no concurrent mutation, reentry or asynchronous
callback. Zero edges permit null input without pointer arithmetic. Vertex0..n−1
is identity; input order determines order among edges from one source.

The actual CSR builder copies edges into its typed storage and validates endpoints.
Parallel flag0 rejects repeated ordered endpoints; flag1 retains each occurrence
and weight. Negative weights are rejected globally, including unreachable edges.
Queue capacity below vertex count rejects before frontier work. No dynamic
allocation occurs. The workspace owns a real8-int ring backing array, actual
prior heap and eight actual handles. All handles remain internal to this call;
no caller may retain them across reinitialization. The caller owns the workspace,
which is scratch and may change after admission, including on a later range error.

Finite accepted distances are0..65534. Reachability is a separate bit mask;
unreachable distance/hops fields are0 and predecessor8. The source has distance/
hops0 and predecessor8 in the public result. Internal producer sentinels and its
self predecessor are converted explicitly. Every candidate from a reachable
settled vertex is range checked before addition, including an edge to an already
settled vertex. Thus a candidate65530+10 rejects the entire result even if another
route is shorter. An unreachable positive edge of weight `INT_MAX` is never an
examined candidate. Invalid/range/space failures preserve result and trace; they
do not promise to restore the scratch workspace. No arbitrary pointer-liveness
or alias validation is possible in ISO C: those are caller obligations.

## Mechanism and necessary distinctions

BFS uses the actual ring push/pop and preserves T15 discovery semantics. Its
FIFO frontier orders hop layers. Dijkstra uses the actual min heap with one item
per open vertex and handle-based decrease-key; it preserves T15's strict
improvement rule and range admission. The prior heap's equal priorities keep
insertion order, so encode `distance*8 + vertex` to enforce vertex-ID ties. The
maximum524279 fits the selected `int`. Equal distances then choose lower vertex
IDs independently of insertion order. Each vertex is inserted at most once,
because settled vertices are not reopened; at most eight handles are created.
This also avoids exhausting the prior heap's bounded slot generations/sequence.

Trace every BFS frontier before pop, every weighted frontier before settlement,
and every strict distance improvement. Retain seen/settled sets, tentative labels,
actual frontier identities/priorities, CSR edge index and old/new distance. The
heap backing order is an implementation representation, not sorted output.
Its root must be the minimum encoded priority and each parent's priority must
be no greater than its children's. Do not claim a final distance array alone
demonstrates the frontier invariant.

The original A/B/C/D/E graph gives weighted0,2,1,3,unreachable; hops0,1,1,2,
unreachable; weighted settlement A,C,B,D. The queue instead processes A,B,C,D.
B and D each receive an improvement while open. Explain these states before
running the fixture. A valid parent chain only exhibits some path and therefore
an upper bound; it does not by itself prove optimality.

## Deliverables and verification

- D1: graph, type/ownership and arithmetic contracts; distinct BFS/weighted
  traces; connected historical presentation versus this source-reachability API.
- D2: reviewable patches to the actual prior functions, queue/heap/CSR imports,
  checked public adapter, staged results/traces, exact tool/link identities and
  boundary/cost ledger. Keep the original completed producers unchanged.
- D3: independent simple-path and certificate checker, emitted trace replay,
  range/output preservation, isolated/parallel/self/zero-edge cases, and intended
  semantic mutation diagnoses. Submit actual raw observations, not just PASS.
- D4: all three original changed-condition tasks below, plus the original
  Problem1/Problem2 transfer comparison, with fresh delayed reasoning.

Compile and run the public tests with your completed prerequisites and adapters.
`prerequisite_stubs.c` supplies link-only placeholders and is not a completed
producer. The author runner uses GCC/Clang debug, optimized and ASan/UBSan
profiles, original learner-stub rejection, and two strict Cortex-M4 emulator
profiles. It checks final image closure separately from hosted sanitizers.

The independent oracle enumerates all simple paths on729 three-vertex graphs
with each directed nonself edge absent, zero or weight2, and all three sources.
It checks complete results and replays the actual emitted frontier/improvement
packets. Six additional fixture/boundary/parallel graphs bring the count to2,193
packets per native profile. Exponential enumeration is deliberately confined to
tiny graphs; it is not the target solver. Every claimed mutation must fail its
intended assertion: omitted range admission, insertion-order ties, skipped real
heap decrease-key, or omitted certificate edge inequalities. A compile failure,
timeout or unrelated diagnostic is not a successful semantic mutation control.

Count queue pushes/pops, heap inserts/changes/takes, actual heap comparisons and
relaxations. The fixture has four inserts/takes and two updates; three successively
shorter parallel edges to one vertex cause one insert and two updates. These are
mechanism counts, not latency. Derive O(V+E) traversal work and O((V+E)logV) heap
frontier work separately from CSR building/duplicate validation, trace writing
and certificate checking. The original no-parallel builder scans for duplicates,
so its O(E²) admission cost must not disappear from an end-to-end claim. Account
for the retained sixteen-entry heap, eight handles,32-byte ring backing, local
CSR and staged trace, not just the logical number of open vertices. Native and
Thumb handle sizes differ; record target `sizeof`/stack/code measurements before
making a storage or timing comparison.

## Original variants and independent witness

`AX-PR-T36-07-T36.09.a-V1`, minimum seven days: change the graph to A→B1,
A→C2,C→B−3. Derive the first invalid settlement argument and compare its early
B1 with the actual shortest B−1. The nonnegative public solver must reject this
graph; bypassing its check does not turn it into a negative-edge solver. Explain
which inequality used nonnegative suffixes and why a later relaxation matters.

`AX-PR-T36-07-T36.09.b-V1`, minimum seven days: admit parallel edges explicitly.
Retain each edge identity, explain CSR ownership and deterministic source order,
and compare one insertion plus repeated decrease-key with a separate lazy
duplicate-entry design. Implement/test the selected one-item-per-vertex policy;
derive the other policy's additional storage, stale-entry filtering and failure
admission before claiming equivalence. Preserve input bytes and old outputs on
rejected duplicates/range failures. Equal-cost edges do not replace a parent.

`AX-PR-T36-07-T36.09.c-V1`, minimum seven days: derive and implement the signed
certificate checker for negative edges without a reachable negative cycle.
`AGCertificate` holds signed64 distances, a reachability mask and predecessor
vertices. Its admitted finite labels are−65534..65534; edges retain the selected
32-bit `int` weight. Validate all labels before any sum, so an arbitrary forged
`INT64_MAX` label cannot cause signed overflow. Every reachable vertex must have
a source-rooted, acyclic parent chain with an actual edge realizing each label
equation. For every edge from a reachable vertex, its destination must be
reachable and `d[to] <= d[from]+weight`. Unreachable labels/parents are ignored.
The source label is0 and its certificate predecessor is itself, distinct from
the public result's source predecessor8. The checker returns status only.

Prove that realized parent paths give upper bounds while edge inequalities along
every path give lower bounds. A reachable negative cycle makes those inequalities
inconsistent; an unreachable negative cycle does not invalidate source distances.
The checker verifies supplied witnesses, not a general algorithm for discovering
them. The independent tiny-path model and actual signed witnesses are executable;
the general proof and arithmetic premise remain assessed reasoning obligations.

For the project's Problem1/Problem2 transfer, use an undirected triangle with
edge weights AB2, AC3, BC2. A minimum total spanning tree has weight4; the source-A
shortest-path tree has weight5. The former's A→C distance4 fails the unused AC3
edge inequality. No new spanning-tree implementation is required or claimed.

## Rubric, hints and evidence limits

For `.a`, level2 requires both actual frontier traces and the settlement premise;
level3 diagnoses the negative-edge changed case without copying a trace. For
`.b`, level2 requires real producer calls, exact range/publication behavior and
disconnected results; level3 derives and verifies parallel-edge ownership/ties/
update costs. For `.c`, level2 independently checks complete paths and falsifies
a parent-consistent nonoptimal certificate; level3 proves and applies the signed
witness conditions and arithmetic limits. Replaying expected arrays is level1;
unexplained violations are level0. The original rubric/variant IDs remain binding.

Graduated hints: draw the frontier and distinguish tentative from settled;
next examine the first failed improvement or heap update; finally separate the
parent-path equation from constraints on unused edges. Record an unaided attempt
before using a hint. Assessor analysis, generator, reference adapter and oracle
remain under `assessor/algorithm_frontier/`, separate from the learner scaffold.

Use only original local synthetic graphs and dedicated emulation. No external
systems, user data, host policy change or board flashing is authorized. Harmless
success conditions are wrong results, an invalid certificate accepted by a mutant,
or violated staged-output contracts; none implies arbitrary code execution.

Current exact author commands and source/output pins are in
`research/v5/expansion/legacy_algorithm_frontier/validation.json`; actual export
compilation is in `exports.json`. They are not external review, independently
administered delayed learning, physical timing or absolute mastery. PR-T36-08
and all T37 original/changed work remain separate and required.

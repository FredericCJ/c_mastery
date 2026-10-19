# Assessor analysis: graph frontier refinement

Do not ship this analysis, generator, oracle or implementation in learner exports.
Assess each unchanged T36.09 objective and V1 task independently after its delay.

## T36.09.a: distinct frontier invariants

The fixture BFS frontiers are `[A]`, `[B,C]`, `[C,D]`, `[D]`, with distance layers
0,1,1,2. Weighted frontiers contain tentative B4/C1 after A; after C, B2/D6;
after B, D3. Settlement order is A,C,B,D and E remains unreachable. The actual
trace is emitted before each frontier removal, so its done mask excludes the
selected item. An evaluator that mistakes after-pop membership for before-pop
membership will reject a correct packet; the contract fixes that boundary.

The source-derived generator retains discovery/strict-improvement/range logic
from the actual T15 functions. Its substitutions replace the physical queue and
minimum selection/update operations, add trace calls and constrain the limit.
It does not implement a second untraceable complete solver. Compare generated
function hashes and exact generated text; changing a prior function invalidates
the recipe until reviewed. The unchanged prior functions remain separately linked.

For A→B1,A→C2,C→B−3, B is prematurely settled before C and remains1 under a
no-reopen algorithm, although path A,C,B totals−1. The failing argument is that
an extension of a path to an unsettled vertex cannot later reduce a settled
minimum. The accepted nonnegative API rejects the graph globally. Its negative
diagnostic is not a promise that the heap algorithm supports negative edges.

## T36.09.b: real handles, ties and failure

T14's heap compares equal priorities by insertion sequence; encoding only distance
would inherit edge-insertion order. Encoding `8*distance+vertex` supplies the
required lexicographic order because vertices are0..7. This is an integer adapter,
not a modified heap comparator. Root-selection tests intentionally insert C before
B at equal distance. One live handle per open vertex supports actual decrease-key;
the snapshot checks both encoded priority and parent heap order, so a skipped
update is detected even when one final distance array happens to remain correct.

The handles borrow the stable workspace heap; no handle escapes or survives
reinitialization. At most eight vertices are ever inserted, so the bounded
sixteen-slot provider has adequate generation/slot headroom without silently
changing its retirement policy. Parallel edge A→B9,4,2 creates one B entry and
two actual changes, not three owned heap items. The copied CSR preserves input
edge order within a source. Equal candidate distances retain the first strict
improvement and do not create zero-weight predecessor cycles.

The whole adapter stages output and traces. It may consume scratch workspace
before rejecting a later65530+10 candidate; it makes no scratch rollback promise.
The range check is before addition and applies to every examined outgoing edge,
not just one that would improve an unsettled label. Do not infer reachability
from an internal sentinel after narrowing to32 bits; public mask conversion is
explicit. Duplicate-endpoint rejection and short queue admission are separate
from later arithmetic rejection.

## T36.09.c: independent proof and negative transfer

Simple paths are independently enumerated on729 bounded nonnegative graphs and
all sources. Nonnegative cycles cannot improve a shortest distance, so simple
paths suffice there; the oracle is exponential and deliberately tiny. Complete
frontier/result packets, predecessor equations, actual open membership and every
relaxation are replayed. The actual source's scan-min implementation is a useful
differential comparison but is not the independent simple-path oracle.

A claimed B5 via direct A→B5 is parent-consistent in the graph A→C1,C→B1,
yet violates the unused C→B inequality. The signed witness variant checks all
reachable labels' finite range before arithmetic, source-rooted paths, valid
edges/equalities and every reachable edge inequality. Adding along any source
path gives label<=path length; a realized parent path gives path length=label.
Together these prove shortest distances. Summing strict negative-cycle reduction
would require d[v]<d[v], so no valid reachable-cycle witness exists. An unrelated
unreachable negative cycle is outside the source-reachability closure.

The actual changed witness is `[0,-1,2]` with B's parent C. The early-settle
`[0,1,2]` witness is rejected. A reachable negative cycle and a forged `INT64_MAX`
label are also rejected before unsafe arithmetic. Parent predecessor IDs alone
do not identify which parallel edge realized equality; checking that at least
one exact edge exists is sufficient for this value witness. An edge-identity
provenance requirement would need an extended certificate.

The triangle AB2/AC3/BC2 distinguishes minimum spanning total4 from source-A
shortest-path tree total5. Its spanning-tree C distance4 violates AC3. The inspected
1959 paper separates these two problems and presents connected inputs; this API
adds an explicit disconnected/unreachable contract. Its historical work comparison
is not a modern timing observation. No finite packet set, sanitizer result or
emulator run proves universal correctness, independent learner mastery or physical
performance.

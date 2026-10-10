# Contracts and evidence limits

All capacities are finite teaching bounds. Public structure fields are visible
for invariant inspection; clients must initialize them and must not corrupt
representations between calls. Live backing objects, disjoint output where a
function requires it, and valid lifetimes remain caller obligations. A supplied
capacity cannot prove pointer provenance. Functions return the enum in `algo.h`;
unsuccessful queries preserve their output. Null data is permitted only when its
length is zero. Initialize optional work counters to zero before a measured run.

| Mechanism | Representation and obligation | Independent evidence |
|---|---|---|
| Ring | Capacity 1–16; head identifies oldest element; count separates full from empty; failed full insertion changes no state | Wraparound trace, unchanged full state, empty output preservation |
| Hash | 16 slots, low-four-bit hash; states 0 empty, 1 live, 2 tombstone; every search bounded by 16 probes; duplicate replacement scans past earlier tombstones | 6,000 mixed operations against an independent 32-key presence/value map, deliberately colliding keys |
| AVL | 64 typed slots; signed keys, index -1 for absent child; height null=0, leaf=1; distinct keys; duplicates are successful no-ops | 6,000 operations against a presence map; independent recursion checks bounds, cycle freedom, strict order, stored heights, balance and live/reachable equivalence after each operation; full capacity and reuse |
| Disjoint sets | At most 64 elements; parent forest, union by rank and path compression | Label-replacement partition oracle across all element pairs |
| Graph | Directed adjacency matrix of at most 16 nodes; `UINT32_MAX` means no edge and unreachable; other weights nonnegative | BFS against unweighted Floyd-Warshall; Dijkstra against uint64 Floyd-Warshall on 100 bounded random graphs including zero edges and disconnected vertices |
| Lower bound | Sorted unsigned words, n≤1024; return first index with value≥key or n; checked function validates order | Linear scan oracle, duplicates, high bit, null/empty and invalid order; ≤8 search decisions for tested n≤128 |
| Stable sorts | Whole `{uint32_t key, tag}` records; C insertion/merge n≤1024; unsigned key; equality retains original order; fixed 8KiB merge scratch | Independent standard-library sort by `(key, original ordinal)` validates value and stable permutation; rejection before mutation |
| Sift down | n≤1024; every heap edge is valid except possibly children of p; if p has a parent, it also dominates both children of p; equal children choose left; stop on parent≥child | Declared fixture has 3 comparisons/1 swap, invalid premise rejection; assembly suite verifies heap edges and independent multiset preservation |
| Byte match | Length-delimited bytes including zero; empty pattern matches zero; first match or unchanged missing output | Exhaustive binary alphabets for texts length0–8 and patterns0–4; independent bounded memcmp oracle |
| Byte arena | Caller byte array; power-of-two alignment of actual address; subtraction before bounds checks; mark is an earlier `used` offset from this arena's current lifetime | Deliberately offset base, alignment, huge request, invalid alignment, failure atomicity and rewind |
| Typed pool | 16 int32 slots; handle high32 generation/low32 slot; released generations advance; generation maximum retires slot | Full pool, stale/double release, slot reuse and generation exhaustion |
| Numerical | Neumaier sum of ≤1024 finite doubles, reject nonfinite intermediate/final sum; Q7 MAC≤16 products, int32 accumulation, nearest ties away from zero, final signed saturation | Cancellation fixture, nonfinite/overflow outputs unchanged; all 65,536 single-product Q7 pairs compared to floating arithmetic rounding, max MAC |
| ALU/cache | Boolean full-adder slices form 4-bit add with carry/overflow; cache is 4 direct-mapped 16-byte lines and 4-line fully associative LRU shadow, at most32 references | All512 ALU triples; declared traces distinguish compulsory/conflict/capacity misses |

Dijkstra rejects a candidate for an unsettled vertex when its finite distance
would reach or exceed `UINT32_MAX`, even if an alternate shorter path could later
avoid it. It publishes no distances on that failure. This conservative contract
is explicit; it is not arbitrary wraparound or unbounded mathematical infinity.

The arena returns byte offsets. It does not promise that a declared byte array
may be accessed as an arbitrary struct under C's effective-type rules. Marks are
not generation handles and are not transferable between arenas. The separate
typed pool demonstrates stale-handle detection without that aliasing ambiguity.

`algo_work.comparisons` counts search/sort key comparisons; insertion `moves`
counts each whole-record shift and each final placement (including placement
without movement). Thus the five-record fixture has six shifts, four placements,
nine comparisons. Heap `moves` counts swaps. Hash `visits` counts slots inspected;
graph `visits` counts scanned matrix entries. These units are different, declared
models. Validation scans, loop arithmetic, bytes and cycles are not silently
included. A checked lower-bound call is O(n) because it validates sortedness;
the trusted search kernel is O(log n). AVL updates use O(log n) rotations/path
work but allocation currently scans the fixed 64-slot pool; derive that term.

## Assembly boundary

Implement all three trusted kernels in both `.S` files. Lower-bound n≤1024,
stable insertion n≤32, sift n≤1024. Call only with valid backing objects and
the stated semantic premises. `adapters/checked_asm.c` supplies the complete
checked facade; it rejects invalid counts, order or heap premise before entering
assembly. It cannot validate a forged live pointer.

Sift's admission and guarantee concern the whole heap: all edges except those
from `p` to its children must already satisfy the max-heap order. When `p` has
a parent, that parent must also dominate both children of `p`; this bounds the
maximum that could be promoted. A rejected premise leaves every element intact.
The trusted assembly kernels require the same premise. Merely validating the
unchanged edges is insufficient: repairing index1 of `[10,1,9,20,0]` would promote
20 above its parent10, so the checked facade rejects that input.

The x86 route is Linux SysV scalar integer/pointer calling convention; it is not
Windows x64. The Thumb route is base scalar AAPCS32, Cortex-M4, no VFP arguments.
Probe all preserved integer registers, preserve the stack pointer, respect
alignment before nested calls and return correctly through LR/return address.
The harness seeds preserved registers and perturbs available caller scratch.
Each of the three kernels is invoked through the probe. Native guard-page tests
place a one-word array immediately before an inaccessible page, including an
empty range at that boundary. Hardware faults beyond this admitted test are not
sanitizer evidence; assembly accesses are not ASan-instrumented.

Native tests use 1,000 generated arrays and Thumb uses500, with independent
permutation/order checks. Maximum repeated-key lower-bound arrays contain1024
words. The assembly mutant families replace unsigned with signed branch,
shift equal keys, or clobber a preserved register. They must fail for a semantic
reason. These finite cases do not prove arbitrary assembly correctness. Retain
the loop invariant, address proof and ABI ledger alongside results.

## What remains project work

This family does not implement dynamic arrays, linked containers, tries,
advanced structures, quickselect/radix sorts, DAG topological sort, negative-edge
paths, general malloc/free, slab/buddy allocators, matrix/filter/ODE kernels,
instruction decoders, pipelines, DMA or physical measurement. Each project
mapping in `family.json` states its subset. The AVL implementation does not
substitute for its mathematical height proof. A fixed hash with adversarial
collisions supports no expected constant-time claim. Numerical fixtures are not
an error-bound proof for every floating-point input. The cache is a declared
educational model, not the host's cache implementation.

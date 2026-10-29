# Corpus-discovered index mechanisms, contract 1

These four required additive exercises retain the C17 baseline and historical
project records. Their parent projects' old elective labels and hours do not
limit this required extension. Implement the mechanisms in `src/index.c` using
the interfaces in `index.h`. The default workspace contains compiling stubs,
public tests and a text driver; no assessor implementation is distributed.

## Environment and common admission

The original component uses profile C: C17 expressions, actual typed automatic
or caller-owned objects, fixed arrays and explicit loops. No allocation,
container, sorting, search or arithmetic-coding library may perform the studied
mechanism. `stdint.h` and `stddef.h` describe types; the hosted test/driver may
use the standard library. Require 8-bit bytes and the exact-width integer types
in the header. These are implementation assumptions, not guarantees of every
ISO C implementation. A target claim additionally requires the generated-code
and final-link audits below, including compiler-inserted helpers.
The explicit [dependency contract](dependency-contract.md) lists the permitted
compiler-support requirements and separates this component audit from a strict
whole freestanding image.

Callers provide live, initialized objects after successful initialization and
serialize access. Control objects, outputs and all input spans are disjoint,
including outputs of the same call. A null zero-length input span is admitted;
other required null pointers return `CI_INVALID`. Non-null pointer validity,
object lifetime, span length and disjointness are caller preconditions: an
arbitrary address cannot be validated by portable C. Initialization may replace
a previously initialized value. Rejected operations preserve the entire existing
control object and all output values. Null arguments are checked before range
errors. The implementation does not attempt to repair caller-corrupted internal
state. No callback, threading, persistence or device effect is part of this API.

`CI_RANGE` denotes an inadmissible index, size, value or interval; `CI_FULL`
denotes exhausted node storage or insufficient result capacity;
`CI_DUPLICATE` and `CI_MISSING` have their stated key/search meanings. All
admission must precede publication; fixed-capacity designs do not excuse a
partially applied mutation on rejection. Unsigned wrap is not an accepted
approximation to the mathematical frequency sum.

## Fenwick frequencies

`struct ci_fenwick` represents n nonnegative uint32 frequencies, 0<=n<=64, with total
at most UINT32_MAX. `init` consumes n values and copies their meaning; no input
borrow survives. `add(i,delta)` applies an int64 delta only if i<n, the resulting
point frequency is nonnegative and the new total fits uint32. In particular
INT64_MIN is an admissible input to diagnose and reject without negating it.
`prefix(end)` returns sum of [0,end), admitting end in [0,n]. `select(rank)`
returns the smallest i whose prefix(i+1)>rank, for 0<=rank<total. Zero-frequency
gaps are skipped. An empty or all-zero table has no valid rank.

The one-based representation stores at bit[i] the sum over the last lowbit(i)
frequencies ending at i; bit[0] is unused. State this index translation before
comparing the inspected paper's zero-origin algorithms and its published erratum.
`rescale(round_up)` replaces EVERY point frequency by floor(f/2) for 0 or
ceil(f/2) for 1, rebuilding the tree. This is not division of each tree cell.
Other values of round_up return CI_RANGE. For input [1,0,3], prefixes are
[0,1,1,4], ranks0..3 map to [0,2,2,2], floor rescale gives [0,0,1] and
ceil rescale gives [1,0,2]. Derive O(log n) prefix/update/select and O(n)
initialization/rescale, with O(n) typed storage/scratch. Count exact iterations
separately from this asymptotic family argument at the current capacity64.

## Ordered segment aggregate

`struct ci_segment` stores n residues in [0,65521), n<=64. `query(l,r)` returns the
half-open sequence aggregate with hash H=fold(h,x -> (257*h+x) mod65521),
power=257^(r-l) mod65521, geometric=sum(257^j,j=0..r-l-1) mod65521 and length.
The empty aggregate is (0,1,0,0). Joining L followed by R yields
(L.hash*R.power+R.hash, L.power*R.power,
L.geometric*R.power+R.geometric, L.length+R.length), reducing the first three
modulo65521. This operation is associative and generally NOT commutative.
This small polynomial residue is an order-sensitive test aggregate, not a
collision-free equality test or cryptographic authentication mechanism.

`apply(l,r,a,b)` maps every selected x to a*x+b mod65521. Admit 0<=l<=r<=n
and a,b<65521; an empty update succeeds unchanged. a=0 means assignment and
must remain representable. Compose newer map f after older map g as f(g(x)).
The aggregate action changes hash to a*hash+b*geometric and preserves the other
fields. A query must not mutate the tree. Derive O(n) build, O(log n) contiguous
update/query with O(n) stored nodes and bounded logarithmic call depth. For
[1,2,3], H=66566 mod65521=1045; applying x->2x+1 to [1,3) gives [1,5,7],
whose H=67341 mod65521=1820. Verify these independently before coding.

## Augmented interval tree

`struct ci_intervals` owns64 actual typed nodes, a free-list and an AVL search tree keyed
lexicographically by (low,id). The id is a tie breaker within a low endpoint;
it is not globally unique. Each node stores exact subtree maximum high and AVL
height. No public operation returns a node pointer or an identity that survives
removal. `init(closed)` chooses half-open intervals [low,high) at0 (low<high),
or closed intervals [low,high] at1 (low<=high); the same convention applies to
queries. Invalid conventions/endpoints return CI_RANGE. Key duplication is
checked before capacity, so an existing key in a full tree returns CI_DUPLICATE.
`remove(low,id)` returns CI_MISSING without mutation when absent. Two-child
deletion and rotations must repair all affected ancestor metadata.

`find` returns any overlapping interval, or CI_MISSING preserving its output.
`all` returns every overlapping interval in increasing (low,id) order. The
caller supplies capacity and a distinct count output; insufficient capacity
returns CI_FULL with neither output array nor count changed. NULL output is
admitted only when capacity0. With half-open [1,3) and [3,5), query [3,4)
matches only the second. With closed endpoints the analogous [3,3] matches both.
Use exact inequalities to derive pruning; a stale underestimated maximum can
hide a true match. Derive O(log n) insert/remove/find and O(n) worst-case all,
including staged-result scratch O(n). Do not label this particular enumeration
O(log n+k) without proving that bound for its actual traversal.

## Suffix index and repeated spans

`struct ci_suffix` owns a copied text of n<=64 unsigned bytes, including embedded0 and255.
Its order is a permutation of offsets0..n sorted by lexicographic suffix order;
offset n is the empty suffix and sorts first. `lcp[0]=0`; other entries are
the exact adjacent longest common prefix length. Construction must use rank
doubling and stable integer-key distribution, not qsort/strcmp or materialized
copies of every suffix. A missing rank component is a sentinel below all bytes;
it must not confuse byte0 with text termination. Prove rank refinement and
derive O(n log n) construction and O(n+alphabet) scratch; derive the linear
adjacent-LCP scan separately from the inspected paper's construction.

`range(pattern,m,&lo,&hi)` returns a half-open interval in suffix-order positions
whose text suffixes begin with the exact m-byte pattern. Admit m<=64; empty
pattern matches all n+1 suffixes. Absent patterns return equal insertion bounds.
Plain binary search comparing up to m bytes costs O(m log n), not the paper's
LCP-accelerated O(m+log n) bound. `lce(a,b)` accepts offsets0..n and returns their
common prefix, using the minimum adjacent LCP between their ranks; identical
offsets return n-a. The supplied interface does not require an O(1) query index.

`longest(disjoint,&a,&b,&length)` finds two DISTINCT text start offsets a<b with
the longest equal spans, then chooses smallest (a,b) on ties. At disjoint0,
overlap is permitted; at1 require a+length<=b. Other modes return CI_RANGE.
No positive repeat returns (0,0,0). For `banana` the suffix order is
[6,5,3,1,0,4,2], LCP [0,0,1,3,0,0,2], pattern `ana` range[2,4), overlapping
longest (1,3,3) and nonoverlapping longest (1,3,2). A correct O(n^2) pair scan
with running LCP minima satisfies this bounded interface; disclose its cost.

## Required evidence and changed conditions

Keep a mechanism proof, source comparisons, failure snapshots and resource
ledger for EACH exercise. Build GCC/Clang at O0 and O3 and under AddressSanitizer
plus UndefinedBehaviorSanitizer; record versions and actual commands. Inspect
undefined symbols and stack-usage output for the core alone. Hosted Python
oracles, C stdio and sanitizer runtimes are verification dependencies only.
Cross-compile strict freestanding Cortex-M4 objects when claiming portability;
do not silently supply an unexamined division/memory helper. If no target audit
has executed, retain that limitation rather than inferring it from hosted tests.

Tests must compare a flat frequency vector, flat sequence, unsorted interval
model and lexicographically sorted copied byte suffixes, respectively. These
representations are intentionally different from the learner algorithms.
Include exact rejection atomicity, empty/singleton/capacity states, repeated
updates/deletions and changing endpoint/rank assumptions. Kill both specified
semantic mutants per exercise with ordinary property failures. A crash alone
does not establish which invariant failed. Test the public compiling stubs to
ensure the acceptance command rejects missing implementations.

The named delayed variant requires a source-bound study event at least7days
earlier and an independently authored implementation/argument. Author reference
execution does not stand in for learner transfer. Each exercise's three rubric
criteria require level2 for the ordinary contract and level3 reasoning for the
changed condition. Evidence does not prove absolute mastery or unbounded
correctness. External review, independent learner performance and physical
observation are distinct and are not supplied by the author campaign.

For the suffix workload decision, `corpus-index-benchmark` emits81 raw timing
rows: sizes8/32/64, equal/periodic/mixed texts and9 alternating-order rounds.
Both paths answer the same four-byte occurrence COUNT query, including absence;
the indexed path derives count from its range, the flat path scans text positions.
They must agree on the checksum. Each row separately reports1000 builds and
20000 queries; divide by those counts before comparing or computing a break-even
query count. Include index storage and build cost. Retain raw samples and
compiler/CPU/load facts, report distribution/dispersion, and explain scheduling,
clock granularity, warmed-cache and optimizer confounds. At most64bytes, measured
crossovers are specific to this workload and do not establish a universal
speedup. Query output position order and updates are outside this particular
equal-count comparison, while the full correctness suite retains range semantics.

# T67.07: Owned arena certificates and cyclic dataflow

This required successor completes the original X-T67-007 investigation and
delayed variant. Implement `arena_flow.c` using the actual retained `ad_eval`
producer, after the earlier domain work on checked indices and storage. No
external graph, lattice or solver library is permitted. Public artifacts are
the header, compiling stub, tests and independent-model driver interfaces.
Reference implementation and technical answers remain assessor-only.

## Whole-arena certificate

The original contract admits1..32 nodes. A literal holds uint32; an addition
references two strictly earlier indices. Every node must be valid and every
addition must fit uint32, including unreachable nodes. A selected root only
selects the result; it never weakens whole-arena admission. Typed ADNodeIndex
does not establish range membership or arena identity.

`adf_arena` copies all supplied nodes into staged inline storage, calls actual
`ad_eval` on the entire copy, then computes and stores every node value by
topological order. Successful publication creates an `struct adf_arena` that owns its
inline nodes and cached values. Later mutation or destruction of the original
input does not change that copy. `adf_root` checks the index and returns the
cached value without repeating structural validation. Constructor or root
refusal leaves its output unchanged. Empty, over32, missing arguments, malformed
kind/edge and arithmetic overflow are INVALID in this constructor contract.

All pointers denote live, aligned objects of the declared type and real input
extent. Input and output storage are disjoint and externally serialized. A
certificate is initialized only by successful construction; clients never
modify its public representation. Const access through `adf_root` is not an ISO
C guarantee that another alias cannot write it. Invalidating that premise
invalidates the certificate. Public value copies are allowed and own independent
inline copies; no allocation, callback, hidden borrow or destructor exists.

Derive the cached pass by induction: every referenced value was computed earlier
from the same validated copy, and actual whole-arena validation established
representable additions. Consequently the internal pass needs neither another
index check nor another overflow check. Copy, validation and evaluation are
O(n); selected cached roots are O(1). Do not relabel a recursive expansion of
shared children as O(n). The supplied16-node doubling graph makes65535 recursive
tree visits but represents only16 arena nodes. Its value32768 is checked by both
methods, and compiler stack-usage records accompany the implementation.

## Finite monotone cyclic analysis

The changed graph has1..8 nodes and8 possible facts per node. Each node supplies
a predecessor bitmask, a generated-fact mask and a keep mask, all validated.
Self edges, cycles, disconnected nodes and empty predecessor sets are admitted.
The transfer equation for node i is:

`value[i] = generate[i] | ((union(value[j] for predecessor j)) & keep[i])`.

Use subset order, empty fact sets as bottom and union as join. The whole domain
is the product of these finite subset lattices. `adf_start` copies the complete
graph into an owned session, initializes values to bottom and marks every node
pending. Missing arguments, count outside1..8, nonexistent predecessor bits or
fact bits above7 are INVALID and preserve the old output session. The input
graph need only remain stable through this copy; session fields then remain
private by caller discipline, not language-enforced opacity.

`adf_step(session,budget,out)` repeatedly processes the smallest pending index.
Remove that bit, compute the transfer and, when its value changes, mark every
successor pending, including a self successor. Each pop increments the session
evaluation count. This progress commits in the session; it is not rolled back
when the caller's budget expires. BUDGET leaves `out` unchanged and retains the
session for another step. Budget0 performs no work. Once no pending node remains,
OK publishes all eight result slots (unused slots zero) and the cumulative
evaluation count. Repeated OK with budget0 is idempotent. Null session/output
is INVALID; every nonnull session must be successfully started and unmodified.

Monotonicity follows from union, intersection with a fixed keep set, and union
with a fixed generated set. Starting at bottom, no update removes a fact.
Every node changes at most8 times. Each change can enqueue each successor once,
while the pending bitset merges redundant enqueues. Thus at most `n+8E` pops
occur, bounded by `n+8n*n<=520`; scanning predecessors/successors gives
O(n*(n+8E)) time and O(n) storage for the variable n/B model. These are bounds,
not a claim that the fixed8-node representation measures every general solver.
The smallest-index rule is fair here because only finitely many increases can
keep re-enqueuing a lower index. On termination every equation holds, and each
intermediate value is below every fixed point, establishing the least one.

Concrete cycle: node0 takes node1; node1 generates fact0 and takes node2;
node2 takes node0; all keep255. All three final masks are1. Processing node0
before the generated fact arrives requires later re-enqueueing. Replacing union
with intersection loses facts at joins; removing re-enqueueing can falsely claim
completion. A keep mask may remove an incoming fact from a node's transfer but
does not make repeated evaluation decrease from its earlier state.

## No fixed point is not a timeout

Under the admitted finite monotone premises a fixed point always exists. The
monotone solver must never report NO_FIXED_POINT for an exhausted budget. To
assess the original requirement honestly, a distinct small equation API admits
arbitrary Boolean transfer tables, including nonmonotone ones. It is explicitly
not the same convergence theorem or a substitute production static analyzer.

`adf_boolean` accepts1..4 Boolean variables. Each node chooses two in-range
variable indices and a four-bit truth table, indexed by `left+2*right`. It
enumerates assignments0 through2^n-1 in numeric order. Each tested assignment
costs one budget unit. The first satisfying assignment publishes OK. Exhausting
the budget before testing every assignment returns BUDGET unchanged; testing
every assignment without a solution returns NO_FIXED_POINT unchanged. Missing
arguments, invalid indices or table bits above3 are INVALID unchanged. This
bounded exhaustive search supplies actual nonexistence evidence, not inference
from an oscillating iteration. No ownership or resource transfer occurs.

One variable with table5 and both inputs itself is `x=!x`: budget1 is BUDGET,
budget2 proves NO_FIXED_POINT. Two equations `x=!y,y=!x` have fixed assignments1
and2, although simultaneous updates from00 oscillate00→11→00. The teaching
iteration observes that oscillation, while exhaustive search finds assignment1.
As a separate mathematical changed assumption, `F(x)=x+1` on unbounded natural
numbers has no finite fixed point; do not simulate that proof using wrapping
machine integers or confuse it with the finite lattice above.

## Independent verification and rubric

Before execution, submit whole-arena induction, graph ownership/immutability,
the lattice order/height, worklist invariant, budget semantics and the exact
premise separating nonexistence from incomplete computation. The Python arena
oracle recursively evaluates every node using mathematical integers and detects
invalid kinds/edges/overflow independently of the cached C pass. The flow oracle
uses synchronous equations from bottom rather than the C worklist order. It
checks exact final masks, partial results below the least fixed point, bounded
evaluation counts, resumability and unchanged budget outputs. The Boolean oracle
enumerates assignments as independent tuples and computes every solution.

Run all four anchors in GCC/Clang debug/release/ASan+UBSan profiles, and retain
exact sources, compiler dependencies, commands and outputs. Eight compiled
controls independently validate only a prefix, return a node's raw literal field
instead of its derived value, omit successor rescheduling, intersect joins,
publish partial values as converged, label limited search as no solution, admit
retained forward references or omit retained overflow checks. They must fail
ordinary assertions; no crash or sanitizer termination earns the intended
rejection. Four compiling learner stubs must fail their anchors.

R1 assesses prior-index acyclicity and the lifetime of the copied certificate.
R2 assesses the unchecked internal pass proof, O(n) construction/O(1) roots,
and actual shared-tree duplication. R3 requires all-node independent evaluation,
unreachable malformed/overflow nodes, multiple roots, maximum/empty counts and
both retained semantic controls. R4 requires actual cyclic propagation, budget
resumption, least-fixed-point justification and the separate witnessed
nonexistence/oscillation distinction. Public tests alone are level1; level2
requires all these arguments and artifacts; level3 requires an unaided changed
case, an invalidated prior premise and a justified repair.

For X-T67-007-DELAYED reverse the direction of the generated-fact ring, restrict
different keep masks, mutate the original graph after starting the owned
session, and split the budget into one-pop slices. Compare exact final facts
against synchronous equations and prove the saved partial state remains valid.
Then replace a transfer with NOT: require an explicit model change and a bounded
nonexistence search, not an unsupported monotonicity claim. Unaided delayed
learner attempts remain separate from author executions.

Hints: (1) count strict fact additions rather than cyclic edges; (2) a changed
predecessor must wake all dependent equations; (3) a complete finite search and
a stopped iteration provide different evidence about whether a solution exists.

## Thirteen cost/enforcement dimensions

| Dimension | Evidence and boundary |
| --- | --- |
| Representation | Measured arena size/alignment, session size, node/index/result encodings. |
| Reachable storage | Inline copied nodes/cached values/session; no retained input pointer or owned heap. |
| Lifetime | Constructors publish owned values; input can change later; public certificate/session mutations violate explicit premises. |
| Mutation/aliasing | Disjoint outputs, externally serialized calls; budget progress mutates only the session. |
| Allocation | None in graph/arena mechanism; actual compiler storage/protection helpers are audited separately. |
| Movement/copying | Fixed-size value copies and staged constructor; graph copying supplies independence from original mutations. |
| Destruction | No destructor; no callback cleanup or recursion in the core. |
| Dispatch | Direct retained ad_eval validation, internal transfer loops and Boolean truth-table selection. |
| Concurrency | No concurrent-session or reclamation guarantee; independent sessions may use disjoint storage. |
| Failure | Invalid constructors/search preserve outputs; BUDGET preserves result but intentionally retains progress. |
| Compilation | Strict C diagnostics, actual clean/unchanged build observations and source-bound compiler inputs. |
| Binary/stack | Recorded binary/object/stack evidence; O(32) arena scratch and fixed8-node session, separate16-level recursive teaching harness. |
| Boundary | Machine uint32 checked arithmetic versus mathematical oracle; finite monotone theorem versus arbitrary Boolean search and unbounded arithmetic. |

Seven repeated measurements compare10000 retained full evaluations with one
certificate construction plus10000 cached queries on the same immutable16-node
input. The cached timing includes its construction; report the stronger
immutability premise and amortization explicitly. A second workload performs
1000 full8-node fact-ring analyses per sample, reports actual evaluation counts
and checksum2040000. No speedup is assumed. Source reports retain compiler
helpers rather than claiming a freestanding zero-runtime image. Finite evidence
does not establish soundness for arbitrary source languages, absolute mastery,
non-author review, independent learner performance or physical target behavior.

All malformed examples are original local teaching inputs with synthetic values.
They demonstrate validation/analysis failures; no external system or exploit
claim is involved. Published C17/C23 access gaps remain explicit. The inspected
Clang dataflow introduction supplies bounded lattice/worklist context; the
exact models, proofs, solver and exhaustive counterexamples here are original.

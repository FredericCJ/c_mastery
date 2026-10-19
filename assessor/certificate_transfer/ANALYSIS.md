# Assessor: preservation of predicates across representation changes

Keep this file, implementations, oracle and delayed packet out of learner exports.
The original X rubrics remain authoritative. This package fills two implementation
gaps; signed-graph, red-black, duplicate adaptation and streaming-match work retain
their separate provider evidence. Inspect the learner's provider hashes and
contracts before treating composition as demonstrated.

For X-T36-001 R1/R2, the path witnesses establish attainable labels; each reachable
edge inequality establishes the other optimality direction by telescoping.
Rooted paths prevent a disconnected zero-cost cycle from posing as reachability.
Negative edges invalidate Dijkstra's greedy premise, not this certificate
argument. The existing provider bounds signed labels before adding an `int`
weight in int64. A reachable negative cycle cannot satisfy all inequalities;
an unreachable cycle does not affect the source's reachable closure. The stream
extension must validate the whole enclosing message before applying this exact
predicate and publishing a CSR. Its wire domain is narrower than arbitrary C
`int`, and neither byte order nor signed representation is inferred from a cast.

The negative edge bytes `ff fd` represent65533 unsigned, then65533-65536=-3.
The implementation's split expression subtracts32768 twice so no intermediate
conversion of65533 to a16-bit signed `int` is necessary. The original profile
requires representable-32768. A tempting `*(int16_t *)p` breaks alignment, alias,
endianness and representation arguments. Unsigned decoding accepts the wrong
numeric graph; the planted mutant rejects the independently correct witness.
The early-publication mutant changes output on a nonfinal complete frame before
a possible trailing octet; a subsequent error status cannot undo a published
effect. Both controls compile and return marked assertion failures; no UB value
oracle is assigned.

For X-T36-002 R1/R3, perform postorder with an explicit frame stack. At phase0
validate node/parent/ancestor interval and schedule left; phase1 schedules right;
phase2 combines child results. A mark precedes traversal, so shared/cyclic links
are rejected without repeated descent. Push checks node range and liveness
before indexing. After a child returns, its parent's phase tells which pair of
results receives the value. Finally require every live slot reached and count
agreement; a correct root subtree alone is insufficient. Preserve output until
all checks pass. The64-node chain is an intentional non-AVL valid policy0
certificate. It witnesses the full explicit stack requirement and distinguishes
valid-unbalanced from malformed. `CT_SPACE` at a smaller workspace is a new
resource result, not a claim of equivalent rejection semantics at all capacities.

The BOUNDS mutant accepts key45 in the left subtree of40 because it removes the
ancestor interval check. The HEIGHT mutant accepts a wrong cached root height
under policy1. Both have explicit hand-derived witnesses before the broader
recursive differential corpus. The Python oracle instead checks incoming-edge
counts, transitive descendant closure and all ancestor ordering, then eliminates
leaves to derive heights/colors; it does not copy the explicit frame algorithm.
Its synthetic corpus and seed are bounded. It is not a new balanced-tree update
implementation, and differential agreement alone does not prove correctness.

For R2 use the already separate real-tree duplicate adapter and its source-bound
tests; this companion does not award duplicate-policy credit from tree checking.
Likewise the red-black policy must retain the actual provider's valid non-AVL and
invalid-color examples. Nil-black counting includes a sentinel and the current
black node, whereas CLRS13.1 defines black-height excluding the current node;
state the convention, preserve equal paths, and do not compare raw counts without
conversion. The inspected source also bounds valid red-black height, not arbitrary
malicious certificate traversal. Such input requires the full64-slot guard.

Resource assessment: report both caller workspace and call stack. The iterative
workspace includes64 frames plus64 marks; moving it out of automatic storage is
an ownership/placement choice. Compiler `.su` values describe emitted frames for
the exact profile, including inlining effects. For an unoptimized retained
recursive `visit`, a conservative depth bound includes the64 nodes and the NIL
call. Do not present a single function's frame as the entire recursive stack,
or a compiler estimate as measured board high water. Function-size comparison
uses the actual `ct_tree`/`push` versus `at_check`/`visit` symbols; instrumentation,
dead stripping and target ABI are recorded. Runtime speed is not measured here.

Alternative valid implementations include an explicit enter/leave event stack,
indexed postorder plus height arrays, or a two-phase topology/policy checker,
provided they preserve predicates, capacity/error/output contracts and bounds.
For the stream, direct partial edge decoding with a bounded unpublished CSR is
valid; the proof must preserve incomplete-edge storage and rollback rather than
silently publishing each edge. These alternatives change resource accounting.

Public runner reference invocation (author execution only):
`python3 labs/certificate_transfer/run.py --implementation assessor/certificate_transfer --provider assessor/algorithm_frontier/frontier.c --provider assessor/algorithm_tree/certificate.c --provider assessor/containers_depth/indexed.c --include labs/algorithm_frontier --include labs/algorithm_tree --include labs/algorithm_depth --include labs/containers_depth --include labs/algorithms/include`.
Run the same command without `--implementation` and require the compiling stub
to reject. For an isolated export, copy only public harness/header/contract and
required public dependency headers. Supply the author's three prerequisite files
explicitly from outside that export for the author-side adapter check; never
call them learner artifacts. A subsequent learner attempt substitutes actual
learner source identities and receives its own report.

Pass requires explained publication/interval invariants, exact status and
unchanged-output results, all ordinary/negative/independent checks, actual
resource evidence and source-bound integration. Plausible incorrect solutions
that only cap recursion, validate local keys, accept completed prefixes, or rely
on sanitizer silence fail the corresponding criterion. External review and
learner level3 remain pending until separately observed.

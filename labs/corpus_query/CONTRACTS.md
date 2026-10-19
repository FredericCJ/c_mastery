# Required query-index mechanisms

The three exercises share one original row store so composition uses real
component interfaces. No database, bitset, search or indexing library may
implement the taught mechanisms. Host test/oracle libraries are separate.
The component baseline is C17 with8-bit bytes and exact-width integers.

## Representation, ownership and lifetime

There are at most32 stable row slots. Each QiRow has alive and has_value flags
(each0 or1), a signed16-bit numeric value and up to8 term IDs0..7. Deleted slots
retain their row numbers; replacement never compacts another row into that ID.
Null numeric values do not remove a document or its terms. Repeated terms occupy
distinct token positions. This is an already-tokenized byte-domain exercise;
natural-language normalization, collation, ranking and SQL parsing are absent.

QiIndex owns copies of rows, eight uint32_t term-incidence masks, sorted postings
and eight group summaries. Each posting carries one row ID and an8-bit set of
positions. It appears exactly once per present term/row. Posting row IDs strictly
increase; position bit p is set exactly when token p equals that term. The live
mask is the universe for complements. All bit shifts use an unsigned one and
an index below32; no expression shifts by a word width. Unused high bits are0.

A zone describes four consecutive row slots, including deleted slots. Its
present flag means its interval encloses at least the potentially nonnull live
values; exact=1 additionally means current extrema, while exact=0 permits stale
wider bounds. No output borrows the input. No function retains external pointers.
Caller-provided objects and input/output spans must be live, initialized where
required and disjoint; caller discipline establishes these facts. Index objects
are trusted values made by these APIs, not serialized attacker bytes.

## Operations and failure publication

qi_build accepts0..32 validated rows and publishes a fully built index. A null
row pointer is permitted only for zero rows. qi_replace validates one existing
slot and stages a complete successor; invalid flags, terms, length, slot or
lazy_zone argument leave the original byte state unchanged. Deleting a slot
uses alive=0. Both operations rebuild term masks and sorted postings; this
reference deliberately charges full reindex work rather than claiming an
incremental-update complexity it does not have.

qi_terms(required,forbidden) returns live rows containing every required term
and none of the forbidden terms. Each argument is an8-bit term set. Empty
required/forbidden sets select all live rows, including empty documents. A term
in both sets gives an empty result. Query results are row-ID bitsets, not counts
or compacted offsets.

qi_compare(threshold) partitions known live values into yes (>= threshold) and
no (< threshold). Remaining live rows are unknown. QiTruth masks are disjoint
subsets of live. qi_logic operation0=AND,1=OR,2=NOT; both input truth objects are
validated, though NOT ignores b after validation. AND has yes=a.yes&b.yes and
no=a.no|b.no; OR has yes=a.yes|b.yes and no=a.no&b.no; NOT swaps masks. Derive
these formulas from all nine scalar truth-table pairs. Bitwise complement alone
cannot implement NOT in the presence of unknown values or deleted rows.

qi_phrase accepts0..8 terms and requires consecutive ordered occurrences in a
document. An empty phrase matches every live document. First intersect sorted
posting row lists, then intersect positional starts after shifting each term's
positions by its phrase offset. Repeated query terms must occupy the distinct
positions dictated by their offsets. Do not reduce a phrase to term membership.
qi_near(a,b,d) requires distinct occurrences in either order at distance1..d,
where d is0..7. For a=b a single occurrence never suffices; d=0 always returns0.

qi_range(low,high) is inclusive and rejects low>high. A group may be skipped
only if absent or its entire enclosure is disjoint from the query. Every row in
an admitted group is scanned; only live known in-range values are returned.
QiCost.groups counts admitted groups and QiCost.rows counts examined slots,
including dead/null slots inside those groups. Summaries generate candidates;
overlap alone never proves that a matching row exists.

An exact replacement recomputes every group. A lazy replacement retains old
enclosure and widens it to include any new live nonnull value; deletion/null
replacement does not shrink it. Mark the changed group conservative. This
policy may increase false positives but cannot lose a true match. Rebuild
restores exact current bounds. Widening only the new minimum, or retaining a
too-small old maximum, is an incorrect repair.

qi_search composes actual qi_terms, qi_phrase and qi_range calls and intersects
their results. It stages both result and cost until all three succeed. Every
non-OK status leaves every output/counter unchanged. Error priority is argument
validation, then calls in the order above. QI_SPACE is reserved for a future
resizable contract and is not emitted by this fixed-capacity API.

## Required construction and transfer

X-T14-012 implements incidence masks and row replacement, derives the live
universe invariant and compares dense masks with sorted sparse postings.
Its delayed variant implements nullable comparison and all three truth
operators without turning unknown into false. Supply a counterexample where
complementing the yes mask incorrectly returns a null or dead row.

X-T14-013 implements sorted document intersections and exact phrase positions.
Its delayed variant implements unordered proximity with repeated terms and
distinct-occurrence semantics. Show a document containing every term in the
wrong order, and a single-occurrence document that defeats a same-term shortcut.
Derive how a streaming posting cursor could replace the bounded arrays without
retaining pointers across refill; this part is an ownership analysis, not a
claim that an I/O backend is already implemented.

X-T14-014 implements exact zone construction and inclusive filtering. Its
delayed variant implements widening-only updates and explicit rebuild, then
composes all three real mechanisms in qi_search. Demonstrate false positives
without false negatives and a stale narrow summary that loses a new extreme.
Compare sorted and permuted row order under identical selected values, including
null-only groups, deleted extrema and non-multiple-of-four final groups.

## Invariants, cost and enforcement

For T terms, N rows and L tokens, this simple full rebuild costs O(T*N*L), owns
O(T*N+N*L) fixed storage and uses a staged full successor on the stack. Incidence
filtering costs O(T) word operations for N<=32; general N requires ceil(N/word)
words and an explicit tail mask. Do not call that general problem O(1).
Merge intersection costs O(sum of scanned posting lengths); this deliberately
simple positional lookup adds bounded posting searches and position operations.
Zone query costs O(number of groups + examined rows). Randomized row order can
destroy pruning while preserving exact results; sortedness is not a requirement
for correctness. Rebuild cost and retained false positives are real tradeoffs.

Measure sizeof structures, compiler stack frames, text bytes, actual imported
helpers, examined rows and process CPU ticks. An observed tick comparison does
not replace asymptotic reasoning or establish board latency. Profile C permits
only explicitly accounted compiler support. The target audit links the existing
original assessor/execution/support.c byte helpers and records imports before
and after closure; no libc/libgcc/compiler-rt archive may be linked there.
ISO C does not enforce row-version identity, stale snapshot lifetimes, alias
preconditions or the correctness of the enclosure invariant. Diagnostics,
runtime checks, independent models and programmer obligations must be separated.

## Verification, isolation and rubric

The private oracle uses raw rows, substring search, pairwise position distances,
scalar truth predicates and independently calculated group extrema. It does not
build the tested indexes. Public checks cover full word width, empty sets and
documents, invalid replacement rollback, all nine truth pairs, deleted rows,
equal interval boundaries, null groups and stale-envelope rebuild.

Six compiling mutants respectively omit universe masking, retain old term bits,
ignore phrase positions, permit self-pairs, omit lazy widening and incorrectly
exclude equal extrema. Reject each through its intended QI_CHECK assertion;
translation failure or a sanitizer crash is not a semantic-control pass. Run
six GCC/Clang profiles and the independent driver oracle, then isolated exports
with an explicit provider. The learner template remains unsolved.

Attacker control is synthetic original row/query input only. The local property
is result integrity and bounded access, including false-negative avoidance.
A harmless stale-summary demonstration omits a synthetic matching row; this
does not establish privilege escalation, data theft or code execution. Repair
through widening/rebuild and verify regression inputs and composed filters.
No third-party database/service is a target. Concurrent mutation, persistence,
authenticated storage and arbitrary corrupt in-memory pointers are outside this
explicit API, not silently provided by tests.

Each exact exercise rubric requires representation/proof, construction and
independent falsification plus the changed-condition work. Level1 is nominal
output only; level2 is the full bounded ordinary contract with evidence;
level3 requires independent delayed transfer and a defensible cost/assumption
analysis. Author reference runs, external review, learner evidence and physical
observations remain separately recorded.

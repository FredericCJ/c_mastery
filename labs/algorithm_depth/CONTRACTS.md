# T15 algorithm mechanisms and changed assumptions

This required additive laboratory retains the ten PR-T15 projects, all forty-five
leaf objectives, and their original `AX-PR-T15-…-DELAYED` identifiers and seven-day
delay. Parent briefs, leaf-specific rubrics, graduated hints and assessor links
remain binding. Historical hours describe the accepted route; they do not limit
this work. The historical elective label on PR-T15-10 does not make its current
required supplement optional.

Use the original project problem and its concrete input before implementing the
corresponding `ad_*` entry points in `learner.c`. Explain predictions, construct
the mechanism, diagnose the relevant mutant, submit a source-bound dossier, and
later attempt the original changed-condition task without copying the assessor.
The reference implementations and transfer analyses are withheld under
`assessor/algorithm_depth/`. Tests, source passages and a passing reference run
are teaching evidence; none is an independent learner assessment.

## Environment, prerequisites and common contract

Use ISO C17 and the repository Profile C. Implement the studied mechanisms from
primitives. No library sorting, search, parsing, serialization, compression or
allocation implementation may supply an assessed operation. Host Python integer
arithmetic, `Fraction`, hosted I/O, clocks, sanitizers and test `memcmp` are
verification tools. They are not target dependencies. The selected wire adapter
requires eight-bit bytes; the existing T14 heap adapter requires `int` to
represent `int32_t`. The actual compiler settings, target and data model belong
in each submitted record. Exact published C17/C23 access remains a reference
gap; an inspected C11 draft must not be relabeled C17.

Prerequisites are C object lifetime, array extents, signed/unsigned arithmetic,
loop invariants, ownership, asymptotic cost and the actual retained T11 byte and
T14 heap/graph artifacts. The graph consumer uses `struct cd_csr` and `cd_csr_build`.
Heap sorting uses `cd_heap_build`/`cd_heap_take`. Byte operations import
`fr_copy`, `fr_compare`, and portable same-arena `fr_arena_move`. These are
original learner prerequisite implementations, not substituted libraries.
Record provider source/header hashes. Do not change the frozen provider to make
a consumer pass. T15 depends on T11; T11 does not depend on T15.

All pointers refer to live objects of their declared type and sufficient extent.
Inputs, outputs, workspaces and control records are disjoint unless a specific
operation below allows aliasing. Zero-length spans permit null without pointer
arithmetic. Output controls must be live and distinct from payloads and other
output controls. No concurrent access, signal interruption or callback reentry
is admitted. Ownership remains with the caller; there is no target allocation.
Caller-owned workspace contents are scratch and may change after successful
admission. A failed batch operation leaves result objects and payload outputs
unchanged unless its explicit streaming contract permits partial progress.
Diagnostic objects are intentionally updated on failure. C does not enforce
these alias, lifetime, ownership or synchronization requirements.

Every submission includes: implementation and headers; input/output fixtures;
invariants and termination arguments; boundary/failure results; actual compiler
and link commands; resource ledger; mutation or falsification evidence; and the
original changed-condition dossier. A level2 result must meet each original
leaf's concrete contract and explain its limitations. Level3 additionally derives
the changed argument, diagnoses the discriminating counterexample, and transfers
the mechanism independently. Compilation or matching one fixture alone is
level0/1 evidence. The original per-leaf rubric remains the assessor authority.

## Search, sorting and selection: T15.01–05

`struct ad_record` contains a signed32 key and a non-owning integer label; admit0..16
records. `ad_linear` returns the first equal index or length. `ad_bounds` requires
the stated ascending/descending ordering and returns the lower bound, upper
bound, and first equal index or length. Prove `0<=lo<=hi<=n`, exclusion outside
the interval and decreasing width. Analyze a synthetic logical index space near
`SIZE_MAX` separately; no such giant C object is claimed to exist.

Partitions stage the pivot before mutation, including when it aliases an input
key. Stable partition requires a disjoint record workspace of at least n and
rejects insufficient workspace before movement. Three-way partition returns
`[0,lower) < pivot`, `[lower,upper) == pivot`, and `[upper,n) > pivot` and preserves
the labeled multiset. Explain why the scan does not advance after exchanging
an unexamined high element. The label is not a unique resource or self-pointer;
resource-owning payloads require a new move/rollback contract, not byte copying.

Insertion and merge preserve original equal-key order. Merge requires two sorted
runs and disjoint n-record workspace. Merge sort needs n scratch records and
logarithmic recursive depth. The reference heap adapter builds the actual T14
max heap, then fills a shrinking sorted suffix through extraction. It additionally
holds16 keys,16 labels and a `struct cd_heap`; it does **not** claim constant auxiliary
space or stable ties. Derive the in-place active-array alternative as a distinct
storage contract before comparing it with this producer adapter.

Both quicksort variants use three-way partitioning and an explicit nonnegative
depth budget. Exhaustion invokes the prior heap sort. The recursive version
recurses on the smaller side; the explicit version defers the larger side and
continues the smaller. Each nested deferral halves the active bound, so five
frames suffice for the admitted maximum16. Budget0 must visibly exercise the
fallback. Selection rejects empty/invalid ranks before mutation, supports checked
one-based conversion, and returns the complete equal-rank interval. This concrete
first-pivot selection is deterministic: random-looking tests do not establish an
expected-linear bound. The original median-of-medians transfer is a derivation,
including groups, duplicate handling, rounding and the recurrence base cases.

Counting sort admits keys−2..7 and uses ten counters. Radix sort uses four- or
eight-bit digits, stable scatter and a value-based unsigned order transform.
Host endianness is irrelevant to this transform. Record all count/output arrays;
the reference reserves256 counters even for four-bit digits. A hypothetical
baseline without an exact signed32 type needs a different representation and
value mapping, not an unchecked reinterpretation of sign-magnitude bytes.

`struct ad_work` must start at zero for a measured operation. Its comparisons count key
comparisons; writes count the record assignments explicitly instrumented in the
sort layer. They do not count all heap/control/scratch traffic. Enumerate that
additional traffic and stack separately. The expensive-comparison variant changes
the cost model, not the ordering relation. The benchmark times input reset and
algorithm calls, and compares only sorted labeled multisets; stability promises
are assessed separately. Raw timings never establish a universal winner.

## Bounded and streaming matching: T15.06

Batch input length is0..64 and pattern length1..16; the local empty-pattern policy
is rejection. Inputs are byte spans, including zero. Naive, prefix-function and
rolling-hash paths report all overlapping starting offsets. Result capacity is
checked before publication. Prefix initialization copies the pattern and builds
its own failure table; keep that table immutable until reinitialization.

`ad_match_feed` admits chunks0..64. It stages the complete new state and matches,
then publishes only if the offset addition and output capacity are valid. Failed
chunks consume nothing. Absolute offsets use `uint64_t`; overflow rejects before
reading input. The prefix length decreases through failure links without losing
possible matches. Comparison/fallback counters saturate at their maximum; this
prevents wrapping counters from masquerading as low work. Derive a linear bound
from total prefix increases and decreases, then check every two-chunk split.

The pedagogic rolling hash has radix2 modulo17. The strings `[1,0]` and `[0,2]`
collide. Exact byte verification is required, and false candidates are counted.
Omitting verification changes the correctness/error contract. No adversarial
false-positive probability follows from this fixed small hash.

## Graph frontiers, paths and optimization: T15.07–09

Consume the immutable T14 `struct cd_csr`: at most8 vertices and32 edges. Validate offsets,
source ownership and destination bounds before edge access. BFS and iterative DFS
use caller workspaces conservatively sized for every vertex, so a short workspace
rejects before changing result/frontier data. Start-first forest mode next visits
the remaining vertex IDs in increasing order. Reachability from one start is
distinct from a forest over all components. Discovery marks prevent duplicate
enqueueing; parallel edges still count as separately inspected edges.

DFS frames retain the vertex and next edge. They produce discovery and finish
orders. A stack of vertex IDs alone may compute reachability but cannot claim
the same finish-order evidence without retaining the suspended edge position.
Kahn topological ordering chooses the smallest available zero-indegree vertex;
a residual cycle returns `AD_FAILED` with output unchanged. Explain its
indegree invariant and compare with the source's reverse-DFS-finish method.

Dijkstra rejects every negative edge, including unreachable ones. Its scan-based
minimum selection costs O(V²+E), rather than claiming heap complexity. Distances
use `uint64_t` and an explicit limit strictly below `UINT64_MAX`, which denotes
unreachable. Any examined reachable distance addition beyond the limit rejects
the whole result, even if another route would be shorter. Equal-cost ties keep
the first strict improvement under deterministic vertex/edge order. Settled
predecessors cannot form a zero-weight predecessor cycle. Verify each reported
path independently. The negative-edge and reachable-negative-cycle transfer
uses a separate exact edge-list Bellman-Ford model, not the nonnegative solver.

Interval scheduling admits up to8 positive-length half-open intervals. It stably
orders by finish time and returns labels of a maximum-cardinality compatible
subset. The exchange proof preserves cardinality; weighted optimality is a
different problem. Knapsack admits up to8 items, positive weights, nonnegative
uint32 values and capacity0..32. Descending capacity gives0/1 semantics;
the separately selected ascending mode gives unbounded reuse. Exact-fill mode
tracks unreachable states explicitly and returns `AD_MISSING`, not zero value.
Any admitted addition overflow rejects with the old result intact. Derive the
state recurrence before compressing it, then compare with exhaustive subsets.

## Byte spans and integer conversion: T15.10–11

Byte spans are bounded to64. Copy requires nonoverlap. Same-arena move validates
offset/count extents and preserves a source snapshot. Compare promises only sign,
using unsigned-byte order then length. Search returns the first byte index or
length. Split returns offset/length metadata; keep-empty mode yields one empty
field for empty input, while drop-empty loses delimiter information. Metadata
continues to borrow the caller's unchanged backing storage. ASCII trim recognizes
space, tab, newline, carriage return, vertical tab and form feed. In-place trim
moves the retained span to index0 and returns length without appending a zero.

Integer parsing admits bases2..16, an optional sign, and optional `0x`/`0X` only
when explicitly enabled with base16. No whitespace is skipped. At least one digit
is required. Whole-input mode rejects a suffix; partial mode returns the exact
consumed prefix. Unsigned parsing rejects `-0` as well as other negative forms.
Accumulate a magnitude against the appropriate bound before multiplication;
handle signed minimum without negating it. On failure both value and consumption
are unchanged. Formatting emits lowercase digits and optional minus, no prefix
or terminator, and rejects insufficient capacity before writes.

The advanced decimal/binary task remains an explicitly separate dossier. The
supplied assessor uses exact fractions in an original p3/emin−2/emax2 format to
check subnormal spacing, ties-even, overflow and the difference between exact
decimal expansion and shortest round trip. It is not a production binary64
converter. Derive a bounded workspace/input policy and explain which obligations
a production algorithm would still need; do not label the toy as such an algorithm.

## Wire records, checksums and progress: T15.12–13

The exact eight-octet record is version1, flags limited to bits0..1, unsigned16
count big endian, and signed32 value encoded as two's-complement big endian.
Both encoder and decoder require exact length8; neither relies on struct layout
or unaligned typed access. Rejection preserves output. The extension decoder is
a separate format: the same eight-byte prefix followed by `(type,length,payload)`
triples. Type1 is one annotation byte, permitted once; unknown types with high
bit clear are skipped within bounds, and unknown critical types are rejected.
It accepts at most64 bytes. Do not make the exact decoder silently accept it.

CRC-8 baseline is polynomial0x07, initial0, final-XOR0, nonreflected, with check
`123456789 -> F4`. Implement bit and table paths and compare with polynomial
division. The explicit reflected variant uses the reciprocal bit-processing
constant0xE0 and independent initial/final parameters; the baseline check value
is not transferable merely because the width is still eight.

`struct ad_framer` owns eight bytes. Mode0 reads one length byte0..8 followed by that many
payload bytes. Mode1 uses delimiter0x7E, escape0x7D and XOR0x20 escaping; an empty
delimited frame is valid. Feed accepts at most64 bytes and returns consumed input.
It stops at the first ready frame. While ready it consumes zero new bytes until
acknowledgement; peek borrows internal storage only until ack, cancellation,
reinitialization or destruction. A length9 or oversized delimited payload is a
terminal local error after consuming the exposing byte. EOF with partial payload
or a pending escape is incomplete and terminal. Recovery requires reinitializing
an isolated stream; there is no claimed automatic synchronization with arbitrary
remaining input. Cancellation invalidates borrows and clears retained bytes.
The nested-format transfer separately proves both byte and depth budgets.

## Lexing, syntax and evaluation: T15.14

The lexer consumes at most64 bytes, recognizes ASCII decimal nonnegative signed32
literals, single lowercase names, `+ * ^ ( )`, and selected ASCII whitespace.
It returns explicit byte offsets/lengths and an end token at the input length.
Unknown bytes and literal overflow have precise diagnostics. Tokens retain source
locations; `ad_token_copy` offers owned spelling before the input is recycled.
Names in this bounded extension are one byte, not borrowed arbitrary identifiers.

The ordinary grammar is `expression := term ('+' term)*`,
`term := atom ('*' atom)*`, `atom := integer | '(' expression ')'`.
The extension enables names and right-associative exponentiation above
multiplication. Precedence climbing constructs an AST with at most32 nodes;
children precede parents. The depth budget1..16 counts active expression calls,
including operator-right-side and parenthesis calls, not just parentheses.
Capacity and an injected node-admission ordinal can fail; failed parsing publishes
no AST. Diagnose the first syntax error, without claiming multi-error recovery.

Evaluation is a distinct pass with checked signed32 sum/product. Exponents are
0..31 and `0^0` is locally defined as1. A missing variable is a semantic lookup
failure, not a syntax error. Lookup is synchronous, pure, non-reentrant and may
not retain output pointers; failures leave the final value unchanged. The API
does not roll back arbitrary external callback effects. Verify that malformed
node edges cannot read future/uninitialized values. Explain how the source's
Java allocation and conversion conveniences differ from this C contract.

## Compression, limits and evidence: T15.15

Baseline RLE emits `(count,value)` octet pairs, count1..255, splitting longer
runs. Plain input is at most1024 bytes; encoded input at most2048. Batch decoding
preflights odd length, zero counts, total decoded bound and destination capacity.
Canonical mode also rejects adjacent equal-value runs unless the preceding run
has count255. Permissive mode accepts the equivalent noncanonical form. The tagged
variant uses a positive seven-bit count: high bit selects a repeated following
byte, otherwise that many literal bytes follow. Zero-count tags and truncated
payloads reject; it has the same decoded bound.

The streaming RLE decoder explicitly permits partial progress. It retains a
pending count/value and unfinished run, returns both consumed and written counts,
and returns `AD_SPACE` while output remains pending. Capacity0 may consume a
complete pair and retain its run. An error after a committed prefix does not undo
that prefix. EOF rejects a missing value or undrained output. Compare every chunk
split and small output capacities with the batch result; a zero-progress retry
must change output availability rather than loop indefinitely.

Measure encoded size, scans, stack/caller storage and raw timing for repeated,
alternating and irregular inputs. The raw baseline copies into an intermediate
buffer and back: all three measured methods have the same lossless round-trip
output contract, but different intermediate-size properties. Setup and correctness
checks are outside that timing;64 repeated conversions and a volatile consumption
are inside. Report every sample and clock/toolchain assumptions. Compression
does not promise reduced size or faster execution for every distribution.

## Verification and security boundary

Compile each `_test.c` with its matching implementation and the explicitly named
prerequisite provider. The maintained runner performs GCC/Clang debug, optimized
and ASan/UBSan builds, compiled unsolved-scaffold rejection, intended semantic
mutations, independent integer/model checks, raw measurements and object/final
link audits. The custom image is Linux x86-64 with original `_start` and raw exit;
it is not an ISO startup, board, or firmware observation. Separate compiler-facing
memory helpers may exist even where source loops call none; audit their actual
provider and every final undefined symbol after optimization.

All adversarial inputs target only these original local teaching programs and
synthetic buffers. No external service, deployed device, credentials or other
person's data is involved. Success is a bounded wrong result, rejected invariant,
ownership/progress violation, or diagnostic; a crash/sanitizer report is not proof
of code execution. Repair the mechanism and retain the discriminating regression.
Finite case coverage, sanitizer silence and source identity do not prove all
inputs, compiler behavior, learner mastery or physical performance. See the
source-bound validation report for what was actually executed and the original
per-leaf rubric for what remains to be independently assessed.

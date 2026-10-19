# T15 reference analysis and withheld transfer checks

Use each original `AX-PR-T15-…` rubric and its exact delayed identifier. The
objective-specific evidence selectors and reasoning are in
`research/v5/expansion/legacy_algorithms/bindings.py`; they are not a replacement
for the original task. A copied reference, a passing public test or this author's
campaign cannot establish level3 independent performance.

## Search and sorting arguments

The lower-bound search excludes keys less than the query below lo and keys not
less than it at or beyond hi. Upper bound reverses which equality branch advances
lo. Both shrink `[lo,hi)`; empty ranges produce length without forming an element
pointer. A midpoint computed from interval width cannot exceed hi. The finite
logical-index witness is evidence about arithmetic, not a physically allocated
array of that size.

For stable insertion, every shifted record has a strictly larger key; using
greater-or-equal moves equal records across their original order. Merge must take
the left equal record because it came earlier in the concatenation. Stable
partition's two scans preserve relative order within each class. The actual
heap producer's identity is the input index from bottom-up build; the separate
label array restores the record label after extraction. Its private heap is an
explicit O(n) auxiliary adapter, not an in-place heap-sort proof.

Three-way partition maintains four ranges: less, equal, unexamined and greater.
A high swap shrinks the unexamined region without advancing the scan because the
incoming element remains unclassified. Staging the pivot prevents an in-array
alias from changing the comparison relation. Each recursive/continued small
partition is at most half its predecessor, so explicit deferred frames and the
recursive call chain have logarithmic depth. Depth-budget exhaustion changes the
work path to the real heap implementation. Counters do not substitute for this
termination/resource argument.

Radix order comes from unsigned *value* conversion, followed by a fixed bias;
extract digits with shifts, never object-byte casts. Under the inspected C11
exact-width definition, `int32_t` is already two's complement without padding.
The hypothetical sign-magnitude transfer instead maps admitted mathematical
values−127..127 to0..254 after normalizing zero. The reference model exercises
that distinction without claiming an exotic C compiler run.

The first-pivot selector has no randomness. An expected-linear statement requires
an explicit random-pivot probability model, not an input appearance claim. A
groups-of-five transfer can use three-way equality: at least three elements in
each qualifying group lie on the non-strict side of the chosen median. Excluding
incomplete/end groups introduces an O(1) term; the remaining recursive problem
has at most7n/10+O(1) elements, while pivot selection has at most ceil(n/5).
The9/10 coefficient leaves linear slack after choosing a sufficiently large base
case and constant. A two-way rule that sends every equal item to one side needs
a new argument and may invalidate the claimed shrinkage. The source's displayed
distinct-key SELECT is not copied as a duplicate-proof implementation.

## Matching and graph arguments

KMP's state is the longest useful proper prefix equal to a suffix of consumed
input. Failure links enumerate shorter candidates without rescanning input.
Prefix increases occur at most once per input byte; every fallback decreases it.
After a full match, falling back through the final prefix value retains overlapping
matches. Reset-to-zero is a separately rejected mutant. The complete chunk state
and match list are staged so output admission cannot partially advance absolute
offsets. Counter saturation is instrumentation behavior, not shortened work.

The fixed small rolling hash is deliberately easy to collide. The exact verifier
turns a hash candidate into an actual match. A probabilistic acceptance claim
would need a stated key/randomness model and a fresh false-positive bound; neither
the prime modulus nor a benchmark supplies one against chosen inputs.

BFS marks on enqueue, so each vertex occupies the work queue once; adjacency
duplicates still contribute work. DFS frames preserve the unexamined successor
position and produce a valid finish order. The simpler stack transfer reports
only reachability and uses discovery marking to stay within V slots. It must not
label its pop order as DFS finish order. Topological removal maintains current
indegree among unremoved vertices; if no zero-indegree vertex remains, a finite
residual directed graph has a cycle. This differs from the source's DFS proof
but satisfies the same ordering specification.

Dijkstra's settled-distance argument uses nonnegative weights. The original
negative-edge counterexample finds a later lower cost to an already finalized
vertex, so the nonnegative solver rejects the input. Its strict-improvement rule
and settled exclusion keep zero-weight predecessor ties acyclic. The explicit
distance limit is an admission contract: even a noncompetitive overflowing edge
causes whole-result rejection. The independent edge-list Bellman-Ford model
relaxes only from reachable vertices and distinguishes reachable from unreachable
negative cycles. No infinity sentinel is added as an ordinary integer.

Earliest-finish interval scheduling exchanges the first chosen activity for one
that ends no later, preserving later feasibility and cardinality. The weighted
counterexample disproves an unjustified extension. The0/1 knapsack recurrence
reads the previous-item row. Descending compressed capacities preserve it;
ascending capacities read current-row values and permit repeated use. Exact-fill
needs a separate reachability bit, since zero value and impossibility differ.

## Byte, conversion, framing and parser arguments

Same-arena offsets avoid relational comparison of unrelated pointers. Raw T11
`fr_move` has a separately declared flat-address contract and is not used to
justify portable arena ordering. Split/trim metadata borrows backing storage;
stored offsets do not confer ownership. Copy controls and lengths must not
alias output bytes. The source/object audit compares hosted loop recognition
with restricted compilation and links the actual original standard-symbol
provider, rather than silencing an unresolved `memcpy` diagnostic.

Magnitude parsing tests `value <= (limit-digit)/base` before multiplying. The
negative endpoint is formed without first negating an unrepresentable positive
signed value. Formatting signed minimum uses `-(value+1)` then unsigned+1.
The Python oracle accumulates unbounded integers and formats with independent
division, so it can distinguish mutually agreeing but wrong C parse/format
implementations. Whole-input and partial-input failure precedence remain explicit.

The decimal/binary dossier's p3 format deliberately separates exact rationals,
finite representable values, ties-even and overflow. Enumerating nearest values
is independent of quotient/remainder rounding. The midpoint between zero and
the smallest subnormal rounds to even zero;1/10 rounds to1/8. Decimal0.1 is a
shorter round-trip spelling of1/8 in this format than its exact0.125 expansion.
This finite derivation neither implements production binary64 conversion nor
establishes shortest-string uniqueness for every format.

Wire encoding shifts numeric fields into octets, while decoding handles values
above signed maximum through an in-range complement magnitude. CRC polynomial
division and bit-register updates use different representations of the same
GF(2) remainder. For nonzero initialization, the initial register contributes
`initial*x^(8*n)` before reduction. Accidentally shifting it eight more bits is
a real oracle defect: the corrected independent reflected/init255/xor255 check
is0x2F, while baseline nonreflected/init0/xor0 is0xF4.

Framing has two independent budgets: storage8 and bounded input progress.
Zero-length frames consume their header; ready frames consume zero until ack.
The escaped variant retains escape state across chunk boundaries. A borrowed
ready buffer is invalid after ack/cancel even if an old pointer still addresses
allocated bytes. Nested frames additionally charge each header/payload byte once
and cap active container depth. The C parser and separate Python model exercise
depth, total budget and local child extent as different failures.

Lexing, AST admission and evaluation fail at different stages. Operator
precedence determines structure, not callback evaluation order. AST children
always have smaller indices than their parent; evaluation rejects malformed
forward/self edges before reading uninitialized values. Pure single-byte variable
lookup is a declared external semantic effect. The exponent extension uses equal
right binding strength to obtain `2^(3^2)`, while ordinary left-associative
operators raise the right threshold. Node-admission injection proves unpublished
AST cleanup without pretending to have called an external allocator.

## Compression and assessment limits

RLE batch decoding computes total output before any payload write. Canonical
adjacent equal runs are allowed only when the earlier count is255; otherwise
merging them produces the same output with a shorter representation. Tagged
literals change the framing and worst-case expansion. The streaming decoder has
a different failure contract: emitted prefixes are committed, unfinished runs
remain owned by the state, and consumed/written counts describe partial progress.
Retrying with zero capacity and no changed condition is not forward progress.

The raw timing baseline performs two copies, giving the same lossless round-trip
observable result as encode/decode. It does not give the same encoded size.
Sort comparison costs and tracked record writes are not complete memory traffic;
resource analysis must include heap metadata, scratch and staged structures.
Finite exhaustive spaces, compiler profiles, semantic mutants and sanitizers
support their named claims only. Independent delayed assessment, external review,
physical timing and untested target behavior remain unclaimed.

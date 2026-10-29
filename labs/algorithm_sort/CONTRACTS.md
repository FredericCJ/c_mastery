# Stable assembly insertion and changed contracts

Version1, required additive implementation for the three original
`AX-PR-T36-03-T36.05.a/b/c` criteria and their exact `-V1` seven-day variants.
Use the original project and legacy shared assessment contract for publication,
four-level rubrics and evidence separation. Historical hours are descriptive,
not a ceiling. Families include derivation, boundaries, diagnosis, falsification,
failure recovery, cost analysis and changed-condition composition.

## Prerequisites, sources and environment

Bring the completed T15 insertion, stable merge and unstable heap-sort artifacts,
and the paired ABI proof/probes from PR-T36-02. This successor identifies the
actual T15 `struct ad_record`/`ad_insertion`/`ad_merge_sort`/`ad_heap_sort` interfaces;
its heap sort calls the actual T14 `struct cd_heap` producer. Both prior sources remain
unchanged. Their admitted n<=16 and signed32 keys are narrower than the new
assembly n<=32/unsigned32 contract. Do not cast incompatible record pointers.

Read the bounded CLRS fourth-edition insertion-record/invariant passages and
the actual Open Data Structures11.1 merge comparator from the successor
resource register. Explain which tie policy the inspected source implements.
Reuse the explicitly inspected SysV AMD64 and AAPCS32 clauses from the ABI
successor. These ABI documents are not ISO C language specifications.

The active C baseline remains C17. GNU assembly and selected scalar ABIs are
extensions/environment contracts. Profile C target components use no malloc,
libc or OS. Audit compiler-generated copies and bind the supplied original
byte-loop helpers where actually required. The hosted comparison/oracle harness
is profile A; the separate MPS2 Cortex-M4 reset/semihost scaffold is emulator
support, not a physical-board or time model. No library implements the new
insertion mechanism; prior full sorts are explicitly reused comparison subjects.

## Problem and implementation contract

Implement only the bounded assembly insertion kernel for typed
`algo_pair {uint32_t key,tag}`. Empty permits NULL; n<=32, and live capacity>=n
must hold. The checked facade rejects bad count/capacity/nonempty NULL before
any writes. Sort unsigned keys in ascending order, preserving original relative
order of equal keys. A tag is inert payload, not a pointer, ownership token or
assumption that input records are unique. All record moves preserve key/tag
pairs. The object has eight-byte stride on the selected ABIs; assert it.

Provide a separate instrumentation build that records each completed insertion
prefix and three distinct counters: key comparisons, right-shift record writes
and final pending-record placements. A stopping predecessor comparison counts;
the j==0 check without a predecessor does not. Count the final placement even
if no shift occurred. Record the full array after a completed placement, not
a falsely described intermediate permutation after one half of a movement.
Observer copies, global counters, call preservation and sampling overhead are
additional instrumentation work; do not include their runtime in kernel timing.

The supplied `struct as_stats`/`struct as_prefixes` observer is serialized, reset before each
call and not reentrant or thread-safe. It has room for all31 possible prefixes
of the admitted32 records. The observer may inspect borrowed input only during
the call; it owns its copied snapshots. It cannot mutate or retain input.

For prior comparisons, implement an explicit numeric order map from unsigned32
keys to signed32 keys, call the actual T15 providers on n<=16, then reverse the
map. Preserve every tag without a narrowing conversion. Insertion/merge must
match stable record order; heap sort must match sortedness and full-record
multiplicities, with no tie-stability requirement. `as_prior` selects method0
insertion,1merge,2heap. `as_prior_merge` separately exercises two already sorted
runs split at `middle`. Invalid controls, missing storage or unsorted runs
return1 before publication; a provider failure returns2; success0 publishes
records and the prior layer's comparison/write counters. Outputs and inputs
are disjoint live objects, cost output is mandatory, n<=16 and middle<=n.

Implement descending stable insertion as a separate interface; reverse strict
key ordering while retaining the stop-on-equality policy. Implement `struct as_wide`
with unsigned32 key and unsigned64 tag, key at offset0, tag at offset8 and
stride16 on these ABIs. Derive the new loads, register pressure and preserved
state; do not use an eight-byte whole-record move for this object. Padding is
not a key or payload and is not compared as value evidence.

Implement `as_fallible` as a new transactional comparator interface. It admits
n<=32 and stages all records in a local32-record array. A callback returns0
with relation exactly−1,0,+1; any nonzero status or malformed relation aborts
with status2 and leaves the caller's array unchanged. Publish attempted work
in `struct as_stats` on this callback failure, including the failing comparison. On
success0 commit the complete stable result. Admission failure1 leaves both
array and statistics unchanged and makes no callback. Empty still requires
a comparator and statistics object but makes zero calls.

Comparator/context ownership remains with the caller. A callback may change
its context, but cannot change/retain the borrowed records, reenter, terminate
nonlocally or use an inconsistent order while claiming a sorted result. Context
side effects cannot be rolled back by staging the array. Statistics and array
must be disjoint, as must all live control objects. This contract is intentionally
different from the infallible assembly interface, and its stack/call costs must
be reported separately.

## Necessity and reasoning

Distinguish sortedness, stable ties and preservation of every full-record
multiplicity. During insertion, the saved pending value is part of the logical
multiset even when the physical array temporarily duplicates a shifted value.
Mark the current hole; array excluding that hole plus the pending record must
equal the original multiset. At placement, the complete array recovers the
ordinary multiset and the enlarged prefix is sorted/stable.

For the original fixture `(3,A),(1,B),(3,C),(2,D),(1,E)`, derive each prefix,
stopping comparison and whole-record move. Explain the stable merge tie rule
and provide a two-run counterexample to a right-on-tie merge. Analyze finite
index arithmetic, nil/singleton/32-record states, high-bit keys, allocation-free
stack use, observer state, failure publication and callback ABI preservation.
Explain what is checked dynamically, what the ABI and diagnostics establish,
and what still relies on a well-behaved caller.

## Deliverables and verification

D1: Original invariant/pending-hole proof and annotated assembly, source IDs,
prior producer identities and explicit order-map derivation. D2: Ordinary,
descending and wide-tag kernels plus separate observed source and staged
fallible-comparator interface. D3: Complete fixture prefixes and independent
multiset/rank/inversion checks; sorted, reverse, equal and mixed/high-bit
workloads for every n0..32. D4: Actual native and emulator executions, preserved
register probes, object/final symbol and link maps, intended mutation controls
and unchanged-output failure evidence. D5: Separate kernel/observer/callback/
prior-adapter cost ledgers, source limitations and the unaided delayed variants.

The fixture requires B,E,D,A,C, nine key comparisons, six shifts and four final
placements. Sorted distinct five keys require four comparisons; reverse five
require ten comparisons and ten shifts. Recompute counts from inversions plus
the presence of a stopping predecessor, not a copy of the tested loop.
The Python host oracle checks actual emitted prefix and metric logs. Compare
all common-domain prior sort cells, including the direct merge checkpoint.
Test all33 wide lengths with high tag bits, and both callback error forms at
each of the nine comparison positions. Check capacity rejection before writes.

An equal-key shift mutant, a truncated-wide-tag mutant, a failure-publication
mutant and a missing-comparison-counter mutant must fail different intended
assertions. Test raw and instrumented outputs against independent value/rank
relations. Compile and execute each architecture; a cross-compile is insufficient.
Inspect no-runtime final target closure and explain explicit byte helpers.

## Rubric, hints and delayed assessment

Assess each existing leaf independently: .05a needs pending/multiset and
merge-tie reasoning; .05b needs actual unchanged prior sorts, ABI-correct
replacement and revised layout; .05c needs the three distinct work categories
and failure/callback accounting. Level0 fails its relation or boundary; level1
replays outputs without the relevant invariant; level2 derives and verifies
its complete D1–D5 evidence; level3 independently transfers to the exact changed
contract and produces a new counterexample. No single passing executable
awards all objectives or absolute mastery.

Hint1: Draw the hole and pending record before a shift. Hint2: Count inversions
and distinguish a stop at equality from exhaustion at index0. Hint3, after an
unaided attempt: use equal keys with different high tag bits, then fail the
comparator after one earlier insertion has completed in staged storage.
The separate assessor analysis contains answers and is not exported.

The existing `-V1` variants remain .05a descending stable order; .05b changed
tag width and stride/ABI; .05c fallible comparator and separate callback work.
Administer at least seven days later using the existing assessment contract.
Reference execution is author evidence, not independent delayed learner work.

## Security, status and limits

Only original local synthetic records and deliberately flawed curriculum
variants are executed. Failure controls detect wrong records/publication/counts;
no invalid pointers, external targets, credentials, host security settings or
privilege claims are involved. Keep array ownership and callback trust explicit.

Current source-bound execution and export status live in the successor receipts.
Assembly itself is not automatically instrumented by C sanitizers. Finite
observations and ABI probes are falsification evidence, not a proof of all
compiler states or inputs. Target observations are functional emulation, not
board timing. External review, physical observations and independent learner
assessment remain separate; final published C17/C23 text remains a source gap.

# PR-T37-05: Cache residency, reference models and dimensional costs

This required successor implements all three original T37.07 criteria and their
seven-day variants. Original project, fixture, assessment objects and accepted
receipts retain their identities. Use the original shared assessment contract
with this concrete mechanism specification. Inspected source passages are in
`research/v5/expansion/legacy_architecture_cache/resources.json`; separate
assessor reasoning is in `assessor/architecture_cache/ANALYSIS.md`.

## Prerequisites and environment

Bring the completed `algo_cache` artifact and identify its exact source. That
producer admits at most32 unsigned addresses and models four direct-mapped
16-byte lines with a four-block fully associative LRU shadow. Its aggregate
results are reused unchanged on the compatible original case; it does not
already implement this extension's snapshots, associativity, FIFO shadow,
rational arithmetic or overlapping requests.

Use ISO C17, fixed-capacity scalar arrays and type headers, with no cache,
simulation, collection or rational-arithmetic library implementing the studied
mechanism. Profile C core has no allocation, callbacks, threads, I/O or retained
input pointer. Hosted GCC/Clang, Python certificate/Fraction checks and symbol
tools are verification dependencies. The dedicated MPS2 Cortex-M4 image runs
the model and audits final helpers; it does not measure Cortex-M4 caches or
physical memory latency. Final C17/C23 text and new external textbook passages
remain source gaps, not newly inspected evidence.

## Inputs, operations and representation

Implement `ach_trace`, `ach_cost`, `ach_overlap` in `cache.h`. All input and
output objects are live and disjoint for the call. Output is committed only on
`ACH_OK`; every rejection preserves all caller output bytes. Declared extents
and live backing storage are caller obligations. Invalid geometry, unsupported
policy, invalid rational denominator, address or extent returns `ACH_INVALID`.
Representational or arithmetic range failure returns `ACH_RANGE`. There is no
partial-success publication or ownership transfer.

`ach_trace` admits0..64 byte addresses in0..UINT32_MAX, supplied as signed
64-bit values so negative and wider values can be rejected explicitly. A null
address pointer is allowed only for zero count. Line count is1,2,4 or8; block
size is a power of two from1 through256; ways is a power of two no greater
than line count. Total capacity is `lines*block_bytes`; number of sets is
`lines/ways`. Target replacement is LRU within each set. The reference shadow
is fully associative with the same number of blocks, selectable LRU or FIFO.
Reject the entire trace before modifying any output.

Derive block number=address/block size; offset=address%block size;
index=block number%sets; tag=block number/sets. Each output step contains these
fields, category, target full-block identities and explicit valid bits, plus
shadow full-block identities and used count. Target entries are grouped by set
and ordered most-recent first within each set; invalid entries and unused
slots are zero. Shadow entries are most-recently used first for LRU and
most-recently inserted first for FIFO. FIFO hits do not change order. No target
or shadow lookup occurs through a numerical simulated address.

Evaluate miss category from **pre-access** state. A target hit is `ACH_HIT`.
An unseen block is compulsory. A previously seen target miss is conflict when
the equal-capacity shadow still holds it, and capacity otherwise. Update both
target and shadow on every reference, including target hits. Keep first-seen
history independent of current residency. Zero tag without a valid entry is
not a hit. Under FIFO shadow this is a FIFO-relative classification; it is no
longer the original LRU three-C reference classification. Report the policy
with every category, and do not interpret it as a host hardware counter.

## Concrete traces and equivalent layout work

The original nine-reference trace0,16,0,64,0,16,80,16,0 must produce the full
fixture fields and categories: three hits, four compulsory misses, two conflict
misses. Reuse actual `algo_cache` for those aggregate values, then independently
verify every new snapshot. The six-reference capacity trace0,16,32,48,64,0 ends
in capacity. With four total lines and two ways, the first trace instead has
five hits and four compulsory misses, with no conflict/capacity miss.

The shadow-policy witness0,16,32,48,0,64,0 ends with a target miss classified
conflict by LRU and capacity by FIFO. Explain which hit preserved0 in LRU and
why that hit did not reorder FIFO. Changing only the shadow leaves the actual
target hit sequence unchanged; classification is a diagnostic comparison.

Compare two original useful-field layouts with identical logical work: read
eight four-byte selected fields twice, either at addresses32*i or4*i. Under
four16-byte direct-mapped lines, the strided representation has zero hits in16
accesses; the packed representation has14. These are declared byte layouts,
not assertions about an unspecified C struct's padding. Separately traverse
all64 four-byte entries of an8×8 row-major matrix: row order has48 hits and
column order has zero under the same geometry. Both visit exactly the same
64 offsets once; locality differs. The models exclude prefetching, TLBs,
writebacks, multilevel caches and physical timing. Do not convert counts into
claimed measured speedup.

## Dimensional arithmetic and overlapping service

`ACHFraction` is a nonnegative numerator and positive denominator, each32 bits.
Normalize outputs to coprime terms. Checked cross-cancellation and64-bit
intermediates must not overflow. The admitted arithmetic implementation may
return `ACH_RANGE` when an intermediate numerator/denominator cannot fit32
bits even if a later reduction could fit; document this conservative range.
Zero references makes AMAT undefined and is rejected. Miss count cannot exceed
reference count, estimated miss rate must be at most one, frequency is positive,
and the total-miss-latency flag is0 or1. Reject total latency below hit time.

AMAT in nanoseconds is hit time plus observed miss fraction times **additional**
miss penalty. If supplied miss time is total latency, subtract hit time first.
Separately compute base cycles=instructions*CPI; stall cycles=estimated memory
references*estimated miss rate*penalty cycles; total cycles is their sum;
time in ns=total cycles*10^9/frequency in Hz. These are different models and
different miss-rate inputs. The original fixture gives21ns AMAT,120 base and
120 stall cycles,240 total cycles and120ns at2GHz. Supplying31ns total miss
latency gives the same21ns AMAT as30ns additional penalty. A fractional case
has hit3/2, additional13/2, miss2/7, CPI7/3,9 instructions,7 memory references,
miss rate2/7,5 penalty cycles and1GHz: AMAT47/14ns and31 total cycles/ns.

`ach_overlap` is a separate declared service model for0..16 misses. Each request
has release0..256, positive latency1..256 and either no predecessor(−1) or one
strictly earlier request. There are1..4 simultaneous service slots. At each
integer tick, completions release slots first. Issue at most one oldest-ready
request, requiring release time reached, predecessor complete and a free slot.
Finish=start+latency; service intervals are half-open. A dependent request may
start exactly at predecessor finish. Empty input has makespan0. The admitted
worst chain completes by4352; retain this endpoint rather than dropping it.

Four independent20-cycle misses released at0 take80 cycles with one slot,
41 with two, and23 with four, because the one-per-tick issue limit remains.
Their summed service work is80 in every case. A dependency chain still takes80
with four slots. Staggered releases and unequal latencies distinguish capacity
from readiness. This counterexample invalidates blindly summing penalties as
elapsed duration; it does not establish real CPU overlap, MSHR counts or AMAT.

## Reasoning, deliverables and verification

D1 derives address partitions, valid/history/residency invariants, exact LRU
and FIFO changes, and dimensional equations before execution. D2 is the new
core plus the exact retained-producer comparison. D3 contains complete source-
bound certificates, resource accounting, failed-input preservation and seven
intended defect diagnoses. D4 is each original delayed transfer on a new trace
after at least seven days. D5 separates model, author execution, emulator,
independent learner and physical evidence.

Target/set uniqueness and shadow capacity hold after every reference. For LRU,
derive shadow presence independently from distinct blocks since the previous
reference; for FIFO, show hit order stability. Trace work is O(n²+n*lines) with
the explicit linear first-seen history, not a claimed constant-time lookup.
All storage is bounded by64 accesses/eight lines; no unbounded allocation exists.
Rational helpers use Euclid and checked products; request scheduling is bounded
by4353 ticks times16 requests. Actual compiler frame reports and final target
symbols are required; neither replaces a full stack/high-water analysis.

Run `ordinary`, `layouts`, `geometry`, `costs`, `overlap` under six GCC/Clang
debug/optimized/sanitized profiles and the dedicated Thumb image. Check the
original fixture, complete state, both layout pairs, all admitted set/way and
endpoint-address interactions, empty state, late invalid address, arithmetic
rejection and the maximal4352-tick chain. A separate Python list/stack-distance
oracle checks69 trace certificates; exact rational arithmetic checks three cost
packets; an event-driven oracle checks12 overlap schedules. Matrix checks
support a finite model claim, not absolute mastery.

Reject the seven mutants at their intended assertion: valid-bit omission,
ignored associativity, all repeated misses mislabeled conflict, FIFO hits
incorrectly reordered, serialized overlapping service, wrong seconds/ns scale,
and total miss latency treated as additional penalty. Build errors or unrelated
crashes do not count. Learner scaffolds must compile and fail semantically;
assessor implementations and private producer extraction stay withheld.

## Rubrics, delayed variants and graduated hints

For .07a, level2 requires every partition/valid/residency transition and two-way
result, not just six misses; level3 solves its exact two-way changed task on an
unseen trace. For .07b, level2 derives units, equivalent useful work and rejects
the two cost mutants; level3 completes the original overlapping-miss task with
readiness/resource intervals. For .07c, level2 derives the last capacity miss
and pre-access shadow evidence; level3 completes the original FIFO-shadow task
and qualifies the changed interpretation. Level0 violates a contract; level1
replays totals without the corresponding derivation. Original IDs are
`AX-PR-T37-05-T37.07.a-V1`, `.b-V1`, `.c-V1` with their full corresponding ID
prefix, minimum seven days. Author execution does not satisfy that learner delay.

Hints: first split a byte address into block/offset, then index/tag; next draw
valid entries separately from zero-filled storage; then ask whether the block
was ever seen and whether the pre-access shadow still holds it. For units,
write ns and cycles on separate columns before any division. For overlap,
draw half-open service intervals and distinguish ready requests from free slots.
Assessor solution details and extra diagnosis cases remain in the private file.

## Security and validation limits

Only original synthetic addresses, traces and dedicated emulator programs are
used. Numerical addresses are model inputs, never host/target pointer probes.
No cache side-channel attack, third-party data or host security change occurs.
Current author evidence is `legacy_architecture_cache/validation.json` and
actual learner export evidence is `exports.json`, each source-bound. Original
receipts remain unchanged. External review, independent learner performance,
physical cache/timing and unavailable final-edition sources remain separate.

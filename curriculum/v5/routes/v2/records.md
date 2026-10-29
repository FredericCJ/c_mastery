# Record processing: ordering follows the query

In `contextual-v2` the representation path is authored as episodes:
EP-algorithmic-vocabulary (U-T01-01-01, U-T01-01-02), EP-sequence-representations,
EP-ordered-search (U-T15-01-01..04, X-T15-002), EP-cost-contracts
(U-T43-01-01..03), EP-amortized-growth (LAB-EX-01, U-T14-02-02..04, X-T14-017,
U-T43-02-01, U-T43-02-02), EP-adversarial-partitions (U-T15-02-01..04),
EP-key-model (LAB-EX-04, U-T15-03-02, U-T15-03-03), EP-search-kernel-abi
(U-T36-01-01..02, U-T36-02-01..04), EP-sort-heap-kernel-abi (U-T36-03-01..03,
U-T36-04-01..02), EP-locality-traffic (U-T43-05-01..03), EP-paired-measurement
(LAB-EX-12, U-T43-08-02..03, X-T43-001), EP-pmu-counting, EP-layout-columns
(X-T70-001, LAB-DO-01, LAB-DO-02) and EP-work-to-decision (U-T43-03-01..03).
The heap kernel that heap sort reuses is built earlier, in EP-deadline-heap.

Start with a fixed record array, linear filtering and a stable output order. The
first consumer needs a bounded answer, so a scan is a useful complete design.
Before adding an index, write the operation mix: construction, successful and
unsuccessful lookup, insertion, deletion, range iteration and full traversal.

PR-T15-01 introduces lower bound only after a sorted partition is available
(U-T15-01-01 derives it; U-T15-01-03 implements it against a linear oracle).
Maintain the half-open candidate interval; prove that its length decreases and
that every excluded index belongs to the appropriate comparator partition. An
empty result is distinct from a found equal key. Comparator consistency, integer
conversion and borrowed key lifetime are separate admission obligations.

PR-T15-02 changes duplicate frequency and adversarial partitions (U-T15-02-01..04).
Derive the depth and work recurrences under the actual partition rule, including
equal keys; bounded recursion depth does not bound work. Use tagged
records to test stable algorithms and do not require stability from a contract
that does not promise it. PR-T15-03 changes the model to bounded key digits:
counting/radix storage and per-digit stability replace comparison-only assumptions.
Explain why the comparison sorting lower bound does not prohibit linear work
under this stronger key model.

Reuse the same ordered records in configuration lookup and durable range scans.
Compare array-of-structures and structure-of-arrays layouts with identical keys,
payload accesses and result checks. Sweep sizes across the detected working-set
regimes; record build identity, affinity, warm-up and ordering. Inspect whether
the compiler retained a branch before attributing a branch-event change to it.
A cache-miss difference does not by itself identify which level or access caused it.

The ordered-search mechanism moves from five-element traces (U-T15-01-01) to the
assembly kernel (U-T36-02-02), whose changed assumption is the full uint32_t key
range at 0x80000000 (U-T36-02-04), and transfers to multilevel search (X-T14-015).
Stable ordering transfers to versioned run merging (X-T15-017).

For independent transfer, provide a read-mostly catalog with bursts of deletion,
stable record identity and a bounded scratch budget. Require a choice, a rejected
alternative and a workload that would reverse that choice. A sorted array, hash
index and tree are candidates, not predetermined answers. Acceptance requires
query-equivalent outputs, a cost argument and revision after the changed workload.

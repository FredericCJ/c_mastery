# Record processing: ordering follows the query

Start with a fixed record array, linear filtering and a stable output order. The
first consumer needs a bounded answer, so a scan is a useful complete design.
Before adding an index, write the operation mix: construction, successful and
unsuccessful lookup, insertion, deletion, range iteration and full traversal.

PR-T15-01 introduces lower bound only after a sorted partition is available.
Maintain the half-open candidate interval; prove that its length decreases and
that every excluded index belongs to the appropriate comparator partition. An
empty result is distinct from a found equal key. Comparator consistency, integer
conversion and borrowed key lifetime are separate admission obligations.

PR-T15-02 changes duplicate frequency and adversarial partitions. Derive the
recurrence under the actual partition rule, including equal keys. Use tagged
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

For independent transfer, provide a read-mostly catalog with bursts of deletion,
stable record identity and a bounded scratch budget. Require a choice, a rejected
alternative and a workload that would reverse that choice. A sorted array, hash
index and tree are candidates, not predetermined answers. Acceptance requires
query-equivalent outputs, a cost argument and revision after the changed workload.

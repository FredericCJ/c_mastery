# Deadline collection: independent author-evidence review

The source-bound `deadlines.json` campaign passed its declared scope. This review
verified every one of its 34 command-log hashes, all ten current source hashes,
and both measured binary hashes. GCC and Clang each passed debug, optimized and
ASan/UBSan protocol runs reporting 36000 independent histories across all three
representations. Stale-identity, inverse-position and lost-periodic-registration
mutants failed semantically; the unfinished learner implementation also failed
as intended. Those tests and native execution validate different obligations.
The original receipt's external-review and independent-learner gates remain
unchanged: this document is another agent's review of author evidence, not a
human learner assessment.

Both compilers produced 837 valid raw benchmark records: nine size/pattern
regimes, 31 blocks, three representations. Every one of the 279 triples per
compiler has exactly the scan/heap/wheel kinds and identical checksums. Checksums
also agree between compilers for each trace. I recomputed all recorded medians
and paired differences directly from raw rows; `deadlines-analysis.json` retains
those results, separate preparation and combined interval medians, memory sizes,
paired win counts, and the hash of the reviewed receipt.

## Work performed and what the timer includes

Each trial starts with n live one-shot deadlines, then performs exactly 256
steps: 64 cancellation attempts, 192 arm/rearm operations, and 256 minimum
queries. A query's selected slot/deadline is folded into an observable checksum.
The independent oracle uses a flat deadline array and an explicit minimum scan;
the timed implementations use the real `TcCollection` and retained `TdTimer`.
This is mutation/minimum-selection throughput, not expiration, dispatch or
callback throughput. `tc_begin` and `tc_finish` are exercised by the finite and
native tests, and by the separate integrated-service PMU experiment.

The timed interval includes mutation, selection and checksum fold. Allocation,
initialization/initial insertion, independent oracle and free are excluded;
initialization/insertion time is reported separately as `preparation_ns`.
Combining preparation and mutation intervals still excludes allocation/free and
is not an end-to-end wall measurement. Each regime warms all three variants
once, then rotates order each block. Trials allocate fresh arrays, so this is
not a single persistent service whose state continues across blocks.

Pattern 0 generates `1 + (slot*37)%8191`, dispersed deadlines. Pattern 1 generates
`1000 + slot%3`, dense equal/near-equal deadlines. Pattern 2 generates
`1000 + 64*(slot%61)`, periodic alignment. The wheel has 64 buckets and quantum
16: pattern 1 uses one bucket; pattern 2 aliases into 16 buckets. The wheel is
a sorted-bucket exact-minimum structure: selection scans all bucket heads and
insertion walks its selected sorted list. It is not a claim of constant-time
classic timing-wheel insertion or expiration. These operation patterns
therefore discriminate bucket occupancy as well as structure size.

The slot sequence `(step*73)%n` repeats at n=16; only four distinct slots are
cancelled there, so 60 later cancellation attempts are no-ops. At n=256 and
4096, all 64 cancellations in the trace hit distinct live slots. The workload
mix changes with size in this specific way. It is deterministic and bounded,
with no randomized deadline arrival, advancing time or callback costs.

## Observed comparison

Median elapsed microseconds per 256-step batch follow. Kind 0 is scan, 1 heap,
2 sorted-bucket wheel. These are descriptive host observations; small differences
are not calibrated latency claims or population confidence intervals.

| Compiler | n | Pattern | Scan µs | Heap µs | Wheel µs |
| --- | ---: | ---: | ---: | ---: | ---: |
| gcc | 16 | 0 | 8.032 | 7.054 | 17.181 |
| gcc | 16 | 1 | 8.032 | 6.705 | 19.137 |
| gcc | 16 | 2 | 8.102 | 6.635 | 18.648 |
| gcc | 256 | 0 | 73.334 | 8.032 | 34.292 |
| gcc | 256 | 1 | 63.766 | 7.124 | 42.674 |
| gcc | 256 | 2 | 54.895 | 6.146 | 19.975 |
| gcc | 4096 | 0 | 743.050 | 7.682 | 37.714 |
| gcc | 4096 | 1 | 738.021 | 9.848 | 490.641 |
| gcc | 4096 | 2 | 861.082 | 7.962 | 68.236 |
| clang | 16 | 0 | 9.080 | 8.102 | 18.229 |
| clang | 16 | 1 | 9.219 | 7.193 | 19.416 |
| clang | 16 | 2 | 9.499 | 7.613 | 19.486 |
| clang | 256 | 0 | 71.029 | 8.032 | 27.448 |
| clang | 256 | 1 | 69.982 | 8.241 | 43.931 |
| clang | 256 | 2 | 70.750 | 8.101 | 21.790 |
| clang | 4096 | 0 | 1021.649 | 9.987 | 41.137 |
| clang | 4096 | 1 | 1013.688 | 10.476 | 492.666 |
| clang | 4096 | 2 | 1015.573 | 9.918 | 69.493 |

At n=256 and 4096 the heap wins all 31 mutation-interval comparisons against
scan in every pattern/compiler. At n=16 it wins 27–31/31, with much smaller
absolute differences. The wheel loses every n=16 comparison to scan, wins all
n=256 comparisons, and wins 30–31/31 at n=4096. This supports workload-dependent
selection and update tradeoffs rather than a universal ranking. Both compilers
show the same broad shape; they ran separately, so their time difference is
not an isolated compiler-quality experiment.

The dense wheel case exposes a material setup tradeoff. At n=4096, pattern 1,
GCC preparation medians are 46.166 µs scan, 41.906 µs heap and **7005.574 µs
wheel**. Its mutation median is another 490.641 µs. Thus the wheel's win over
scan inside the reported mutation interval does not imply a win for a collection
built once for that batch. Sorted insertion into one bucket walks growing lists;
the retained code supports that explanation. Clang shows the same roughly
7.027-ms wheel preparation. Pattern 0's GCC wheel preparation is 197.653 µs,
and pattern 2's 642.825 µs, consistent with their different bucket occupancy.
These observations motivate measuring the actual expected ratio of creation,
mutation, query and dispatch work before choosing a representation.

Reported retained array storage at n=4096 is 262144 bytes scan, 327680 heap and
360960 wheel; at n=16 it is 1024, 1280 and 1920. The count includes timer/token
arrays and representation metadata, but excludes allocator headers, the
`TcCollection` stack object, stack frames and unrelated process memory. It is
an explicit storage model, not RSS. All three use stable caller-owned slots;
heap inverse positions and wheel links add storage to accelerate selected
operations. Generation tokens are scoped to their owning collection; these
measurements do not establish cross-collection token validation.

## Generated code and native integration

Both `-O3 -g` binaries are unsanitized; their retained symbols and disassembly
show actual calls within the measured loop. GCC retains `tc_arm` at ELF 0x18ce,
`tc_cancel` at 0x1969, `tc_peek` at 0x1900/0x199b and the 64-bit checksum multiplier
0x100000001b3. Clang retains corresponding calls at 0x1d4f/0x1be4/0x1c12 and the
same checksum fold. Timer reads bracket the loop. The compiler has not replaced
the workload with a precomputed result. `tc_peek` retains separate scan, heap
and bucket paths; the wheel's sorted insertion and the heap's repair code remain.
This verifies that the representation comparison survives optimization; elapsed
time alone cannot attribute particular cache misses or branch mispredictions.
No PMU counts belong to this standalone deadline benchmark receipt.

The Linux adapter uses absolute `timerfd` deadlines and poll/read wakeups. Each
compiler/representation successfully cancels one slot, dispatches three live
slots (including tied deadlines ordered by slot), calls finish, and empties the
collection. The six reported maxima span 13.893–55.100 µs late. Each is a tiny
smoke run's observed maximum, not a distribution estimate, representation ranking,
scheduling guarantee or WCET. The mutation benchmark and timerfd smoke test
exercise complementary boundaries and their timings must not be combined.

## Limits and next discriminating evidence

The benchmark records neither timer-read overhead/calibration nor CPU affinity,
endpoint migration or frequency. In particular, the smallest 6–20-µs batches
should not support precise per-operation or universal crossover claims. Thirty-one
rotated blocks do not remove operating-system interference. All observations
are retained; no outlier filtering or statistical significance is asserted.
Checksum agreement is strong experimental agreement for these complete traces,
not a proof against hash collision; finite state/history tests provide separate
semantic evidence. Return values in the timed mutation loop are not each checked;
its final checksum is the benchmark acceptance check, with contracts validated
by the separate finite suite.

The next design decision should use the intended ratio of initialization,
cancellation, rearm, minimum queries and actual dispatch, with calibrated batch
duration. Dense-bucket insertion, period crossing, long-lived collections and
unfamiliar workload distributions remain useful transfer tasks. The current
results support choosing and revising a representation for this explicit trace;
they do not certify a learner's independent or delayed transfer.

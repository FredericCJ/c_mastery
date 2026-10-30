```json
{
  "schema_version": 2,
  "kind": "assessor-packet",
  "packet": "deadlines",
  "path": "deadline-structures",
  "route": "contextual-v2",
  "tasks": [
    {
      "id": "T-DL-1",
      "title": "Cancellable service appointments",
      "role": "primary",
      "initial_follows": {
        "nodes": ["LAB-HS-03", "U-T14-05-04", "U-T24-05-02", "LAB-RZ-05", "LAB-RZ-06", "U-T24-05-03"],
        "episodes": ["EP-deadline-scan", "EP-deadline-heap", "EP-indexed-cancellation", "EP-timer-wheel", "EP-deadline-comparison"]
      },
      "changed_follows": {
        "nodes": ["X-T75-003", "U-T24-05-03"],
        "episodes": ["EP-deadline-transfer", "EP-deadline-comparison"]
      },
      "also_required_nodes": [],
      "bound_not_required_acceptance": ["X-T75-003"],
      "mechanisms": ["deadline-collections"],
      "gap_mechanisms": [],
      "requires_selection": true,
      "min_rejected_alternatives": 2,
      "requires_revision_under_changed_assumption": true,
      "released_after_seal": true,
      "changed_assumption": "Pending appointments drop to 256, wakeup lateness up to 2 milliseconds is acceptable, deadlines nearly all coincide, cancellation rises to 95 percent and an occasional deadline is an hour away.",
      "canaries": [
        {"public": "four-slot model and bounded scan of 4-8 entries (LAB-HS-03, LAB-RZ-05)", "task": "4096 pending appointments, generation limit reduced to 3 for the exhaustion trace"},
        {"public": "equal-trace comparison of sparse, clustered, cancellation-heavy and periodic traces (U-T24-05-03)", "task": "the 4000-admit, cancel-half, replace mix and a running-callback cancellation reporting retained ownership"}
      ]
    },
    {
      "id": "T-DL-2",
      "title": "Lateness report in a fixed byte budget",
      "role": "gap",
      "initial_follows": {
        "nodes": ["U-T24-05-03", "X-T75-003"],
        "episodes": ["EP-deadline-comparison", "EP-deadline-transfer"]
      },
      "changed_follows": {
        "nodes": ["X-T75-003"],
        "episodes": ["EP-deadline-transfer"]
      },
      "also_required_nodes": ["X-T15-005", "X-T15-006", "X-T15-009"],
      "bound_not_required_acceptance": ["X-T75-003"],
      "also_required_episodes": ["EP-summaries"],
      "mechanisms": ["deadline-collections", "summaries-selection"],
      "gap_mechanisms": ["summaries-selection"],
      "requires_selection": true,
      "min_rejected_alternatives": 2,
      "requires_revision_under_changed_assumption": true,
      "released_after_seal": true,
      "changed_assumption": "Shards report in any order and may report twice, the requested percentile rises to the 99.9th, and some deliveries are tens of seconds late.",
      "canaries": [
        {"public": "weighted Count-Min, HyperLogLog and t-digest exercises with their own byte budgets (X-T15-005, X-T15-006, X-T15-009)", "task": "a 2 KiB per-shard budget for one exact maximum, one exact threshold count and one nearest-rank percentile, mergeable across four shards"}
      ]
    },
    {
      "id": "T-DL-3",
      "title": "Payload reclamation under cancellation",
      "role": "gap",
      "initial_follows": {
        "nodes": ["U-T24-05-02", "X-T75-003"],
        "episodes": ["EP-indexed-cancellation", "EP-deadline-transfer"]
      },
      "changed_follows": {
        "nodes": ["X-T75-003"],
        "episodes": ["EP-deadline-transfer"]
      },
      "also_required_nodes": ["U-T16-03-03", "U-T55-05-01", "X-T55-002"],
      "bound_not_required_acceptance": ["X-T75-003"],
      "also_required_episodes": ["EP-allocator-pools", "EP-reclamation"],
      "mechanisms": ["deadline-collections", "allocation-pools", "concurrent-reclamation"],
      "gap_mechanisms": ["allocation-pools", "concurrent-reclamation"],
      "requires_selection": true,
      "min_rejected_alternatives": 2,
      "requires_revision_under_changed_assumption": true,
      "released_after_seal": true,
      "changed_assumption": "Four dispatcher threads run callbacks that may block for an unbounded time.",
      "canaries": [
        {"public": "three-slot pool and generation-checked fixed-block pool (U-T16-03-01, U-T16-03-03); capacity-4 slot reuse chains (U-T55-01-01)", "task": "256 payload slots of 256 bytes in a 65,536-byte region, stale handles from cancelled appointments and a stalled callback"}
      ]
    }
  ],
  "criteria": [
    {
      "id": "DL-M1",
      "name": "Collection invariants and workload-dependent cost",
      "dimensions": ["mechanism"],
      "tasks": ["T-DL-1"],
      "earns_2": "States and preserves every ordering and index invariant of the chosen collection (the key order on deadline then slot, the consistency of any index from slot to position, generation per slot) across arm, cancel, rearm and expiry.",
      "earns_3": "Also derives the cost comparison for the stated mix with explicit counts (4,000 expiries, 6,000 admissions, 2,000 cancellations) and finds the crossover where the ranking reverses.",
      "rejects": [
        "Asserting that a logarithmic structure always beats a linear scan without counting the operation mix.",
        "Breaking equal-deadline ties by arrival order instead of by slot.",
        "Cancelling a heap entry without an index from slot to position, so cancellation silently costs a linear search."
      ]
    },
    {
      "id": "DL-C1",
      "name": "Agreement with an independent trace and stale handles",
      "dimensions": ["correctness"],
      "tasks": ["T-DL-1"],
      "earns_2": "Matches the assessor trace exactly (slots 0 then 1 at 100, the stale slot 2 notification never dispatches, slot 2 once at 110) and an independently coded stable scan of an immutable event log on tiny exhaustive histories and bounded larger traces, including periodic catch-up and integer-edge rejection.",
      "earns_3": "Also falsifies the assessor's stale-handle mutant (dispatch after rearm when the old deadline is due) and tie-order mutant with a trace the learner constructs.",
      "rejects": [
        "Dispatching a stale notification because its original deadline is not later than now.",
        "Treating a deadline equal to now as not yet due.",
        "Adding a delay to now in uint64 without checking for overflow."
      ]
    },
    {
      "id": "DL-O1",
      "name": "Request, callback borrow and reclamation",
      "dimensions": ["memory_ownership"],
      "tasks": ["T-DL-1"],
      "earns_2": "Distinguishes the cancellation request, the callback's borrow and reclamation: cancellation of a running callback reports retained ownership, a second request is idempotent and storage is not reclaimed before the callback finishes.",
      "earns_3": "Also adapts the distinction through shutdown, with a trace in which stop arrives while a callback is borrowed.",
      "rejects": [
        "Reporting a running callback as cancelled and reclaiming its storage at once.",
        "Releasing twice when cancellation is requested twice while borrowed."
      ]
    },
    {
      "id": "DL-R1",
      "name": "Live entries, stale state and worst-case bursts",
      "dimensions": ["resources_dependencies"],
      "tasks": ["T-DL-1"],
      "earns_2": "Bounds the live entries by 4096 and every piece of stale state (tombstones, retired generations) by a stated number independent of the run length.",
      "earns_3": "Also bounds the worst single call, for example 4096 equal deadlines expiring in one dispatch, rather than quoting an amortized mean.",
      "rejects": [
        "Deleting lazily and letting tombstones wait for their deadline, which never frees those of an hour-away deadline under 95 percent cancellation.",
        "Quoting amortized O(1) as a bound on one dispatch call."
      ]
    },
    {
      "id": "DL-D1",
      "name": "Semantics apart from measurement noise",
      "dimensions": ["failure_diagnosis"],
      "tasks": ["T-DL-1"],
      "earns_2": "Checks equal delivered identities and effects before any timing, keeps the check out of the timed interval and separates a semantic failure from run-to-run noise by reading the raw samples.",
      "earns_3": "Also proposes an experiment that can reverse the design (a sweep of cancellation fraction and tie fraction with a predicted crossover) and states in advance what would refute the prediction.",
      "rejects": [
        "Declaring a winner from a faster median whose raw samples overlap.",
        "Timing a run that includes the oracle comparison.",
        "Reading a timing difference as evidence that the behaviours are equal."
      ]
    },
    {
      "id": "DL-T1",
      "name": "Independent selection under the new contract",
      "dimensions": ["communication_transfer", "mechanism"],
      "tasks": ["T-DL-1"],
      "earns_2": "Selects under 256 appointments, 2 ms lateness, near-coincident deadlines, 95 percent cancellation and an hour-away outlier from derived costs, and declares the revised precision contract before measuring any design that rounds times.",
      "earns_3": "Also defends a choice that retains the original representation when the evidence warrants, or justifies reversing it with the changed term identified, and states which earlier proofs were replaced.",
      "rejects": [
        "Choosing a structure because timers are conventionally built from it.",
        "Rounding deadlines to the tolerance and still claiming exact delivery order.",
        "Rejecting a linear scan because it is linear, with 256 entries in 2 KiB."
      ]
    },
    {
      "id": "DL-M2",
      "name": "Summary selection from the budget",
      "dimensions": ["mechanism", "resources_dependencies"],
      "tasks": ["T-DL-2"],
      "earns_2": "Derives the budget arithmetic (10^9 samples of 4 bytes is 4 GB against 2,048 bytes), keeps the maximum and the threshold count exact, and gives the percentile a stated guarantee with its assumptions and the counter-overflow bound (counts total at most 10^9, below 2^32).",
      "earns_3": "Also derives a deterministic bound for the chosen percentile summary (for geometric buckets, roughly 470 of them give a value ratio near 1.048 with an exact rank) and shows the merge is associative and commutative.",
      "rejects": [
        "Using a sketch for the exact maximum or the exact threshold count.",
        "Using uniform-width buckets over the whole range, which put the 2,000 microsecond boundary inside the first bucket.",
        "Claiming a deterministic percentile bound from a random sample of 256 values."
      ]
    },
    {
      "id": "DL-C2",
      "name": "Exact numbers exact, estimates labelled",
      "dimensions": ["correctness"],
      "tasks": ["T-DL-2", "T-DL-3"],
      "earns_2": "Reproduces the assessor traces: for 990 samples of 100 and 10 of 5,000 the reported p99 is within the declared factor of 100 (not of 5,000) with 10 exactly above 2,000, and for 989 and 11 it is within the declared factor of 5,000 with 11 exactly above 2,000, so the two traces are distinguished; the payload schedule produces no read of reclaimed memory under the address sanitizer with failure injected at every acquisition point.",
      "earns_3": "Also builds an independent oracle (a sorted copy of the samples; a model of payload states) and uses it to find the assessor's mutant (off-by-one in the rank; reclamation on the first of two cancellation requests).",
      "rejects": [
        "Rounding the rank 990 to 989 and reporting the wrong sample.",
        "Believing that a run without a sanitizer report shows reclamation is safe on schedules it did not hit."
      ]
    },
    {
      "id": "DL-D2",
      "name": "Deterministic versus probabilistic claims",
      "dimensions": ["failure_diagnosis", "communication_transfer"],
      "tasks": ["T-DL-2"],
      "earns_2": "States for each reported number whether its guarantee is exact, deterministic with a bound, or probabilistic under a named assumption, and predicts before running the effect of a duplicated shard report (maximum unchanged, counts increased, percentile shifted toward that shard).",
      "earns_3": "Also predicts the interval of the 99.9th percentile estimate, and explains why a published figure cannot be corrected by merging a late shard without a sequence number.",
      "rejects": [
        "Applying an error bound observed on benign data to an arbitrary or adversarial order.",
        "Treating a probability formula as a per-query guarantee.",
        "Assuming a merge of summaries is idempotent."
      ]
    },
    {
      "id": "DL-O3",
      "name": "Shard state during a report",
      "dimensions": ["memory_ownership"],
      "tasks": ["T-DL-2"],
      "earns_2": "States who owns each shard's state while a report is taken and takes the report from a consistent copy or an explicit handoff, so the maximum, the threshold count and the percentile counters describe the same set of dispatches.",
      "earns_3": "Also constructs the torn report (a dispatch of 9,000 microseconds lands between reading the threshold count and the maximum, so the count excludes the sample the maximum includes) and shows the ordering or handoff that excludes it, with the cost to the dispatching thread.",
      "rejects": [
        "Reading a live shard's counters one at a time while it keeps updating and calling the result a snapshot.",
        "Making each counter atomic and concluding that the set of counters is consistent."
      ]
    },
    {
      "id": "DL-O2",
      "name": "Payload reclamation under a stalled callback",
      "dimensions": ["memory_ownership", "resources_dependencies"],
      "tasks": ["T-DL-3"],
      "earns_2": "Reclaims a payload at once when cancellation precedes the callback and only after completion when the callback runs, with no read of reclaimed memory, no leak after shutdown and bounded time per call.",
      "earns_3": "Also computes the consequence of a stalled callback (256 slots; with one slot pinned, per-payload reclamation admits all 1,000 cancel-and-admit cycles while a reclaim-when-idle policy admits 255), adapts to four dispatchers and defends the policy against a second one by a schedule.",
      "rejects": [
        "Freeing the payload when cancellation is requested.",
        "Deferring all reclamation to a point at which no callback is running, which a single stalled callback blocks.",
        "Decrementing a reference count without ordering the final release against the callback's reads."
      ]
    }
  ],
  "worked_numbers": [
    {"id": "scan-initial", "expr": "4000*4096", "expected": 16384000, "text": ["16,384,000"], "occurrences": 1},
    {"id": "heap-initial", "expr": "4000*24+6000*12+2000*24", "expected": 216000, "text": ["216,000"], "occurrences": 1},
    {"id": "initial-ratio", "expr": "round(4000*4096/(4000*24+6000*12+2000*24))", "expected": 76, "text": ["about 76"], "occurrences": 1},
    {"id": "admissions", "expr": "4000+2000", "expected": 6000, "text": ["6,000"], "occurrences": 3},
    {"id": "wheel-ticks", "expr": "3600000/2", "expected": 1800000, "text": ["1,800,000"], "occurrences": 1},
    {"id": "wheel-memory-mb", "expr": "1800000*4/1e6", "expected": 7.2, "tol": 1e-09, "text": ["7.2 MB"], "occurrences": 1},
    {"id": "three-level-reach", "expr": "128**3", "expected": 2097152, "text": ["2,097,152"], "occurrences": 1},
    {"id": "three-level-buckets", "expr": "3*128", "expected": 384, "text": ["384 buckets"], "occurrences": 1},
    {"id": "all-samples-gb", "expr": "10**9*4/1e9", "expected": 4.0, "tol": 1e-09, "text": ["4 GB"], "occurrences": 2},
    {"id": "uniform-width-seconds", "expr": "3.6e9/480/1e6", "expected": 7.5, "tol": 1e-09, "text": ["7.5 seconds"], "occurrences": 1},
    {"id": "ln-range", "expr": "math.log(3.6e9)", "expected": 22.0, "tol": 0.001, "text": ["22.0"], "occurrences": 1},
    {"id": "bucket-ratio", "expr": "math.exp(math.log(3.6e9)/470)", "expected": 1.048, "tol": 0.001, "text": ["1.048"], "occurrences": 3},
    {"id": "reservoir-rank-sd", "expr": "math.sqrt(0.99*0.01/256)", "expected": 0.0062, "tol": 0.01, "text": ["0.0062"], "occurrences": 1},
    {"id": "u32-limit", "expr": "2**32", "expected": 4294967296, "text": ["4,294,967,296"], "occurrences": 1},
    {"id": "p99-rank", "expr": "math.ceil(0.99*1000)", "expected": 990, "text": ["rank 990"], "occurrences": 2},
    {"id": "payload-slots", "expr": "65536//256", "expected": 256, "text": ["65,536 / 256 = 256"], "occurrences": 1},
    {"id": "per-payload-admits-idle", "expr": "256-1", "expected": 255, "text": ["admits 255"], "occurrences": 2},
    {"id": "idle-fails", "expr": "1000-255", "expected": 745, "text": ["fails the remaining 745"], "occurrences": 1},
    {"id": "four-dispatchers-usable", "expr": "256-4", "expected": 252, "text": ["252 usable"], "occurrences": 1}
  ],
  "leak_fingerprints": ["slots 0, 1, 2, 3 with deadlines 100, 100, 90, 120", "The service dispatches up to 10^9 appointments over its lifetime", "A local service admits at most 4096 pending appointments"],
  "entry_check": {
    "accepted_artifacts": [
      {"artifact": "bytes.checked", "producer_node": "U-T04-01-04", "needed_for": ["T-DL-1", "T-DL-2", "T-DL-3"]},
      {"artifact": "containers.ring", "producer_node": "U-T14-01-04", "needed_for": ["T-DL-1"]},
      {"artifact": "codec.records", "producer_node": "U-T20-01-04", "needed_for": ["T-DL-1"]},
      {"artifact": "io.deadline", "producer_node": "LAB-HS-03", "needed_for": ["T-DL-1", "T-DL-3"]},
      {"artifact": "containers.depth.heap", "producer_node": "U-T14-05-04", "needed_for": ["T-DL-1"]},
      {"artifact": "measurement.capture", "producer_node": "U-T43-01-03", "needed_for": ["T-DL-1", "T-DL-2"]},
      {"artifact": "memory.pool", "producer_node": "U-T16-03-03", "needed_for": ["T-DL-3"]}
    ],
    "source_identities": [
      "SHA-256 of curriculum/v5/routes/contextual-v2.json and the repository commit",
      "SHA-256 of the learner's timer, collection and event-loop adapter sources and of every imported bundle",
      "the build record of each run, including any TEST IMPLEMENTATION label",
      "compiler, flags, sanitizer and python3 versions; for hardware-backed comparison the perf_event_paranoid value and whether a counting group opened"
    ],
    "tools": ["C17 compiler with ASan, UBSan and ThreadSanitizer", "python3", "clock resolution record (CLOCK_MONOTONIC)", "perf_event_paranoid value (recorded, never changed)"],
    "probes": [
      {"id": "P-DL-1", "ask": "Prove one generation-exhaustion transition: what the call returns and why an already armed registration survives.", "tests_node": "LAB-HS-03", "remediation": "Redo the generation lifecycle and exhaustion trace with the reduced limit."},
      {"id": "P-DL-2", "ask": "What does cancellation mean while the callback is running, and who owns the storage then?", "tests_node": "U-T24-05-02", "remediation": "Redo callback borrowing through cancellation."},
      {"id": "P-DL-3", "ask": "Why does the service use absolute uint64 deadlines, and what happens to now plus a delay near the top of the range?", "tests_node": "U-T30-03-02", "remediation": "Redo modular deadline arithmetic and integer-edge rejection."},
      {"id": "P-DL-4", "ask": "How is a median of paired differences different from a difference of medians, and why does the raw sample list stay?", "tests_node": "X-T43-001", "remediation": "Redo paired-trial reporting."},
      {"id": "P-DL-5", "ask": "For a count-style or quantile summary you built, which assumption does its error statement rely on?", "tests_node": "X-T15-009", "remediation": "Redo the summaries episode (EP-summaries) before T-DL-2."},
      {"id": "P-DL-6", "ask": "What must be true of a removed object before its storage is reused?", "tests_node": "U-T55-05-01", "remediation": "Redo the reclamation episode (EP-reclamation) before T-DL-3."}
    ],
    "known_unstated": [
      "X-T75-003 lists execution.tasks (U-T50-04-03) among its component prerequisites and its environment names an imported learner conc.channel (LAB-SC-04), but conc.channel is not among its typed component prerequisites and neither producer is placed in an authored episode of this path (U-T50-04-03 is an unauthored block needed by EP-reclamation; LAB-SC-04 belongs to unauthored PR-T73-02). Consequence: a learner who follows only this path cannot have accepted X-T75-003, so no task here requires its acceptance (bound_not_required_acceptance). T-DL-1, T-DL-2 and T-DL-3 use no task scheduler or channel; they follow X-T75-003 as the route's own transfer encounter and require acceptance of U-T24-05-03 and LAB-RZ-06 instead. Do not fail a path-only learner for lacking execution.tasks or conc.channel; do record the gap against the route.",
      "T-DL-2 and T-DL-3 need work (EP-summaries, EP-allocator-pools, EP-reclamation) that the deadline-structures path does not contain; they are released only when those nodes are accepted in the learner's profile, otherwise the entry check fails as a route gap."
    ]
  },
  "imitation_probes": [
    {"check": "IM-VERBATIM", "applies_to": ["T-DL-1", "T-DL-2", "T-DL-3"], "compare_with": ["labs/hosted_systems/timer_depth/", "curriculum/v5/routes/v2/deadlines.md", "curriculum/v5/exercises/X-T75-003.md"], "probe": "Look for the public traces and slot counts in the learner's evidence; ask the learner to change the tie rule to latest slot first and name the lines and the traces that change."},
    {"check": "IM-ADAPTER", "applies_to": ["T-DL-1", "T-DL-2", "T-DL-3"], "compare_with": ["assessor/hosted_systems/", "assessor/resilience/", "labs/support/"], "probe": "Hash the learner's collection, timer and adapter sources against every assessor C/H/S file and the shipped lab files; the author campaign links assessor implementations and says nothing about learner sources."},
    {"check": "IM-REJECTED", "applies_to": ["T-DL-1", "T-DL-2", "T-DL-3"], "compare_with": [], "probe": "For each rejected alternative ask for a concrete workload on which it loses and run the assessor's model of it on that workload."},
    {"check": "IM-PREDICT", "applies_to": ["T-DL-1", "T-DL-2", "T-DL-3"], "compare_with": [], "probe": "Compare the sealed prediction with the worked answers below, then mutate one learner line offline (flip a tie comparison, free on first cancellation) and ask which trace fails first."}
  ]
}
```

# Withheld packet: deadline structures to a transfer under changed requirements

Path `deadline-structures` in `contextual-v2`. This packet follows the episodes
EP-deadline-scan, EP-deadline-heap, EP-indexed-cancellation, EP-timer-wheel,
EP-deadline-comparison and EP-deadline-transfer, and the nodes in each task's
manifest entry. The route's own transfer encounter (X-T75-003) names the
candidate collections it assesses; these tasks name none. Task T-DL-1 is primary.
T-DL-2 and T-DL-3 cover mechanisms the route review lists as having no
independent transfer (summaries; allocation and reclamation) in the setting of
this path's service. Reject an imported author reference as evidence of learner
construction.

### Task T-DL-1

#### Release: initial

A local service admits at most 4096 pending appointments. Each has a stable slot
identity, a monotonically increasing generation and an absolute uint64 deadline.
The workload starts with 4000 appointments spread across a minute, cancels 2000 of
them and admits 2000 replacements, while a small ready set is dispatched repeatedly;
every appointment that is not cancelled expires and runs exactly once.
Cancellation of a running callback reports retained ownership until completion, and
repeating a cancellation request has no further effect. An appointment whose
deadline equals the current time is due. Equal deadlines dispatch in stable slot order. No appointment may run early or
more than once. Reject arithmetic or generation exhaustion without destroying an
already armed registration.

Choose and implement the collection and its event-loop adapter. Derive costs for
admission, earliest query, cancellation and expiration under the specified mix.
Record the requirements you derived, your selection, two rejected alternatives with
a concrete workload that defeats each, and the condition under which you would
reverse your choice, before implementing. Measure only after checking equal
delivered identities and effects. Report storage, code or operation costs and
bounded timings separately; no implementation must win.

#### Release: changed condition

After the delay, the service admits only 256 appointments, accepts wakeup lateness
up to 2 milliseconds, and receives bursts in which almost every deadline is
identical. Cancellation rises to 95 percent, but occasional deadlines are an hour
away. Your earlier ranking may reverse. Adapt your design and its semantic
comparison. If your design rounds times, declare the revised precision contract
before measuring it. Before running, predict which of your two alternatives needs
fewer comparisons over 10,000 operations, which has the larger worst single call,
and what bound on lateness your design guarantees.

#### Assessor material (not released)

Use slots 0, 1, 2, 3 with deadlines 100, 100, 90, 120. Cancel slot 2, rearm it at 110 with
a new generation and retain its old notification. At now 100, deliver 0 then 1;
the old slot 2 notification cannot dispatch. At now 110, deliver 2 exactly once.
Begin slot 3 at 120, request cancellation twice while it is borrowed, then finish;
its storage cannot be reclaimed before finish. Repeat at reduced generation
limit 3 so exhaustion is observable without long runs; the exhaustion transition is
the learner's own accepted contract (P-DL-1), so score that a rejected arming leaves
the armed registration intact, not a particular number.

Compare against an independently coded stable scan of an immutable event log,
including periodic catch-up and integer-edge rejection. Run a tiny exhaustive
history first, then bounded larger traces. A source that merely labels failures
without invoking cancellation does not meet the test.

Worked costs, initial mix (comparison counts, not times). The release fixes 6,000
admissions (4,000 + 2,000), 2,000 cancellations and so 4,000 expiries. A scan with
earliest query over at most 4,096 entries costs at most 4,000 x 4,096 = 16,384,000
comparisons. A binary heap on (deadline, slot) with an index from slot to position
(12 levels, since log2 4096 = 12; an admission sifts up at most 12 comparisons, an
expiry sifts down at most 12 x 2 = 24, a cancellation does one sift of at most 24)
costs at most 4,000 x 24 + 6,000 x 12 + 2,000 x 24 = 216,000. The ratio
(about 76) is a comparison ratio; the scan reads 32 KiB sequentially and the heap
branches, so the timing ratio can be far smaller. Do not accept either as a
winner without equal-effect checks and raw samples.

Worked reversal, changed condition. With 256 live entries a scan costs 256
comparisons per query (2 KiB of deadlines) against 16 for a heap (8 levels, 2 per
level). A wheel with a 2 ms tick needs 3,600,000 / 2 = 1,800,000 buckets to cover
an hour in one level (7.2 MB of 4-byte heads for 256 entries); three levels of 128
buckets cover 128^3 = 2,097,152 ticks with 384 buckets. Rounding to the tick merges
deadlines that differ by less than 2 ms and changes the (deadline, slot) order unless
the bucket keeps it; the learner must declare which contract holds. Lazy cancellation
keeps 95 percent of entries as tombstones; the hour-away ones do not leave during the
run, so a bound on stale state must come from eager removal or from a cap. A learner
may defend the original representation: score the derivation, not the choice.

Seeded mutants for DL-C1: the stale-handle mutant (dispatch if the stored deadline
is due, ignoring the generation); the tie mutant (insertion order); the equality
mutant (`<` for `<=`); `now + delay` unchecked. Keep a raw record of the tiny
exhaustive history that distinguishes each.

### Task T-DL-2

#### Release: initial

The service dispatches up to 10^9 appointments over its lifetime. For each one,
lateness is the delivery time minus the deadline, in whole microseconds, from 0 to
3,600,000,000. An operator needs, for each of four worker shards and merged in
any order: the exact maximum lateness; the exact number of appointments later than
2,000 microseconds; and an estimate of the 99th-percentile lateness, defined as the
value at rank ceil(0.99 n) in sorted order, with a stated guarantee. The state for
all three may not exceed 2 KiB per shard, must be updated in bounded time per
dispatch and must not allocate. A report may be taken while a shard keeps dispatching.

Decide what to keep, what each reported number claims and under which assumptions,
and what a late shard report can change. Record the requirements you derived, your
selection, two rejected alternatives with the smallest input that defeats each,
and a predicted discriminating experiment before implementing.

#### Release: changed condition

Shards now report in any order and a shard may report twice. The requested percentile
is the 99.9th, and some deliveries are tens of seconds late during pauses. Adapt your
design. Before running, predict the guarantee your 99.9th-percentile estimate has,
and what a duplicated report from one shard does to each of the three numbers.

#### Assessor material (not released)

Arithmetic the learner should reach. Keeping every sample needs 10^9 x 4 bytes =
4 GB (values fit in 32 bits) against a 2,048-byte budget. A random sample of 256
eight-byte values has a rank standard deviation near sqrt(0.99 x 0.01 / 256) = 0.0062
and holds only about 2.6 values above the 99th percentile; its bound is
probabilistic. Uniform buckets over the full range with 480 four-byte counters are
about 7.5 seconds wide, so the 2,000 microsecond boundary lies inside the first.
Geometric buckets with ratio r over [1, 3.6 x 10^9] need ln(3.6 x 10^9)/ln r buckets
(about 22.0 / ln r): roughly 470 buckets give r near 1.048, leaving room in 2,048
bytes for an exact maximum, an exact threshold count and a total. The bucket holding
rank ceil(0.99 n) is exact; the reported value is within a factor r. Counters need
only 32 bits because the lifetime total is at most 10^9, below 2^32 = 4,294,967,296,
and the sum of four shards cannot exceed it.

Trace for DL-C2. 1,000 samples: 990 at 100 microseconds and 10 at 5,000 give a true
p99 of 100 (rank 990) and 10 above 2,000; 989 at 100 and 11 at 5,000 give a true p99
of 5,000 and 11. A bucketed estimate reports a value within the declared factor
(about 1.048) of the true one: 100 to 104.8 and 5,000 to 5,240. A
one-sample change moves the answer fiftyfold; the exact counts reproduce it and a
random sample of 256 does not.

Changed condition. For the 99.9th percentile the target rank is ceil(0.999 n);
at n = 10^9 the bucket is again exact and the value within r. A duplicated report
from shard A leaves the maximum unchanged (idempotent), adds shard A's threshold
count again (not idempotent), and moves the percentile toward A's distribution
because A's weight doubles. Without a (shard, sequence) identity a merge cannot
distinguish a retransmission from new data. Late deliveries tens of seconds late
(10^7 to 10^8 microseconds) remain inside the bucket range; a design with an
overflow bucket must report that the estimate is then a bound.

### Task T-DL-3

#### Release: initial

Each appointment carries a payload of up to 256 bytes taken from a region of 65,536
bytes that the service owns; no other allocation is allowed. A callback reads its
payload while it runs on the dispatcher thread. Other threads may request
cancellation at any time. A request reports one of two results: cancelled before the
callback started, in which case the payload may be reclaimed at once, or running, in
which case the payload stays valid until the callback finishes. Shutdown may be
requested at any time.

Requirements: no read of reclaimed memory, no leak once shutdown completes, bounded
time for every service call, payload addresses stable while any callback may read
them, and every acquisition point able to fail on demand without corrupting state.
Decide when each payload is reclaimed and by whom. Record the requirements you
derived, your selection, two rejected alternatives with the smallest schedule that
defeats each, and a predicted discriminating experiment before implementing.
Predict this schedule: one callback stalls indefinitely holding its payload; for
1,000 cycles another appointment is admitted and then cancelled before it starts.
How many admissions succeed?

#### Release: changed condition

Four dispatcher threads now run callbacks that may block for an unbounded time.
Adapt your design. Before running, predict the largest number of payload slots that
can be unusable at one time, how many cancel-and-admit cycles of the same kind
succeed, and what shutdown reports while all four callbacks are stalled.

#### Assessor material (not released)

Arithmetic. 65,536 / 256 = 256 payload slots. With one slot pinned by the stalled
callback, a policy that reclaims each payload when its own cancellation precedes the
callback admits all 1,000 cycles (one slot is reused each time). A policy that reclaims
only when no callback is running admits 255 and then fails the remaining 745, since
each cancelled payload is held back by the stall. Changed condition: at most four
slots are pinned under per-payload reclamation (252 usable, all cycles succeed); a
reclaim-when-idle policy fails after 252 cycles; shutdown reports four outstanding
callbacks and must not free their payloads. The learner may choose either policy for
the initial task; the criterion tests that the prediction follows from the choice and
that a rejected alternative is defeated by a schedule.

Cases. Cancel racing the start of the callback (both orders); double cancellation;
cancellation of a stale handle after the slot was reused; shutdown during a running
callback; failure injected at each acquisition point (admission, any internal table
growth, the cancellation bookkeeping) with the state compared before and after.
Run under ASan, UBSan and ThreadSanitizer; a clean run bounds only the schedules it
visited, so ask for the schedule enumeration or the model that covers the rest.
Seeded mutants: free on request; reclaim at the next idle point; final decrement
relaxed.

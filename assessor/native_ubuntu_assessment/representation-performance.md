```json
{
  "schema_version": 2,
  "kind": "assessor-packet",
  "packet": "representation-performance",
  "path": "representation-to-transfer",
  "route": "contextual-v2",
  "tasks": [
    {
      "id": "T-REP-1",
      "title": "Lookup service with an uncertain bottleneck",
      "role": "primary",
      "initial_follows": {
        "nodes": ["U-T15-01-04", "X-T15-002", "U-T14-02-04", "X-T14-017", "U-T43-05-03", "X-T43-001", "U-T43-09-01", "U-T43-09-02", "LAB-DO-02", "U-T43-03-03"],
        "episodes": ["EP-ordered-search", "EP-amortized-growth", "EP-locality-traffic", "EP-paired-measurement", "EP-pmu-counting", "EP-layout-columns", "EP-work-to-decision"]
      },
      "changed_follows": {
        "nodes": ["U-T43-03-03", "LAB-DO-02", "U-T14-02-04"],
        "episodes": ["EP-work-to-decision", "EP-layout-columns", "EP-amortized-growth"]
      },
      "also_required_nodes": ["LAB-MP-12"],
      "mechanisms": ["arrays-ordered-search", "stable-ordering"],
      "gap_mechanisms": [],
      "requires_selection": true,
      "min_rejected_alternatives": 2,
      "requires_revision_under_changed_assumption": true,
      "released_after_seal": true,
      "changed_assumption": "Records become mutable under a single writer, stable ID must survive relocation, most queries return one very large equal-key group and the byte budget falls to 16 MiB.",
      "canaries": [
        {"public": "lower_bound traces on [1,2,2,2,5] for keys 0, 2 and 6 and 31 alternating raw pairs (U-T15-01-01, U-T43-08-02)", "task": "key-then-ID order with tagged duplicates at the minimum and maximum keys, six record counts up to 262,144 and a record whose sizeof is 64"}
      ]
    },
    {
      "id": "T-REP-2",
      "title": "Page-resident ordered index with bounded memory",
      "role": "gap",
      "initial_follows": {
        "nodes": ["U-T43-05-01", "U-T43-05-03"],
        "episodes": ["EP-locality-traffic"]
      },
      "changed_follows": {
        "nodes": ["U-T43-05-03"],
        "episodes": ["EP-locality-traffic"]
      },
      "also_required_nodes": ["U-T14-06-04", "X-T14-016", "U-T47-01-01"],
      "also_required_episodes": ["EP-balanced-trees"],
      "mechanisms": ["ordered-trees-pages"],
      "gap_mechanisms": ["ordered-trees-pages"],
      "requires_selection": true,
      "min_rejected_alternatives": 2,
      "requires_revision_under_changed_assumption": true,
      "released_after_seal": true,
      "changed_assumption": "Keys become variable length up to 64 bytes and range queries rise to 80 percent of the load.",
      "canaries": [
        {"public": "seven-key search tree 4,2,6,1,3,5,7 and AVL replay 30,20,10 (U-T14-05-02, U-T14-06-01); two-block LRU write-back model (U-T43-05-01)", "task": "10^7 entries in 4,096-byte pages, a 256-page memory budget and crashes between page writes"}
      ]
    },
    {
      "id": "T-REP-3",
      "title": "Bounded object store under allocation failure",
      "role": "gap",
      "initial_follows": {
        "nodes": ["LAB-EX-01", "U-T14-02-04"],
        "episodes": ["EP-amortized-growth"]
      },
      "changed_follows": {
        "nodes": ["U-T14-02-04"],
        "episodes": ["EP-amortized-growth"]
      },
      "also_required_nodes": ["U-T16-03-03"],
      "also_required_episodes": ["EP-allocator-pools"],
      "mechanisms": ["allocation-pools"],
      "gap_mechanisms": ["allocation-pools"],
      "requires_selection": true,
      "min_rejected_alternatives": 2,
      "requires_revision_under_changed_assumption": true,
      "released_after_seal": true,
      "changed_assumption": "The region halves to 128 KiB and values may now grow in place up to 512 bytes.",
      "canaries": [
        {"public": "24-byte arena trace (3,1),(5,8),(10,4),(1,1) and a three-slot pool (U-T16-01-03, U-T16-03-01)", "task": "values of 8 to 512 bytes in a 256 KiB region, held addresses and failure injected at every internal acquisition"}
      ]
    },
    {
      "id": "T-REP-4",
      "title": "Index selection under a space budget",
      "role": "gap",
      "initial_follows": {
        "nodes": ["U-T43-03-02", "U-T43-03-03"],
        "episodes": ["EP-work-to-decision"]
      },
      "changed_follows": {
        "nodes": ["U-T43-03-03"],
        "episodes": ["EP-work-to-decision"]
      },
      "also_required_nodes": ["LAB-EX-05", "U-T15-06-02", "U-T15-06-03"],
      "also_required_episodes": ["EP-greedy-dp"],
      "mechanisms": ["dp-greedy"],
      "gap_mechanisms": ["dp-greedy"],
      "requires_selection": true,
      "min_rejected_alternatives": 2,
      "requires_revision_under_changed_assumption": true,
      "released_after_seal": true,
      "changed_assumption": "Savings stop being additive: candidates that serve the same query class contribute only their best saving.",
      "canaries": [
        {"public": "interval set [0,3),[1,2),[2,4),[3,5) and knapsack items (2,3),(3,4),(4,5) at capacity 5 (U-T15-06-02, U-T15-06-03)", "task": "items (4,7),(5,8),(3,6),(6,10) at capacity 9, candidate lists to 40 and capacity to 4,096"}
      ]
    },
    {
      "id": "T-REP-5",
      "title": "Latency statistics near the top of the range",
      "role": "gap",
      "initial_follows": {
        "nodes": ["U-T43-08-02", "X-T43-001"],
        "episodes": ["EP-paired-measurement"]
      },
      "changed_follows": {
        "nodes": ["X-T43-001"],
        "episodes": ["EP-paired-measurement"]
      },
      "also_required_nodes": ["U-T17-01-01", "U-T17-02-01", "X-T17-002"],
      "also_required_episodes": ["EP-numeric-accumulation"],
      "mechanisms": ["numerical-kernels"],
      "gap_mechanisms": ["numerical-kernels"],
      "requires_selection": true,
      "min_rejected_alternatives": 2,
      "requires_revision_under_changed_assumption": true,
      "released_after_seal": true,
      "changed_assumption": "A timeout sentinel is excluded from the mean but counted, and per-thread summaries merge pairwise in any order.",
      "canaries": [
        {"public": "cancellation triple [2^53,1,-2^53] and six Q7 product pairs (U-T17-01-01, U-T17-02-01)", "task": "32-bit latencies near 4,000,000,000 with spread 2, up to 2^32 samples"}
      ]
    }
  ],
  "criteria": [
    {
      "id": "REP-M1",
      "name": "Representation tied to every query and update",
      "dimensions": ["mechanism"],
      "tasks": ["T-REP-1"],
      "earns_2": "Connects the chosen representation to point lookup, closed range and batch update with the key-then-ID order and repeated keys defined, and states the invariant that makes each operation correct.",
      "earns_3": "Also gives a cost argument under explicit assumptions (probes for 262,144 records: 18 for a successful binary search and 19 in the worst case of a half-open lower bound, and 19 is not penalised; the amortized cost of a batch merge over a stated number of queries) and the workload mix at which the choice reverses.",
      "rejects": [
        "Choosing a hash-based index because 90 percent of queries are point lookups, ignoring range order.",
        "Treating a named representation as best without costing the 10 percent and 80 percent range shares.",
        "Charging an index build to the queries of one workload without stating how many queries amortize it."
      ]
    },
    {
      "id": "REP-C1",
      "name": "Independent oracle with equal semantics",
      "dimensions": ["correctness"],
      "tasks": ["T-REP-1"],
      "earns_2": "Checks every candidate against an independent sorted-sequence oracle on tagged duplicates at the minimum and maximum keys, missing keys adjacent to the bounds and an empty range, with randomized seeds recorded, and keeps consumer-visible results identical across candidates.",
      "earns_3": "Also rejects symmetric oracle defects: shows that an oracle sharing the implementation's comparator or search would pass the same bug, and uses a separately constructed oracle (filter then stable sort of a copy) to expose the assessor's mutants.",
      "rejects": [
        "An oracle that calls the implementation's own search.",
        "Dropping duplicates or tie order from the contract to make candidates agree."
      ]
    },
    {
      "id": "REP-O1",
      "name": "Readers, failed publication and stable identity",
      "dimensions": ["memory_ownership"],
      "tasks": ["T-REP-1"],
      "earns_2": "Keeps a retained reader valid while a replacement is built; a replacement whose allocation fails halfway publishes nothing and leaves the old snapshot intact.",
      "earns_3": "Also explains identity through relocation (a stable ID reaching its current position through a mapping that survives movement) and what a reader holding an old position observes.",
      "rejects": [
        "Using an array position as the stable ID.",
        "Publishing the new snapshot before it is complete.",
        "Freeing the old snapshot when the new one is published while a reader holds it."
      ]
    },
    {
      "id": "REP-R1",
      "name": "Complete byte accounting and failure atomicity",
      "dimensions": ["resources_dependencies"],
      "tasks": ["T-REP-1"],
      "earns_2": "Counts payload, index and every copied snapshot against 128 MiB using sizeof (the record is 60 bytes of fields but 64 bytes as a struct on x86-64 System V, so 262,144 records are 16 MiB and one copy 32 MiB).",
      "earns_3": "Also treats capacity and failure atomicity together at 16 MiB: shows that the padded records alone exhaust the budget, packs or splits the layout, and states the largest record count that still fits with the ID mapping.",
      "rejects": [
        "Counting only the live data and not the transient copy needed to publish.",
        "Estimating the record size as the sum of its fields."
      ]
    },
    {
      "id": "REP-D1",
      "name": "Observations qualified before interpretation",
      "dimensions": ["failure_diagnosis"],
      "tasks": ["T-REP-1"],
      "earns_2": "Keeps raw enabled and running metadata, classifies a zero count with positive running time, a zero-running group and a denied event as different states, and rejects both misleading runs (a candidate that omits payload consumption; a group that never schedules) before naming a winner.",
      "earns_3": "Also separates competing causes with a controlled experiment whose outcome is predicted first, for example a size sweep across the learner's measured cache boundaries, and checks that correctness checks sit outside the measured interval.",
      "rejects": [
        "Reading a zero count as the event not happening.",
        "Reporting a scaled count from a group with zero running time.",
        "Declaring a candidate faster when it never consumed the payload.",
        "Attributing a cache-miss difference to a cache level without evidence for the level."
      ]
    },
    {
      "id": "REP-T1",
      "name": "Independent adaptation and honest inconclusiveness",
      "dimensions": ["communication_transfer"],
      "tasks": ["T-REP-1", "T-REP-2", "T-REP-3"],
      "earns_2": "Adapts the design to the changed condition unaided, with the prediction sealed before running and the cost and ownership argument redone.",
      "earns_3": "Also defends a decision to keep the original design or an alternative with a rigorous argument, and reports an inconclusive measurement as inconclusive without inventing a universal ranking.",
      "rejects": [
        "Discarding the equal-key group's duplicates as an optimization.",
        "Declaring one representation faster in general from one machine and one workload.",
        "Restating the old argument unchanged under a contract that no longer supports it."
      ]
    },
    {
      "id": "REP-M2",
      "name": "Page arithmetic and its dependence on key size",
      "dimensions": ["mechanism", "resources_dependencies"],
      "tasks": ["T-REP-2"],
      "earns_2": "Derives pages and height for the stated sizes (256 entries per page, 39,063 leaf pages, 153 interior pages, 3 levels), compares them with about 16 page reads for a binary search over the file, and shows that the 256-page budget holds the top two levels.",
      "earns_3": "Also recomputes under 64-byte keys (60 per page, 166,667 leaves, 4 levels, about 1.9 page reads per lookup with the budget) and explains the effect on a 5,000-entry range (84 leaf pages against 20).",
      "rejects": [
        "Binary search over the file because it is logarithmic, ignoring the page reads.",
        "A hash-based index for a workload with range queries.",
        "Computing the height as log2 of the entry count."
      ]
    },
    {
      "id": "REP-O2",
      "name": "Atomic publication across crash points",
      "dimensions": ["memory_ownership", "failure_diagnosis"],
      "tasks": ["T-REP-2"],
      "earns_2": "Makes an update visible by a single step after all new pages are written and flushed, so a crash at any point leaves the old or the new version, and states which claims rest on POSIX flush semantics and which on file system and device behaviour.",
      "earns_3": "Also enumerates the crash points of a three-page update (before any write, after each of three page writes, before and after the switch) and predicts the recovered version at each before running, including a torn switch handled by a checksum or sequence number.",
      "rejects": [
        "Updating pages in place because a 4,096-byte write is atomic.",
        "Treating a successful write call as durable.",
        "Writing the switch before the flush of the pages it points to."
      ]
    },
    {
      "id": "REP-M3",
      "name": "Space, failure and reuse in a bounded store",
      "dimensions": ["mechanism", "memory_ownership", "resources_dependencies"],
      "tasks": ["T-REP-3"],
      "earns_2": "States the admissible set (count at most 4,096 and rounded sizes plus overhead at most the region, since 4,096 values of 512 bytes need 2 MiB against 256 KiB), keeps held addresses fixed, and leaves state unchanged when any internal acquisition fails.",
      "earns_3": "Also constructs the adversarial sequence that defeats the rejected policy where free bytes exceed the request (eight 512-byte values, four non-adjacent destroyed, 1,024 requested) and predicts which creations succeed under the chosen policy before running; for the halved region and in-place growth, shows why growth must fail or add indirection.",
      "rejects": [
        "Assuming a creation succeeds whenever free bytes are at least the request.",
        "Reserving 4,096 maximum-size slots.",
        "Testing failure only at the outer allocation call and not at each internal acquisition."
      ]
    },
    {
      "id": "REP-M4",
      "name": "Recurrence state, direction and a counterexample to greed",
      "dimensions": ["mechanism", "correctness", "communication_transfer"],
      "tasks": ["T-REP-4"],
      "earns_2": "States what each table cell means, proves that each candidate is used at most once under the loop direction chosen, and matches exhaustive enumeration of all 16 subsets of the assessor instance (best saving 16 with the 3 MiB and 6 MiB indexes).",
      "earns_3": "Also exhibits the counterexample to selecting by saving per MiB (it picks the 3 MiB and 4 MiB indexes for 13), shows the ascending one-row update reusing the 3 MiB index three times for 18, and re-derives the state when candidates overlap (best set becomes the 4 MiB and 5 MiB indexes for 15), saying where the general overlap problem leaves the polynomial table.",
      "rejects": [
        "Greedy by saving per unit of space.",
        "A one-row update iterated in ascending order, which lets an index be built repeatedly.",
        "Exhaustive enumeration for 40 candidates (about 1.1 x 10^12 subsets).",
        "Adding the savings of overlapping candidates."
      ]
    },
    {
      "id": "REP-N1",
      "name": "Exact quantities, rounded quantities and merging",
      "dimensions": ["correctness", "resources_dependencies", "failure_diagnosis"],
      "tasks": ["T-REP-5"],
      "earns_2": "Keeps the count in 64 bits (2^32 samples do not fit in 32), the sum exact in 64 bits (at most 2^32 x (2^32 - 1)), computes the spread with a shifted or running-update method and reproduces 2/3 for 4,000,000,000, 4,000,000,001, 4,000,000,002, naming which results are exact and which round.",
      "earns_3": "Also shows the textbook mean-of-squares formula failing on the same data (the squares near 1.6 x 10^19 need 64 bits, spaced 2,048 apart in a double), gives the pairwise merge formula, and states that floating-point merging is not associative so the merge order can change low-order bits while the integer sum cannot.",
      "rejects": [
        "Believing a double holds the squares of 32-bit values exactly because it holds the values exactly.",
        "Using the difference of the mean of squares and the square of the mean on clustered large values.",
        "Counting samples in a 32-bit integer."
      ]
    },
    {
      "id": "REP-N2",
      "name": "What the summary means and how it merges",
      "dimensions": ["mechanism", "communication_transfer"],
      "tasks": ["T-REP-5"],
      "earns_2": "States the invariant of the spread summary (the running sum of squared deviations from the running mean, with the count) and predicts before running, from that invariant, whether two merge orders agree and what the merge computes with the sentinel excluded.",
      "earns_3": "Also derives the merge from the invariant, including the between-summary term d^2 n_a n_b / n, and states which of count, sum, mean and spread are order-independent exactly and which only to rounding.",
      "rejects": [
        "Merging two summaries by averaging their means and their variances, which omits the between-summary term.",
        "Predicting identical results for any merge order because the mathematics is associative, without regard to rounding."
      ]
    }
  ],
  "worked_numbers": [
    {"id": "fields", "expr": "8+4+48", "expected": 60, "text": ["8 + 4 + 48 = 60"], "occurrences": 1},
    {"id": "struct-size", "expr": "-(-60//8)*8", "expected": 64, "text": ["64 bytes"], "occurrences": 4},
    {"id": "records-mib", "expr": "262144*64/2**20", "expected": 16.0, "text": ["262,144 records are 16 MiB"], "occurrences": 2},
    {"id": "copy-mib", "expr": "2*262144*64/2**20", "expected": 32.0, "text": ["32 MiB"], "occurrences": 2},
    {"id": "packed-mib", "expr": "262144*60/2**20", "expected": 15.0, "text": ["15 MiB"], "occurrences": 1},
    {"id": "idmap-mib", "expr": "262144*4/2**20", "expected": 1.0, "text": ["1 MiB"], "occurrences": 4},
    {"id": "probes-worst", "expr": "math.ceil(math.log2(262145))", "expected": 19, "text": ["ceil(log2(262,145)) = 19"], "occurrences": 1},
    {"id": "probes-success", "expr": "int(math.log2(262144))", "expected": 18, "text": ["18 probes"], "occurrences": 1},
    {"id": "entries-per-page", "expr": "4096//16", "expected": 256, "text": ["256 entries per page"], "occurrences": 2},
    {"id": "leaf-pages", "expr": "math.ceil(10**7/256)", "expected": 39063, "text": ["39,063"], "occurrences": 3},
    {"id": "interior-pages", "expr": "math.ceil(39063/256)", "expected": 153, "text": ["153 pages"], "occurrences": 1},
    {"id": "binary-pages", "expr": "math.log2(10**7)-8", "expected": 15.25, "tol": 0.001, "text": ["15.25"], "occurrences": 1},
    {"id": "range-leaves", "expr": "math.ceil(5000/256)", "expected": 20, "text": ["20 leaves"], "occurrences": 1},
    {"id": "var-per-page", "expr": "4096//68", "expected": 60, "text": ["60 per page"], "occurrences": 2},
    {"id": "var-leaves", "expr": "math.ceil(10**7/60)", "expected": 166667, "text": ["166,667"], "occurrences": 3},
    {"id": "var-level2", "expr": "math.ceil(166667/60)", "expected": 2778, "text": ["2,778"], "occurrences": 4},
    {"id": "var-level3", "expr": "math.ceil(2778/60)", "expected": 47, "text": ["the 47"], "occurrences": 1},
    {"id": "var-avg-reads", "expr": "1+(1-208/2778)", "expected": 1.93, "tol": 0.003, "text": ["1.93"], "occurrences": 1},
    {"id": "var-range-leaves", "expr": "math.ceil(5000/60)", "expected": 84, "text": ["84 leaves"], "occurrences": 1},
    {"id": "max-size-values", "expr": "256*1024//512", "expected": 512, "text": ["512 maximum-size"], "occurrences": 1},
    {"id": "max-size-values-halved", "expr": "128*1024//512", "expected": 256, "text": ["256 maximum-size"], "occurrences": 1},
    {"id": "dp-best", "expr": "max(7+8,7+6,8+6,6+10)", "expected": 16, "text": ["best is c+d with 16"], "occurrences": 1},
    {"id": "dp-greedy", "expr": "6+7", "expected": 13, "text": ["c then a for 13"], "occurrences": 1},
    {"id": "dp-unbounded", "expr": "3*6", "expected": 18, "text": ["18 at 9 MiB"], "occurrences": 1},
    {"id": "dp-overlap-best", "expr": "7+8", "expected": 15, "text": ["a+b with 15"], "occurrences": 1},
    {"id": "dp-subsets", "expr": "2**40/1e12", "expected": 1.0995511627776, "tol": 0.01, "text": ["1.1 x 10^12"], "occurrences": 2},
    {"id": "dp-cells", "expr": "40*4097", "expected": 163880, "text": ["163,880"], "occurrences": 1},
    {"id": "count-wraps", "expr": "2**32>2**32-1", "expected": true, "text": ["33-bit count"], "occurrences": 1},
    {"id": "sum-fits", "expr": "2**32*(2**32-1)<2**64", "expected": true, "text": ["below 2^64"], "occurrences": 2},
    {"id": "true-variance", "expr": "(1+0+1)/3", "expected": 0.6666666666666666, "tol": 1e-09, "text": ["2/3"], "occurrences": 2},
    {"id": "double-spacing", "expr": "math.ulp(1.6e19)", "expected": 2048, "text": ["2,048"], "occurrences": 3},
    {"id": "merge-formula", "expr": "2+50+13**2*3*2/5", "expected": 254.8, "tol": 1e-09, "text": ["254.8"], "occurrences": 1},
    {"id": "merge-direct", "expr": "(1-7.2)**2+(2-7.2)**2+(3-7.2)**2+(10-7.2)**2+(20-7.2)**2", "expected": 254.8, "tol": 1e-09, "text": ["mean 7.2"], "occurrences": 1}
  ],
  "leak_fingerprints": ["A store of 10,000,000 entries must answer a point lookup", "A store holds up to 4,096 values of 8 to 512 bytes", "4,000,000,000, 4,000,000,001 and 4,000,000,002"],
  "entry_check": {
    "accepted_artifacts": [
      {"artifact": "bytes.checked", "producer_node": "U-T04-01-04", "needed_for": ["T-REP-1", "T-REP-2", "T-REP-3", "T-REP-4", "T-REP-5"]},
      {"artifact": "bytes.cursor", "producer_node": "U-T03-02-03", "needed_for": ["T-REP-1"]},
      {"artifact": "measurement.capture", "producer_node": "U-T43-01-03", "needed_for": ["T-REP-1", "T-REP-5"]},
      {"artifact": "containers.array", "producer_node": "LAB-EX-01", "needed_for": ["T-REP-1", "T-REP-3"]},
      {"artifact": "meta.reflect", "producer_node": "LAB-MP-12", "needed_for": ["T-REP-1"]},
      {"artifact": "memory.pool", "producer_node": "U-T16-03-03", "needed_for": ["T-REP-3"]},
      {"artifact": "algorithms.dp", "producer_node": "LAB-EX-05", "needed_for": ["T-REP-4"]}
    ],
    "source_identities": [
      "SHA-256 of curriculum/v5/routes/contextual-v2.json and the repository commit",
      "SHA-256 of every learner source file and of each imported bundle",
      "the learner's PMU protocol source (labs/performance_followthrough/pmu.c) and the build record naming any TEST IMPLEMENTATION label",
      "compiler, flags, sanitizer, python3 versions; clock identity and resolution; perf_event_paranoid and whether any counting group opened; CPU model and cache sizes read from the machine"
    ],
    "tools": ["C17 compiler with sanitizers", "python3", "perf counting access (optional, recorded either way)", "perf_event_paranoid value (recorded, never changed)"],
    "probes": [
      {"id": "P-REP-1", "ask": "Classify a zero count with positive running time, a group whose running time is zero and a denied event, before any hardware experiment.", "tests_node": "U-T43-09-01", "remediation": "Redo the PMU protocol classification (PMU_PROTOCOL.md) before T-REP-1 measurement."},
      {"id": "P-REP-2", "ask": "State the half-open lower-bound invariant and the decreasing measure that proves termination.", "tests_node": "U-T15-01-01", "remediation": "Redo ordered search with the invariant and termination proof."},
      {"id": "P-REP-3", "ask": "A growth step fails halfway. What state is the container in and how do you know?", "tests_node": "U-T14-02-04", "remediation": "Redo failure-preserving growth with the history oracle."},
      {"id": "P-REP-4", "ask": "What is reported apart from the median difference of paired trials, and why does a consumed checksum matter?", "tests_node": "X-T43-001", "remediation": "Redo paired trial reporting."},
      {"id": "P-REP-5", "ask": "In your block-transfer ledger, when is a ratio undefined and what does a write-back add?", "tests_node": "U-T43-05-01", "remediation": "Redo the two-block model and traffic ledger before T-REP-2."},
      {"id": "P-REP-6", "ask": "What does a foreign owner or a stale generation do in your pool, and what fails at a given acquisition number?", "tests_node": "U-T16-03-03", "remediation": "Redo the allocation episode (EP-allocator-pools) before T-REP-3."},
      {"id": "P-REP-7", "ask": "Why does loop direction matter in a one-row table for items used at most once?", "tests_node": "U-T15-06-03", "remediation": "Redo the dynamic-programming episode (EP-greedy-dp) before T-REP-4."},
      {"id": "P-REP-8", "ask": "Which intermediate in a sum of products can leave the range before the final result does?", "tests_node": "U-T17-01-01", "remediation": "Redo the numeric accumulation episode (EP-numeric-accumulation) before T-REP-5."}
    ],
    "known_unstated": [
      "EP-layout-columns enters with containers.array and measurement.capture, but its nodes (X-T70-001, LAB-DO-01, LAB-DO-02) also list meta.reflect (LAB-MP-12) as a component prerequisite that no authored episode of this path produces. A learner without it cannot have completed the node T-REP-1's layout comparison builds on; a failed artifact check on it is a route gap.",
      "T-REP-2 to T-REP-5 need work from episodes outside this path (EP-balanced-trees and the page codec U-T47-01-01, EP-allocator-pools, EP-greedy-dp, EP-numeric-accumulation). The route review lists no independent transfer for these mechanisms. They are released only when those nodes are accepted in the learner's profile.",
      "LAB-EX-05 implements 0/1 knapsack but is catalogued under greedy exchange (T15.09.a) and is a prerequisite of both units that derive the recurrence, so the learner may have seen an implementation before the derivation; check the learner's algorithms.dp source identity against the shipped lab file."
    ]
  },
  "imitation_probes": [
    {"check": "IM-VERBATIM", "applies_to": ["T-REP-1", "T-REP-2", "T-REP-3", "T-REP-4", "T-REP-5"], "compare_with": ["curriculum/v5/routes/v2/records.md", "labs/containers_depth/", "labs/performance_followthrough/PMU_PROTOCOL.md", "labs/algorithm_depth/"], "probe": "Look for the public example values in the learner's traces (the five-element array, the 3-item knapsack, the 31-pair protocol) and ask the learner to rerun their table on a changed capacity by hand before running it."},
    {"check": "IM-ADAPTER", "applies_to": ["T-REP-1", "T-REP-2", "T-REP-3", "T-REP-4", "T-REP-5"], "compare_with": ["assessor/performance_followthrough/", "assessor/containers_depth/", "labs/support/"], "probe": "Hash the learner's sources, including the PMU protocol file and any imported container, against every assessor C/H/S file and the shipped lab files; a TEST IMPLEMENTATION run is an author run."},
    {"check": "IM-REJECTED", "applies_to": ["T-REP-1", "T-REP-2", "T-REP-3", "T-REP-4", "T-REP-5"], "compare_with": [], "probe": "For each rejected alternative ask for the smallest instance on which it fails (a 5-record array, a 4,096-byte region, the 4-item table) and run the assessor's model of it there."},
    {"check": "IM-PREDICT", "applies_to": ["T-REP-1", "T-REP-2", "T-REP-3", "T-REP-4", "T-REP-5"], "compare_with": [], "probe": "Compare the sealed prediction with the worked answers below, then mutate one learner line offline (comparator tie, loop direction, flush order) and ask which case fails first."}
  ]
}
```

# Withheld packet: representation choice to independent transfer

Path `representation-to-transfer` in `contextual-v2`. This packet follows the
episodes EP-ordered-search, EP-amortized-growth, EP-locality-traffic,
EP-paired-measurement, EP-pmu-counting, EP-layout-columns and EP-work-to-decision,
and the nodes in each task's manifest entry. The route's bound transfers for the
mechanisms below mostly name the structure they assess, and none exists for
allocation, trees and pages, summaries, dynamic programming, numerics or
reclamation. T-REP-1 is primary; T-REP-2 to T-REP-5 each cover one of those
mechanisms in the setting of the lookup service. Have the learner classify a
zero count with positive running time, a zero-running group and a denied event
before any hardware experiment. Record the actual hardware-access state; lack of
access does not waive the protocol gate.

### Task T-REP-1

#### Release: initial

A service holds immutable records with a 64-bit key, a stable 32-bit ID and 48 bytes
of payload. Queries request either the first matching ID or all IDs in a closed
key range, preserving increasing key then ID order. Keys may repeat. Updates
arrive in batches while existing readers retain their old snapshot. Workload A
is 90 percent unsuccessful point lookup, 10 percent short range queries; workload B
is 20 percent point lookup, 80 percent long ranges. Storage is bounded to 128 MiB
including any copied snapshots and indexes.

Select a representation and define its ownership and snapshot boundary. Record
the requirements you derived, two rejected alternatives with the smallest instance
that defeats each, two competing bottleneck hypotheses and a predicted
discriminating size or workload change, then inspect generated code and measure.
Preserve identical query results and consumer-visible semantics. An index build may
be amortized only over an explicitly stated number of queries; report construction
separately. Do not assume any named representation or hardware event is available.

#### Release: changed condition

After the delay, records become mutable under a single writer, the stable ID must
survive relocation, and most queries return one very large group of equal keys.
The byte budget is reduced to 16 MiB. Provide a new cost and ownership argument and
either an alternative representation or a defended decision to retain the old one.
Before running, predict the largest record count your design supports in 16 MiB
and the work to return the first matching ID for a key with 100,000 equal-key
records.

#### Assessor material (not released)

Use tagged duplicates at minimum and maximum keys, missing keys adjacent to the
bounds and an empty range. Retain a reader while publishing a replacement whose
allocation fails halfway. Old readers must keep valid data and no partial new
snapshot may publish. Sweep record counts 1, 16, 256, 4096, 65536 and 262144 if the
measured storage bound admits them, plus actual cache-boundary regimes chosen by
the learner. Keep randomized query seeds and an independent sorted-sequence oracle.

Introduce an intentionally misleading run where one candidate omits payload
consumption, and a second run where the hardware group never schedules. The
learner must reject these observations before interpreting a performance winner.
Observe whether correctness checks pollute the declared measured interval.

Worked storage. The fields total 8 + 4 + 48 = 60 bytes; with an 8-byte-aligned key
(x86-64 System V; confirm with sizeof) the struct is 64 bytes. 262,144 records are 16 MiB, a full copy for publication 32
MiB, and an 8-byte-key plus 4-byte-ID index entry padded to 16 bytes adds 4 MiB:
all inside 128 MiB. In 16 MiB the padded records alone fill the budget and nothing
remains for an ID mapping; packing to 60 bytes gives 15 MiB, and a 4-byte ID-to-position
map for 262,144 IDs is exactly the remaining 1 MiB. A design that must stay inside 16
MiB with any spare therefore supports fewer than 262,144 records; the largest count
is (16 MiB - fixed overhead) / (per-record bytes including the ID mapping). Binary
search over 262,144 records takes 18 probes when successful and at most 19 for a half-open
lower bound (ceil(log2(262,145)) = 19); accept either with its derivation. A changed contract that simply discards
duplicates is not an optimization. For 100,000 equal keys, ID order within the
group means the first match after locating the group start is the first ID, and a
design that keeps equal keys in arrival order must scan the group or keep a minimum.

Seeded mutants: comparator on key only; upper bound where lower bound is required;
publish before build completes; free old snapshot on publish.

### Task T-REP-2

#### Release: initial

A store of 10,000,000 entries must answer a point lookup and a closed key range
and accept batches of updates. It lives in a file of 4,096-byte pages, and at most
1 MiB of memory may hold pages at any time. An entry is a 64-bit key and a 32-bit
ID and occupies 16 bytes in a page; wherever a page refers to another page it stores a
32-bit page number, and an entry that carries a key and such a number occupies 16 bytes.
Ignore page headers for the estimates, but
state what your design adds. A crash may happen between any two page writes; after
recovery a reader must see exactly the version before an update or the version
after it, never a mixture. Readers may continue on the old version during an update.

Select a layout and an update procedure. Record the requirements you derived, two
rejected alternatives with the smallest failing instance, and a predicted
discriminating experiment. Before running, predict the number of pages read by a
point lookup and by a range of 5,000 entries, and the version recovered after a
crash at each point of an update that rewrites three pages.

#### Release: changed condition

Keys become variable length up to 64 bytes (an entry is then at most 68 bytes) and
range queries are now 80 percent of the load. Adapt your design. Before running,
predict the new height, the pages read by a point lookup with the same 1 MiB and
the pages read by a range of 5,000 entries.

#### Assessor material (not released)

Arithmetic. 4,096 / 16 = 256 entries per page; 10^7 / 256 = 39,063 leaf pages; one
interior level has ceil(39,063 / 256) = 153 pages; a root above it: 3 levels. 1 MiB
is 256 pages, holding the root and the 153 interior pages (154) and 102 leaves.
A point lookup then reads one page (the leaf). A binary search over the file reads
about log2(10^7) - 8 = 15.25, so 16, pages since the last 8 probes fall in one page.
A range of 5,000 entries reads ceil(5,000 / 256) = 20 leaves (21 if unaligned).
Variable keys: 4,096 / 68 = 60 per page; ceil(10^7 / 60) = 166,667 leaves; ceil(166,667
/ 60) = 2,778; ceil(2,778 / 60) = 47; a root: 4 levels. The 256-page budget holds
the root, the 47 and 208 of the 2,778, so a uniform lookup reads 1 + (1 - 208/2,778)
= 1.93 pages on average. A 5,000-entry range reads ceil(5,000 / 60) = 84 leaves.

Crash points for a path-copy update of a leaf, its interior page and the root:
0, 1, 2 or 3 new pages written, then the switch (an alternating metadata page that
carries a sequence number and checksum). A crash before the switch is durable
recovers the old version at every point; after the switch is durable, the new one;
a torn switch write is rejected by the checksum and the other slot wins. The flush
that makes the pages durable must precede the switch write and another must follow it
before the update is acknowledged. ISO C says nothing about durability; POSIX
specifies fsync and fdatasync; what survives a power loss also depends on the file
system and the device cache, which the learner must name rather than assume.

Cases: duplicate keys across a leaf boundary; a range that starts in a gap between
leaves; an update that splits the root; a crash mid-split.

### Task T-REP-3

#### Release: initial

A store holds up to 4,096 values of 8 to 512 bytes in a single region of 256 KiB
that the store owns; it may not obtain memory any other way. Values are created and
destroyed in no fixed order. A reader may hold a value's address until it releases
it, so a held value never moves. Creation can be made to fail at any acquisition
point, including internal bookkeeping, and a failed creation changes nothing.

Decide how space is found and returned and what the store promises about when a
creation of a given size succeeds. Record the requirements you derived, two rejected alternatives with the
smallest failing sequence, and a predicted discriminating experiment. Before
running, predict the outcome on a 4,096-byte model region (count only value bytes for
this model; say where your bookkeeping lives for the real region): eight 512-byte values
are created, values 2, 4, 6 and 8 are destroyed, and 1,024 bytes are requested;
then value 3 is destroyed and 1,024 bytes are requested again.

#### Release: changed condition

The region halves to 128 KiB and values may grow in place up to 512 bytes while a
reader holds the address. Adapt your design. Before running, predict what a growth
request does when the following block is in use, and the number of maximum-size
values that fit.

#### Assessor material (not released)

The model trace counts value bytes only. If the learner keeps bookkeeping inside the
region, eight 512-byte values plus their headers no longer fit in 4,096 bytes; accept
the learner's own stated overhead and trace if it is consistent, and score the
prediction against that trace, not against the one below.

Arithmetic. 4,096 values of 512 bytes need 2 MiB against 256 KiB, so 4,096 is a bound
on count only; at most 512 maximum-size values fit before overhead, and 4,096
minimum-size values need 32 KiB plus per-value overhead. Model trace: eight 512-byte
values fill 4,096 bytes; destroying 2, 4, 6, 8 frees 2,048 bytes in four runs of 512.
The first request for 1,024 fails under any policy that places a value in one
contiguous run (free bytes are not the criterion); after value 3 is destroyed the
run 2 to 4 is 1,536 bytes, so the second request succeeds if adjacent free runs
merge or the search finds runs, and fails for a fixed-block design whose largest
value is 512. Changed condition: 128 KiB holds 256 maximum-size values; growth in
place succeeds only if the following run is free and large enough, otherwise it
must fail or the design must add an indirection that breaks the held-address
contract.

Cases: fail-at-N sweep over every acquisition; double destruction; destroy while
held (the learner must say what is promised); fill to the count limit with
minimum-size values; fill to the byte limit with maximum-size values. Seeded
mutants: free bytes treated as sufficient; lost coalescing; count limit checked
after mutation.

### Task T-REP-4

#### Release: initial

The lookup service can build secondary indexes. Each candidate has a size in whole MiB
and a measured saving in milliseconds per 1,000 queries; savings add when several
are built. Total space for indexes is 9 MiB. The candidates are: a, 4 MiB and 7 ms;
b, 5 MiB and 8 ms; c, 3 MiB and 6 ms; d, 6 MiB and 10 ms. Choose the set with the
greatest total saving and show how you know no set is better. Your method must also
serve a list of up to 40 candidates and a space of up to 4,096 MiB. Record the
requirements you derived, two rejected alternatives with the smallest failing
instance, and a predicted discriminating experiment before implementing.

#### Release: changed condition

Savings stop being additive: candidates c and d serve the same query class, so a
set that contains both saves only the larger of their savings on that class and
the others add as before. Adapt your method. Before running, predict the best set
and its saving, and state what your method must now keep track of that it did not before.

#### Assessor material (not released)

Worked answer. Of the 16 subsets, those within 9 MiB: a+b = 9 MiB for 15; a+c = 7 for
13; b+c = 8 for 14; c+d = 9 for 16; d alone 10; the best is c+d with 16. Selecting by
saving per MiB (c 2.00, a 1.75, d 1.67, b 1.60) takes c then a for 13. A one-row
update in ascending capacity order allows c to be used three times: 18 at 9 MiB, an
impossible plan. Descending order gives 16. Exhaustive enumeration over 40 candidates
is 2^40, about 1.1 x 10^12 subsets; a table of 40 x 4,097 cells is 163,880.
Changed condition: c+d now saves max(6, 10) = 10 for 9 MiB, so a+b with 15 is best
(b+c is 14); the optimum moves. Candidates in disjoint overlap groups form a group
choice problem (choose at most one representative per group, or both for the
maximum); arbitrary overlaps are a budgeted coverage-type problem that is NP-hard
in general, so a polynomial table cannot be claimed. Score whether the learner says
what structure the supplied overlap has before reusing the table.

Seeded mutants: ties (equal saving) broken by smaller space; capacity off by one;
zero-size candidate; saving overflow at 40 candidates of 2^31.

### Task T-REP-5

#### Release: initial

The service reports the count, the exact sum, the mean and the population standard
deviation of query latencies. A latency is a 32-bit unsigned count of nanoseconds.
A run can have up to 2^32 queries across eight threads whose summaries are merged
at the end. Each thread uses constant memory. The compiler provides 64-bit integers
and IEEE-754 binary64 doubles; assume no wider arithmetic. Latencies may cluster
within a few nanoseconds of 4,000,000,000.

Decide how to represent count, sum and spread, which results are exact and which
round, and what is lost when values cluster. Record the requirements you derived,
two rejected alternatives with the smallest failing input, and a predicted
discriminating experiment. Before running, predict the reported variance of
4,000,000,000, 4,000,000,001 and 4,000,000,002 under your method and under the
textbook mean of squares minus the square of the mean.

#### Release: changed condition

A timeout sentinel, 4,294,967,295, must be counted but excluded from the mean and
spread, and the eight summaries merge pairwise in any order. Adapt your design.
Before running, predict whether two different merge orders give identical results
and what the merge computes.

#### Assessor material (not released)

Arithmetic. 2^32 samples need a 33-bit count; a 32-bit count wraps to zero. The sum is
at most 2^32 x (2^32 - 1), below 2^64, so a 64-bit sum is exact. (2^32 - 1)^2 is
below 2^64 but the sum of two squares is not, so a sum of squares needs about 96
bits and does not fit in 64. Three values 4 x 10^9 + {0, 1, 2}: mean 4 x 10^9 + 1,
population variance 2/3. A running update (Welford) gives 0.6667. The textbook formula
in binary64 yields 2048.0 on this input: each square is about 1.6 x 10^19, near
2^63.8, so doubles near it are spaced 2^11 = 2,048 apart, and the difference of two
rounded quantities is a multiple of that spacing. Merge of summaries (n, mean, M2):
n = n_a + n_b; d = mean_b - mean_a; mean = mean_a + d n_b / n; M2 = M2_a + M2_b + d^2 n_a
n_b / n. Check: the summaries of 1, 2, 3 (n = 3, mean 2, M2 = 2) and of 10, 20 (n = 2,
mean 15, M2 = 50) merge to M2 = 2 + 50 + 13^2 x 3 x 2 / 5 = 254.8, which equals the
sum of squared deviations of all five values from their mean 7.2. Floating-point merging is not associative, so different orders can differ in
low-order bits; the integer sum and count are exact in any order. The sentinel adds a
count without a contribution to sum or M2.

Seeded mutants: 32-bit count; naive formula; sentinel included; merge missing the
cross term.

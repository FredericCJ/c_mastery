# Mechanism-specific cost and enforcement review

Complete measured columns with compiler flags, architecture, workload and equivalent correctness contracts. The following are analytical expectations, not invented measurements.

## PR-T76-01: Positioned event logs and persistent snapshot roots

Inline16events and17roots plus the imported65node arena; append clones3nodes, lookup has bounded trie depth, replay O(events). Retained roots pin all prior published nodes; no allocator or reclamation occurs. Publication and persistence costs are distinct.

- **representation:** List exact fields, sizeof/alignment/padding observations and the abstraction function; distinguish ISO constraints from ABI observations.
- **reachable storage:** Draw pointers/indices and the retained backing extent; include storage kept alive by a borrow even when owned bytes are zero.
- **lifetime:** State creation, invalidation and final permitted access for each representation; construct a counterexample to any automatic-lifetime claim.
- **mutation and aliasing:** Identify all writable fields and permitted aliases; distinguish disjoint API preconditions from constraints actually diagnosed by C.
- **allocation:** Count allocation requests including prerequisite adapters; bounded inline storage is zero dynamic allocation, not zero resource use.
- **movement and copying:** Count copied payload bytes and metadata moves; show whether a copy changes ownership, identity or borrow validity.
- **destruction:** Account for every resource on success and failure; a borrowed/trivial value has no owned destructor but still has an invalidation point.
- **dispatch:** Identify direct branches, function pointers or generated specializations; inspect release assembly before claiming dispatch elimination.
- **concurrency:** The supplied operations are single-threaded; prove absence of shared mutation or provide synchronization and reclamation for the required transfer. No TSan pass is inferred.
- **failure:** List each admitted failure and exact post-state; assess a boundary counterexample and distinguish caller-contract violations from operational failure.
- **compilation:** Record preprocessing/compilation time and diagnostic controls; generated source size and type-checking are distinct from runtime safety.
- **binary and stack:** Record object text/data, compiler stack-use report and call depth; compare under identical semantics, not a weaker unchecked implementation.
- **boundary:** Name the language-level counterpart, identify ISO/diagnostic/runtime/external-verification/caller enforcers and falsify one overstated equivalence.

Specific falsification and transfer: Run replicated.log and the three allocation-cut assertions, compare every prefix against an independent key/value dictionary and rebuild into a fresh arena. Reject count-before-commit and latest-root-for-all-snapshots mutants. Inspect source/link dependencies to establish that the real imported HAMT is used. Separately implement the required codec.records/storage.pages adapter with short writes and crash schedules; the memory anchor does not validate that adapter. Replace the in-memory append with versioned checksummed records on the prior storage.pages artifact; enumerate every tear/flush prefix and recover the longest committed prefix. Persist snapshots with their event position and schema identity. Demonstrate why replaying an external effect twice or using a changed external query result changes semantics; retain original responses as events where required.

## PR-T76-02: CQRS projection admission and atomic position advancement

Sixteen semantic event copies plus count and int64 total; O(1) admission, retained history bounded to16, no allocation. Mutex contention and total adapter latency must be measured separately from pure admission. Durable inbox/outbox costs are additional.

- **representation:** List exact fields, sizeof/alignment/padding observations and the abstraction function; distinguish ISO constraints from ABI observations.
- **reachable storage:** Draw pointers/indices and the retained backing extent; include storage kept alive by a borrow even when owned bytes are zero.
- **lifetime:** State creation, invalidation and final permitted access for each representation; construct a counterexample to any automatic-lifetime claim.
- **mutation and aliasing:** Identify all writable fields and permitted aliases; distinguish disjoint API preconditions from constraints actually diagnosed by C.
- **allocation:** Count allocation requests including prerequisite adapters; bounded inline storage is zero dynamic allocation, not zero resource use.
- **movement and copying:** Count copied payload bytes and metadata moves; show whether a copy changes ownership, identity or borrow validity.
- **destruction:** Account for every resource on success and failure; a borrowed/trivial value has no owned destructor but still has an invalidation point.
- **dispatch:** Identify direct branches, function pointers or generated specializations; inspect release assembly before claiming dispatch elimination.
- **concurrency:** The supplied operations are single-threaded; prove absence of shared mutation or provide synchronization and reclamation for the required transfer. No TSan pass is inferred.
- **failure:** List each admitted failure and exact post-state; assess a boundary counterexample and distinguish caller-contract violations from operational failure.
- **compilation:** Record preprocessing/compilation time and diagnostic controls; generated source size and type-checking are distinct from runtime safety.
- **binary and stack:** Record object text/data, compiler stack-use report and call depth; compare under identical semantics, not a weaker unchecked implementation.
- **boundary:** Name the language-level counterpart, identify ISO/diagnostic/runtime/external-verification/caller enforcers and falsify one overstated equivalence.

Specific falsification and transfer: Run replicated.projection, including actual pthread schedules under GCC TSan when available. Kill duplicate-reapplication and gap-acceptance mutants with semantic assertions. Record mutex boundaries and model the crash point between query-state and cursor persistence. A TSan pass observes these executions only; it does not certify arbitrary effects or all schedules. Add a durable inbox/outbox with log UUID and idempotency keys using storage.pages and ipc.local. Enumerate crash/retry points on both sides of a local synthetic receiver. Produce one duplicate-side-effect counterexample without receiver deduplication and show the residual requirement for atomic receiver state. Compare rebuildable read models with authoritative event history.

## PR-T76-03: Lamport, vector and hybrid logical clock boundaries

Lamport8bytes, vector3uint64 values, HLCuint64+uint32 plus ABI padding; no dynamic storage. Updates O(1) for fixed membership, O(replicas) generally. Persisted counters and clock reads are separate costs; the C core performs no system clock calls.

- **representation:** List exact fields, sizeof/alignment/padding observations and the abstraction function; distinguish ISO constraints from ABI observations.
- **reachable storage:** Draw pointers/indices and the retained backing extent; include storage kept alive by a borrow even when owned bytes are zero.
- **lifetime:** State creation, invalidation and final permitted access for each representation; construct a counterexample to any automatic-lifetime claim.
- **mutation and aliasing:** Identify all writable fields and permitted aliases; distinguish disjoint API preconditions from constraints actually diagnosed by C.
- **allocation:** Count allocation requests including prerequisite adapters; bounded inline storage is zero dynamic allocation, not zero resource use.
- **movement and copying:** Count copied payload bytes and metadata moves; show whether a copy changes ownership, identity or borrow validity.
- **destruction:** Account for every resource on success and failure; a borrowed/trivial value has no owned destructor but still has an invalidation point.
- **dispatch:** Identify direct branches, function pointers or generated specializations; inspect release assembly before claiming dispatch elimination.
- **concurrency:** The supplied operations are single-threaded; prove absence of shared mutation or provide synchronization and reclamation for the required transfer. No TSan pass is inferred.
- **failure:** List each admitted failure and exact post-state; assess a boundary counterexample and distinguish caller-contract violations from operational failure.
- **compilation:** Record preprocessing/compilation time and diagnostic controls; generated source size and type-checking are distinct from runtime safety.
- **binary and stack:** Record object text/data, compiler stack-use report and call depth; compare under identical semantics, not a weaker unchecked implementation.
- **boundary:** Name the language-level counterpart, identify ISO/diagnostic/runtime/external-verification/caller enforcers and falsify one overstated equivalence.

Specific falsification and transfer: Run replicated.clocks and independent exhaustive small HLC branch enumeration plus uint64/uint32 extrema. Kill ignoring-remote Lamport and incorrect equal-physical HLC-counter mutants. Build small message histories and derive causal relations from graph reachability before comparing clock outputs; timestamp ordering alone is not the oracle. Change replica membership and persisted restart identities; show why resetting a vector slot or reusing a timestamp identity loses history. Add a bounded clock-skew model and prove exactly which HLC closeness bound follows. Compare scalar timestamp wire size, vector growth, and a protocol that waits versus rejects on finite counter exhaustion.

## PR-T76-04: Join-semilattice counters and deterministic registers

Sixuint32 components and a timestamp/replica/value register; fixed O(3) merge, no allocation/drop. State traffic grows with membership. Logical deletion cannot shrink history without a separately proven protocol.

- **representation:** List exact fields, sizeof/alignment/padding observations and the abstraction function; distinguish ISO constraints from ABI observations.
- **reachable storage:** Draw pointers/indices and the retained backing extent; include storage kept alive by a borrow even when owned bytes are zero.
- **lifetime:** State creation, invalidation and final permitted access for each representation; construct a counterexample to any automatic-lifetime claim.
- **mutation and aliasing:** Identify all writable fields and permitted aliases; distinguish disjoint API preconditions from constraints actually diagnosed by C.
- **allocation:** Count allocation requests including prerequisite adapters; bounded inline storage is zero dynamic allocation, not zero resource use.
- **movement and copying:** Count copied payload bytes and metadata moves; show whether a copy changes ownership, identity or borrow validity.
- **destruction:** Account for every resource on success and failure; a borrowed/trivial value has no owned destructor but still has an invalidation point.
- **dispatch:** Identify direct branches, function pointers or generated specializations; inspect release assembly before claiming dispatch elimination.
- **concurrency:** The supplied operations are single-threaded; prove absence of shared mutation or provide synchronization and reclamation for the required transfer. No TSan pass is inferred.
- **failure:** List each admitted failure and exact post-state; assess a boundary counterexample and distinguish caller-contract violations from operational failure.
- **compilation:** Record preprocessing/compilation time and diagnostic controls; generated source size and type-checking are distinct from runtime safety.
- **binary and stack:** Record object text/data, compiler stack-use report and call depth; compare under identical semantics, not a weaker unchecked implementation.
- **boundary:** Name the language-level counterpart, identify ISO/diagnostic/runtime/external-verification/caller enforcers and falsify one overstated equivalence.

Specific falsification and transfer: Run replicated.counters; enumerate small component states and three-way merge permutations, including duplicate, reordered and dropped-then-retransmitted messages. Kill additive-state-merge and missing replica tie-break mutants. Show a counterexample for unchecked uint32 wrap and for independent writers sharing one replica slot. Add membership replacement, serialized snapshots and state corruption detection. Distinguish crash fault tolerance from Byzantine tolerance: demonstrate a synthetic forged high component monopolizing the merge and specify an authenticated admission/identity policy without claiming signatures solve semantic authorization. Compare state size with operation-log retention under the same delivery assumptions.

## PR-T76-05: Observed-remove sets and replayable delta state

Eightuint16 tag masks plus2counters; O(4) merge, O(1) local operation, no allocation. At most16 add identities lifetime; removed tags remain stored. Wire and acknowledgment costs differ from local bitset cost.

- **representation:** List exact fields, sizeof/alignment/padding observations and the abstraction function; distinguish ISO constraints from ABI observations.
- **reachable storage:** Draw pointers/indices and the retained backing extent; include storage kept alive by a borrow even when owned bytes are zero.
- **lifetime:** State creation, invalidation and final permitted access for each representation; construct a counterexample to any automatic-lifetime claim.
- **mutation and aliasing:** Identify all writable fields and permitted aliases; distinguish disjoint API preconditions from constraints actually diagnosed by C.
- **allocation:** Count allocation requests including prerequisite adapters; bounded inline storage is zero dynamic allocation, not zero resource use.
- **movement and copying:** Count copied payload bytes and metadata moves; show whether a copy changes ownership, identity or borrow validity.
- **destruction:** Account for every resource on success and failure; a borrowed/trivial value has no owned destructor but still has an invalidation point.
- **dispatch:** Identify direct branches, function pointers or generated specializations; inspect release assembly before claiming dispatch elimination.
- **concurrency:** The supplied operations are single-threaded; prove absence of shared mutation or provide synchronization and reclamation for the required transfer. No TSan pass is inferred.
- **failure:** List each admitted failure and exact post-state; assess a boundary counterexample and distinguish caller-contract violations from operational failure.
- **compilation:** Record preprocessing/compilation time and diagnostic controls; generated source size and type-checking are distinct from runtime safety.
- **binary and stack:** Record object text/data, compiler stack-use report and call depth; compare under identical semantics, not a weaker unchecked implementation.
- **boundary:** Name the language-level counterpart, identify ISO/diagnostic/runtime/external-verification/caller enforcers and falsify one overstated equivalence.

Specific falsification and transfer: Run replicated.sets and enumerate two-writer add/remove/delivery traces against a mathematical tagged-set oracle. Include duplicate and remove-before-add delivery. Kill remove-all-possible-tags and omitted-tombstone-merge mutants. Inventory retained tags and show the exact full condition rather than silently dropping history. Introduce a third replica, disconnection and crash/restart. Design a causal delta-interval anti-entropy protocol with durable incarnation/counter identity and full-state fallback; falsify premature garbage collection with a delayed old delta. Prove a stable frontier before discarding tombstones, or retain them and quantify growth.

## PR-T76-06: Replicated sequence identities and contextual transformation

Sixteen inline nodes; insertion O(n), simple rendering O(n squared) and recursion depth<=16; no heap or reclamation. Tombstones retain anchor topology. Production balancing, Unicode and compacted histories require new contracts and measurements.

- **representation:** List exact fields, sizeof/alignment/padding observations and the abstraction function; distinguish ISO constraints from ABI observations.
- **reachable storage:** Draw pointers/indices and the retained backing extent; include storage kept alive by a borrow even when owned bytes are zero.
- **lifetime:** State creation, invalidation and final permitted access for each representation; construct a counterexample to any automatic-lifetime claim.
- **mutation and aliasing:** Identify all writable fields and permitted aliases; distinguish disjoint API preconditions from constraints actually diagnosed by C.
- **allocation:** Count allocation requests including prerequisite adapters; bounded inline storage is zero dynamic allocation, not zero resource use.
- **movement and copying:** Count copied payload bytes and metadata moves; show whether a copy changes ownership, identity or borrow validity.
- **destruction:** Account for every resource on success and failure; a borrowed/trivial value has no owned destructor but still has an invalidation point.
- **dispatch:** Identify direct branches, function pointers or generated specializations; inspect release assembly before claiming dispatch elimination.
- **concurrency:** The supplied operations are single-threaded; prove absence of shared mutation or provide synchronization and reclamation for the required transfer. No TSan pass is inferred.
- **failure:** List each admitted failure and exact post-state; assess a boundary counterexample and distinguish caller-contract violations from operational failure.
- **compilation:** Record preprocessing/compilation time and diagnostic controls; generated source size and type-checking are distinct from runtime safety.
- **binary and stack:** Record object text/data, compiler stack-use report and call depth; compare under identical semantics, not a weaker unchecked implementation.
- **boundary:** Name the language-level counterpart, identify ISO/diagnostic/runtime/external-verification/caller enforcers and falsify one overstated equivalence.

Specific falsification and transfer: Run replicated.sequence; enumerate topological arrival orders and deletion subsets against a recursive tree oracle. Kill arrival-order sibling rendering and deleted-subtree omission mutants. Check full capacity, duplicate identity, missing dependency and output failure atomicity. Enumerate both paths of the two-insert OT diamond, then provide a three-operation context counterexample to an unjustified generalization. Add delete operations delivered before insert, Unicode scalar payloads and snapshot compaction. Compare an explicit pending-dependency queue with tombstone admission. Implement an OT context-control protocol with state vectors and retained transformations, identify assumptions about fixed membership and failures, and test more than pairwise transformation equations.

## PR-T76-07: Merkle commitments and bounded anti-entropy

Bounded stack buffers128bytes padding plus64uint32 schedule per SHA call; recursive tree depth<=3 for8leaves, O(n) full root, changed-range search retains hash costs. No heap. Measure actual stack with inlining and account for repeated subtree hashing in this inspectable implementation.

- **representation:** List exact fields, sizeof/alignment/padding observations and the abstraction function; distinguish ISO constraints from ABI observations.
- **reachable storage:** Draw pointers/indices and the retained backing extent; include storage kept alive by a borrow even when owned bytes are zero.
- **lifetime:** State creation, invalidation and final permitted access for each representation; construct a counterexample to any automatic-lifetime claim.
- **mutation and aliasing:** Identify all writable fields and permitted aliases; distinguish disjoint API preconditions from constraints actually diagnosed by C.
- **allocation:** Count allocation requests including prerequisite adapters; bounded inline storage is zero dynamic allocation, not zero resource use.
- **movement and copying:** Count copied payload bytes and metadata moves; show whether a copy changes ownership, identity or borrow validity.
- **destruction:** Account for every resource on success and failure; a borrowed/trivial value has no owned destructor but still has an invalidation point.
- **dispatch:** Identify direct branches, function pointers or generated specializations; inspect release assembly before claiming dispatch elimination.
- **concurrency:** The supplied operations are single-threaded; prove absence of shared mutation or provide synchronization and reclamation for the required transfer. No TSan pass is inferred.
- **failure:** List each admitted failure and exact post-state; assess a boundary counterexample and distinguish caller-contract violations from operational failure.
- **compilation:** Record preprocessing/compilation time and diagnostic controls; generated source size and type-checking are distinct from runtime safety.
- **binary and stack:** Record object text/data, compiler stack-use report and call depth; compare under identical semantics, not a weaker unchecked implementation.
- **boundary:** Name the language-level counterpart, identify ISO/diagnostic/runtime/external-verification/caller enforcers and falsify one overstated equivalence.

Specific falsification and transfer: Run replicated.merkle, standard empty/abc hash vectors, all padding boundaries and independent randomized hashlib differential cases. Kill omitted leaf-domain prefix and ignored right-child mutants. Inspect the generated code for the intended unsigned operations; benchmark nodes hashed versus full leaf comparison under equal correctness/security contracts. Add inclusion proofs with explicit leaf index/count, verify malformed/truncated/extra paths, then implement anti-entropy over a synthetic local framed transport. Vary chunking and domain identifiers and show root incompatibility. Model a malicious peer returning a missing or inconsistent node; fail without publishing partial repaired state. Do not claim collision resistance from a finite test suite.

## PR-T76-08: Field tags, minimal varints and canonical bytes

Eightinlinefieldpairs,48byte staged output; parsing O(bytes), simple tag-scan encoder O(15*8), no allocation. Unknown field retention consumes bounded slots. Wire compactness and CPU branches must be measured against a fixed-width alternative with identical validation.

- **representation:** List exact fields, sizeof/alignment/padding observations and the abstraction function; distinguish ISO constraints from ABI observations.
- **reachable storage:** Draw pointers/indices and the retained backing extent; include storage kept alive by a borrow even when owned bytes are zero.
- **lifetime:** State creation, invalidation and final permitted access for each representation; construct a counterexample to any automatic-lifetime claim.
- **mutation and aliasing:** Identify all writable fields and permitted aliases; distinguish disjoint API preconditions from constraints actually diagnosed by C.
- **allocation:** Count allocation requests including prerequisite adapters; bounded inline storage is zero dynamic allocation, not zero resource use.
- **movement and copying:** Count copied payload bytes and metadata moves; show whether a copy changes ownership, identity or borrow validity.
- **destruction:** Account for every resource on success and failure; a borrowed/trivial value has no owned destructor but still has an invalidation point.
- **dispatch:** Identify direct branches, function pointers or generated specializations; inspect release assembly before claiming dispatch elimination.
- **concurrency:** The supplied operations are single-threaded; prove absence of shared mutation or provide synchronization and reclamation for the required transfer. No TSan pass is inferred.
- **failure:** List each admitted failure and exact post-state; assess a boundary counterexample and distinguish caller-contract violations from operational failure.
- **compilation:** Record preprocessing/compilation time and diagnostic controls; generated source size and type-checking are distinct from runtime safety.
- **binary and stack:** Record object text/data, compiler stack-use report and call depth; compare under identical semantics, not a weaker unchecked implementation.
- **boundary:** Name the language-level counterpart, identify ISO/diagnostic/runtime/external-verification/caller enforcers and falsify one overstated equivalence.

Specific falsification and transfer: Run replicated.wire and an independent Python parser/encoder for boundary values and reordered field maps. Kill overlong-varint acceptance and incorrect negative ZigZag mutants. Enumerate every truncation, duplicate field, forbidden tag/wire type and output-capacity boundary; demonstrate two noncanonical orders producing the same canonical bytes. Add length-delimited nested records with depth and total-byte budgets, a critical/advisory negotiation rule and an incremental io.frame decoder. Compare schema migration that retains unknown fields against field-wise reconstruction that loses them. Define canonical NaN/negative-zero/string normalization policy before adding floating or textual values; do not assume the current integer proof extends.

## PR-T76-09: Pure reducers, explicit effects and rollback replay

Four scalar state fields, bounded8effect staging and imported HAMT arena; replay O(horizon), each checkpoint6clonednodes, retained roots pin storage. No core dynamic allocation or system calls; adapter latency, duplicated speculation and retained snapshot costs are separate dimensions.

- **representation:** List exact fields, sizeof/alignment/padding observations and the abstraction function; distinguish ISO constraints from ABI observations.
- **reachable storage:** Draw pointers/indices and the retained backing extent; include storage kept alive by a borrow even when owned bytes are zero.
- **lifetime:** State creation, invalidation and final permitted access for each representation; construct a counterexample to any automatic-lifetime claim.
- **mutation and aliasing:** Identify all writable fields and permitted aliases; distinguish disjoint API preconditions from constraints actually diagnosed by C.
- **allocation:** Count allocation requests including prerequisite adapters; bounded inline storage is zero dynamic allocation, not zero resource use.
- **movement and copying:** Count copied payload bytes and metadata moves; show whether a copy changes ownership, identity or borrow validity.
- **destruction:** Account for every resource on success and failure; a borrowed/trivial value has no owned destructor but still has an invalidation point.
- **dispatch:** Identify direct branches, function pointers or generated specializations; inspect release assembly before claiming dispatch elimination.
- **concurrency:** The supplied operations are single-threaded; prove absence of shared mutation or provide synchronization and reclamation for the required transfer. No TSan pass is inferred.
- **failure:** List each admitted failure and exact post-state; assess a boundary counterexample and distinguish caller-contract violations from operational failure.
- **compilation:** Record preprocessing/compilation time and diagnostic controls; generated source size and type-checking are distinct from runtime safety.
- **binary and stack:** Record object text/data, compiler stack-use report and call depth; compare under identical semantics, not a weaker unchecked implementation.
- **boundary:** Name the language-level counterpart, identify ISO/diagnostic/runtime/external-verification/caller enforcers and falsify one overstated equivalence.

Specific falsification and transfer: Run replicated.replay and exhaustively compare all movement histories of length0..8 with a Python integer model; verify suffix rollback versus full replay. Kill omitted random-score contribution and failed second-update allocation-leak mutants. Build a synthetic effect adapter with idempotency key(timeline,tick) and injected failure; record one duplicate-effect counterexample if replay directly invokes the adapter. Inspect undefined symbols to confirm the core has no time/random/I/O dependency. Introduce two local simulated players, predicted remote inputs and delayed corrections using ipc.local/io.frame, with bounded rollback horizon and retained snapshots. Separate authoritative committed effects from speculative presentation and define replacement/cancellation semantics. Reuse the actual T20.03 learner reducer adapter and storage.pages snapshot backend, then vary compiler/endianness and compare canonical state hashes. Implement a hidden wall-clock/random dependency as a deliberately failing counterexample, not a production change.

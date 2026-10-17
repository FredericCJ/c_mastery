# Mechanism-specific cost and enforcement review

Complete measured columns with compiler flags, architecture, workload and equivalent correctness contracts. The following are analytical expectations, not invented measurements.

## PR-T67-01: Products, tagged sums and checked elimination

ADValue inline tag plus maximum union member; no reachable heap; direct branch dispatch; measure O(1), no allocation or cleanup; layout/padding and branch lowering measured separately.

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

Specific falsification and transfer: Run adt.sums and compile the missing-enumerator probe with -Wswitch-enum -Werror. Both semantic mutants must fail: omitted second product member and silently accepted invalid tag. Add every variant and invalid boundary, inspect enum size without assuming it is the wire tag size, and compare debug/release generated branches. Add an owning text variant with a failing clone callback. Derive copy/drop/rollback and exhaustiveness obligations for every old eliminator; demonstrate which clients fail compilation and which silently preserve a default path.

## PR-T67-02: Newtypes and checked dimensional conversions

Single uint32 field per wrapper; no storage graph; one comparison and multiply; O(1), no allocation or destruction. Measure whether wrapper calls inline under equivalent overflow contracts.

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

Specific falsification and transfer: Run adt.quantities and reject wrong nominal type in a separate compilation. Kill missing-range-check and wrong-scale mutants, then test both sides of the quotient boundary. Show an explicit cast or field extraction bypass and identify programmer discipline rather than claim unforgeability. Introduce signed microseconds and rational tick periods with a nonintegral ratio. Implement checked conversion with explicit rounding toward zero and a loss-of-precision result; derive bounds without a wider integer type.

## PR-T67-03: Typed flag sets and reserved-bit admission

One uint32 payload, no heap or aliasing; constant-time masks; no allocation/drop. ABI width and wire width remain separate. Forwarding unknown bits increases retained state and trust assumptions.

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

Specific falsification and transfer: Run adt.flags; exhaustively test0..255 and each single bit position. Reject the silent-mask mutant and empty-set rejection mutant. Show a newly reserved bit cannot be accidentally converted into success by masking. Negotiate separate required and advisory bit sets between two schema versions. Preserve unknown advisory bits for forwarding while rejecting unsupported required bits, and prove the union operation cannot manufacture support.

## PR-T67-04: UTF-8 smart constructors and validated borrowed text

Pointer+two size_t fields; retains entire caller allocation without owning it; O(bytes), no allocation/copy/drop; scalar count costs a scan. Decoder branches and cache traffic measured separately from later text access.

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

Specific falsification and transfer: Run adt.text and the Python strict UTF-8 differential oracle for all one/two-byte strings plus specified three/four-byte boundary samples. Kill overlong and surrogate acceptance mutants. Record that Python decoding is an independent implementation, not a proof of normalization or all higher-level text properties. Add an incremental decoder across chunks with at most3retained bytes and finalization. Test every split of each boundary sequence, cancellation that drops pending bytes, and a copying owner that can preserve validation under caller mutation.

## PR-T67-05: Branded pool handles with checked generations

Four inline slots plus owner/index/generation handles; no allocator; acquire O(4), lookup/releaseO(1); retirement consumes capacity; handles copy cheaply but lifetime obligations remain. No concurrent support is claimed.

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

Specific falsification and transfer: Run adt.brands and enumerate short acquire/release/get traces against an independent occupancy/generation model. Kill owner-check omission and generation-check omission independently. Compile a wrong-category handle probe; show that copying or explicitly constructing a valid public representation bypasses any unforgeability claim. Replace the owner pointer with a nonreused externally assigned pool identity and implement relocation of the pool. Then introduce concurrent get/release: provide a reclamation/lifetime protocol, not just an atomic generation counter.

## PR-T67-06: Read-only strided views and ownership boundaries

Pointer+five size_t values; no owner/allocator/drop; constructionO(1), indexingO(1); striding changes memory traffic and vectorization; empty views retain no required elements. Measured layout-specific throughput must preserve admitted semantics.

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

Specific falsification and transfer: Run adt.views; enumerate small shapes/strides/capacities against a Python coordinate-set oracle. Kill row-major-assumption and missing-capacity-admission mutants. Audit zero/SIZE_MAX boundaries and distinguish arithmetic acceptance from existence of a real caller object of that extent. Implement mutable transpose/submatrix views and a SIMD reduction adapter. Establish injectivity or documented repeated-write semantics, equivalent overflow/alias contracts, and measure scalar/vector behavior only on admitted layouts.

## PR-T67-07: Recursive arena values and typed indices

Up to32inline nodes and128bytes value scratch on32bit uint32; no heap; linear traversal; O(1)call depth; shared subexpressions computed once; whole-arena validation includes unreachable nodes and costs time.

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

Specific falsification and transfer: Run adt.arena and independent postfix/tree evaluation cases. Kill forward-reference acceptance and arithmetic-overflow omission. Add unreachable malformed nodes, multiple roots, shared children, empty and maximum count, then record stack-use output. Support bounded cyclic graphs for a monotone dataflow analysis. Replace topological induction with a lattice/worklist convergence argument and distinguish no fixed point, insufficient iteration budget and successful convergence.

## PR-T67-08: Lenses and prisms with falsifiable laws

Inline product/sum values; copies bounded fields, no allocation/drop; O(1)direct calls; law-check enumerationO(domain^4); optimizer inlining does not enforce algebraic laws.

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

Specific falsification and transfer: Run adt.optics:256combinations assess get-put/put-get/put-put and prism round trips. Kill the collateral-y-write and EMPTY-as-NUMBER mutants. Add composition of nested pairs, identify a law violated by a clamping setter, and explain why exhaustive small domains are not proof for arbitrary callback optics. Implement a lens into an optional nested configuration and a prism for a versioned error sum. Decide which updates are partial, state revised laws, and produce counterexamples to importing total-lens laws unchanged.

## PR-T67-09: Evolution-aware sums and lossless tolerant readers

Tag+pointer+size_t borrow; no parse allocation; readO(1),writeO(payload), retained backing; re-encode copies bytes; no destructor; critical policies alter admissibility not just performance.

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

Specific falsification and transfer: Run adt.evolution and construct a two-version relay trace. Kill unknown-payload-discard and malformed-known-acceptance mutants. Test all truncation points, trailing bytes, capacity shortages, zero-length unknown payload and borrowed-buffer mutation. Record which operations preserve unknown data and which deliberately reject it. Add duplicate fields and canonical ordering to an extensible record format. Specify how unknown duplicates participate in signatures and forwarding, and demonstrate why accepting arbitrary unknown fields can violate a higher-level security policy.

## PR-T67-10: Zero-copy packet view with precise error sum

Kind+pointer+size_t, zero payload copy/allocation; parseO(1), frame assembly cost accounted separately; borrowed retention can delay buffer reuse and dominate memory cost; no thread safety or automatic pinning.

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

Specific falsification and transfer: Run adt.packet and enumerate header/extent/tag combinations against the independent byte parser. Kill trailing-byte acceptance and nonempty-ack acceptance separately. Test all truncations and prior-output preservation, then integrate actual learner io.frame and bytes.cursor artifacts with recorded source identities. Public direct-parser anchors do not establish that integration. Add segmented zero-copy payloads from a receive ring with wraparound and a retained borrow token. Prove storage cannot be recycled while a consumer holds the packet; compare copying and borrowing under equivalent release/cancellation semantics.

# Mechanism-specific cost and enforcement review

Complete measured columns with compiler flags, architecture, workload and equivalent correctness contracts. The following are analytical expectations, not invented measurements.

## PR-T68-01: Template-header monomorphization and associated types

Two typed inline4slot rings; generated copies of push/pop code; no dynamic allocation/dispatch; O(1)operations. Measure preprocessed tokens, object symbols and text size; owning transfer adds callback and cleanup costs.

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

Specific falsification and transfer: Run metaprog.mono and inspect preprocessed two-instance source. Kill incorrect-tail-index and stationary-head mutants. Compile and link two translation units using declarations to detect accidental multiple definitions; record generated symbol count and text size separately from handwritten equivalents under the same capacity/failure contract. Instantiate a ring of owning objects with fallible clone/drop callbacks. Derive rollback on failed push and destruction order on clear; compare monomorphized and erased implementations with equal semantics and measured code growth.

## PR-T68-02: Constraint probes and constant storage parameters

No owned storage in bound checker; constant instances embed N*T bytes; O(1)runtime admission. Compilation expands each constant instance; binary/stack use grows with representation even when the check optimizes away.

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

Specific falsification and transfer: Run metaprog.constraints and compile the negative constraint probes expecting a named diagnostic, not a link failure. Kill multiplication-overflow omission and storage-budget omission independently. Test size_t-width changes analytically and inspect whether published C23 wording is available before attributing a C2x compiler behavior to the standard. Support a compile-time power-of-two ring capacity and a runtime fallback. Derive which optimization can use masking, reject an invalid constant capacity, and ensure a nonconstant caller cannot bypass runtime validation.

## PR-T68-03: Policy composition with explicit failure semantics

Two small policy fields, inline accumulator, O(n)time/O(1)space; no allocation/drop; dynamic branch versus generated code; projection transfer adds dispatch/effect costs that must be included.

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

Specific falsification and transfer: Run metaprog.policies and inject every prefix crossing the limit. Kill accidental-saturation-for-rejection and wrong-empty-identity mutants. Inspect specialized and dynamic assembly and benchmark only comparisons preserving the same overflow and empty-input contracts. Compose reduction with a caller-defined projection and fallible element acquisition. Specify evaluation order, callback side effects, rollback limits and whether saturation may short-circuit without skipping required effects.

## PR-T68-04: Portable C17 variadic machinery and single evaluation

Preprocessor-only selector plus typed calls, no runtime allocation; evaluationO(arity<=4); generated declarations and diagnostics have compile-time cost. Zero case explicit; no GNU/C23 mechanism dependency.

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

Specific falsification and transfer: Run metaprog.variadic, inspect -E output, and compile strict C17 probes. Kill duplicate-evaluation and incorrect-zero-identity mutants. Add arguments with commas protected by parentheses and demonstrate an unsafe unparenthesized form in a diagnostic-only target; retain a separate C2x __VA_OPT__ observation. Build a bounded logging dispatcher that optionally attaches source metadata, with a distinct zero-field operation. Preserve single evaluation and type checking while making expansion limits explicit; diagnose unsupported arity without compiler-specific comma extensions.

## PR-T68-05: Generic customization with cv-preserving receivers

No wrapper allocation or type tag; compile-time selection, direct calls after expansion; runtime marker adds ordinary arithmetic; pointer expressions evaluate once. Diagnostics and code size measured per association set.

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

Specific falsification and transfer: Run metaprog.generic and compile unsupported-type/const-write diagnostic probes. Kill const-receiver-policy substitution and extra controlling-expression evaluation mutants. Check GCC and Clang with C17, and record implementation differences involving top-level qualifiers as observations requiring precise source interpretation. Add volatile device-view receivers through explicit APIs. Separate permitted volatile access, side effects and ordering from constness; inspect emitted accesses and avoid claiming that volatile supplies synchronization or device barriers.

## PR-T68-06: Single-source field reflection and explicit wire operations

Inline record plus static descriptor strings/offsets; descriptor storageO(fields), encodeO(wirebytes); no heap; registry expands several consumers and increases generated source/binary data. Native padding never enters the wire cost.

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

Specific falsification and transfer: Run metaprog.reflection and regenerate a changed schema in a temporary workspace. Kill endian reversal and missing-field-descriptor mutants. Compile under altered packing/alignment only as an implementation-specific comparison and retain canonical wire bytes unchanged. Add an optional tagged field and derive decode, equality, validation and visitor operations. Define migration/version rules and demonstrate a stale generated consumer that still compiles; design a build-time schema fingerprint check that catches it.

## PR-T68-07: Derived equality, hashing and shrinking law counterexamples

Keys inline3words; equalityO(1),hash16specifiedbytes; generatorO((bound+1)^2)calls and boundedshrinking,O(1)storage; no allocator; callback dispatch/effects explicit. Crypto collision resistance is not promised.

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

Specific falsification and transfer: Run metaprog.laws, independently recompute canonical hashes and enumerate bounded law triples. Kill equality-ignores-y and hash-includes-cache mutants. Inject a deliberately wrong equality/hash callback through the learner testing.faults adapter and retain a minimized witness; explain why equal hashes do not imply equal keys. Add an allocation-failing string owner and generators for operation traces. Shrink while preserving preconditions and the same failure class; compare bytewise, semantic and normalized-text equality contracts before deriving hashes.

## PR-T68-08: Typed formatting and transactional bounded sinks

Tagged argument plus bounded sink triple; up to10temporarybytes; decimalO(digits),textO(length); no heap or varargs runtime; failure preflight may scan text beforecopy. Streaming changes resource and recovery semantics.

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

Specific falsification and transfer: Run metaprog.formatting, compile an unsupported MP_ARG category probe, and test every capacity0..11. Kill failure-used-count modification and zero-as-empty-output mutants. Compare against an independent decimal-string oracle and a guard-filled output suffix; distinguish valid text termination preconditions from bound checking. Add a parsed format DSL with typed placeholder descriptors and partial streaming sinks. Specify format validation separately from argument typing, and make partial writes/retry offsets explicit rather than silently keeping the transactional API promise.

## PR-T68-09: A bounded hygienic token expander written in C

Tokens3unsignedfields, at most32temporarytokens; O(tokens),noheap; copies alloutput transactionally; boundedcontextnamespace can exhaust; no native code execution or parser dependency. Nested expansion adds explicit budget/stack costs.

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

Specific falsification and transfer: Run metaprog.hygiene and compare against an independent token/context transformer. Kill retagged-call-site-argument and wrapping-context mutants. Build a tiny environment resolver to demonstrate harmless capture: the flawed output resolves the argument to the introduced synthetic binding instead of its caller binding. Retain corrected and flawed token traces. Add nested macros, repetition with equal-length metavariables and a depth/token expansion budget. Specify intentional capture escapes explicitly, prove budget consumption, and distinguish nontermination prevention from complete macro-language equivalence.

## PR-T68-10: Declarative build graphs and deterministic table generation

At most8nodes, temporary8keys/order, bounded256bytegeneratorbuffer; graphO(n^2+sourcebytes),noheap; staticgeneratedarray8bytes/key; compile/linkcost assessed separately. Cachecollision verification and parallel ownership add costs.

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

Specific falsification and transfer: Run metaprog.buildgraph and independent Python topological/hash reconstruction. Kill omitted-configuration and omitted-dependency-key mutants. Compile/run the generated C table consumer, permute input storage without changing logical IDs, and inject a missing implicit dependency to demonstrate stale generated metadata. Introduce an incremental cache with toolchain, environment, include-file and schema dependencies. Simulate crashes during artifact publication and hash collisions; define atomic replacement and full-input verification. Compare a parallel scheduler only after establishing dependency and output ownership.

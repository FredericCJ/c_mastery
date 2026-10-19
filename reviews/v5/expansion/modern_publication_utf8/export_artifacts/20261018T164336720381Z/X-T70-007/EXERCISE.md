# X-T70-007: Bitmap rank, full-hash collisions and persistent map updates

## Prerequisites

Bring pinned contracts and fresh recall for memory.pool, containers.array, algorithms.radix; prerequisite exercises: X-T70-006. Required knowledge includes C object lifetime, checked arithmetic, ownership, typed arrays and the selected source passages. The shared contract and modern-cost contract are explicit parts of this exercise. Existing project estimates describe author accounting; they impose no learner time ceiling.

## Environment

Use C17 hosted Linux with GCC and Clang, CMake and host Python. Record compiler/ABI versions, integer widths, sizeof/alignment and optimization. Mechanism code uses no library implementing the mechanism. Allocator callbacks are supplied dependencies with separate ownership. A strict freestanding port requires its own generated-code/link audit and is not established by the hosted run. Compare C11 draft/C17/C23 only where inspected normative evidence supports the claim; exact published C17 access remains a reference gap.

## Problem

Implement DHamt for keys0–31 with hash(key)=key&15, two2-bit levels, compact bitmap-indexed children, and equality-checked two-entry terminal collision buckets. Empty root0 and64 typed nodes bound the model. Update copies leaf and ancestors; lookup returns D_STALE for absence. Add a deletion operation with an explicit policy for empty internal nodes, and a tree-to-flat-map oracle. Do not use hash equality as key equality. Discuss extension to arbitrary-length keys by defining hash/equality contracts and ownership before substituting a hash function.

Supplied artifacts: labs/dod/include/dod.h, deliberately incomplete labs/dod/src/dod.c, executable anchors, fixture PR-T70-07.json and the shared CONTRACTS.md. Expected example: Keys1 and17 deliberately share the full4-bit hash but retain values10 and20. Updating17 to88 in a new root leaves20 in the old root. Inserting key0 changes bitmap rank and must not make key1 unreachable. Each of3 allocation failure points preserves old snapshots.

## Necessity

A HAMT description hides three distinct equivalences: shared prefix, identical hash and equal key. Confusing them overwrites unrelated values. Sparse child packing also makes insertion rank as important as lookup rank.

## Reasoning

T70.07.a: Derive bitmap rank as the population count of preceding bits and prove compact-child count/order invariants; distinguish trie-prefix, full-hash and actual-key equality.

T70.07.b: Implement lookup/update with collision buckets and path copying, then deletion with defined empty-node normalization and immutable prior snapshots.

T70.07.c: Exhaust keys0–31 and insertion/deletion permutations for collision pairs and differing bitmap positions; compare each version with an independent dictionary and inject every clone failure.

T70.07.d: Analyze hash quality, collision-bucket growth, retained versions and pointer/bitmap costs under equal contracts. Transfer to an attacker-selected constant hash and specify bounded denial-of-service behavior without claiming constant worst-case lookup.

State preconditions, postconditions, representation invariants, resource ownership, failure publication and complexity. For each guarantee identify ISO C rules, diagnostics, runtime checks, external verification and programmer discipline separately. Include a minimal counterexample to an overstated equivalence with the corresponding language/library abstraction. Apply all13 dimensions in modern-cost-contract.md, recording genuine non-applicability with reasons and unavailable measurements as gaps.

## Deliverables

- D1: representation/state diagrams, source-to-claim ledger and proofs for every listed objective, including a plausible incorrect explanation and a concrete counterexample.
- D2: mechanism implementation and every separately requested extension, dependency adapters and a reproducible build; document each source of allocation, invalidation and partial progress.
- D3: Generate every key pair and both insertion orders, then update and delete either member while retaining all roots. Add triplets selecting low chunks0,1,3 and full-hash pair0/16; compare flattening to sorted key/value pairs from a dictionary. For every node recompute count and rank independently; test pool exhaustion. The deletion extension now has its own allocation/normalization controls and source-bound campaign evidence; it is not credited from the original lookup/update anchor.
- D4: changed-condition implementation/analysis, the13-dimension cost dossier and a delayed independent attempt. Record assistance and all unexecuted target/measurement obligations.

## Verification

Run `cmake -S labs/dod -B build/dod` then build and `ctest --test-dir build/dod -R "^dod.hamt$" --output-on-failure` for your implementation. Rebuild GCC/Clang at debug/release and sanitizer profiles. Generate every key pair and both insertion orders, then update and delete either member while retaining all roots. Add triplets selecting low chunks0,1,3 and full-hash pair0/16; compare flattening to sorted key/value pairs from a dictionary. For every node recompute count and rank independently; test pool exhaustion. The deletion extension now has its own allocation/normalization controls and source-bound campaign evidence; it is not credited from the original lookup/update anchor.

Pass requires the exact observable results, no invariant violation in the admitted state space, intended mutant rejection, independent expected values and level2 or better for every objective. A public test pass alone cannot satisfy written extensions or level3 transfer. The seven-day delay is an assessment condition, not a time limit on learning.

## Rubric

The machine-readable objective-specific rubric above is normative. Levels0–1 are incomplete; level2 requires justified implementation and falsification evidence; level3 requires independent transfer under changed assumptions. Assess each objective separately. Source-bound tests establish only their bounded behavior; they are not reviewer independence or learner mastery.

## Hints

1. The mask is a set of occupied logical slots, not a dense array index.
2. Full-hash collision still requires key comparison.
3. Deletion can leave an empty branch or normalize it, but the contract and cost model must choose explicitly.

## Variants

X-T70-007-DELAYED: use two-byte keys and a caller-supplied hash/equality dictionary. Construct unequal keys with equal hash and equal logical keys with different backing bytes; define canonical equality and ownership, then re-evaluate collision and failure costs.

Also change compiler/optimization and an available data model. Revisit integer availability, object layout, helper dependencies and performance arguments rather than assuming numerical output validates portability. Assessor technical analysis and withheld cases are separate from learner publication.

## Security

Use only these original local teaching structures and synthetic values. Attacker influence is limited to admitted operation sequences, lengths, masks, hashes or allocation-failure schedules in this process. Observe stale-access rejection, lost-state prevention or resource-exhaustion behavior without dereferencing expired storage. A crash or sanitizer finding is not evidence of arbitrary execution. Repair violated invariants and add regression traces; surviving raw aliases, malicious allocator callbacks and concurrent mutation remain outside this bounded contract unless the variant explicitly adds them.

## Validation

Authored complete specification; executable anchor and assessor reference are separately source-bound in the campaign receipt. Learner artifacts, independent assessment, written extensions, additional data models and physical measurements are not implied by author test results. The source register records only inspected passages, and explicitly records inaccessible material. Residual obligations remain required, not optional exclusions.

## Required extension support

Public headers `dod_persistent.h` and `dod_text.h`, their compiling source
stubs and the explicit shared extension contracts now supply the separately
required mechanisms assessed by dod.hamt_delete. Include these
checks and the independent dictionary/sequence oracle in D3. Interpret each
new result at its stated finite bounds; no prior anchor result is retroactively
credited as execution of this extension. Preserve separately required cost,
changed-hash, independent/delayed and target obligations.

## Caller dictionary and adversarial hash investigation

The required changed-hash and delayed two-byte-key work now has a compiling
scaffold `dod_dictionary.h` / `src/dod_dictionary.c`, fixture
`fixtures/dod/dictionary.json`, and the `dod.dictionary` anchor. Read the shared
"Caller dictionary" contract before implementing it. This is a separate path
from the original four-bit map: all32 hash bits participate, equal logical keys
can have distinct byte representations, and immutable nodes own two copied key
bytes. Hash/equality callback contexts remain borrowed and stable.

D2 must implement update, lookup and normalized deletion, preserving retained
roots at all17 allocation cut points. D3 must derive the independent canonical
map oracle, recompute bitmap/rank invariants, and demonstrate the constant-hash
bucket cost and bounded ninth-insertion rejection. Compare the measured node
and arena footprint with the original map; the large fixed union-of-fields
node is intentionally inspectable, not a claim of space-optimal HAMT storage.

For R4, distinguish collision comparisons, cloned bytes, arena retention and
callback cost. An adversarial callback that never returns is outside the finite
work bound. ISO C type compatibility checks callback signatures; canonical
semantic equality, equal-key/equal-hash consistency and stable context lifetime
remain programmer obligations. The supplied constant-hash dictionary is valid
but slow; a hash that fails to canonicalize logically equal keys violates a
precondition and may make them unreachable. A bounded local exhaustion result
is not a security guarantee for an unrestricted production map.

## Objective-specific assessment anchors

### X-T70-007-R1 / T70.07.a

Derive bitmap rank as the population count of preceding bits and prove compact-child count/order invariants; distinguish trie-prefix, full-hash and actual-key equality.

- 0: Absent or contradicts the stated representation/contract.
- 1: Runs the nominal example but cannot derive the relevant invariant or explain a failing boundary.
- 2: Provides the complete stated result, a justified invariant and independently checked falsification evidence.
- 3: Transfers the argument to the changed assumptions without a worked solution, identifying the revised costs and limits.

### X-T70-007-R2 / T70.07.b

Implement lookup/update with collision buckets and path copying, then deletion with defined empty-node normalization and immutable prior snapshots.

- 0: Absent or contradicts the stated representation/contract.
- 1: Runs the nominal example but cannot derive the relevant invariant or explain a failing boundary.
- 2: Provides the complete stated result, a justified invariant and independently checked falsification evidence.
- 3: Transfers the argument to the changed assumptions without a worked solution, identifying the revised costs and limits.

### X-T70-007-R3 / T70.07.c

Exhaust keys0–31 and insertion/deletion permutations for collision pairs and differing bitmap positions; compare each version with an independent dictionary and inject every clone failure.

- 0: Absent or contradicts the stated representation/contract.
- 1: Runs the nominal example but cannot derive the relevant invariant or explain a failing boundary.
- 2: Provides the complete stated result, a justified invariant and independently checked falsification evidence.
- 3: Transfers the argument to the changed assumptions without a worked solution, identifying the revised costs and limits.

### X-T70-007-R4 / T70.07.d

Analyze hash quality, collision-bucket growth, retained versions and pointer/bitmap costs under equal contracts. Transfer to an attacker-selected constant hash and specify bounded denial-of-service behavior without claiming constant worst-case lookup.

- 0: Absent or contradicts the stated representation/contract.
- 1: Runs the nominal example but cannot derive the relevant invariant or explain a failing boundary.
- 2: Provides the complete stated result, a justified invariant and independently checked falsification evidence.
- 3: Transfers the argument to the changed assumptions without a worked solution, identifying the revised costs and limits.

## Traceability and contract selection

Project: PR-T70-07

Required earlier exercises: X-T70-006

Required baseline units: none

Required artifact imports: memory.pool, containers.array, algorithms.radix


- [R-MOD-DOD-HAMT](https://docs.rs/im/latest/im/hashmap/struct.HashMap.html): HashMap description: immutable HAMT and hash/equality requirements; update, update_with and without methods
- [R-MOD-DOD-IM](https://docs.rs/im/latest/im/): Why Would I Want This?: structural sharing and workload tradeoffs; Values: Clone requirement for copied shared nodes

Delayed transfer: wait at least 7 days after the first recorded study of this contract; select X-T70-007-DELAYED.

Contract SHA-256: 8828460af7d75fd696c612bfcba6698429924f23b454efd3b44fde18fc935e6c

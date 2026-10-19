# X-T69-005: Owning erased calls over the byte-only small buffer

## Prerequisites

Bring pinned contracts and fresh recall for dod.small_buffer, memory.pool; prerequisite exercises: X-T70-002, X-T69-004, X-T66-009. The completion stage imports memory.adversarial_allocator through the explicit character-row adapter in capture.h. Required knowledge includes C object lifetime, checked arithmetic, ownership, typed arrays and the selected source passages. The shared contract and modern-cost contract are explicit parts of this exercise. Existing project estimates describe author accounting; they impose no learner time ceiling.

## Environment

Use C17 hosted Linux with GCC and Clang, CMake and host Python. Record compiler/ABI versions, integer widths, sizeof/alignment and optimization. Mechanism code uses no library implementing the mechanism. Allocator callbacks are supplied dependencies with separate ownership. A strict freestanding port requires its own generated-code/link audit and is not established by the hosted run. Compare C11 draft/C17/C23 only where inspected normative evidence supports the claim; exact published C17 access remains a reference gap.

## Problem

Implement DSErased assign/call/move/drop by importing DSmall, not copying its algorithm. Captures are explicitly encoded counted bytes; the supplied thunk accepts one byte and argument[-100,100]. Add an int32 capture codec with a portable admitted signed conversion and a capture larger than16 bytes to exercise spill. Resource ownership transfers only after assign succeeds. A separate borrowing callable view must document owner/table lifetime and immediate versus retained use.

Supplied artifacts: labs/dispatch/include/dispatch.h, deliberately incomplete labs/dispatch/src/dispatch.c, executable anchors, fixture PR-T69-05.json and the shared CONTRACTS.md. Expected example: Capture byte7 called with3 produces10. Move leaves the source inactive/empty and destination active. Dropping source then destination twice invokes the resource destructor exactly once. Allocation failure during a large capture assignment leaves the destination empty and ownership with the caller.

## Necessity

The owning wrapper adds obligations beyond a function/context pair: capture representation, external cleanup, movement and borrowed result lifetime. Reusing T70 preserves a single storage contract.

## Reasoning

T69.05.a: Derive erased-call representation, invocation thunk, ownership and empty states; distinguish byte encoding from placing an arbitrary typed object in character storage.

T69.05.b: Implement capture construction, explicit movement and no-fail destruction through dod.small_buffer with no duplicated allocator logic, including a spill and failure rollback.

T69.05.c: Inject each allocation/codec failure, test source/destination/self move and repeated drop, and verify exactly-once external destruction plus unchanged outputs on failing calls.

T69.05.d: Compare owning C erasure, a borrowed function reference and the documented C++ move_only_function/function_ref contracts. Analyze inline storage, movement constraints and empty invocation without claiming common ABI or universal small-buffer capacity.

T69.05.e: Transfer to a self-referential or over-aligned capture and a callback returning a borrowed reference. Either provide a justified typed representation/relocator/lifetime protocol or reject it explicitly; a byte copy is not a general solution.

State preconditions, postconditions, representation invariants, resource ownership, failure publication and complexity. For each guarantee identify ISO C rules, diagnostics, runtime checks, external verification and programmer discipline separately. Include a minimal counterexample to an overstated equivalence with the corresponding language/library abstraction. Apply all13 dimensions in modern-cost-contract.md, recording genuine non-applicability with reasons and unavailable measurements as gaps.

## Deliverables

- D1: representation/state diagrams, source-to-claim ledger and proofs for every listed objective, including a plausible incorrect explanation and a concrete counterexample.
- D2: mechanism implementation and every separately requested extension, dependency adapters and a reproducible build; document each source of allocation, invalidation and partial progress.
- D3: Drive assign/call/move/drop traces through a logical ownership model and an allocator request/release log. Exercise inline16 versus spill17 and capture128 versus129 using thunks that accept those schemas. Compile with sanitizers, but diagnose ownership-state errors before invoking an inactive or mismatched thunk. Count bytes copied, allocations, owner size, dispatch and destructors under the13-dimension contract.
- D4: changed-condition implementation/analysis, the13-dimension cost dossier and a delayed independent attempt. Record assistance and all unexecuted target/measurement obligations.

## Verification

Run `cmake -S labs/dispatch -B build/dispatch` then build and `ctest --test-dir build/dispatch -R "^dispatch.erased$" --output-on-failure` for your implementation. Rebuild GCC/Clang at debug/release and sanitizer profiles. Drive assign/call/move/drop traces through a logical ownership model and an allocator request/release log. Exercise inline16 versus spill17 and capture128 versus129 using thunks that accept those schemas. Compile with sanitizers, but diagnose ownership-state errors before invoking an inactive or mismatched thunk. Count bytes copied, allocations, owner size, dispatch and destructors under the13-dimension contract.

Pass requires the exact observable results, no invariant violation in the admitted state space, intended mutant rejection, independent expected values and level2 or better for every objective. A public test pass alone cannot satisfy written extensions or level3 transfer. The seven-day delay is an assessment condition, not a time limit on learning.

## Rubric

The machine-readable objective-specific rubric above is normative. Levels0–1 are incomplete; level2 requires justified implementation and falsification evidence; level3 requires independent transfer under changed assumptions. Assess each objective separately. Source-bound tests establish only their bounded behavior; they are not reviewer independence or learner mastery.

## Hints

1. Keep the resource with the caller until capture storage succeeds.
2. A successful move must change both ownership states.
3. Reconstruct typed values in actual typed locals rather than casting the inline byte array.

## Variants

X-T69-005-DELAYED: allow cloning only for captures whose clone callback can fail. Specify source preservation, destination rollback and destructor ordering when the second of two owned subresources fails to clone.

Also change compiler/optimization and an available data model. Revisit integer availability, object layout, helper dependencies and performance arguments rather than assuming numerical output validates portability. Assessor technical analysis and withheld cases are separate from learner publication.

## Security

Use only these original local teaching structures and synthetic values. Attacker influence is limited to admitted operation sequences, lengths, masks, hashes or allocation-failure schedules in this process. Observe stale-access rejection, lost-state prevention or resource-exhaustion behavior without dereferencing expired storage. A crash or sanitizer finding is not evidence of arbitrary execution. Repair violated invariants and add regression traces; surviving raw aliases, malicious allocator callbacks and concurrent mutation remain outside this bounded contract unless the variant explicitly adds them.

## Validation

Authored complete specification; executable anchor and assessor reference are separately source-bound in the campaign receipt. Learner artifacts, independent assessment, written extensions, additional data models and physical measurements are not implied by author test results. The source register records only inspected passages, and explicitly records inaccessible material. Residual obligations remain required, not optional exclusions.

## Completion stage: signed captures and typed comparison

Implement capture.h and src/capture.c using the counted int32 capture contract. Keep T70 storage as the producer and T69 invocation as its consumer; never add a reverse prerequisite. Build the actual T66 counting allocator with the explicit byte-row adapter. Compare an owning encoded capture, a borrowed typed slice and DSTypedCapture under the same final-sum overflow policy. Use fixtures/dispatch/captures.json and dispatch.capture_codec / dispatch.typed_capture. The independent host model checks48 traces of exact bytes, arithmetic and allocation ownership; the C boundary suite additionally fills all eight allocator rows and rejects the ninth acquisition, then proves complete release. Explain why the64-byte adapter bound is smaller than the general128-byte DSmall bound.

D2 requires all three implementations, ownership transitions and rejection behavior. D3 includes both allocation failure cuts, type/layout evidence, four compiled semantic mutant controls and the48-row oracle. R1–R3 assess codec legality, actual dependency reuse and failure atomicity; R4 compares borrowing/copying/destruction and concrete footprints; R5 still requires an independently reasoned self-reference/borrowed-result policy. The supplied safe self-reference counterexample demonstrates only that copying does not relocate its member pointer. The clone-failure variant is specified concretely below; the cost dossier must distinguish the measured host cases from unavailable environments.

## Completion stage: fallible two-resource cloning

Implement `DSCloneCapture` through actual DSErased/DSmall and the actual T66 allocator adapter. The header defines a clone/drop dictionary and exactly two distinct owned resources; all owners stay at stable addresses. Construct inline8 and spilled20-byte captures, then clone with no failure and cuts at first resource, second resource and spilled-byte allocation. The source must remain callable and retain both resources under every cut. Cut1 leaves no new resource; cut2 drops the completed first clone; spilled-allocation failure drops second then first and leaves destination empty. Success creates distinct pairs, and dropping destination leaves source live. Final source drop must release every resource/allocation exactly once.

Run `dispatch.clone_pair`, both mutants and the42-row extension oracle. D1 derives acquisition-prefix rollback and the source/destination invariants; D2 implements clone/adopt/drop without byte-copying a self-referential owner; D3 compares the independent expected live/drop/attempt counts and actual allocator failure boundary. R2/R3 assess real producer reuse and rollback, R4 compares clone costs with borrow/move, and R5 requires the address-stability and trusted-callback limitations. The method implements X-T69-005-DELAYED's support and assessor control, not a learner's independent delayed attempt. Failed callback side effects are retained; successful callback output must be distinct owned storage by contract. Cloning arbitrary over-aligned or self-referential byte encodings remains prohibited.

## Required rubric transfer implementation

Implement ds_capture_checked and explicitly reject self-referential/arbitrary typed captures, over-aligned storage requests and borrowed-reference returns before allocation or ownership transfer. Accepted int32 encoding delegates to the actual previous capture/DSmall/T66 implementation. Show a typed self-reference copy still points into the original while both objects remain live, then repair it with an explicit typed relocation; never invoke it after its owner ends. Compare a genuine32-byte-aligned type to the admitted scalar schema. Compile the scalar-thunk positive control and incompatible borrowed-pointer-return negative control. Explain why the restricted interface cannot promise an arbitrary relocator or returned-reference lifetime. The two admission mutants accept a disallowed kind or borrowed result; they are rejected by state assertions before any invalid typed access.

The supplied original fixtures, incomplete learner source, shared contract and separate assessor analysis define D1 through D4. Use the newly mapped named tests and independent oracle. Hints: identify the missing relation or lifetime first; keep the concrete owner visible; publish only after the stated evidence is available. Public author results do not grade an independent learner or remove the delayed attempt.

## Objective-specific assessment anchors

### X-T69-005-R1 / T69.05.a

Derive erased-call representation, invocation thunk, ownership and empty states; distinguish byte encoding from placing an arbitrary typed object in character storage.

- 0: Absent or contradicts the stated representation/contract.
- 1: Runs the nominal example but cannot derive the relevant invariant or explain a failing boundary.
- 2: Provides the complete stated result, a justified invariant and independently checked falsification evidence.
- 3: Transfers the argument to the changed assumptions without a worked solution, identifying the revised costs and limits.

### X-T69-005-R2 / T69.05.b

Implement capture construction, explicit movement and no-fail destruction through dod.small_buffer with no duplicated allocator logic, including a spill and failure rollback.

- 0: Absent or contradicts the stated representation/contract.
- 1: Runs the nominal example but cannot derive the relevant invariant or explain a failing boundary.
- 2: Provides the complete stated result, a justified invariant and independently checked falsification evidence.
- 3: Transfers the argument to the changed assumptions without a worked solution, identifying the revised costs and limits.

### X-T69-005-R3 / T69.05.c

Inject each allocation/codec failure, test source/destination/self move and repeated drop, and verify exactly-once external destruction plus unchanged outputs on failing calls.

- 0: Absent or contradicts the stated representation/contract.
- 1: Runs the nominal example but cannot derive the relevant invariant or explain a failing boundary.
- 2: Provides the complete stated result, a justified invariant and independently checked falsification evidence.
- 3: Transfers the argument to the changed assumptions without a worked solution, identifying the revised costs and limits.

### X-T69-005-R4 / T69.05.d

Compare owning C erasure, a borrowed function reference and the documented C++ move_only_function/function_ref contracts. Analyze inline storage, movement constraints and empty invocation without claiming common ABI or universal small-buffer capacity.

- 0: Absent or contradicts the stated representation/contract.
- 1: Runs the nominal example but cannot derive the relevant invariant or explain a failing boundary.
- 2: Provides the complete stated result, a justified invariant and independently checked falsification evidence.
- 3: Transfers the argument to the changed assumptions without a worked solution, identifying the revised costs and limits.

### X-T69-005-R5 / T69.05.e

Transfer to a self-referential or over-aligned capture and a callback returning a borrowed reference. Either provide a justified typed representation/relocator/lifetime protocol or reject it explicitly; a byte copy is not a general solution.

- 0: Absent or contradicts the stated representation/contract.
- 1: Runs the nominal example but cannot derive the relevant invariant or explain a failing boundary.
- 2: Provides the complete stated result, a justified invariant and independently checked falsification evidence.
- 3: Transfers the argument to the changed assumptions without a worked solution, identifying the revised costs and limits.

## Traceability and contract selection

Project: PR-T69-05

Required earlier exercises: X-T70-002, X-T69-004, X-T66-009

Required baseline units: none

Required artifact imports: dod.small_buffer, memory.pool, memory.adversarial_allocator


- [R-MOD-DISPATCH-MOVE](https://eel.is/c++draft/func.wrap.move): func.wrap.move.class paragraphs1–2: wrapper and small-object recommendation; func.wrap.move.ctor paragraphs1–6: call constraints and moved-from state
- [R-MOD-DISPATCH-REF](https://eel.is/c++draft/func.wrap.ref): func.wrap.ref.class paragraphs1–3: thunk and bound entity; func.wrap.ref.ctor paragraphs3–9: bound function/object construction; func.wrap.ref.inv paragraph1: invocation
- [R-MOD-DISPATCH-C-POINTERS](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf): 6.3.2.3p1–2 and7–8 Pointer conversions; 6.7.2.1p14–17 Structure/union layout and initial member; 6.5.16.1p1 and6 Assignment qualifiers and unsafe double-pointer example

Delayed transfer: wait at least 7 days after the first recorded study of this contract; select X-T69-005-DELAYED.

Contract SHA-256: ab4d260dff4617ed94d8b22549830cb840ce54f7f0ad819c31676a58508d43b6

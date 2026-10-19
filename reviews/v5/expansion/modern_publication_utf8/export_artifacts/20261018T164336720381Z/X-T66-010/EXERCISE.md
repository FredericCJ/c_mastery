# X-T66-010: A checked façade around a trusted core: bounded tenant-isolation failure

## Prerequisites

Complete the earlier domain projects and retain the named prerequisite artifacts with their contracts, ownership ledger and tests. Required learner artifacts: memory.pool, containers.array, testing.faults, bytes.checked, io.async, errors.result. Do not import an assessor implementation.

## Environment

Implement mechanisms locally using declared typed storage/callbacks; hosted harness and diagnostic runtimes are separate. See laboratory contract for exact dependencies. Use C17 with extensions disabled and warnings treated as errors. A diagnostic extension is identified explicitly and does not become an ISO guarantee. The full platform, pointer, callback and concurrency preconditions are in [ownership/CONTRACTS.md](../../../labs/ownership/CONTRACTS.md).

## Problem

Inspect the supplied intentionally flawed ow_flawed_write on OwTenantArena, one actual int32_t[16] divided into two logical eight-element tenants. The attacker controls tenant0 offset,count,value, with count<=8; unsigned addition is checked after wrapping, and accesses are masked into the actual16-element array. Construct a defined execution that writes tenant1 despite tenant0 authority. Implement ow_checked_write using tenant<=1, offset<=8 and count<=8-offset before any trusted-core indexing. Invalid calls leave all16 elements unchanged; offset8,count0 is valid. The trusted core may assume a validated in-range descriptor, but no unchecked descriptor crosses the public boundary.

Concrete inputs and expected results: With 32-bit unsigned, offset=UINT_MAX-3,count8,value7 passes the flawed wrapped check and changes tenant1 indices12..15 from99 to7. The repaired API returns INVALID and leaves both tenants unchanged. This is a bounded integrity-property violation in original teaching code, not an out-of-bounds C access.

Supplied artifacts: labs/ownership/include/ownership.h, compiling unsolved src/ownership.c, CTest ownership.facade, fixtures/ownership/PR-T66-10.json, and the shared contracts. State every additional backend assumption before running it.

## Necessity

Derive the integer-wrap admission defect, attacker-controlled transition and exact violated authority property; distinguish a checked trusted-core boundary from whole-process memory safety. The misconception and changed-condition case below are separately assessed; ordinary successful execution alone does not cover them.

## Reasoning

Implement the subtraction-based boundary check, unchanged-on-failure behavior and an internal validated descriptor, preserving the simple core while documenting every remaining caller and same-process assumption. Give the abstraction function, admitted states, pre/postconditions, ownership graph, failure effects and complexity before implementation. Identify the enforcement boundary of every safety claim using the shared modern-cost contract. Distinguish an invalid caller object from an operational failure admitted by the API.

## Deliverables

- D1: Derive the integer-wrap admission defect, attacker-controlled transition and exact violated authority property; distinguish a checked trusted-core boundary from whole-process memory safety.
- D2: Implement the subtraction-based boundary check, unchanged-on-failure behavior and an internal validated descriptor, preserving the simple core while documenting every remaining caller and same-process assumption.
- D3: Run the local cross-tenant demonstration with synthetic canaries, compare flawed/repaired outcomes, enumerate all ordinary tenant/offset/count combinations around8 and test UINT_MAX wrap cases. Reject the flawed-check mutant by a normal semantic assertion. Explain why ASan may remain quiet: every actual array access is in bounds. Test mitigation regression without claiming arbitrary code execution or privilege escalation.
- D4: Move the descriptor across an FFI or serialized boundary: reject fabricated fields and revalidate at that boundary rather than trusting a C typedef. Delayed assessment separates read from write capabilities and allows a read-only subview; derive authority attenuation and test that subview construction cannot widen rights. Complete all thirteen cost/enforcement dimensions with evidence or a mechanism-specific non-applicability.

## Verification

Run the local cross-tenant demonstration with synthetic canaries, compare flawed/repaired outcomes, enumerate all ordinary tenant/offset/count combinations around8 and test UINT_MAX wrap cases. Reject the flawed-check mutant by a normal semantic assertion. Explain why ASan may remain quiet: every actual array access is in bounds. Test mitigation regression without claiming arbitrary code execution or privilege escalation. Preserve predictions before execution, exact commands, source/input/toolchain identity and observed outputs. Explain independent-oracle assumptions and minimize disagreements. A supplied anchor or compiled mutant checks only its named property. Failure-side effects, ownership accounting and the required changed-condition result are pass/fail gates; finite tests cannot establish absolute mastery.

## Rubric

The metadata rubric maps each leaf to a distinct D1–D4 criterion. Require every criterion at level2 and at least two at level3 on independently attempted cases; record assistance. A violated ownership/failure contract, unsupported guarantee or incorrect exploitability claim fails regardless of performance. Passing only the public suite earns at most level1. Apply the shared six-dimension rubric as well.

## Hints

1. Unsigned wrap is defined; the authority check can still be wrong.
2. Compare count against remaining extent before adding offsets.
3. A private type name does not stop hostile code with arbitrary writes in the same address space.

## Variants

Move the descriptor across an FFI or serialized boundary: reject fabricated fields and revalidate at that boundary rather than trusting a C typedef. Delayed assessment separates read from write capabilities and allows a read-only subview; derive authority attenuation and test that subview construction cannot widen rights.

Delayed assessment X-T66-010-DELAYED: use the separately held assessor variation at least seven days after assisted work, without the previous implementation. Record the new invariant and a counterexample to the old proof before coding.

## Security

Target only original ow_flawed_write/ow_checked_write in the local hosted laboratory with a synthetic16-element array. The attacker controls numeric fields for tenant0, has no arbitrary pointer primitive, privileges or external target. The defect is a wrapped extent check; the observable harmless success condition is changing tenant1 canary99 to7. The demonstration requires the declared unsigned width/masking model and does not demonstrate code execution, escalation or an ISO out-of-bounds access. Repair before trusted indexing, retain wrap/boundary regressions, and document that same-process arbitrary memory access or forged native pointers are outside this façade guarantee.

## Validation

Authored; actual executions come from a matching source-bound family campaign. Independent review and learner assessment have not occurred. Host state models and tests do not establish physical or privileged target behavior. The full independent investigation and delayed variant remain required even after public tests pass.
## Required authority-boundary investigation

[AUTHORITY.md](../../../labs/ownership/authority_boundary/AUTHORITY.md) defines a sixteen-octet foreign request, trusted out-of-band parent, nested read-only subviews, staged publication and nonwrapping epoch retirement. The original wrapped-check target is imported unchanged. Native and serialized requests share the private validated core; six anchors and eight compiled controls distinguish logical authorization from physical array bounds. Original R1-R4 and X-T66-010-DELAYED remain separate assessments.

## Objective-specific assessment anchors

### X-T66-010-R1 / T66.10.a

Derive the integer-wrap admission defect, attacker-controlled transition and exact violated authority property; distinguish a checked trusted-core boundary from whole-process memory safety.

- 0: Missing or contradicted by the stated contract.
- 1: Nominal test only; fails to explain the listed mechanism or boundary.
- 2: Meets this complete criterion with justified assumptions and independent evidence.
- 3: Meets it under the unseen changed-condition variant and explains a counterexample to the former assumptions.

### X-T66-010-R2 / T66.10.b

Implement the subtraction-based boundary check, unchanged-on-failure behavior and an internal validated descriptor, preserving the simple core while documenting every remaining caller and same-process assumption.

- 0: Missing or contradicted by the stated contract.
- 1: Nominal test only; fails to explain the listed mechanism or boundary.
- 2: Meets this complete criterion with justified assumptions and independent evidence.
- 3: Meets it under the unseen changed-condition variant and explains a counterexample to the former assumptions.

### X-T66-010-R3 / T66.10.c

Run the local cross-tenant demonstration with synthetic canaries, compare flawed/repaired outcomes, enumerate all ordinary tenant/offset/count combinations around8 and test UINT_MAX wrap cases. Reject the flawed-check mutant by a normal semantic assertion. Explain why ASan may remain quiet: every actual array access is in bounds. Test mitigation regression without claiming arbitrary code execution or privilege escalation.

- 0: Missing or contradicted by the stated contract.
- 1: Nominal test only; fails to explain the listed mechanism or boundary.
- 2: Meets this complete criterion with justified assumptions and independent evidence.
- 3: Meets it under the unseen changed-condition variant and explains a counterexample to the former assumptions.

### X-T66-010-R4 / T66.10.d

Move the descriptor across an FFI or serialized boundary: reject fabricated fields and revalidate at that boundary rather than trusting a C typedef. Delayed assessment separates read from write capabilities and allows a read-only subview; derive authority attenuation and test that subview construction cannot widen rights.

- 0: Missing or contradicted by the stated contract.
- 1: Nominal test only; fails to explain the listed mechanism or boundary.
- 2: Meets this complete criterion with justified assumptions and independent evidence.
- 3: Meets it under the unseen changed-condition variant and explains a counterexample to the former assumptions.

## Traceability and contract selection

Project: PR-T66-10

Required earlier exercises: X-T66-009

Required baseline units: none

Required artifact imports: memory.pool, containers.array, testing.faults, bytes.checked, io.async, errors.result

- T66.10.a: Derive the integer-wrap admission defect, attacker-controlled transition and exact violated authority property; distinguish a checked trusted-core boundary from whole-process memory safety. Scope: Authored assessment obligations, not an assertion that a learner has demonstrated them.
- T66.10.b: Implement the subtraction-based boundary check, unchanged-on-failure behavior and an internal validated descriptor, preserving the simple core while documenting every remaining caller and same-process assumption. Scope: Authored assessment obligations, not an assertion that a learner has demonstrated them.
- T66.10.c: Run the local cross-tenant demonstration with synthetic canaries, compare flawed/repaired outcomes, enumerate all ordinary tenant/offset/count combinations around8 and test UINT_MAX wrap cases. Reject the flawed-check mutant by a normal semantic assertion. Explain why ASan may remain quiet: every actual array access is in bounds. Test mitigation regression without claiming arbitrary code execution or privilege escalation. Scope: Authored assessment obligations, not an assertion that a learner has demonstrated them.
- T66.10.d: Move the descriptor across an FFI or serialized boundary: reject fabricated fields and revalidate at that boundary rather than trusting a C typedef. Delayed assessment separates read from write capabilities and allows a read-only subview; derive authority attenuation and test that subview construction cannot widen rights. Scope: Authored assessment obligations, not an assertion that a learner has demonstrated them.

- [R-MOD-REFCELL](https://doc.rust-lang.org/std/cell/struct.RefCell.html): borrow and try_borrow; borrow_mut and try_borrow_mut; replace; get_mut; try_borrow_unguarded
- [R-B01-N1570-OBJECT-ARITH](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf): 6.2.5p9 Unsigned arithmetic

Delayed transfer: wait at least 7 days after the first recorded study of this contract; select X-T66-010-DELAYED.

Contract SHA-256: 0c7a6308a7fe6e2607428c9a425bf5d704b8d8d762f213f4e91bdb94605309bb

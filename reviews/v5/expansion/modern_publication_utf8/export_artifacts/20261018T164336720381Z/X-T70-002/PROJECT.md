```json
{
  "id": "PR-T70-02",
  "title": "Inline bytes, allocator failure and explicit relocation",
  "module": "T70",
  "topics": [
    "T70.02"
  ],
  "status": "core",
  "hours": 8,
  "profile": "C",
  "fixtures": [
    "fixtures/dod/PR-T70-02.json"
  ],
  "edition": 5,
  "requires_projects": [
    "PR-T18-01"
  ],
  "evidence_template": "component"
}
```

# PR-T70-02: Inline bytes, allocator failure and explicit relocation

## Contract

Implement the DSmall operations: init, append, truncate, compact, move, reset and borrowed data access. Inline capacity16 and maximum128 are part of the shared contract. Supply a fixed-capacity, typed byte-array allocator adapter that records requests and fails each requested allocation in turn; a host malloc adapter is an additional test only. Construct a separate fixed-capacity16-byte variant that always rejects spill. Compare its contract and owner size with DSmall. No string terminator, generic typed placement, automatic copy constructor or raw-struct move is supplied.

The full required exercise is {ref}`X-T70-002`; its shared contracts, D1–D4, objective-specific rubric and delayed variant are part of this project. Eight hours is descriptive accounting, not a ceiling on required work.

## Inputs and expected results

Appending16 bytes performs no allocation. A failed17th byte preserves length16 and all bytes. Successful spill followed by move leaves the source empty and transfers exactly one allocation. Truncating to8 and compacting frees that allocation once; resetting both owners performs no second free.

## Steps

- **T70.02:** Implement all capacity transitions and a fixed-capacity alternative using allocator injection, with strong append rejection and exactly-once resource release under move/reset. Complete every exercise deliverable, including separately specified extensions.

## Failure cases

Small-buffer optimization introduces two ownership regimes and an often-hidden object-type problem. An address satisfying alignment is not proof that a declared unsigned-char array may be accessed through any desired struct pointer. Run an operation-model generator over append lengths0,1,15,16,17,127,128,129; truncate, compact, move and reset. Preserve operation traces and independent byte-list state. Inject allocator failure at every spill and verify live allocation ownership, exact release counts and output state separately from callback instrumentation. Compile without a container/string library. Record sizeof/_Alignof on each available target. The header intentionally exposes fields for teaching, so forged state is outside the checked API contract and must be discussed.

## Acceptance and evidence

- **T70.02:** Run an operation-model generator over append lengths0,1,15,16,17,127,128,129; truncate, compact, move and reset. Preserve operation traces and independent byte-list state. Inject allocator failure at every spill and verify live allocation ownership, exact release counts and output state separately from callback instrumentation. Compile without a container/string library. Record sizeof/_Alignof on each available target. The header intentionally exposes fields for teaching, so forged state is outside the checked API contract and must be discussed. Assess each leaf independently with the exercise rubric and retain unresolved evidence as a gap.

## Hints and recovery

Do not store a pointer that points into the owner itself. Allocate and copy before releasing or publishing. Represent typed captures in actual typed storage or encode bytes and reconstruct a typed local in a thunk.

## Transfer challenge

X-T70-002-DELAYED: inline capacity becomes0 and the allocator has a128-byte total quota shared by two buffers. Implement move and rollback when one buffer spills during composition; demonstrate that allocator rollback and byte-state rollback are different properties.

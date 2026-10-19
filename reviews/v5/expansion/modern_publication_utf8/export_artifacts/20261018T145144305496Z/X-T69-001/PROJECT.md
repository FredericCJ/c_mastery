```json
{
  "id": "PR-T69-01",
  "title": "Named interfaces, structural probes and semantic conformance",
  "module": "T69",
  "topics": [
    "T69.01"
  ],
  "status": "core",
  "hours": 8,
  "profile": "C",
  "fixtures": [
    "fixtures/dispatch/PR-T69-01.json"
  ],
  "edition": 5,
  "requires_projects": [
    "PR-T18-01"
  ],
  "evidence_template": "component"
}
```

# PR-T69-01: Named interfaces, structural probes and semantic conformance

## Contract

Implement DSView/DSOps and ds_measure for a borrowed int implementation and a second concrete counted-byte implementation that measures logical length. Every successful callback publishes one int; the wrapper calls exactly once and preserves caller output on failure. Build a nominal typed constructor for each implementation and a C17 _Generic structural probe that accepts only explicitly listed receiver types. Supply three rejected translations: wrong callback signature, an unlisted receiver, and incompatible named wrapper types. A second implementation with the correct signature but an incorrect length result must compile and fail semantic tests.

The full required exercise is {ref}`X-T69-001`; its shared contracts, D1–D4, objective-specific rubric and delayed variant are part of this project. Eight hours is descriptive accounting, not a ceiling on required work.

## Inputs and expected results

A borrowed int7 measures7. A callback returning DS_FAILED after writing123 into its private output leaves caller output99 unchanged. A missing table rejects before invocation. The wrong-return-type callback fixture must fail strict C17 compilation.

## Steps

- **T69.01:** Implement typed view constructors and a checked erased call wrapper with exactly-once callback invocation, stable table lifetime and failure-output staging. Complete every exercise deliverable, including separately specified extensions.

## Failure cases

An operation table can validate the presence of a function pointer but cannot infer that the pointer uses the same concrete representation as its object or satisfies the promised law. Enumerate int values-8..8 and counted spans0..16; instrument callback count and outputs. Compile every diagnostic fixture separately under GCC/Clang -std=c17 -Wpedantic -Werror and retain the intended diagnostic. Wrong semantic callbacks must compile and be rejected by behavior/invariant checks, not by a cast or a runtime type-name string. Never execute a deliberately incompatible function-pointer call.

## Acceptance and evidence

- **T69.01:** Enumerate int values-8..8 and counted spans0..16; instrument callback count and outputs. Compile every diagnostic fixture separately under GCC/Clang -std=c17 -Wpedantic -Werror and retain the intended diagnostic. Wrong semantic callbacks must compile and be rejected by behavior/invariant checks, not by a cast or a runtime type-name string. Never execute a deliberately incompatible function-pointer call. Assess each leaf independently with the exercise rubric and retain unresolved evidence as a gap.

## Hints and recovery

Separate type compatibility from the relation between object and table. Create a typed constructor before erasing the receiver pointer. A temporary output confines a failed callback write, but not arbitrary callback side effects.

## Transfer challenge

X-T69-001-DELAYED: add an interface whose read method may make partial progress. Preserve both progress and status and show why the original single-output success/failure wrapper cannot express it.

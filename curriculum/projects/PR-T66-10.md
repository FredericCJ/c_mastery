```json
{
  "id": "PR-T66-10",
  "title": "A checked façade around a trusted core: bounded tenant-isolation failure",
  "module": "T66",
  "topics": [
    "T66.10"
  ],
  "status": "core",
  "hours": 8,
  "profile": "A",
  "fixtures": [
    "fixtures/ownership/PR-T66-10.json",
    "fixtures/ownership/cases.json"
  ],
  "edition": 5,
  "requires_projects": [
    "PR-T66-09"
  ],
  "evidence_template": "component"
}
```

# PR-T66-10: A checked façade around a trusted core: bounded tenant-isolation failure

## Contract

Inspect the supplied intentionally flawed ow_flawed_write on OwTenantArena, one actual int32_t[16] divided into two logical eight-element tenants. The attacker controls tenant0 offset,count,value, with count<=8; unsigned addition is checked after wrapping, and accesses are masked into the actual16-element array. Construct a defined execution that writes tenant1 despite tenant0 authority. Implement ow_checked_write using tenant<=1, offset<=8 and count<=8-offset before any trusted-core indexing. Invalid calls leave all16 elements unchanged; offset8,count0 is valid. The trusted core may assume a validated in-range record, but no unchecked range record crosses the public boundary.

The full required exercise {ref}`X-T66-010`, its shared contracts and objective rubric are part of this project. Eight hours is descriptive author accounting, not a learner-time or scope ceiling.

## Inputs and expected results

With 32-bit unsigned, offset=UINT_MAX-3,count8,value7 passes the flawed wrapped check and changes tenant1 indices12..15 from99 to7. The repaired API returns `INVALID` and leaves both tenants unchanged. This is a bounded integrity-property violation in original teaching code, not an out-of-bounds C access.

## Steps

- **T66.10:** Implement the subtraction-based boundary check, unchanged-on-failure behavior and an internal validated range record, preserving the simple core while documenting every remaining caller and same-process assumption. Submit the exercise D1–D4 and retain prerequisite interfaces.

## Failure cases

Run the local cross-tenant demonstration with synthetic canaries, compare flawed/repaired outcomes, enumerate all ordinary tenant/offset/count combinations around8 and test UINT_MAX wrap cases. Reject the flawed-check mutant by a normal semantic assertion. Explain why ASan may remain quiet: every actual array access is in bounds. Test mitigation regression without claiming arbitrary code execution or privilege escalation.

## Acceptance and evidence

- **T66.10:** Derive the integer-wrap admission defect, attacker-controlled transition and exact violated authority property; distinguish a checked trusted-core boundary from whole-process memory safety. Every leaf has a separate rubric criterion in X-T66-010. Public test passage alone does not assess independent understanding.

## Hints and recovery

Unsigned wrap is defined; the authority check can still be wrong. Then Compare count against remaining extent before adding offsets. Record the failed hypothesis before using further hints.

## Transfer challenge

Move the validated range record across a foreign-function interface or serialized boundary: reject fabricated fields and revalidate at that boundary rather than trusting a C typedef. Delayed assessment separates read from write capabilities and allows a read-only subview; derive authority attenuation and test that subview construction cannot widen rights.

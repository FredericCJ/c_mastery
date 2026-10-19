```json
{
  "id": "PR-T70-05",
  "title": "Archetype migration with capacity admission and row repair",
  "module": "T70",
  "topics": [
    "T70.05"
  ],
  "status": "core",
  "hours": 8,
  "profile": "C",
  "fixtures": [
    "fixtures/dod/PR-T70-05.json"
  ],
  "edition": 5,
  "requires_projects": [
    "PR-T18-01"
  ],
  "evidence_template": "component"
}
```

# PR-T70-05: Archetype migration with capacity admission and row repair

## Contract

Implement DEcs with4 entity IDs, component masks1(position),2(velocity),3(both), and2 rows per archetype. Set creates, replaces or migrates an entity; step updates only the both-component table. Build a query adapter for required/excluded masks and a structural version counter for cached table selection. Specify deletion/reuse as a separate generation-bearing extension, never silently reuse an ID while old views survive. All structural edits happen outside an active iteration borrow.

The full required exercise is {ref}`X-T70-005`; its shared contracts, D1–D4, objective-specific rubric and delayed variant are part of this project. Eight hours is descriptive accounting, not a ceiling on required work.

## Inputs and expected results

Create entity0/1 in mask1 and entity2 in mask3. Moving0 into mask3 succeeds and moves1 to row0 of mask1. Moving1 into now-full mask3 rejects without deleting it. Step changes both-component positions30,10 with velocities2,1 into32,11; a later overflow preserves the whole table.

## Steps

- **T70.05:** Implement migration with destination admission before source removal, swap-row repair and transactional query updates; implement required/excluded-mask query selection and versioned cache invalidation. Complete every exercise deliverable, including separately specified extensions.

## Failure cases

An archetype label does not teach migration atomicity or query invalidation. Deleting the old row before discovering a full destination loses an entity even though the numerical component code is correct. Enumerate all legal entity/mask requests for bounded traces up to length6, compare masks and active values with a dictionary oracle and check both mapping directions after every step. Explicitly test self-migration, last-row removal, destination full and overflow at each visited row. For cache tests, distinguish a changed set of matching tables from changed rows within an existing table. Benchmark equal query results with a scalar dictionary and include structural edits in the workload.

## Acceptance and evidence

- **T70.05:** Enumerate all legal entity/mask requests for bounded traces up to length6, compare masks and active values with a dictionary oracle and check both mapping directions after every step. Explicitly test self-migration, last-row removal, destination full and overflow at each visited row. For cache tests, distinguish a changed set of matching tables from changed rows within an existing table. Benchmark equal query results with a scalar dictionary and include structural edits in the workload. Assess each leaf independently with the exercise rubric and retain unresolved evidence as a gap.

## Hints and recovery

Admission is separate from commit. Source swap-removal changes a different entity mapping. A cached table match does not grant a live borrow of its old row storage.

## Transfer challenge

X-T70-005-DELAYED: entities can be deleted and IDs reused. Integrate DSparse-style generations, then deliver a deferred migration command list whose capacity failure leaves the currently iterated tables unchanged.

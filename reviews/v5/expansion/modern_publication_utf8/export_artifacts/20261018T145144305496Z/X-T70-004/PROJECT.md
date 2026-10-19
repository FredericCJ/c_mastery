```json
{
  "id": "PR-T70-04",
  "title": "Stable typed slots, dense iteration and finite identity",
  "module": "T70",
  "topics": [
    "T70.04"
  ],
  "status": "core",
  "hours": 8,
  "profile": "C",
  "fixtures": [
    "fixtures/dod/PR-T70-04.json"
  ],
  "edition": 5,
  "requires_projects": [
    "PR-T18-01"
  ],
  "evidence_template": "component"
}
```

# PR-T70-04: Stable typed slots, dense iteration and finite identity

## Contract

Implement DSparse add/get/remove with4 typed slots, a dense ID list, rank inverse, owner-bound handles and generations1–3 followed by retirement. Derive a segmented extension with2 pages of4 slots allocated through a caller-supplied page table: page addresses remain stable while the table can move. Give the extension a complete acquire/release and exhaustion contract before implementing it. Contrast a pointer to a stable live slot with a handle that must be revalidated after deletion.

The full required exercise is {ref}`X-T70-004`; its shared contracts, D1–D4, objective-specific rubric and delayed variant are part of this project. Eight hours is descriptive accounting, not a ceiling on required work.

## Inputs and expected results

After inserting11,22,33 and removing22, the dense list contains the slots for11 and33; rank of33 is1. The removed handle rejects and preserves output. Removing/reusing slot0 through generation3 retires it; further insertion must choose another slot or report full.

## Steps

- **T70.04:** Implement swap-erased dense IDs with typed slot storage, generation admission and retirement; implement a two-page extension without moving live payload objects. Complete every exercise deliverable, including separately specified extensions.

## Failure cases

Dense iteration and stable addresses can coexist only by choosing what moves. Many sparse-set bugs repair one mapping but forget the inverse; generation counters eventually wrap unless exhaustion is part of the contract. Validate every live dense index round-trips through rank and every unoccupied slot is absent. Enumerate bounded operations with independent logical IDs and retain every stale handle. Inject page allocation failure before publication; verify existing page addresses and values. Record holes, page metadata, indirection, iteration traffic and retired capacity. The public4-slot anchor does not validate the submitted segmented extension, so attach its separate oracle and failure traces.

## Acceptance and evidence

- **T70.04:** Validate every live dense index round-trips through rank and every unoccupied slot is absent. Enumerate bounded operations with independent logical IDs and retain every stale handle. Inject page allocation failure before publication; verify existing page addresses and values. Record holes, page metadata, indirection, iteration traffic and retired capacity. The public4-slot anchor does not validate the submitted segmented extension, so attach its separate oracle and failure traces. Assess each leaf independently with the exercise rubric and retain unresolved evidence as a gap.

## Hints and recovery

Move the ID in the dense list, not the typed slot. Repair rank for the entity moved into the removed row, including removal of the last row. Generation exhaustion is a state with a defined policy; an integer width alone is not a proof that it never happens.

## Transfer challenge

X-T70-004-DELAYED: preserve insertion iteration order while retaining stable payload addresses and O(1) handle lookup. Derive a tombstone/linked-order alternative and measure the new deletion or iteration costs.

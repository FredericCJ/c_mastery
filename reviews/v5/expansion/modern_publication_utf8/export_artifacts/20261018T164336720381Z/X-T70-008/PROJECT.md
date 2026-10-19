```json
{
  "id": "PR-T70-08",
  "title": "Shared text DAGs, editing alternatives and coordinate meaning",
  "module": "T70",
  "topics": [
    "T70.08"
  ],
  "status": "core",
  "hours": 8,
  "profile": "C",
  "fixtures": [
    "fixtures/dod/PR-T70-08.json"
  ],
  "edition": 5,
  "requires_projects": [
    "PR-T18-01"
  ],
  "evidence_template": "component"
}
```

# PR-T70-08: Shared text DAGs, editing alternatives and coordinate meaning

## Contract

Implement the DRope leaf/concat/flatten anchor with64 typed nodes, leaf length0–8 and maximum expanded length128. A rope is an ordered shared directed acyclic graph: concatenating a root with itself must duplicate its logical bytes without mutating the shared leaf. Extend it with byte-index lookup and split, specifying allocation rollback and balancing separately. Implement two equal-byte-contract editing alternatives for a128-byte document: a gap buffer with explicit gap bounds and a piece table whose immutable original/add buffers are caller-owned and whose pieces are offset/length triples. Apply the same edit scripts to all three and a flat byte-list oracle. No Unicode scalar, grapheme or line-number equivalence is implied by byte offsets.

The full required exercise is {ref}`X-T70-008`; its shared contracts, D1–D4, objective-specific rubric and delayed variant are part of this project. Eight hours is descriptive accounting, not a ceiling on required work.

## Inputs and expected results

Leaves ab and cd concatenate to abcd; concatenating that root with itself flattens to abcdabcd while the ab leaf remains ab. Capacity7 rejects an8-byte flatten without touching the output canary or written. An empty leaf returns root0.

## Steps

- **T70.08:** Implement transactional leaf/concat/flatten and byte-index/split operations, including empty boundaries and shared subgraphs without cycles. Complete every exercise deliverable, including separately specified extensions.

## Failure cases

Ropes trade copying for metadata, traversal and retained sharing. Text-editing structures are not interchangeable until coordinate system, ownership, capacity, snapshot and failure contracts agree. Generate edit traces length0–5 over insert empty/a/ab/NUL, delete zero/one/all and each boundary position, comparing byte sequences after every step. Check invalid positions and capacity exhaustion preserve state. Independently traverse the directed acyclic graph to verify cached lengths, older-node edges and sharing; flatten must visit logical occurrences, not each unique node once. Construct left-deep concatenation to measure depth and compare balanced rebuild. The author anchor covers concat/flatten; split, balance, gap and piece implementations require their own submitted evidence and are not counted executed by that anchor.

## Acceptance and evidence

- **T70.08:** Generate edit traces length0–5 over insert empty/a/ab/NUL, delete zero/one/all and each boundary position, comparing byte sequences after every step. Check invalid positions and capacity exhaustion preserve state. Independently traverse the directed acyclic graph to verify cached lengths, older-node edges and sharing; flatten must visit logical occurrences, not each unique node once. Construct left-deep concatenation to measure depth and compare balanced rebuild. The author anchor covers concat/flatten; split, balance, gap and piece implementations require their own submitted evidence and are not counted executed by that anchor. Assess each leaf independently with the exercise rubric and retain unresolved evidence as a gap.

## Hints and recovery

A node may appear twice in the logical sequence even though it is stored once. Use subtree length to choose the branch for byte lookup; a split can share untouched subtrees. A gap is unused capacity, while a piece references immutable backing bytes. Neither fact establishes a Unicode index.

## Transfer challenge

X-T70-008-DELAYED: preserve immutable snapshots while compacting piece-table backing storage under a fixed additional-memory budget. State whether compaction may fail, how old snapshots remain valid and which reclamation event releases the previous backing store.

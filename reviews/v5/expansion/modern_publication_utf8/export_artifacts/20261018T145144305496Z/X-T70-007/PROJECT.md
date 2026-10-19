```json
{
  "id": "PR-T70-07",
  "title": "Bitmap rank, full-hash collisions and persistent map updates",
  "module": "T70",
  "topics": [
    "T70.07"
  ],
  "status": "core",
  "hours": 8,
  "profile": "C",
  "fixtures": [
    "fixtures/dod/PR-T70-07.json"
  ],
  "edition": 5,
  "requires_projects": [
    "PR-T18-01"
  ],
  "evidence_template": "component"
}
```

# PR-T70-07: Bitmap rank, full-hash collisions and persistent map updates

## Contract

Implement DHamt for keys0–31 with hash(key)=key&15, two2-bit levels, compact bitmap-indexed children, and equality-checked two-entry terminal collision buckets. Empty root0 and64 typed nodes bound the model. Update copies leaf and ancestors; lookup returns D_STALE for absence. Add a deletion operation with an explicit policy for empty internal nodes, and a tree-to-flat-map oracle. Do not use hash equality as key equality. Discuss extension to arbitrary-length keys by defining hash/equality contracts and ownership before substituting a hash function.

The full required exercise is {ref}`X-T70-007`; its shared contracts, D1–D4, objective-specific rubric and delayed variant are part of this project. Eight hours is descriptive accounting, not a ceiling on required work.

## Inputs and expected results

Keys1 and17 deliberately share the full4-bit hash but retain values10 and20. Updating17 to88 in a new root leaves20 in the old root. Inserting key0 changes bitmap rank and must not make key1 unreachable. Each of3 allocation failure points preserves old snapshots.

## Steps

- **T70.07:** Implement lookup/update with collision buckets and path copying, then deletion with defined empty-node normalization and immutable prior snapshots. Complete every exercise deliverable, including separately specified extensions.

## Failure cases

A HAMT description hides three distinct equivalences: shared prefix, identical hash and equal key. Confusing them overwrites unrelated values. Sparse child packing also makes insertion rank as important as lookup rank. Generate every key pair and both insertion orders, then update and delete either member while retaining all roots. Add triplets selecting low chunks0,1,3 and full-hash pair0/16; compare flattening to sorted key/value pairs from a dictionary. For every node recompute count and rank independently; test pool exhaustion. Public reference implements lookup/update only: submitted deletion and its failure/normalization evidence are separately required and remain unexecuted in the author campaign.

## Acceptance and evidence

- **T70.07:** Generate every key pair and both insertion orders, then update and delete either member while retaining all roots. Add triplets selecting low chunks0,1,3 and full-hash pair0/16; compare flattening to sorted key/value pairs from a dictionary. For every node recompute count and rank independently; test pool exhaustion. Public reference implements lookup/update only: submitted deletion and its failure/normalization evidence are separately required and remain unexecuted in the author campaign. Assess each leaf independently with the exercise rubric and retain unresolved evidence as a gap.

## Hints and recovery

The mask is a set of occupied logical slots, not a dense array index. Full-hash collision still requires key comparison. Deletion can leave an empty branch or normalize it, but the contract and cost model must choose explicitly.

## Transfer challenge

X-T70-007-DELAYED: use two-byte keys and a caller-supplied hash/equality dictionary. Construct unequal keys with equal hash and equal logical keys with different backing bytes; define canonical equality and ownership, then re-evaluate collision and failure costs.

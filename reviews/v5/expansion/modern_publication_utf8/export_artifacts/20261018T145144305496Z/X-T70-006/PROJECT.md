```json
{
  "id": "PR-T70-06",
  "title": "Path copying, zipper contexts and retained-version costs",
  "module": "T70",
  "topics": [
    "T70.06"
  ],
  "status": "core",
  "hours": 8,
  "profile": "C",
  "fixtures": [
    "fixtures/dod/PR-T70-06.json"
  ],
  "edition": 5,
  "requires_projects": [
    "PR-T18-01"
  ],
  "evidence_template": "component"
}
```

# PR-T70-06: Path copying, zipper contexts and retained-version costs

## Contract

Implement the8-cell persistent binary trie using a64-node typed arena. Root0 denotes all zeros. An update clones one leaf and three ancestors, returning a new root; old roots remain readable. Add an explicit zipper with a focus plus three ancestor/direction crumbs and reconstruct by path copying. The zipper may borrow a live arena and immutable root, but never retain raw pointers across arena reset. Implement a reachability census over a chosen root set and compare region reset with a separately specified reference-count reclamation design, including rollback if construction fails.

The full required exercise is {ref}`X-T70-006`; its shared contracts, D1–D4, objective-specific rubric and delayed variant are part of this project. Eight hours is descriptive accounting, not a ceiling on required work.

## Inputs and expected results

Updating root0 cell3=17 creates4 nodes; updating that root cell6=29 creates4 more. The first root still reads cell6=0. Each of the four allocation-failure ordinals rejects with the output root and previously reachable nodes unchanged.

## Steps

- **T70.06:** Implement leaf-to-root path copying, an explicit zipper reconstruction and failure rollback without mutating published nodes; state allocator ownership and snapshot lifetime. Complete every exercise deliverable, including separately specified extensions.

## Failure cases

Cheap root copying shifts costs into path cloning and retained history. Const qualification does not freeze aliases, and a path-copy algorithm that silently mutates one old node breaks every older version sharing it. For update sequences of length0–5 over indices0,3,7 and values0,1,-1 compare every retained version with a copied8-element Python/list model. Traverse nodes independently to count reachability and shared edges; reject cycles or depth errors. Run each allocation failure ordinal. Test zipper root/leaf moves and reconstruction equality, including returning to the original snapshot. Report measured allocation/retention and not merely O(log n); for the bounded trie exactly4 nodes/update is independently expected.

## Acceptance and evidence

- **T70.06:** For update sequences of length0–5 over indices0,3,7 and values0,1,-1 compare every retained version with a copied8-element Python/list model. Traverse nodes independently to count reachability and shared edges; reject cycles or depth errors. Run each allocation failure ordinal. Test zipper root/leaf moves and reconstruction equality, including returning to the original snapshot. Report measured allocation/retention and not merely O(log n); for the bounded trie exactly4 nodes/update is independently expected. Assess each leaf independently with the exercise rubric and retain unresolved evidence as a gap.

## Hints and recovery

A root names a version, not the entire arena contents. Construct the changed child before its new parent and publish the root last. A zipper must remember which sibling survives at every ancestor; mutation of a crumb into the old tree defeats persistence.

## Transfer challenge

X-T70-006-DELAYED: replace region reset with incremental reclamation under an explicit work budget. Return unfinished reclamation state; prove no live root is reclaimed and report worst-case retained nodes while work is deferred.

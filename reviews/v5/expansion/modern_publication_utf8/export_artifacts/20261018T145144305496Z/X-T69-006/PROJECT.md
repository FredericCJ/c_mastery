```json
{
  "id": "PR-T69-06",
  "title": "Ordered multiple dispatch and cache invalidation epochs",
  "module": "T69",
  "topics": [
    "T69.06"
  ],
  "status": "core",
  "hours": 8,
  "profile": "C",
  "fixtures": [
    "fixtures/dispatch/PR-T69-06.json"
  ],
  "edition": 5,
  "requires_projects": [
    "PR-T18-01"
  ],
  "evidence_template": "component"
}
```

# PR-T69-06: Ordered multiple dispatch and cache invalidation epochs

## Contract

Implement a3x3 ordered binary-method table and one-entry cache. A cache is bound to one DSMethods object; its key includes ordered type pair and non-wrapping table epoch. Epoch1 is empty; each admitted registration/removal increments it and epoch3 forbids further changes. Add a two-entry cache and compare direct table lookup. Specify a separate subtype fallback graph and reject ambiguous equally specific matches instead of silently choosing registration order.

The full required exercise is {ref}`X-T69-006`; its shared contracts, D1–D4, objective-specific rubric and delayed variant are part of this project. Eight hours is descriptive accounting, not a ceiling on required work.

## Inputs and expected results

Register subtraction for(0,1): arguments10,2 produce8; reversed types(1,0) are unsupported. Replace the same entry with addition: the old cached result must be invalidated and produce12. A third registration rejects at the finite epoch limit.

## Steps

- **T69.06:** Implement registration and cached invocation preserving output on failure; add a two-entry policy with an explicit eviction rule. Complete every exercise deliverable, including separately specified extensions.

## Failure cases

Caching a method is caching a semantic decision. Ignoring registration generation or sorting an ordered pair changes behavior even when the called functions are individually correct. Use an uncached dictionary keyed by ordered pair and version as the independent oracle. Replay every sequence of up to4 registrations/calls over types0–2 and methods add/subtract/missing, respecting finite epochs. Record cache hits/misses separately from correctness. Add a new table identity while retaining cache storage to demonstrate why ownership binding matters.

## Acceptance and evidence

- **T69.06:** Use an uncached dictionary keyed by ordered pair and version as the independent oracle. Replay every sequence of up to4 registrations/calls over types0–2 and methods add/subtract/missing, respecting finite epochs. Record cache hits/misses separately from correctness. Add a new table identity while retaining cache storage to demonstrate why ownership binding matters. Assess each leaf independently with the exercise rubric and retain unresolved evidence as a gap.

## Hints and recovery

Cache a miss as carefully as a hit. The tuple(left,right) is not a set. Finite versions need exhaustion or a proof that no stale cache can survive reuse.

## Transfer challenge

X-T69-006-DELAYED: methods can be registered by a reloadable plugin. Integrate module ownership into each cache entry and prove quiescence before discarding code pointers, including cached misses after a new plugin loads.

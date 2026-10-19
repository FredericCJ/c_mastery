# T70 extension passage inspection

RFC3629 sections3 and4 were opened and read from the RFC Editor HTML on
2026-10-17. The inspected passages give one-to-four-octet encoding, scalar upper
bound, prohibited surrogate range, shortest encoding and lead/continuation
constraints. The original decoder implements these constraints arithmetically
and publishes an offset only after validating the whole input. This inspection
does not establish normalization, grapheme segmentation or display-width rules.

The existing dod_resources.json inspection records remain bounded to their
originally named collection, small-buffer, archetype and rope passages. New
original algorithms extend the prior model: generated row/column declarations,
partial three-lane tiles, two-page ownership, query cache generations, explicit
zipper crumbs, reference-counted string leaves, budgeted retirement, normalized
HAMT deletion, copying rope rebuild, gap/piece editing and snapshot-preserving
compaction. No library source implementation was substituted for the assessed
mechanisms. Rebuilding a balanced rope by flattening is explicitly a different
cost policy from incremental balancing; no paper measurements are reused as
laboratory observations.

Follow-up inspection2026-10-17: reopened actual im15.1.0 HashMap description
and with_hasher/get contracts, and Boehm/Atkinson/Plass Rebalancing printed
1319–1320 (PDF pages5–6). Existing R-MOD-DOD-HAMT and R-MOD-DOD-ROPES
registered locators cover this reuse. The rope root Fibonacci condition permits
unbalanced descendants; the paper forest algorithm has an additive finishing
depth allowance. Our midpoint-copy rebuild is not that forest algorithm. No
new performance/source-body claims are inferred from API or paper prose.

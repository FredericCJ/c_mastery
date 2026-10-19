# Cache successor technical analysis

The baseline is not a slot-only cache: cached() compares the physical page tag.
That detail makes the separate-invalidation task more subtle than its original
wording. The separate compiled omissions violate the required transition state,
but the remaining invalidation plus exact tags masks the stale value under the
selected valid API trace. The combined omission leaves the primed home page
across commit and checkpoint when no intervening WAL read replaces it. The raw
page is generation0; the current handle expects1, so real decoding returns
corruption. Preserve this distinction instead of manufacturing successful stale
data or treating an assertion of cache emptiness as a stale-value observation.

The original and successor policies have the same four-page payload budget.
Two-way LRU updates its recent bit on every successful hit/fill; on conflict it
replaces the other way. Empty ways are chosen deterministically first. The root
policy reserves one actual slot rather than adding a fifth page. Generation and
image identity remain part of the tuple even if they do not determine the set.
The read callback is an external effect: incrementing attempt count is permitted
on failure; changing output, entries or replacement state is not.

Use real store insertion/deletion to establish reuse: with20one-byte keys the
tree allocates page2, deletion collapses it back to root1/mask3, and a new20-key
set allocates page2 again. At generation3 the repeated WAL physical page must not
hit generation1. The cache does not establish that a supplied view is current;
the caller must obtain it from backend authority under serialization. This is
the counterexample to an overclaimed self-validating shared cache.

Exact cold traces provide more evidence than a vague speed claim. Repeated point
reads yield1read/15hits for each policy, while the eight-page double scan yields
16/0 for direct and two-way and15/1 for the pinned root. Native `sizeof` and stack
reports account for implementation padding and temporary pages, without assuming
ABI identity or physical latency. The independent7203-history model exercises
owner/generation/view changes and a partially failing read. It is finite model
evidence, not an arbitrary interleaving proof; all APIs here require external
serialization. No learner or human-independent assessment is implied.

# Resizing storage: distinguish the caller's failure policy

Required reading supplement for T21 resource ownership and the existing
PR-T21-01 assessor ledger, also applicable to T45.07.c. R-EC2-LEAK-SUCCESSOR
identifies the actually inspected second-edition passage, physical PDF151–152,
printed121–122. Its copyright page says2025; the retained publisher release
record says2024. The earlier unsuccessful `Books` search is historical: this
manifestation is held in the Humble Bundle collection.

The passage explains why overwriting the sole owning pointer with a failed
resize result loses access to still-allocated storage. Its worked example
chooses to free that storage and return after failure. That is a caller policy;
it does not provide the recoverable contract that preserves the original
container. The book uses C23 syntax and warns about zero-size resize. Keep the
curriculum baseline C17, reject zero size before this exercise's call, and keep
edition-specific zero-size semantics outside the selected contract.

Consider an original byte container with `{owner, length, capacity}` equal to
`{allocation-A, 5, 8}`, containing the five initialized bytes `alpha`. A request
for capacity13 has these distinct valid designs:

| Outcome/policy | Owned storage afterwards | Metadata | Permitted next action |
|---|---|---|---|
| Success | New allocation with first five bytes preserved | capacity13, length5 | Initialize newly used bytes before reading |
| Recoverable failure | Allocation-A remains owned | capacity8, length5 | Inspect old contents, retry or destroy |
| Destructive failure | Old allocation explicitly released | Empty, non-owning state | Report failure; never expose old contents |

The API must select a failure policy before an implementation is judged. A
temporary result alone does not decide which policy applies. Updating capacity
before allocation and leaving it inflated after failure violates the recoverable
invariant even when no allocation leaks. Checked size derivation must precede
the call; a successful allocation cannot repair an earlier wrapped size.

On success, reconstruct interior views from validated offsets and the new live
base. Do not compare or evaluate an old pointer to decide whether the allocation
“moved.” A numerically unchanged address does not preserve the old allocation's
lifetime. A handle only helps if every consumer uses it and respects its
invalidation/borrow contract; a copied raw alias defeats that indirection.

For the existing criterion's reasoning artifact, predict all three rows and a
failure after metadata preparation. Explain which policy the existing learner
ledger implements and identify the single point where ownership changes.
The assessor must reject a solution that reports recoverable failure after
freeing the old object, or a “no leak” proof that ignores aliases and metadata.
Use the already required allocation-failure campaigns and independent byte
oracle. This reading supplement does not claim fresh execution of those tests
or a new learner assessment. The separate assessor note supplies the worked
diagnosis without distributing book text or book example code.

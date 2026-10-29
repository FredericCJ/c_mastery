# Allocation, storage and durable indexes

A record service first needs bounded storage, not a general allocator. Begin with
caller-owned pools and explicit object lifetime. Add an arena when a request can
release a cohort together. Free lists and bitmaps represent different allocation
queries; account for scan bounds, alignment, fragmentation and retired identities.
A total-free-byte counter does not prove that a contiguous request can succeed.

Connect finite pool histories to configurable storage. Preserve the independently
checkable tiny instance while deriving larger integer and multiplication bounds.
Inject allocation failure at each acquisition. A failed operation must preserve
all previously published objects, not merely avoid a leak. Reclamation policy
must name outstanding borrowers and the event that permits reuse.

Ordered trees answer point and range queries; PR-T14-06 separates balance from
ordering correctness. The durable store changes the cost model to pages, writes
and recovery. Path-copy B-tree versions and ordered-run compaction retain roots
and tombstones until their readers and recovery contract permit removal. Explain
why a logically redundant tombstone can still be necessary during compaction.

The publication boundary distinguishes write completion, file synchronization,
rename and directory synchronization. Inject real calls through the synchronizer
interface, observe propagated error, inspect retained artifacts and forbid a
successful durability claim after failed sync. Label injected failures as simulated
and actual native outcomes as observed; neither proves every power-loss history.

Transfer gives point/range proportions, a restart contract, a byte budget and an
update burst. Select representations and a recovery protocol without being told
to use a tree or log. Compare credible alternatives using page cost and resource
ownership, then revise when range scans or failure atomicity change.

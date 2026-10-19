# Entity, reclamation and rope investigations

`dod_structure_measurements.json` under the modern_completion review directory
records140 observations across20 cells, with seven trials per compiler/workload.
Its source pins, exact compiler commands, logs, separate instrumented-source
transformation and static stack reports are authoritative. The workload uses
the actual entity adapter, DSparse handle checks and DQueryWorld cache, compared
with an original component sparse-set baseline (dense IDs/values and inverse
indices). Both produce the same independently checked result for two stable
live entities. This is an admitted success workload, not a claim of equal
unrestricted failure or allocation APIs.

For twelve mutations, observed archetype ID/component stores total272 bytes;
the component sparse sets store124. Full-world staging and metadata writes are
excluded from this payload counter and remain in runtime cost. Queries with
required mask3 invalidate the actual cache twelve times, independently of
whether one or sixteen queries follows each mutation. Query-heavy timing uses
256 queries after each of6000 edits; mutation-heavy timing uses one. GCC O2
median times were8971580ns versus968433ns for query-heavy archetype/sparse
representations and158068ns versus33255ns for mutation-heavy work. Clang gave
8707203/1213301ns and137846/26282ns. These small workloads favor the sparse
baseline; they do not demonstrate a universal layout advantage. Different
translation-unit boundaries and inlining opportunities are part of these
binaries. Cache hits still require result iteration, and the staged archetype
adapter deliberately provides a stronger rejected-batch guarantee than the
success-only benchmark needs. A larger-population cache study remains a changed
experimental condition, not an observation hidden in these numbers.

For reclamation, all modes destroy the same15-node reference-counted tree with
eight owned one-byte leaves. A synchronous release takes one call; budgets1,
2 and4 require15,8 and4 drain calls. The driver reconstructs1000 trees outside
the timed intervals and sums actual release/drain durations. GCC medians were
62212ns synchronous,79679ns at budget1 and62820ns at budget4. Clang medians were
51937,71210 and62557ns. Every run checks15000 reclaimed nodes and8000 drops;
the pending stack never exceeds4. Clock overhead remains in the sum, and a
budget limits processed reference entries, not callback wall time. No hard
deadline follows from these host samples.

The raw GCC static reports give32 bytes per `d_rc_release` frame and16 for the
successful drop callback; Clang reports24 and8. The fixed depth-four tree gives
conservative internal call-path sums144 and104 bytes, before caller/runtime
stack. `d_rc_drain` plus drop reports80 and64 bytes. The work record itself is
272 bytes and must live somewhere; the GCC benchmark caller separately reports
3248 bytes. Incremental reclamation shifts storage into an explicit record and
adds scheduling calls rather than eliminating cost. General callbacks need new
stack and latency arguments. Updating each of eight positions of the integer
region tree allocates32 nodes; retaining all roots reaches32 nodes, retaining
only the latest reaches15 while all32 remain reserved. Seven nodes have shared
incoming references. Its12-byte integer nodes and the32-byte reference-counted
owned-text nodes have different payload contracts; their structural census is
not a fair cleanup-time race.

The rope companion's independent32-row tree oracle checks before/after depth,
reachable nodes, unique payload storage, unused leaf capacity and Fibonacci
threshold. Sixteen one-byte leaves in a left chain have depth15,31 reachable
nodes and112 unused leaf bytes. The copied balanced rebuild has depth1, three
reachable nodes and no leaf slack, while31 prior nodes remain reserved. Sharing
one three-byte leaf twice gives six logical bytes but only three stored bytes.
These are fixed-region internal slack/retention observations, not a malloc heap
fragmentation measurement. Piece editing separately leaves twelve dead appended
bytes after twelve insert/delete pairs. Compaction requires16 snapshot+48 append
bytes of additional backing, fails unchanged with15 snapshot bytes, preserves
old descriptors, and permits reuse of old backing only after all its descriptors
are retired. ISO C does not track those borrowed lifetimes.

Follow-up primary passage inspection: Boehm/Atkinson/Plass, *Ropes*, Rebalancing,
printed1319â€“1320 (PDF pages5â€“6) defines root depth against a Fibonacci minimum;
its condition permits unbalanced subropes and its forest algorithm can finish
within an additive depth allowance. Our copying midpoint rebuild and exact
root predicate are distinct choices. We did not implement or claim execution
of the paper's sharing-preserving forest algorithm or import its measurements.
The existing registered Algorithms locator bounds this reuse.

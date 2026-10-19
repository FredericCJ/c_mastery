# Logical redo assessor analysis

The checkpoint is `(applied_id,total,last_delta)`. The command is
`(id,predecessor,delta)`, so replay performs checked arithmetic rather than
copying an after-image. If selected checkpoint ID is lower, exactly-next
predecessor authority permits one ADD. If equal, equality of the stored delta
proves this is the already-applied command under the sequential-client policy;
reclaim without another ADD. If older, a stale command cannot supersede state.

The checkpoint flush must precede command reclamation. Publishing the total
without applied ID is insufficient: a failed clear followed by replay would
add again. The compiled second mutant does exactly that logically, bypassing
the equal-ID skip. Its failure is a value/identity assertion, not C undefined
behavior. The fifth mutant deliberately computes modulo64 via unsigned values
and defined signed decoding, so its rejected overflow result is also defined.

For a pending command, commit's last flush is the acknowledgment boundary.
An error there may follow persistence. The public result remains untouched;
the handle is poisoned. Recovery followed by the same operation request is
safe within the stated one-client/one-image lifetime policy. A different ID
cannot mean retry: the supplied positive counterexample yields24 rather than17.
Old IDs are rejected because this bounded design retains only the latest
receipt. It does not invent historical responses or provide global exactly-once
effects. Image reformat/replacement requires a new external identity discipline.

The Python oracle independently generates1093 integer histories,28 commit
outcomes and the entire canonical command page using struct/zlib. C tests
exercise28 checkpoint cuts and112 repeated interruption histories, plus
checked extrema, maximum-ID image, malformed-but-resealed commands and partial
read refusal. The arithmetic admission test precedes any previous-checkpoint
I/O so a rejected new operation cannot silently change durable progress.

This is intentionally a new logical counter protocol. It does not claim that
one command record can replace arbitrary B+tree operations, structural redo,
undo, ARIES page-LSN rules or multi-client ordering. Require the learner to
derive those additional obligations rather than treating this model as a
complete database recovery theorem. CRC detection and successful flush remain
declared premises; arbitrary post-acknowledgment media corruption is not one of
the old-or-new interruption choices.

Initial optimized author execution passed seven anchors and five semantic
controls. Use the final source-bound report for actual compiler/profile claims.
No physical observation or independent human/learner assessment is supplied.

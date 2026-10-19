# Assessor-only callback stream analysis and delayed release

The actual source imports `al_stream_init/feed`, which in turn imports the
retained `ad_match_init/feed`; there is no replacement matching algorithm. This
adds a delivery cursor, not a second prefix algorithm. On a failed callback the
source input has already been consumed, so keeping only the prefix state loses
the failed output. Keep the staged offsets and retry the same cursor. A new feed
must not overwrite pending storage. On cancellation the protocol intentionally
discards unpublished values and closes; it retains no asynchronous references.

The three mutants are discriminating: SKIP advances on rejected publication
and loses one offset; REPLACE permits a new feed to overwrite pending results;
CANCEL continues after a cancellation result. Ordinary success alone does not
reject any of these. Tests inspect complete output lists, attempted/accepted
counts, consumed input and pending cursor. They also require no report change on
BUSY/CLOSED and arithmetic-overflow rejection. Failure that writes before returning
is outside callback contract; runtime code cannot detect or undo arbitrary
external effects. A proof of this API cannot be generalized to arbitrary callbacks.

The binary text/pattern oracle enumerates naive windows independently of prefix
failure links. It compares every offset rather than output cardinality alone.
The earlier lookup provider's actual4815-packet oracle and prefix mutation remain
separate evidence for that prerequisite. Callback tests add40 split/failure
histories, six cancellation/boundary categories and4815 compositions per profile.
No unbounded-history, concurrency or cryptographic guarantee is claimed.

For X-T36-003 R2, require explicit distinction among unchanged ordinary data,
changed required-count output, atomic per-chunk prefix computation and partially
committed callback effects. For R3, require every overlapping offset, the one-time
empty initial boundary,65-offset capacity derivation, and the retry/cancel resource
bound. Finite tests are necessary evidence, not the proof or independent mastery.

Delayed V1/V2 release after at least seven days: pattern`abab`, chunks`ab` then
`ababab` gives offsets0,2,4. Fail the callback for2 once: accepted0 remains,
the retry must emit2 then4 without replaying the chunk. In a separate reset
lifetime cancel at2:4 must never be attempted. Change to empty pattern with an
initial empty chunk and two later empty chunks; only one boundary is emitted.
Have the learner seal predictions, identify which stream fields own each fact,
derive a fresh self-overlap case and substitute actual learner prefix artifacts.
Preserve attempt/source/timing/hint records. Author execution does not satisfy
the delay or supply non-author review.

Alternative valid implementations may stage a bounded pending ring or use an
explicit owned publication record, provided they maintain identical offset order,
no-effect-on-failure assumptions, retry/cancel semantics and capacity. A policy
that abandons the entire input chunk on any callback failure is different and
requires a different contract and replay proof; do not silently accept it here.

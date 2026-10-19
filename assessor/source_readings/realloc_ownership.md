# Assessor reading reconciliation: allocation ownership

Use the existing criterion rubric; this note is a worked teaching comparison,
not a replacement test or an independently completed learner attempt.

The recoverable transaction's commit point is publication of the successful
new pointer and capacity after checked admission. Length is unchanged until
new payload bytes are initialized. On failed allocation the old metadata and
bytes remain observable. A destructive policy instead consumes old ownership
and must invalidate every exposed container state before reporting failure.
Neither policy permits a stale capacity claiming unavailable storage.

Hint1: track each allocation separately from each pointer variable. Hint2:
draw the error return before metadata publication. Hint3: mark all borrowers
whose lifetimes must end before a successful resize. A temporary variable is
necessary for the chosen recoverable implementation but does not itself prove
that no other owner or view exists.

Incorrect “repair”: null the caller pointer on every error. This loses a live
allocation under the recoverable contract. Incorrect “optimization”: compare
the old pointer with the returned pointer before invalidating interior aliases.
After successful resize the old lifetime has ended; such evaluation is not
the permitted identity test. Save offsets before the call while the old object
is live and reconstruct only validated offsets after success.

Assess failure semantics, metadata invariants and alias invalidation separately.
An answer can understand leak avoidance while still failing the alias criterion.
The delayed resource-transfer assessment must independently choose and justify
an error policy for its changed owner; copying the book's destructive path into
a retryable container does not satisfy that criterion.

The named source passage has now actually been read. This closes its access
and inspection child only. It does not inspect published ISO C17/C23 wording,
the entire book or an actual allocator implementation.

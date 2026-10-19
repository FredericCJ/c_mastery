# Guarded callback cell — assessor analysis

Two lifetimes are distinct: the fixed mutex/control object and each permission
token. Synchronization protects both the scalar and every guard-table transition.
The callback does not run under the mutex: recursive entry can lock normally and
observe the active writer, rather than relying on recursive mutex behavior.
The retained writer excludes all other admitted whole-cell permissions until
the original callback returns. Nested update also observes updating and refuses.

Commit takes the same lock as close. Therefore selected close-before-commit
rejects staged output; commit-before-close stands. Callback rejection is another
nonpublication path, but its external effects persist. Each path releases the
same acquired token and clears the pending update. Making the scalar atomic
would not preserve these multi-field and lifetime invariants.

Destroy requires no future or current callers independently of loan count. A
thread can be between entry and lock acquisition with no recorded guard. The
implementation never treats a zero guard count as proof that such a thread does
not exist. Tests join every worker and drain tokens before actual destruction.
The old token cannot be used against a new lifetime at a reused address; this
protocol intentionally has no global incarnation allocator.

The sequential oracle predicts admitted callback count independently of code.
Native four-worker and condition-controlled close cases test actual exclusion
and lifetime. TSan is a diagnostic for observed executions, not a proof of all
histories. Tests do not deliberately execute a data race as an expected result.

Score the four original criteria separately. Ask the learner to contrast the
chosen BUSY rejection with a snapshot API that would need a stated version and
revalidation/publication policy. That alternative is an independent assessment
question, not silently claimed implemented by the rejection policy.

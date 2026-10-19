# T51.12 technical analysis

The retained core implements writer preference: its writer queue scan is
recomputed when a writer is abandoned, so removing the final writer can admit
readers without first waiting for every active reader to release. The adapter
caches that actual terminal result and tracks an acquisition ticket separately
from a held share. The reservation cap includes pending and uncollected read
results; therefore a whole raw-core cohort cannot exceed the declared limit.
Repeated acquisition is rejected rather than implementing POSIX recursive read
holds. Nonowner release is checked, unlike the undefined POSIX nonowner call.

The phase core implements new policy over actual CMX latch waits. It admits
all queued readers after a writer, then gates later readers when another writer
waits. Arrival numbers order writers only. With an absent opposite class,
conceptual empty phases do not impose idle execution. The finite phase counter
is bounded by total admissions; every granted nonempty phase consumes at least
one distinct admitted request. A waiter cannot overwrite its selected result
by starting another operation before collection. Retained ownership and pending
wait state are distinct during upgrade.

For a writer behind k previously admitted writers, later writers cannot pass
it. A finite current reader cohort and at most one reader cohort between each
preceding writer grant can delay it. A reader arriving behind a pending writer
joins the next reader phase after that writer (or its already selected current
phase drains). Every cohort has at most N registered participants, and late
readers cannot extend it while a writer waits. These facts bound overtaking
under eventual execution/release; they do not bound elapsed time without
scheduler and critical-section bounds. A writer-preferred counterexample keeps
W1/W2 alternating requests ahead of a queued reader, just as reader preference
keeps the rolling reader cohort ahead of a queued writer. Exclusion alone
admits both counterexamples. Single-upgrader admission never bypasses an
already queued writer; its retained read share otherwise creates a cycle.

Do not call a cohort wake linear from its outer loop: `rw_admit` repeatedly
calls O(N) `first`, and phase reader grants each call the O(N) latch search.
Both are O(N²) in the generalized participant count. The actual priority
recomputation has up to N relaxation passes scanning N tasks, plus the mutex
table; cancellation includes that imported work. At fixed N=16 these are
bounded operations, but not measured execution-time bounds. The native driver
adds two timestamped endpoints per ownership interval, locks around metadata,
condition broadcasts and possible sleeping; those costs can dominate a short
64-word body. Each timed sample requests600 reader snapshots and200 writer
updates, or38,400 payload-word validations and12,800 assigned words. Each
reader also loads an initial comparison value and checksums the words.
These counts are semantic payload work, not emitted memory instructions or
cache-line transfers: compiler combining, cache hits, line sharing and coherence
invalidate a direct conversion. All samples are retained; the support guard
reports per-policy elapsed ranges and medians without claiming a speed winner.

An upgrader may retain its read share only if no writer predates the token.
Otherwise the writer would wait for the upgrader's read share while the
upgrader waited behind the writer. A second upgrading reader must receive
DEADLOCK and keep its existing share, permitting it to release rather than
blocking both upgrades. Cancellation/timeout returns the first upgrader to an
ordinary held-read state after result collection. Conversion changes reader
bits and writer identity in one externally serialized transition and does not
increment ownership count. New requests cannot pass an active upgrader.

The native test's outer mutex protects metadata, not entire payload bodies.
The final reader's release follows all its payload reads; a subsequent writer
acquires the same mutex before entering. A writer's payload stores precede its
metadata release, which precedes future reader acquisition. These native
mutex edges provide C publication, while logical ownership prevents conflicting
bodies from overlapping. ThreadSanitizer checks exercised schedules only.
The selected M4 image has sequential logical tasks, so it does not independently
establish a multicore memory-order or interrupt-exclusion port.

Reader preference admits the rolling A/C cohort and keeps B waiting for64
handoffs. This is the required concrete progress discriminator, not proof that
a finite-counter program executes forever. The proper policy admits B after
the original reader drains. A faster read-heavy sample cannot repair a failed
ownership or fairness contract. Compare equal payload counts and safety while
stating policy differences and the costs of actual CMX scans, external mutex,
condition wakeups, trace clocks and cache coherence. Raw CPU observations on
WSL do not identify physical cores or establish hardware cache-event counts.

POSIX2024 explicitly permits multiple read holds and has scheduling-option
qualifications for admission. Its write-lock call while already holding a
read lock can deadlock or report EDEADLK; it is not this token upgrade API.
Its nonowner unlock is undefined. The inspected phase-fair manuscript supplies
policy definitions; this blocking adapter does not inherit its spin-lock
implementation, remote-reference complexity or schedulability bounds.

Assess `.a` using the exact A/B/C trace, rolling-reader counterexample and
R-W-Rbatch-W-R phase witness. Assess `.b` with two pending writers removed in
both timeout/cancel orders, cap2 failure without ownership mutation, stale
operation tickets, upgrade-cycle refusal and retained-read cancellation.
Assess `.c` with interval-set audit, actual native affinity observations and
all equal-work samples, not a favored speed ratio. The delayed variants require
new phase/upgrade/multicore reasoning; current author execution does not stand
in for an unaided delayed learner attempt or independent reviewer judgment.

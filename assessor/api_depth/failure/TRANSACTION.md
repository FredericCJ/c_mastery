# Private T21.05 analysis

05.a: at capacity2, expected strong/basic/partial states are[1,2],[] and[3,4],
respectively. Partial accepted=2 does not mean whole request success. Notification
failure after an otherwise successful update gives AMBIGUOUS, committed1 and the
complete replacement. Grade preconditions, aliasing input and exhausted version.

05.b: the independent oracle computes prefix length from the earliest failing
index and capacity; observers reject old-version/new-value mixtures. The
`AE_MUTANT_MIXED` branch compiles but fails that check. The concurrent variant
holds two old leases while publishing a third root, reaches the three-node bound,
then releases leases and runs1000 actual replacements concurrently with a reader.
All created nodes must eventually be destroyed. `AE_MUTANT_UNPINNED` is rejected
by the counts before the test dereferences a stale pointer. A plain atomic root
load followed by incrementing its reference count is an invalid alternative:
the node may already have been freed. Hazard pointers/epochs are valid alternatives
only with their own complete admission and reclamation proof and implementation.

05.c: request sequence(1,2),(2,1),(3,1),(1,2) has four effects,value6 after expiry.
An unresolved intent is retained on crash1/2. The implementation intentionally
does not guess whether the effect happened. `AE_MUTANT_REPLAY` clears intent
after effect and causes a second effect on retry; the semantic test rejects it.
Crash3 has a completion record, so retry deduplicates. Do not award durability
credit for a process-memory model or for inspecting an oracle unavailable to the
specified recovery client.

Release changed concurrency and crash packets after the original minimum seven
days. Accept mutexed immutable publication or a proven alternative; require
actual threaded execution plus lifetime reasoning. A correct scalar result with
unsafe load-then-retain, conflated error contracts or unsupported exactly-once
claims fails the corresponding criterion. The author campaign is not a learner
attempt, independent review or proof of all interleavings.

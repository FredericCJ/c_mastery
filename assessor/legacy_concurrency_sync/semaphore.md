# Assessor analysis: exact T51.02 successor

The reference composes original CMX task/wait transitions with the generational
wait owner. It does not substitute a host semaphore. A direct handoff increments
successful posts and acquisitions together, leaving free count unchanged.
An ordinary post increments posts and free count together. A successful take
decrements free count while incrementing acquisitions. Timeout/cancel/close
remove claims and do not consume future posts. At maximum2/initial1, after two
waits and four posts, exactly three posts are accepted and one is refused.
The separate event-bit comparison must explain coalescing instead of treating
a refused counted post as delivered.

Priority selection is a policy adapter around actual CMX FIFO completion. The
temporary stamp exchange is unobservable under the explicitly required common
exclusion, and the original stamps are restored even if the underlying operation
refuses. Generation tokens do not use these internal ordering stamps. The
reference tie-breaker is FIFO within one effective priority. A higher-priority
task can repeatedly reenroll and overtake an old lower-priority waiter; conserved
tokens do not imply that every waiting task eventually consumes one.

The deferred ledger is independent of the semaphore ledger. Offering transfers
ownership to the mailbox; successful drain transfers it to the semaphore, and
close explicitly cancels it. Full target count cannot justify reducing pending.
Budgeted partial drain preserves its completed-prefix count. The coordinator
must arrange a later drain when capacity becomes available; a bounded handler
alone does not guarantee that future trigger exists.

IRQ0 priority0 may preempt a BASEPRI0x80 region, so it cannot call a core whose
exclusion policy relies on that threshold. The raw mailbox uses saved PRIMASK
around its short counter operation and pends PendSV. The target masks PendSV
while three distinct IRQ0 entries happen, showing two retained credits and an
explicit overflow rather than three assumed pending exceptions. Pending IRQ
state is a notification, not the ledger. An unrelated newly ready task makes
an illicit scheduler call in the urgent handler observably change `current`.

The native compiler/runtime of the host tests is not a freestanding target
dependency. Target helpers must be individually declared and audited. An
undefined symbol, fault, timeout or arbitrary crash does not demonstrate the
required property violation. The bounded target has no NMI/HardFault mailbox
access, no multicore producer and no physical timing observation.

Grade the original `.a`, `.b`, `.c` criterion and delayed tasks separately.
Do not give independent-transfer credit for copying this explanation or for
executing supplied author code before the original delay. The exact finite
ledger and changed-policy counterexamples are useful falsification evidence,
not universal safety or liveness proofs.

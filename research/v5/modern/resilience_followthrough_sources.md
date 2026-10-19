# Additional T75 passage inspection

On2026-10-17 the author opened and read the primary
[CircuitBreaker documentation](https://resilience4j.readme.io/docs/circuitbreaker),
specifically Count-based sliding window, Failure rate and slow call rate
thresholds, and its thread-safety discussion (rendered lines33-69), and
[Bulkhead configuration](https://resilience4j.readme.io/docs/bulkhead)
(lines25-31,47-53,75-85). The count-window passages motivate retained outcome
eviction and minimum observations; the separate concurrency discussion prevents
claiming a history size bounds active resource use. These are bounded page
reads, not a full source-code/library audit.

The implemented tenant reservations, finite epochs, cancellation ownership,
neutral-probe policy and rational thresholds are original course contracts.
They are not attributed to a matching external API. No production performance
numbers or independent assessment are inferred. Exact published C17 remains
an access gap. The canonical resource register/gate separately pins this note.

## Journal I/O and fencing inspection

On2026-10-17 the author opened and read the actual Linux man-pages6.19 pages:
[fsync(2)](https://man7.org/linux/man-pages/man2/fsync.2.html), DESCRIPTION,
RETURN VALUE and ERRORS (rendered lines39-98);
[write(2)](https://man7.org/linux/man-pages/man2/write.2.html), DESCRIPTION,
RETURN VALUE and NOTES (lines26-72,147-170); and
[pread(2)](https://man7.org/linux/man-pages/man2/pread.2.html), DESCRIPTION,
RETURN VALUE and BUGS (lines36-61,90-96). These passages distinguish partial
progress from a failed transfer, successful write from durability, file from
directory synchronization, and positional I/O from Linux append-open behavior.
The adapter rejects append-open files; the journal preserves uncertain outcomes
and stages publication around its declared flush contract. Actual local syscall
execution does not test physical power removal or prove a device honors flush.

The author also reread the existing registered
[fencing argument](https://martin.kleppmann.com/2016/02/08/how-to-do-distributed-locking.html),
Making the lock safe with fencing and Using time to solve consensus
(lines59-79). It supports resource-side monotonic checks and the distinction
between a timeout and proof that a remote actor stopped. The two synthetic
independent-writer replicas and the single-authority/read-snapshot repair are
original bounded experiments, not an implementation or assessment of Redis,
ZooKeeper or a current deployed service. No reference followed from its
bibliography is credited as inspected here.

## Local framed service inspection

On2026-10-17 the author opened and read Linux man-pages6.19
[poll(2)](https://man7.org/linux/man-pages/man2/poll.2.html), DESCRIPTION and
RETURN VALUE/ERRORS (rendered lines30-135,193-216);
[send(2)](https://man7.org/linux/man-pages/man2/send.2.html), DESCRIPTION,
MSG_NOSIGNAL, RETURN VALUE and selected ERRORS (lines28-70,118-124,179-203,
227-228,261-263); and
[socketpair(2)](https://man7.org/linux/man-pages/man2/socketpair.2.html),
DESCRIPTION, RETURN VALUE and HISTORY (lines25-42,72-81).
The bounded claims concern actual readiness/progress, per-call signal suppression,
local endpoint ownership and timeout/error distinctions. No operating-system
implementation audit or external service execution follows from reading them.

The author also inspected the actual retained `hs_read_until` and
`hs_reactor_sum` source bodies and common helpers. The former is a borrowed
descriptor read with cancellation/deadline; the latter aggregates one byte from
each descriptor. The new service owns its own parser and write loop, and does
not credit the sum routine as a preexisting general reactor. Its unnamed local
socketpair and synchronous file calls are distinct from the retained virtual
TCP fault model and from any physical network or storage validation.

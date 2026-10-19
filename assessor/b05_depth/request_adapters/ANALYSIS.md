# Request reset solution and independent transfer package

The state table in the public tests is a separately written expected relation.
Do not derive it by invoking rq_apply. Ownership is exactly INFLIGHT, CANCELLED,
UNKNOWN or RESETTING. Only COMPLETE or RESET_ACK increments terminals; both
transition to DONE, where neither can be accepted again. REAP increments the
generation or retires, so a late event cannot address the next admitted request.
The full table includes otherwise unusual synthetic states to test unchanged
failure and boundary arithmetic; it is not a claim every injected state is
reachable through the public history.

An alternative valid representation separates a completion-known bit from a
DMA-owned bit. Assess both dimensions; a cancellation bit alone is insufficient.
The alternative must preserve the same failure and retirement contract, including
reset unknown outcomes and completion during reset. Per-request hardware abort
could replace a whole-device reset only under an actually negotiated/documented
quiescence contract, with corresponding tests; writing a cancellation flag is not
such an implementation.

The kernel reference uses two3-descriptor chains, two independent DMA buffers
and the original single queue. A ticket encodes slot plus32-bit generation in
64 bits. A used entry supplies a chain identity, not a user-controlled result
pointer. The global lock serializes descriptor ownership and process reaping.
Used-ring polling does not depend on a one-interrupt/one-completion relation.
Holding the observer until both real entries exist discriminates that mistake.
131,074 single-outstanding post-reset reads cross both actual16-bit indices
twice; the exact
final index includes the one read performed immediately after reinitialization.

During reset, retain control entries and DMA memory while rejecting submissions.
Reading device status zero is the transport observation. The withholding control
tests the driver observer premise explicitly; QEMU may have reset immediately.
Only after acknowledgement may vd/va/vu and used_idx be cleared. The outstanding
request becomes DONE with UNCERTAIN, because successful reset cannot determine
whether its earlier write took effect. Receipt-based application deduplication
from X-T56-005 is the next composition mechanism, not a device-reset guarantee.

The cancellation mutant prematurely sets DONE; the target fails “cancel intent.”
The reset-intent mutant sets DONE before acknowledgement; it fails the retained
request assertion. The missing-observer-check mutant reinitializes and publishes
despite a withheld acknowledgement; it fails that specific assertion. The status
mutant suppresses an actual blkdebug EIO and fails “device error propagated.” All
are compiled C programs. The public scaffold instead rejects “hold observer”
after successful link/boot; this is an intentionally unfinished learner artifact.

Owner exit must not erase pending metadata. The child test uses the actual spawn,
disk capability, process exit, completion and remote reaping path. Its payload
is copied at submission before the child mutates its buffer. The test observes
a dead owner with pending ownership, then checks the original bytes after
completion and reaping. A different valid design can pin a user buffer rather than use a
bounce buffer, but must supply unmap/exit pinning, allocation-failure and cost
evidence; the reference deliberately makes no such claim.

The original AX-PR-T56-07-T56.07.a asks about the retained one-request adapter:
its condition requires both an advanced IRQ sequence and used-index progress.
For (new IRQ, used progress)=(0,0),(0,1),(1,0),(1,1), it proceeds only in the last
case, then checks descriptor/status. That is the retained policy, not a universal
virtio requirement. This successor polls the used ring independently: (0,1)
may complete after descriptor/status validation, while (1,0) cannot. Explain the
changed liveness premise rather than rewriting the old source observation. A
stale IRQ alone cannot publish any result; old application tickets fail through
generation checks. The original a/b/c delayed variants are supported respectively
by two blocked matching waiters, reset acknowledgement, and repeated single-
outstanding ring wrap. None is an independently administered learner result.

Withhold this directory when issuing independent transfer. Release V1 after the
original seven-day minimum with an unseen sequence mixing timeout, completion,
reset start and duplicate acknowledgement. Release V2 with the learner's actual
prerequisite revisions, two distinct page patterns and a withheld observation.
Require a predicted trace, changed invariant/cost argument, fresh implementation
and diagnosed negative control. Record assistance and each original R1/R2 score;
do not fill learner records using these author executions. A valid follow-on may
change the pool capacity but must rederive descriptor allocation and queue bounds,
rather than merely substitute a number in the existing program.

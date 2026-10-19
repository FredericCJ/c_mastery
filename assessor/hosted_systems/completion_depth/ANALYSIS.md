# Completion I/O assessor packet

Withhold this file, reference C, event-set oracle and all raw author measurements.
The original three criterion objects and seven-day variants remain authoritative;
`CONTRACTS.md` makes their inputs, outputs and required implementations concrete.

For `.a`, demand a timeline with two identities. The pool conservatively retains
the slot until both target and requested cancellation records are consumed. It
does not infer target state from cancel `res`. Alternative designs may keep a
separate cancel-record table and reuse a buffer after the target CQE if they prove
the outstanding cancellation cannot name reused metadata. Reusing just an index
is not such a proof. Explain why stale rejection and generation retirement are
distinct. Check unchanged invalid-input state and oversized-result rejection.

`order_trace.c` enumerates720 orders of six events on two independently cancelled
slots. The Python oracle uses event sets, not the production state-transition
code. Releases attempted too early are rejected, not postponed invisibly. The
three compiling controls delete token comparison, treat cancel acknowledgment as
target completion, and ignore a pending cancel at release. Each must exit1 at the
intended public CHECK, not crash or fail translation. A learner who special-cases
the six public examples must also handle the hidden two-slot orders.

Native modes use the actual T62 provider and current installed operation probe.
The canceled blocked read returns `-ECANCELED`; its distinct cancel request
returns0. Application dispatch can occur in either order. Delaying dispatch of a
target already completed in the kernel produces the separate absent-request
case. The stale-generation cancellation must not cancel the newer blocked read;
that newer read receives exactly `safe`. Registered subrange+17 returns `abc`;
an overrun of that indexed region is rejected before use. Five injected setup
cuts precede publication; do not claim they prove recovery from every partially
published SQE. For submission uncertainty retain resources and reconcile the
raw-ring queue; the scaffold does not offer a general rollback transaction.

For `.b`, the two backend implementations must consume identical bytes and keep
the file immutable until peer consumption. Application allocation and maximum
borrowed response bytes are different metrics. Fixed buffers add an explicitly
measured registration interval. Ring mappings and stack/object evidence must be
included; kernel resident bytes and bus traffic are not measured here. The
stalled sender must actually reach EAGAIN on each backend. A preliminary64KiB
sendfile specimen did not: the current1MiB fixture fixes this vacuous assumption.
Preserve the observed capacity instead of assuming SO_SNDBUF equals queued
payload. After half-close the receiver must consume the exact prefix and reach
EOF. No wall-clock maximum can be derived without a bounded receiver/scheduler;
derive the finite admitted-byte bound, state the conditional time expression,
then report the measured sample separately.

For `.c`, inspect the sealed prediction, its creation time and input identity.
Do not pass an after-the-fact fastest-backend story. The author pilot already saw
four cells, so its later sealed matrix prediction is explicitly pilot-informed,
not a blind learner attempt. The withheld learner assessment starts with a fresh
seal before any alternate result. Accept a wrong prediction followed by a sound
causal diagnosis; reject a correct ranking obtained from an unequal contract.
The workload includes source reads, sending, peer byte verification, setup and
cleanup. It is a batched regular-file-read/local-stream implementation, not a
fully asynchronous server; block-on-CQ batch collection is a real design cost.

Delayed packet: after at least7 days, assign the registered-buffer variant for
`.a` without this reference; for `.b` withhold the observed full-socket prefixes
and require both cancellation/resource arguments; for `.c` disclose only the
small scattered request generator and require a new sealed prediction. Reuse
the same stable delayed IDs and record learner-source hashes, commands, result
and independent reviewer identity. A fresh author run cannot fill that record.

Rubric discrimination: copied output alone earns at most1. Level2 separately
requires `.a` ownership/token argument and real CQEs; `.b` complete retained
resource accounting and a stalled receiver; `.c` equivalent byte contract and
interpretable full matrix. Level3 requires the changed solution and explanation
under independent delayed assessment. Do not count the independent Python model
as independent human review. No finite campaign proves absolute mastery.

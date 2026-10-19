# T62 completion assessor analysis

The ring copies SQE descriptions and tracks published versus submitted counts.
Subtracting only syscall-reported consumption preserves pending entries after a
partial enter. Metadata reuse follows SQ head, while payload lifetime follows
the matching terminal CQE; these are different resources. Single issuer permits
ordinary local-tail updates, not multiple producers racing on the same slot.

Cancellation success is a request result. The target's terminal event is still
consumed before buffer reuse. Generation1 cancel cannot match generation2 because
the full64-bit identity is passed. The timeout pair has two owners and may arrive
in either order. Multishot positive results with MORE retain the operation; a
terminal event closes it. The MORE-removal mutant is killed before cancellation.

The writev oracle checks the actual short prefix across a zero-length vector.
The splice observer establishes byte transfer/drain, not zero-copy internals.
Withheld transfer: feed cancel-EALREADY and an ordinary target success in reverse
order, then attempt buffer reuse between them. Require the learner to preserve
the target owner despite the failed cancel. Another variant replaces one issuer
with two: adding atomic_fetch_add to tail is insufficient without ordered
publication of initialized SQEs. A valid repair needs a stated reservation and
publication protocol or an external issuer lock.

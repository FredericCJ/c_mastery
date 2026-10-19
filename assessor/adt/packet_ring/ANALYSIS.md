# T67.10 packet-ring technical assessment

Read the learner contract first. The existing ad_packet verdict is authoritative
for the unchanged contiguous exercise. adp_parse calls it and then actual
FCursor/f_take; success has exact extent and byte-aligned accesses. The original
census C++ expected/variant separates alternatives in its type, while its span
still borrows caller storage. C has explicit status plus initialized output;
neither raw view creates an owning reference. A caller can ignore a status in C;
tests do not make that impossible.

For the ring, map logical offset i to `(head+i)%32`. Live offsets are `[0,count)`;
an active lease owns `[0,held)`. Feed writes only `[count,count+n)` after
`n<=32-count`, so no active byte can be overwritten. Header positions are valid
after3 <= n <= count. A payload's start is `(head+3)%32`; first length is the smaller
of payload length and `32-start`; second is the remaining length. No ring
payload pointer is formed for zero length. All intermediate index sums are
bounded below96; size_t wrap is not invoked. Descriptor/output publication
follows all possible refusal branches.

The ownership fact survives cancellation because cancel changes a flag and
does not alter head/count/held. The consumer may finish reading after request;
only its final release is a quiescence acknowledgment. Close likewise retains
the borrow. An old generation cannot affect a new current lease, and MAX is
issued only once. Fixed-address lifetime is an essential extra premise: finite
generation does not make a pointer to a destroyed/reused ring valid. Runtime
token checks cannot protect an unchecked dereference after release. Externally
serialized calls are essential; this core has no C atomic/thread protocol.

The copy comparison intentionally retains the same ring lease to the same
final release. Both versions sum the same29 bytes, half see cancellation, and
both feed and release on every iteration. Copying adds a local29-byte copy;
claims are raw measurements, not guaranteed speedups. Early ring release for a
copied consumer is a different design whose cancellation/output lifetime must
be specified independently.

The imported hs_read_frame owns no descriptor and makes actual read calls.
It can consume a prefix and alter storage before EOF; the adapter guarantees
only unchanged packet output. An outer oversize header must not be treated as
an empty payload or resumed as another header. The original close-on-error
policy matters. The inner16-bit payload maximum and outer16-bit frame maximum
are different by3. The actual codec API demands exact input/output encoded
length, not a capacity argument; preserve that distinction in learner adapters.

Controls1/2 remove the wrapped second span and recycle on cancellation;
3/4 omit generation equality and permit counter reuse;5 publishes output on
transport failure;6 erases codec version;7/8 activate original MOD19/20 trailing
and nonempty-ACK defects. Require ordinary ADO_CHECK rejection with return1;
compiler, timeout, sanitizer or unrelated process failures do not count.

The Python oracle derives byte grammar independently and models ownership with
a list count and identity states. It covers every physical origin, selected
header/extent/kind combinations and all8-operation histories of length5 plus
longer targeted retirement/reuse/close histories. It does not establish
unbounded native-pointer safety or all concurrent executions. The explicit
declared serialization model must not be confused with a lock-free ring.

For delayed R4, demand a trace where the header wraps, payload wraps, a consumer
holds a token across cancellation, and free capacity excludes the held bytes.
Then reach the last generation and show refusal after release. In a separate
two-lease redesign, one global active bit is insufficient: if the later lease
releases first, reclamation must retain earlier occupied bytes and advance only
across a contiguous acknowledged prefix. A reference count alone does not
identify that prefix. This counterexample bounds the ownership argument;
the supplied single-lease reference does not implement multiple consumers.

Level2 requires all ordinary and declared segmented obligations with invariants
and independent evidence. Level3 requires unaided delayed work and explanation
of the counterexample. Source-author tests and this analysis never substitute
for the learner result or an independent assessor. This document claims no
physical timing, arbitrary-code exploit or normative C++/ISO equivalence proof.

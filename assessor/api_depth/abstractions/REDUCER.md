# T19 reducer assessor reasoning

T19.06.a: Requested work becomes WAIT before the adapter runs. Failure-before-
effect leaves backend counter0; lost acknowledgment leaves1. Both reducer inputs
are EFFECT_FAILURE and both become FAILED with the same key. TIMEOUT emits that
key again; the backend applies the first case once and recognizes the second.
Ack then returns READY. An adapter that returns successful state before the effect
commits, or retrying with a new key, violates the requested/committed distinction.
The outbox is fixed-payload in-memory state, so no durable exactly-once claim is
supported. The replay mutant and changed-key mutant are compiled semantic controls.

T19.06.b: All256 cells compare three dispatch mechanisms and sparse mapping with
the independent next-state relation. Invalid e=-1/6, invalid phase/sequence pairs
and sequence exhaustion leave the sentinel output unchanged. Short-circuit admission
must occur before indexing. Sparse IDs are translated by bounded lookup;70 is
absent. The shared transition builder is acceptable here because the experiment
compares dispatch representation; distinct effect histories independently assess
the builder's identity and commit semantics. Inspect actual objects before making
claims about final branches or calls.

T19.06.c: Retired context ownership survives publication of a new backend. Lease
copies store the selected function/context/epoch, so work does not accidentally
reload the current table during an old operation. Last retired lease releases
that owner only after decrementing its count and clearing the retired slot.
Rejected replacement transfers no ownership. Caller discipline forbids forged/
copied leases and calls through released leases; generation values alone cannot
enforce that linear discipline. An opaque managed lease can improve diagnostics
but still needs live control storage and synchronization in a threaded variant.

The original harness uses independent backend namespaces and proves ownership/
routing. It does not prove transparent migration of a shared external service.
Ask for a counterexample where old/new backends each believe a global counter
starts at0; require either namespaced identities (the chosen contract), draining,
state transfer or a shared idempotency ledger. This distinguishes a correct
lifetime protocol from overstated semantic equivalence. Assess the three exact
leaves and delayed tasks separately with the public rubric.

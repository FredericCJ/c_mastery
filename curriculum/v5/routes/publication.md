# Concurrent state: publication is not reclamation

Reuse a working finite queue before adding threads. The producer owns construction;
the consumer may read only after the publication transition. Draw conflicting
accesses and identify the synchronizes-with relation. Atomics do not automatically
make surrounding non-atomic accesses safe, and a correct count does not prove the
absence of a data race.

SPSC ownership gives each position one writer. A multi-producer queue changes
that invariant; reservation and publication may need distinct states. A close
operation must wake blocked participants, reject new ownership transfer and state
whether accepted items drain or cancel. Join users before destroying synchronization
objects. Explain every retained borrow during a cancellation acknowledgment.

Treiber-style stacks and managed-runtime tables revisit identity reuse. An address
may become valid again while an old observer still associates it with a different
object. Compare generation handles, hazard publication, epochs and ownership
transfer under the actual participant and retention limits. A lock-free progress
claim is separate from per-thread starvation freedom and memory reclamation.

Compare contention, false sharing and placement hypotheses using equal work and
actual affinity constraints. A larger instruction count with lower elapsed time
can be consistent with reduced waiting; it is not automatically a contradiction.
Use deterministic protocol tests, thread diagnostics and native measurements as
different evidence. An unavailable sanitizer runtime is not a passing result.

Transfer changes producer count or allows cancellation while a borrower is delayed.
Require a new ownership proof, retained-memory bound and a counterexample showing
why the earlier single-producer or immediate-reclamation assumption fails.

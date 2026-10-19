# T52.07 assessor analysis

Keep this reference separate from the learner's unsolved source. The periodic
admission result applies to a deterministic synchronous implicit-deadline task
set with one fixed supervisor slot in four. It is not a transplanted Linux
bandwidth test. At the complete joint cycle every task's last deadline has been
checked; with all remaining service zero and the same modulo phases, the next
cycle repeats. A successful prefix shorter than the joint cycle cannot establish
that induction. Reject and inconclusive both preserve the old installed set.

The capacity2 burst witness accepts0,1,3,6,9, completes0,1,3,6 and leaves9.
All other seven IDs are rejected. It refutes the inference from a feasible CPU
reservation to bounded burst arrivals. The queue owns copied synthetic values;
the fixed96-entry diagnostic ledger is separate from its live capacity. Dropping
a partially executed request keeps the consumed work and records DROP. A stale
result can never become DONE simply because a slot was reused. Latest-value
semantics is a changed application response contract, not a faster FIFO.

The supervisor's reserved tick is selected before application work. Its modeled
progress follows directly from that control path, subject to the controller
itself running and the represented integer-time assumptions. Runtime jobs are
charged to actual service and terminated at declared cost if unfinished. This
provides containment for the modeled overrun; it does not certify the original
periodic cost assumption or an actual ISR/accounting port.

Criticality switching discards pending low work and rejects later low arrivals;
high work retains consumed service, deadline, budget and identity. The example
compares recovery before versus after pending high work drains. Restoring LOW
does not resurrect dropped requests or prove future costs/arrivals valid. The
contract intentionally does not claim a general mixed-criticality theorem.

For the delayed dependency experiment, producer0 owns the mutex and waits for a
free-space token, while consumer1 waits for the same mutex before draining.
The CMX implementation actually records both waits. Cancelling producer0 yields
the retained API's CMX_CLOSED result, then unlock hands the mutex to consumer1.
Giving space now increments the semaphore because the cancelled producer is no
longer enrolled. Cleanup cancels any remaining wait, releases the live owner,
closes registrations and retires both local task records. Merely raising a
priority does not remove either resource edge.

For the I/O variant, io_until100 makes two copied requests ineligible without
consuming CPU, yet both occupy queue slots. At horizon8 the result is explicitly
pending, with later IDs rejected and supervisor heartbeats intact. At deadline20
they expire. This finite eligibility model has no borrowed buffer or callback;
applying immediate terminal cleanup to actual asynchronous I/O requires a
separate cancellation/quiescence proof. A harness timeout is neither that proof
nor graceful recovery.

The independent oracle uses generated explicit periodic job records and a
separate live-ID set rather than consuming C summaries. Its workloads include
out-of-order arrivals and equal-deadline ties. Seven controls discriminate
capacity, dispositions, supervisor, budget, mutex repair, staging and mode
transition. A failed compilation or crash is not the intended negative result.
Strict target execution supplies real compiler/linker/emulator observations,
not physical worst-case timing or an independent learner assessment.

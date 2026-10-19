# Assessor analysis: owned pressure and credit conservation

The simple wrapper delegates numeric semantics to actual `rz_offer` and keeps
owner descriptors in the same physical ring slots. A successful overwrite
uses the old head slot after the core advances its head; dispose that old
owner before replacement. Degrade changes newest only when the new value is
strictly larger; equal values keep the old owner. BLOCK is the sole valid
offer result retaining the caller owner. Other terminal outcomes dispose or
move it. Wrong callback/lifetime preconditions cannot be repaired by testing.

The pump's capacity is four global payload slots, not four queue slots plus
four transport slots plus arbitrary producer-held values. The state partition
gives `free+queued+transport+leased+retired=4`; an additional two pending owner
records give a maximum of six. The ledger `4-consumed+returned` equals usable
free credits even with retirement because a retired generation does not grant
a replacement credit. Using logical slot ownership allows actual MsgHub to
copy only the slot index while the payload stays stable. Removing that index
from the channel is reception, not receiver-capacity release.

Begin owns the pending record before progress examines free capacity. Poll
holds the same mutex until `pthread_cond_wait` atomically releases it. Hence
a release either sees the registered pending operation, or occurs before the
polling predicate recheck; no transition depends on a naked edge notification.
A terminal record remains occupied until result collection, preventing reuse
under a stale waiting poller. A separate polling flag makes duplicate polling
BUSY rather than allowing two waiters to consume one result. Native tests
observe that flag while holding the mutex to establish the waiter has released
it into the condition wait before constructing the duplex state.

Cancellation is selected before closed state during progress. It only acts on
pending owners. Admitted payloads no longer hold the pending scope and cannot
be withdrawn by cancellation of that offer. Closing wakes pending producers
and then drains the existing payloads. Destroy refuses live receiver leases
and uncollected results. This is why destructor count alone is insufficient:
it must be paired with state/credit identity and absence of premature release.

Mutant51 returns PENDING on zero credits before storing the owner/ticket;
the full-pipeline assertion catches the unregistered owner. Mutant52 accepts
the same released generation again; the immediate duplicate return must be
STALE before any later slot reuse. Mutant53 omits overwritten-owner disposal;
per-object counts expose the leak before test teardown. Mutant54 returns
credit during receive; the explicit snapshot must still show zero credits
and both producers pending. These are independent semantic observations,
not timeout/sanitizer substitutions.

In the delayed two-endpoint case, four outstanding grants per direction leave
both fifth sends waiting. Forwarding merely relocates grants. With both actor
threads waiting for sends, no actor releases a received lease. A separate
receive-progress loop closes that wait-for cycle: receive/release each direction,
observe the respective pending send terminal, join and drain. This is an
explicit controlled schedule demonstrating the mechanism, not an assertion
that POSIX guarantees fairness or all deadlocks have been excluded.

The policy experiment intentionally differs from a lossless blocking caller:
it explicitly disposes PENDING input to compare one identical finite offer
schedule. Report those counts as pending-caller-disposed, not accepted/drop
equivalents. The pump experiment separately retains pending owners. An
eight-offer burst gives four initial admissions; overwrite accepts eight while
displacing four, degrade coalesces four, reject/drop distinguish their four
terminal outcomes, and block returns four owned pending inputs. Raw queue
output plus disposer traces assess R1/R2; finite identity/wait traces assess
R3; separated workload metrics and independent delayed transfer assess R4.

Per operation, scanning two producers/four slots is bounded; payload disposal
cost is caller-defined and runs under the pump's serialization contract, so
callbacks must not block/reenter. Pthread contention/scheduling, borrowed
payload size and actual MsgHub storage must appear in the cost dossier.
There is no lock-free, arbitrary weak-memory or freestanding claim.

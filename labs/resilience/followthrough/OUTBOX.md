# Required T75.09 successor: durable effects and notification acknowledgments

This is substantive support for X-T75-009-R1--R4 and X-T75-009-DELAYED, composed
with X-T75-008's actual journal. It does not alone complete the transport,
measurement or independent transfer obligations. Implement `src/outbox_follow.c`
against `outbox.h`, and the additive journal operations in `src/journal_follow.c`.
Run `resilience.service_outbox` and its three compiled controls. The separate
service I/O contract supplies actual local partial I/O; the integrated fault
workload must still bring the retry, timer, limiter and hedge producers together.

Use C17 on a hosted 8-bit-octet, signed32-int target. Import the actual RZQueue,
MsgHub and RzjJournal implementations. No database, durable queue, serialization
or idempotency library may implement these mechanisms. Python is a host oracle.
The original RZService remains a serialized volatile baseline with its original
identity and tests; this stronger contract does not rewrite its acceptance.

## Representation, ownership and exact operations

A noncopyable RzsOutbox borrows one exclusively owned journal, a distinct ACK
device and channel0 of a live MsgHub. The two devices have independent storage;
distinct callback contexts alone cannot prove that. All outbox/journal operations
and callbacks are externally serialized. No other code accesses the device or
channel0, mutates the journal, invokes callbacks reentrantly, or destroys these
objects while an operation runs. RzvService supplies the mutex for its adapter.
Callbacks are synchronous, do not retain byte spans and obey JOURNAL.md's
partial-progress and uncertainty rules. No callback may escape with longjmp or
thread cancellation. Control, descriptor and output storage do not overlap.

Keep at most four unacknowledged events, in journal sequence order. Each carries
the original sequence, epoch, client, request ID and result, plus state WAITING,
SENT or DELIVERED. WAITING events also occur once in the actual RZQueue. SENT
means the sequence was copied into channel0; DELIVERED means select removed it.
Neither transition returns capacity. Only durable acknowledgment removes an
event and returns its credit. No event owns a heap allocation. The original
effect record is also the durable notification record, so there is no second
effect-to-outbox write window.

`open(journal, ack_io, hub, out)` requires an already recovered journal and a
fresh empty, open buffered channel0 (capacity1..4). It reconstructs all committed
new-effect records and matches ACK records against that exact ordered list.
Duplicate/replay journal records are not new notifications. It rejects a skipped,
reordered, repeated, future or malformed ACK, more than four remaining events,
or a nonempty/closed/stopped channel. It restores every remaining event to
WAITING, even if a prior process delivered it. Validate the complete prefix
before truncating/flushing a short uncommitted tail. Publish output only after
successful recovery; errors preserve output but may have repaired the device
tail. Output owns no previous live service; reset requires joined callers and
discarding the old volatile transport under the explicit reset model.

`apply(now, request, out, duplicate)` requires current epoch, client0..1, ID1..15,
delta-8..8, and a monotone journal clock. First stage the actual receiver
operation. A live equal-intent replay returns its original result, appends the
time observation and creates no notification. Different intent is CONFLICT.
A new effect needs one free event credit before the journal write starts. FULL
must leave effect, log, event list and outputs unchanged. The imported journal
can still refuse receiver capacity, log capacity, arithmetic or I/O. An uncertain
write poisons the journal; no later operation guesses whether it committed.
Recover before retrying. Only a committed new effect appends the volatile event.

Example: delta3 under `(epoch1,client0,id1)` commits total3 and event sequence2.
A retry returns3, duplicate=true, and leaves one event. Forward/deliver followed
by reset before ACK restores that same sequence2 for redelivery. After durable
ACK and reset, no event remains. The effect is not repeated in either case.

`forward` calls actual `rz_forward`; FULL/CLOSED/CANCELLED preserves the pending
event and its credit. `deliver(out)` calls actual nonblocking `msg_select` on
channel0 and marks the matching SENT event DELIVERED. `ack(sequence)` accepts
only the oldest DELIVERED event. It durably appends its ACK before removing the
event. An old ACK arriving before redelivery after reset is STALE. This
conservative policy requires a new delivery confirmation, even if the external
consumer remembers a prior delivery. A stopped channel does not erase durable
work; recover into a fresh channel after quiescence.

## ACK wire format and failure semantics

Each ACK record is32 octets: `RZA1` at0..3, zero4..7, BE64 event sequence at8,
BE32 ACK ordinal at16, FNV-1a32 over bytes0..19 at20, `ACK1` at24..27 and the
bitwise complement of the checksum at28. Ordinals begin at1. Maximum128 ACKs;
no compaction or infinite log claim. Write the first24 octets with short-write
loops, flush, write the final8, then flush. Only then publish acknowledgment.
A failed, zero-progress or oversized-progress callback yields UNCERTAIN and
poisons the live outbox. Retain the event and output until recovery determines
the durable prefix. A complete invalid record is CORRUPT, never silently erased.
The checksum detects the selected corruptions; it is not authentication.

The storage premise is exactly JOURNAL.md's ordered durable-prefix model.
Short tails are uncommitted. Failed flushes may have persisted nothing or an
available prefix/complete record. Arbitrary device reordering, malicious storage,
sector atomicity, physical power loss and directory publication are not proved.
The native file adapter uses borrowed descriptors, real partial pread/pwrite,
fdatasync and child exit. It does not make an actual power-cut claim.

## Extended request identity

The baseline receiver forgets entries after a finite window. This service
retains a seen-ID bitset reconstructed from every new effect since the last
durable rotation. If an ID was used but its result has expired, a replay is
STALE; it cannot become a new effect. Live replays still return the original
result. This sacrifices old-result availability to preserve no-repeat safety.
The identity is `(durable epoch, client, ID)`, not a raw byte ID alone.

Add `rzj_rotate`: journal kind3 has the same checked, durable next-fence format
as kind2 issuance, and additionally clears receiver dedup entries after commit.
Recovery performs that same transition. Kind2's original behavior is unchanged.
`rzs_rotate` requires no unacknowledged events and, as a caller prerequisite,
all requests and delayed workers from the old epoch quiescent. It commits kind3,
then clears the seen bitsets and publishes the new epoch. Old-epoch inputs are
STALE. This is coordinated single-authority rotation, not concurrent consensus
or a secret capability. Never reset or clone the durable authority to obtain
fresh IDs. Tests use45 logical requests over three epochs and repeat old IDs
after both expiration and restart. Finite journal capacity eventually refuses
further progress; no wrap or unbounded retention is hidden.

`rzj_record` reads a sequence from an already recovered, exclusively owned,
unchanged journal and returns decoded metadata. It checks its sequence/checksum
before output. Its trust premise is that full validation happened at open;
it is not a substitute for validating arbitrary attacker-controlled log bytes.

## Deliverables, assessment and transfer

Provide D1 a state/ownership proof and separate linearization points for effect,
enqueue, delivery, durable ACK and credit return; D2 the implementations and a
byte-level cut ledger; D3 exact native/model/mutant outputs; D4 a changed-condition
consumer policy and all13 modern cost dimensions. Recovery scans at most128
records; live event search/shifting is bounded by4. Count journal writes and
flushes, receiver work, two separate storage bounds and notification backlog.
Discuss the throughput cost of synchronous flushes and global serialization.

Required tests include the five-slot pressure counterexample with expired
receiver entries, replay after delivery-before-ACK reset, duplicate/stale ACK,
all31 partial ACK tail lengths, complete corruption, closed channel recovery,
epoch rotation refusal, and exactly one effect under a lost reply. Enumerate
414 selected write/flush cuts across effect, ACK and rotation, in two persistence
outcomes. A native child commits and delivers then exits before acknowledgment;
the parent recovers, replays the request and receives the same event identity.
The Python oracle compares79,896 independent logical histories over two channel
capacities, including five-step exhaustive schedules and longer mixed histories.
Byte-level faults and process tests are separate from that logical oracle.

The three controls remove capacity preflight, omit final ACK flush, and allow an
expired used ID to apply again. Passing requires the intended assertion to reject
each compiled control, exact status/output/persistence behavior, unchanged output
on refusal, and no missing or invented event. A finite pass does not prove all
device faults, unbounded histories or independent learner mastery.

Rubric: R1 assesses the representation and imported producer assumptions; R2
assesses reservation, uncertainty and epoch transition invariants; R3 requires
independent oracle/cut/mutation evidence with stated limits; R4 requires a delayed
design where reset occurs after external delivery and before ACK persistence.
Superficial work that stores a volatile `sent` bit or equates send with durable
delivery fails. Strong transfer must derive whether the external consumer needs
its own durable dedup table. The supplied policy is at-least-once delivery with
stable event identity; it does not promise exactly-once arbitrary side effects.

Hints: first enumerate the five separate states; next reserve credit through
ACK, not through channel send; then derive recovery from effect records before
designing the ACK format. The private OUTBOX_ANALYSIS contains the solution and
counterexamples. Security scope is original local teaching programs, unlinked
temporary files and synthetic integers. An adversary may supply malformed frames,
replays or selected fault cuts; harmless success for a flawed control is a wrong
counter, lost event or premature credit. No external target or privilege claim.
Author executions, learner work, independent assessment and physical validation
remain distinct evidence states.

# Durable inbox/outbox and finite local delivery

This is the required changed-condition implementation for X-T76-002 D4 and
X-T76-002-DELAYED, and the external-effect counterexample for X-T76-001 D4.
The original exercise identifiers and four objective-specific rubrics remain
in force. It adds actual durability and process boundaries to the retained
serialized in-memory projection; it does not alter historical acceptance.

## Inputs, prerequisites and composition

Complete X-T76-001, its durable adapter, and PR-T76-02's original projection.
Retain actual `storage.pages` (PR-T47-10), the T20 `codec.records` implementation,
the imported DOD HAMT, and the hosted stream parser/outbox from
`labs/hosted_systems/stream_depth`. The latter is the concrete local byte-stream
support used for the `ipc.local` boundary, not a claim that a contract heading
provided a general reactor. `testing.faults` supplies the callback-cut model;
the public `test_support.c` is an explicit original adapter for this experiment.
Learners must supply their completed dependency workspaces; the assessor build
selects the corresponding private sources explicitly. No database, message
broker or deduplication library implements the studied mechanism.

Use C17 on hosted Linux with POSIX file/socket/process adapters. GCC and Clang
debug/release, address/undefined-behavior and thread sanitizer configurations
are separate host verification builds. The core is externally serialized;
concurrent calls are not admitted. Inputs, controls and output objects are live
and disjoint. Descriptor storage may be uninitialized before construction but
may not conceal a live owner. Borrowed stores and descriptors remain at fixed
addresses. Callbacks are synchronous, nonreentrant and retain no buffers.

## Representation and authority

Three distinct exclusive `struct rpl_log` stores share a nonzero log UUID. The source
event log is also the outbox: every committed event after the acknowledged
prefix remains a delivery obligation. No second insertion can fail between
event commit and outbox creation. A separate ACK log has schema `RPO_ACK_SCHEMA`,
key zero, kind SET and values 1 through its count. Its exact prefix cannot exceed
the source count. The receiver inbox has the source schema. Each log holds at
most sixteen immutable events; no position or UUID is recycled during its life.
Selecting a fresh UUID is an external provisioning obligation, not randomness
or authentication supplied by this API. Distinct `store *` values must refer
to distinct backing devices; pointer inequality alone cannot verify that fact.

The authoritative receiver effect is its durable event prefix. Its query model
is the sum rebuilt through the actual `rp_project` producer. It is not a separate
file or callback that must mysteriously share a commit with the inbox. Given
sixteen int32 values, the mathematical sum fits int64. `rpo_projection(log,k)`
returns the sum of the first k events and preserves its output on rejection.

## Operations and outcomes

`rpo_sender` validates the source/ACK relationship before publishing the two
borrowed pointers. `rpo_receiver` admits a live inbox and its projection.
`rpo_next` returns the event after the ACK prefix; `RPO_EMPTY` preserves output.
All sender calls require the source to remain append-only, and ACK updates must
go through this sender. No external mutation may invalidate these invariants.

`rpo_receive` checks UUID, schema, position1..16, key0..31 and kind SET or recorded
query response. The next event is durably appended before a reply is published.
An old identical `(position,key,kind,value)` returns `RPO_REPLAY` with the sum at
that historical prefix, even if the inbox has since advanced. A conflicting old
event returns `RPO_CONFLICT`; a future gap returns `RPO_GAP`. These rejections
preserve the output and inbox. Admission errors are `RPL_INVALID`, wrong identity
is `RPL_IDENTITY`, and poisoned or active stores are `RPL_STATE`.

`rpo_ack` accepts only the next position, exact source UUID and independently
rebuilt expected sum. It durably appends the corresponding ACK. A delayed old
reply while another event is pending conflicts; after all source events are
acknowledged the result is EMPTY. A forged correct reply is indistinguishable
from a genuine one without an authenticated trusted receiver. Neither a checksum
nor knowledge of the expected sum establishes that the remote effect occurred.

The lower durable-store contract governs partial writes. On failure, volatile
published count rolls back, but accepted persistent bytes might contain the
commit. Poison requires reopen/recovery before any further operation. A failed
receive preserves reply output; the caller must retry the same immutable event
after recovery, not infer that it was absent. A failed ACK likewise does not
authorize inventing a new idempotency key. The disk model and native adapter
never claim that returning an error rolled back durable storage.

## Concrete stream format and ownership

A request is the exact64-byte RPL1 event record. A reply is24 bytes: ASCII RPO1,
8-byte unsigned big-endian UUID, 4-byte position with high three bytes zero,
and 8-byte two's-complement big-endian total decoded arithmetically. Position
must be1..16 and UUID nonzero. No C struct representation is transmitted.

Compose the actual stream producer with exactly eight payload octets per frame
and one length octet. Requests contain eight frames (72 wire octets); replies
contain three (27 wire octets). End each direction with EOF. Empty, short,
noncanonical segment lengths, oversized, trailing, and truncated records reject
without publishing a partial output. Any failed connection is discarded; retries
use a fresh parser and resend the complete logical event. `rpo_wire_read` uses a
64-byte staging buffer. `rpo_wire_write` uses eight stack message descriptors
and the actual bounded stream outbox; callbacks borrow its wire only during the
call, and all messages are delivered or cancelled before stack exit.

Read callbacks return a positive prefix, zero for EOF, minus two for retry or
another negative error. Write callbacks return a positive prefix, zero for
retry or a negative error. The limit counts every attempt, including retries;
callbacks themselves must have bounded waits. The native socketpair adapter
uses poll with a one-second bound and fixed small read/write chunks, then
shutdown(SHUT_WR). No external address is contacted.

## Required reasoning and deliverables

- D1: Distinguish the authoritative immutable events, rebuildable query model,
  durable delivery obligations and trusted reply. Draw all three ownership and
  authority boundaries. Explain why model separation is independent of delivery
  guarantees. Read only the registered Fowler passages; the local byte protocol
  and failure schedule are original curriculum artifacts.
- D2: Implement the six protocol and four transport/codec operations from the
  declared producers. Prove by induction that ACK and inbox histories are exact
  prefixes, and that duplicate replay preserves the receiver count. Explain
  historical-prefix replies and why source mutation after admission is invalid.
- D3: Predict then execute the five anchor tests, four receive/ACK controls,
  two stream controls, two failure-publication and two native-retry controls,
  every37 callback cut under discard/persist modes for
  both receiver and ACK stores (148 cases), and the independent list oracle.
  Preserve output sentinels, actual process statuses, and source/toolchain pins.
- D4: Run the native source-after-commit death, receiver-before-reply death and
  sender-after-reply-before-ACK death, then recover and retry. Rebuild the total
  five from events seven and minus two exactly once each. Execute the original
  separate-effect specimen: durable increment, terminate before inbox commit,
  retry increment; the synthetic counter becomes two. Explain precisely why
  correcting the inbox cannot undo an unrelated external authority.

Pass only when all required reasoning, independent expected states, unchanged
rejection outputs, recovery behavior, dependency evidence and changed-condition
work agree. A passing anchor does not replace the original four rubrics or the
delayed unaided attempt. Superficial completion merely obtains total five once;
competence explains every retry and failure boundary; independent mastery must
rederive those arguments under the unseen variant without copying this solution.

Graduated hints: first identify the one authority that can rebuild the effect;
then ask whether an event can be committed without becoming a delivery obligation;
finally compare a reply's position with the ACK count before appending anything.
The separate assessor analysis is outside the learner package.

Changed assumptions: cut the connection at every wire prefix; replace loss
with duplicate and conflicting old records; change the receiver identity while
keeping the numeric position. For the delayed variant, let the source and inbox
advance while an old identical request and its reply are delayed. Reconstruct
the historical-prefix response and prove that it cannot acknowledge the next
source event. The public receive/ACK tests supply the changed-state support;
the delayed attempt must explain the interleaving without copying the solution.

## Security laboratory boundary

Targets are only these original stores, temporary unlinked files and AF_UNIX
socketpairs. Synthetic attacker-controlled records may alter UUID, position,
kind, value, framing or delivery order. The defect is missing identity, prefix
or duplicate admission, observable as a wrong local count/result or the original
counter becoming two. Exploitability requires those precise missing checks and
delivery privileges; there is no claim of code execution or privilege escalation.
Repair the checks and atomic authority boundary, rerun rejection and recovery
regressions, and retain the lack of authentication, permanent partitions, finite
capacity, provisioning and external-effect atomicity as explicit limitations.

## Cost and enforcement evidence

| Dimension | Mechanism-specific obligation |
| --- | --- |
| Representation | Record sizeof/alignment of pointer-only sender/receiver and reply; wire sizes are independently fixed72/27, never sizeof. |
| Reachable storage | Three distinct67-page stores, three bounded logs, ACK prefix and transient stream descriptors; no hidden network queue authority. |
| Lifetime | Fixed live controls through calls; fresh parser after failure; stores survive sender/receiver descriptors. |
| Mutation/aliasing | External serialization and disjoint live objects are caller duties; runtime prefix/identity checks do not validate arbitrary pointers. |
| Allocation | Core performs no heap allocation; actual store/HAMT use fixed arenas; native descriptors/files can fail before protocol admission. |
| Movement/copying | Requests/replies copy canonical octets and stage output; the stream writer borrows stack messages synchronously only. |
| Destruction | Core descriptor teardown has no recursive work; native close waits for no users and process waits are bounded by harness timeout. |
| Dispatch | Direct producer calls plus byte I/O callbacks; inspect object disassembly and unresolved helpers for the selected compiler. |
| Concurrency | Serialized core; separate native processes communicate by bytes and durable files; no lock-free or memory-model proof. |
| Failure | Enumerate every write/flush callback boundary plus three actual death stages; prefix model is not every possible physical persistence ordering. |
| Compilation | Record clean build command/time and source dependency closure; successful compilation proves neither ownership nor authentication. |
| Binary/stack | Measure actual text/data and stack-usage files; stack messages/staging and imported store frames must be included. |
| Boundary | C17 arithmetic, POSIX calls, exact byte protocol/schema, local transport and successful-flush persistence premise are separate contracts. |

Finite campaigns are author execution evidence. Non-author review, delayed
learner assessment and physical power-loss behavior remain separate. The final
published C17/C23 source-access gaps are not repaired by a C11 draft citation.

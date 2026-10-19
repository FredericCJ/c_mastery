# Checkpoint changed-condition contracts

These are executable elaborations of the existing `AX-PR-P1-*`, `AX-PR-P2-*`,
`AX-PR-P3-*` and `AX-PR-P4-02-*` criteria and their `-DELAYED` variants. They do
not create replacement IDs, reduce any original requirement, or award a delayed
attempt. The fourteen-field contract, individual objectives, prerequisites,
rubric, deliverables, graduated hints and minimum seven-day delay remain in
`curriculum/v5/expansion/legacy_assessments/PR-Pn-pp.json`. Read that record and
the shared assessment contract before the relevant section below. The new
readiness register maps each original ID to its precise implementation/tests.

Use hosted C17, 8-bit bytes and the exact-width integer types on Linux x86-64.
The memory/protocol algorithms use the supplied primitives, never a container
or parser library. `malloc/free` are permitted only in the allocated-storage
backend and test harness. The modern comparisons explicitly import completed
`PR-T66-01`/`PR-T65-03` components and their contracts. OpenSSL3 is a separately
declared hosted HMAC primitive for the authentication exercise; implementing
the protocol is the task, not inventing a cryptographic primitive. None of
these hosted additions enters the strict entry/firmware image.

Every pointer denotes a live, suitably aligned object of sufficient extent.
Control objects and outputs are disjoint unless stated otherwise. No numeric
check can establish the provenance/lifetime of an arbitrary pointer. API state
begins zero-initialized, access is externally serialized, and callbacks neither
reenter nor exit nonlocally unless a separate adapter explicitly says otherwise.

## Allocated records, linked nodes and relocation — P1

Implement `memory.h` in a copy of `memory_stub.c`. `CPXStore` has 1..4 records,
1..capacity nodes, a stable caller-owned control object and two separately
allocated arrays. The backend returns genuine allocated storage suitable for
the requested C objects; allocation failure acquires nothing. Records have a
value, a 0..16-byte payload and its length. Stores through `CPXRecord` lvalues
establish the allocated objects' effective type. Alignment of a declared byte
array alone would not establish that type. The node array is independent, so
record availability does not imply admission availability.

Initialization acquires records, nodes and callback registration in that order.
Each successful subscription returns a nonzero cookie that cannot collide with
an outstanding callback, including callbacks from earlier store lifetimes.
Failed subscription leaves no registration. Test failure at all three stages;
release each acquired allocation exactly once in reverse order, leaving the
public owner uninitialized on failure. Release callbacks cannot fail, block or
reenter. Registration failure and asynchronous unregister are distinct.

Handles are `(slot,generation)` under one stable store owner; generations1..3
retire instead of wrapping. A raw pointer returned by lookup is borrowed until
release, relocation or destruction. `pin` prevents all three. Each queued
node owns one pin. Pop releases that pin and transfers the record handle to its
caller, who still must release the record. A callback must separately pin its
borrow; cancellation is not completion. Queue nodes link by indices, never
borrowed record addresses. Duplicate enqueues are permitted only as separately
pinned references; each pop releases exactly one pin.

Batch admission uses accepted-prefix semantics. On node failure, release the
current unpublished record; prior queued records remain accepted. Internal
generation consumption on this rollback is visible and bounded. Inject every
node-acquisition ordinal, including node capacity2 with record capacity4.
Halve both capacities and submit the same four records; do not shrink the input.
The accepted counts change4→2 while FIFO order and cleanup remain invariant.

Relocation first allocates an entire new record array and copies typed objects,
then releases the old array and publishes the new base. Failure preserves the
old base and handles; pins refuse relocation. Handles remain meaningful because
slot/generation/control identity remain unchanged. **Do not evaluate an expired
borrow even merely to compare its address after successful relocation.**

`grow` increases record/node capacity up to4, acquiring both replacement arrays
before any publication. Refuse live pins and failure at either acquisition;
preserve existing handles, contents, registration and capacity on failure.
Initialize new slot generations to1. Its cost includes copying old records and
nodes, not just changing a capacity field. `CPXLease` separately owns an arena
pin under a nonzero callback cookie. Cancellation retains it; only a matching
quiescent completion drops it, and duplicate completion cannot unpin twice.

Closing refuses outstanding pins. Otherwise request unregister once. A false
response means pending, retaining both arrays and the cookie. Only the matching
quiescent acknowledgement releases them; repeated close while pending does not
resubmit, and duplicate/stale acknowledgements cannot release again. A bounded
wait is a caller policy, not permission to free while callbacks remain possible.

For the fixed-capacity distribution experiment, compare four1-byte and four16-byte
records against a64-byte arena with three-byte headers/alignment1. The pool has
64 payload bytes but additional record metadata and nodes: report its **actual**
allocation sizes instead of claiming equal total memory. The small arena uses16
bytes; the large workload accepts three records and retains57 bytes until reset.
The four-slot pool accepts four in either case and releases logical records
individually. Distinguish internal unused payload, metadata and lifetime retention.

## Asynchronous ownership and progress products — P1/P2

Implement `adapters.h` using the learner's completed `OwOwner` and `er_transfer`.
An owner holds three blocks. Unregister may complete later, so the adapter must
retain the complete owner until a matching quiescent acknowledgement. Calling
the original nonfailing destructor immediately is a contract error. Compare
control bytes, unsubscribe requests and release calls; C still permits ownership
duplication by structure assignment and supplies no automatic destruction.

Map one `CPWWrite` invocation to an `er_progress` product while preserving the
accepted offset and a separate first backend cause/cleanup cause. `ER_OK` means
that invocation made progress, not necessarily that the whole transfer ended.
On3,EINTR,2,EIO, retain offset5/cause5; subsequent cleanup9 cannot overwrite5.
Terminal state does not call the backend again. The core works with no logging
adapter. The product alone does not carry the full original cause, nor does it
automatically unwind, check exhaustive matching or promise another language's
layout. Report actual size, static generated branches, callbacks and payload
copying under this narrower equivalent contract.

The supplied local pipe test fills a nonblocking pipe to EAGAIN, observes no
write readiness, drains it, observes readiness, and sends the same nine bytes
once. Each core step invokes the writer at most once. Read/write/errno belong
to the hosted adapter. Capture errno immediately; raw negative Linux errors
and libc's -1/errno are different interfaces. The EBADF control closes only its
own write descriptor under serialized descriptor allocation. It does not send
to a closed reader, change process signal handling or contact a service.

The dependency probe separately builds native inspection images with and
without LTO/stack protection. Audit generated disassembly, undefined symbols,
all resolved providers and callback effects. A fully resolved `malloc` remains
a forbidden provider; an indirect callback can block even if the caller's own
object has no undefined symbol. Stack-protector helpers and guard initialization
are explicit dependencies. These inspection images are not executed without
their missing startup/runtime contract.

## Version evolution, overlap and consumer schedules — P3

Implement `protocol.h` in a copy of its stub. Keep v1 unchanged. v2 adds a flags
byte after the six-byte common header; only flags0/1 are valid. Flag1 adds a
four-byte little-endian timestamp before payload. CRC covers version through
the last payload byte, including every extension byte. Total size is10+n for
v1,11+n for v2 without timestamp and15+n with timestamp. Old v1 parsers reject
v2 after its version byte; they do not reinterpret extension bytes as payload.

Compile the same source with maximum payload16 and64. Runtime limit0..maximum
may narrow accepted lengths but does not shrink the C representation. Derive
candidate storage `maximum+15`, message storage and queue cost separately.
On rejection restart one byte after the sync, preserving nested candidates.
Worst-case shifting/CRC work is O(NB), B=maximum+15, rather than O(N) independent
of the configured maximum. Pending output prevents any further input consumption.
Queues admit1..4 copied messages; refusal preserves pending state.

`inplace` starts with payload in bytes[0..n). Validate all numeric/edition/capacity
conditions before mutation, then move payload toward higher addresses in reverse
order and write header/CRC. This permits exactly that overlap shape; ordinary
`encode` keeps its disjoint input/output contract. Insufficient capacity preserves
both buffer and written-count sentinel. Inspect every source access in the proof.

The deterministic corpus feeds raw bytes **and consumer-refusal decisions**.
Every run starts from clean parser/queue state, caps input256 bytes and decision
schedule256 turns, and drains after that schedule. Test chunk1/7/256, capacities1/3,
80 initial refusals and decisions derived from corpus bytes. Compare complete
accepted frames with the separate Python whole-buffer/zlib oracle. Retain actual
minimized failures; no timeout or crash substitutes for the intended assertion.

## Authentication and replay — P3

Implement `auth.h`; `openssl_mac.c` is a supplied primitive adapter. The original
local target uses only synthetic keys/messages. Format: four domain bytesP3A1,
key ID32LE, epoch64LE, sequence64LE, payload length16LE, 0..64 payload bytes,
then32 HMAC-SHA256 bytes. Authenticate **all26 header bytes and payload**. A
payload may be a complete original P3 frame; its public CRC is not authentication.
Use the inspected OpenSSL `EVP_Q_mac`/`CRYPTO_memcmp` contracts and the RFC4231
case1 known answer; check all primitive failure results and exact tag size.

The trusted receiver is provisioned with one32-byte key, key ID and epoch. Admit
strictly increasing sequence numbers only after successful authentication and
capacity checks. Publication and replay-ledger advancement form one serialized,
nonfailing commit. Overflow never wraps into an accepted older sequence. Reject
wrong key/epoch, tampering, truncation and duplicate/stale sequence with output
and state unchanged. Simulate primitive failure and output refusal before commit.

The attacker controls packet bytes and replay order, knows the public format and
CRC algorithm, but lacks the configured key. A harmless defect demonstration
changes sequence1→100 without a new tag, or poisons the high-water mark using a
bad tag. Repair by authenticating the complete header and delaying all state
publication; regress altered payload plus recomputed CRC, wrong keys, replay,
maximum sequence and failure-side effects. The keys are public test fixtures,
not credentials. No network, external target or secret data is used.

This is authentication/integrity, not encryption, identity enrollment or durable
anti-replay. Resetting the volatile ledger under the same key/epoch accepts an
old packet again; the test demonstrates that limitation. Production key rotation,
entropy, revocation, persistent epochs and denial-of-service policy need separate
contracts. The library's tag comparison contract does not make every protocol
branch or allocation constant-time.

## Durable acknowledgement composition — P3

Implement `durable.h` using completed P3 framing, `receipt.h` and the actual
retained `store` component. This later transfer requires their accepted dossiers;
it is not an extra prerequisite for the first P3 attempt. Admit only a nonzero
16-bit sequence and an exactly four-byte LE signed delta. Provision client1,
epoch1 on a trusted fresh store. IDs must increase; wrapping the wire sequence
requires a separately authorized epoch/key policy, not silent reuse.

Queue removal transfers a full message into a held slot; it does not acknowledge
durability. `commit` calls the receipt transaction. Only successful commit marks
durable and exposes the saved result. On storage/indeterminate failure retain
the held message, leave the caller's output unchanged and reopen the storage
owner before retry. An exact retry returns the retained receipt, never applies
the delta twice; changed-payload reuse conflicts and evicted IDs remain stale.

The supplied two-image device model distinguishes written working pages from
durable pages. Test all37 write/flush cuts with discarded pending writes and
with persistence despite a reported flush error:74 compositions. Observe that
no write/flush callback sees a durable acknowledgement. Reopen after every cut,
retry +7 and require effect7, never14. This finite model does not assert physical
power-failure atomicity. The imported receipt campaign separately exercises
tears/reordering and actual guest I/O; cite those observations at their own scope.

## P4 analytical timing and observation protocol

Implement `halfduplex.h` around the completed `CPFTx`. The direction callback
is serialized, nonfailing and nonreentrant; it represents the dedicated
transceiver's driver-enable output, not an arbitrary board pin. Assert direction
before the first byte. TXE alone cannot release it. After a separately observed
TC, retain direction for1..32767 configured ticks, using the half-range elapsed
contract, then release exactly once. No real interval between observations may
reach the ambiguous half range. A stale early TC sampled before writing the last
byte cannot start turnaround. The public test crosses65535→0. The host boundary
test is not an electrical measurement; an actual board adapter must supply its
inspected pin mapping, polarity and transceiver turnaround requirement.

Use `fixtures/legacy_checkpoints/transfer/P4_timing.json` and independently derive
the recurrence before invoking its oracle. Let the considered job need C CPU
time, block once for at most B, and face one higher-priority sporadic interrupt
with execution H, minimum release separation T and bounded release jitter J.
Assume no self-suspension, another interfering task, multiprocessor interference
or unaccounted overhead. For a candidate completion interval R, at most
ceil((R+J)/T) releases interfere. Starting C+B, iterate that workload bound until
a fixed point or the declared deadline is exceeded. Explain the carry-in/release
model needed for that ceiling; these synthetic inputs are not measured WCET.

The baseline converges50→74→74 microseconds. More interrupt cost gives86; the
overloaded case exceeds200. Separately count arrivals during a closed74us
blackout at minimum25us spacing: three items fit a four-slot SPSC ring's three
usable slots. UART18-byte frames at8N1/115200 need1562.5us each;1000 frames/s
require180000bit/s and cannot be stabilized by a larger finite queue. The20ms
finite burst yields12 completions and peak9 outstanding under departure-first
ties. Distinguish queue occupancy from the in-flight item.

For physical follow-up seal clock tree, compiler/image hash, interrupt priorities,
sample rate, trace pin/logic-analyzer setup and trigger definitions. Collect
arrival→ISR entry, entry→publication, publication→UART first byte, and last-byte
write→TC separately. Account for timestamp wrap, lost events, trace overhead,
DMA contention and maximum masked intervals. Change optimization and clock
without reusing old cycle→time conversions. Report raw traces and observed
maxima; finite maxima are not automatically WCET bounds. No board flashing or
physical timing is performed by these host tools.

## Verification, hints, assessment and evidence

Required deliverables remain D1 reasoning/invariants/cost, D2 original learner
implementation, D3 reproducible positive/negative evidence and D4 changed-condition
transfer after the original delay. The provided `*_tests.c` are public checks;
private references and technical analysis remain under `assessor/`. Exported
`*_stub.c` deliberately return not-implemented. Do not copy the private reference
into a learner workspace or label an author execution as a learner attempt.

Run GCC/Clang debug, optimized and ASan/UBSan profiles. Each semantic mutant must
compile and fail its named assertion; build failure, timeout, wrong marker or
sanitizer-only failure is rejected evidence. The transfer runner pins direct
sources and imported dependency sources. Static conditional-branch counts,
structure sizes and callback counts are distinct cost observations, not universal
performance results. The independent oracle cannot prove absence of every defect.

For a learner run, compile the selected `*_tests.c` with the learner's copied
`*_stub.c` after implementation, using C17 and `-Wall -Wextra -Wpedantic -Werror`.
Add this directory and the completed checkpoint `include/` to header paths.
Memory imports the completed `checkpoint_memory.c`; protocol imports
`checkpoint_depth.c`; halfduplex imports both that file and
`checkpoint_firmware.c`. Adapters additionally import the accepted ownership
and errors implementations/headers plus `checkpoint_writer.c`. Durable imports
the accepted receipt implementation/header and storage implementation/header.
Auth imports the supplied `openssl_mac.c`, completed `checkpoint_depth.c`,
OpenSSL3 headers and `libcrypto`. Record all imported source hashes and acceptance
dossiers. Keep these commands distinct from the author-only reference runner.

Hints, in order: draw ownership/publication edges; identify the first fallible
operation after each acquisition; choose one rejection and predict every byte,
counter and handle that must remain unchanged. For authentication, ask which
field changes routing/replay policy before choosing the MAC extent. For timing,
separate work demand, service rate and finite-burst storage.

Use the original per-objective rubric: level1 nominal traces only; level2 requires
the revised representation, invariants, failure behavior and discriminating
evidence; level3 additionally needs a fresh unaided changed-condition solution
and counterexample to an overstated guarantee. A premature release, mutated
failure output, lost frame, forged accepted packet or early durable acknowledgement
is a noncompensable failure. Review/test/physical/learner status is recorded
separately in the readiness register; no finite test suite certifies absolute mastery.

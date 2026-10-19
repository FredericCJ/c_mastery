# Logical redo and applied-operation authority

## Prerequisites, imports and environment

This required changed-condition exercise supplies
`AX-PR-T47-09-T47.09.b-DELAYED`: replace after-images with logical operations
and design sequence/deduplication metadata. First complete PR-T47-01,06,07,08
and09, including the byte codec, commit uncertainty and repeated-recovery
arguments. Use the actual `store_io` declaration and your completed bounded
failure medium from `../durability/medium.h`. Both public declarations/stubs
ship with this lab; the reference harness imports the actual medium explicitly.
This is a new three-page counter protocol, not a CST2 B+tree or an ARIES
implementation. A logical ADD must be re-executed during recovery; the WAL
does not contain its resulting counter value.

Implement the public `logical.h` API in ISO C17 with exact64-bit integers,
eight-bit bytes and hosted4096-byte page callbacks. GCC/Clang debug, optimized
and address/undefined-behavior sanitizer configurations are separate evidence.
Host Python struct/zlib are permitted for the independent oracle; the target
implements serialization, signed decoding and checksum from primitives.

## Representation and ordering

An exclusive image contains checkpoint pages0/1 and command page2. Each SLR1
record is4096 bytes: little-endian magic0x534c5231 at0, uint32 kind at4,
uint64 operation ID at8, uint64 field at16, signed64 delta at24, zeros32..4091
and reflected CRC32 at4092. CRC uses polynomial0xedb88320 with initial/final
XOR0xffffffff over the preceding4092 bytes. Kind1 checkpoint stores the current
total as the field's modulo-2^64 wire representation and the last applied delta.
Kind2 command stores the predecessor ID as its field, exactly ID-1. Commands
have nonzero IDs. Initial checkpoint is ID0,total0,delta0. Decode negative
wire values arithmetically; do not assume an out-of-range unsigned-to-signed
conversion has a portable ISO C result.

Formatting a fresh disposable image invalidates the command and flushes, then
writes each initial checkpoint with a flush. No old handle survives format.
Opening constructs fresh unusable control until all required work succeeds.
Select the greatest valid checkpoint ID; reject differing valid checkpoints
with the same ID. Ignore an older valid command. For a command equal to the
selected ID, require the stored last delta to match, then reclaim without
reapplying. For a newer command require exactly the next ID and matching
predecessor, check addition, write the alternate checkpoint and flush before
reclaiming the command and flushing. A valid checksum alone cannot authorize
a skipped predecessor or overflowing addition.

Application admission accepts only the next ID or an exact duplicate of the
latest ID. A latest-ID/different-delta request is SL_CONFLICT; older IDs are
SL_STALE and gaps are SL_GAP. ID0 is invalid. This is a single sequential
client policy within one image lifetime. It is not a UUID scheme, multi-client
ordering service or unlimited historical receipt store. A latest duplicate
returns the original total without writes or flushes. The caller must retain
the same ID/payload after a lost reply; using the next ID denotes a new effect.

A newly admitted operation first finishes any previous checkpoint, then
invalidates old command bytes and flushes. Encode only the new ID, predecessor
and delta, write the command and flush. Publish the result/control only after
that flush. Checkpoint writes the complete total and applied ID together to the
alternate slot, flushes, then invalidates the command and flushes. Recording
the total without its applied ID would allow a repeated ADD; recording the ID
without the total could lose the effect. Both belong to one sealed checkpoint.

## Ownership, failure and resource contracts

The caller serializes the live exclusive control and backend. Constructors
require fresh storage, outputs are live and disjoint from control/backend, and
callbacks cannot reenter, retain borrowed buffers or mutate state between
inspection and publication. Successful flush persists all accepted writes;
errors may occur after partial or complete persistence. Selected unsynchronized
tears are modeled separately from arbitrary corruption of acknowledged bytes.
CRC is not authentication and collisions remain possible.

Admission errors and integer overflow preserve output/control and issue no I/O.
A failure finishing a previous checkpoint poisons the handle before the new
command is submitted. Failure invalidating the old command is SL_IO; failure
writing/flushing the new command is SL_UNCERTAIN. Both preserve result output
and poison the handle. Reopen before further use and retry the same identity.
Checkpoint errors also poison; a failed constructor exposes no usable control.
Reading poisoned state returns SL_STATE. UINT64_MAX never wraps: the latest
duplicate and checkpoint cleanup remain legal, ID0 is invalid and lower IDs
are stale. Arithmetic overflow returns SL_FULL without treating it as a
capacity or transport error.

The durable state uses three4096-byte pages and fixed-size control. Command
append after a settled checkpoint costs two writes/two flushes; checkpoint
costs two writes/two flushes. Recovery reads three pages and either installs
one new checkpoint before reclaiming or merely reclaims an already-applied
command. No memory grows with operation count. The tradeoff is explicitly
limited historical duplicate response: an old ID is rejected, not replayed.
There are no external side effects to compensate in this counter model.

## Workloads, independent checks and controls

Start at ID1,total10. Submit ID2,delta7, expecting17. Run all four write/flush
cuts under the medium's seven persistence choices for commit and checkpoint.
Commit cut1/2 is SL_IO;3/4 is SL_UNCERTAIN. In the selected medium, even/all
persistence at cut3/4 yields17 on reopen; other choices yield10. Repeating
ID2,delta7 after reconciliation yields17 in either case, with no second effect.
Checkpoint interruption always recovers17. Cross every first/second boundary
under each first-cut persistence mode (112 histories), interrupt recovery
again with all pending writes persisted, then reopen and verify17.

Enumerate every sequence of up to six deltas from {-1,0,1}:1093 histories.
Reopen after every operation and retry its exact ID once. Independent Python
integer arithmetic supplies every expected sum. Separately compare the complete
4096-byte command encoding for ID2,predecessor1,delta-7; it must contain no
after-total. Check checked INT64_MIN/MAX transitions, a valid maximum-ID disk
fixture, stale/gap/conflicting requests and failed-read output. Reseal a skipped
predecessor, an overflowing command and equal-ID/different checkpoint; reject
before writes. Inject all three initial read failures including partial copies.

Compile five defined defect controls: omit final flush; reapply an already
checkpointed command; publish output before the final flush; accept conflicting
duplicate payload; replace checked addition with defined modulo arithmetic.
Each must reach a named semantic assertion, not a process or sanitizer failure.
The blind-new-ID comparison is a positive counterexample: lost reply followed
by new ID3,delta7 yields24, while the correct same-ID retry yields17.

## Deliverables, rubric and changed assumptions

D1: derive the difference between idempotent page replacement and logical ADD,
including the exact point at which recovery proves an operation already applied.
D2: implement canonical records, admission, checked arithmetic, poison/reopen,
publication and cleanup under the stated ownership policy.
D3: retain commands, source identities, complete result traces, all112 repeated
cuts, independent1093-history/28-cut/record oracles and five compiled controls.
D4: demonstrate lost-response reconciliation and explain why this does not
guarantee exactly-once network delivery or arbitrary external side effects.

Superficial work adds an integer and stores a sequence only in RAM. Satisfactory
work passes the declared cases and explains why total and applied ID must
share the checkpoint. Independent assessment reconstructs an unfamiliar replay
trace and repairs a changed identity policy without copying the solution.
Hints, in order: separate the command from its result; consider a cut after
checkpoint flush but before log clearing; then ask what historical information
is available when an older request returns. Assessor implementation/analysis
is separately packaged.

Delayed transfer: admit two named clients with independently sequenced requests,
then design bounded retained receipts and safe eviction; do not reuse one
global latest-ID check. Another required analysis variant adds a noncommutative
operation, such as checked multiply, and derives ordering and replay metadata
before implementation. Neither extension is claimed executed by this packet.
Validation records report the actually executed profiles; the supplied tests
do not establish absolute mastery, external assessor independence or physical
persistence. All images and corruptions are original synthetic local data.

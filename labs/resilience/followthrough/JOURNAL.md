# Required T75.08 successor: journaled effects and fencing authority

The required T75.09 OUTBOX contract adds kind3 coordinated identity rotation
and a read-only decoded-record accessor. Kind2 issuance and every original
journal operation retain this contract. Kind3 clears retained receiver entries
only after durable issuance, preserves total and is replayed during recovery.
The stronger caller quiescence/notification prerequisites belong to OUTBOX.md.

This contract supplies X-T75-008-R1--R4 and X-T75-008-DELAYED with executable
support. Implement `src/journal_follow.c` against `followthrough/journal.h`.
Keep the original RZReceiver and its anchor, then run `resilience.receiver_journal`.
The actual retained receiver supplies checked integer effects, bounded request
retention, conflicting-intent rejection and fencing. The new mechanism adds a
canonical wire format, append/flush protocol, recovery and durable token issuance.
The supplied POSIX adapter implements file operations only; it does not implement
the journal or idempotency mechanism. It is a hosted dependency, not ISO C.

## Device and lifetime contract

RzjIO supplies size, positional read/write, truncate and flush callbacks over
one original local byte device. Successful reads/writes return1..requested bytes
except read at end may return0. A zero-progress write, premature end, invalid
progress count or callback error is refused. An error may already have changed
bytes: errors are not rollback. All callbacks/context and borrowed buffers remain
live for the synchronous call; callbacks must not retain spans, reenter the
journal, mutate its control, or access it concurrently. They are trusted device
adapters, not attacker-supplied function pointers. Inputs/outputs must not overlap
the journal or each other. One serialized writer owns the device; external file
modification and concurrent copied journal objects violate that precondition.

Successful flush makes all prior writes/truncations durable in this device model.
Between successful flushes, the cut model tests three permitted persistence
outcomes: none of the new bytes; the current written prefix; or an entire available
eight-byte commit region becoming durable before other unflushed bytes. Commit
region durability can extend the observed file length. Every byte-write and flush
boundary is exercised. This is a stated finite fault model, not a claim about
every filesystem sector, controller cache or arbitrary torn metadata. A device
that lies about successful flush, silently corrupts acknowledged bytes or violates
these assumptions requires a different argument. No host security/storage settings
are changed and no physical power is removed.

The journal has128 records of64 octets and refuses further appends. There is no
compaction, overwritten circular log, hidden eviction or unlimited durable history.
Opening an overlong device is CORRUPT. Profile requires8-bit octets and32-bit int;
the C static assertion diagnoses other representations. uint64 fields use explicit
big-endian arithmetic. File offsets are at most8192 and fit the hosted adapter's
off_t. The control owns no heap allocation; callbacks/file descriptions remain
borrowed. Do not reuse an initialized control concurrently, or use it after its
device/context is closed.

## Canonical record format

| Octets | Meaning |
|---|---|
|0..3|ASCII RZJ1|
|4|kind1 accepted request, kind2 issued fence|
|5,6,7|client, request identifier, delta+8; issuer uses0,0,8|
|8..15|sequence1..128, unsigned64 big-endian|
|16..23|logical monotonic observation time, unsigned64 big-endian|
|24..31|fence, unsigned64 big-endian|
|32..35|original signed32 response, two's-complement wire encoding by arithmetic|
|36|duplicate flag0/1; issuer0|
|37|retention window1..16, identical for all records|
|38..47|zero reserved bytes|
|48..51|FNV-1a32 over octets0..47: offset2166136261, prime16777619, modulo2^32|
|52..55|zero reserved bytes|
|56..59|ASCII COM1|
|60..63|bitwise complement of the stored checksum, unsigned32 big-endian|

The checksum detects the selected corruption controls; it is not cryptographic
authentication and does not prove detection of arbitrary modifications. A full
record with invalid syntax/checksum/sequence or impossible replay is CORRUPT,
including a bad final full record. Do not silently discard an acknowledged record
as though corruption were an incomplete append. A short final record is an
uncommitted tail under the device model and is truncated only after the entire
complete prefix has validated. Signed conversion must handle INT_MIN without
negating it or performing an out-of-range signed conversion.

## Operations and publication points

`open(io, window, out)` reconstructs a staged receiver from the entire complete
prefix, invoking actual rz_receive for each request and checking original result
and duplicate flag. Issuer records must increment the recovered issued fence
exactly once. Records must be monotonic in time; the persisted window must match.
After validation, an incomplete tail is truncated and flushed before publishing
the recovered control. Any read, truncate or flush error preserves output and
returns RZJ_UNCERTAIN. A failed recovery must be retried from device bytes, not a
partially reconstructed receiver. Truncation may itself already have persisted.

`issue(now, out)` appends a kind2 record for issued+1. Only after durable commit
does it advance the live issuer and resource high-water fence and return the new
token. Restart therefore resumes from the durable counter; it never resets to1.
Issuance itself fences off earlier owners at this resource. This is stronger
than waiting for the new owner's first effect. Counters refuse overflow. In this
bounded journal, record capacity is reached first during ordinary execution.

`apply(now, request, result, duplicate)` admits only previously issued positive
tokens; future or zero tokens are DENIED. The actual receiver then enforces the
current high-water fence, retained identity/intent, capacity and checked total.
An accepted replay returns its original response even when total has changed.
Every accepted observation, including a replay, is journaled so time/fence changes
survive recovery; rejected requests produce no record. Capacity exhaustion never
evicts a live dedup entry or an old log record. Result/output remains unchanged
on rejection. The issued token is an ordering value, not an authentication secret;
the two client identities and authority to issue tokens are trusted inputs here.

For either append, stage receiver/issuer state; write all56 body octets; flush;
write the eight-byte commit region; flush; only then publish live state and reply.
An I/O error after admission sets `poisoned`, returns RZJ_UNCERTAIN and preserves
the live receiver/result. A durable operation may nevertheless exist. Subsequent
appends refuse while poisoned. Reset the process/device view and reopen before
retrying the same identity. An acknowledgement implies a committed record under
the device contract; a missing acknowledgement does not imply absence of effect.
The second flush is necessary even though the commit bytes were written, and the
first prevents a durable commit region from outrunning an unflushed body.

Times remain comparable across recovery. The harness uses one logical clock.
A rebooted physical clock with a new origin cannot simply reuse old timestamps;
restore a monotonic epoch or conservatively suspend expiry until the time premise
is reestablished. Finite request retention still permits an expired identity to
apply again. Durable storage does not grant indefinite exactly-once execution.

## Delayed storage replicas and local attack boundary

The deliberately flawed fixture copies the fence4 journal into two independent
effect resources. Primary then durably issues5 and applies delta3; the delayed
replica still accepts stale fence4 and applies delta7. Both local fencing checks
can be individually correct while the claimed single-resource history is false.
The harmless success observation is totals3 and7 in two synthetic byte arrays.
Only the original teaching devices and admitted replay/reorder fields are in
scope; no operational storage, credentials or other users' data is involved.

The repaired composition has one durable effect/issuer authority. All write
requests go through that authority; follower snapshots expose RzjReplica with no
effect-writer operation. `mirror` copies only a prefix extension, refusing any
existing byte mismatch before writing, and uses the same body/flush/commit/flush
protocol for each new record. Repeated interruption is recoverable. A divergent
independent-writer history is refused, never overwritten to hide its effect.
`replica_read` validates/reconstructs a read snapshot. It may repair an incomplete
tail through the trusted replication/device owner. It cannot acknowledge writes.
The authority and follower must be quiescent during mirroring. If the authority
is unavailable, the repaired design sacrifices write availability. This is not a
consensus algorithm, failover protocol or distributed transaction guarantee.
The C type guides honest callers; raw device access/casts can bypass the protocol.

## Supplied verification and required deliverables

The native adapter borrows an O_RDWR regular-file descriptor with O_APPEND absent;
this qualification matters because Linux pwrite on an append-open file does not
follow the supplied offset. It uses pread/pwrite, supports forced short chunks,
and reports fdatasync/ftruncate failures. Read/write EINTR retries preserve offsets;
flush/truncate errors are uncertain. It may block and does not implement deadlines.
The test creates and immediately unlinks its own temporary file, writes through
three-byte chunks, forks an original child that commits then exits without sending
a reply, and reopens/replays in the parent. It separately adds and repairs a durable
short tail. This executes local POSIX persistence calls and process restart; it
does not simulate machine power loss, verify drive firmware or prove namespace
durability. A real named-file creation/replacement also needs its directory's
durability contract; this unnamed-file test makes no such claim.

Run the original receiver anchor, receiver_journal, and its three semantic controls:
omitted final flush; omitted body flush; and stale-resource fence suppression.
Require actual compiling learner rejection and GCC/Clang debug/release,
ASan/UBSan and separate TSan executions. Native/fork tests here exercise no
concurrent journal writer; TSan does not add such a guarantee.

The C harness covers408 append/issuer failure cuts, six repeated recovery cuts,
210 interrupted mirror copies, replay of original response, mismatch, fence
restart, no live eviction, full log, corruption, persisted-window mismatch,
zero/excessive transfer progress, and actual append-open descriptor rejection.
The independent Python tool compares2,916 request/expiry/fence histories and exact
wire bytes, plus414 durability cases including no-cut controls. Its map/byte model
must not call the C reference implementation to derive expected values. Check
that retries after uncertain outcomes return the committed result once or apply
the previously absent effect once, preserving the original intent.

D1 supplies source-bounded state/ownership and durability proofs; D2 supplies the
journal plus actual receiver/file adapters and their exact dependencies; D3 supplies
the failure matrix, independent bytes/results, mutants and bounded stale-replica
violation/repair; D4 supplies13 cost dimensions and the changed replica/issuer
argument. Source records distinguish write visibility from durability, file from
directory persistence, and a client-side lease check from resource enforcement.
Hints: first mark the two flush boundaries; then replay into a temporary receiver;
then follow a committed-but-unacknowledged identity through restart. Assess
R1--R4 separately: superficial success misses uncertainty and stale replicas;
independent understanding derives the failure partition and revises it under a
different device model. The seven-day learner transfer remains independent
assessment, not an elapsed-time claim made by these author executions.

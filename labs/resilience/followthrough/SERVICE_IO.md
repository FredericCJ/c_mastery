# Required T75.09 successor: actual local framed service I/O

Implement `src/service_io_follow.c` against `service_io.h`. Prerequisites are
X-T75-001 owned scopes, X-T75-004 fractional credits, X-T75-006 tenant permits,
X-T75-008 journal recovery and X-T75-009's OUTBOX contract. Import the actual
`hs_read_until` from `io.deadline` (PR-T24-05/LAB-HS-03). The native completion
test also imports `hs_reactor_sum` from `io.reactor` (PR-T24-04/LAB-HS-09) for
its precise one-byte-per-descriptor completion aggregation. It does not turn that
sum routine into a general framed reactor. The new scoped poll/write adapter is
authored here; the local socketpair is real POSIX IPC. The retained reversed
packet exchange and any newer root event adapters are not silently substituted.

C17 is the language baseline. POSIX threads, nonblocking AF_UNIX stream sockets,
poll, CLOCK_MONOTONIC and Linux MSG_NOSIGNAL supply the hosted environment.
These are library/OS contracts, not ISO C guarantees. No framing, service,
resilience or serialization library may implement the mechanism. The payload
and all files are synthetic; socketpairs are unnamed local endpoints. No listener,
external address, credential, production service or host signal-policy change
is involved.

## Framing and incremental state

A request has18 octets. Bytes0..1 are BE16 payload length16; byte2 version1;
byte3 client0..1; byte4 ID1..15; byte5 delta+8 in0..16; bytes6..13 BE64 positive
epoch; bytes14..17 zero. The full length is fixed but must still be validated;
reading a C struct directly is not a codec. Encode from a validated value via
arithmetic. On invalid input, encode preserves the output. Source/output/control
objects are distinct, live and correctly typed.

`frame_feed(frame, bytes, size, consumed, request)` takes a valid initialized
frame. NULL+0 is legal; NULL+nonzero is an admission error preserving outputs.
On valid admission, set consumed to the exact prefix used. Stop at the first
complete frame and leave a coalesced next frame for the caller. PENDING keeps
request output unchanged; OK publishes only after every field is validated.
An invalid length is detected at the second octet. Other malformed fields latch
CORRUPT after18 octets; later calls consume zero. A completed frame returns BUSY
until explicitly initialized for a new request. Do not reset a malformed frame
to resume scanning an attacker-controlled connection. `frame_end` distinguishes
empty clean EOF (CLOSED), complete frame (DONE), and a truncated prefix (latched
CORRUPT). Control invariants are caller-owned; byte fuzzing does not authorize
arbitrary corruption of the parser object itself.

A reply also has18 octets: length16, version1, status0..18, duplicate0/1,
reserved0, BE signed32 result, then BE64 epoch. On nonzero status, duplicate
and result are zero. `reply_decode` validates that contract and publishes once.
Signed decoding uses arithmetic without out-of-range unsigned-to-signed casts.

## Service lifetime, admission and partial I/O

RzvService is stable and noncopyable. It owns a mutex, actual RzbGate and one
shared actual RzfBucket; it borrows the outbox. The gate reserves capacity2 per
tenant and records an eight-outcome, minimum8, all-failed threshold window.
The limiter has caller-selected initial capacity and refill1/65535 milliseconds.
Every admitted request, including a same-ID replay, costs one unit. Work that
cannot obtain a gate permit does not commit the staged debit. A durable backend
refusal after admission still costs work. This rate is a declared teaching
configuration, not a production recommendation.

`serve(service, scope, socket, cancel_read, max_io, observation)` handles exactly
one request/reply. max_io1..18 bounds each read/send to exercise partial progress.
The connected stream socket must already be O_NONBLOCK; the function does not
change borrowed open-file-description flags. Cancellation descriptor differs
from it. One reader/writer owns each connection during the call; other threads
cannot close, repurpose or perform competing I/O on these descriptors. No POSIX
thread cancellation/longjmp may unwind through the call. Descriptors remain
caller-owned; caller closes them after return and thread join.

The scope uses milliseconds from CLOCK_MONOTONIC. A scope deadline must convert
to signed64 nanoseconds without overflow for actual `hs_read_until`. Journal
time is floor(milliseconds/1000), giving a16-second receiver retention window.
The two origins are the same boot's monotonic clock. Cross-boot recovery needs
the separately designed epoch/time-origin policy; an arbitrary new clock origin
is not assumed comparable. The deadline is an admission/publication boundary,
not a promise that blocking fdatasync or scheduling will finish by that instant.

Retain an owned scope before any blocking read. Validate the complete frame,
then under the mutex recheck scope/closed state, stage one credit debit, admit an
actual tenant permit, publish the debit and call actual `rzs_apply`. The mutex
serializes durable mutation. Record admission, effect status, committed-new-effect
flag and duplicate separately. Synchronous durable work may overrun a deadline;
after it returns, suppress a late reply but keep the committed notification.

Reply writing uses poll(POLLOUT) plus the cancellation descriptor, then partial
send(MSG_NOSIGNAL). Handle EINTR, EAGAIN/EWOULDBLOCK and short sends; advance only
by actual positive progress. Readiness alone does not promise a whole-frame
write. Cancellation readiness wins a poll tie. A pipe becoming ready after the
last read does not retroactively prevent a durable effect: publication uses the
shared scope check at admission. Cancellation requests should set the scope flag
and wake the pipe when prompt interruption of an active read/write is required.
The mechanism cannot revoke a previously committed effect.

Keep the gate permit and owned scope through every reply byte or until the
synchronous I/O call has stopped on timeout, cancellation or peer failure.
Release each exactly once on every terminal path. The returned observation
publishes read/written progress and cleanup; a malformed admission leaves it
unchanged. A successful transport return means the reply was locally sent,
not that a peer received it or durably acknowledged the notification.

The gate outcome is backend success on a successful effect/replay; backend
refusals are neutral. Later reply loss does not reclassify a committed backend
operation. `close` refuses new work and marks active permits cancelled; it does
not wake unrelated blocking syscalls, discard committed events or destroy scope
references. Caller wakes each connection and joins it. `destroy` requires no
active calls/permits plus external quiescence against new callers. Its BUSY
check is not a general lifetime lock. `notify` serializes forward/deliver/ACK
against active service calls and remains usable while the service is closed.

## Required evidence and assessment

D1 derives parser prefix accounting and the state transitions from read through
effect, reply and cleanup. D2 supplies codec, service and explicit producer
imports. D3 records exact native controls and independent wire oracle results.
D4 changes reply progress/cancellation order and derives the effect uncertainty
visible to a retrying client. Explain which properties come from bounds checks,
mutexes, reference ownership, POSIX calls and programmer lifetime discipline.

Run `resilience.service_io`, its two compiled controls and the wire oracle.
Tests cover every split of a valid request, coalesced frames, each malformed
non-epoch field, every truncated prefix, NULL+0, maximum epoch, native one-byte
reads/writes, two simultaneous same-ID requests, shared rate exhaustion,
closed-peer reply loss after commit, cancellation/readiness tie and expiry.
Fill an owned socket's send buffer until EAGAIN. Commit an effect while its reply
cannot progress, observe the actual permit still held and destroy BUSY, release
the creator's scope reference, then wake cancellation and join. Only then may
the final reference disappear. No process-wide SIGPIPE disposition is changed.
The compiled controls omit reserved-field validation and release the permit
before reply completion. The independent Python grammar checks11,714 inputs,
including changed chunk boundaries and precise consumed prefixes.

R1 requires the actual producer/API dependency map and representation; R2 the
separate admission/effect/reply/notification/cleanup invariants; R3 exact native,
wire, sanitizer and mutation evidence; R4 a changed partial-I/O/recovery schedule
with an explicit at-least-once notification policy. A superficial implementation
that treats one send as complete, closes a borrowed descriptor, discards a
committed event on client timeout or releases ownership on result availability
fails. Passing these finite tests is author evidence for this slice, not all
T75.09 investigations or independent learner mastery.

Hints: first keep consumed input separate from successful requests; then stage
credit until permit admission; finally trace the committed effect while the
reply is blocked. The private SERVICE_IO_ANALYSIS gives the repair arguments.
Report all13 cost dimensions, including per-call stack, fixed shared state,
two synchronous persistence barriers, mutex serialization, slow-reader permit
occupancy, copied frame bytes and OS scheduling. Virtual transport workloads,
native observations and physical network timing must remain separate.

# Composition contracts

This required additive laboratory consumes the learner's actual `errors`,
`ownership`, `coroutines` and `messaging` source interfaces. C17 and hosted Linux
are the implementation baseline. POSIX and sanitizer libraries are host
dependencies; this family makes no freestanding claim. CMake receives explicit
`CMASTERY_<FAMILY>_ROOT` paths. Assessor paths are separate, opt-in inputs and
must never be copied into a learner export.

## Transactional publication

`mc_publish_pair` accepts a borrowed byte span and an exclusively owned valid
`struct ow_owner`, empty or owning three blocks. The allocator obeys `struct v4_allocator`:
counts are numbers of actual `int32_t` objects, not byte extents. No mechanism
may replace the prior owner until all stages succeed. Reject `n > INT32_MAX`
before inspecting bytes, allocating or changing the owner. Parse exactly two
decimal integers separated by `:`, with no trailing bytes, using the earlier
`er_parse_pair` contract: operands and sum at most65535. Empty/null zero-length
input is invalid syntax; a nonempty span must designate accessible storage.

The result has separate parse and storage fields and a commit flag. Parse
failure preserves the complete prior owner and performs zero allocations.
Storage failure preserves parse success and returns `OW_NOMEM`, with every
temporary block released and the prior owner unchanged. Success publishes
three one-element blocks containing the sum, operand count2 and byte count,
then releases the old three blocks. The allocator must be able to hold both
versions during staging. Allocation failure on attempts1,2,3 means respectively
zero, one, two temporary releases, without touching old blocks. Publication is
serialized; it is not a lock-free transaction or durable storage commit.

Allocator release callbacks are nonfailing, nonreentrant and do not perform a
nonlocal jump. A callback capable of failure requires a different result and
retained-owner contract. An ordinary C struct copy does not transfer ownership.
The parse is O(n), allocation/release count is bounded by six, and peak live
storage with an existing owner is six typed cells plus allocator bookkeeping.

## Coroutine to receiver adapter

`mc_read_init` connects and starts one operation, prepares a `struct co_async` frame
wanting1..8 bytes and one unconditional cleanup. A successful initialization
does not call the backend. A failed initialization leaves the destination
unchanged and does not transfer the cleanup obligation. After success the
caller keeps the adapter at the same address through its terminal callback.
It also retains the output array, port context and cleanup context. No byte
buffer overlap with the adapter is admitted.

Every API call and backend callback is serialized. `read`, `arm`, `disarm` and
cleanup must not call the adapter recursively. `arm` establishes the frame's
borrow; `disarm` must return only when callbacks and queued notifications no
longer reference it. This is a backend prerequisite, not a promise made by a
generation token. The original tests emulate that quiescence explicitly. The
Linux extension below supplies it with a joined readiness worker; it does not
cancel a blocked system call or allow simultaneous owner calls.

Poll returns `MSG_BUSY` while pending, without publishing bytes or notifying
the receiver. Complete input copies exactly the requested bytes to the output,
then runs cleanup once and delivers `MSG_OK` with the byte count. Backend error
returns/delivers `MSG_FAILED` with accepted partial progress; caller output is
unchanged. Cancellation disarms before cleanup, preserves output and delivers
`MSG_CANCELLED` with partial progress. All terminal paths run cleanup before
the receiver. The receiver may free the whole adapter. Code must not read even
a status field in that object after delivery. Duplicate poll/cancel is invalid
only when the caller still owns a live terminal object; calling through a
freed pointer is outside the API precondition.

The two-attempt read/recheck behavior and generation admission come from the
actual `struct co_async` implementation. There is no new scheduler, implicit thread
synchronization or language-enforced immovability. Each poll is bounded by two
backend attempts plus at most8 copied bytes; backend latency itself is an
explicit external assumption. The original tests use synthetic port replies;
the Linux extension uses only local socket pairs and synthetic bytes.

## Five-resource recovery (MC-FALLIBLE-CLOSE)

`recovery.h` extends X-T66-011 with three actual `struct ow_owner` cells and two opaque
external resources. `mc_recovery_init` receives empty destination storage and
non-null open/close callbacks. All calls are serialized. Callbacks must not
reenter, mutate the owner, retain borrowed storage, perform a nonlocal jump or
return an invalid result. Successful open returns a fresh owned identity;
failed open transfers no resource. Identities remain unique until release.

Close returns one of three outcomes: `MC_RELEASED` leaves no obligation;
`MC_RETAINED` means no release occurred and retry is permitted;
`MC_RELEASED_ERROR` means release occurred with a diagnostic and the identity
must never be retried. Linux `close` can report later errors after the
descriptor number is reusable. This is not an ISO C or universal POSIX rule.
The teaching port injects a failure before close, or a diagnostic after an
actual successful close of a local `/dev/null` descriptor. It does not force
a kernel close error or establish filesystem durability.

`mc_publish_recoverable` first parses and stages three cells with the actual
earlier parser and counting allocator, then opens resources in order 0,1.
Precommit failure preserves current pointers and values. Its staged owner is
retained in `pending` if cleanup cannot finish. Successful publication commits
the complete staged owner, then attempts to retire the prior one. Both paths
release resources in reverse order and retain all three cells until both
external resources have been released. A diagnostic count is distinct from
`cleanup_pending`; it never authorizes retry of a released identity.

While recovery is pending, publication rejects without calls or state changes.
`mc_recover` accepts only the current token and preserves its error output on
a stale/repeated request. It returns 1 only when cleanup is complete.
`mc_retire_current` moves current ownership to recovery and returns a token;
it performs no close itself. Generations never wrap: publication rejects at
`UINT32_MAX-1` to reserve a final retirement token; retirement rejects at
`UINT32_MAX`. Empty final state is valid but cannot be reused after exhaustion.
Finish pending recovery before discarding storage. Repeated retained failure
can prevent progress indefinitely.

Input `2:3` followed by `7:9` gives sums5 then16. Enumerate allocation failure
0..3 and open failure0..2 for the second update. The independent fixture records
live cells/resources, releases, opens, attempts, generation and pending state.
Allocation failures preserve three cells/two resources. Retained retirement
after successful commit retains six cells/four resources; second-open failure
can retain six cells/three resources while keeping sum5. Peak storage is six
typed cells and four descriptors plus fixed owner records. Parsing is O(n),
each cleanup attempt has at most two closes, and retry count is unbounded.

## Linux readiness and cancellation (MC-REAL-IO)

`linux_port.h` extends X-T73-011. Initialize `struct mc_linux_port` once at its final
address with a live `struct mc_read` and borrowed nonblocking file descriptor. Retain
that descriptor until terminal completion. Initialization owns a wake pipe,
mutex and condition variable; failure releases created resources but leaves
destination storage unspecified. Do not copy or reinitialize a live object.

Exactly one owner thread invokes public adapter/COAsync operations. A private
worker polls input and wake pipe. Readiness permits retrying a nonblocking
read, not declaring completion: partial data, EAGAIN, EOF and error remain
separate. The worker queues at most one notification under the mutex, takes a
reference for it and waits for consumption or stop. Only the owner dispatches
the notice through `co_async_notify`; the worker never changes `struct co_async`.

`mc_linux_request_cancel` records stop intent and wakes the worker. It returns
without joining or releasing a queued borrow. `mc_linux_finish_cancel` joins
the worker, discards a remaining notice, establishes zero references, then
invokes `mc_read_cancel`. Synchronous disarm uses the same quiescence operation.
A join failure is not successful reclamation: the teaching void-disarm adapter
terminates instead of continuing with unsafe cleanup. This fail-stop limit is
distinct from normal cancellation recovery. No host security setting changes
or external peers are involved.

Cleanup follows quiescence and may destroy the port. The receiver follows
cleanup and may free the enclosing port/frame immediately; no later access is
allowed. Test cancellation without data and with an undispatched queued notice,
fragmented completion, partial EOF, and data available before first poll. For
each, retain or free the owner inside the receiver. Require
queued=consumed+discarded, joins=worker exits, zero references at cleanup,
one terminal notification and unchanged output unless all bytes were read.
Counters observe bounded tests; they are not nonwrapping correctness tokens.
One worker, two owned pipe descriptors and at most two references bound resources.
Join latency depends on host scheduling; no real-time bound is asserted.

Seal a predicted ownership trace before opening assessor analysis. Delayed
variants change acknowledgement ordering or close outcome semantics, not only
constants. Independent learner execution and non-author assessment remain
separate evidence.

## Evidence and containment

Only original local adapters and synthetic data may be used for defect
demonstrations. The four compiled controls remove old-owner preservation,
old-owner release, quiescent disarm or pre-receiver cleanup. Success is a named
integrity/lifetime assertion; a sanitizer finding alone does not prove code
execution. Restore the correct order, repeat the matching failure trace and
state the remaining backend assumptions.

Six compiler/optimization/sanitizer profiles are required author checks. The
fixtures are independent hand-derived expected states; supplied C assertions
do not implement an exhaustive oracle. Learners must derive their own failure
histories and delayed transfer. Required cost and enforcement evidence follows
`curriculum/v5/exercises/modern-cost-contract.md`. Finite runs do not establish
arbitrary correctness, independent review, target behavior or learner mastery.

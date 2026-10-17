# Composition contracts

This required additive laboratory consumes the learner's actual `errors`,
`ownership`, `coroutines` and `messaging` source interfaces. C17 and hosted Linux
are the implementation baseline. POSIX and sanitizer libraries are host
dependencies; this family makes no freestanding claim. CMake receives explicit
`CMASTERY_<FAMILY>_ROOT` paths. Assessor paths are separate, opt-in inputs and
must never be copied into a learner export.

## Transactional publication

`mc_publish_pair` accepts a borrowed byte span and an exclusively owned valid
`OwOwner`, empty or owning three blocks. The allocator obeys `V4Allocator`:
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

`mc_read_init` connects and starts one operation, prepares a `COAsync` frame
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
generation token. Tests emulate that quiescence explicitly. Cancellation of a
concurrent OS operation is not demonstrated by this serialized port.

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
actual `COAsync` implementation. There is no new scheduler, implicit thread
synchronization or language-enforced immovability. Each poll is bounded by two
backend attempts plus at most8 copied bytes; backend latency itself is an
explicit external assumption. No real I/O, credentials or external target is
used by the supplied tests.

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

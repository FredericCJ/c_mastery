# Linux capstone exact composition and transfer

This supplement supplies the original twelve Linux capstone ordinary tasks and
twelve delayed variants. It incorporates `../core/CONTRACTS.md` and the original
four PR-CAP-LINUX cards; IDs, objective rubrics and seven-day interval are unchanged.
The source register is `research/v5/expansion/legacy_capstone_linux/resources.json`.
Read the actual Linux epoll(7) LT/ET, descriptor/open-file-description and cached-
event passages, write(2) partial-progress/errors/NOTES, and the imported producer
contracts. These OS contracts are not ISO C semantics. No exact final-standard
inspection or independent learner success is implied by those readings.

## Actual service and publication

The native adapter owns successfully registered nonblocking input descriptors;
registration failure leaves ownership with its caller. `csl_close` invalidates
the slot/generation before deregistration/close, records failure and never retries
a possibly reused descriptor number. The owner serializes all session operations.
No callback is allowed to enter a freed control. Session generations start7 and
retire before UINT_MAX; a cached fd5/gen7 event cannot enable fd5/gen8. Event tokens
contain indices/generations, never a dereferenced freed heap address.

A turn reads at most its1..42-byte budget and performs a bounded number of parser
actions. Exhausting the byte budget retains software readiness. EAGAIN alone
permits this adapter to clear readiness; EINTR returns to scheduling without
busy-spin. Test ten available bytes with budget4, obtaining4,4,2 and an actual
EAGAIN observation. LT can notify again while data remains; ET need not. Separate
native tests register duplicated descriptors with different tokens: closing one
integer does not end the open-file-description lifetime. Retained cached tokens
must still be invalidated even after kernel deregistration.

Run two original local pipes through the actual P3/core/event adapter into a private
pipe and an unlinked temporary file. Sequence3 is denied while the seven other
authorized frames appear exactly once. Reading the file back checks visibility,
not flush ordering, durable storage or power-loss recovery. Output acceptance and
durability remain different claims. No listening socket or external endpoint is
needed. Caller storage and all live session objects outlast close and event-cache
drain. Kernel queue storage and libc/pthread runtime costs are outside the252-byte
portable backing figure and must be listed separately.

`CMakeLists.txt` takes explicit core/protocol source and include paths. Build with
the adapter and examples disabled, install into a clean prefix, then compile the
consumer outside both source and build trees. Test all68 split positions across
empty/golden/maximum frames. Repeat static/shared packaging, relocate the installed
prefix and use an explicit executable-relative runtime search path. A private
symbol or missing generated in-tree header must fail. A seeded sys/epoll.h include
must fail the strict-target header audit. Merely arranging source folders is not
dependency isolation.

## Modern abstractions at their actual boundaries

`cs_result_step` calls actual `er_transfer` with a normalized product. Synthetic
zero progress becomes ER_AGAIN because the producer rejects ER_OK with zero.
EINTR and EAGAIN preserve the same cursor; terminal errors preserve the separate
accepted prefix. `er_map` on failure must not invoke a success callback. The
comparison has the same13-byte output and six callback attempts as the baseline.
The result tag does not enforce exhaustive switches, payload consistency or
ownership in ISO C. Hosted errors.h includes setjmp.h; no whole-family strict-
firmware import is claimed. The core's plain enum/out-parameters are the selected
freestanding interface.

`struct cs_resources` acquires, in order, an input duplicate,84 candidate bytes, an output
duplicate, three queue-wire objects and an epoll registration. Fixture-owned
source/output descriptors and epoll instance are external prerequisites, not
unlisted service acquisitions. Inject refusal before1..5 and after5 before
publication. Explicit reverse cleanup and actual `struct ow_scope` must release the same
IDs exactly once. Cleanup callbacks are nonreentrant and cannot escape or fail to
release. The scope has eight entries; five are used. OS cleanup errors are retained
and never justify retrying a close integer. The asynchronous variant retains all
acquisitions while a real worker may access candidate storage. Owner-close alone
does not run cleanup. Join the borrowed worker, acknowledge, then unwind. The
reference tests deliberately hold the worker at a condition predicate.

The shared-slab variant uses four42-byte wire slots and actual `struct ow_shared` rights.
Publication creates two independent consumer rights, then drops the producer's
right. Each token includes slot, generation and consumer index. Cancelling one
consumer drops only that right; the other retains the immutable bytes. Last drop
destroys exactly once; duplicate/stale drops refuse; generation3 retires the slot.
All slab operations are owner-serialized, not advertised as a concurrent refcount.
This is a changed storage model:168 slab bytes plus ownership metadata, at most
eight consumer rights and bounded queue/reference bookkeeping. It does not retain
the original copied-frame252-byte interpretation by renaming storage.

`struct cs_receipts` demonstrates explicit message retry semantics: a trusted nonzero epoch
and sequence identify one request; the exact payload must match on retry. Four
entries retain receipts without eviction. A duplicate has no new effect; changed
payload under the same identity conflicts; capacity refuses before effect. After
five bytes of a failed stream, abandon that transport and resubmit a complete
frame under the same receipt identity. Byte replay on the old stream is not a
transaction rollback. This volatile bounded ledger is not crash-safe exactly-once
delivery, authentication, a durable store or infinite sequence-wrap support.

The actual P5 `struct p5_bridge` uses a32-byte capture and the actual FAsync/queue shutdown
protocol. A42-byte capstone frame therefore needs bounded13-byte or32-byte chunks.
For every chunk, retain the capstone lease through withheld acknowledgement,
observed matching quiescence and worker join. The worker copies from an original
local pipe; it is not a generic network writer. Never label callback observation
as join. Both normal and shutdown-before-ack paths execute. The capstone accounts
one final message release, even though the bridge releases several chunk operations.

The selected changed architecture alternative consumes actual `struct co_async`: requesting
42 bytes refuses, while8,8,8,8,8,2 reconstructs the same frame. Each producer poll
has at most two read attempts, and synchronous disarm quiesces its notification
source. The port here is a deliberately synchronous fixture. It cannot be replaced
by asynchronously cancelling P5 without another acknowledged-lifetime adapter.
No coroutine language lowering, stack lifetime enforcement or two-worker race-
freedom claim follows from this single-owner alternative.

## Resource and evidence comparisons

Use the13-dimension modern-cost contract under equal output/failure obligations:

| Dimension | Concrete evidence and limitation |
|---|---|
| Representation | Record actual CsService, er_result, er_progress, OwScope and slab sizes. |
| Allocation | Core zero; constructor two explicit heap acquisitions plus descriptors/registration; slab storage fixed. |
| Stack | Inspect target .su and call graph; temporary CPParser/message and encoder state are additional to persistent252. |
| Indirection | Six baseline/result sink callbacks; five cleanup callbacks; two consumer drop paths. |
| Branching | Inspect selected emitted functions; static branch sites are not dynamic counts. |
| Copy/move | Frame bytes copied into owned wire storage; P5 copies through pipe/capture; slab fanout retains rights instead. |
| Memory traffic/locality | State the touched arrays and copies; no cache-miss claim without counters. |
| Code size | Record final native/target sections and selected dependency closure, not just source lines. |
| Failure/cleanup | All seven acquisition cuts, first error/prefix, async borrow and exact release identities. |
| Synchronization | Owner serialization plus actual P5/predicate mutexes; no hidden C ownership synchronization. |
| Scheduling/latency | Bounded per-turn work and explicit pending state; OS/peer progress and join have no proved deadline. |
| Compilation/tooling | GCC/Clang debug/O3/sanitizers; clean static/shared install and target link audit. |
| Integration/portability | Same P3/core identities, separate Linux and MCU adapters, explicit helper/profile limits. |

The native measurement uses6000 equal32-byte frames, nine repetitions and rotated
capacity1/3 order. It reports raw monotonic times, bytes, callback counts and output
checksum; no minimum speedup or physical worst-case execution bound is required.
The runtime/tool upgrade variant reopens race instrumentation and timing evidence,
while unchanged arithmetic facts remain valid. Any changed source or dependency
reopens the affected claim rows as identified by its hash and assumptions.

The property-evidence checker is intentionally modest: it rejects checksum-page
evidence for durability and sanitizer-only evidence for race freedom. It checks
declared categories/premises; the assessor must also inspect actual bound logs,
source semantics and unsupported claims. The service's durability row is pending.
Host output visibility is not relabeled as durable storage.

## Required artifacts, hints, changed tasks and validation

The exact original24 Linux ordinary/delayed IDs map through objective_mapping.json
to concrete public tests, separate assessor analysis and source-bound commands.
Provide D1 predictions/architecture alternatives, D2 implementations and installed
consumer, D3 independent zlib whole-buffer comparison, raw logs, semantic mutants
and minimized counterexamples, D4 the unchanged named delayed task. The private
schedule changes frame split, payload A5 and stale callback order. Auth mutation
minimizes to U (one denied offer wrongly accepted); release mutation minimizes to
ABLX (admit/begin/lease/cancel). Single-command deletion minimality is property-
specific; it is not a proof of globally minimal programs or universal safety.

Hints: identify which API acknowledgement transfers which right; count the decoded
P3 temporary separately; trace cached events before touching an fd; mark the final
callback access before permitting destruction. Original objective rubrics and
their noncompensable correctness/failure/dependency gates remain in force. The
assessor solution is separate; public tests may be inspected by the learner but
cannot alone establish independent transfer.

All security controls are original small local targets and synthetic bytes/counters.
No real credentials, outside service, arbitrary control transfer or host setting
is involved. CRC recomputation exposes an authorization misconception, not a
cryptographic break. The early-release control is caught by a retained-lease
assertion before any unsafe memory access. Positive ordinary output and repaired
controls are mandatory. Residual same-process ambient authority remains explicit.

Validation status is the current source-bound campaign and export/guard receipts,
not this prose. Authored, compiled, actually executed and independently assessed
are separate states. Seven-day learner defense, external curriculum review and
all physical evidence remain unperformed. Do not close a required pending property
merely because unrelated finite tests pass.

# Capstone portable telemetry composition, version1

This required successor implements the concrete system in the retained capstone
contract. It preserves the eight parent identities, all original criteria and
their seven-day variants. Use it with `curriculum/v5/expansion/legacy_assessments/
capstone-contract.md`, the original project card and the learner's P3 protocol
dossier. P3's actual `struct cp_parser`, `cp_feed`, `cp_peek` and `cp_encode` are imported;
the signed-decimal `struct in_stream` is not this protocol. Implement `learner.c` against
your own P3 provider. No reference implementation belongs in a learner export.

## Environment and prerequisites

The same core is ISO C17 with eight-bit bytes, `uint16_t`/`uint32_t`, fixed storage,
and no allocator, POSIX, register or hosted-library interface. The relevant C11
object/unsigned-arithmetic rules also apply; no C23-only feature is needed. Exact
published C17/C23 text remains a separate source gap. CMake and Python are host
tools, not target dependencies. Inspect every final helper and relocation; the
author target uses explicitly supplied original byte-copy/clear helpers with no
libc/libgcc/libatomic/default startup. The target is a named Cortex-M4 emulator,
not evidence that STM32 peripherals or physical timing behave identically.

Prerequisites are the actual P1 ownership/container, P3 framing, P4 acquisition
and P5 service artifacts appropriate to the selected capstone, including their
failure and dependency dossiers. The Linux adapter additionally imports the
registered error, cleanup, coroutine and P5 callback boundaries where named.
Those imports are specific mechanisms, not permission to replace the wire codec
with a library or to import a hosted runtime into the strict core.

## Representation, inputs and outputs

`struct cs_service` owns two 42-byte candidates, three 42-byte queued wire arrays and one
42-byte active wire array: exactly252 persistent frame-byte backing bytes. The
actual structure additionally contains lengths, indices, generations, counters
and state. Count its `sizeof`, padding and stack separately. A persistent full
`struct cp_parser` would also retain a decoded message; hiding that extra payload in
"metadata" would understate storage. This adapter restores a temporary parser
from the exact candidate state produced by the previous call. It publishes or
explicitly rejects each complete decoded message before that temporary dies.
The temporary parser/message and any encoder/compiler scratch are real stack
costs, measured in the generated target audit. They are not included in252.

Controls, inputs, outputs and callback contexts are distinct live objects with
valid extents; numeric checks do not validate arbitrary pointers. Calls on one
service are serialized by one owner. Initialized objects stay at stable addresses
and are never copied or reinitialized while an adapter may access them. Public
fields support observation, not arbitrary modification. Each feed offers at most
42 bytes and returns the exact consumed count. Null with zero input is valid.
One call processes at most one parser result; a rejection may leave an embedded
candidate requiring a subsequent empty feed. Preserve the P3 one-byte-after-sync
resynchronization rule, including valid payload bytes equal to A5.

`cs_offer` copies a complete valid message into a wire slot only on admission.
Failure preserves the source message. OPEN admits at most the configured1..3
queued messages; one separately active output may coexist. Trusted authorization
is evaluated at admission, outside attacker bytes. A wire payload bit is never
authority. A denied valid frame changes only the denied diagnostic, not accepted
count or queue. The caller owns and explicitly disposes of an input rejected by
the selected reject-newest policy. The stream adapter reports that rejection;
it does not pretend the frame was accepted or silently retry commands.

`replaceable` and `overwrite` are trusted application policy arguments. Eviction
requires both the new and oldest queued messages to be explicitly replaceable
telemetry. It accounts the previously acknowledged old item as discarded and
released. Commands cannot inherit this policy from a payload flag. At most65535
admissions occur in one initialized service; admission then refuses. Denied and
rejected diagnostics saturate. Operation generations retire at their configured
limit instead of wrapping. Exhaustion does not authorize reinitialization while
old callbacks remain possible.

## Output, failure and lifetime

`cs_begin` transfers the oldest queued frame to the one active array. A synchronous
sink borrows only the offered suffix during its call and may not retain it or
reenter the service. One `cs_step` makes at most one callback. Accepted progress
is checked before advancing; zero, AGAIN and INTERRUPTED preserve the suffix and
return to scheduling. Counts exceeding the offered length or inconsistent retry
tags produce cause4096 with unchanged cursor. Terminal failure preserves both
accepted prefix and first cause. Explicit recovery resumes the suffix; it never
erases externally accepted bytes. Message-level retries need a separate receipt
protocol and usually a new transport after abandoning the partial old stream.

`cs_lease` instead lends the active suffix until matching terminal quiescence.
The owner must not step, reuse or release it while borrowed. `cs_complete` rejects
stale generations, retains the lease on a nonquiescent observation, and validates
progress on acknowledgement. A partial acknowledged completion returns ownership
of the remaining suffix to the service. A cancellation request discards queued
work but cannot release a borrowed active frame. Quiescence with an accepted
prefix records that prefix and explicitly discards the remaining message; it
does not roll back the accepted bytes. The exact conservation relation is
`accepted = delivered + discarded + queued + active`, with
`releases = delivered + discarded`. Repeated close/completion cannot add releases.

All terminal state/counter changes precede `cs_done_fn`. The callback is the last
object access and may destroy the service. The caller must also abandon its old
pointer after such notification. An arbitrary late caller cannot safely pass a
freed service pointer merely because generations exist; its external lifetime
gate must first exclude entry. This is explicit C discipline, not static affine
ownership, language unwinding or a universal safe-reference abstraction.

## Problems, deliverables and assessment

Use the exact golden13-byte frame, empty10-byte and maximum42-byte frames, all
two-chunk split boundaries, both sessions and the supplied pressure trace.
Implement the state table, helper closure, independent output ledger and failure
transitions before extending adapters. The ordinary output script2/AGAIN/0/
INTERRUPTED/3/8 must yield cursors2,2,2,2,5,13. Keep positive authorized output
alongside malformed/unauthorized, full queue, stale completion and cancelled
borrow cases. A successful compile or one expected output is insufficient.

Submit D1 predicted states, pre/postconditions and resource derivation; D2 source,
provider identities and exact composed interfaces; D3 commands, logs, independent
oracle, compiled defects and repaired regressions; D4 the original named delayed
task with assistance and elapsed study interval. The original objective-specific
rubrics remain authoritative: level0 incorrect/missing; level1 example repetition;
level2 correct composed invariant with a discriminating counterexample; level3
independent changed-condition derivation. An author reference run never awards a
learner level or substitutes for the seven-day assessment.

Graduated hints: first identify the last owner permitted to overwrite each array.
Then distinguish accepted bytes from terminal message disposition. Finally mark
the last callback access and compare its knowledge with the owner's observation.
The separate assessor analysis and changed schedules are withheld from exports.

The parser adapter performs the existing O(N*42) bounded rescans and at most one
frame admission per call, with fixed persistent storage. Queue operations are
O(42); no hidden growth or I/O occurs. Host/native/target results support only
their named paths, compiler flags and assumptions. Source binding, actual author
execution, external review, independent learner assessment and physical evidence
remain separate. Finite passing tests cannot establish absolute mastery.

Security demonstrations target only this original local program and synthetic
state. The attacker controls bounded bytes and permitted scheduling, not trusted
authorization or host policy. Authorization mutation increments a synthetic
counter; early-release mutation violates a retained-lease assertion before unsafe
access. Neither is claimed as arbitrary execution. No network target, credential,
deployed device, flashing or host-security change is authorized by this package.

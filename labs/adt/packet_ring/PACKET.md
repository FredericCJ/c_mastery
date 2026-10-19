# Required T67.10 packet and retained receive-ring work

This contract extends X-T67-010, including its original R1-R4, T67-10-TRANSFER,
T67-10-INVESTIGATION and X-T67-010-DELAYED. It supplies author support, not a
learner acceptance. Implement `packet_ring.c`; separate assessor code is not a
permitted learner dependency. The ordinary contiguous parser and the changed
segmented receive ring are both required.

## Prerequisites and actual components

Retain X-T67-009 and the original T67 sum/view contracts. Core C17 prerequisites
are initialized object lifetime, pointer bounds, checked size arithmetic and
explicit status/ownership transitions. For the contiguous stage import actual
`bytes.cursor` from PR-T03-02 (`FCursor/f_take`) and `bytes.checked`
PR-T04-01/U-T04-01-04 (`f_checked_size`) from the completed foundations workspace. The packet-specific
baseline is actual `ad_packet` in the completed ADT source, not a replacement
parser merely bearing its name. For the record stage import actual
`codec.records` PR-T20-01 (`ArRecord/ar_encode/ar_decode`) from the completed
api_depth architecture workspace. For the pipe stage import `io.frame`
PR-T23-04/LAB-HS-06 (`hs_read_frame`) from hosted_systems.

Dependency declarations belong to this exercise's family adapter. CLI learners
provide their completed dependency workspaces; author campaigns explicitly use
the separate assessor providers and pin their compiler-discovered source
closure. The later T24 incremental framing producer is not substituted for
`io.frame`. The core ring requires none of the pipe/thread services; this
particular combined harness is hosted Linux, GCC/Clang, eight-bit bytes, ISO C17.
No hidden target library implements the ring or the ownership mechanism.

## Problem and error table

`adp_parse` composes the actual `ad_packet` verdict with a cursor walk and returns
an `ADPacket` borrowing the input. A null output is INVALID. The retained parser
precedence is: null input or fewer than3 bytes  ->  HEADER; unsupported kind  -> 
KIND; BE16 length unequal to the exact remaining extent  ->  LENGTH; kind2 with
nonzero payload  ->  ACK; otherwise OK. Kind1 admits zero payload. Subtract3 only
after the header guard. No struct overlay, unaligned integer load, allocation
or payload copy is allowed in contiguous parsing. Failures preserve output.
An input `[1,0,2,7,8]` yields payload=input+3,length2; `[2,0,0]` is ACK;
`[3,0,9]` gives KIND even though its extent also disagrees.

`adp_build` constructs that envelope from disjoint inputs after complete
preflight: nonnull output/used and nonnull nonempty payload; kind1/2; length
 <= 65535 and actual checked total; zero-length ACK; sufficient capacity. Failure
preserves all output bytes and `used`. Contiguous parsing supports the full
65535-byte payload; the receive ring intentionally narrows it to29 bytes.

`AdpRing` has32 owned bytes and `(head,count)` describing their logical order.
`adp_init(r,origin,first_generation)` admits origin0..31 and generation1..MAX.
Production starts at0,1; explicit seeds make layout and finite-counter boundaries
testable. Initialization is once per object lifetime, or only after all tokens,
leases and raw pointers from its previous initialization cease to exist. A live
ring has fixed address, is not copied, and is externally serialized. The byte
source, ring, output descriptors and copy destination are disjoint. Native
pointer validity and object initialization are caller preconditions, not wire
validation tasks. Clients may inspect but not mutate representations.

`adp_feed` appends all bytes or none. Its source need only survive this call.
Empty input may be null. It refuses closed rings and capacity overflow, in that
order, and preserves all state on refusal. Count includes bytes held by an
outstanding consumer, so producers may append only in the remaining free region.
This is an owned receive buffer: ingress copies once. Zero-copy describes
payload parsing/consumption, not kernel-to-user ingress.

`adp_acquire(r,exact_frame_bytes,out)` consumes no bytes immediately. It refuses
closed, already active and retired rings, then incomplete requested extents.
For an available extent it applies HEADER -> KIND -> LENGTH -> ACK and publishes only a
valid lease. The first `exact_frame_bytes` bytes constitute one envelope; later
queued bytes belong to subsequent envelopes. A stream integrator must supply
that exact boundary, not guess it from available storage. The held region
includes the3 header bytes. Payload spans have total length equal to payload
length, first span ending at physical byte31 and optional second starting at0.
An empty payload has two null, zero-length spans. No payload linearization or
allocation is permitted. A copied3-byte header is allowed, though this reference
reads its individual positions directly.

## Ownership, cancellation and finite identity

A lease carries `(owner address,generation)`. At most one lease is outstanding.
`adp_validate` requires that exact live owner, active state and generation;
it returns OK, CANCELLED or STALE. Each successful acquisition issues the next
generation. MAX issues once and then permanently retires future acquisitions;
refusals consume no identity. Address reuse is not solved by this counter: all
surviving aliases must disappear before the ring's lifetime ends or restarts.

`adp_cancel` requests cancellation and is idempotent for the current lease. It
does **not** return the consumer's borrow or free capacity. CANCELLED still
means the exact retained payload bytes may be read by that consumer. The caller
must establish consumer quiescence, then call `adp_release`; only that operation
advances head/count and invalidates the token. No read may race release. There
are no hidden threads, callbacks or asynchronous acknowledgments in this core.
Duplicate release and old-generation release are STALE. A token check cannot
revoke a raw C pointer: dereferencing a saved pointer after release is a caller
violation even if its physical storage still exists.

`adp_close` prevents new ingestion/acquisition and requests cancellation of an
active lease. It preserves the held bytes and all queued content. Release is
still required; `adp_discard` can then drain queued bytes. Discard while a lease
is active is BUSY, even for zero bytes; excess discard is PENDING. A retired ring
may be drained but never issues another generation. Close itself is idempotent.

`adp_copy` accepts an unmodified lease returned by acquire, including a cancelled
but unreleased lease. It verifies the token before touching payload; then
preflights capacity and copies spans in logical order. Output remains unchanged
on refusal. A forged span descriptor is outside the native-object contract.
Copying does not release the token. For the required equivalent comparison,
both borrowed and copied consumers hold the ring until the same cancellation
and final release point. Earlier copied-consumer release is a different protocol
and must be analyzed separately, not silently credited as a speed advantage.

## Actual framing and record adapters

`adp_read_pipe` calls actual `hs_read_frame`: an outer BE16 count, followed by
exactly that many inner packet bytes. Real local pipes are required. EOF during
header/body is EOF, OS failure is IO, outer capacity excess is OVERSIZE; only a
complete outer body reaches the packet parser. The packet output is unchanged
on every error, but descriptor progress and caller backing storage may already
have changed. No rollback or resumption is claimed. Close the owned descriptor
on error; the adapter does not close a descriptor owned by its caller. An empty
outer frame is a complete transport frame but a packet HEADER error.

The outer count is16 bits, so this transport admits at most65535 inner bytes,
not the contiguous parser's65538 maximum envelope. One-byte writer operations
exercise actual pipe delivery; they do not guarantee one-byte reader returns.
No deadline, nonblocking reactor, process authentication or external network is
claimed. Synthetic bytes and local teaching processes are the entire scope.

`adp_record` gathers at most5 payload bytes from the current lease into local
storage and calls actual `ar_decode`. Version1 needs exactly3 bytes (signed16
big-endian plus XOR), version2 exactly5 (signed32 plus XOR). Only DATA is
accepted. Unsupported version, invalid extent/checksum or non-DATA is CODEC;
stale tokens are rejected before reading payload when the record shape is
admitted. Publication is last; output stays unchanged on failure. This small
codec boundary deliberately copies bytes and is not labeled zero-copy. XOR
detects only some corruption and supplies no authenticity.

## Required evidence and rubric

- R1: run the supplied census C++23 artifact as a bounded behavioral comparison;
  explain `variant`, `expected` and `span` representation versus the C tag/status
  pair, including raw-span lifetime obligations. The census is executable
  teaching input, not normative language authority. Read the registered primary
  span/enum passages for language claims. Neither version pins backing storage.
- R2: derive the error table and all range proofs. Show that the ring's logical
  occupied region excludes every producer write while its lease exists. Supply
  counterexamples for premature cancellation release and generation wrap.
- R3: retain byte predictions; run every direct-parser and ring boundary, actual
  pipe truncation/zero/oversize cases, actual codec versions, independent Python
  grammar/history comparison and all eight normal semantic mutant rejections.
  Compiling learner stubs must fail their assigned assertions, not compilation.
- R4: implement the retained token and changed segmented layout. Exercise every
  physical origin, reuse, stale aliases, MAX retirement and close while held.
  Compare copying and borrowing with equivalent release/cancellation contracts,
  include the evidence and thirteen dimensions below, and explain limitations.

All four need substantive reasoning plus correct implementation for level2.
Passing nominal anchors alone is level1. Level3 additionally needs an unaided
delayed changed-condition attempt and a minimized counterexample to the old
argument; author-supplied tests do not award it. Ownership violation, publication
on failure or a fabricated execution result fails regardless of timing.

Hints: (1) draw logical bytes separately from physical indices; (2) count must
include the whole held envelope, not only unread bytes; (3) cancellation changes
permission to admit work, while release changes storage ownership. Assessor
derivation, mutation analysis and delayed solution remain separate.

Delayed X-T67-010-DELAYED changes a contiguous view into a wrap-split payload,
retains a cancellation request while the consumer still reads, and reaches the
last generation. Implement it from the stated primitives without the old
solution; predict the wrapped spans and unchanged bytes before running. A
counterexample to a broader claim permits two simultaneous leases: explain why
out-of-order acknowledgments invalidate a single active-bit proof. This is an
analysis of the boundary, not a requirement that the supplied single-consumer
implementation implement an additional multiple-consumer protocol.

## Thirteen cost and enforcement dimensions

| Dimension | Required evidence and bounded conclusion |
|---|---|
| Representation | sizeof/alignment of packet, ring, token and spans on the actual ABI; no universal layout claim. |
| Operations | Contiguous parse O(1); ingress/build/copy O(n); ring acquire/validate/release O(1); codec <= 5-byte gather. |
| Invariants | count <= 32, head<32, held <= count, exact active identity, nonwrapping issuance. |
| Ownership | Fixed live ring, one current consumer, borrowed spans until explicit quiescence/release; ingress source borrowed only during feed. |
| Failure | Admission transactional; transport progress/storage effects persist; source parse outputs preserved. |
| Allocation | Core performs no dynamic allocation. Test maximum-buffer malloc, processes and tools are host verification dependencies. |
| Memory traffic | Ingress copies32 maximum bytes; borrowed sum scans29; copied sum adds29 writes and29 reads under identical ownership. |
| Stack | Compiler stack-usage records plus5-byte codec gather; bounded descriptors, not hidden heap. |
| Code size | Object/executable size, assembly and undefined-symbol audit include actual imported primitives. |
| Compile/build cost | Strict C17 GCC/Clang, clean and unchanged Ninja timings; no cross-machine build-time guarantee. |
| Latency/throughput | Seven repeated10000-frame runs, same4930000 checksum, half cancel before release, same29-byte payload. Raw CPU-clock observations, not physical network latency. |
| Portability/editions | Eight-bit-byte requirement explicit; byte decoding avoids alignment/endian ABI assumptions. C17 has no affine lifetime enforcement; C23 does not automatically revoke pointers. Published C17/C23 texts remain access gaps. |
| Enforcement/composition | ISO types/bounds rules, diagnostics, operational token checks, sanitizer/model scope and programmer discipline listed separately. C++ wrappers discriminate alternatives but do not give the original span an owning lifetime. |

## Validation limits

Source-bound reports distinguish authored scaffolds, actually compiled controls,
executed host cases and the exact source/toolchain identities. Pilot translation
first caught misleading test formatting; a later pilot caught incorrect exact
codec length supplied by the new harness. These were corrected before final
campaign. No physical target, kernel privilege, unaided learner performance,
non-author assessment or all-interleavings proof is inferred. Tests do not prove
absolute mastery or arbitrary pointer validity. Multiple simultaneous consumers
are outside this explicitly single-consumer ownership protocol.

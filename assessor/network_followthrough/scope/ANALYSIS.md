# PR-T41-01: withheld solution and transfer assessment

Use the unchanged AX tasks and V1 identities. The implementation is an author
reference, not evidence of a learner's independent performance. Record the first
unaided explanation, then release hints progressively. Require a separate attempt
at least seven days later; the current executable variants prepare that assessment
but do not satisfy the elapsed-time or learner requirements.

## 01.a

The tuple is local/peer addresses and ports plus an externally assigned local
incarnation. In the restricted pair adapter, both addresses and local port are
fixed, so two distinct configured remote ports select two complete `struct net_stack`
objects. The epoch labels local lifetime; it is not transmitted authentication.
Initialize only fresh or externally quiescent storage. Changing epoch while an
old device still writes does not make reuse safe. Inactive capacity still occupies
two objects in this representation; do not charge only the active entry.

The executable variant performs two SYN/SYN-ACK/ACK exchanges and two different
requests. Preserve the first retained packet while delivering POST to the second
parser. Routing both to entry zero must fail on the second entry's state/port,
not on an unrelated backwards timestamp. The fixture uses globally monotonic
times for this reason. An alternative compact common-neighbor/table design is
valid if its lifetime proof and two independently observable states are complete.

A host success cannot prove normative host conformance. RFC9293's transport
specification explicitly relies on congestion-control requirements elsewhere;
RFC1122 requires IPv4 reassembly. The predecessor profile rejects fragments and
retains one flight; its successful controlled GET implements neither mechanism.
Required reassembly and congestion work is registered separately, not downgraded
to optional. A candidate who calls `struct nw_pair` a general TCP implementation fails the
scope criterion even when all packet fixtures pass. Also identify the predecessor's
fixed MAC, literal Date, fixed advertised window and lower-send assumptions.

Resource proof: six buffers of2048 and six descriptors of16 require12384 bytes;
the ELF must place all of them in the selected DMA-reachable interval, independently
of ordinary BSS. RM0090 Rev21 pp63/68/71 supports Ethernet access to SRAM and CPU-only
CCM. The F439 linker audit uses SRAM1/2 and Flash; it does not boot or operate the
Ethernet controller. Moving `.dma` to CCM may still produce a perfectly linked
ELF and must fail the independent reachability checker. This is a runtime-closure
and layout argument, not a measurement of bandwidth, cache coherence or PHY behavior.

Inspect compiler stack-usage output as well as static object sizes. A previous
`*s = (struct nw_pair){...}` generated a27768-byte local temporary at GCC O0. Together with
its callers this exceeded the declared32768-byte reserve despite a passing pilot.
The retained `NW_PAIR_MUTANT=2` reproduces this resource defect. The corrected
initializer assigns scalar metadata and initializes each actual stack in place.
The first attempted runtime-constant branch still reserved the dead temporary at GCC O0; the specimen is now selected by preprocessing. The269-check failed development campaign preserves that discovery. Functional behavior alone is insufficient evidence; require the stack gate.
The conservative gate sums all compiler-emitted nonrecursive C frames once,
plus1024 bytes for entry/exception allowance. This overapproximates paths only
under the documented no-recursion/no-callback/no-enabled-IRQ call graph; it is
not a universal stack analyzer. Inspect emitted calls and reject dynamic frames.

## 01.b

For payload3 the independent nested sizes are IP43, unpadded Ethernet57,
padding3, pre-FCS60, with-FCS64. FCS is MAC-generated in this contract; neither
preamble nor interpacket gap belongs to the stored frame length. Check endpoints
and invalid1461 before arithmetic. A packet observation must name which quantity
the capture includes.

Transmit invariant: `0 <= acknowledged <= sent <= length <=1600`; exactly one
prepared attempt; at most one accepted flight; DMA and remote flight are separate
obligations. `prepare` does not advance `sent`. Rejection clears preparation only.
Acceptance advances by the prepared length and records `flight_end`. DMA-done
requires the current operation serial, and affects only DMA. ACK validates the
connection epoch/owner/generation and a cumulative offset no greater than sent;
it may cover a prior segment so does not require current local serial. The full
TCB must validate the network ACK before invoking this local ledger. Duplicate
ACK and duplicate completion report their category without changing ownership.

For600bytes, MSS256/window512, the independent segmentation is256/256/88.
With window17 the next segment is bounded by17; window zero makes no progress.
The180-case oracle constructs occupied offset bins and an independently defined
byte sum, rather than repeating the reference's cursor transition loop. It covers
response3/256/257/600/1600, MSS1/7/256, window1/17/512, two owners and both completion
orders, with zero-window and refused-admission probes on every step. Its finite
bounds do not prove arbitrary workloads or transport liveness.

The harmless fault demonstrations run only against original arrays in local
teaching processes/emulation. The attacker controls completion order, stale or
foreign local tickets, admission refusal and byte contents. Premature ACK on DMA
completion violates remote-delivery accounting; freeing on ACK while DMA is busy
violates local ownership. The observable success condition for a mutant is the
specific assertion failure, never arbitrary code execution or privilege gain.
Repair by preserving the two ledgers and identity checks; repeat both orders,
refusal and stale identity regressions. Residual requirements include genuine
device quiescence, an authenticated/control-validated peer where needed, and
bounded counter/epoch allocation. No numerical ticket proves those properties.

The changed-condition task supplies600 then1600bytes with repeated lower rejection,
partial ACK and overwritten caller input. Reject an answer that keeps only the
last DMA packet as the sole unacknowledged response. Retaining the entire response
is conservative but valid; a sliding retained window is valid with a separate
proof of prefix reclamation and overlap handling. Report its actual storage/copy
cost rather than claiming equivalence because both produce the same ordinary GET.

## 01.c

The ingress fixture creates actual Ethernet/IPv4/TCP frames with an independent
byte-oriented checksum builder. The adapter uses the predecessor's `net_checksum`
and the actual HTTP parser. It validates IP total length and removes Ethernet
padding before queue admission. The acceptance sequence advances only after the
whole span is copied. BUSY leaves both queue and sequence unchanged. TCP sequence
wrap uses unsigned32-bit arithmetic under the exact-next-sequence bounded profile;
this does not implement arbitrary reordering, half-range comparison or a TCB.

Queue invariant: head<capacity<=512 and count<=capacity. Occupied logical offsets
map to `(head+i)%capacity`. Peek exposes at most the first contiguous span;
consume advances only the exact parser-reported prefix. The parser may finish
inside that span. Consuming the offered amount instead of `used` loses the suffix
even though the body is correct. The tests retain four literal NEXT bytes after
DONE, including when repeated pumping makes no progress. They overwrite borrowed
frame and caller buffers after return; accepted bytes must remain independent.

The native fixture checks6834 three-frame partitions/budgets; the M4 fixture
checks18 selected changed-layout cases. Neither count can be relabeled as the
other. Body `A,0,B,255` distinguishes binary spans from C strings. Budgets1/7/512,
delayed parser service, a one-byte queue rejecting five bytes, queue wrap and
IP-padding mutants expose distinct obligations. A separate exhaustive native
queue oracle is unnecessary for claiming all possible queues; no such universal
claim is made. Require a learner's invariant argument and a new capacity/split
counterexample independently of these fixtures.

For the delayed assessment, hide the reference and oracle and supply a different
request split at CR/LF plus an already queued next request. Demand exact consumed,
remaining and advertised capacity after every step. Use the actual learner HTTP
artifact; report author-reference integration separately. If they enlarge a
buffer to hide leftover loss, the repair fails the mechanism criterion.

## Evidence and grading

Grade each original leaf independently:0wrong/absent;1ordinary example only;
2correct representation, failure argument, discriminating compiled tests and
resource/evidence boundary;3unaided transfer with a defended changed-condition
argument and reproducible artifacts. No aggregate score erases a failed ownership
or scope criterion. A compile failure is not a successful semantic mutant kill.

ISO C17 supplies unsigned arithmetic and object access rules, not DMA ownership,
TCB validity or callback serialization. The target uses GCC/Clang Arm extensions
for placement and semihosting. Original string/memory helpers fulfill actual
imports; direct linking and omission controls check the closed image. Host libc
and sanitizers remain host dependencies. C23 comparisons are limited: neither
edition statically enforces these lifetime/protocol guarantees. Exact published
ISO wording, physical validation, non-author review and independent delayed
attempts remain separately recorded gaps.

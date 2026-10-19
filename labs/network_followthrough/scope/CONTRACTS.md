# T41.01: finite endpoint resources and lifetime boundaries

This required successor preserves PR-T41-01's three original AX criteria and
seven-day V1 transfers. Implement the public `scope.h`, `wire.h`, `stream.h` and
`pair.h` contracts in their compiling incomplete C17 sources. Use original C
byte operations and the earlier learner HTTP/checksum components; no OS socket or
third-party TCP/IP stack performs these mechanisms. Host Python and sanitizers
are verification dependencies. The target build explicitly selects original
memory/string helpers and audits the linked image; including a standard header
alone does not authorize a standard-library archive.

Prerequisites are T18 checked spans/ownership, T27 memory maps, T28/T29 completion
and quiescence, the T39 frame-channel contract and the T42 HTTP prelude explicitly
listed by the parent module. Actual author reference imports are separately
identified; they are not learner-built artifacts. A later full T39 driver binding
must wait for that producer's actual API and evidence.

## Guided derivation and representation

Draw byte offsets first: three TCP application bytes plus20TCP and20IPv4 header
bytes give43IP bytes. Ethernet adds14bytes, yielding57; pad3to the minimum60
pre-FCS bytes, then the MAC's4-byte FCS gives64. Preamble, SFD and interpacket gap
belong to different quantities. Six2048-byte buffers plus six16-byte descriptors
occupy12384bytes before any additional alignment gaps or state. Validate payload
0..1460 and check all larger values before computing. Ethernet padding must be
trimmed by validated IP total length before delivery to TCP/application parsing.

`NwTx` owns up to1600 immutable response bytes. `nw_offer` copies caller bytes;
the input expires on return. It accepts a new response only after all previous
bytes are acknowledged and no DMA/prepared operation remains. Generation advances
without wrap. Every prepared segment carries epoch, connection owner, generation
and monotonically advancing serial. The fields are checked identities under
programmer discipline; they are not cryptographic network authentication.

`nw_prepare` returns a copied segment of at most256bytes, further bounded by MSS,
peer window and remaining response. It reserves one attempt without advancing
the accepted cursor. `nw_submit(...,0)` releases that reservation after lower-send
refusal; bytes and accepted cursor remain intact. Acceptance advances the cursor
and creates separate DMA and remote-ACK obligations. DMA completion releases only
the local transfer. A remote ACK can arrive first and still cannot release a
DMA-owned buffer. Partial ACK retains the outstanding suffix; the conservative
implementation keeps the whole response until reuse. New segments wait for both
flight and DMA obligations. Stale/foreign tickets and exhaustion are explicit
failures, with no partial mutation. Reinitialization requires a fresh/quiescent
object and externally justified epoch; it is not a cancellation shortcut.

All calls on one object are serialized. Input spans are live, valid, and do not
overlap the state being mutated. C's type system does not enforce these lifetime
or serialization rules. Neither a `const` pointer nor a numeric generation stops
a physical device. ACK arguments have already passed the full TCB/peer/sequence
validation; this ledger tracks response offsets and does not validate TCP control
packets by itself.

`NwRx` owns a circular queue with capacity1..512. Admission copies an entire span
or returns BUSY without consuming it. `nw_rx_window` is unused capacity. A peek
borrows only the first contiguous portion until the next queue mutation;
`nw_rx_consume` advances by a checked exact prefix. Failed admission must not
advance the TCP acceptance sequence. The stream pump gives the existing HTTP
parser at most its budget, consumes exactly `used`, and preserves all bytes after
DONE/ERROR. Parser reinitialization and connection-close policy are explicit.

The supplied established-data ingress profile checks configured MAC/IP/ports,
fixed20-byte IPv4/TCP headers, lengths/checksums, no fragments/options, ACK or
PSH|ACK, and exact next sequence. It trims padding and commits sequence only after
queue admission. A complete TCB validates ACK/window/control state before using
this adapter. Its intentionally bounded admission contract does not replace
SYN/FIN/RST/overlap or congestion processing.

`NwPair` changes the historical one-connection admission limit to two configured
remote ports under the fixed peer/local IP profile. It owns two separate actual
`NetStack` objects and dispatches TCP by remote port; a shared ARP observation
updates both while emitting one response. Capacity1 refuses the second port.
Each stack retains its own parser, sequence state and retransmission packet.
The original stack's lower-send/window/neighbor/date gaps remain explicitly
assigned to subsequent T41 work. This comparison is not permission to claim that
adding an array implements every general-host requirement.

## Required exercises and changed conditions

**T41.01.a (A,B,F,H,I,J).** Build the profile/exclusion table and `NwPair`; run two
actual independent handshakes, GET and binary POST requests, and a wrong-route
control. Prove parser, retransmission and incarnation isolation. Calculate native
and32-bit object storage, static placement and stack reserve. Delayed V1 changes
admission to two connections and requires an explicit tuple/epoch identity and
resource plan before changing the interoperability claim. Explain why the
repository's controlled successful GET does not implement IP reassembly or general
congestion control; these mechanisms remain required comprehensive-course work
in `research/v5/expansion/legacy_network_scope/remaining_work.json`.

**T41.01.b (B,C,D,F,G,H,J).** Implement nested sizes and transmit ownership.
Supply response3,256,257,600 and1600bytes; MSS1,7,256; peer windows0,1,17,512;
both DMA-first and ACK-first orders; partial/duplicate/oversized ACK; lower-send
refusal; stale serial, owner, generation and epoch; counter exhaustion. Return
exact segment offsets/bytes and both lifetime ledgers. The delayed response beyond
MSS256 must preserve every response byte through segmentation, refusal, local
completion and acknowledgement. Corrupt the caller buffer after offer to expose
a borrowed-reference implementation safely. Do not infer TCP ACK from DMA completion.

**T41.01.c (B,C,D,F,G,I,J).** Deliver an HTTP header across three actual synthetic
TCP frames, coalescing its tail with a binary body and an additional suffix.
Delay the consumer; exhaust capacity; wrap the byte queue; change pump budgets.
Every accepted byte must remain available until consumed or explicitly discarded
by a documented close policy. Demonstrate that frame padding is excluded and
BUSY leaves the acceptance sequence unchanged. Delayed V1 changes parser service
timing and capacity; derive the new advertised-window bound and show accepted
leftovers survive. A protocol library that ignores `used` fails even when an
ordinary GET appears to work.

Deliver public implementations, proofs of invariants and failure atomicity,
independent expected byte/length tables, generated object/import/map/stack records,
complete command/source/seed/raw-result identities, and a criterion-indexed
assessment dossier. Target `.dma` storage is a placement reservation; it performs
no physical DMA. MPS2 execution and F439 placement qualification are separate.
Actual cable/PHY/waveform evidence belongs to its dedicated board protocols.

Complexity: offered/transmitted/received spans require linear copying; each queue
operation has constant bookkeeping plus copied bytes; the parser pump is bounded
by input budget and the retained parser's per-byte/line work. Pair dispatch is
bounded by two ports, but state storage doubles even when the configured active
capacity is one. Include both retained response and DMA-copy storage in resource
costs. Use generated32-bit layouts instead of assuming host `size_t`/alignment. Inspect local initialization temporaries: an unoptimized aggregate assignment can spend stack proportional to the entire connection table even if the persistent table is static. Require the32768-byte stack reserve and placement audit to pass independently from functional execution.

Rubric per criterion:0incorrect/missing;1example passes without a valid lifetime
or scope argument;2correct implementation, independent expected results,
discriminating compiled controls and complete contract;3independent transfer
under changed layout/timing/capacity with a defended new argument. Original
seven-day assessments require an actual later learner attempt. Author runs and
parallel authorship do not substitute for it or non-author review.

Hints, in order: draw the separate owners of a byte; name every length's first
byte and excluded trailer; pause between prepare/submit, take/release and
DMA/ACK; use the parser's exact consumed count; then construct a changed-condition
case where an apparently equivalent implementation loses a retained byte.
Technical solutions and independent-oracle implementations are assessor-only.

The exact inspected RFC9293, RFC9112 and RFC1122 passages and editions are in the
topic source register. C17 is retained; C23 does not supply these ownership,
protocol or device guarantees. All executable adversarial cases use original
local teaching arrays/programs and synthetic data. No external service, live
network, credentials, deployed device or host security change is involved.

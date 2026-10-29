# Ethernet service, frame and bridge changed conditions

This required successor retains all original criteria and seven-day variants in
PR-T39-01, PR-T39-02 and PR-T39-03. Families A/B/C/D/E/F/H/I/J apply: construct
the missing adapters, falsify their assumptions, and defend the analysis below.
The existing `ethernet_depth/CONTRACTS.md` and original project prerequisites
remain in force. Implement `learner.c`; import your `ethernet_depth.c` rather
than a library implementation. The separate assessor is never a learner import.

## Environment and ownership

ISO C17, native GCC/Clang and selected Cortex-M4 soft-float freestanding builds.
No allocator, operating system, sockets or networking library belongs to the
target. Explicit original byte-copy/clear/comparison helpers are permitted;
audit compiler-generated calls, objects, linker map and undefined symbols.
Host Python, sanitizers and QEMU are verification tools. C23 does not supply
record boundaries, buffer ownership, VLAN policy or DMA ordering automatically.
Do not infer C17 semantics from the C11 draft or claim final C23 text inspection.

Pointers designate live objects of stated sizes; input/output/control objects
are disjoint, with one serialized owner and no concurrent mutation. Stream
controls begin zeroed and remain API-managed. Parsing borrows the input bytes
until the caller releases them; construction and record extraction copy bytes.
Neither a C type nor a checksum authenticates a peer. Every failure documented
as atomic preserves the output bytes, count and control; partial stream input
is deliberately committed and its consumed count is authoritative.

## PR-T39-01: actual framing and authority boundaries

First derive the 12-byte SDU's three PDU sizes under wrappers8/20/14 and label
each local service handoff and remote peer protocol. Execute `ed_wrap`, including
late overflow, and explain why these synthetic wrappers are not IP encoders.
Construct a seven-purpose diagram for this concrete HTTP request:
`GET /sample HTTP/1.1`, authority `lab.invalid`, representation `application/octet-stream`,
body bytes01/02/03 in its response. Locate representation, request/response
dialogue, ordered byte delivery, routing, frames and physical symbols. Do not
invent separate session/presentation headers: Internet applications can perform
those functions without separate OSI protocol instances. HTTP connection
identity is not user identity; TCP does not certify a command's authorization.

The changed frame-to-stream service uses two big-endian length octets followed
by 0..256 payload octets. `struct ew_stream` owns one copied record; `ew_feed` consumes
only through its first completed record, reports `EW_READY`, and leaves a
concatenated suffix with the caller. Further feed while ready consumes zero.
`ew_take` copies the record and returns the control to empty; insufficient
capacity leaves everything unchanged. NULL+zero is admitted where the size is
zero. A zero-length record is a real record. A length above256 consumes the
two-octet prefix then poisons the stream, with no guessed resynchronization.
`ew_end` reports ready before EOF, clean EOF on an empty control, and truncation
then poison on an incomplete prefix/body. A terminal EOF control stays closed.
Feed is O(consumed bytes), take O(record bytes), fixed256-byte storage plus
metadata. No input pointer is retained. Reason about upstream backpressure
when the next complete record cannot be admitted.

For the seven-day task retain the original variant ID. Independently implement
the framing adapter and account for every split of a maximum record and three
concatenated records. Then replace only the peer's prefix byte order: the same
C function signature interprets01/00 as256 versus1. Supply the divergent byte
trace, not merely a different protocol name. Separately classify bridge
forwarding, bad frame CRC, missing route, duplicate delivered stream byte and
application rejection by the first owning contract. Explain how a lower
symptom can originate upstream without changing the violated contract.

Moving this boundary into the MCU transfers checksum/sequence/window/timer,
ARP/routing/reassembly, frame-buffer and device lifetime responsibilities from
the hosted stack. T41's actual `struct nb_net`/`struct nh_http` composition is a later required
consumer, not an imported prerequisite that reverses the curriculum order.
The local adapter does not claim to implement TCP or an HTTP server.

## PR-T39-02: route and explicit frame extension

Draw the F439 MAC → RMII → selected PHY → magnetics → connector → twisted pair
route. Keep the management MDIO/MDC bus separate from data. Then draw a changed
original SPI-controller architecture: MCU command transport and controller frame
engine move across SPI; PHY coding/signaling and isolation do not disappear.
This is an interface allocation analysis, not a claim that MCP2518FD is an
Ethernet controller or that a selected SPI Ethernet chip has been implemented.

Derive collision-domain differences for shared half-duplex and switched
full-duplex operation. Inspect the selected LAN8742A negotiation and parallel
detection rules; a forced or half-duplex peer invalidates the retained full-only
driver's admission premise. Refuse it explicitly instead of treating a link-up
bit as full-duplex proof. Compute ideal serialization and explain processing,
ACKs, retransmission and queueing excluded from that bound. No measured HTTP
rate can be inferred from a nominal100Mbit/s line rate.

Execute the original three-byte length/type frame, ambiguous1520, excluded8100,
and inconsistent length2 cases. Check all43 padding bytes, CRC coverage and
the hardware-TX/no-FCS versus retained-RX/FCS adapter. The supplied device-CRC
fixture appends four bytes after a60-byte DMA span; it is a behavioral model,
not an F439 observation. An already stripped60-byte capture must not be reduced
to56. Preamble and gap belong to wire/time accounting, never these arrays.

Implement a **new selected single-tag profile** in `ew_vlan_build/parse/pop`.
It follows the inspected historical C-tag encoding:8100, three priority bits,
CFI=0, VID1..4094, then an inner type/length. Priority-only VID0, CFI1, nested
8100/88A8, unassigned type/length1501..1535 and payload>1500 are excluded.
This profile is not a claim of current full IEEE802.1Q conformance; CFI must
not silently be relabeled as a modern drop-eligibility policy. Maximum frame
size including FCS is1522; minimum is64 with at least42 stored payload octets.
Length encoding equals meaningful payload; EtherType parsing cannot infer its
meaningful upper-layer length. Output is transactional; CRC covers addresses,
tag, inner field, data and pad. A parser rejects bad FCS without publishing a
borrow. Pop removes the tag, restores enough padding for the untagged minimum,
and recalculates FCS. It cannot just remove four bytes from a minimal frame.
Prove all bounds before copying. Work is O(frame bytes); construction has a
1522-byte temporary and parsing constant metadata, plus the imported CRC work.

Compare UM1974 Rev8 §6.11/Table11 and Rev11 §7.11/Table11 using the separately
supplied `BOARD_LEDGER.md`. A manual revision is not a board revision. A QEMU
frame trace supplies neither a PHY marking nor a50MHz reference-clock capture.
The physical gate remains required, including fresh observations for changed
board routing; the current author campaign contains none.

## PR-T39-03: identity, capacity and loops

Retain the original four entries, three ports, TTL10 and nonwrapping monotonic
ticks. `ew_bridge` uses the actual existing `ed_bridge`. It additionally rejects
duplicate live MAC keys, non-boolean live fields and group source keys. Policy0
retains atomic FULL. Policy1 removes the oldest `seen` entry (lowest slot breaks
ties) only when admitting a previously unknown source into a full table, then
performs learning and forwarding as one commit. Invalid input never evicts.
Prove the destination decision uses the resulting table. Explain why source
aging and output-port selection cannot be independently committed on failure.

Classify individual/group and universal/local bits in02:00:00:00:00:01,
01:00:5E:00:00:01 andFF:FF:FF:FF:FF:FF. Give two local stations the same02
address; learning alternately moves the location and misdirects a later unicast.
There is no IP address in this experiment and no uniqueness guarantee from the
local bit. Replay the original five-event trace, then fill the table and compare
FULL with oldest eviction. Construct an unknown-destination consequence after
eviction. A promiscuous endpoint cannot receive a frame the switch never sent
to its port.

For the delayed loop task, connect two bridges by two parallel links. Send one
broadcast and trace eight forwarding waves, then prove by induction why the
unblocked cycle can continue: group forwarding has no hop counter and learning
does not alter flooding. Block one redundant port in each bridge and verify the
same trace terminates. This explicitly configured acyclic topology is a repair
for this fixture, not a spanning-tree protocol. A larger learning table cannot
remove the cycle; automatic topology discovery/reconfiguration is outside this
selected model and must not be claimed as implemented.

## Deliverables, verification and assessment

Submit D1 source/ABI/dependency identities and the three implementation APIs;
D2 per-criterion pre/postconditions, state/ownership diagrams and failure proofs;
D3 ordinary and changed input/output traces, independent CRC/arithmetic checks,
defect-control diagnosis and resource audit; D4 the physical-source/observation
ledger with unavailable evidence left explicit; D5 the original-ID seven-day
transfer packet authored without hints or assessor access. Use the original
criteria rather than replacing them with a single overall score.

Native build: compile `tests.c native.c your-wire.c your-ethernet-depth.c` with
both public include directories under C17 and warnings as errors. Select cases
0..5: stream, VLAN, frame, bridge, loop and layers. Exercise both compilers at
O0/O3 and ASan+UBSan; separately build through `target_build.py` with explicit
original provider/runtime/boot/comparison inputs. The target uses a16KiB test
stack reserve. Audit retained stack/callgraph and implicit helper closure;
runtime high-water, wire rate and physical timing remain unobserved.

The separate oracle checks mathematical lengths, standard CRC arithmetic and
finite trace packets; supplied tests cover33667 split choices and1501 payload
lengths. They do not quantify all byte sequences, interleavings or architectures.
Every rejection control must compile and reach its named semantic assertion.
Compilation failure does not count as detecting the intended defect.

Apply the original objective rubric plus these explicit levels to every leaf:
0: wrong boundary/ownership/result or fabricated source/physical evidence;
1: matches ordinary output but cannot derive a bound or counterexample;
2: correct construction/analysis with a justified invariant, exact failure
semantics, changed-condition evidence and limitations;
3: independently assessed delayed transfer and explanation under a new premise.
Author execution cannot award level3 or prove absolute mastery. Route-only
objectives need the actual diagram/source argument, not a passing byte-parser
test. CRC acceptance does not establish peer identity or application authority.

Graduated hints: identify the retained bytes first; then mark the single commit
point; finally compare the original predicate with its changed assumption.
For streams ask what remains after the first length-delimited record. For tag
removal account for minimum padding before recalculating the check. For bridges
separate table capacity from graph topology. Stop using hints before transfer.

Security scope is original local arrays and bridge fixtures, synthetic bytes
and identities only. An attacker controls input length/tag/FCS or local frame
source fields. Harmless success is wrong local acceptance, stale record data,
misdirected port selection or bounded broadcast amplification. Repairs reject
the exact defect and regress its counterexample; checksums and source learning
remain unauthenticated. No external network interaction, host security change
or physical flashing occurs in this campaign. Published evidence distinguishes
authored, native/CPU-executed, independent and physical dimensions.

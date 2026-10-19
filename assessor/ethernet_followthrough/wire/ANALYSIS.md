# Assessor analysis: PR-T39-01–03

Keep this file and the reference implementation outside learner exports.
All original criterion IDs and seven-day IDs remain authoritative. The rubric
is objective-specific: a passing implementation test cannot substitute for
the route, authority or physical-evidence arguments.

<a id="t3901a"></a>
## T39.01.a

The local payload handed down becomes20,40,54 bytes; each added header is
interpreted by its matching remote peer. The original sizes are synthetic.
Late addition overflow must preserve all earlier output slots. The changed
adapter consumes exactly a completed record; `00 01 07 00 00 00 02 08 09`
produces records[7],[],[8,9]. Consuming all nine bytes before returning the
first record loses framing/ownership information. A prefix split and zero
record expose mistakes ordinary nonzero complete inputs hide. There are
sum(n+3,n=0..256)=33667 two-piece splits; this is not all fragmentation or
payload combinations. Partial EOF is not clean EOF. Backpressure begins when
the owned record is ready and ends only after successful extraction.

<a id="t3901b"></a>
## T39.01.b

Application semantics select resource/method; presentation determines the
representation and its encoding; dialogue/session work accounts for request,
response and connection-use rules; transport orders bytes; network addresses
and routes datagrams; link frames and checks local transmission; physical
signaling crosses the medium. These purposes do not mandate seven separate
headers or seven separately implemented components. HTTP is stateless at the
request-semantics level, not a promise that all application state disappears.
Neither a TCP connection nor a reused HTTP connection is authenticated user
identity. The MCU transfer must explicitly acquire timer, retransmission,
window, reassembly, ARP, frame and DMA lifetime work previously delegated to
the OS. The later actual NbNet/NhHttp consumer supplies bounded implementations;
the stream adapter alone cannot be marked as that stack's implementation.

<a id="t3901c"></a>
## T39.01.c

Bridge forwarding and bad Ethernet FCS first implicate distinct link contracts;
missing next-hop selection is network-layer routing; duplicate bytes exposed
by a promised reliable ordered stream violate its transport service; a
well-formed rejected command can be correct application authorization behavior.
Do not assert root cause from a layer number alone. Identical C signatures
can assign opposite byte order to01/00. The reference big-endian peer waits for
256 bytes while the changed little-endian peer expects1; all type signatures
and nominal OSI classifications can still match. A protocol ID is only useful
when both parties assign it the same actual semantics.

<a id="t3902a"></a>
## T39.02.a

F439's MAC assembles/accepts frames; RMII crosses to the PHY, whose coding and
electrical signaling cross magnetics to the cable. MDIO/MDC manages the PHY
and is not the frame-data channel. In the explicitly hypothetical external
SPI-controller architecture, frame-engine registers/buffers are remote bus
resources. Their admission, ownership, timeout and FCS conventions need a new
contract; the bus does not implement cable signaling. MCP2518FD is a CAN FD
controller and is not evidence for a real SPI Ethernet controller.

<a id="t3902b"></a>
## T39.02.b

Half duplex shares a collision domain with carrier sense/collision handling.
Full duplex permits simultaneous independent directions and removes that
collision model. LAN8742A's inspected parallel-detection rule presumes half
duplex when the peer cannot negotiate. A full-only driver must reject the
result; forcing the MAC to full duplex does not repair a mismatch. A32-byte
application payload with the declared fixed IP/TCP headers consumes110 octet
times including preamble/gap,8800ns at100Mbit/s, before processing, ACKs or
retransmissions. This is neither measured throughput nor a worst-case bound.

<a id="t3902c"></a>
## T39.02.c

The selected Rev8/Rev11 table comparison preserves the same nine signal routes
but changes section/page and the paragraph's PHY naming detail. Neither source
proves actual populated board revision or a50MHz waveform. Reject an emulator
trace labeled as physical acceptance. A physical packet with no raw identity,
instrument settings or source/image binding also fails. The changed board
requires a fresh ledger; no observed values may be copied from the specification.

<a id="t3903a"></a>
## T39.03.a

The three-byte length frame has43 pad bytes; a type frame cannot infer the
upper-layer meaningful length. Field1520 is in the excluded gap,8100 is
unsupported in the original profile, and length2 disagrees with three supplied
bytes. The new single-tag profile is a separate API:18-byte header,42-byte
minimum data/pad and four-byte FCS, retaining1500-byte maximum payload and
increasing maximum frame to1522. It deliberately uses the inspected draft's
CFI=0 profile; it must not advertise unimplemented modern DEI, priority-only,
stacked-tag, VLAN-membership or current-standard conformance. Reserved and
excluded inputs preserve output. VID1/4094 and priorities0/7 should also be
independently challenged; there is no security identity in a VLAN ID.

<a id="t3903b"></a>
## T39.03.b

64 stored wire-frame octets are14 header+46 data/pad+4FCS. Preamble/SFD and
gap account for20 more octet times but are absent from this array. If capture
strips FCS,60 is the correct total; subtracting again produces a runt. Popping
the tag from a64-byte tagged frame requires replacing lost padding before
computing the untagged FCS. Blindly deleting four bytes produces60 bytes
including FCS and violates the selected minimum. The parser's borrowed span
retains the lifetime of the original capture and cannot outlive DMA reuse.

<a id="t3903c"></a>
## T39.03.c

Changing pad byte17 leaves the CRC over the three meaningful payload bytes
unchanged while invalidating whole-frame FCS. The target fixture's frame CRC
for the fixed length3 example is2763460683; the Python oracle independently
uses zlib over an explicitly constructed header/data/pad image. Hardware TX
must receive the no-FCS span and add one FCS; retained-FCS RX validates the
complete frame before presenting the upper span. Those are different contracts.
The behavioral device step does not claim that F439 hardware has executed it.

<a id="t3904a"></a>
## T39.04.a

02 has individual=0/local=1,01 has group=1/local=0,FF is broadcast and has
both low bits set. Local administration does not ensure uniqueness. When two
stations use A on ports1/2, later source learning moves A to2 and a third
station's destination A is sent there. There is no IP namespace in this test;
changing an IP address cannot repair the duplicate MAC forwarding identity.

<a id="t3904b"></a>
## T39.04.b

The original masks are6,1,2,0,3. At tick12 B last seen1 has expired; A last
seen3 remains, and C is learned on3. The FULL variant commits neither time nor
output. The eviction variant removes smallest seen, ties by lowest slot, then
learns and forwards in the new table. Unknown destinations may flood where a
previous retained key would have selected one port. Regressing time, malformed
ports/flags/group keys and duplicate live keys fail before mutation. This is
a selected finite policy, not a complete VLAN filtering database.

<a id="t3904c"></a>
## T39.04.c

Promiscuous mode changes receive filtering only for frames delivered to the
endpoint's port. The bridge may send another unicast solely to a different
port. Two parallel inter-bridge links make a cycle: every group frame received
on one exits the other. Eight bounded waves demonstrate16 forwards, and the
transition rule supports the induction for continued circulation. More table
entries do not affect group flooding. Blocking one redundant link makes the
specific graph acyclic; the observed one forward then termination is a static
repair, not an implemented spanning-tree election or recovery algorithm.

## Evidence and withheld assessment

Mutants cover consumed suffix, peer byte order, tag control layout, missing
FCS validation, ingress reflection, newest-instead-of-oldest eviction, original
pad behavior, original FCS bypass and wrapper arithmetic/compatibility. They
must reach their intended assertions; compiler rejection is separate evidence.
For delayed assessment withhold this analysis and choose a new fragmentation,
VID/priority boundary, capture convention or topology while retaining the
original criterion ID. Require the learner to derive the outcome before
execution. Independent review, delayed learner performance and actual physical
board evidence are absent from the author campaign. A finite exhaustive loop
over a chosen bound is not a universal proof or absolute mastery.

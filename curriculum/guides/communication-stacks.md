# Three complete communication project families

The three families teach different ownership boundaries. Each consists of small module milestones with exact contracts, fixtures, failure cases and evidence gates. The family view adds no hours to the workload table: every milestone is already charged in its owning module.

| Family | Owned implementation | Supplied mechanisms | Completion evidence |
|---|---|---|---|
| {ref}`T40`: EDU7-v1 | Original seven-layer protocol, routing relay, message transport, dialogue/session, codec and application | MCU UART engine and declared RS-485 transceivers/media | Independent layer faults plus actual two-segment electrical and end-to-end routed exchange |
| {ref}`T41`: Ethernet through HTTP | Custom PHY-management/MAC-DMA adapter, Ethernet/ARP/IPv4/ICMP/TCP subset and HTTP integration | F439 MAC hardware, LAN8742A analog PHY, magnetics and cable; previously authored T42 HTTP core | Real PHY/MAC/DMA records and independent ARP/ping/TCP/HTTP captures, loss/flow/close/cable-reset campaigns |
| {ref}`T42`: HTTP server | Original HTTP octet parser, router, serializer, socket lifecycle, queues and deadlines | OS TCP/IP/link/physical stack | Original C inspection and independent curl/raw-client results with fragmentation, framing, partial I/O and bounded resources |

X.200 supplies a conceptual service/peer model, not a wire implementation specification. EDU7-v1 is an original educational protocol designed to make all seven responsibilities observable. Internet protocols use their own normative contracts; TCP/IP and HTTP do not form seven matching OSI protocols. See {ref}`R-NET-X200-1994` and the exact assigned RFC/device passages.

## Family 1: a real seven-layer exchange

Use endpoint A, router R and endpoint B across two distinct RS-485 segments. R has two independent UART/transceiver channels and relays at the network layer. Each segment has its own master/grant schedule and local link addresses. The source/target IDs at different layers are separate namespaces.

| Layer | Original responsibility and public service | State/identity owned here | Fault that must stop here |
|---|---|---|---|
| L1 physical | Declared UART/transceiver configuration exposes ordered octets and explicit UART/link faults | Bit timing, DE/receiver state, medium and local transmit ownership | Bad voltage/wiring, contention, framing/overrun and premature DE release |
| L2 data link | Delimit/escape/CRC bounded frames and grant access on one segment | Segment-local source/destination, frame parser and grant state | Bad CRC/length/escape, foreign local address or invalid grant |
| L3 network | Deliver bounded end-to-end datagrams through a static relay | Node IDs, route/interface/next hop and hop limit | Unknown route, invalid protocol/length or exhausted hop limit |
| L4 transport | Segment/reassemble complete messages with matching ACKs, credit, probes and bounded retry | Ports, transfer IDs, fragment index/count and reassembly | Missing/duplicate/wrong ACK, out-of-order fragment and blocked credit |
| L5 session | Negotiate the schema/units, control dialogue token, checkpoints and retained-state resume | Session ID, epoch, token holder and committed checkpoint | Token violation, incompatible OPEN, conflicting duplicate or lost-state resume |
| L6 presentation | Convert original typed records to canonical bounded TLVs | Schema, tag ordering, integer byte order, units and UTF-8 validity | Malformed/noncanonical representation or unrepresentable typed value |
| L7 application | Execute bounded GET/SET fixture-plant operations and correlate responses | Request ID, setpoint and permitted value range | Unknown verb, missing semantic fields or out-of-range SET; old state survives |

Adjacent-layer services use owned spans or complete events with explicit consumed/retained storage. Peer headers travel as lower-layer payload. The router copies the datagram before returning an RX buffer; it does not decode or apply the application's SET. A CRC is integrity evidence, not authentication. A successful transport ACK is separate from session commitment and successful application execution.

Work a SET of 2500 centidegrees with request ID 0x1234. Its presentation PDU without label is 15 octets: schema 1, verb TLV, request-ID TLV, signed value TLV and unit TLV. A seven-octet session DATA header makes 22 octets. One transport fragment fits the 24-octet fragment payload; its eight-octet header makes 30. The six-octet network header makes 36, within the 40-octet link payload. Five link header octets plus CRC make 43 unescaped body octets. Compute CRC before delimiter escaping; actual wire length depends on escaped values.

R preserves network source 17/destination 51 and every transport/session/presentation/application octet, decreases hop 4 to 3, selects the other segment and rebuilds that segment's link frame/CRC. The local address numbers 1/2 may recur on both segments; their meanings belong to their segment, not a global node namespace. Verify both ingress and egress capture points. A direct A–B loopback cannot close the relay objective.

Complete PR-T40-01 through 08 in order, reusing T38's byte-channel interface and earlier parser/queue/timer work. The physical milestone measures the actual declared transceiver, guards, final-stop-bit release and cable setup; porting the same API to another transceiver requires part-specific timing evidence. Host traces may close higher-layer tests while physical gates remain pending. Checkpoint/resume retains RAM state only. Peer reboot refuses resume; no durable exactly-once or authenticated-session claim follows.

## Family 2: Ethernet layer 1 through HTTP

The selected physical route is the NUCLEO-F439ZI/MB1137 with LAN8742A. Verify the actual board marking, revision, fitted bridges/jumpers and inspected manual boundary. The PHY and magnetics implement analog cable signaling. Learner C configures and observes the digital PHY-management/MAC/DMA route and constructs the higher protocols. A byte array or host TAP interface cannot close physical layer 1.

| Boundary | Contract and ownership | Required evidence |
|---|---|---|
| Cable/PHY | Actual media, reset/identity, reference clock, link and negotiated mode | Board/part identity, RMII clock/status and cable removal/recovery |
| MAC/DMA | Reachable buffers, normal descriptors, complete publication before OWN and completion before reuse | Map/ordering ledger, descriptor/status traces, backpressure and reset handling |
| Ethernet | Untagged Ethernet II, local/broadcast filtering, type, FCS and padding policy | Validated frame spans; exclude preamble/SFD and subtract retained FCS exactly once |
| ARP/IPv4/ICMP | Bounded configured-peer resolution, fixed IPv4 header/checksum/length and unicast echo | Cache/timer, corrupt length/checksum, exact echo and explicit options/fragment rejection |
| TCP | Passive-open tuple, checksums, modular sequence/ACK/window, retransmission, flow and close | Loss/duplicate/prefix/zero-window/timer/FIN/RST/TIME-WAIT traces |
| HTTP | Reused portable T42 parser/serializer on preserved stream bytes | GET/HEAD/binary echo and error response through actual custom transport |

The precise controlled profile is in the T41 contracts. Static addresses, one configured peer, one connection, a small MSS and one outstanding segment make state inspectable. Omitted IPv4 options/reassembly and general TCP event/congestion coverage mean this is a restricted interoperability laboratory. A cooperative client exchange does not certify a general Internet host.

Use virtual monotonic time for deterministic TCP timer tests; actual firmware keeps its declared physical linger/deadline behavior. Distinguish congestion allowance from advertised receive window and from local storage. ACK-only, SYN and FIN consume different sequence space. IP total length excludes Ethernet padding; TCP checksum includes its IPv4 pseudo-header. PHY link, MAC completion, TCP acceptance and HTTP application success are different evidence.

T39 supplies the architecture and validated frame-channel mechanisms. PR-T41-01 through 08 add finite protocol state, exact controller work and end-to-end closure. Reuse the T39 adapter and its artifacts when compatible; charge only the changed controller/protocol/integration work specified in T41. A solved vendor Ethernet driver or lwIP/OS TCP stack cannot perform the custom work being assessed.

## Family 3: an HTTP server from scratch

Begin this family on Linux loopback using the previous event/descriptor adapters. PR-T42-02 through 05 build the portable grammar, framing, chunk decoder, router and serializer. PR-T42-01/06/07 attach the owned socket lifecycle, partial-output persistence and deadlines. PR-T42-08 requires independent clients and a scope defense.

GET and HEAD /health select exactly three representation octets, hex `6f6b0a`. HEAD sends the GET-equivalent length and no body. POST /echo returns the bounded original octets. The profile supports bounded origin-form and absolute-form authority, separate trailers, immediate 100-continue or header-determined error, FIFO responses, strict ambiguous-framing rejection and staged close. Root fixtures include exact one-request close-mode headers for reuse by T41.

Test every selected byte split, multiple coalesced requests, duplicate Content-Length, TE+CL, chunk extensions/trailers, premature EOF, short sends, EAGAIN/EINTR, slow progress and request-total deadlines. Following optimistic CONNECT/Upgrade bytes and requests after close never receive another application dispatch. RFC 9112 plus the inspected March 2026 RFC 9931 update inform the limits; the project's unsupported-transition rejection policy remains explicitly an origin-server policy.

No HTTP library implements the target. OS sockets supply the lower stack; curl and an independent raw-octet peer are verification tools. Completion requires actual original learner C and peer results, rather than the supplied static fixture verifier alone.

## A practical shared route and equipment

1. Preserve the accepted T14/T15/T18 library and T23/T24 adapters. Complete the relevant machine/startup/driver/ISR/DMA prerequisites.
2. Develop T36/T37 alongside the appropriate earlier foundations. Complete T38 serial/electrical and T39 frame-channel gates.
3. Build the portable T42 parser/serializer milestones before the final T41 HTTP milestone. They are charged once in T42. Hosted-server work can progress while physical equipment is unavailable.
4. Complete the original T40 seven-layer family and the T41 custom Internet subset as separate contracts. Reuse mechanisms and evidence by original project ID.
5. Defend each family's layer ownership, source scope, limits, resource bounds and physical-versus-synthetic closure independently.

| Route | Declared equipment and evidence boundary |
|---|---|
| Hosted algorithms/HTTP | Linux x86-64/WSL toolchain, debugger, independent clients and injected test clocks |
| Thumb/MCU | Exact Cortex-M4/F439 toolchain/ABI, startup/map artifacts and selected board |
| Serial electrical labs | Declared matching UART/USART/SPI/I2C targets, logic analyzer/appropriate voltage measurement, transceiver/connector/power documentation |
| EDU7 physical | Three declared MCU nodes, router with two UARTs, four THVD1400 adapters, two short terminated bench segments and common-reference measurement |
| Ethernet physical | Declared F439ZI/LAN8742A board, dedicated host Ethernet adapter/cable and independent packet capture; isolated same-link configuration |
| Optional physical CAN FD | Supported external controller/adapter or alternate native-FD board, correct separate supplies, exact additional setup readings and an independent peer |

Equipment identity and physical measurements belong to the learner's evidence. The curriculum supplies contracts and static/reference oracles; it does not report these future hardware runs as completed. See {ref}`guide-workload` for totals and {ref}`guide-assessment` for mastery records.

<!-- generated milestone accounting -->

## Milestone directory and accounting

These are module milestones, not an additional capstone assignment. Their implementation and verification hours are already included in the module workload.

### EDU7-v1: all seven OSI responsibilities

| Milestone | Work | Hours |
|---|---|---|
| {ref}`PR-T40-01` | Seven boundary contracts and encapsulation ledger | 4 |
| {ref}`PR-T40-02` | Two-segment RS-485 grant scheduler and measured adapter | 8 |
| {ref}`PR-T40-03` | Incremental escaped link framer with independent CRC | 6 |
| {ref}`PR-T40-04` | Static two-link router with hop and lifetime oracles | 6 |
| {ref}`PR-T40-05` | Stop-and-wait message transport with atomic reassembly | 10 |
| {ref}`PR-T40-06` | Negotiated session, dialogue token and retained-state resume | 8 |
| {ref}`PR-T40-07` | Canonical TLV presentation with strict UTF-8 | 8 |
| {ref}`PR-T40-08` | Routed SET/GET exchange and physical family closure | 8 |

Owning-module project work: **58 hours**. Added family budget: **0 hours**.

### Ethernet layer 1 through HTTP

| Milestone | Work | Hours |
|---|---|---|
| {ref}`PR-T41-01` | Controlled Ethernet endpoint and ownership ledger | 4 |
| {ref}`PR-T41-02` | Actual PHY readiness, MDIO deadlines and link incarnation | 8 |
| {ref}`PR-T41-03` | Original normal-ring MAC/DMA driver with reuse recovery | 10 |
| {ref}`PR-T41-04` | Untagged Ethernet and one-peer ARP state | 8 |
| {ref}`PR-T41-05` | Fixed IPv4, odd-span checksums and exact Echo replies | 8 |
| {ref}`PR-T41-06` | Passive TCP with wrap, overlap, reset and close traces | 12 |
| {ref}`PR-T41-07` | RTO estimation, one-flight pressure and zero-window probes | 12 |
| {ref}`PR-T41-08` | Shared HTTP core over custom TCP and measured cable closure | 10 |

Owning-module project work: **72 hours**. Added family budget: **0 hours**.

### HTTP server from scratch over OS sockets

| Milestone | Work | Hours |
|---|---|---|
| {ref}`PR-T42-01` | Owned socket lifecycle | 6 |
| {ref}`PR-T42-02` | Incremental header and authority parser | 8 |
| {ref}`PR-T42-03` | Unambiguous body framing | 8 |
| {ref}`PR-T42-04` | Chunk decoder with separate trailers | 8 |
| {ref}`PR-T42-05` | Router and HTTP response serializer | 8 |
| {ref}`PR-T42-06` | Persistent output and close state | 10 |
| {ref}`PR-T42-07` | Deadline and failure campaigns | 8 |
| {ref}`PR-T42-08` | Independent client and parsing-security defense | 8 |

Owning-module project work: **64 hours**. Added family budget: **0 hours**.


# Executable communication components

Implement the public headers in `learner/`. The initial C sources compile and
deliberately fail the behavioral tests. `assessor/communications` contains a
separately selected original reference implementation, including the socket
event loop; it is not copied by a normal learner workspace initialization.

```sh
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

An assessor explicitly adds
`-DCMASTERY_ASSESSOR_ROOT=/absolute/repository/assessor/communications`.
Clang/GCC sanitizer runs add
`-DCMAKE_C_FLAGS="-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer"`.
The socket test needs Linux, Python and curl. It binds only loopback, asks the OS
for a free port and terminates its own server. The standalone server defaults to
127.0.0.1:8080; `com_server 0` prints its ephemeral endpoint. The portable core
uses C17 plus memory/string operations, fixed storage and injected time.

## What actually executes

| Component | Executed boundary | Separate evidence still required |
|---|---|---|
| HTTP | Octet parser, framing, chunks/extensions/trailers, router/serializer, bounded output offsets, deadlines, real nonblocking Linux loopback sockets and independent raw client/curl | Project-specific resource/descriptor measurements, unseen diagnosis and delayed transfer |
| EDU7 | Original CRC/escaping, two distinct byte-stream link decoders, copied static forwarding, retry/credit/reassembly receipts, session/token/checkpoint, canonical TLV/UTF-8 and application setpoint | Full grant-scheduler integration, both directions/epochs and timer wrap campaign, physical UART/RS-485 topology and measured timing |
| Ethernet-to-HTTP | Actual C Ethernet/ARP/IPv4/ICMP/TCP decoding and serialization, passive handshake, sequence wrap/duplicate/gap/reset, one-flight response, retry/RTO, zero-window probe, FIN/TIME-WAIT, same HTTP core | Active ARP retry/queued-datagram mechanism, complete TCP close/window/options campaign, PHY/MDIO and actual STM32 normal-ring register driver, independent physical packet capture |
| MCP2518FD | Original command/address codec, little-endian register data, bounded Configuration-mode handshake before RESET, mock disconnect/stuck mode | Clock/oscillator configuration, RAM/FIFO/filter/interrupt initialization, actual loopback and CAN FD peer exchange |
| DMA | Original ownership transitions, publish barrier callback, reuse, checked single-frame length/FCS removal | Real SRAM placement, descriptor/status translation, processor/device ordering and W1C recovery |

These named tests are **component anchors**, not automatic completion of every
objective in a mapped project. In particular `edu7_physical_model`, `mcu_spi`
and `mcu_dma` contain no physical measurement. A passing packet test is not a
PHY run or general TCP/IP conformance. Follow the curriculum physical dossier
and retain every pending item above when reviewing a mapped milestone.

## Contracts and experiment design

The normative original lab choices remain
`research/enhancement/networking_contracts.json`. `family.json` maps exact CTest
names to existing IDs without replacing those project contracts. The supplied
tests use independent byte-level checksum arithmetic, every split point for
representative HTTP messages, each premature EOF, binary body bytes, invalid
framing, deliberately lost transport ACK, duplicate-completion receipt, timer
advance and an actual two-segment forwarding path. The deterministic malformed
corpus is finite, seeded and repeatable. A passing model never licenses an
unobserved hardware or complete RFC claim.

HTTP parsing retains no borrowed input. Completion stops exactly at one message
boundary; reinitialization is explicit. The server owns four connections and two
output slots per connection. The queue applies read backpressure, responses stay
FIFO, and close/error stops later optimistic request dispatch. Send offsets
advance only on positive progress. Fixed Date is a trusted injected fixture
value. DNS/IPv4 configured authority is the bounded profile; IPv6 URI literals
are a separately identified extension. Content and output are octets.

EDU7 state is per declared endpoint direction/port tuple and per session epoch.
After reopen, initialize transport/session objects rather than reuse completed
IDs across epochs. `EduSession.effects` counts accepted L5 publications, not
successful L7 changes. Only the application owns setpoint mutation. A complete
unconsumed message occupies one receiver slot; a new DATA fragment is not ACKed
until accepted. A PROBE reports current credit. An ACK of an accepted fragment
advances that fragment independently of the credit for the next fragment.
`edu_tx_begin` starts with credit1: its caller/session must admit a new message
only after the peer slot is known available. The fragment engine does not own
that inter-message admission policy. Final and duplicate-final ACKs advertise
credit0 while the completed receive slot remains occupied.

The custom network adapter uses 192.0.2.2, peer192.0.2.1, port8080 and fixed MACs
from the contract. Fixture ISNs are not suitable production randomness. It
requires the configured peer's validated ARP exchange before serving IP. The
portable packet driver is ready to connect to a learner MAC service but contains
no vendor driver or OS TCP/IP implementation. `net_link_down` clears the live
incarnation. Physical TIME-WAIT remains240000ms; tests advance an injected clock.

MCP helpers implement only their named small boundary. The caller supplies
measured SPI mode/clock and a synchronous CS-owning transfer; helper return does
not prove hardware timing. The 32-bit helper excludes IOCON (byte-write rule),
reserved address regions and unaligned RAM accesses. A RESET is emitted only
after OPMOD reports Configuration. This is not a complete initialization driver.
The TCAN4550 metadata fixture is a different controller and remains separate.

## Evidence and recovery

Record source hashes, compiler flags, exact CTest name, failing input/clock and
first invalid transition. Run the relevant small group before the cumulative
pipeline. Keep raw socket/packet observations and capacity/lifetime ledgers.
Review `HINTS.md` progressively. Expose a controlled wrong implementation only in
an isolated learner copy and demand behavioral rejection; a compile error does
not show the intended invariant was tested. Test and explain one changed
condition after seven days.

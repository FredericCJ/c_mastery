# Dedicated IPv4/Echo and reassembly capture protocol

This protocol is authored, not performed. Use only dedicated teaching firmware,
an isolated Ethernet segment and a synthetic peer configured as192.0.2.1; target
192.0.2.2. No routed connection, production data, external traffic or host security
setting change is authorized. Board flashing requires a separately authorized
hardware session. Complete T39 PHY/ring qualification and its exact copied-port
adapter first; packet-array/emulator success is not a substitute.

Record board/PCB/PHY identity, clocks, firmware SHA256 and readback, compiler,
linker map, DMA descriptor/buffer placement, driver revision, lower-port copy/ownership
contract, cache policy and quiescence protocol. Identify the capture interface,
software and version, capture timestamp source/calibration, receive/transmit
offload configuration as observed, tap/capture location, FCS presence and snaplen.
Do not disable host protections or fabricate missing fields. If outbound captures
precede checksum offload, use a qualified independent receive-side capture. Retain
the immutable raw capture and its hash; extracted hex must name packet indices.

First predict each frame independently. Establish association through a real
ARP request. Send Echo with id1234/seq1 and data6162636465, then the peer's actual
default56-byte ping and an explicit64-byte payload. Preserve the actual payload,
since a ping program may embed timestamps. Compare source/destination, type/code,
id/seq, IP/ICMP lengths/checksums, payload, padding and available FCS. Pre-FCS
lengths are60,98,106 respectively. Absence of FCS in the capture is a capture
property, not missing hardware transmission. Capture counts must agree with
descriptor-reported lengths and software admission. Reject a clipped frame.

For the reassembly firmware variant, use two original fragments from the same
Echo datagram. Deliver last first, then first; expect one complete Echo reply.
Leave a gap; no Echo reply may occur. At the qualified60000ms deadline, require
type11/code1 with the unchanged first fragment header and8 data bytes. With only
a noninitial fragment, require no timeout reply. Repeat an identical overlap in
each policy and introduce a conflicting overlap; distinguish strict/identical
admission and require no mixed datagram. Use at most two active identities and
explicitly inspect refusal of a third. Do not infer permanent replay prevention:
identity reuse after the tombstone deadline is allowed.

Repeat with receive spans delivered at unaligned host addresses and while the
copied TX port temporarily refuses admission. The output owner retains each
complete packet until successful copied admission, then releases it exactly once.
Record cancellation/quiescence if a link reset interrupts transmission. A timeout
packet constructed in memory but never accepted by the driver is not wire success.
Capture every lost/refused path and count; do not replace the observation with
the emulator's expected trace.

Use fixtures/networking/ip_physical_capture.json to index the three Echo captures.
Run `python tools/check_network_ip_capture.py <record.json>`. This checker verifies
structural byte relations and raw identity, not authenticity of the operator,
measurement calibration or hardware behavior. A qualified reviewer must inspect
raw captures, reassembly/timeout sequences and timing uncertainty. Record the
reviewer separately from the author; leave these gates pending until observed.

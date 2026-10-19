# Required isolated TCP observations

This package is authored; no board observation is supplied. Use a dedicated
F439/LAN8742 teaching board only after its T39 pin/clock/PHY/ring contracts pass.
Authoring this protocol does not authorize flashing hardware or modifying host
security. Connect only a dedicated laboratory peer, synthetic192.0.2.0/24 data
and the original TCP teaching firmware. No deployed device or external service
is a target. Disposable guests may provide the independent peer.

Record exact board/revision, PHY, clock source/calibration, compiler/options,
firmware ELF SHA256 and linker map, EnRing adapter source SHA256, ISR priority,
DMA/cache policy and packet-copy acceptance timestamp location. Qualify RAM,
six2048-byte buffers and normal DMA descriptors from T39. Establish link before
opening the teaching port. Retain raw PCAP and decoded fields, event JSON and
clock uncertainty; state capture point, timestamp domain, snaplen, FCS inclusion,
checksum offloads and packet loss. A capture before checksum insertion is not
a TCP checksum failure at the peer. Use a reference peer with independently
implemented packet decoding, not a second copy of NcStack.

Run six scenarios, resetting only after lower-layer quiescence with a new epoch:

1. **normal**: handshake with peer MSS100; split/coalesce a request and compare
   the exact response body and byte ACK ledger. Log prepared, copied, DMA-reaped,
   peer-ACKed and parser-consumed as distinct events. Include both ACK/DMA orders
   by deterministic teaching-driver event injection; label injection versus
   actual wire scheduling. Hold the lower copy port BUSY longer than1000ms;
   confirm no flight/timer until acceptance and compare the accepted timestamp
   with the eventual on-wire timestamp.
2. **loss**: the dedicated peer withholds ACKs. Observe accepted retransmissions
   at origins plus current RTO and abort after three; nominal immediate-admission
   ledger0/1000/3000/7000/15000 is a model prediction. Compare measured intervals
   with clock uncertainty and scheduling/copy delay, without rewriting the
   deadline to hide early retransmission. Repeat a lost SYN and verify3000ms
   initial data RTO. Show retained bytes unchanged after DMA reap.
3. **zero**: peer advertises0; observe no new application progress. Capture the
   declared ACK probe at SND.NXT-1 and backoff, then reopen8 with a recorded
   wrapped32-byte flight in the local state fixture. Separately stop the actual
   HTTP consumer until RX capacity is full, consume a prefix and capture the
   window update. Distinguish the synthetic wrap fixture from physical ISNs.
4. **close**: active, passive and simultaneous close; peer FIN before local FIN
   ACK must traverse CLOSING. Keep the real240s TIME-WAIT; accepted repeated FIN
   restarts it. A four-byte body truncated to two plus FIN must yield an HTTP
   framing error, not successful EOF. Record timeout clocks through the wait.
5. **reset**: inject only into this teaching target, with known tuple and
   checksums. Exact reset aborts, unequal in-window challenges, outside-window
   produces no response. Compare the purpose-built mutant that accepts the
   unequal reset. Harmless success is loss of the synthetic connection's state.
   No arbitrary-code-execution/privilege claim follows. Exact sequence knowledge
   remains sufficient to reset; document residual lack of authentication and
   challenge-rate protection.
6. **quiescence**: refuse copy, queue a copy, cancel/reset link and deliver a
   delayed completion. Record EnRing epoch/ticket and NcTicket mapping. Reuse
   no buffer until actual DMA stop/reap proof; old tickets must not retire a new
   connection. Log fault/recovery paths and verify logical abort does not erase
   the lower outstanding owner.

Copy `fixtures/networking/tcp_physical_capture.json`, replace only with observed
values, and run `python tools/check_network_tcp_capture.py record.json`. Its
checks validate file identities, required scenarios and local event timing;
they do not authenticate observations or decode the PCAP. Independently decode
IPv4 total/header/checksum and TCP pseudoheader/checksum, tuple, flags, option
lengths, sequence/ACK/window and exact payload from the raw capture; compare each
packet to the event ledger, including padding and truncated snaplen. Retain that
criterion-indexed table and reviewer analysis. Record loss/offload ambiguity as
unresolved rather than silently dropping a mandatory packet. Measurements,
manual packet analysis and non-author review remain separate evidence items.

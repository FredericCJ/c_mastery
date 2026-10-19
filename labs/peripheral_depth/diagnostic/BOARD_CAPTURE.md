# Four-column board experiment and changed-PHY assessment

This is a required unexecuted physical protocol, not a board receipt. Use only
dedicated development boards and a directly connected isolated teaching peer.
Do not attach the target to an enterprise/home production LAN. Use synthetic
diagnostic payloads; isolate bench equipment and power according to the actual
board/instrument manuals. No flashing or host security changes are performed by
the authoring campaign.

Start with the active NUCLEO-F439ZI/MB1137 route, selected assembly/revision and
LAN8742A. Confirm actual populated parts/bridges against the schematic and record
photos/inspection identifiers. The generic T28 GPIO contract does not establish
the Ethernet pin route. Before driving pins, resolve exact RMII alternate functions,
PHY reset/address straps, connector isolation, supply voltage, reference-clock
source/direction and conflicting PA1 use. UM1974 Rev11 selected earlier routing
passages are retained; the current full-document fetch failure is a source gap,
not permission to guess. Exact board schematic and errata entries must be cited.

The application is the six-byte fixture `000100020102`; also send zero/32-byte
payload, sequence65535, replay, malformed lengths and abrupt link loss. Run the
declared IPv4/UDP adapter with exact stack configuration separately from the
original raw length-frame adapter. An Ethernet length-frame cannot be described
as an IPv4 packet. Record port/address configuration, FCS stripping policy and
whether checksum offload or timestamping requires enhanced descriptors. Target
buffers must be proved by linker symbols and address intervals, not by a host
pointer cast. Descriptor writes and observation barriers follow T39; a C compiler
barrier alone does not order the device bus.

| Column | Required observation | Positive and rejecting control |
|---|---|---|
| Controller | Register configuration/readback; actual RX/TX descriptor addresses, OWN transitions, FS/LS/FL/error fields, queue/recovery trace | One completed frame and retained borrow; OWN still set or incomplete chain must not publish an application message |
| PHY/link | Actual PHY ID/revision, MDIO transaction/readback, two BSR reads for latch-low history, negotiated mode and reset/link history | Ready mode; disconnected cable/held reset/wrong PHY fixture must prevent an asserted ready link |
| Protocol stack | Declared captured frame/IP/UDP bytes, padding/FCS/checksum policies and exact stack build/configuration | Correct local packet; bad lengths/checksums/port produce the specified discard rather than application acceptance |
| Application | Accepted sequence/payload and count, exact byte representation and lifetime/replay results | First valid request once; duplicate/wrap/oversized input must not repeat its synthetic effect |

Capture MAC/DMA state through the dedicated debug/UART path without reading
read-to-clear registers incidentally. Pin every image, map and trace by hash.
Record instrument model, firmware, probe loading, sample rate/bandwidth, trigger
edge and pre/post-trigger window. Capture RMII REF_CLK and chosen TX/RX control
signals only with suitable bandwidth/probes and documented access; a packet
capture at the peer does not establish those waveforms. State timestamp origin,
clock synchronization, resolution, uncertainty and drift bound before ordering
events across instruments. Measure ready-to-publication and peer-return latency
under named load, repeated resets and link renegotiation; report distributions
and observed maxima, not a proved WCET. Keep failure timeouts/poll bounds explicit.

After every failure, disable admission, confirm DMA stopped and any FIFO drained,
return or quarantine each buffer, and restore configuration through the target's
documented reset sequence. A timeout or cable loss does not return descriptor
ownership automatically. Record cleanup proof before removing power or reusing
memory. Physically repeat after changed routing; rerunning a host parser is not
that repetition.

For the seven-day variant, provide a replacement-PHY datasheet and altered board
route as independent inputs. Keep application bytes fixed. Submit a dependency
diff for identity/strap/reset, MDIO register meanings, clocks, pin electrical
requirements, negotiated mode interpretation and capture alignment. The supplied
`evidence.py` invalidates records using their declared dependencies; it does not
authenticate measurements. `synthetic-fixture` records can exercise its rejection
rules and can never become physical evidence merely by passing the checker.

Physical completion requires the trace files, source identities, exact procedure,
instrument settings, uncertainty analysis, positive and negative observations,
and a reviewer able to inspect them. Published standards-conformance certification
is a different claim and requires the applicable full standard and test process.

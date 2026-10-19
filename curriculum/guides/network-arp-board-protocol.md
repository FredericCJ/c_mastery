# T41.04 physical association and queue observation

This is an unexecuted acquisition protocol. It applies only to a dedicated
teaching board with the actual qualified T39 frame driver and one controlled
peer on an isolated cable/switch. Use synthetic192.0.2.1/.2 data. Do not attach to
an operational network, change host security settings, or flash hardware as part
of author-side curriculum validation. A qualified operator performs subsequent
board setup under its separate authorization.

Before acquisition retain board/revision, PHY model/revision/address, schematic
route, clock source/rate/tolerance, firmware ELF/source/link-map hashes, driver
configuration and runtime-closure evidence. Record the physical DMA-reachable
buffer/DMA descriptor placement. The lower adapter must own its copy after accepted
admission and retain it until actual device quiescence; record its exact ticket
and completion contract. CPU-model execution alone does not qualify this premise.
An API that borrows the caller frame requires an owning queue adapter first.

Use a dedicated external Ethernet capture port or isolated peer capture with
explicit timestamp clock/resolution and capture-loss counters. Export complete
frame bytes excluding FCS consistently, direction, and timestamp. Retain the
original capture file and its SHA256; produce the JSON summary without discarding
the original. The summary checker validates structure and finite byte relations,
not whether an instrument or operator is truthful.

Synchronize the firmware millisecond trace and capture timebase with at least
two observable markers spanning the run. Bound tick quantization, oscillator
drift, marker uncertainty, capture timestamp uncertainty and buffering. Report
intervals rather than exact physical1000ms claims. Capture request preparation,
copy acceptance, DMA completion and received ARP publication as separate events.
Record initial time origin, monotonic units and uncertainty budget explicitly.

Perform these controlled observations:

1. With peer response disabled by the teaching peer's own application, enqueue
   one synthetic datagram. Observe request slot0/1/2 and expiration of the pending
   ownership at3000 software milliseconds. A second enqueue is BUSY. Compare
   physical intervals using the calibrated bounds; do not equate software calls
   with wire transmission. Repeat with delayed service to verify no catch-up burst.
2. Restore the peer response. Capture request/reply fields and show the observed
   sender MAC reaches outbound Ethernet DA while the queued IP bytes/checksum
   are preserved. Force lower copy-admission refusals using the dedicated adapter's
   local queue capacity; verify no premature release. Restore capacity before
   the3000ms total admission deadline. Record DMA completion separately.
3. Allow the30000ms cache lifetime to expire. Change the controlled peer's declared
   MAC through its dedicated teaching interface, publish a new valid association,
   and capture outbound DA using that value. Do not edit the repository fixture
   to manufacture a match. A stale association must not authorize an outbound copy.
4. Feed malformed fields through the original local program/fixture channel;
   physical transmission of forged traffic is unnecessary. Preserve the local
   unauthenticated-association demonstration separately from cable observations.

`fixtures/networking/arp_physical_capture.json` is an unobserved template. Populate
operator/instrument identities, calibration and raw-file paths/hashes, and include
at least two received valid associations with distinct peer MACs followed by their
outbound IPv4 frames within the corresponding cache lifetime. The structural
checker must pass; the assessor must additionally inspect raw captures, driver
acceptance/completion traces, refusal/timeout ownership records and uncertainty
derivation. Missing timing, dropped capture records, unsupported clock assumptions
or missing changed association leave those physical claims open.

Repeat after board/PHY revision, DMA memory placement, clock, compiler or driver
changes. State which prior arguments survive. No successful finite capture proves
general-host conformance, peer authentication, bounded Internet latency, or learner
mastery. External review and independent delayed attempts require their own records.

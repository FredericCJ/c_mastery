# Dedicated F439 service observation

Use the existing T28 UART board protocol, selected F439 documentation, PuClock
generation authority and inspected board connector routing. No authoring command
here flashes a board or changes host security settings. Disconnect external
actuators and networks; the peer only increments a synthetic counter. Share ground,
use compatible 3.3 V logic and avoid competing drivers on ST-LINK or other routes.

Build original startup/clock and USART3 PD8/PD9 AF7 providers. Place PlService in
admitted SRAM, not CCM; record its linked address and the full symbol/import audit.
Establish the same serialization boundary for service calls and ISR dispatch.
Record board marking/revision, probe, compiler flags, source/image hashes, voltage,
clock source, baud divisor, timebase and analyzer sample rate. Preserve raw RX/TX,
DMA request/ack, memory-release marker, TC and peer-accept markers with a common
timebase. The marker itself has overhead; document it and instrument loading.

Send F0/F1-valued commands with all prefix cancellation cuts0..8 and the normal
path. Cut by observed byte boundary, not an uncalibrated CPU delay. Check that
incomplete prefixes produce no peer command, full frames may commit despite a
later cancel, and a fresh F0 frame recovers. Verify memory is not overwritten
before DMA acknowledgement and line reuse waits TC. If quiescence cannot be
established, retain the buffer; do not force a passing trace by recycling it.

Capture fields: experiment_id, source_sha256, image_sha256, board_id,
silicon_revision, backend, clock_generation, baud, requested_cut, observed_bytes,
cancel_request_tick, dma_quiescent_tick (null for PIO), memory_release_tick,
tc_tick, line_release_tick, peer_accept_count, next_frame_count, raw_capture_path,
raw_sha256, instrumentation_notes. Reject missing identity or reordered causal
markers. Expected peer count is zero for cuts0..7 and may be one at cut8; next
frame count is one in a recovered isolated trial. An unobserved marker is missing
evidence, not proof of ordering. IRQ latency and wire timing require actual
measurements; RAM-model/QEMU results cannot fill these fields.

# Separate physical normal-ring observations

This is a required evidence protocol, not permission to flash or attach to a
network during curriculum authoring. Use an owned dedicated development board
and an isolated synthetic peer only after a separate physical-work instruction.

Record exact MCU/silicon/board/PHY revisions, jumper/bridge route and inspected
manual/errata editions; provide a link map showing the entire12384-byte region
in DMA-visible SRAM. CPU pointer shape or a passing host range model is not a
physical mapping. Record voltage/load/output-speed and actual clock tree.

Observe continuous50MHz RMII_REF_CLK, MDC frequency and relevant RMII setup/hold
limits with a qualified instrument. Record the DWT ownership/configuration and
whether clock/debug changes occurred. Observe RCC-reset selection separately
from SWR completion and bounded missing-clock behavior. Do not start DMA after
timeout. Establish the successive-write spacing at the selected clock domain.

Capture complete descriptor/control/base snapshots before publication, after
OWN transitions, after at least32 ring wraps, during RX exhaustion, after local
TX error and across link loss plus stop/reset. Every held CPU token must remain
unchanged while the device lacks ownership. A late software completion must
fail the new epoch. Preserve actual DMA process-state observations before reuse.

Capture one known synthetic ARP frame and one synthetic IPv4 frame at both the
application span and an external peer. State exactly whether each capture
contains padding/FCS/preamble; NIC capture stripping is not evidence that the
wire omitted those bytes. Distinguish copy admission, OWN publication, local
error/success completion, wire receipt and peer-application acceptance. Measure
throughput/latency only under equal correctness contracts and state instrument
uncertainty and scheduling/load. Retain unsupported/failing physical results;
RAM-model success cannot fill any of these observations.

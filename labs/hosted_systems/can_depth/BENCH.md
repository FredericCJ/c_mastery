# Changed-condition packet: dedicated two-node evidence

This is the physical-transfer protocol for AX-PR-T24-06-T24.10.c-DELAYED, not
permission to flash hardware or change host/network settings in this authoring
session. Its author-side deliverable is a complete protocol and analyzable
capture format. The actual attempt needs two identified development nodes,
selected controller/transceiver documentation, an isolated teaching harness,
appropriate instruments and a learner at least seven days after the ordinary
assessment. None is silently inferred from a virtual frame result.

1. Fill bench_plan.json. Record board revision, controller and oscillator source,
   transceiver and supply ranges, schematic/pin mapping, firmware/source/build
   hash, cable topology and both termination components. Inspect the exact
   controller manual, board schematic and transceiver datasheet, including
   electrical limits and relevant errata. Do not substitute an unrelated board's
   CAN clock or register encoding. The current repository has no selected
   two-node/transceiver pair for this packet; these exact fields are blockers
   for board-specific register initialization and electrical acceptance numbers.
2. Draw the disconnected bench boundary and establish a safe ground/reference
   scheme under the selected equipment documentation. Verify isolation from
   deployed networks, power limits, pin continuity and unpowered termination
   resistance against the selected topology. Log actual resistor values and
   meter uncertainty. Do not infer correct high-frequency termination from
   resistance alone. Stop on inconsistent wiring or missing ratings.
3. Derive the decoded prescaler, synchronization segment, tseg1/tseg2 and SJW
   using the actual controller clock. The arithmetic checker reports nominal
   bitrate and sample point. It deliberately cannot validate controller-specific
   ranges, minus-one register encodings, oscillator tolerance or propagation
   delay. Review those against the selected manual and derive an acceptance
   interval before programming. Record readback after the authorized operator's
   setup. No fixed register values are invented in this generic packet.
4. Use two fixed synthetic IDs and monotonically increasing sequence bytes.
   Send bounded single-shot and alternating A/B cases, with zero and eight-byte
   data, standard and extended formats. Capture physical waveform, controller
   status, send result, receive result, timestamps and payload. Distinguish local
   echo from reception on the other physical node. Confirm the matching node's
   application sequence explicitly; a sender's local echo is insufficient.
5. For arbitration, arrange both nodes ready before a controlled simultaneous
   release. Use distinct data-frame IDs with the same format. Capture enough
   waveform to locate arbitration and retransmission, controller error counters
   and receive order. If submissions did not overlap, classify the run as
   non-discriminating and repeat within the bounded test plan. Do not equate
   API submission order with actual wire arbitration.
6. Exercise a *documented safe* receiver-disabled/listen-only condition only
   if supported by the selected hardware. Predict acknowledgment and counter
   behavior from the controller manual; stop at predetermined limits. No shorts,
   overvoltage, uncontrolled bus-off or externally connected traffic are part
   of the packet. Record whether diagnostic error frames correspond to actual
   controller evidence. Return both nodes to the inspected idle configuration.

Use a CSV record per message with columns
`case,node,sequence,format,id_hex,length,payload_hex,submit_clock,receive_clock,clock_unit,local_echo,controller_state,tx_error_count,rx_error_count,raw_capture_path,raw_capture_sha256`.
Record clock origin, synchronization uncertainty, instrument sample rate,
probe attenuation/range and calibration in the plan; never subtract timestamps
from unrelated clocks without a conversion/error model. Retain raw captures
and separately produce decoded tables. A checksum of a table does not prove
the underlying observation. Keep stopped/failed/non-discriminating runs.

Acceptance is a dossier, not one Boolean: software decoding matches the adapter
contract; both nodes receive the intended bounded sequence; selected device
limits and timing/error margins are supported by inspected sources; the
arbitration case has an actual discriminating waveform; diagnostics and cleanup
match the declared conditions. State the observation interval, payload count,
measurement uncertainties and residual untested fault modes. Missing manual,
instrument or physical run blocks only that evidence dimension. Reference
math self-tests and a complete written plan are not a physical pass.

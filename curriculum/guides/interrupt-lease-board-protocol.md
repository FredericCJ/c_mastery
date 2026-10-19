# Dedicated-board DMA lease observation protocol

This is the unexecuted physical protocol for PR-T29-04. It does not authorize
flashing a board or changing a host. Use a dedicated, explicitly identified
F439 or H745 development board, synthetic32-byte source/destination buffers,
the retained T27 boot/clock/MPU qualification and the T28.10 DMA2 M2M image.
Read its errata and route restrictions first. The port-only ELF produced by
`lease/board_build.py` has no vector/boot image and must not be flashed.
Import the actual learner or separately labelled assessor lease/bridge into
the qualified image. Audit the resulting full image, map and helper closure.

Record board/PCB/silicon revision, debugger, supply, clock source/frequency,
firmware ELF/source hashes, linker placements, MPU/cache settings and all
active bus masters. For H745 include both images, SRAM3 line ownership,
HSEM reservation and peer boot policy. Exclude shared dirty neighbours from
the repaired run. Do not invent GPIO routes: select otherwise unused pins
from the board schematic, record connector/pad/alternate-function/loading and
verify that probes do not conflict with SWD or peripheral signals. Missing
board/schematic/qualification blocks that capture, not the hosted/model work.

Use the T29 handoff timing protocol's acquisition uncertainty and instrument
identity rules. Sample markers for submit, cancel request, stop observation,
visibility completion and reclaim, alongside the software event log. Identify
marker placement, GPIO-write latency, timer width/wrap, analyzer sample period,
timestamp quantization and estimated loading. Store raw analyzer capture and
raw log files with SHA256; do not replace them with a screenshot alone.
Report intervals and censored/time-out observations rather than assigning
missing events zero latency. No finite maximum is a WCET proof.

Run and retain these six schedules:

1. Complete after cancel request: all32 bytes arrived and drained; COMPLETE
   wins and a later stop notification does not republish the result.
2. Stop before complete: confirm partial progress and drain, publish STOPPED,
   then inject only a metadata completion; terminal fields stay unchanged.
3. EN cleared while FIFO drain is pending: no reclaim or new submit is allowed.
   Do not infer FIFO quiescence from an interrupt's arrival or EN alone.
4. Reuse7->8 then callback7: only metadata is replayed. Validate rejection
   before any payload access. Preserve the device-stop evidence from7.
5. Software-domain restart with live DMA: first log the engine/supervisor
   identity and queued transfer, then restart only the service state. Its
   zeroed lease remains quarantined. The supervisor stops/drains and completes
   visibility before issuing a fresh epoch. Do not assert that a full reset
   preserves these domains without the board/SoC reset specification.
6. Cache visibility: on F439 show the cacheless SRAM path; on qualified H745
   compare the owned-line repair with a deliberately isolated scratch-line
   dirty-neighbour specimen. Harmless success is loss of a synthetic sentinel,
   never corruption of unrelated firmware state. Restore the repaired image
   and rerun regressions. MPS2 or the host cache model is not this observation.

The capture record template is
`fixtures/embedded/interrupt_lease_capture.json`. Keep status `not-run` until
actual acquisition. Each schedule supplies ordered event records containing
sample sequence, ticket, phase, EN, pending-write observation, visibility and
terminal count, plus the corresponding raw capture identifiers. Pending-write
fields must identify the controller observation establishing them; an invented
Boolean is not evidence. Record observer/assessor identity separately from the
author. Analyze failures at the violated edge and retain contradictory traces.

Acceptance: no CPU reuse while the old device can write, one terminal result,
old tickets rejected before current access, correct complete/possible extent,
cache-neighbour preservation under the repaired ownership contract, closed
runtime dependencies, and raw source-bound observations for every claimed
schedule. If a schedule cannot be produced, keep its physical evidence pending
and give the exact trigger/instrument/board limitation. The capture checker
only checks structural consistency; it cannot authenticate hardware or prove
that an observer told the truth. Independent learners, delayed transfer and
external technical review remain separate from a physical author run.

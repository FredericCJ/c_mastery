# Scoped T28 clock/UART implementation audit

The original PR-T28-02 retains six criterion objects, their delayed IDs, baseline
bindings and historical hours. The prior FW-04 anchor only established a bounded
TXE wait and data write. It did not establish clock generation, receive capacity,
error metadata, DMA quiescence or wire completion. The actual serial provider
already implements frame/divisor encoding; this extension imports it unchanged.
FW-04/09/11 are composed through an explicit status adapter, not renamed as the
new service. The old SDReceive suppression policy differs from retained metadata.

New source and assessor work implements all six ordinary mechanisms and six
changed-condition packages. Hardware mapping is original F439 source, host-tested
against explicit addresses and cross-audited. Serialized IRQ service is driven
through the same stepper and CR1 controls; actual NVIC/interrupt/timing evidence
belongs to the separately composed T29/board run. No source file writes host MMIO.
The dedicated capture procedure and blank format preserve external observations.

The first campaign passed365 checks before final software-ready GPIO/public-runner
additions; it is retained as attempt01_validation.json and is not final evidence.
Final source-bound campaign, guards and actual CLI export are required. Every
failure remains a failed attempt, including build or guard defects. T28.06–.12,
T29 and the wider reconciled workload remain required, not excluded by this batch.

Inspected passages and access failures are in resources.json/source_access.json.
The RM0090 TIMPRE prose/register-name discrepancy is resolved using the actual
DCKCFGR definition. No publication-wide inspection, physical measurements,
external review, independent learner or delayed performance is asserted.

Dependency correction: teach the pure serial baud/frame operation in the first
guided stage of this extension; do not make all of later T38 a prerequisite of
T28. FW-09/11 author-reference comparisons are explicitly separate from later
actual learner integration, preventing a hidden curriculum dependency cycle.

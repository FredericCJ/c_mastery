# Current T38.09 author-work audit

The actual unchanged PR-T38-08 contract and all three legacy criteria/V1 tasks were
read. The earlier SDRs485 timing/event model does not implement the full copied
byte channel over actual UART/DMA, half-range tick port, selected-device shutdown
delay or T40 decoder composition. This successor supplies those mechanisms while
retaining the original questions, hints, rubric, minimum seven-day delay and
historical hours. It does not rewrite previous accepted receipts.

Inspected actual public/private UaTx, PuDriver/PuRx, selected F439 UART port,
SDUart formatter, EDU7 frame encode/byte decode and their tests/contracts. Only the
frame portion of EDU7 is claimed here; other retained EDU7 methods remain outside
this inspection/acceptance. The resulting code calls the actual frozen producers.
Explicit target byte helpers and retained functions are audited at final link.
The new local-decode adapter is separate from the reusable byte/event channel.

Primary selected PDF passages were read from actual cached bytes, not bibliography
records: THVD1410 SLLSEY3E (May2019), THVD1400 SLLSF78B (October2021), and RM0090
Rev21 selected GPIO and memory text. Exact hashes/locators are in resources.json.
THVD1410 gives600ns with receiver enabled and11us from receiver-disabled driver
enable; THVD1400 gives650ns/10us. Receiver-enable timing is distinct, so the
selected executable port ties /RE low. THVD1400's recommended bus input limits
are narrower than THVD1410's: no family-wide envelope is inferred. Full TIA
standards, selected board schematic/current errata and physical captures remain
required access/measurement work, not claimed read or validated by these PDFs.

The original 16 eighth-load receivers plus2 loads are modeled as32 eighths.
The original third-byte overflow is represented by explicit software FULL without
consumption; true UART ORE is a different event. Unknown DMA stop retains storage
and line ownership, and event reservations preserve both possible unavailable and
terminal notices. Exact ordinary/V1 obligations are mapped individually, not
inferred from a heading. Physical non-overlap and measured timing stay separate.

Attempt01 and attempt02 stopped because the expected mutation assertion substring
was later than the actual first rejection. Both flawed early-release mechanisms
were rejected at the second data-register write; the harness now names that exact
earlier assertion. The original failed reports/logs remain preserved. No positive
implementation contract was relaxed to make the mutants fail.

The current report identifies actual commands, compiler/runtime identity, raw
logs, target map/disassembly and output hashes. Failed compiler/target attempts
are retained if any occur. Author tests and independent rational/CRC oracles
establish bounded observed properties, not external review, learner transfer or
physical behavior. T38 PR09/10, remaining T39/T40 and mixed LRUN parents remain
required work beyond this package.

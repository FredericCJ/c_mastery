# Integration families and the two capstones

Integration adds new boundary work to already completed components. Do not rewrite or charge the same implementation twice. P1–P6 each have two 8-hour milestones; each capstone has four 8-hour milestones. Additional reading, review and delayed transfer are budgeted separately in the workload guide. Complete the listed module foundations before the corresponding family, then retain reusable evidence.

The enhancement adds three complete {ref}`communication stack families <guide-communication-stacks>`: an original seven-layer exchange, Ethernet through HTTP, and a hosted HTTP server. Their small milestones belong to T40/T41/T42 and are already charged in those modules. They complement the integration work below; they do not add a second family budget or replace its existing objectives.

| Family | Existing foundations | New milestone work | Handoff |
|---|---|---|---|
| P1 bounded library | T14, T16, T18, T21, early T32 | {ref}`PR-P1-01` integration API; {ref}`PR-P1-02` failure/dependency audit | Portable containers/allocator interfaces |
| P2 minimal Linux image | T08–T11, T13, T23 partial-I/O foundations | {ref}`PR-P2-01` entry/link integration; {ref}`PR-P2-02` syscall/error audit | Explained custom-entry executable |
| P3 protocol core | T04, T12, T15, T18, T21 | {ref}`PR-P3-01` shared framing; {ref}`PR-P3-02` streaming/fault campaign | Same tested decoder for both tracks |
| P4 firmware acquisition | P3, T27–T31 plus selected T25 memory foundations | {ref}`PR-P4-01` driver/buffer assembly; {ref}`PR-P4-02` timing/fault audit | Bounded acquisition pipeline |
| P5 Linux telemetry | T23–T25, P1, P3, T32 | {ref}`PR-P5-01` ownership/backpressure; {ref}`PR-P5-02` shutdown/load audit | Bounded concurrent service |
| P6 kernel investigation | T17, T32, T34 | {ref}`PR-P6-01` scalar/numeric oracle; {ref}`PR-P6-02` vector/measurement audit | Defended optimization result |
| Linux capstone | P3, P5, T20, T22, T33/T35 applicable evidence | {ref}`PR-CAP-LINUX-01`, {ref}`PR-CAP-LINUX-02`, {ref}`PR-CAP-LINUX-03`, {ref}`PR-CAP-LINUX-04` | Reviewed service with portable core |
| Firmware capstone | P3, P4, T20, T33/T35 applicable evidence | {ref}`PR-CAP-MCU-01`, {ref}`PR-CAP-MCU-02`, {ref}`PR-CAP-MCU-03`, {ref}`PR-CAP-MCU-04` | Reviewed image using the same protocol |

## Shared educational wire contract

P3 and both capstones use this deliberately small protocol unless a dossier documents a compatible alternative:

| Field | Encoding |
|---|---|
| Sync | One byte `A5` |
| Version | One byte `01` |
| Sequence | Two unsigned bytes, little endian |
| Payload length | Two unsigned bytes, little endian, 0–32 |
| Payload | Exactly length bytes, including zero bytes |
| Check | Four little-endian CRC-32 bytes |

The CRC covers version, sequence, length and payload, excluding sync and the CRC itself. Use the reflected IEEE polynomial `EDB88320`, initial value `FFFFFFFF` and final XOR `FFFFFFFF`. The supplied fixture oracle uses Python's independently maintained `zlib.crc32`; the learner writes the C mechanism. CRC detects selected transmission corruption, not intentional tampering or authenticity.

A valid empty payload is a complete frame. An incomplete frame reports `NEED_MORE` and retains only bounded state. Length above 32 or unsupported version reports rejection before payload storage. A bad CRC emits no application message. The streaming decoder scans for sync and, after rejecting a candidate, resumes searching one byte after that candidate's starting sync. Sequence numbers wrap modulo 65536; sequence policy belongs to the application, not the framing parser. Declare duplicate/gap handling separately.

The maximum frame is 42 bytes. This bound follows from 1+1+2+2+32+4. Avoid overlaying a C structure on wire bytes: padding, alignment, endianness and object lifetime would introduce unrelated assumptions. Decode integers explicitly and validate size before computing or advancing offsets.

## Integration dossier and perturbations

Link existing component dossiers rather than duplicating their contents. Add a boundary diagram, ownership transfer table, end-to-end error paths, resource ceiling and objective-to-artifact map. For each family change one boundary: allocator policy, capacity, backend, ABI, concurrency strategy or runtime dependency. Explain which invariants survive and which evidence must be regenerated.

The final capstones share the portable protocol core and its tests. They may use different adapters and scheduling policies. A Linux mutex or file descriptor must not leak into the firmware core. A register address must not leak into the Linux core. Prove separation with builds and dependencies as well as architecture prose.

Accept both capstones only under the common noncompensable gate in {ref}`guide-assessment`. Host/model fixtures are reproducible starting cases. Physical firmware timing, bus behavior and recovery need actual board evidence and remain pending for the learner until measured.

<!-- performance-debugging guide extension -->

## Use the communication stacks as experimental subjects

A completed stack provides several narrow subjects for {ref}`T43`–{ref}`T45`: frame/record work and traffic amplification, parser/buffer storage peaks, queueing and completion latency, short I/O and descriptor cleanup, or late DMA completion and borrowed-buffer lifetime. Select the component named by the new project; do not rebuild all three stacks or charge a second capstone.

For T40, keep layer responsibilities and logical record versus link traffic explicit. For T41, distinguish CPU processing, descriptor/device completion and physical observations. For T42, retain HTTP grammar/framing, socket cursor and deadline contracts while collecting cost or diagnosing a controlled defect. A fast response cannot compensate for incorrect framing or unsafe ownership. Existing milestones retain their budgets; the additional experiment belongs to its new owning module.

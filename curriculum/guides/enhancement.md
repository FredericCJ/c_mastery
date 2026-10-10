# Algorithms, architecture and interfaces: the extension route

T36–T42 extend the preserved T01–T35 curriculum. They deepen existing implementations and connect them to machine execution, electrical interfaces and complete communication services. Complete the named prerequisite or selected prelude before its dependent milestone. A project family is a view of module work, not an additional budget.

## From mathematical mechanism to reusable C

Use this progression for each selected algorithm. The complete vector, list, heap, hash, AVL and graph work from T14/T15 remains the reusable base.

| Level | Question to answer | Artifact and failure evidence |
|---|---|---|
| Mathematical relation | Which outputs are correct for which inputs? | Input/output relation, invariant, termination measure and counterexample |
| Finite representation | Where do values, links and states live? | Index/offset/ownership diagram, checked arithmetic, capacity and unreachable-state case |
| Handwritten assembly | Which instruction/register transitions implement that relation? | Original x86-64 or Thumb source, liveness table, ABI harness, bounded accesses and disassembly |
| Typed C | Which C object and lifetime rules permit the representation? | Actual typed storage or representation-safe byte interface, failure-atomic operations and differential evidence |
| Higher abstraction | Which representation details can a client ignore? | Typed wrapper, opaque/context/callback interface, comparator/ownership/invalidation contract and replacement implementation |
| Measured execution | What determines actual work and elapsed time? | Operation counts, cycle/model assumptions, repeated measurements and disclosed uncertainty |

An abstraction preserves a promised relation while permitting a different representation. It does not remove lifetime, error or resource obligations. A generic pointer collection and a byte-copy value container have different contracts. An assembly kernel has the chosen platform ABI; its arithmetic does not grant permission for C signed overflow.

For lower-bound search, derive a half-open search interval before writing instructions. In a sorted array, all positions below `lo` contain values less than the key, and all positions at or above `hi` contain values at least the key. Every update preserves those statements and shrinks `[lo, hi)`. At `lo == hi`, return that boundary; it can equal the array length. Duplicate keys make the difference between “some matching position” and “the first position whose value is at least the key” observable. Compare the C and handwritten kernels on the same relation, then expose the typed wrapper. Compiler output is useful comparison evidence but does not replace writing the required assembly.

T36 applies this progression to search, stable sorting, heap repair, AVL certificates, collision probing, byte matching and graph frontiers. T37 explains why equivalent operation counts can produce different machine behavior: finite ALU flags, clocked state, instruction encoding, hazards, locality, memory reachability and exception boundaries.

## Three architecture levels

| Level | Authority and scope | Curriculum experiment |
|---|---|---|
| ISA and ABI | Exact Intel/Arm instruction semantics and selected call convention | Decode actual bytes, handwrite callable kernels, prove register/stack obligations |
| CPU implementation or declared model | Pipeline/cache structure and quantitative assumptions | Four-bit ALU, clocked toy datapath, hazard schedule, tag/index/replacement traces, CPI and Amdahl calculations |
| Microcontroller and board system | Selected STM32 bus masters, memory targets, clock/peripheral integration and fitted board components | CPU/DMA reachability, reset/exception artifacts, driver ownership and physical measurements |

The toy datapath, pipeline and cache are deliberately small models with explicit inputs and outputs. Their predictions are not observations of an x86 implementation or a Cortex-M4. F439 CCM is CPU-only for the selected DMA use, and F439 does not have the data-cache behavior of a Cortex-M7. Interrupt entry has an architectural saved frame and separate compiler/handler obligations; it is not an ordinary function call with another name.

The source spine includes {ref}`R-clrs-4` (fourth-edition *Introduction to Algorithms*) and {ref}`R-hanson-cii` (*C Interfaces and Implementations*), alongside the exact Knuth and Hennessy/Patterson editions assigned in {ref}`T36` and {ref}`T37`. The historical publications include Dijkstra's 1959 graph paper, Hoare's axiomatic basis and Dijkstra's refinement notes. Native contracts come from pinned Intel/Arm manuals and ABI documents. Follow exact module reading locators and their limits: a contents page, graph-origin badge or old textbook example is not an inspected current contract.

## Communication progression

T38 separates peripheral behavior from voltage conversion, medium access and application meaning. T39 separates PHY/media, MAC frame service, descriptor ownership and upper-layer protocols. Use `Communication stack families` for the complete projects.

| Interface | First-principles implementation/measurement focus | Main teaching |
|---|---|---|
| UART | Start/data/parity/stop timing, divisor error, RX faults, TXE versus final TC | `T38.02` |
| USART synchronous mode | Peripheral-generated clock, polarity/phase and last pulse; compare with asynchronous UART and SPI | `T38.03` |
| I2C | Wired-AND, pull-up feasibility, arbitration/stretching, combined transactions, terminal NACK and conditional recovery | `T38.04–05` |
| SPI | All four sampling modes, target-defined chip-select framing, receive draining and buffer/completion ownership | `T38.06` |
| RS-232 | Logic/wire voltage and polarity conversion, DTE/DCE and connector/pin contract | `T38.07` |
| RS-422 | One-driver differential multidrop, pair polarity, termination and receiver observations | `T38.08` |
| RS-485 | Differential multidriver medium, part-specific enable timing, final-stop-bit release, grant schedule and byte-channel events | `T38.09` |
| Classic CAN | Actual transmitted-bit arbitration, frame/error states, timing encodings and bxCAN mailbox/filter/FIFO ownership | `T38.10–11` |
| CAN FD | ISO DLC, FDF/BRS/ESI, CRC/stuffing changes, nominal/data timing and external-controller capabilities | `T38.12` |
| Ethernet | Physical media and link, MAC frame/FCS/padding, addressing/switching, RMII/MDIO and DMA handoff | `T39` |

The F439 has classic bxCAN. A transceiver alone cannot add CAN FD. The FD topic includes an original software format/capability project and a documented external-controller route; physical FD closure is conditional on exact additional controller/adapter setup inspection and hardware evidence. The inspected TCAN4550 has separate VSUP and VIO supplies. Its full device supply cannot be replaced by the Nucleo's 3.3 V rail.

## Reuse and assessment

Reuse the earlier container/parser/ownership library, Linux adapters, startup/linker artifacts and MMIO/ISR/DMA mechanisms. Add the new invariant certificate, assembly kernel, interface or protocol boundary specified by each milestone. Mark reused artifacts by their original project IDs; carry actual prior evidence rather than assuming a component still meets its contract after a change.

For each new objective retain a distinct evidence item: derivation, state trace, original implementation, negative case, tool result or physical observation as appropriate. A synthetic verifier establishes only its static calculation. Actual machine code, OS socket behavior and electrical/PHY results need their own execution records. After a delay, reconstruct a mechanism and change one capacity, representation, target or fault assumption.

Read {ref}`guide-roadmap` for the full dependency order, {ref}`guide-communication-stacks` for the three families, {ref}`guide-profiles` for implementation boundaries and {ref}`guide-workload` for reconciled effort.

<!-- performance-debugging guide extension -->

## Continue into cost and causal diagnosis

The assembly, architecture and communication artifacts become experimental subjects in {ref}`T43`–{ref}`T45`. Follow {ref}`guide-performance-debugging` for complexity models, hosted/bare-metal profiling, progressive debugger practice and controlled memory/lifetime experiments. Earlier implementations and evidence are reused rather than charged twice.

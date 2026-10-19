# T41.02–03: the actual Ethernet-to-transport boundary

This mandatory successor also supplies the previously missing `NW-ARP-DRIVER`,
`NW-IP-DRIVER` and `NW-TCP-DRIVER` integration. Preserve the original six
criterion IDs and their seven-day variants. Prerequisites are the implemented
T39 PHY/ring and T41 ARP, reassembly, receive stream and TCP artifacts. First
derive the interface diagram from their public headers; then implement
`learner.c` and `board_learner.c`. Assessor sources are separately packaged.

## Representation and authority

`NbNet` borrows one live `EnRing`, which borrows its `EnMemory`, bus context and
`EpPhy`. The adapter owns ARP pending bytes, two reassembly slots, one transport
control block, two copied-descriptor records and two queued control packets.
Every call is serialized and this adapter is the ring's only TX producer.
No callback may reenter. Fixed storage is explicit in `driver.h`; calculate its
actual `sizeof` and alignment for each ABI, including padding. The six 2048-byte
buffers and six 16-byte normal descriptors alone occupy 12384 bytes. The
selected actual target section is SRAM address `0x20001000`, never CCM.

ISO C17 governs arrays, lifetimes, arithmetic and calls. It does not establish
DMA visibility, MMIO effects, physical SRAM reachability or external reference
clocks. The selected compiler's Arm assembly, AAPCS, RM0090 Rev21, ES0206 and
the identified board revision provide separate premises. C23 does not turn a
volatile pointer, `const`, or a local generation number into device ownership.
The dependency closure must contain only explicitly supplied original providers
and original runtime helpers: no libc, libgcc, compiler-rt, libatomic or HAL.
Python, native sanitizers and QEMU are verification tools, not target imports.

## Construct the protocol

For each call provide pre/postconditions, retained spans, refusal behavior and
the precise point at which an owner changes. Trace this concrete sequence:
SYN seq30, local ISN1000, ARP unresolved, peer reply, SYN-ACK copy, ACK1001.
Run both ACK-before-DMA and DMA-before-ACK. They converge on established state
without collapsing these two independent events into one completion.

1. A bounded step observes actual PHY/status, reaps at most two descriptors,
   expires reassembly before receive, takes/releases at most four frames, and
   attempts at most one copied send. The selected ring already removes FCS;
   the adapter must not remove another four bytes. Rejecting upper-layer input
   must still return each accepted ring lease exactly once.
2. `nc_prepare` retains a candidate. `na_queue` copies it while resolving the
   neighbor. During this interval, pause incoming TCP and application mutation;
   changing the TCB would invalidate the retained ticket. ARP and ICMP continue.
   The finite ARP expiry aborts the logical connection. Dropping TCP during
   resolution is a declared throughput/retry limitation, not a silent queue.
3. `en_tx_copy` success means an entire internal copy exists. Commit ARP and
   TCP admission at that point, before publishing OWN. Refusal commits neither
   sequence space nor RTO time. Publication failure after OWN retains the
   descriptor record and begins stop; it cannot undo the earlier real copy.
4. A descriptor completion, including an error, releases the lower copy once.
   It does not acknowledge a TCP flight. Link loss or fatal status aborts the
   logical connection while preserving copies until positive DMA-process stop
   and return of every CPU RX lease. A timeout is failure, not quiescence.
5. Restart requires positive stop, a new ring epoch, a strictly newer transport
   epoch, and requalified PHY readiness. Local generations reject stale local
   handles; they are not carried in TCP packets and do not authenticate a peer.

Monotonic transport times are uint64 milliseconds, with 240000 reserved against
overflow. A stop uses a fresh local elapsed window: 16 observations and 100ms,
whichever expires first. Crossing 32-bit uptime cannot shorten that window.
Backward time rejects. Reassembly has two strict-overlap slots and 60000ms
absolute expiry. Copy timeout feedback before RX can tick away its saved first
header. The two-entry feedback queue is best effort; overflow increments an
explicit drop counter. No unbounded allocation hides pressure.

## Target construction and changed conditions

`NbBoard` composes the actual F439 GPIO/RCC/DWT port, MDIO port, PHY and ring.
Its cold entry requires the identified MB1137 route, stable 168MHz HCLK,
external 50MHz RMII reference, PHY address0 and exclusive peripheral ownership.
It accepts a separately established monotonic millisecond clock. That 32-bit
provider must not wrap within a session: requalification before its horizon is
required; an observed regression fails closed. PHY reset/negotiation uses a
500ms/512-step bound and each MDIO transaction 10ms/1024 polls. DMA reset gets
its own 100ms/100-step observation window. Partial hardware preparation retains
the opened target for diagnosis. Do not reset clocks to manufacture cleanup.

Required changed-condition packages reuse the actual T39 implementations:

| Criterion | Required changed input and outcome |
|---|---|
| T41.02.a | Compare the inspected UM1974 Rev8 and Rev11 board evidence. Name revision, pin, bridge and clock source individually; do not transfer wiring from the board name alone. Follow the original physical capture protocol. |
| T41.02.b | Remove link after copied payload admission. The TCB closes while DMA remains owned; only observed stop permits release/restart. Also hold reset and MDIO beyond their bounds. |
| T41.02.c | Reuse the T39 10/full and 100/full mode fixtures and separately authored bench protocol. RMII reference stays50MHz; a current-up register/model result alone cannot demonstrate it, a cable or magnetics. |
| T41.03.a | Execute `EvVisibility`: dirty byte0, neighboring byte31 on the same leased32-byte line, byte32 on another line, device write9, and stale clean7. Derive clean/invalidate/lease order without claiming M7 execution. |
| T41.03.b | Hold one RX lease through four-slot wrap; stop with it held, return it while stopping without republishing OWN, then reject a duplicate return. Reconstruct descriptor control bits after writeback. |
| T41.03.c | Execute `ev_span` with CSTF0 and CSTF1, type0800 versus length46 and0600, and oversize input. Derive the capture-to-span rule. The selected hardware ring remains CSTF0/APCS0; a changed hardware FL interpretation requires its own source/capture evidence. |

These are concrete implementations and analysis tasks, not automatic approval
to alter host security settings, flash a board or use an operational network.
Physical work uses dedicated authorized boards and a private synthetic subnet.
Use `ethernet_followthrough/ring/BOARD_CAPTURE.md` and the ARP/IP/TCP board
protocols. Retain empty observation records until actual instruments and board
identity are supplied. CPU and mapped-register execution cannot fill them.

## Submission and verification

Submit both implementations, a criterion-indexed ownership/state proof, exact
provider identities, event traces, generated objects/link map, stack bound,
failed-control explanations and changed-condition reasoning. Native invocation:
`python labs/network_followthrough/driver/run.py --providers providers.json
--build build/driver --source path/to/your/driver.c`. The JSON must explicitly
identify all 13 predecessor sources listed in `run.py`; no private fallback is
permitted. Target construction additionally requires explicit original runtime,
board and selected MMIO sources using `target_build.py --help`.

Predict the supplied assertions before executing them. Explain each of the
seven semantic defect controls: false copy admission, DMA-as-ACK, premature
release, missing RX return, double FCS subtraction, late expiry reporting, and
TCB mutation during retained ARP. Compilation failure is not their expected
semantic result. The incomplete learner source must compile and fail its first
unimplemented contract. CCM placement and missing providers are separately
classified negative build/configuration fixtures.

Per-step bounded work is four maximum-size frame parses/copies plus two
completion observations, bounded PHY accesses and one output copy; packet work
is linear in admitted bytes. Stop loops are caller-driven and finite. Measure
actual code/static storage and audit emitted stack frames, callbacks and
implicit helpers. A successful link alone does not establish the permitted
closure, an observed maximum is not WCET, and finite tests are not mastery.

Rubric, applied to every mapped criterion: level0 for a wrong ownership edge,
length, source boundary or fabricated observation; level1 for matching traces
without an invariant; level2 for a correct implementation, explicit failure
proof and a new counterexample; level3 only after independent delayed transfer
and explanation of the changed premises. Author runs cannot award level3.

Hints in order: (1) draw four events—prepare, copy, DMA return, ACK; (2) identify
which event starts a timer and which actually returns storage; (3) keep a
descriptor-to-transport ticket ledger and inspect the pending ARP lifetime.
Then solve the delayed task without these hints or assessor access.

Security scope: only the original packet arrays/register models or dedicated
teaching firmware, synthetic192.0.2.0/24 traffic. A hostile local peer may forge
malformed, replayed or correctly checksummed packets. Observable success is a
local invariant violation, stale delivery or retained-resource error. A checksum
does not authenticate ARP/TCP. Repair exact ownership/validation edges and rerun
regressions; do not call a crash arbitrary execution or a replay test universal
anti-replay protection. Physical Ethernet and independent review remain pending.

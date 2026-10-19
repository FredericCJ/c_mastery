# RS485 byte channel, current additive contract

This required successor implements the author-work portion of PR-T38-08 and its
unchanged T38.09.a/b/c and seven-day V1 tasks. The parent project, legacy shared
assessment contract and actual UART producer contracts remain applicable. The
historical 10 hours are descriptive; this additional work has no time ceiling.

## Problem and prerequisites

Implement `channel.h` in `learner.c` and `port_learner.c`. Supply your earlier
`UaTx` (`labs/serial_followthrough/uart/uart_depth.h`), `PuDriver`/`PuRx`
(`labs/peripheral_depth/uart/uart.h`), `SDUart` formatter, and the exact
`EduFramer`/`EduFrame` interface in `labs/communications/include/com_edu7.h`.
Use fixed caller storage; implement the channel and selected port yourself.
The T40 changed task must bind the learner's actual frame implementation.
The separate author campaign uses the retained assessor producers and establishes
composition feasibility, not a successful learner attempt.

For the first task derive sixteen eighth-unit receivers plus sixteen eighths
of modeled bias, and independently enumerate reference, load, termination and
exclusive-drive premises. Unit-load arithmetic alone says nothing about those
premises. For the second task send `[17,34]`, overwrite the caller array after
admission, and present the precise TXE/TC schedule in `tests.c`. For the third,
admit bytes17 and34 at capacity2, attempt51, then inject byte33 with FE/ORE;
cancel an actual DMA transfer while EN still remains observed active. Preserve
the trace of every accepted/consumed count, event, token and ownership state.

## Environment and limits

ISO C17; profile E fixed storage, no allocator or full runtime. GCC/Clang host
harnesses may use stdio/sanitizers. Cortex-M4 soft-float builds explicitly link
the submitted byte helpers and startup, then audit unresolved symbols, map,
instructions and stack usage. `freestanding/string.h` only declares the byte
functions referenced by the imported translation unit; it supplies no library.
Dead-code removal and the final image, not compiler flags alone, determine the
retained dependencies. The supplied target builder receives every prerequisite
source explicitly; no assessor solution is discovered or shipped.

The original MPS2 guest executes Thumb with RAM register effects. Its simulated
UART flags are explicit inputs, not observations of F439 silicon or wire time.
The non-model port is separately compiled and inspected. Actual two-node board
captures required by the parent are specified in BOARD_CAPTURE.md and remain
unperformed until real evidence is supplied. No physical WCET or oscillator
accuracy follows from the modeled timestamps.

## Representation and ownership

One initialized, zero-origin `R5Channel` has one foreground owner; no ISR or other
thread may call or modify it concurrently. It contains16 copied TX bytes,
1..16 pending RX byte capacity, and3..32 event capacity. `UaTx`, `PuRx` and
`PuDriver` retain their exact invariants. All inputs and outputs denote live,
appropriately aligned, disjoint objects unless explicitly copied here; do not
forge control state. Source ranges and output objects cannot overlap the channel.

The `PuIo` and drive callbacks are trusted bounded operations: terminate, do not
allocate, reenter, mutate channel controls, retain borrowed arguments or dispatch
another user of the same UART/GPIO/DMA. This contract has no asynchronous callback
completion. Fresh `r5_open` requires an entirely zero-initialized channel and
quiescent peripheral. A failed open may have touched peripheral configuration;
retry only through the producer's documented preparation, not by erasing a live
lease. There is no close/reset operation that can discharge unknown DMA ownership.

Successful send copies1..16 bytes before DE assertion and returns that count.
Caller data may then be changed/released. BUSY/FULL/STALE/INVALID preserve the
accepted output and do not replace an existing transfer. External positive tokens
increase without wrap for an owner lifetime; internal producer tokens also require
headroom. Retire the owner at exhaustion only after quiescence. DMA uses up to two
8-byte producer chunks. A token does not authorize a peer, prove message delivery,
or make an arbitrary pointer valid.

## Timing and failure semantics

`r5_delays` rounds each nonnegative duration upward to the declared1..1,000,000ns
tick. The selected THVD1410 SLLSEY3E maximum driver enables are600ns with /RE low
and11us with receiver disabled; THVD1400 SLLSF78B uses650ns and10us. Both selected
tables give200ns maximum driver disable under their specified test conditions.
These are different conditions, not interchangeable typical numbers. Driver
enable does not bound receiver wake; an adapter toggling /RE must additionally
honor that device's receiver-enable bound before treating incoming samples as
valid. The provided F439 adapter ties /RE low and does not toggle it.

Ticks are unsigned32 modular values. Every submitted budget, elapsed call gap,
enable deadline and disable-plus-guard interval is strictly below2^31 ticks.
Each due-time observation must occur before the half-range ambiguity window;
an arbitrary multi-wrap pause cannot be reconstructed. A backwards/half-range
input is rejected, while an intentional wrap within those bounds is supported.
The caller provides a stable tick scale and clock-generation discipline. Ordinary
and wrapping variants use the same correctness contract, not saturating addition.

IDLE -> ENABLING -> ACTIVE -> TURNAROUND -> IDLE. TXE means space in the data
register, not final stop completion. DE stays asserted until both memory and
line ownership have ended in the actual lower producer. A cancellation during
ENABLING prevents all UART writes. Cancellation/deadline while hardware stop or
TC is unknown emits DEVICE_UNAVAILABLE once, retains channel storage and DE,
and cannot emit TX_DONE. On later confirmed quiescence, deassert DE, wait rounded
disable plus guard and emit ABORTED. A cancelled first DMA chunk cannot start
the second. The caller must keep driving the stop protocol and retain all state;
software timeouts do not force a peripheral to stop.

Reserve two control-event slots at send admission for possible UNAVAILABLE and
terminal events. RX and explicit LOSS cannot consume this reservation. Every
terminal appears once with its matching token. Completion releases local storage,
not proof of receipt by a remote application.

## RX, backpressure and composition

`r5_receive` consumes one supplied snapshot only on OK. A BYTE with its raw error
mask and separate FRAMING/OVERRUN events are distinct; admission is atomic for all
events that snapshot requires. FULL preserves the caller observation and consumed
output. It does not promise hardware can wait: the caller must preserve/retry a
software observation or publish LOSS if acquisition discarded bytes. The original
fixture's third-byte overflow indication is represented by this explicit FULL
return; it is not falsely labeled a UART overrun event. If actual hardware sets
ORE, that separate error is preserved. `r5_receive_poll` conservatively reserves
three slots before reading DR and delegates to actual `pu_rx_poll`/`pu_take`.

While a local transaction owns the bus, supplied observations are consumed as
local echo and counted, not delivered to the peer parser; this includes local
echo error snapshots. Do not infer absence of contention from that policy.
The separately required peer coordinator must prevent overlap. Counters do not
wrap silently. Take copies one event, reports consumed1, or preserves its output
and reports consumed0 when empty. LOSS is explicit and cannot overwrite events.

`R5Link` is a distinct adapter, not framing inside the byte channel. It passes
clean BYTE events to the actual EDU7 incremental decoder, invalidates partial
frames on error/LOSS, and publishes a copied frame only on `R5_FRAME` after local
destination and trusted current grant checks. Other results must not be treated
as frame publication. The grant is scheduler policy, not cryptographic authority.
Loss at every prefix boundary is followed by a complete fresh frame; recovery
must not publish a corrupted prefix or retract a frame already published before
the loss notice. Stale channel completions cannot act on a new transaction.

## Selected F439 port

Dedicated USART3 PD8 TX/PD9 RX AF7 and dedicated PD10 DE, /RE tied low, shared
reference, declared transceiver supply and exclusive quiescent GPIOD ownership.
The adapter imports actual `pu_f439_pins`/`pu_f439_io`; it first sets DE's output
latch inactive, then selects push-pull/low-speed/no-pull/output mode and uses BSRR
plus DSB for direction. Register visibility is distinct from an analog enable
delay. The DMA wrapper admits its entire16-byte stable TX region only within
F439 AHB SRAM0x20000000..0x20030000. CCM is rejected by the linker control;
the MPS2 normal image exercises actual numerical SRAM-address admission. The
abstract callback entry requires equivalent DMA reachability from its owner.
Moving to another MCU, pin mux, cache, UART or DMA requires a new adapter audit.

## Deliverables, verification and rubric

Submit D1 unit-load/timing derivation and explicit missing assumptions; D2 channel,
port, event/resource and dependency ledgers; D3 replay and first-divergence diagnosis
for all supplied cases plus a new boundary; D4 original unchanged delayed variant
after at least seven days; D5 physical capture packet with measurement uncertainty.
`sizeof` values, bounded loops, copies and function stack reports are measured for
each actual build. Admission copies O(n), n<=16; receive/take/step are bounded by
fixed capacity and terminating callbacks. No fairness/scheduling bound is promised.

Require the original per-objective rubric plus: .a derives four unit loads and
recomputes enabled/shutdown timing without changing the device's test conditions;
.b establishes half-range assumptions, rejection before state change, TC rather
than TXE, and the copied-buffer lifetime; .c explains event reservations, retained
DMA lease, stale token behavior and actual frame-prefix invalidation. Level1 is
only reproducing numbers, level2 needs the mechanism and a discriminating repair,
level3 requires an independently attempted changed-condition solution with limits.
Author-run tests or finite mutants cannot confer independent mastery.

Use config/timing/events/cancellation/link/port selectors, GCC/Clang O0/O3 and
sanitizers, then explicit target builds and nm/disassembly/stack audits. Reject
the unsolved scaffolds for semantic assertions, not compile errors. The separate
assessor package provides independent rational/CRC oracles and deliberately flawed
variants; it is withheld from learner exports. Compare full state and output
preservation, not only a final success message. No physical receipt means the
physical criterion remains open even if every software test passes.

## Hints and variants

First draw separate owners for caller input, copied storage, DMA and line. Next
identify which transition consumes each reserved control slot. Finally inspect
the exact enable column selected by /RE and derive a modular comparison from
its preconditions. Retain the unaided attempt before using later hints.

The unchanged original variants are AX-PR-T38-08-T38.09.a-V1 (shutdown mode),
AX-PR-T38-08-T38.09.b-V1 (wrapping32-bit timer), and
AX-PR-T38-08-T38.09.c-V1 (actual learner T40 frame adapter with event loss and
stale completion). All retain the seven-day delay. Additional transfer: change
tick resolution, move buffers into CCM, or add an ISR producer with an explicitly
verified synchronization protocol; the present single-owner implementation does
not claim that concurrent extension.

## Security and validation status

Only original local models, emulators and dedicated teaching boards/synthetic
bytes. No deployed bus, operational vehicle, third-party service or private data.
This authoring task performs no physical flashing. Two drive intervals can
demonstrate a model violation, not an attack on external equipment. Inspect the
source-bound `legacy_serial_rs485/validation.json` for exact actual commands and
limits. External review, independent delayed learner performance and physical
waveforms remain separate, unperformed gates.

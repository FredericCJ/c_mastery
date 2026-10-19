# Clock-aware UART service, revision 1

This is the required implementation extension for PR-T28-02, objectives T28.03.a–c
and T28.05.a–c. Retain each original criterion and delayed assessment. Read the
shared legacy assessment contract and the original serial and firmware contracts.
Prerequisites are T27 startup/image identity, T28.01/.02/.04 register effects,
unsigned arithmetic and finite-state invariants. The first guided artifact is
the pure `sd_uart` baud/frame operation under its existing shared contract;
construct it here before the service. The rest of T38 is not a prerequisite.
FW-04/09/11 are contract comparisons and later learner-component integration
points; author composition explicitly uses their assessor references. Exported `uart.c` and `target.c` are intentionally
unfinished; reference implementations and transfer answers remain assessor-only.

Use ISO C17, exact uint8/uint32 types and eight-bit bytes, profile E. The portable
service has fixed storage and no allocator, libc, OS, HAL or atomic library. Host
Python, sanitizers and the PowerPC Linux syscall verifier are verification tools.
The target is the STM32F439 Cortex-M4 with a separately identified board assembly.
Original byte-loop compiler helpers from the execution component may be explicitly
linked and audited; no compiler runtime archive is permitted. A cross-link cannot
establish peripheral behavior. No C23 feature is used, and no draft is relabeled
as the published C17/C23 standard.

## Mechanism and guided construction

A configured UART depends on three different authorities: the clock manager's
frequency record, the peripheral register state, and the lifetime of an active
request. A correct BRR does not make an old frequency record current. A DMA
completion does not empty the UART shift register. Represent those authorities
separately before implementing the API.

Start by deriving one clock tree, framing one byte and drawing a request state
diagram. Implement the pure clock and receive functions, then the serialized
register adapter, then asynchronous ownership. Use the same service tests for
each backend only where the input and completion meanings match. Finally bind
the original F439 register map, audit actual objects, and perform the separate
capture protocol when the required dedicated equipment and authorization exist.

### Clock record and generation

`PuClock` supplies SYSCLK, generation, AHB divisor, separate APB divisors and
TIMPRE. Generation is positive and must never wrap into a live old generation.
AHB accepts 1,2,4,8,16,64,128,256,512; APB accepts 1,2,4,8,16. All rates must be
integral Hz in this teaching API. SYSCLK is at most 180 MHz; APB1 at most45 MHz,
APB2 at most90 MHz. These numerical limits do not establish voltage, flash
latency, over-drive, PLL input or oscillator tolerance requirements. The clock
manager must establish those separately before publishing the record.

Derive HCLK=SYSCLK/AHB, PCLKx=HCLK/APBx. With TIMPRE0 the timer clock equals
PCLK when APB divisor1, otherwise twice PCLK. With TIMPRE1 and divisor1/2/4 it
equals HCLK; with divisor8/16 it equals four times PCLK. Use RM0090 Rev21
Figure16 and pp154–155,168–169,210; its introductory mention of RCC_CFGR for
TIMPRE conflicts with the actual RCC_DCKCFGR register field at p210. Select the
documented field, and record the discrepancy. Invalid records preserve output.
All other operations are O(1), except explicitly bounded polls.

`pu_open` rejects outstanding memory or line ownership. It validates all pure
inputs before effects, marks the service unavailable, then polls the selected
source's readiness. Readiness on the final allowed poll succeeds. Never-ready
leaves the service unavailable without touching peripheral registers. It enables
the gate, reads the corresponding RCC enable register back, asserts/releases
reset, then configures UE, 8N1, BRR and TE/RE. Source readiness is not a fictitious
USART ready bit. This selected sequence assumes exclusive RCC/peripheral access,
no active RX character and a stable clock snapshot throughout configuration.
The readback implements the workaround in [ES0206 Rev24 §2.2.7 p11](https://www.st.com/resource/en/errata_sheet/es0206-stm32f427437-and-stm32f429439-device-errata-stmicroelectronics.pdf).
It is a device rule, not a portable consequence of `volatile`.

The F439 adapter checks the currently selected source via SWS and its ready bit;
it does not infer source frequency, force a PLL transition or verify a supplied
record against every register. The clock manager owns that consistency. Configure
using generation g, publish a new frequency with g+1, and attempt submit with
the new generation against the old configuration: it must return STALE.
During a live request the manager must stop admission, request cancellation,
retain both ownership obligations as applicable, drain, change frequency,
reconfigure and renegotiate communication. If the clock changes unexpectedly,
delivery is uncertain; a token alone cannot prevent corrupted wire bits.

### Baud, framing and reception

Reuse the actual `sd_uart` contract for nearest divisor, ties upward. It supplies
the BRR and 8N1 LSB-first frame. For the supplied16 MHz/115200 case derive the
rational realized rate16,000,000/139 and signed ppm error using an independent
wide rational calculation. Compare OVER8 encodings and their minimum and maximum
divisors; do not assume BRR is always the integer divisor. RM0090 Rev21
pp973–977,981,1010–1018 distinguish framing, baud updates, status clearing and
TXE/TC. BRR must not change during communication.

`PuRx` is a capacity-two queue. Initialized zero, it retains each available byte
and its raw PE/FE/NF/ORE mask; the caller decides whether an errored byte is usable.
An absent RXNE returns BUSY and never manufactures zero. A full queue drops the
incoming byte, saturates `dropped` at UINT32_MAX and returns FULL. Each received
errored byte increments `errors`, even if dropped; exhausted error accounting
rejects publication with FULL. This is an explicit accounting failure, not a
claim that hardware can undo a consumed byte. `pu_take` returns one oldest item;
empty preserves output. The conservative original `sd_uart_receive` instead
suppresses errored bytes: explain this deliberate application-policy difference.

`pu_rx_poll` reads SR and only when RXNE is present reads DR once. It preserves
the prior status in the queue and lets the documented SR→DR sequence clear
errors. In particular ORE does not mean that the older RDR byte was lost. PE
without RXNE is not cleared prematurely. A poll budget bounds observations,
not microseconds; callbacks must terminate. Queue owners must remain live and
serialized. No atomicity or interrupt exclusion is supplied by ISO C.

For the changed backpressure policy set `backpressure=1`, `reserve` to an
explicitly justified maximum number of in-flight bytes0..2, and drive the peer's
ready signal only when free slots exceed that reserve. With reserve1, stop
requesting after the first queued byte. One in-flight byte fits; a second still
overflows under the same explicit drop/count policy. F439 automatic RTS reports
its hardware receive buffer, not this software queue's capacity. Software-driven
flow control or a protocol credit needs its own wiring/peer contract. Never claim
losslessness for an unbounded peer or an unmeasured stop response. The supplied
F439 target contract includes `pu_f439_rts_init` and `pu_f439_rts`: exclusive PD12
GPIO output, active low, initial high before output mode, then one BSRR write
from the queue readiness decision. Configure the dedicated peer's compatible
input and verify the actual board route. This deliberately does not enable
automatic RTSE or assume the ST-LINK VCP provides that extra signal.

### One API, three progress adapters

`PuDriver` begins zero. `pu_submit` accepts1..8 immutable bytes, a strictly newer
positive token, matching clock generation, backend POLL/IRQ/DMA, a 32-bit
monotonic timestamp and positive budget below2^31 ticks. Admission transfers
buffer and line-use rights to the service. Every call has exclusive access to
the driver and its registers; all pointers and controls are disjoint, valid,
appropriately aligned objects. Pending callbacks and DMA accesses must drain
before destruction or token reuse; exhaustion requires retirement, not wrap.

For polling/IRQ, one `pu_step` writes at most one byte after TXE. The final source
read releases the buffer; the request still owns the line until a later SR read
observes TC. The pre-write snapshot's TC cannot acknowledge that write. IRQ
submission enables TXEIE; the last write changes to TCIE. An ISR calls the same
bounded step under the platform's exclusion contract. NVIC vector/priority and
tick delivery are prerequisites of the T29 adapter; these source audits do not
claim an executed interrupt latency. A clock-independent deadline event must
also call step, even if the peripheral never interrupts.

DMA retains the caller buffer until the stream's EN reads zero. Normal completion
also requires TCIF and zero remaining count. Errors enter cancellation and
preserve ownership until that observation. Cancellation writes EN0; the write
is not the acknowledgment. After EN0, disable the peripheral DMA request, then
observe USART TC separately. Cancellation after the final memory read can release
memory while leaving line turnaround forbidden. The F439 normal direct-mode
adapter selects DMA1 stream3/channel4, byte source/destination, incremented
memory, fixed USART3_DR, no circular/double buffering. `pu_f439_dma_submit`
admits only SRAM0x20000000..0x20030000 spans and rejects CCM/overflow. This
wrapper does not make a stale or non-live C pointer valid. See RM0090 Rev21
Table43 p310, §10.3.17 pp325–326, register fields pp332–335 and §30.3.13 p1004.

Each service event returns BUSY while ownership remains. A timeout initiates
cancellation, records TIMEOUT and keeps the request active until safe release;
timeout never means that the caller may free memory. `pu_poll` also has a finite
operation budget, independent of elapsed time. `pu_reset_notice` invalidates
configuration, records IO and requests DMA stop; resetting USART does not reset
the DMA controller. Terminal events and old tokens return STALE. Deadline
arithmetic requires each observed increment below2^31; readiness at the exact
deadline wins. Admission after a terminal event requires a newer token.

`pu_deadline_fit` evaluates the necessary wire-time bound for1..8 8N1 bytes,
input clock<=45 MHz, divisor1..65535 and remaining time<=1 second. Derive
`bytes*10*divisor*10^6 <= clock*remaining_us`; bounded64-bit multiplication
avoids rounding a needed duration down. This condition is not sufficient under
CTS pauses, scheduling delay, preamble, reconfiguration or peer renegotiation.
Preserve the original absolute application deadline when frequency changes.

## Deliverables, evidence and grading

Submit the implementation and a criterion-indexed dossier: assumptions and
predictions; register/state traces; ownership invariants; one minimized defect
per mechanism; actual compiler/link/helper closure; independent oracle inputs
and outputs; and cost accounting. Separate bytes copied, register operations,
CPU polling, ISR work, DMA setup, buffer retention and wire time. Fixed storage
is O(1); transmission uses O(n) data movements for all backends. DMA reduces CPU
per-byte work but still uses the bus and does not bound interrupt scheduling.

Run `python labs/peripheral_depth/uart/run.py --build <new-directory>` after
implementing both this component and the prerequisite serial source. Explicit
`--source`/`--serial-source` paths can select external implementations. A compiling
stub must fail. Never edit public checks to accept a solution. Use additional
tests for all invalid, empty, boundary, error and changed conditions described
here. Inspect the actual target object, linked imports, map and stack-usage
files; preprocessor headers and successful linking alone do not prove closure.

Level0 violates a contract or claims unobserved target behavior. Level1 passes
ordinary examples but cannot explain its invariants or distinguish TXE, DMA
completion and TC. Level2 supplies each original criterion's implementation,
causal explanation and discriminating boundary/control evidence. Level3 solves
its separately delivered changed condition after the required delay and explains
which old evidence no longer applies. Finite tests support bounded claims, not
absolute mastery. Independent review, learner/delayed attempts and physical
captures remain separate from author execution.

Hints, in increasing detail: (1) Write the ownership set beside every transition.
(2) Track memory and line rights independently, and keep the status snapshot
from before any side effect. (3) Treat clock-generation mismatch, poll exhaustion
and cancellation acknowledgment as different events; use a rational oracle for
rate and deadline calculations.

All injected defects target original local programs and synthetic bytes. Harmless
success means a wrong queue item, stale configuration accepted or premature
ownership release in the teaching model. Do not infer code execution from a
bad return or sanitizer finding. No external service, live vehicle, production
data, host security change or board flashing is authorized by this exercise
authoring run. Follow BOARD_CAPTURE.md for the separately authorized dedicated
equipment observation.

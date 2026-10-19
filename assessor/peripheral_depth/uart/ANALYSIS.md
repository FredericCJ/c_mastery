# PR-T28-02 assessor packet

Use the unchanged six ordinary criteria and their six DELAYED IDs. Public checks
are examples, not the complete rubric. Select transfer cases separately and
record the actual seven-day interval; an author execution is not a learner attempt.

## 03.a

Set source readiness to the second observation with budget2: configuration is
legal, and the first peripheral access follows RCC enable/readback and reset
release. Never-ready consumes exactly the budget, publishes no ready service and
writes no byte. Deleting readback is rejected by the side-effect model. There is
no F439 USART ready flag: source oscillator readiness and clock-enable latency
are separate. Alternative implementations can use the documented DSB workaround
under the same device/ISA contract. A generic millisecond delay lacks a derived
necessity and needlessly changes the latency contract.

Delayed: reset USART while DMA owns a three-byte source. `pu_reset_notice` clears
ready, records IO and requests DMA disable. It cannot release memory before EN0.
After draining, initialize with a newer clock generation. A reset flag that simply
clears the C driver struct is wrong: the independent DMA master can still read it.
The source record must identify which reset domain actually covers the DMA.

## 03.b

168 MHz, AHB1/APB1=4/APB2=2 gives42/84 MHz bus clocks. TIMPRE0 gives84/168 MHz
timer clocks; TIMPRE1 gives168/168 MHz. Change only APB1 to8: CPU remains168 MHz,
UART input21 MHz, timer42 MHz at TIMPRE0 or84 MHz at TIMPRE1. Reusing BRR365 halves
the baud; the timer multiplier does not apply to USART. Reject missing APB1 and
frequency limits rather than substituting the CPU rate. Independent rational
clock enumeration includes non-integral Hz records that this API rejects, rather
than silently rounding them and claiming exact hardware clocks.

## 03.c

Configure generation1; submit with current generation2 must be STALE before byte
publication. A clock-manager transaction must first prohibit new work and drain
the current DMA/line ownership, then change hardware, obtain stable rates and
publish generation2 plus a new BRR. Holding generation constant while physically
changing frequency is a violated caller contract, not a safety feature C enforces.
UINT32_MAX tokens and generations must be retired with old events drained.

Delayed application deadline: at16 MHz/divisor139, three8N1 bytes require
260.625 us.260 us cannot fit;261 us passes the necessary wire-only inequality.
If a reconfiguration uses80 us of a300 us remaining budget, the transmission
cannot retain its old delivery promise. Renegotiate or cancel; do not reset the
application deadline. CTS can stretch completion indefinitely and can make even
the necessary261 us check insufficient. Use a timebase whose units remain known
when SYSCLK changes; a bare loop count or old SysTick reload is not one.

## 05.a

The divisor139 gives115107.913669... baud and signed error
`(16,000,000-115200*139)/(115200*139)*10^6 = -799.360511... ppm`.
OVER8=0 BRR=139=0x008B; OVER8=1 has mantissa17,fraction3,BRR=0x0113=275.
The eightfold encoding's bit3 is zero. At42 MHz/115200, nearest divisor365 gives
0x016D or0x02D5. Rounding quotient/remainder without adding clock+baud/2 avoids
overflow. The serial provider independently emits start0,data LSB first,stop1.

Delayed: test representable edge divisors8/16 and32767/65535 plus the first
unrepresentable values; ties upward and small clocks that round below minimum.
Receiver masks use F439 hardware positions; they differ from the synthetic
SDReceive enum. `pu_rx_poll` keeps PE/FE/NF/ORE beside the observed byte, while the
original SD policy suppresses it. An ORE observation says a subsequent shift
register character was lost, not that the saved RDR byte is necessarily corrupt.
Application acceptance of that byte requires its protocol context. Actual
oscillator/peer errors are additional to divider quantization.

## 05.b

Input0,65,255 gives two queued bytes0,65 and one dropped byte. Empty must preserve
the caller's prior value. Inject RXNE|ORE on65 and check metadata8 plus one error
count. PE without RXNE must not force an early DR read; RXNE with FE/NF/PE is
consumed once after saving SR. Never-ready TX/RX must terminate under the supplied
poll bound. Timeout after a TX write retains line ownership.

Delayed backpressure: capacity2,reserve1, one accepted byte deasserts readiness;
one legitimate in-flight byte fills the queue, a second yields a counted loss.
For reserve2 this capacity cannot advertise readiness at all: increase capacity
or negotiate a stricter peer bound. F439 automatic RTSE cannot observe a software
queue after DR is consumed. A valid alternative uses a software-controlled
ready GPIO, protocol credits, or sufficient ring capacity plus a proven delay
bound. Merely enabling RTSE and asserting zero loss fails the transfer rubric.

## 05.c

POLL and IRQ reuse the serialized stepper. IRQ's final write disables TXEIE and
enables TCIE; a ready level before enabling must still be serviced. One step
has at most one source-byte read/write. Caller synchronization, vector routing,
interrupt priorities and independent deadlines remain explicit platform duties.
DMA's EN0 acknowledges cessation of accesses, not line completion; normal
success additionally checks TCIF and NDTR0. When a DMA error arrives, stop the
stream and retain its buffer until disabled. A TC snapshot from before the final
DR write is deliberately rejected, avoiding an idle-TC false success.

Delayed: final DMA memory read, EN0/TCIF/NDTR0, UART TC0, then cancel. Memory can
be reused; line turnaround and reconfiguration cannot. After TC1 the terminal
result remains CANCELLED, not successful delivery. If stop times out, retain the
buffer until an independently verified reset/quiescence procedure; do not free
it merely because a deadline elapsed. Alternatives with internal copies may
release caller storage at submit but must report their different storage/copy
cost and retain the internal DMA buffer. They are not equivalent zero-copy APIs.

## Controls and scope

Seven compiled core mutants alter TIMPRE, omit enable readback, use the wrong
OVER8 encoding, erase error metadata, ignore clock generations, release DMA
memory on cancel, or substitute TXE for TC. Two separately compiled target
mutants select the wrong DMA channel or pin AF. Learner stubs compile and fail
semantic checks. A translation error is not a killed semantic mutant.

The independent oracle uses Fraction clocks, a deque receive model and an
ownership-set DMA ledger. It checks122,614 rows per native profile, including
7,776 five-event histories/38,880 transitions. These are synthetic legal and
adverse observations, not exhaustive physical schedules. The target memory
mapping harness checks actual adapter address/bit choices but does not emulate
bus delays, side effects or electrical behavior; the separate Device model
checks sequencing. PPC64 execution changes endian and pointer width for the
portable API. M4 audit retains every public function and named prerequisite
provider, records original helper calls and links without external runtimes.

Score every objective separately. Do not assign 03.c from baud arithmetic or
05.c from a receive-only fixture. Require a register trace, independent
expectation, relevant rejected control and the causal argument above. The board
capture packet is a complete procedure; blank/unobserved fields are remaining
external evidence. It is never acceptable to fill them with host-model outputs.

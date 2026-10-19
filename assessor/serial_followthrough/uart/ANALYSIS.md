# PR-T38-01 assessor analysis

Use the six original criterion identifiers; do not award their delayed evidence
from the author's suite. Inspect unaided reasoning before disclosing this file.

## T38.01.a and V1

C-object ownership is software's obligation. CPU writes traverse the selected
bus to USART DR; USART constructs asynchronous characters. The line transceiver
converts logic levels and polarity. Cable/peer add electrical and reception
conditions. A numeric divider proves none of those conditions. The MAX3232
driver table's opposite DIN/DOUT levels supports inversion, not a framing engine.

Under SPI-to-MCP2515, SPI transfers command/register bytes; the CAN controller
owns arbitration/framing and transmit state. Local buffer priority and bus
arbitration priority are distinct. Copied SPI bytes may be reusable after the
SPI/DMA transaction is quiescent while CAN is still pending. A transceiver's
electrical action and application-level receipt are additional boundaries.
Reject the explanations 'SPI performed CAN arbitration' and 'an interrupt means
the remote application committed the message'. Require a counterexample with
arbitration lost after a successful local SPI transfer.

## T38.01.b and V1

Correct AF7 does not change CCM's bus reachability. The mapped test compiles the
actual retained address/bit adapter and rejects CCM and an aperture crossing
before enabling a stream. SRAM admission plus a matching linker map support
placement; they are not observations of an operating DMA engine.

The original one-line model runs all 255 nonzero byte values. Omitting clean
leaves TX RAM stale; omitting invalidate leaves RX cache stale. Cleaning a dirty
shared line after DMA writes erases the new DMA byte; invalidating the same line
loses an adjacent CPU update. Whole-line isolation or an equally justified
ownership protocol is essential. Distinguish ordering barriers from visibility
maintenance and from bus reachability. This is an analysis/model successor,
not a newly validated M7 UART driver. Any target port must inspect its actual
memory map, MPU/cache settings, maintenance implementation and errata.

## T38.01.c and V1

The host bit trace checks a declared discrete model. A linked Thumb image checks
selected toolchain/layout/helper closure. RAM readback checks address/bit code
against that model. Only a suitably calibrated physical observation measures
that board's waveform, within its uncertainty. One decoded byte at +3% does not
meet the declared ±1% accuracy gate and does not establish receiver margin over
all patterns or conditions. Do not silently choose a universal 3% tolerance.

## T38.02.a and V1

8N1 A5 is start0, data10100101 in temporal LSB-first order, stop1: 0101001011.
Payload01 makes reversed ordering visible. Seven payload bits with parity use
M0; eight with parity use M1. The format adapter rejects unsupported 7N framing.
The software decoder deliberately rejects corruption of either configured stop
bit; that strict model is not an assertion that F439 samples both. Pure decode
failure must preserve output. Require proof of each bit position, not a table
memorized from the public test.

## T38.02.b and V1

N is nearest clock/baud with ties upward; actual baud is clock/N. OVER8 packs
floor(N/8) into BRR's high field and N mod8 into its low three bits. At42 MHz,
115200 produces N365 and BRR0x16D/0x2D5; actual baud8400000/73. Its signed error
is −250000/219 ppm. N14 at3 Mbaud is below the OVER16 minimum but valid for
OVER8, whose BRR is0x16. At21 MHz N182; at16 MHz N139. Fraction-based oracle
prints their exact errors. Reusing a configured generation after an APB change
must fail before any data register write. A source-clock record still requires
truthful input; no algorithm infers oscillator frequency from a forged record.

## T38.02.c and V1

TXE admits the next write; TC admits final line release only when it is observed
after the last write. The aggregate reserves all internal tokens and keeps the
whole span until final quiescence. It cannot extend a request's deadline by
starting a second chunk. RXNE+ORE retains the older data with a loss indication;
application discard is a policy. Full software storage cannot stop an
uncooperative peer. The adapter conservatively refuses DR reads after its error
counter saturates rather than silently dropping the associated event.

Mapped DMA tests require EN acknowledgment before buffer release; even then
TC0 retains line ownership. Cancellation likewise waits. A synthetic model can
delay EN acknowledgment after a stop request without claiming real hardware
latency. Finite engine/TC liveness is an environmental premise, not established
by the successful trace. Changes to direction-sensitive transmission must
retain the separate line lease. Guard mutants deliberately confuse these edges.

## Resource and validation limits

The new aggregate is allocation-free; chunk gaps are allowed. Complexity per
step and receive call is constant; formatting is bounded by the frame width.
Target helpers are explicit and the retained nonrecursive call graph's compiler
stack frames are conservatively bounded below the linked8 KiB reserve with
interrupts masked. No physical stack watermark or worst-case peripheral latency
is measured. Compare equivalent correctness and admission rules before any
throughput claim. Actual campaign, raw logs, source pins, mutant failures and
copied learner rejection are in the successor reports. External review,
independent learner assessment and physical work remain required.

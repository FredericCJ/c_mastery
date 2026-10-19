# PR-T38-02 assessor analysis

The private implementation actually invokes the retained PuDriver/PuRx functions;
it is not a second convenient UART stub. An internal one-byte request makes the
RX response and final TC obligations explicit. This is less efficient than a
fully pipelined DMA exchange, but the contract requires no such throughput. Work
is O(n) for n<=8; each step is O(1), with no allocation and bounded state. Calling
the progress operation more often cannot establish a wall-time bound without a
timer/scheduling premise. Own both spans until terminal state, even though the
lower layer may release the current TX byte earlier.

## T38.03.a and its V1

CPOL1/CPHA0/A5/LBCL0 produces14 edges, first falling, seven capture strobes at
one-based odd edges. The missing last pair belongs to data bit7. Enabling LBCL
adds edges15/16 and the eighth capture. The actual sd_sync producer is exercised
over2048 distinct value/mode/LBCL states, then su_trace is independently checked
against explicit edge equations. The changed peer requiring eight captures
rejects LBCL0; the complete eight-bit exchange adapter refuses that configuration.
This does not establish analog setup/hold. Physical data remain separately required.

## T38.03.b and its V1

RE alone admits no exchange. An actual explicit transmit byte is submitted to
the retained producer for each response. The test peer returns TX xor5A, with
raw SR/DR operations retained in model state. Check TXE-only before each later
TC; an early-return mutant must fail. The externally clocked SPI-slave comparison
changes the clock owner: sampling can be driven by external edges without local
transmission. It is not permission to treat USART CK as input. The comparison
is a finite edge/ownership analysis, not a second physical SPI device test.

## T38.03.c and its V1

Configuration with either TE or RE enabled returns BUSY without a CR2 write.
After both clear, CR2=CLKEN|CPOL<<10|CPHA<<9|LBCL<<8; CR3=0 excludes conflicting
modes. TE/RE are enabled together in one write. The actual F439 target mapping
uses PD10 AF7 and preserves unrelated port fields. A CPHA mismatch diverges at
edge1 despite correct idle polarity; a correct shortened trace first diverges
at edge15 when a full trace is required. The first-error report is useful only
with an independently declared expected mode, not by trusting the captured trace.

## Fault and evidence analysis

RXNE+ORE retains the old DR data and marks it erroneous; lost later input is not
manufactured as a zero byte. Error termination preserves the first IO result
over the producer's subsequent CANCELLED result. Timeout before TC stays active;
after quiescence the adapter disables TE/RE and drains residual RXNE. All later
RX cells remain untouched. Unsigned wrap tests use increments below2^31 and a
single absolute deadline. Exhausted internal-token headroom rejects admission.

The initial campaign's stack check overcounted mutually exclusive test cases
and functions eliminated by the linker. The corrected audit reads retained ELF
symbols, sums all nonrecursive retained frames and takes the maximum of the five
sequential test alternatives, plus32bytes. This is a conservative declared
callgraph bound under masked IRQs, not a dynamic high-water or interrupt-stack
measurement. The initial216-command attempt is preserved. Actual final source
and helper closure, intended mutant failures and exported-stub rejections are
verified separately; no independent assessor or learner credit is invented.

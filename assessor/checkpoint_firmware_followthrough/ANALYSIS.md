# P4 assessor analysis and claim boundaries

Use the unchanged original criterion IDs and objective-specific rubrics. Assess
each line below separately; a nominal UART test cannot stand for all objectives.

## t2708b

The actual reset images copy initialized data from flash, clear BSS and select
an aligned vector table. QEMU runs poison the RAM before reset, so loader setup
cannot masquerade as startup. The new F439 image adds nonzero/zero sentinels and
the EXTI0 vector; its six explicit object imports resolve only to the selected
original providers. Inspect final vector bytes as well as symbol names. SRAM
relocation0x4000 remains reachable; CCM0x10000000 links but fails the DMA audit.
All `.su` C frames twice plus1024 bytes is conservative under no recursion and
one interrupt level; neither missing frames nor nested/unbounded callbacks may
be ignored. This is not a physical stack watermark or F439 reset observation.

## t2805c

Polling accepts bytes at TXE, then waits at WIRE for TC. DMA may be quiescent
while TC remains false; both are necessary for the selected completion. An old
generation returns STALE and leaves ownership unchanged. Mutant35 releases at
the last register write; the public harness rejects it while the bytes still
look correct. Half-duplex mutants11/12 deassert too early or omit turnaround;
they compile and fail the retained test. The board adapter requests direction
release through the pure component and separately waits16000 DWT cycles after
the sampled TC marker. This guard avoids treating a pre-call tick sample as the
physical TC observation. Clock qualification converts cycles into a lower time
bound. Physical direction/transceiver behavior remains unobserved.

Versioned correction: the retained historical F439 demonstration used `~0x40`
when clearing USART_SR.TC, setting reserved31:10 contrary to RM0090 p1011. The
successor's executed entry uses `p4_tc_clear_word()` =0x3bf; the retained old
demonstration entry is not called. The original file and its historical reports
remain intact. A defined write-zero flag model covers1024 states and rejects
the old reserved-bit mask. This is register-contract evidence, not an observation
of how a particular chip reacts to a forbidden reserved-bit write.

## t2906b

N=4 means three waiting records. The queue's two release/acquire edges protect
payload publication and reuse. Drop-newest preserves the three accepted values.
The private oracle enumerates3^8 offer/take/idle histories for N4 and N2, using
a shifting array independent of the ring representation. Two producers can
both reserve head0: stores11 then22 and two publications of head1 lose11. This
is an explicit operational model, not undefined racy C executed as a golden
test. The implemented correction uses separate SPSC rings and source identities;
round-robin service is not a proof of physical arrival order. A real MPSC design
is another valid solution only with a complete reservation/publication proof.

## t2906c

The complete board adapter, capture ABI, decoder and instrument protocol now
exist. Only cross-build/object evidence is available for F439. Capture self-tests
are synthetic and cannot close physical evidence. In the declared zero-framing
UART model, 21 arrivals0..20ms at1ms intervals and service1562.5us admit16 at N4
and14 at N2. Smaller capacity reduces accepted latency while losing more events.
The independent unit-step and arrival-ledger models agree. The physical BRR139
baud differs, so these numbers are not golden board output. Criticize any report
that removes dropped/missing events from its offered-work denominator.

## t2909b

65530→6 is12 within the half-range premise. Reject32768 and preserve output.
At low-word rollover the naive combined value0 is neither committed0xffffffff
nor0x100000000. Four atomic store cuts reject odd-sequence reads until the even
commit; the final read returns0x100000000. UINT_MAX-1 sequence retires before wrap.
Bounded retries can fail; a higher-priority reader spinning while its interrupted
writer is suspended cannot guarantee progress. A plain-word seqlock in ISO C
threads has data races even when it retries. Interrupt-masked words are a possible
target-specific alternative only with nesting/compiler/barrier and latency proof.
Missing sequence, wrong capture time, corrupted CRC and guessed correlation are
separate capture-check rejections. ISR-entry time is not physical arrival time.

## t3005b

The new workload C3/T20/J0 is additional to C12/T50/J5. Its response iteration
is50,83,89,89, compared with50,74,74 originally. A separately simulated unit-step
priority schedule reaches the same finish. Raising the new C to12 exceeds the
declared200us bound. This is the named workload, not an exhaustive jitter theorem.
The UART model includes a separate active sender plusN-1 waiting slots; count
departures before arrivals at ties. Its infinite offered rate exceeds service.
End-to-end analysis must add ISR/foreground/framing interference and changed
baud, not simply label89us the sample-to-wire latency.

## t3005c

Raw captures bind to image, setup and clock identities. Internal trace stamps
and external electrical endpoints are separate channels. Trace-off has no
internal marker observations; accepted external latencies still exist. Warmup
exclusion affects distribution, not conservation. The checker rejects all-
warmup completion sets, wrong hashes, missing completions, ambiguous correlation,
wrong wire bytes and early direction release. A failed run with zero completions
has no latency result. The checker does not authenticate an instrument or convert
a claimed physical manifest into a physical measurement. New compiler/clock
configurations require new observations, not rescaling old maxima as a guarantee.

## t3110c

Timeout preserves first fault17 despite later99, retains ownership through100
failed DMA/UART acknowledgements, refuses a second begin, and rejects late TC in
ABORT. Only both disabled acknowledgements release and advance generation. A
stale old generation cannot complete the next transfer; the second failed
operation reaches the declared retry limit and retires. Mutant36 releases while
DMA/UART still active and is rejected. The board records bounded RAM failure
data and stops; it does not claim reset-persistent crash storage or automatic
exactly-once replay of a partially transmitted frame.

Accept alternatives that preserve these contracts and identify their costs.
Reject suppression of warnings, unsigned-wrap assumptions without bounds,
resolved forbidden helper providers, data races disguised as volatile, timing
claims made from host models and runtime absence inferred from source includes.
The closed author package provides evidence for finite executions and analyses;
non-author review, learner performance and board observations remain separate.

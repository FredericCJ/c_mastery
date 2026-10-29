# UART architecture and completion successor, revision 1

This required successor supplies the six original PR-T38-01 criterion/V1 pairs
T38.01.a–c and T38.02.a–c. Their exact AX identifiers, seven-day delayed tasks,
project contract and legacy shared assessment contract remain in force. Reuse
the learner's actual `sd_uart`, `sd_uart_decode`, `struct pu_driver`, `struct pu_rx`, F439 pin
adapter and T27 reset/linker; component names alone do not satisfy composition.

## Environment and prerequisites

Use ISO C17, profile E, eight-bit octets, uint32_t timestamps and the selected
F439 layout. Target code has fixed caller storage and scalar operations, no
heap, operating system, vendor HAL or library UART mechanism. The unchanged
producer contracts in `labs/peripheral_depth/uart/CONTRACTS.md` and
`labs/serial_depth/CONTRACTS.md` apply. Host stdio, Python exact fractions and
sanitizers verify target-independent claims only. MPS2 AN386 execution uses an
original RAM register model; it is not F439 peripheral or electrical execution.
Imported reset/vector code runs with interrupts masked. Inspect the actual
linked image and explicit compiler helpers, not only a successful compilation.

Required knowledge includes object lifetime, disjoint mutable controls, volatile
register side effects, half-range timestamp order and the difference between
DMA stop requests, buffer quiescence and final line completion. Source records
R-UA-RM21, R-UA-DS14, R-UA-CACHE, R-UA-MAX and R-UA-CAN in the successor resource
register identify the selected passages actually inspected. Board route,
oscillator tolerance and measurement apparatus require their own evidence.

## Problem and supplied inputs

Implement `uart_depth.h` using `learner.c`. The ordinary inputs are 0xA5/0x01
frames; 42 MHz, 115200 baud in both oversampling modes; 42 MHz, 3 Mbaud; payloads
of 1..16 bytes; 32 receive slots; delayed TXE/TC; errors; a stale clock generation;
a deadline at the first eight-byte chunk boundary; and a DMA buffer in CCM.
Public tests and the original PR-T38-01 fixture are supplied. Add tests which
distinguish suppression of a symptom from repair of the ownership error.

Return a frame/BRR/configuration result, a bounded transmit state and preserved
received byte/error pairs. Separately produce the architecture and evidence
dossier below. The exercise is necessary because a valid C array, correct AF,
successful DMA interrupt and decoded character establish different properties.
In particular, the three-million-baud divider example does not authorize a
three-million-baud MAX3232 electrical link.

## Operations and ownership

All controls and arrays are live, correctly sized, disjoint and exclusively
owned; controls start zeroed. Callbacks terminate, obey the original register
contract, neither reenter nor retain caller storage. No concurrent ISR adapter
is implicit. Mutating a control directly while active violates the contract.

`ua_format` derives the existing software frame and selects M/PCE/PS, STOP and
OVER8 fields. F439 supports seven payload bits with parity, eight without
parity, and eight with parity; reject seven payload bits without parity here,
although the more general software codec admits it. Pure rejection preserves
the entire output. `ua_apply` requires an immutable successful format result,
ready driver, idle ownership and TE/RE clear. It writes disabled CR1, CR2, CR3=0,
BRR, then enables TE/RE. It cannot validate a forged format or roll back physical
effects from a lying callback. The caller must ensure no receive is in flight.

`struct ua_tx` is deliberately an **8N1 polling aggregate**, not a general-width sender.
Initialize its `byte` producer with `pu_open`; preserve asynchronous CR2=0 and
do not reconfigure during a request. `ua_begin` borrows 1..16 immutable bytes,
requires a fresh positive external token and matching clock generation, and
reserves ceil(count/8) fresh internal tokens without wrap. Retire exhausted
controls until all old observations are impossible; do not simply reset a token.
The adapter uses actual producer chunks of at most eight bytes and waits for TC
between chunks. Gaps are allowed; no continuous-throughput claim follows.

A positive budget below 2^31 defines one absolute request deadline. Consecutive
observations within that request must advance by less than 2^31 ticks. Each
`ua_step` progresses at most one lower-layer operation and admits at most one
next chunk with the remaining budget. Deadline equality prevents admitting the
next chunk. The lower producer's completed final chunk may win at equality as
its contract specifies. `offset` counts fully line-completed chunks, not all
bytes already copied to a data register. Cancellation/deadline may leave a
partial prefix; no rollback or hidden retry is promised. The whole borrowed
span remains owned until aggregate `active` is zero, even if the current lower
buffer is already released. A missing TC can retain line ownership indefinitely;
a finite polling limit is not quiescence. A pre-write TC snapshot cannot release
the byte written after that observation.

`struct ua_rx` owns 1..32 caller `struct pu_byte` slots and a retained producer bridge. Before
each call the bridge is empty; only these APIs mutate it. Full output storage or
a saturated bridge error counter returns FULL without reading DR. Otherwise
`ua_rx` observes at most one SR/DR pair, transfers the actual older RDR byte with
its raw error mask, and preserves queue order across wrap. Empty hardware returns
TIMEOUT; empty software queue returns BUSY and preserves `ua_take`'s output.
An overrun records loss; it does not prove that the older RDR byte vanished.
The application decides whether to consume or discard an errored byte. A peer
can still overrun hardware while the application is full. Resetting counters
requires a separately quiescent owner; this is no lossless-flow-control guarantee.

## Architecture and changed-condition dossier

For .01.a draw CPU/C-object → bus/MMIO → USART → MAX3232 → cable → peer and
assign payload ownership, framing and electrical conversion separately. State
the payload lifetime under polling and DMA. For .a-V1 replace the controller
path with SPI and MCP2515: identify which completion permits releasing SPI
source storage, which component performs CAN arbitration, and which completion
could support a CAN-delivery claim. A selected CAN transceiver and route remain
separate from the controller; this task does not implement a complete CAN port.

For .01.b use the supplied mapped F439 adapter. Show correct PD8/PD9 AF7 and
reject [0x10000000,0x10000008), despite its CPU accessibility. Check the selected
DMA aperture's upper endpoint and a crossing range. Bind a linker map to the
actual object, not an address written in a report. For .b-V1 use the declared
original 32-byte write-back line investigation: initially cache/RAM contain zero;
CPU changes byte0; DMA reads RAM; then reverse the writer; finally place a dirty
unrelated byte31 in the same line. Derive cleaning, invalidation and whole-line
ownership obligations. Clean and invalidate are not interchangeable. Identify
the extra selected-M7 routing, MPU/cache attributes, maintenance/barrier and
errata evidence needed for a real port. The AN4839 F7 DTCM example is not evidence
for an arbitrary H7 DMA route. No M7 maintenance instructions execute here.

For .01.c classify four concrete artifacts: host bit trace, linked Thumb ELF,
RAM register readback, calibrated physical capture. For each state its strongest
supported claim and one counterexample to an overstatement. In .c-V1, the analyzer
decodes one character while an independent measurement reports +3% clock error.
Use a declared ±1% local accuracy gate and a separate receiver aggregate-error
budget; neither a decoded sample nor the ±1% exercise gate is a universal UART
tolerance standard. Retain the failed accuracy gate and any unmeasured margin.

For .02.a derive all framed bits, inject parity and stop failures, preserve the
decode output on rejection, then change to seven payload bits with parity and
derive M rather than relabeling eight payload bits. The software decoder checks
all selected stop bits; the F439 receiver's documented sampling differs.
For .02.b derive exact rational baud/error for each oversampling mode, then
change APB to 21 MHz and 16 MHz and reject a stale generation. For .02.c retain
the distinction between TXE, TC, RXNE and ORE. Its V1 must show that a DMA flag
alone does not acknowledge EN=0, and that buffer release still does not permit
disabling a direction-sensitive line before USART TC.

## Deliverables, verification and assessment

D1: an unaided boundary diagram, bit/register derivation, invariants, failure
prefixes and exact source/claim ledger. D2: composed C source, original failing
cases and repairs, generated code, explicit helper/link map and stack account.
D3: raw test results and independent exact rational/trace oracles across GCC,
Clang, optimization and sanitizer profiles. Record state/output preservation,
not only return values. D4: six separately identified original V1 tasks after
their minimum seven days, including the cache and evidence counterexamples.
D5: the distinct physical capture dossier in BOARD_CAPTURE.md. Resource costs
include constant control/32-slot storage, one bounded step, chunk gaps, SR/DR
traffic and maintenance over whole lines. No heap use is permitted.

Native tests compile the learner adapter with its UART/serial producers and run
selectors0..4. The mapped test additionally compiles the target producer with
PU_TARGET_MODEL. `target_build.py` takes every target dependency explicitly;
an export never silently selects assessor implementations. Reject a compiled
unfinished stub and mutations of M, fresh generation, error preservation,
deadline renewal, premature TC, DMA cancellation, pin AF and stream channel.
The assessor uses independent Fraction arithmetic and a small write-back model;
the latter demonstrates a mechanism, not the behavior of all cache hardware.

Per original criterion, level0 gives a wrong boundary/unsafe ownership or invents
evidence; level1 repeats happy-path values; level2 derives and implements the
named mechanism, diagnoses its counterexample and states the remaining limits;
level3 independently transfers under its exact V1 and revisits the invalidated
assumptions. D1–D5 and the original criterion-specific rubric are assessed
separately. Finite passing tests cannot prove absolute mastery.

Graduated hints: first mark who can still read each buffer; next mark the source
and time of every completion flag; finally compare a correct pin selection with
the bus-master path and compare a dirty cache line with backing memory. Preserve
unaided attempts before using hints. Assessor implementation/analysis remains
outside learner packages. All defects use synthetic local arrays/registers and
dedicated teaching targets; no external bus, operational equipment or host
security changes. A wrong byte or premature release is a bounded property
violation, not a claim of arbitrary code execution.

Validation is authored plus the separately pinned author campaign. External
review, independent learner attempts and physical observations remain unperformed.

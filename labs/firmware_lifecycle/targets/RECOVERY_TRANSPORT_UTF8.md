# FW-RX: bounded recovery transport before flash authority

Required T59 target continuation for the original PR-T59-01/03/04/05 criteria
and delayed variants. Families A/B/C/D/E/F/G/I/J and the B06 common contract
apply. Prerequisites: actual FW-BOOT authenticated envelope, FW-STORAGE fixed
slot map, FW-START cold clocks, byte/extent arithmetic and exact MMIO ownership.
Use C17 fixed storage; permitted Monocypher signature/hash provider is only the
explicit FW-BOOT primitive. Implement all framing/UART/admission logic from
supplied primitives; do not import a bootloader or protocol library.

Implement recovery_transport.h. One frame is exactly12 header bytes,546..2080
signed-envelope bytes and64 signature bytes, at most2156 total. Header bytes
0..3 are ASCII B6UP,4 is1,5 is core0/1,6 is slot0/1,7 is0,8..9 are little-endian
envelope length,10..11 are0. The selected slot must come from trusted BSLayouts;
the signature-covered payload base/core must agree with that choice. A packet
header grants no authority. No CRC substitutes for signature verification.

RTFrame is a caller-owned fresh/reset parser object, accessible only through its
API during input. rt_byte consumes one byte, returns MORE until the exact end,
then READY. An invalid header, overfull input or another byte on that same
completed object poisons it until explicit reset. There is no implicit scan for
a new magic value. A transport/session timeout abandons the whole frame; reset
only discards parsing state and is not a secure erase. rt_admit may publish an
FBImage only from a complete immutable frame and actual current-policy
verification. All failure outputs remain unchanged. After admission the frame
must stay owned/immutable until its consumer has finished; a forged ordinary C
state value bypasses the API precondition, not the signature check.

The selected H745 UART adapter assumes FW-START admitted cold HSI64 and exclusive
PD8/PD9, USART3 and serialized GPIO/RCC register ownership. Require reset APB1
kernel selection and disabled reset USART mode. Enable GPIO-D/USART3 clocks;
configure PD8/9 alternate-function7, push-pull, medium speed and pull-up, preserve
unrelated pin fields, select oversampling16/8N1/FIFO disabled/PRESC1/BRR556.
The achieved nominal baud is64000000/556, not exactly115200. Wait a finite
iteration budget for TEACK/REACK. Timeout disables UE and retains clocks/pin
configuration; it is not a whole-device rollback or measured deadline.

Capture ISR before reading RDR. PE/FE/NE/ORE have precedence even when RXNE is
also set: preserve output byte, report raw status, clear only observed error
flags through ICR and flush RX through RQR. Never publish an error-marked byte
as a recovery-frame byte. Idle preserves byte. rt_uart_step poisons a partial
frame on any hardware error. TX waits separately for TXE and for final TC; a
post-write timeout means the byte may already have transmitted and retry may
duplicate it. There is no DMA, ISR, OS or external allocation dependency.

D1: derive frame capacity, admission boundaries and exclusive lifetime. D2:
implement parser, current-policy adapter and UART operations. D3: reject every
truncated prefix and every single-byte high-bit corruption of the684-byte
synthetic fixture, accept minimum/maximum envelope extents, and reject revoked
keys and trailing input. D4: trace all four error flags with RXNE, unavailable
clocks/acknowledgments, GPIO preservation and TXE-without-TC; outputs and parser
state must match this contract. D5: run six compiler/sanitizer profiles, reject
the bypass-signature and error-byte-publication mutants plus learner stub, then
cross-link the actual M7 receive/admit probe and audit helper closure, signing
symbol exclusion and stack. D6: connect the resulting immutable authenticated
frame to the persistent-update coordinator while preserving the old committed
pair until the separate health/commit rules allow replacement. D6 remains an
explicit downstream obligation; the receive probe does not claim that commit.

Level0 treats packet magic/CRC as authorization or accepts an error byte. Level1
receives a good frame only. Level2 demonstrates D1–D5 and correctly identifies
D6's separate authority/commit barrier. Level3 independently completes the
original delayed interrupted-transfer or key-revocation variant after seven
days, without copying the worked framing solution. Hints: admit length before
payload storage; keep READY distinct from authenticated; inspect status before
consuming RDR; TXE is not wire completion. Variants change the clock tree,
replace polling with DMA ownership/cancellation, or inject idle/error after a
valid prefix. Each requires a new contract/evidence argument, not new constants.
Assessor reasoning is in RECOVERY_TRANSPORT_ANALYSIS.md.

Only original teaching targets, synthetic keys/bytes and local register models
are authorized. Attacker control is the incoming byte stream, truncation and
error schedule; harmless success is an unverified image reaching the mutant's
admission output. No board flashing, real credentials or external targets are
used. A target-linked MMIO adapter does not establish a waveform, board routing,
electrical loading or actual UART operation. The datasheet supports the MCU AF
mapping, not a particular board's jumper state. The target probe stages/admit
only and never writes flash. Physical serial capture and persistent update
composition remain separate; current command/source evidence is in
recovery_transport_validation.json. No independent review or learner mastery is
inferred from finite tests.

# Changed serial contracts, revision 1

This required successor supplies the exact original X-T38-002-V2, X-T38-003-V1/V2,
X-T38-006-V2, X-T38-008-V1/V2 and X-T38-009-V1/V2 tasks. Keep each original
identifier, rubric and minimum seven-day independent delay. Read its original
Problem, prerequisite learner artifacts and shared exercise contract. The
following concrete mechanisms refine those tasks; author execution cannot
administer an independent delayed attempt.

## Prerequisites and environment

Use ISO C17, eight-bit bytes, exact-width integers, fixed caller storage and one
serialized owner. Implement `variants.h` through the unfinished `learner.c`.
Import the actual learner `serial_depth.h` codec/FIFO and current `can.h`
incremental codec, fault model and controller queue. A library implementing the
assessed divider, framing, filtering or decoder is not permitted. Host Python,
stdio, exact arithmetic and sanitizers are verification tools. Target code has
no allocation, recursion, operating system, standard-library linkage or hidden
compiler archive. Audit final helper closure, retained instructions and stack.
Explicit Cortex-M4 execution tests these arithmetic/state models; it is neither
RP2040 execution nor a physical F439/receiver/CAN observation.

All controls, input spans and outputs are live, correctly aligned, mutually
disjoint and stable during each call. Output preservation on admission failure
means the entire supplied output object. Only these APIs mutate initialized
state. No callback, ISR concurrency, fabricated control state or arbitrary
pointer validity can be established by ISO C diagnostics. Controls retained
across reset have an owner lifetime distinguishable from every old observer.

## Concrete problems and necessary distinctions

**Divider, X-T38-002-V2.** The new inspected RP2040 UART divisor has a 16-bit
integer and a six-bit fractional field in separate registers. Derive
`q = round(4 * clock / baud)` with ties upward; integer `q/64`, fraction `q%64`.
This differs from F439 OVER8 packing. Admit clock 1..180000000 Hz, baud
1..12000000 and q 64..4194303; these are deliberately bounded local operating
inputs, not all board clock configurations. `sv_baud` additionally requires
absolute rational error no greater than `maximum_ppm` (0..1000000): compare
`abs(4*clock - baud*q)*1000000` with `maximum_ppm*baud*q` in sufficiently wide
unsigned arithmetic. For 80 MHz/115200, require integer43/fraction26; 79 ppm
fails and80 ppm passes. Account separately for both oscillators, sampling and
line distortion before claiming an allowable physical baud error. This routine
does not reconfigure a live UART or replace the RP2040 register-update protocol.

**Nine-bit word, X-T38-003-V1.** `sv_word` takes M0/1, parity0 none/1 even/2 odd,
one/two stop bits, CPOL/CPHA/LBCL0/1 and a uint16 payload. Hardware word size is
8+M, and parity occupies its most significant transmitted bit; it does not add
a tenth clocked bit to a nine-bit word. Payload size is word size minus one
when parity is present. Wire output is start0, LSB-first payload, optional
parity, stops1. CK has no start/stop clocks; there are two edges for each
hardware word bit except the last when LBCL0. Return exact clock levels and
capture-edge markers. In particular M1/even parity/FF has eight payload bits,
parity0, and16 edges when LBCL0. Reject a payload that requires the parity bit.
Derive all parameter combinations; do not blindly add two edges to an earlier
8-bit result.

**Captured clock, X-T38-003-V2.** `struct sv_edge` supplies a measured or synthetic clock
level, sample marker, and durations for which data is stable before/after that
edge. Label their provenance. For a declared bit period1..160000000 ns the
selected RM0090 synchronous premise requires at least one sixteenth of a bit
on each side of a sample, conservatively rounded upward. `sv_capture` accepts
2..18 even edges, canonical bits, and reports the first mismatch: polarity1,
phase2, setup4, hold8, combined as needed. Invalid syntax preserves the result;
a well-formed but unsuitable capture returns IO with index/mask. A160 ns bit
requires10 ns;161 ns requires11 ns. Diagnose polarity, phase and hold failures
separately. The fixtures are synthetic observations, not fabricated analyzer
captures; a real selected peer still needs trace uncertainty, timing limits
and instrument evidence.

**Receiver, X-T38-006-V2.** Replace the earlier generic ±200 mV classification
with inspected THVD1450 SLLSEY3E behavior. The local model admits each bus
terminal and their differential only within ±15000 mV, with separately
established supply/temperature/enable conditions. Enabled open input is HIGH;
disabled is HIGHZ. With unknown history, differences below−200 are LOW,
above−20 HIGH, and the intervening range UNKNOWN. The table's typical−130/−100
thresholds are not guaranteed constants. An explicitly characterized synthetic
profile may supply low/high thresholds with low≥−200, high≤−20 and separation
at least30 mV. Preserve the prior LOW/HIGH/UNKNOWN between thresholds, including
equality; transition only outside them. Demonstrate two outcomes at−120 mV
under opposite histories with the example low−130/high−100. Do not claim that
the manufacturer's indeterminate-region table guarantees this characterized
trajectory. Measure actual thresholds, noise, common mode and temperature
before promoting the synthetic profile. A valid high level or failsafe idle
never authenticates a peer or proves a working cable.

**Stream reset, X-T38-008-V1.** `struct sv_stream` owns the actual `struct cn_stream` and a
positive uint32 generation. Reset explicitly abandons its old partial input
and requires a strictly greater generation; exhaustion cannot wrap to1.
`sv_stream_feed` requires the exact live generation before invoking `cn_feed`.
Stale input preserves state, consumed count and decoded output. MORE consumes
a valid prefix, FRAME publishes one complete copied frame, IO reports the
actual terminal invalid-frame path, and invalid input preserves outputs as
specified by the producer. After terminal state use a fresh generation. Test
all two-chunk splits, including mandatory-stuff boundaries, abandoned prefixes,
stale completion/reset and UINT32_MAX retirement. Resetting a bare C struct
without an identity cannot reject a queued old chunk.

**Current acceptance, X-T38-008-V2.** `sv_policy` is a selected current
ISO11898-1:2024 policy decision composed after other field/CRC/stuffing checks;
it is not a full CAN receiver. Error signalling is enabled. For CEFF receive,
dominant SRR and either r0 value are admitted. A CAN FD tolerant receiver treats
recessive CEFF r1 or CBFF r0 as a protocol exception; this is not a form error
and must not increment counters. For an otherwise valid frame, a receiver
admits dominant final EOF and requests overload; a transmitter does not validate
that same frame. Transmitter reserved/SRR requirements remain strict. Inputs
are canonical flags; absent CBFF SRR/r1 must use placeholders1/0. Compare this policy
with the intentionally stricter retained decoder and the separately inspected
historical text; do not silently widen that decoder or invent a revision
change where the same rule already existed. Compose actual `cn_fault` for a
real classified receiver error crossing REC127→128, while a protocol exception
keeps counts unchanged. Broader error-disabled and XL/FD behavior remains
outside this selected decision; the original task requires precise acceptance
differences, not an unsupported whole-protocol conformance claim.

**Banks and loss conversion, X-T38-009-V1/V2.** Implement 32-bit bxCAN packing:
standard ID at bit21, extended ID at bit3 with IDE bit2, RTR bit1. Mask mode
admits one key/mask pair and requires IDE/RTR to match; list mode admits two
exact keys. Decode all admitted fields before shifting. Compare16-bit scale
capacity (four list entries or two mask pairs) as a separate inspected layout;
the current API implements only32-bit scale. Enumerate all standard IDs and
remote/data choices against mask120/7F0, then a list containing both a standard
and extended frame. Explain why the original11-bit predicate lacked format and
frame-kind constraints.

Use actual `struct sd_fifo` newest-overwrite and `struct cn_queue` oldest-drop producers for
the changed loss policy. Histories [0,1,2,3] and [9,1,2,3] leave the same changed
queue [1,2,3], yet the original desired queues begin0 and9. No post-overflow
adapter can reconstruct the lost value without a prior retained copy or extra
service guarantee. State this impossibility instead of relabeling policies.
The existing selected controller's actual NART execution remains required;
reporting an arbitration loss as terminal differs from automatic retry. Any
new hardware conversion must preserve these client-visible outcomes and name
which hardware observations it relies on.

## Reasoning, deliverables and verification

D1: derive representations, bounds, ownership and exact failure/publication
rules for the selected original variant. D2: implement the public API from
permitted primitives and actual prerequisite artifacts. D3: supply independent
bit/rational/trace expectations, full unchanged-output canaries and diagnosis
of the supplied intended mutants. D4: give compiler/optimization comparison,
final link/helper/stack audit and resource costs. D5: record actual source
passages, instrument limits and the fresh delayed changed-condition attempt.
All operations are bounded: O(1) divider/filter/policy, at most18 edges, linear
in the bounded160-bit chunk for decoding, and fixed control storage. Compare
abstractions only at the same accepted fields, progress and failure contracts.

Run baud/word/capture/receiver/stream/policy/bank selectors with both compilers
at O0/O3 and ASan/UBSan; run the explicit target build with supplied source
inputs and audit the final image. A compile-only failure of a mutant is not a
semantic rejection. Require tests for every stated boundary and one new
falsifying condition. The separate assessor oracle derives rational fields and
word counts independently; source-bound reports disclose exact execution.

## Rubric, hints, variants and scope

Keep each original objective rubric. Level0 violates admission, lifetime or
claims unsupported physical/conformance evidence. Level1 repeats fixture values.
Level2 derives premises, implements and diagnoses the actual mechanism with
bounded evidence. Level3 independently solves the unchanged delayed task after
at least seven days and defends a new boundary under revised assumptions.
Finite tests cannot prove absolute mastery. Hints progress from identifying the
new field width, to drawing publication/reset boundaries, to constructing two
indistinguishable loss histories. Keep the unaided attempt before using hints.

Further transfer changes divider rounding at a carry, character parity width,
uncertainty that overlaps a capture boundary, decoder generation exhaustion or
a different filter scale. Do not treat changing only a constant as independent
transfer. All executable inputs target original local teaching models; no
operational bus, production vehicle, external service or other person's data
is involved. Current physical observations, independent learner assessment and
external review remain unperformed. Assessor implementations/solutions are
excluded from learner exports.

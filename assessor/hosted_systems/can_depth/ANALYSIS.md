# Assessor-only classical CAN interpretation

The exact-ID filter compares three independent facts: identifier value, format,
and remote/data kind. Standard0x123 and extended0x1abc0123 both match a low11-bit
mask; including EFF and RTR discriminates them. The independent oracle compares
logical tuples, never the production mask expression. It covers32,768 cells
over all11-bit IDs, both input formats/request kinds and all four filter kinds.
This does not exhaust29-bit IDs; explicit maximum and unequal-high-bits cases
cover their different boundary. Error frames use diagnostic-class admission
and are not normal data IDs. The default error mask0 therefore admits none.

Reject casting an arbitrary byte span to can_frame: the address may lack the
required alignment. A local copy avoids that assumption and the span length
is checked before copying. Do not assert a portable16-byte wire record from
the selected ABI's size. Encoding initializes all ABI bytes; decoding ignores
reserved bytes and copies only meaningful payload. RTR carries requested length
but no application payload, so the normalized value has zero data. A length9
clamped to8 is a rejected mutant: silent truncation would conceal bad input.

The loopback model has16 cells (two sender/receiver choices and two Boolean
flags). A peer receives with loopback enabled; the sender also needs own
reception. Both remain subject to filtering. This model is not kernel CAN
execution, arbitration or a physical acknowledgment. A synthetic BUSOFF class
tests decoder behavior only; the implementation never transmits error frames.
Model outputs, real SocketCAN observations and physical captures are separate
evidence categories. The campaign's read-only interface inventory must not
turn a missing vcan into a passing CAN socket run.

Ordinary level2 requires the format collision counterexample, length and raw-DLC
policy, unaligned-span handling, error filter, loopback/own distinction and
bounded resource/lifetime argument. Reject generic echo tests as evidence for
all those competencies. The four compiling mutants remove EFF discrimination,
clamp length, admit all errors and receive own frames unconditionally. Each
must be rejected semantically, not through compile failure or a crash.

For the separately withheld seven-day transfer, provide the assigned two-node
hardware dossier only when it exists. Ask the learner to fill BENCH.md's plan,
calculate timing under a changed oscillator, justify actual register encoding
and design a discriminating simultaneous-arbitration capture. The mathematical
example16MHz/prescaler2/(1+13+2)=500kbit/s, sample point14/16=7/8, is synthetic
and not a recommendation for any board. Valid alternate parameterizations
must satisfy inspected controller constraints and an uncertainty argument.
Never manufacture physical timestamps or complete the learner attempt.

Counterexamples to superficial transfer: two API sends that never overlap on
the wire; an echo marked own instead of physical reception; a successful byte
comparison with no voltage/termination evidence; correct nominal bitrate with
an illegal SJW/register encoding; and decoding a synthetic error class as proof
of an actual bus fault. The supplied checker deliberately reports device legality
and physical validation false even when its arithmetic is correct.

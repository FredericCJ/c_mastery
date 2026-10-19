# Assessor: exact PR-T46-01 criteria and delayed variants

## 01a

INT0, BOOLfalse, NIL and REF0/gen1 are four distinct values. NIL has no payload;
scalar generation is irrelevant; a reference compares both slot and generation.
Neither equality nor a valid-looking handle establishes referent liveness.
The CVM1 semantics require checking tags before arithmetic. Accept different
physical representations only when these observations and refusal rules agree.
Do not require one enum layout, union layout or padding pattern.

The changed numeric API deliberately separates versioned rules from the old
CVM1 instruction set. The representation is a tag, int32 units and reference
generation. INT/INT stays checked; Q16 promotion is an admission step. Therefore
INT32768 plus Q16(-65536) refuses even though the mathematical sum32767 would
fit. This distinguishes a candidate who postponed promotion checks. Negative
fractional multiplication truncates toward zero; -1*32768/65536 produces0.
The floor-rounding mutant produces -1. Tag-strict equality remains distinct
from mixed arithmetic. BOOL conversion into INT is a semantic change, not a fix.

The Fraction oracle derives exact mathematical values, admission and units,
then truncates once. It does not execute the C implementation to obtain expected
answers. Finite selected extrema and seeded random cases do not prove all inputs;
the intermediate-bound argument supplies a different form of evidence.
Mutants1/2/3 respectively ignore tags/generations, accept Boolean arithmetic,
and round a negative fixed product down. All must compile and reach their
specific semantic counterexample; a crash is insufficient.

## 01b

The literal is17 bytes: four magic bytes, count1, one six-byte descriptor and
six code bytes. Every proper prefix refuses without changing the destination.
Each capacity below17 refuses encoding without writing even the header or
written count. INT32_MIN remains raw little-endian bytes; opcode255 is legal at
the container boundary and illegal only at the later instruction boundary.
Compare selected fields after successful decoding rather than comparing struct
padding. Failure canaries deliberately compare the previously captured object
representation because no publication is permitted at all.

Native LP64 little-endian, M4 ILP32 little-endian and PPC64 ELFv2 big-endian are
different observed profiles. CHAR_BIT8 and the exact-width types are explicit
premises. An unchanged literal on all three supports these profiles, not every
ABI. `rv_signed_words` adapts signed ARM division to the already assessed T11
unsigned limb provider; quotient sign is XOR, remainder sign follows dividend.
Unsigned magnitudes avoid negating INT64_MIN in signed C. Direct exceptional
ABI inputs return zero by this teaching helper's contract; source-level C
division by zero or INT64_MIN/-1 remains undefined and is never executed.

An old Thumb build using `-lgcc` cannot be called strict merely because the new
builder passes. Keep its identity and give the new source/object/link receipts
their own scope. The public target runner accepts explicit runtime paths;
assessor paths are provided externally, never copied into a learner export.

## 01c

The original raw codec checks layout before publishing. Count and length are
structural properties; no instruction work is needed. A gap and overlap both
violate descriptor partitioning even when every individual range fits.

RVE2 is an outer metadata envelope whose base remains exact CVM1. New-reader
version rejection, old-reader magic rejection, unknown optional skipping and
unknown required refusal are four different behaviors. A known optional field
still needs format validation. A digest field carries bytes and grants no
authenticity. Duplicate known fields are rejected to avoid ambiguous precedence.
Unknown sections count against resource bounds even if skipped. Require exact
end consumption so an attacker cannot append unnoticed data to a claimed image.

The encoder rejects an image carrying skipped sections because it cannot
preserve them; claiming a lossless round trip would be false. The decoder's sole
publication point follows all sections and exact end. Mutant4 skips required
unknown fields, mutant5 publishes after an early section and then fails, and
mutant6 ignores trailing bytes. The truncated suffix case distinguishes atomic
publication from merely returning an error code. These are synthetic local
property violations; no claim of control-flow hijacking is supported.

## Independent transfer and alternatives

Keep the learner's original predictions and seven-day interval separately from
author readiness. Withhold one changed numeric admission/rounding policy, one
supported target identity and one unknown-section failure placement. Require
revised equations and acceptance before coding. Valid alternatives may use
checked division decomposition, a caller-owned staging object or a different
internal tag layout, while preserving the declared public observations.
Changing truncation to round-to-nearest, silently discarding sections on encode,
or supplying a native pointer as a reference is a different contract.

The API-versioned numeric extension does not claim a new VM instruction. If a
learner integrates it into bytecode, require a new opcode/version, verifier,
decoder, dispatcher and compiler rule with tests at each boundary. This extra
claim cannot be credited from arithmetic tests alone. The later required T46
verifier/compiler/runtime criteria retain their own obligations.

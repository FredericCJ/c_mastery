# T35.07 assessor analysis

This analysis supplements the original three criteria, never replaces them.
The exact source-bound command/log inventory is in the adjacent research
package. A seven-day delayed learner judgment and external review are absent.

## .07.a: three different properties

The valid copy sums triangular numbers through36. A count9 must be rejected
before copying, leaving77 unchanged. Fortify knows the actual destination8
and stops the original call; ASan independently diagnoses its stack extent.
Neither result demonstrates arbitrary execution. Plain never executes the
defective count. In all variants, role1/trusted0 still increments the original
synthetic counter7→8 using defined ordinary data operations. The repaired
function ignores role, rejects counter saturation, and denies without writes.
This falsifies the claim that PIE/RELRO/NX/canaries/fortification repair policy.

## .07.b: observed eligibility and cost

Read the actual report rather than assume a compiler's default. Ubuntu GCC15
already defines fortify3, causing the aggregate's override diagnostic; retain
the initial strict failure and the explicit warning-only experiment. This
is an explained toolchain interaction, not suppression of a code defect.
Clang21 rejects the aggregate option but builds the individually named flags.
The flawed function retains local storage through an external consumer and
contains both checks in the hardened final images. The tiny identity function
has no remaining local array or guard. DYN plus PIE tag and an interpreter
differs from a shared object; synthetic DYN-alone mutant must fail.

Actual maps are parsed by image path and PT_GNU_RELRO page-covered addresses;
the last partial writable page is not promised protected. Static musl lacks
the glibc memcpy fortify wrapper even with the macro defined; its own guard
runtime remains present. Do not call this a fortify3 implementation. The
exact inspected musl header, glibc fortified wrapper and compile/link trace
make the distinction checkable. The musl static image has no interpreter,
while hosted dynamic images require startup and libc/loader participants.

31 samples×200,000 calls are raw local observations, not a calibrated
microarchitecture study. Compare valid semantics/checksum, retained call
sites, binary size and stack allocation before discussing overhead. Startup
is outside the timed loop but affects process state; scheduling, caching and
frequency are uncontrolled. Changing libc changes more than fortification.

## .07.c: explicit helper closure

The Arm probe references __stack_chk_guard, __stack_chk_fail and the original
touch helper; the incomplete link must fail on the intended guard interface.
The fixed original guard and noninstrumented failure handler close it without
libc, division helpers, allocation or formatted logging. One volatile record
write precedes an indefinite terminal wait. This is a bounded diagnostic
action, not bounded recovery or guaranteed survival of a corrupt stack.
The fixed guard has no secrecy; the link entry is not a reset vector. Native
execution checks the record operation; target disassembly checks the wait
and final undefined closure. No target CPU/startup/physical observation is
inferred from a successful cross-link. A real board adapter would require
valid startup/data initialization, entropy policy and failure containment.

## Discriminating controls

SH_MUTANT_BOUND rejects the valid boundary8: safe but semantically wrong.
SH_MUTANT_POLICY trusts wire role: valid-data authority assertion fails.
SH_MUTANT_RECORD omits the bounded diagnostic action: native sentinel fails.
Each mutation compiles separately and must fail the named assertion, not an
unrelated compiler error. Unfinished public C and Python implementations
compile/import but fail semantic gates. The actual plain binary is also an
observed negative hardening control; its marker absence is not fabricated.

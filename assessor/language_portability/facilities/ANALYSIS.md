# Assessor analysis for the twelve facility/selection tasks

## Arithmetic, bits and failure publication

For int32 addition use b>0/a>MAX-b and b<0/a<MIN-b; the bounds themselves are representable. For negative mixed operand, -(a+1)+1 is formed in unsigned space, avoiding -INT32_MIN. Subtract its magnitude from b only after b>=magnitude; otherwise the mathematical sum is negative. The resulting unsigned value must still fit65535. For nonnegative a preflight a<=65535 and b<=65535-a. Actual ckd_add has infinite-range mathematical operand semantics but writes its converted result even when it returns true; staging is essential to match the C17 wrapper.

Zero is a first-class bit input: count zeros32, width0. Do not use an unspecified/undefined zero builtin as an oracle. The manual divide invariant is r<divisor before each step and the quotient prefix is exact. The next mathematical remainder is2r+bit. A saved carry handles the possible65th bit; if carry is set, unsigned subtraction modulo2^64 equals the mathematical difference because2r+bit-divisor is nonnegative and less than divisor. No signed overflow or width-sized shift occurs. Five mutants isolate genuinely different properties.

## Preprocessing and edition support

N3096 binary inclusion creates integer tokens with the specified octet mapping for unsigned-char array initialization. The depfile must name resource.bin; a changed byte must invalidate the consumer. __VA_OPT__ depends on tokens after argument macro expansion, hence EMPTY behaves like absent arguments. The C17 spelling rules are justified with inspected N1570 header macros and actual C17 translations, not by claiming published C17 was read. C23 keyword aliases and nodiscard recommended diagnostics are distinct source clauses.

The GNU statement expression is documented and accepted in GNU modes, but strict modes diagnose it. Its local variable avoids duplicate evaluation only for this noncolliding example; the manual documents shadowing and lifetime traps. The standard inline replacement keeps the observed integer effect without claiming every GNU expression can be ported mechanically.

## Targets and upgrade

The five-bit selector is conjunction, not a vote or edition comparison. Compilation, header access, linking and execution must each be observed. Strict helper admission is a further repository contract. Withheld headers and compile-only RISC-V are deliberately distinct missing layers; record why the stage is unavailable, never fill it with a presumed true value.

The direct M4 64-bit divide references an implicit ABI helper. Rejecting its unresolved link is meaningful even though a hosted build works. The64-step baseline supplies the mechanism and has an audited zero-undefined final image; semihosting is an explicit harness boundary, not a target arithmetic dependency. Actual M4 QEMU execution is not silicon evidence.

The privately extracted Clang20 and installed Clang21 runs are a real compiler change with exact executable/header/library identities. Only claims affected by those probe sources are rerun; no universal regression or new-support conclusion follows. Keep independent Python expected integers unchanged. Full cost evidence is build-specific, not a universal claim that a language feature is free.

## Assessment

Require all six ordinary and six delayed reasoning artifacts. A student who quotes the edition macro as header evidence, treats ckd_add failure as unchanged output, equates link with run or assumes a no-helper flag provides a helper fails the corresponding criterion despite a green unrelated test. External review, delayed learner execution and exact published-edition access remain separate dimensions.

# Signed rational-unit proof and assessor boundaries

Let a=|v|, n>0, d>0, q=a/d and r=a%d. The identity
a*n = q*d*n + r*n proves floor(a*n/d)=q*n+floor(r*n/d).
Since r<d<=2^32-1 and n<=2^32-1, r*n<(2^32-1)^2<2^64.
The larger product q*n is admitted only when q<=limit/n. Its addition is
admitted only when extra<=limit-base. Thus neither tested arithmetic operation
overflows uint64 or the sign-specific semantic limit. The precision flag is
exactly whether r*n is divisible by d; it is not whether the original ratio
was reduced, nor whether the integer result happened to be zero.

For negative v, conversion to uint64 followed by unsigned0-minus produces its
magnitude even at INT64_MIN. The only successful magnitude not representable
as positive int64 is2^63, handled by the INT64_MIN constant. Other admitted
magnitudes are converted within range before negation. The code never evaluates
-INT64_MIN or casts unsigned2^63 to int64. Failure occurs before the single
result assignment, so both numeric result and inexact flag retain prior values.

The period states microseconds per tick. Conversion to ticks therefore uses
q/p, not p/q. Equal nominal field layout is irrelevant to this dimensional
direction. A typedef cannot distinguish two identical underlying integer types;
a struct can reject accidental substitution but cannot prevent explicit field
extraction or an intentionally false construction.

The old seconds API is a different exact positive conversion, tested through
the actual provider. The new prototype does not replace it or claim that a
C++ static_cast supplies the same failure policy. Under a64-bit common signed
representation, a multiply-then-divide duration expression can overflow its
intermediate even when the final mathematical value would fit. The supplied
C++ comparison deliberately executes only admitted small values, not that
undefined-behavior counterexample.

Tests use exact Python integers for expected quotient/range and then compare
C results. Their finite sample count is not an exhaustive proof. Require the
bound argument, exact callback-free ownership/non-applicability explanation,
assembly and equal-contract timing observations before accepting the complete
criterion. Independent learner and physical-clock claims remain separate.

# T65.09 assessor: canonical inputs make cancellation sufficient

Validate each public fraction before consuming it. For denominator factors d1=g*l
and d2=g*r, gcd(l,r)=1. Canonical inputs imply gcd(n1,d1)=gcd(n2,d2)=1, so neither
l nor r can divide t=n1*r+n2*l. Only shared factor g can cancel. Thus h=gcd(t,g)
gives the reduced result t/h over l*(d2/h). For g>=2 the64bit cross-sum bound is
safe. For g=1, if d1*d2 fits32bits then d1+d2<=UINT32_MAX+1 and the numerator sum
fits64bits. An overflowed sum therefore has an unrepresentable uncancellable
denominator, so the improved32bit API does not reject a representable result
because of that guard. The naive policy has a different admission contract.

The signed representation keeps the denominator positive/unsigned. Converting a
negative numerator to uint32_t then subtracting from unsigned0 obtains magnitude
without signed overflow. Equal-sign scaled terms total at most2^64-2^32; unequal
signs subtract magnitudes. Positive magnitude2^31 is refused while negative2^31
is spelled INT32_MIN directly. Reusing an unsigned conversion of a negative value
for signed division would silently change the result and must fail review.

The opaque object is an actual incomplete type with a private heap lifetime,
not a public struct renamed safe. Empty output is mandatory before make. It
validates before allocating, publishes once, and empties before matching release.
The host allocator callbacks are a separately declared raw-storage comparison,
not evidence of using the T66 typed-cell allocator. A private header is not a
defense against a forged pointer, stale alias or arbitrary memory corruption.

R1-R4 require the precise proofs, all output/refusal fields, finite oracle evidence,
negative client translation and changed signed consumer. Fraction oracle uses
unbounded mathematics for the final result, not this C cancellation procedure.
Independent learner performance and hardware/compiler-helper behavior on other
targets remain separate evidence.

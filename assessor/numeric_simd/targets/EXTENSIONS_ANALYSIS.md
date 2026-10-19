# T64 continuation assessor analysis

The Q3.12 product reaches2^30, safely inside int32. Its toward-zero quotient can
reach262144, outside int16, so overflow policy applies after rescaling. For a
4096-term raw dot product, the maximum magnitude is at most2^42, inside int64.
Error checks must avoid evaluating overflowing signed addition. The explicit
wrap interpretation subtracts65536 from values with the high bit set in a wider
signed type, then converts the representable result.

Four partial accumulators are an exact integer reassociation within the proved
bound; a floating reduction cannot inherit that proof. Midpoint recursion has
logarithmic call depth but still rounds at every node. The two permutations in
N5 give distinct answers despite the same exact real sum. An arbitrary epsilon
that accepts both is not an accuracy argument.

For rounding, half an ulp at1 is2^-53. Upward rounds the positive sum to the next
binary64 number; downward rounds the negative sum to the next more-negative
number. The other directed outcomes and ties-to-even yield +/-1. Integer casts
truncate independently of the current arithmetic rounding direction. The FTZ
probe initially exposed a GCC optimization issue: constant operands allowed
compile-time multiplication despite volatile function parameters. Runtime
volatile operands force the measured operation between MXCSR changes. Retain
the generated multiplication and restore all original control bits.

Matrix transposition changes loads, not k order. The3x3 block bounds must clamp
both dimensions; a full-tile-only loop drops edge cells and is a semantic mutant.
The full input scan and transpose belong in equal-contract timing.8MiB eviction
does not establish a fully cold LLC on an arbitrary machine. Code-size and stack
costs remain part of the learner dossier even if one timing is faster.

The author runner kills ignored-error, dropped-dot-tail and omitted-edge-tile
mutants. Independent Python arithmetic validates integer cases only; it does not
prove floating accuracy for the admitted continuum. Level3 requires original
changed-condition reasoning, not reproduction of the author campaign. No hidden
analysis should be copied into a learner export.

The SSE2 continuation reconstructs each signed32 product from the low and signed
high16 halves, then sign extends to64 before addition. INT16_MIN squared is
positive2^30; treating multiply-low as the whole product would produce zero.
The two64-bit vector partials remain within the4096-term bound. Actual unaligned
loads are compiler intrinsics under the selected x86-64 contract, not ISO C
permission to dereference an arbitrarily aligned vector pointer.

The two-worker continuation preserves the exact midpoint partition. A different
completion order changes neither the subtree operands nor the final addition.
Worker result storage remains live until both joins; a failed second creation
still joins the first. Thread launch costs dominate small workloads and belong
in the recorded end-to-end timing. Numerical ulp distance orders finite binary64
encodings around a collapsed zero. It intentionally does not order NaN payloads
or claim that one ulp is always an acceptable domain error. Subnormal and
ill-conditioned examples distinguish distance, forward error and bit identity.

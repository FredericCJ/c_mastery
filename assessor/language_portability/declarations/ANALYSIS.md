# Assessor technical analysis: six declaration tasks

The learner contract contains exact independent output rules; keep this implementation and analysis out of exports.

## T05.07.a and delayed VLA

N3096 6.7.2.5p4-5 preserves all qualifiers for typeof and removes atomic/top qualification for typeof_unqual. The pointer-to-declared-object generic check distinguishes const atomic int from int. Type inference6.7.9p2 applies lvalue/array/function conversion: auto from the array yields pointer-to-const-int, whereas typeof preserves the array. nullptr conversions are limited by6.3.2.3p3 and6.3.2.4, not universal zero substitution.

The delayed operand `(calls++,p)` has pointer-to-VLA type, which is variably modified; its side effect must occur. In the rejected initial pilot `(calls++,*p)`, the right array operand undergoes conversion inside the comma expression, producing a plain element pointer. Thus the observed non-evaluation was explained by the source, not a compiler bug. The ordinary int typeof operand remains unevaluated. A false expected count0 normal exit1 is the falsification control.

## T05.07.b and delayed public boundary

N3096 6.7.2p4 sets unsigned minimum1 and signed minimum2, both bounded by BITINT_MAXWIDTH. The enum rules are separately6.7.2.2p2-5,11-12; they do not permit bit-precise fixed underlying types. The selected 2^32 enumerator fits unsigned long long, but neither its accepted source nor the enum object's size proves an ABI for all compilers.

For encode, v+8192 for negative v is4096..8191, all int32 operations safe. For decode, byte preflight establishes u<=8191 and subtraction u-8192 stays-4096..-1. No implementation-defined out-of-range signed conversion occurs. High-bit acceptance is a real canonicality defect even when valid round trips pass. The C23 provider converts signed13 to unsigned13 modulo8192, then exports only standard-type values/bytes. Opposite-compiler linking is limited to this actual host ABI; never infer `_BitInt` interchangeability from it.

## T05.07.c and delayed generated constants

A constexpr object must satisfy6.7.1p5-6; explicit pointer initializers must be null. The compiler rejects the function spelling, but N3096 6.7p12's underspecified non-object declaration wording is undefined behavior, not the pointer constraint. This distinction prevents turning a useful diagnostic into an overstated mandatory safety guarantee. An ordinary function call is not an integer constant expression for a static assertion.

The runtime triangle proof factors an even member of consecutive n,n+1 before multiplication. For n<=65535 the result<=2147450880, within uint32_t. Output is not written before the bound. Python generates table constants by summation, independently of the C factoring algorithm; static assertions for17 and31 reject an altered generated constant. Runtime agreement plus independent full bounded oracle is meaningful but not a proof of all source transformations or all C implementations.

## Assessment boundaries

Require D1 exact categories and source identity as well as D2 executable output. A copied compiled result without a conversion/range explanation earns at most1. Score3 requires independently derived delayed reasoning and changed-condition evidence. Initial pilot observations are disclosed separately from final source-bound companion evidence. No physical, human-independent or learner mastery claim follows from author validation.

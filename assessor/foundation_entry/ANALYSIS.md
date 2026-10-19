# PR-T01-01 assessor analysis

Keep this directory out of learner exports. The public contract defines the
mechanism, while this file supplies criterion-specific solutions and discriminators.
Assess each exact original AX task and its delayed variant separately. Do not
substitute an overall test pass for the row's explanation.

## t0101a

Iterative prefix states `(processed,total)` are `(0,0)`, `(1,3)`, `(2,2)`,
`(3,6)`; publication occurs after validation. Recursive entries are indices
0,1,2,3; returns are `(3,0),(2,4),(1,3),(0,6)`. There are four maximum live
activations and eight events. Empty input enters and leaves base index zero,
giving one activation and two events, not zero calls. At length sixteen there
are seventeen activations and 34 events. Every explicit stack record holds a
pending index; popping the base record emits return zero, then each pop performs
the checked addition and corresponds to one return. Induct on remaining records.
Omitting the base return is a compiling semantic mutant rejected by event count.
An optimized compiler may inline or remove actual calls; these counts describe
the algorithmic activation convention, not a measured ABI stack depth.

## t0101b

For successful size n the fixed prefix makes n checked additions and n stage
writes followed by n publication writes. Its declared stage is sixteen int32
objects irrespective of n; caller output is not auxiliary storage. The scalar
two-pass form uses constant work space and 2n additions, excluding the supplied
trace or outputs. Staging uses n temporary objects, one acquire/release pair and
linear work; allocator time is additional. Recursive suffix sum performs n
additions and has n+1 abstract activations. Explicit records are bounded seventeen
objects in this implementation. All actual n are bounded in the fixed problem;
O(n) describes the family parameter and operation count, not an assertion that
unbounded inputs are admitted. Do not report the trace's 34 records as recursion
stack usage.

Two-pass failure is atomic because no output is touched in validation. Exact
in-place publication overwrites only the element just consumed; disjoint arrays
also preserve future inputs. With `out=in+1` on `[3,-1,4,...]`, the first write
destroys `-1`, invalidating the second pass's proof. Reject that *contract*, not
an arbitrary pointer ordering test (relational comparison of unrelated objects
is not a general overlap detector). Staging can support that overlap because
all inputs have been consumed before publication. Its temporary array must be
disjoint and released once, even on the second addition's range failure.
Mutants publish early or omit that release; separate assertions distinguish them.
An allocator that never succeeds does not validate the successful algorithm.

## t0101c

At iteration k, `0 <= k <= n`, the stage contains exactly the k representable
prefix sums, the accumulator is the last such sum (zero initially), and output
is unchanged. For x>0, checking `sum <= MAX-x` avoids evaluating overflowing
`sum+x`; for x<0 use `sum >= MIN-x`. The bound subtractions are representable
under those sign conditions. Successful addition preserves the invariant and
k increases; measure n-k decreases. At exit publish all n values. `[MAX,1,-1]`
fails at index one although its mathematical final sum is MAX. A check *after*
signed overflow is not a repair. The 343 direct-formula corpus independently
checks admitted intermediates but does not replace induction.

For maximum, empty returns EMPTY preserving output. Nonempty initializes with
the first input, not zero. At index k in `[1,n]`, max is greatest in `[0,k)`;
compare the next element to preserve it, and use n-k for termination. This needs
no addition and no n-element stage: defer only the one result write until exit.
Singleton MIN must return MIN. The empty-write mutant is rejected even though
all nonempty results are correct. Summation's neutral zero is not an element of
an empty maximum result under this total tagged contract.

## t0102a

173 = 128+32+8+4+1 = binary 10101101 = hexadecimal ad. A two-octet big-endian
encoding is `[00,ad]`, little-endian `[ad,00]`; the mathematical value is equal.
For three octets use weights 65536,256,1. `0x1234ad` is `[12,34,ad]`, whereas
copying the first three object bytes is host-dependent. Admission precedes writes;
the fourth sentinel remains untouched. Casting a shifted nonnegative value to
unsigned char selects its low eight bits in this explicit CHAR_BIT=8 profile.
The little-endian mutant produces distinct bytes on a defined operation and is
rejected. Reasoning about a changed host endian suffices for this codec task;
no physical big-endian machine execution is claimed by the hosted campaign.

## t0102b

For mask12 the selected set is bits2,3. Values0,4,8,12 give any=false,true,true,true
and all=false,false,false,true. A forbidden-mask predicate gives true,false,false,
false for the same values. Empty mask yields any=false, all=true, forbidden-clear=true:
universal truth over an empty set is not evidence of granted permissions. The
oracle counts present selected bits using division/remainder, rather than copying
`bits & mask`. The mutant substituting all for forbidden-clear fails immediately
on a selected absent bit. Require the learner to explain policy composition:
“required all present AND forbidden none present” can be unsatisfiable if the
sets intersect. Zero masks alone cannot demonstrate that a nonempty policy works.

## t0102c

Unsigned32 MAX+1 gives zero modulo 2^32; the C signed expression MAX+1 has
undefined behavior, not mandated wrapping or saturation. The reference never
executes it. Saturation changes the algebra: `(MAX sat+ 1) sat+ (-1)=MAX-1`,
whereas `MAX sat+ (1 sat+ (-1))=MAX`. Thus reassociation is not a valid
optimization for this specified operation. Prefix and suffix *checked* sums also
have different admission domains: `[MAX,1,-1]` has a valid suffix evaluation
but invalid forward prefix; `[MIN,-1,1]` analogously contrasts negative boundaries.
An answer that claims finite bit patterns alone determine language rules fails.

## Alternative solutions and assessment limits

A different explicit stack representation is valid if it establishes the same
activation bijection, limits and trace. A supplied allocator may be a pool or
heap adapter if its object/ownership contract is met; do not require malloc.
Different overflow predicates are valid only if every intermediate expression
is representable. The implementation may not depend on int64 for its checks;
the hosted independent oracle may. A memory scan, cast or compiler intrinsic is
not a substitute for the specified derivations.

The harness rejects six compiled property-violating implementations; these are
not negative translation fixtures. Source inspection is required for forbidden
dependencies, ownership assumptions and proof quality. ASan/UBSan execution is
bounded evidence under the named compiler, not proof of definedness for all input.
No non-author review, actual learner attempt, delayed evidence or physical result
is produced by running this assessor package.

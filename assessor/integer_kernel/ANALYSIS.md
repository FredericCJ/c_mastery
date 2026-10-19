# Integer kernel assessor analysis

Keep this directory out of learner packages. `integer.c` is the reference, not a
completed learner template. `oracle.c` uses exact small-range mathematics and a
quotient search by identity/remainder predicate; `models.py` generates native
boundary expectations with Python arbitrary integers. Neither duplicates the
reference's threshold branches. `comparisons.c` imports the unchanged actual
foundation-depth provider and exercises actual builtin/C23 primitives. Those
comparisons are differential evidence, not independent proof of either provider.

## t0401a

The observed fundamental table must include plain/signed/unsigned char,
short/int/long/long-long and their unsigned counterparts, sizeof units and
CHAR_BIT. An unsigned maximum or signed maximum decides whether100000 fits;
sizeof*CHAR_BIT is storage bits and may include sign/padding. On the explicit
int16/long32 model, decimal100000 and65535 have type long; hexadecimal0xffff
selects unsigned int. On the actual int32 observations all three have type int.
Do not infer that every16-bit-int system has8-bit characters. Reject a table
inferred only from familiar LP64 sizes. A complete derivation considers the
ordered literal-type candidate lists in N1570 §6.4.4.1, not destination type.

## t0401b

Allocation sizes use size_t, local signed same-array differences ptrdiff_t,
widest signed input intmax_t. A uint_least32_t carries the wire value, with an
explicit0xffffffff upper bound if it has more value bits. uint32_t is optional.
Reject negative signed count; reject an integer difference outside PTRDIFF range
before any attempted conversion (do not create an out-of-range actual pointer
subtraction); reject a source value outside intmax at the parser boundary rather
than pretend it is representable as an intmax argument; reject wire high bits or
cell256. The interface itself cannot receive an unrepresentable intmax value.
Learner must separate these boundaries rather than manufacture invalid arguments.

The provided four-element local array verifies negative local difference and
logical-base>PTRDIFF_MAX translation. `position-base` is evaluated only after
ordering; a successful offset is at most extent and therefore representable in
size_t. uintmax_t covers size_t's unsigned range. This says nothing about the
liveness of a pointer or an actual file of that size. One-past is not dereference.

## t0401c

0x12345678 produces values18,52,86,120. Four unsigned cells can hold these on
CHAR_BIT16, consuming four C bytes=64bits if unsigned int is also16bits, or more
if unsigned int has more storage. The explicit model uses **four character
cells**, not the native `unsigned int[4]` allocation; the latter's storage is
`4*sizeof(unsigned int)*CHAR_BIT`. A packed16-bit-character alternative consumes
two C bytes=32bits but each stream character is16bits, not an octet. File/transport
mapping needs its own contract; explicit shifts encode value order independent
of host address endian. Correctly flag the CHAR_BIT8 adapter as UNSUPPORTED on
wider-character models; do not redefine CHAR_BIT during compilation to fake a
target. All actual executions here have CHAR_BIT8.

## t0402a

With uchar0..255, int16 and int32 both promote to int. x+1 is256. Under the stated
two's-complement representation ~x is-256; without that premise N1570's signed
representation choices prevent a universal answer. x<<8 is65280 mathematically;
int16 signed result is unrepresentable and UB, while int32 is defined. No golden
UB result is assigned. ushort0..65535 cannot fit int16 and promotes unsigned int:
65535+1 wraps0, complement0, left shift8 gives65280 modulo65536. An unsigned-int
8-bit bit-field promotes to int; a full32-bit unsigned-int bit-field on int32
promotes unsigned int. Width-restricted representable range drives promotion.

## t0402b

For -1 and1u, equal rank selects unsigned int, converting-1 to UINT_MAX; comparison
false. Native negative int versus size_t similarly loses its sign under the
recorded rank/range. Reject negative before conversion, then compare in an
unsigned type covering the destination range. Higher-rank signed long long that
contains all uint values converts uint to signed: -1<1 becomes true. “Wider” must
be justified by rank and range, not just sizeof. The observations verify the
recorded model; the learner's rule proof must address other models separately.

## t0402c

`fi_to_size` tests negativity before uintmax conversion, then compares to the
size_t ceiling represented in uintmax. Only publish after success. On native
intmax64/size64, SIZE_MAX and one-above cannot be supplied, explicitly recorded
as two unavailable source values. M4 intmax64/size32 executes both. Ceiling65535
accepts65535/rejects65536. Prefix12,count18,width9 yields174≤180; count19 yields183
and rejects. Prefix1 with zero count/width and limit0 must reject. A zero width
does not waive count representability in the composed layout contract.

## t0403a

Native int always represents130 under minimum ISO limits. Synthetic8-bit signed
range rejects120+10 unchanged. Unsigned8 arithmetic model yields130 (this example
does not wrap);250+10 demonstrates4 modulo256. Saturating signed8 returns127 and
clipped1 with success, changing the postcondition rather than “fixing overflow”.
Clipping may conceal upstream range errors if its flag is ignored. Reject inputs
already outside the selected range, even if their mathematical result would fit.

## t0403b

For addition b>0 compare a>high-b, b<0 compare a<low-b. Given admitted operands
and low≤0≤high, both threshold subtractions fit native int. Subtraction uses
high+b when b<0 and low+b when b>0; it never negates b. For multiplication:
a>0,b>0 rejects a>high/b; a>0,b≤0 rejects b<low/a;
a≤0,b>0 rejects a<low/b; a<0,b≤0 rejects b<high/a. Handle a=0 first in the final
branch. C truncation makes these thresholds the required integer floor/ceiling
bounds; check inequalities with a negative divisor carefully. high/a cannot be
INT_MIN/-1 because high≥0. Other divisors are positive. All final evaluations
occur only after mathematical admission. Native INT_MIN*1 and *0 succeed;
INT_MIN*-1 rejects on asymmetric representation.

Extent first checks prefix≤limit, then width==0 or count≤(limit-prefix)/width.
The subtraction is safe, division has a positive divisor, and accepted product
plus prefix is at most limit≤SIZE_MAX. Treating zero width as unconditional success
is the deliberate mutant. Reduced-range oracle covers[-128,127],[-127,127],[-12,3]
with392451 cases for each operation family, including every8-bit operand pair.
Native oracle contains972 arithmetic and972 division cases around actual int32
extrema and product thresholds. A proof still matters outside these finite sets.

## t0403c

GCC's generic overflow primitives compute as if infinite signed precision then
convert to the pointed destination type. They store a result even when reporting
overflow. Stage a temporary, reject overflow/range failure, then publish. Narrow
signed-char destination can overflow although both operands and exact result fit
int. Mixed -1 and1u have mathematical product-1 for the primitive with signed-char
destination; ordinary C multiplication converts to unsigned and yields UINT_MAX.
The actual C23 experiment requires both edition and stdckdint header version
202311, uses ckd_add/sub/mul, and is not the C17 implementation. N3096 is a draft
comparison source. FI_BUILTIN_EARLY compiles, executes and fails unchanged-output
assertions; a compiler error would not count as killing this semantic mutant.

## t0404a

Truncating pairs are(-2,-1),(-2,1),(2,-1),(2,1). Show a=q*b+r over mathematical
integers, then justify evaluation in C for these bounded inputs. N617's “from”
text reproduces older C89/C90 rules: with a negative operand and inexact quotient,
floor or ceiling direction is implementation-defined and remainder sign follows
the identity. For(-7,3),(-3,2) and(-2,-1) are the alternatives. N617 proposes
the toward-zero rule later present in N1570. Do not relabel this inspected
proposal as the complete published C90 standard or claim an actual C90 runtime.

## t0404b

Zero is rejected before both operators. An asymmetric native minimum/-1 quotient
is unrepresentable even when the apparent remainder is mathematically zero;
both C operations are UB if evaluated. Test `(INT_MIN < -INT_MAX)` before claiming
the representation has that asymmetric edge. Symmetric[-127,127] allows-127/-1=127;
its host representation remains unchanged. Selected-range rejection is separate
from native UB. Both output sentinels must survive rejection and identical output
objects must be rejected. No universal signal/trap observation replaces admission.

## t0404c

Truncation yields r with dividend sign. For positive m, if r<0, r+m is in[1,m-1]
and safe; otherwise return r directly. `(r+m)%m` can overflow for large positive r.
For FLOOR with nonzero remainder of opposite sign to b, decrement q and add b to
r. Nonzero remainder excludes an extreme quotient needing an unsafe adjustment;
the remainder sum has opposite signs. For EUCLIDEAN with r<0, b>0 uses that
adjustment; b<0 increments q and subtracts b, giving0≤r<|b| mathematically.
Even b=INT_MIN is supported without computing abs(b): -1/INT_MIN has Euclidean
q=1,r=INT_MAX on asymmetric int. The C intermediate q*b may be INT_MIN and
representable here; other identity checks need their own proof. Quotient search
in the reduced oracle independently requires identity, strict remainder bound
and convention sign. Do not reuse the implementation's adjustment formula as
the only oracle.

## Rubric, alternatives and evidence

Score **each ordinary and changed-condition criterion separately**:
0 wrong/absent;1 compiles or recites a rule without premises;2 correct ordinary
result and reason but misses a relevant boundary;3 correct implementation,
invariants, independent evidence and limitations;4 independently derives the
changed assumptions and defends a counterexample during oral/source review.
Require≥3 on every applicable technical criterion, all failure-publication and
undefined-evaluation gates, and independent transfer evidence before awarding
level4. Reference execution today cannot award a learner any score or satisfy
the seven-day interval. Reject fabricated int16/CHAR16 execution, hidden runtime
archives, UB goldens and claimed C17 stdckdint support.

Valid alternatives include sign/magnitude magnitude bounds with careful unsigned
conversion, division thresholds arranged differently, or explicit admission
tables for a small bounded application. The primary general API may not implement
itself with a wider type/builtin; comparison backends may. Branchless claims need
actual code and resource/correctness-equivalent evidence. Performance conclusions
must include instruction/helper selection and target model; the supplied stack
audit bounds selected nonrecursive IRQ-masked driver frames plus1024 reserve,
not an arbitrary client's stack or physical watermark.

The six-profile campaign executes five defined C mutants plus a public unfinished
template, separate builtin early-publication mutations, actual qualification-free
native observations, and four direct-linked M4 images. Every target image and
object is inspected. The host harness/sanitizers are outside target runtime
closure. External review, physical measurements and learner attempts remain
unperformed; passed author runs establish only their recorded finite claims.

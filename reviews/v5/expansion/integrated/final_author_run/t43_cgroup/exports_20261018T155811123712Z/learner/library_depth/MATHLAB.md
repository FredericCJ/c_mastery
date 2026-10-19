# T12.13: math contracts, floating state and independent accuracy evidence

Required PR-T12-05 criteria T12.13a/b/c and their original seven-day variants;
families A/B/C/D/F/H/I/J. Prerequisites: floating representation and rounding,
function-pointer types, errno, value versus object representation, and the
learner's ordinary library notebook. Implement mathlab.c against mathlab.h.
The supplied tests and exact binary input fixture are public; reference
implementations, high-precision checker and transfer analysis remain assessor-only.
Read R-LLIB-N1570-MATH, R-LLIB-MPFR422, R-LLIB-GCC15-MATH,
R-LLIB-GLIBC241-MATH and R-LLIB-MUSL125-ACOS in the source register. C17 is
the implementation baseline; the cited WG14 text is explicitly the C11 draft.

Start by deriving a table of header, argument domain, return type and error
contract for sqrt, csqrt, the tgmath sqrt macro and the fenv operations used
below. The scalar sqrt argument must be nonnegative to avoid its domain error;
complex square root has a different domain and branch convention. A real
negative input does not become complex merely because complex.h was included.
The type-generic macro dispatches by types: an integer generic argument acts as
double, float selects float, and long double complex selects long double
complex. Use _Generic assertions and inspect undefined symbols/relocations at
-O0 with builtins disabled; do not infer the call from spelling alone. Execute
the real and complex calls separately, including csqrt(-1+0i). A compiler may
otherwise fold a call or use an instruction, so lack of a relocation is not
proof that the language selected a different result type.

Implement lm_observe as a synchronous borrowed callback adapter. The callback
has exactly double(double), returns normally, does not retain output storage,
and follows the selected standard-math runtime contract. out is a live writable
object, disjoint from callback state. Null function/output is ARGUMENT, leaving
out unchanged. On supported profiles save errno and the floating environment,
clear the exception flags and errno, record the current rounding mode and
math_errhandling, call once, collect value/errno/flags, and restore the caller's
environment and errno before publishing the complete sample. OK means an
observation was collected, even for a mathematical domain or range error.

ENVIRONMENT preserves out and restores errno; try to restore the saved floating
environment after a later fenv failure. If restoration itself fails, the caller
must treat its floating state as unknown. No generic C wrapper can guarantee
restoration after a failing platform restoration operation, a signal-induced
nonlocal jump or an arbitrary callback that does not return. The selected host
campaign exercises successful fenv operations, not a synthetic claim that every
hardware failure is recoverable. Storage is constant; wrapper work is constant
plus the callback's implementation-dependent cost. It performs no allocation.

Use sqrt(4), sqrt(-1), exp(1000) and exp(-1000), with binary64/round-to-nearest
premises checked at runtime. Preserve a pre-existing FE_DIVBYZERO flag and
E2BIG errno while collecting a fresh result. Repeat sqrt(4) with FE_DOWNWARD
and require that the caller's mode remains downward. For domain and overflow,
require only channels advertised by math_errhandling. Underflow detection and
reporting have additional discretion in the cited core clauses: record the
value, errno and flags; do not mechanically require the overflow rule. Absence
of a newly raised exception does not make an arbitrary function accurate.

The changed optimization profile is substantive. GCC/Clang fast-math defines
assumptions incompatible with this notebook, including finite-value/errno or
rounding assumptions. This adapter explicitly returns UNSUPPORTED before
calling or publishing under the detected fast profile. Record the actual
compiler flags and macro evidence. Detection of these macros is a bounded
profile policy, not a universal detector for every unsafe compiler option.
Even GCC's -frounding-math documentation describes limits; the runtime tests
qualify this build, not every transformed expression or compiler version.

For accuracy use exactly represented binary64 inputs, hex output, and
FE_TONEAREST. The sqrt inputs are +0, 2^-1074, 2^-1022, 2^-100, the immediate
neighbors below/above1,1,2 and DBL_MAX. The acos inputs are -1, its next value
toward0,-0.5,0,0.5,the three binary64 predecessors of1,and1. Explain why the
acos derivative -1/sqrt(1-x*x) makes near1 a sensitive region, while relative
error at an exact zero needs a convention. Input conversion error is zero:
the host oracle receives the exact hexadecimal double, not a rounded decimal
approximation to a different mathematical input.

The independent host checker uses MPFR at512 bits with directed RNDD/RNDU
results. Both enclosure endpoints must convert to the same nearest binary64
value; otherwise increase precision or declare ambiguity, never invent a
golden result. Report absolute error, relative error and error divided by the
spacing from that nearest result to its next larger double. These printed
metrics use a512-bit rounded reference and are approximations. Relative error
is0 when both reference/error are0, infinity if reference0 and error nonzero.
Also report integer representable-step distance, which is a different measure
from that local spacing metric near a binade boundary. MPFR/GMP are host
verification dependencies and must never enter strict target runtime closure.

Compare native GCC/Clang, actually loaded glibc2.41 libm and a static musl1.2.5
runner. Retain loader/static-link evidence and library hashes. The finite gate
requires exact nearest sqrt values and at most four representable steps for
acos. The latter is an explicitly chosen application screen for these eighteen
inputs, not an ISO, scalar glibc, or musl universal accuracy promise. The glibc
manual's separate vector-function4-ULP text is not a scalar acos contract.
When moving to a library without a correct-rounding promise, its public
contract alone does not justify inventing a numerical ULP bound. Identify that
missing guarantee; justify any application tolerance separately and qualify it
against the retained independent measurements. A justified refusal to assert
an unsupported all-input bound is required for full credit.

Deliver the type/domain table, implementation, state restoration argument,
flags/runtime manifest, exact eighteen-row data, error analysis and changed
library/optimization conclusions. Six native profiles must pass; three compiled
semantic mutants and the unsolved scaffold must fail at their intended checks.
The checker must reject a forged sqrt result, duplicate/missing inputs and
trailing malformed fields. Tests do not prove accuracy for all inputs.

Rubric:0 incorrect types/state or unsupported claims;1 reproduces values only;
2 explains dispatch, advertised channels, enclosure and error conventions with
working ordinary evidence;3 independently handles the original integer/complex,
fast-profile and weaker-library changes after at least7days. Hints: distinguish
the operation's result from observer success; clear before calling and restore
before publishing; an interval must settle the output rounding boundary.
External review, delayed learner performance and physical observations remain
unexecuted dimensions, separately from the author campaign.

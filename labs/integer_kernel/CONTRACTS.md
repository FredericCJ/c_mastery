# Integer admission, representation and division

This is the executable successor for the twelve retained criteria in PR-T04-01
and their twelve original seven-day variants. The authoritative IDs and task text
remain in `ASSESSMENT.json`; suffixes below refer to T04 objectives. Families are
semantic reasoning, derivation, boundaries, diagnosis, verification, portability
and composition. The mechanisms have different evidence: an executable arithmetic
oracle cannot assess a claim about the type of an unexecuted expression.

Prerequisites: T01 build/diagnostic dossier, T02 interfaces and sequencing, T03
object lifetime and same-array pointer arithmetic; learner artifacts are their
checked-conversion/extent functions and a reproducible compiler manifest. Reuse
the previously implemented `fd_extent` and `fd_from_signed` through their actual
`labs/foundation_depth/include/depth.h` contract in the hosted comparison. A missing
learner prerequisite is recorded as missing; an assessor reference is not learner
work. That prior provider requires CHAR_BIT=8; the portable cell codec does not.

The implementation baseline is ISO C17, no allocation, no library arithmetic
routine, no overflow builtin in `learner.c`, no wider arithmetic type to implement
`fi_binary`, `fi_saturate` or `fi_divide`. `<stddef.h>`, `<stdint.h>` and `<limits.h>`
provide types and limits. GCC builtins and C23 `<stdckdint.h>` are separately named
comparison backends. A capability failure is a failed/unavailable comparison,
never permission to substitute the C17 implementation silently. Host Python,
stdio, sanitizers and arbitrary-precision oracles are verification dependencies,
not dependencies of the target component.

## Representation and API

Implement every declaration in `integer.h`. `FiRange` represents a closed subset
of the native `int` range containing zero (`low<=0<=high`), not a new C integer
type. `FiOperation` and `FiDivision` must be admitted before use. Caller-owned
outputs must be live, correctly aligned and writable; output pairs designate
distinct nonoverlapping objects. Null or identical paired outputs return INVALID.
The API cannot establish arbitrary pointer validity from numeric range checks.
Inputs are borrowed for one call; no pointer is retained, transferred or freed.

| Operation | Success | Rejection and publication |
| --- | --- | --- |
| `fi_binary` | exact add/subtract/product inside selected range | operands outside range or mathematical result outside range: RANGE; unchanged output |
| `fi_saturate` | exact result or nearest endpoint, `clipped` precisely 0/1 | invalid/range-excluded operands reject; arithmetic result overflow clips successfully; both outputs publish together |
| `fi_divide` | quotient/remainder under explicit convention, both in selected range | divisor zero or unrepresentable native/selected truncating quotient reject before either `/` or `%`; all rejection preserves both outputs |
| `fi_mod_positive` | least nonnegative residue for strictly positive modulus | nonpositive modulus INVALID; unchanged output |
| `fi_to_size` | nonnegative `intmax_t` value at most `ceiling` | reject before unsigned conversion; unchanged output |
| `fi_extent` | exact `prefix+count*width<=limit` | check prefix even when count/width is zero; no intermediate wrap; unchanged output |
| `fi_layout` | compose conversion with extent admission | count must fit `size_t` even when width is zero; no partial publication |
| `fi_window` | integer logical position maps to offset in `[0,extent]` | reject position before base or beyond extent; offset equal to extent is one-past, not a dereference license |
| `fi_encode_cells` | four most-significant-first logical octet values | wire above 0xffffffff (when representable) rejects; no partial output |
| `fi_decode_cells` | reconstruct value from four cells in 0..255 | any larger cell rejects before publication |
| `fi_encode_octets` | four actual C characters only when CHAR_BIT=8 | UNSUPPORTED preserves output on wider-character implementations; wire range still checked |

All scalar algorithms use O(1) space/time under a fixed-width arithmetic-cost
model, with four cell operations for a wire value. Hardware division cost,
compiler helper cost, code size and stack use are separate measured properties;
O(1) is not a constant-latency guarantee. There is no allocation failure path to
invent. The failure model is admission/publication, not transactional hardware I/O.

`fi_divide` policies: TRUNCATE removes fractional magnitude toward zero;
FLOOR rounds quotient toward negative infinity and nonzero remainder has divisor
sign; EUCLIDEAN selects nonnegative remainder smaller than divisor magnitude.
Do not form `abs(INT_MIN)`. A symmetric synthetic range does not change native
representation. State the algebraic identity over mathematical integers before
deciding whether a C expression checking it has representable intermediates.

## Guided construction and independent tasks

Begin with a type/limit table and a worked small-range calculation. For example,
for an admitted nonnegative width, first reserve `prefix` from `limit`; only then
ask how many whole widths fit. This derivation is a proof obligation for every
guard expression as well as the final multiplication. Diagnose the tempting
`prefix+count*width > limit` check: its own unsigned wrap can invalidate the test.
Implement ordinary admission before adding clipping and alternative division.

| Objective | Concrete ordinary deliverable | Changed-condition deliverable |
| --- | --- | --- |
| .01.a | actual `CHAR_BIT`, `sizeof`, signed/unsigned limits for all fundamental integer types; decide which can represent every count 0..100000 from limits | explicit int16/long32 model; derive types of 100000,65535,0xffff and compare with actual native `_Generic` observations |
| .01.b | map allocation count to `size_t`, local difference to `ptrdiff_t`, signed external integer to `intmax_t`, wire value to a range-checked at-least32 unsigned type; provide rejected boundary for each role | represent base greater than PTRDIFF_MAX using `uintmax_t`, map only a four-element local window; reject before-base/outside-window offsets; never synthesize a huge pointer |
| .01.c | capability-gated octet encoder and explicit mask/shift cell codec; independently decode 0x12345678 and reject cell256 | CHAR_BIT16 model: four logical-octet cells versus two packed 16-bit characters, storage size and stream-character units; specify transport rather than assuming two C bytes are two octets |
| .02.a | annotate promotions/result types of `unsigned char x=255; x+1; ~x; x<<8` under int16 and int32, with signed-representation premises; execute only admitted cases | an 8-bit unsigned-int bit-field, full-width unsigned-int bit-field, and ushort65535 on int16: derive promotion from range, not the word “unsigned” |
| .02.b | derive `-1<1u` and negative signed length against size capacity; implement negative rejection before conversion | choose a higher-rank signed type containing all unsigned values; recompute comparison; show why “mixed always unsigned” is false |
| .02.c | sentinels for -1,0,SIZE_MAX,one-above only when source represents them; report unavailable values explicitly | limit count to65535 and compose prefix12/count18 or19/width9/limit180; zero-width and prefix-limit cases |
| .03.a | mathematical120+10=130, synthetic[-128,127] rejection, native int success, explicitly reduced unsigned8 model | saturate to endpoint and report clipping; derive how the success postcondition differs and whether composition still checks errors |
| .03.b | derive all sign partitions for add/sub/mul and extent; enumerate all 256² pairs with exact independent expected results | no wider arithmetic implementation; prove each division threshold is representable, include zero operands and asymmetric/symmetric ranges |
| .03.c | wrap all three GCC builtins with temporary output and compare manual contracts; separately capability-check actual C23 header | keep `int` operands, use `signed char` destination, expose primitive result publication on overflow and mixed signed/unsigned infinite-precision semantics |
| .04.a | derive quotient/remainder for (-7,3),(7,-3),(-7,-3),(7,3); explain safe mathematical identity | inspect the older rule reproduced in N617, show both permitted negative-inexact choices, distinguish proposal from published standard |
| .04.b | reject zero and unrepresentable minimum/-1 before both operators, preserve sentinels | symmetric[-127,127] permits -127/-1; native asymmetric minimum still cannot be negated/divided by-1 |
| .04.c | normalize positive modulus without `(r+m)%m`; contrast floor quotient and truncation for each sign | accept negative divisor using explicit FLOOR versus EUCLIDEAN conventions; include `a=-1,b=INT_MIN` without forming positive magnitude in `int` |

Each row needs a criterion-indexed dossier containing premises, predicted values
and types, invariant/guard derivation, an incorrect explanation and counterexample,
source clause/version, actual command/result or explicit analysis-only status.
The retained assessment card supplies the stable ordinary/delayed IDs. Each row's
changed-condition requirement is implemented and assessed separately; a passing
ordinary row is not delayed learner evidence.

## Verification and submission

Supply `learner.c`, your dossier, independent oracle, command manifest, emitted
objects/maps and resource analysis. Run the public contract from the exported
directory with `python run.py --cc gcc --source learner.c --runtime <your original
memory-helper.c> --build <new-directory>` (use full relative paths when outside
this directory). Repeat Clang, O0/O3 and hosted sanitizers in your manifest.
The public runner deliberately requires the explicit runtime argument. It does
not find assessor implementations. An unfinished template must compile and fail
the component assertions; completing it is the learner's assignment.

For M4, `target_build.py --build <new> --source learner.c --runtime <original
memory-helper.c> --driver <your semihosting driver.c> --cc arm-none-eabi-gcc --opt 0`
builds four explicit objects with the supplied startup/link contract. Repeat opt3
and Clang. Execute with QEMU mps2-an386/cortex-m4 and semihosting. The selected ABI
has int32,size32,intmax64; this permits real SIZE_MAX and one-above conversions.
Inspect every object's undefined symbols, linked undefined set, map archive
members, stack-usage records and disassembly. Record original helpers explicitly;
successful linking alone is not closure compliance. Never add libc/libgcc/
compiler-rt/libatomic archives to satisfy an unexplained symbol.

Pass requires correct ordinary and transfer reasoning, all public/withheld
contract cases, independent expected results, unchanged sentinels, actual
compiled mutants rejected for the stated property, explicit runtime closure and
honest environment limits. Compile-only undefined specimens receive no execution
golden value. Sanitizers do not establish the proof, and bounded exhaustive small
ranges do not establish every native-range argument. Publish raw expected/observed
exits and input identities; retain failed attempts rather than relabeling them.

Graduated hints: (1) list the types after promotion before drawing bits;
(2) separate mathematical admission from evaluation and final publication;
(3) partition by signs and use only thresholds whose own range you can prove;
(4) for division, start with the identity and sign/size constraint on remainder;
(5) compare two destination types using the same operands and inspect whether an
overflow primitive writes its output before returning.

Security scope is a local numerical teaching component with synthetic values.
The observable violation is a wrong admitted size, wrong arithmetic result or
modified sentinel. No crash, UB finding or wraparound is evidence of code
execution or privilege gain. Keep unsafe examples compile-only and repair
admission/publication with regressions. Successful admission still assumes live
objects and trustworthy caller-provided extents.

Sources actually inspected: N1570 C11 draft §§5.2.4.2.1,6.3.1.1/.3/.8,
6.4.4.1,6.5.3.3,6.5.5–7,7.20.1; N617 proposal reproducing older division rules;
GCC15.2 manual §7.2.5; N3096 working draft §7.20.1. See the linked source register
in ASSESSMENT.json for exact captures. Neither draft is the published C17/C23
standard. Reference author runs, physical observations, external review and
independent learner performance are separate dimensions in the validation record.

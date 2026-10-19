# Private exact-criterion analysis

Use the public contract plus the unchanged assessment card. Reference files are
author solutions, not learner artifacts. The oracle is independent literal text
composition and Python integer mathematics; it is not a second copy of C overflow
guards. Six hosted profiles and a finite set of cases cannot prove arbitrary
callers, compiler correctness, absence of UB, or mastery.

## t0206a

Character constants I/D have type int. Signed char -2 and short7 promote to int
on the admitted native model; float -0.5 becomes double and double1.25 remains
double. The reference requests exactly those types. Asking `va_arg(list,float)`
is wrong, even though the original source expression was float. The deliberately
faulty specimen is translated only. A diagnosed trap inserted by GCC would not
be an ISO C expected result.

Changed model: ushort16 and int16 make unsigned short promote to unsigned int.
65535 cannot meet the baseline I/int protocol. Use a range-checked conversion to
int or design a separately named unsigned-tag protocol; do not silently alter
I's contract. The standard's signed/unsigned corresponding-type exception applies
when the value is representable by both; thus an unsigned-int value7 can be an
exception, whereas65535 under int16 cannot. Mention also the void*/character-pointer
exception if asserting an exhaustive rule. Compiler silence does not validate
the unnamed arguments. Representation knowledge does not grant runtime reflection.

## t0206b

Owner `tf_format` initializes source and ends it once on every path. Helper
`size_variadic` borrows `va_list *`, creates copy and ends copy before return;
render helper independently does the same. No copy is consumed twice. The source
is never traversed. FULL ends only source plus the already-ended sizing copy;
count0 still initializes/ends lists and follows the specified capacity/allocation
policy. Callback failure during either pass preserves publication invariants.

A pointer-to-list makes the borrowing model explicit across array/scalar
representations. Passing va_list by value to an arbitrary consuming helper cannot
be assumed to preserve usable caller traversal. Native assignment rejects because
the ABI represents va_list as an array; on another ABI acceptance does not establish
portable independent traversal. Valid alternative: the owner creates separate
copies and gives a clearly consuming helper exclusive use, with matching end in
each initializing function. Require a complete ownership diagram, not a count of
the words va_copy/va_end. Tests cannot observe every missing va_end implementation
consequence; source/lifetime review is a separate obligation.

## t0206c

Baseline literal expectations are I=7;D=1.25; length11 and I=-2;D=-0.50; length13.
Required excludes NUL; capacity=required is FULL, required+1 succeeds. Unknown
tags reject before unknown payload access; count5 rejects without reading any
varargs. Missing known payload or wrong type cannot be probed safely.

The explicit-array variant checks count against available and examines tags before
selecting a field. Separate scalar fields avoid an active-union-member fiction.
It cannot check whether the caller's pointer actually designates the claimed
extent, whether storage remains alive, or whether another thread modifies it.
Borrow through both passes, never retain the array after return. Metadata and
caller storage replace opaque traversal state; compare allocation/copy/space
costs under the same transactional contract. Staging is still required to preserve
destination on a fallible second pass under this backend abstraction.

On size_t64/int32, total<=4*INT_MAX so total+1 fits; the format TF_SIZE branch is
unreachable for a conforming scalar backend in these runs. On narrower size_t,
check each addition while reserving NUL. Do not claim executed branch coverage
for this algebraic portability obligation. Negative and changed-length injection
exercise actual partial rendering and cleanup, not a fabricated encoding failure
from a particular locale. Stable scalar callback text is a precondition.

## t0207a

Tree: +(A,*(B,C)); multiplication's result precedes addition's result, but the
three independent calls have any of six invocation permutations. Each function
body writes the shared trace safely because bodies do not interleave. The
conditional tree ?: (A,B,C) admits A then exactly B or C, and its type analysis
still uses both branch types. With A returning1, trace AB and value2 follow.
Precompute A, B, C in separate full expressions to restrict arithmetic trace to
ABC while retaining value7. This is a narrower observable contract, not a proof
that every existing caller permits that change. All observed traces must belong
to the allowed set; observing one does not exclude the others.

## t0207b

`-1<1u`: int -1 converts to unsigned int maximum, then comparison is false.
For ushort u=1, `-1<u` is false under int16/ushort16, true under int32/ushort16.
In `u+(-1)` with u=65535 the value is65534 in both models but result types differ;
equal outputs alone miss the semantic difference. The model file records ranges,
promotion types, conditional values, and comparison results separately.

For changed exact-width types, explicitly assume int32_t=int and uint32_t=unsigned
int on model1. On model2 use 64-bit int and 32-bit short/unsigned short exact-width
types: both promote to signed int because every uint32_t value fits. The comparison
of negative int32_t -1 against uint32_t1 changes false→true; the common arithmetic
type changes. Merely adding a wider long to the original ABI does not change
int/unsigned-int usual conversions. Require rank/representability derivations.

## t0207c

`cond?-1:1u` has unsigned-int type regardless of selected branch. If true, result
is UINT_MAX. Narrowing to unsigned char can map a value to a different residue;
checking after conversion cannot recover rejected information. In the reference,
range admission precedes conversion and output changes only on success. Any
attempt to cast an already overflowing signed a+b has UB before the cast.

Changing signed input to size_t erases provenance: converted -1 and original
SIZE_MAX are identical parameter values. A size_t range check can correctly reject
both for a byte destination; it cannot identify which was a negative source. Preserve
signed input until validation, or add explicit origin metadata if that distinction
is part of the API. Do not assert the size_t function necessarily accepts negatives.

## t0208a

Left && right supplies a sequence point and may skip right. Comma operator orders
left then right and yields right's value. Commas between arguments supply no such
edge. Calls with independent effects can execute in either order; function bodies
are indeterminately sequenced. The model/executed trace must name events rather
than casually classify every subexpression as one event.

Moving effects into `pair(i++,j++)` remains defined for distinct objects. Moving
both into `pair(i++,i++)` creates unsequenced same-object modifications and UB.
Do not say moving any effect out of a body always creates UB; object identity,
other accesses and sequencing rules decide. Compile-only cases must never receive
a portable golden output or a semantic-mutant success credit.

## t0208b

`i++ + i++` and `pair(i++,i++)` are undefined in C17. Independent body counters
produce arguments1/2 with either body order, so pair result12 is fixed. When both
body calls increment a shared scalar from0, body order AB yields pair(1,2)=12;
BA yields pair(2,1)=21; final shared scalar2 in either case. This is defined
indeterminately sequenced execution, unlike the two unsequenced argument increments.
Inlining as an optimization does not remove abstract-machine function sequencing.
No asynchronous signals, threads, recursive callbacks or overflow are admitted in
these small examples; adding them requires a new analysis.

## t0208c

The repair chooses A, gate, B, C in successive full expressions. All callbacks are
checked before reading a result. Track attempted effects separately from successful
completed count: failure in B records A/G/B effects but completed1; gate failure
records A/G and completed1. Neither calls C. Arithmetic failure occurs after all
three successful callbacks, completed3, result unchanged. The caller must not
retry blindly if callbacks have irreversible effects; the interface reports progress
but does not supply rollback/idempotency.

Multiply sign partitions compare operands against INT_MIN/MAX divisions without
ever evaluating INT_MIN/-1. In the a<=0,b<=0 case, `a!=0 && b<INT_MAX/a`
rejects excessive positive products. Addition uses INT_MAX-b or INT_MIN-b only
in ranges where those subtractions are defined. Tests include INT_MIN*-1 and
canceling final expressions whose multiplication intermediate is out of range;
checking only the final mathematical answer is insufficient. Independent Python
big integers admit both product and sum before generating expected results.

## Discriminating assessment

Six defined mutants separately publish a NUL-inclusive required count, omit release,
modify FULL output, reverse A/B, ignore a failing gate, and narrow without admission.
Each must compile successfully and fail at its intended contract assertion. The
allocator harness retains test-owned storage so omitted release is detected by
ownership counts rather than relying on a sanitizer leak report. An unfinished
learner specimen must compile and fail a public semantic assertion; no assessor
fallback is permitted. Dedicated negative translations and compile-only UB are
different categories and never substitute for these semantic rejections.

Accept alternatives with equivalent contracts and arguments, including different
checked-arithmetic proofs, independently owned lists, and different disjoint staging
storage. Reject fixed-size staging without a proved admitted bound, unbounded tags,
wrong payload extraction, direct publication before possible failure, lost callback
progress, and fixed ordering assumptions inferred from one compiler. Require each
criterion's exact changed condition; one formatter test is not evidence of all
expression objectives. Use DELAYED.md after the specified interval and record
actual learner evidence separately from the reference campaign.

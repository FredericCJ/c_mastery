# Private PR-T03-03 analysis

Do not export this file, `alias.c`, `oracle.py` or `measure.py` as learner
solutions. The unchanged canonical criteria and their exact delayed tasks remain
the assessment authority. The following analysis makes each ordinary and changed
task concrete. All source rules below cite inspected N1570 C11 draft, not a
published C17/C23 text. Author tests are not independent learner evidence.

## T03.07.a

The ordinary FDSlot sequence is EMPTY→ACQUIRED→INITIALIZED→PUBLISHED→EMPTY.
Acquisition allocates untyped storage; the initializer writes FDValue fields.
Allocation failure acquires nothing. Failed initialization returns the slot to
empty and frees its allocation. The initializer, however, must roll back its
own partially acquired child because FDSlot cannot know which child operations
completed and deliberately does not invoke the successful-initialization destructor.
`tests.c` allocates eight bytes, writes19, then injects failure and observes one
rollback, one container release and zero destructor calls. Successful initialization
transfers child cleanup to the destructor. Published view withdrawal is a caller
obligation in FDSlot; setting the one tracked view to NULL is a test protocol,
not proof that C prevents another alias. Repeated reset frees nothing further.

Reject “freeing the main object rolls back children” and “the destructor can
always clean partially initialized state.” Require a concrete ownership ledger
for the delayed child-allocation task; unexplained final allocation counts are
only superficial completion. The compiled lifecycle mutant leaves storage/state
behind after initializer failure and fails this precise transition assertion.

## T03.07.b

For an object without declared type, N1570 6.5p6 distinguishes noncharacter typed
stores, representation copying, and other accesses. A live source FDValue copied
as a character array into genuine allocated storage supplies the source effective
type for subsequent nonmodifying accesses. `os_clone` implements that exact full
copy, with size/alignment/disjointness and live-source premises. A declared
unsigned-char array retains its declared type. Matching alignment or bytes does
not authorize accessing that array as FDValue. The test only compares its bytes.

The delayed wire record has specified endian/value semantics, not host object
representation. Four shifts construct a uint32_t; the first word must fit
INT32_MAX before conversion, then actual FDValue fields are assigned. The second
word allows the full uint32 range. Input `{1,2,3,4,0x89,0xab,0xcd,0xef}` produces
value0x01020304/serial0x89abcdef independently of host byte order. Setting the
first byte to0xff rejects without changing the prior output. Padding is never
compared for semantic equality. A decoder returning the first byte fails the
WIRE control. Require the learner to distinguish every premise, not simply add
an aligning cast.

## T03.07.c

The metadata envelope checks capacity and an alignment compatible with the
requested type; this does not prove an allocator returned actual storage, nor
that its pointer is aligned. Those are callback preconditions. The core requests
sizeof/_Alignof and calls only explicit callbacks. All callbacks forbid reentry
or retained access after release. Host malloc/aligned_alloc/free belong to the
test harness. Extended alignment64 is selected, supported by the tested toolchains,
and sizeof(OsWide) is an alignment multiple. An implementation without this
extended alignment needs a different target contract rather than a guessed cast.

For the delayed fallible destructor, LIVE→BORROWED excludes reset. Returning the
exact pointer consumes the borrow. Reset then enters CLEANUP before the callback;
failure retains the allocation and child byte23, and excludes publication.
Retry success releases child then container exactly once. The callback contract
requires failure to leave retryable resources. A destructor that partially frees
then fails would require a richer progress ledger; this protocol does not make
it safe automatically. The DESTRUCTOR mutant ignores failure and frees early;
the intended failure checks short-circuit before touching freed storage, and the
harness releases remaining tracked child allocations. Target audits root clone
through a volatile callback, inspect every final image and reject malloc. They
do not execute allocation on a board.

## T03.08.a

For positive int17, compatible int, const int, corresponding unsigned int and
character inspection are permitted access types under6.5p7. An unrelated float
lvalue is not; alignment is a separate potential violation. The containing
aggregate exception matters: `aggregate_touch(&object.member,&object)` writes
member23 through OsContainer then reads it through int*, requiring23. The aggregate
actually contains that member; this is not permission to cast any int allocation
to a larger struct. Require the delayed response to identify the specific member
relationship and live enclosing object. A matrix listing only “same type good,
different type bad” misses this objective.

## T03.08.b

Character lvalues may inspect any live object's representation. Copying a valid
uint32_t representation into another uint32_t preserves its value; exact-width
uint32_t, when supplied, has no padding. The direct union experiment reads bytes
of stored0x01020304, then constructs the same byte member in another union and
reads its uint32_t member. C's union reinterpretation note explicitly permits
trap representations in general, but this particular uint32_t has none. Do not
generalize to float, arbitrary bytes, or pointer representations. The observed
native byte order is reported separately from the always-big-endian wire result.
The delayed answer must derive shift/mask decoding without claiming host memcpy
performs a portable wire conversion.

## T03.08.c

Only defined character-copy/typed-field tests are compared at O3 strict aliasing
and O3 no-strict-aliasing; invalid examples remain source analysis. A no-strict
flag changes a compiler optimization contract, not ISO's permitted-lvalue list.
The separate original `extension.c` uses a named `may_alias` typedef, checks
binary32 size/alignment/radix/mantissa and actual1.0 representation, and compares
its read against an independent character copy. GCC's documentation explains
direct union access versus a cast pointer; Clang documents additional relaxed
integer/pointer rules and a pointer-TBAA change in version20. The actual two
compiler versions are recorded. Neither identical output nor a sanitizer silence
establishes portable equivalence. The changed task is met by the second real
compiler and exact extension authority, not by relabeling the same executable.

## T03.09.a

The restricted XOR block modifies output elements, so each such object must be
accessed through its output association; its source read range is disjoint.
In `os_shared`, a and b can be the same read-only source because those objects
are not modified; output remains disjoint. This is N1570 6.7.3.1 example3's
relevant distinction. `os_nested` assigns outer restricted associations to an
inner block's restricted pointers, then uses those paths within the inner block;
this follows example4's permitted outer-to-inner relationship. Merely comparing
addresses does not prove the full block/based-on contract. Same-block assignments
or hidden accesses to a modified object require a separate analysis and are not
executed. Require changed-task block diagrams and explicit access paths.

## T03.09.b

Validate dst/src<=length and n<=length-offset before pointer formation, avoiding
overflowing offset+n. Zero length accepts endpoints without constructing NULL+0.
For nonempty intervals, equal starts select the separate ordinary in-place loop
only when admitted; unequal overlap rejects unchanged; adjacent intervals use
the restricted mapping. The independent oracle constructs Python sets rather
than copying the C subtraction predicate and compares all6048 small cases and
all eight output bytes. Actual fd_move covers both overlapping directions using
snapshot semantics. PARTITION falsely returns success without invoking undefined
restricted accesses; OVERLAP performs the wrong safe-direction byte loop and
fails the value assertion. The changed exact-in-place task requires a new branch
and proof, not weakening the original restricted function's precondition.

## T03.09.c

Ordinary and restricted functions receive identical disjoint source/destination
arrays and XOR mask; no overlapping call is timed. Compile them separately from
the caller without LTO, retain emitted calls, inspect vectorization diagnostics
and symbol sizes, and verify every output byte. Four GCC/Clang strict/relaxed
experiments each retain240 raw measurements:2variants×2sizes×2patterns×2offsets×15.
The changed offset1 condition changes alignment while retaining disjointness;
zero versus pseudorandom contents changes data under the same byte operation.
Compare distributions, code size and any versioned loops; a restricted variant
need not win. Scheduling/cache/clock noise prevents a universal speed guarantee.
Do not benchmark an invalid restrict overlap and interpret its speed as overhead.

## T03.10.a

N1570 lifetime rules do not let a saved pointer become valid merely because a
later allocation has a numerically matching address. Its value becomes indeterminate
when the pointed-to object's lifetime ends; even a comparison is not a valid
demonstration to execute here. Pre-release integer history may be logged under
its representation contract but grants no access authority. The actual FDPool
instead retains one live owner and checks generation tokens. The intentionally
wrapping control cycles slot0 generations1,2,3,1 and lets the old token read4.
The corrected producer retires slot0 and allocates slot1, refusing the stale
token. Retirement costs finite capacity. This is a logical lifetime/authorization
counterexample, not a C object lifetime use-after-free or code-execution exploit.
Full independent mastery also requires an unfamiliar exhaustion trace.

## T03.10.b

Keep three authorities separate. Base draft guarantees include6.3.2.3 pointer
conversions,6.5 access types and optional7.20.1.4 valid void-pointer roundtrip.
They do not specify the named provenance proposals. N2219v3 proposes empty,
single and wildcard provenance; its wildcard admits a different reconstruction
story. N2263v4 removes wildcards and proposes fresh allocation identities, empty
or single provenance, propagated through integer operations; its Q3 ordinary
guess/file reconstruction has empty provenance, with open explicit-IO issues.
Both are proposals, not accepted editions or actual tested compiler switches.

The v4 Q2 discussion permits provenance-sensitive pointer equality, while the
partial-core paragraph's with/without `-fno-provenance` wording appears reversed.
Record this textual tension and withhold a resolved model claim. GCC's actual
pointer implementation documentation describes selected bit conversion and
preservation of original referenced objects; Clang's aliasing documentation
describes type-based alias assumptions, not full allocation provenance. The
delayed revision comparison must cite both actual inspected passages and this
limitation, not synthesize a universal pointer semantics from compiler output.

## T03.10.c

The ordinary tag experiment is conditional on supplied uintptr_t, selected live
object mapping and actual zero low bits. Tags exist only in integers; mask restores
exact original integer, conversion returns a void pointer equal to the owner.
The function still accesses through the retained live owner. It does not claim
arbitrary integer fabrication, or prove a new provenance model. `os_member`
starts from the whole live object's character representation, adds its known
member offset, and compares the actual member pointer before using the owner.
This avoids unsupported subtraction from an arbitrary member-alone subobject.

The delayed alternative is the actual owner/index/generation FDHandle: every
lookup checks owner, index, occupancy and generation, and exhaustion retires a
slot. It costs the measured handle size, checks, stable owner and capacity loss;
the i386 and native cost reports make the changed layout concrete. Neither design
provides C-enforced ownership, malicious-pointer admission or universal safety.

## Evidence boundary

Positive finite oracles, compiled safe mutants and strict final-link audits
support only their named premises. No undefined aliasing program is executed.
The proposal analysis is passage-based author reasoning, not a formal proof or
an implemented provenance checker. No target execution, physical measurement,
external review or independently delayed learner attempt is inferred.

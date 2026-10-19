# Private qualified-view assessment

## t0209a

An uninitialized automatic record has indeterminate values; never read it merely
to populate a predicted table. Static implicit record members are arithmetic zero
and pointer null. Explicit designated initialization gives0,7,0,9 and an omitted
pointer member is initialized to null. `assigned=initialized` is later assignment
to an existing object, copying member values. The public test never byte-compares
records. N1570 6.7.9p10 gives zero padding for implicit static aggregate initialization;
6.2.6.1p6 makes padding unspecified after relevant stores. Avoid both the claim that
all padding is always zero and the claim that static implicit initialization says
nothing about it.

Changed pointer member: initialize by NULL/{0}, compare pointer values with NULL.
A hypothetical implementation whose null representation has nonzero bits is a
counterexample to portable memset-zero initialization. Do not fabricate a native
execution on such a target. A byte dump on x86-64 or Cortex-M4 observes only that
implementation. Reset uses aggregate value semantics; an emitted memory helper is
a compiler lowering under that target's representation, not a C source-level proof
that zero bytes represent null everywhere.

## t0209b

Designators1 and3 imply array length4 when the highest designator is3; omitted
elements0 and2 are zero. The immediate argument literal remains alive through the
call in its enclosing block. The file-scope literal has static duration and can be
mutated because its element type is non-const. Its aliases observe the change;
static duration does not imply const. Require a duration/lifetime diagram separate
from observed addresses. Non-const literals in these examples are distinct objects;
do not generalize to the implementation's permitted sharing of const-qualified
compound literals/string literals.

## t0209c

`escape` returns the first element address of an automatic block literal. The
object ends on function return, making every pointer to it or one-past it
indeterminate. `wrapped_escape` returns a struct value containing the same bad
pointer; the struct copy does not preserve the array. Both specimens compile only
and are never called, compared or dereferenced. Diagnostic presence or absence is
not the proof of invalid lifetime.

The repair `fv_initialize` copies the literal's values into the caller record
before returning and returns a pointer into that record. Its view remains valid
until the caller record ends; it is not immortal. Alternative: return a structure
containing the array by value, then let the caller create a view into its received
object. A structure containing only a pointer is not equivalent. Copying values
also preserves null semantics without inspecting padding.

## t0210a

The positive fixture reassigns a pointer-to-const, modifies a mutable pointee
through a const pointer, and observes that mutation through a const-const pointer.
Negative translations separately attempt the prohibited operations. `int**` to
`const int**` is not the single-level qualifier addition rule: if allowed, a callee
could store the address of a truly const int through the latter, leaving a mutable
int pointer capable of writing that const object. The first conversion is a
constraint violation; a cast followed by such a write would not repair the UB.

The opaque owner getter returns the same array as its mutable alias; writing71
through the mutable alias is then observed through the retained const view. Layout
hiding and access qualification constrain different actions. Opaque does not mean
unique, immutable or thread safe. Consumers cannot use sizeof(FvOwner); the actual
negative translation discriminates opacity from a mere typedef synonym.

## t0210b

Copy validates known pointer/length consistency, then capacity, before any writes.
Insufficient capacity preserves all five test values, including the first byte/value
that a partial-copy implementation might clear. Empty source performs no access.
The caller remains responsible for actual extents, initialization, lifetime and
disjointness. Getter const is a compiler access restriction, not proof of these facts.

Live view and snapshot start at0,7,0,9. Owner update to8 at index1 changes the live
view; the independent snapshot retains7. A view across owner reset stays valid
but sees changed values; a view after owner lifetime ends is invalid. The static
opaque owner illustrates a different lifetime, not a general exemption from alias
rules. Never call a racing mutation/read demonstration as a golden expected case.

## t0210c

In this block, restrict pointers read/write disjoint accessed int objects; overlapping
source/destination would violate the intended promise. The changed operation removes
that assumption and uses one base plus integer indices. Reject indices>extent before
subtracting, then require count<=both remaining extents. Backward copy is necessary
when destination is greater than source and overlaps it. Other positions copy forward.
The reference uses no pointer ordering across unrelated objects. The independent
oracle assigns a slice from an unchanged Python snapshot, not the implementation's
direction algorithm; it enumerates11979 index/count/extent tuples up to extent8.

For volatile, the stable-RAM value result0 cannot tell one read from two. Inspect
the optimized function's actual loads for both native compilers and both target
compilers. The cached-read mutant still performs its one required volatile read
and returns0; it is a defined program with the wrong access contract. Its rejection
is an instruction/access observation, not a failed C value test. Actual emitted
loads are compiler/ISA evidence only. Device behavior requires a real register
contract. Volatile creates no interthread synchronization and restrict is not an
ownership lock; conflicting unsynchronized non-atomic accesses would be a data race.

## Evidence and resource analysis

Three separately compiled, defined semantic mutants alter reset value, publish on
FULL, and copy overlapping data in the wrong direction. Each must compile and fail
its intended assertion; an unfinished learner implementation must also compile and
fail semantically. Qualification negatives instead require relevant compile
diagnostics and a compatible positive control. Lifetimes and uninitialized reads
stay compile-only and do not require a diagnostic. These categories are not
interchangeable.

Native regular object audits permit only memory helpers supplied by the explicit
original provider. Sanitizer-instrumented runs are hosted verification, not strict
target dependency claims. Four direct target builds include component, original
helpers, target driver and startup objects. Final unresolved-symbol set must be
empty; every intermediate unresolved symbol must be in the explicit local closure;
maps/link arguments cannot include archives. Record actual helper use rather than
assuming a particular optimization must emit it. Stack evidence sums each live
static C frame once plus1024 for this masked, nonrecursive no-callback driver,
checked against the linker-reserved32768. This conservative calculation is not a
board measurement or interrupt-loaded bound.

Alternative implementations can copy member-by-member rather than use aggregate
assignment and can use any overlap algorithm with equivalent O(1) storage and
proved source-value preservation. A temporary full-size buffer changes the stated
resource contract. Reject casts that silence const diagnostics, memset-based null
claims, object-byte equality as value equality, dangling by-value wrappers, and
hardware/thread guarantees inferred from volatile. Rubric score2 requires each
exact mechanism and an independent counterexample; score3 additionally requires
the withheld changed condition with revised assumptions. No reference receipt is
independent learner evidence.

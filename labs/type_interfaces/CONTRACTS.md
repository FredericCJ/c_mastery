# Types that survive a real consumer

This required PR-T02-01 successor implements its15 original ordinary criteria and
15 unchanged delayed variants. Read the original assessment card with this
contract. Prerequisites are the T01 object/sequence and actual build-stage dossier;
bring your own two-TU tool, compile commands and before/after-output reasoning.
The new artifact is a self-contained public header plus independently compiled
implementation and consumers. The supplied header fixes interfaces; implementing
the bodies and explaining the declarations remain learner work.

Use hosted ISO C17, profile A, Linux x86-64, GCC and Clang. The core uses only
original code and standard type declarations, no allocation or library mechanism.
Host tests may use stdio, POSIX threads, nm/readelf and Python. An isolated C23
capability test compares only empty parameter lists against inspected N3096
6.7.6.3p13; this pre-publication draft is not the published2024 standard. The C17
baseline does not change. Actual object/link inspection belongs to the toolchain
and ABI evidence, while compatibility and lifetime arguments concern ISO C.

## Concrete component and ownership contract

Implement `learner.c` against `types.h`, then run
`python labs/type_interfaces/run.py --build build/type-contract`. The row factory
returns a pointer to one actual four-int static array, initially1,2,3,4. It remains
live to program termination, is borrowed and mutable, and is shared among calls.
It is not a fresh allocation, snapshot or per-consumer owner. The function factory
returns a compatible function computing twice x for x in[-1000,1000]. The wrapped
factory contains that same callable type in a structure; call its member explicitly.

Two opaque owners, indexed0 and1, have distinct static storage and start at0.
`td_owner` returns null for any other index. `td_get` publishes the value and
`td_add` admits only deltas and resulting owner values in[-1000,1000]. Failure
preserves the owner and output. Only genuine returned owner pointers are admitted;
the API does not validate arbitrary pointer representations or forged lifetimes.
Consumers cannot require layout, but the implementation can. No destroy or free
operation is valid for these static borrowed handles. Operations are serialized.

Meters, seconds and speed are separate wrapper types containing int. The teaching
domain is exact nonnegative integral units: distance and speed0 through1000,
duration1 through1000. `td_speed` rejects a nonintegral quotient; `td_distance`
rejects a product outside0 through1000 before multiplying. Both preserve output
on failure; a null output is INVALID. Wrappers reject accidental argument-type
swaps at translation but do not enforce units, range or exact divisibility by
themselves. Explicit construction from an arbitrary integer remains possible.

`td_rows` borrows n actual four-int rows, n0 through4. A live disjoint output is
required. For nonempty input rows and callback must be nonnull. Empty input may
have null rows/callback/context and publishes0 with no callback. Every element
must lie in[-1000,1000]; validate all before making any callback. Call in row-major
order, once per element, and sum returned values. The callback contract requires
a returned value in[-2000,2000], making the total fit even a16-bit int. Context is
borrowed synchronously and may be null only if that callback permits it. It must
not mutate input rows, output or dispatch metadata. Range/admission failure makes
no callback and preserves output. The dispatcher cannot undo arbitrary callback
effects after execution starts; callback failure/cancellation is not part of this
ordinary synchronous operation.

`td_step(int,void*)` borrows a TdContext with bias[-1000,1000], increments its call
count and returns value+bias for value[-1000,1000]. The caller reserves enough
unsigned call-count headroom. `td_adapter(void*,int)` invokes it with reordered
arguments; no function-pointer cast is an adapter. Total work is linear in rows,
constant auxiliary space; factories and single-owner/unit operations are constant
time and storage. No heap ownership or hidden runtime allocation is introduced.

## T02.01: declarators, definitions and aliases

For .01.a, annotate `int *a[4]`, `int (*b)[4]` and `int (*cb)(int,void*)` outward
from the identifier. Supply valid initialization and an expression using each;
produce a negative pointer-array/row-pointer swap. For transfer derive the complete
declaration of the row factory and explain the static lifetime of its result.
The row pointer advances by sizeof one row, not sizeof one pointer.

For .01.b, make a table for `extern int x;`, `int x;`, `typedef int Count;`, a
prototype and function definition in two actual TUs. Distinguish a tentative
definition from a mere declaration; inspect the defined and undefined symbols.
Use -fno-common in the declared experiment. For transfer, add an initializer to
extern and show that it now supplies a definition; combining it with another TU's
definition is invalid even if an alternate linker mode tolerates duplicates.

For .01.c, express a factory returning int(int) through two typedefs and through
the full declarator. Redeclare the same factory both ways and call its returned
pointer. A typedef is an alias, not new runtime storage or a new incompatible
type. For transfer replace the alias-returning API with TdCallable and derive the
new member call and assignment shapes. Explain what a wrapper changes and what
it still cannot enforce about the pointed-to implementation.

## T02.02: type categories and meaningful separation

For .02.a, classify `struct Opaque;`, `struct Pair{int x,y;}`, `int[4]`, void and
int(int). Give completeness, object/function and scalar/aggregate categories;
identify legal sizeof operands. In this C terminology void is an incomplete
object type that cannot be completed; it is not a sizeof-able zero-byte object.
For transfer write two independent opaque-handle consumers, one adding and one
reading, with no layout in their header. Compile them and the provider separately;
move every layout-dependent operation into the provider. Calling an opaque API
does not confer ownership of the underlying static object.

For .02.b, compare declarations of `int f(const int)`/`int f(int)` with
`int g(int*)`/`int g(const int*)`. Top-level parameter qualification and pointed-to
qualification have different compatibility consequences. Diagnose one-TU conflicts
and retain an independently compiled conflicting pair without executing its
incompatible call. For transfer change a callback's int parameter to unsigned.
A cast may be legal to write yet calling through the incompatible type is undefined.
Provide the appropriate explicit conversion/admission wrapper when the intended
numeric domain actually permits it; do not infer permission from the ABI's registers.

For .02.c, show a swapped Meters/Seconds typedef call compiles, then show the same
swap rejected with distinct structures. Implement exact speed and distance above,
including0, division by0 refusal, nonintegral quotient and multiplication limit.
For transfer derive the units of multiplication/division and identify where a
specification or runtime check remains necessary. A wrapper does not prevent a
programmer deliberately constructing seconds from a distance's member value.

## T02.03: scope and namespace are separate coordinates

For .03.a, draw a scope map for a file object, shadowing local, prototype parameter
and label. Mark each declaration's point of entry; a label has function scope,
ordinary identifiers generally enter after their declarator, and a prototype-only
parameter expires with its declarator. Analyze `int x=x;` inside a block with an
outer x without executing an indeterminate-value read. For transfer introduce a
structure tag only inside a prototype, then a file-scope tag of the same spelling.
Show the conflict and repair with a file-scope forward declaration before the API.

For .03.b, use one spelling as a tag, ordinary variable, label and members of two
different structures in a defined program. Contrast this with a typedef and object
of that spelling in the same ordinary namespace. Provide both a positive program
and a real negative translation. “All names conflict” and “typedefs are tags” are
both incorrect explanations.

For .03.c, make a defined shadowing error where an inner status becomes7 but the
outer result remains0. Trace bindings before changing code. Repair the intended
outer assignment, retaining a warning diagnostic with -Wshadow as additional
evidence. For transfer move reporting into a callback carrying an explicit pointer
to the caller's live status; demonstrate which storage changes and delimit that
borrow. C supplies no implicit closure capture and cannot prove the context lives.

## T02.04: storage duration, linkage and separate declarations

For .04.a, separately compile a file static, external object, block automatic and
block static. Classify linkage independently from duration; use symbols plus a
two-call persistent-state observation. For transfer give two TUs identically named
internal state, compare their live addresses and mutate one while the other stays
unchanged. Do not compare expired automatic-object addresses.

For .04.b, a block extern after a visible file static inherits its internal linkage.
A register local has automatic storage duration and no linkage; taking its address
violates a C17 constraint even if the compiler would otherwise store it in memory.
Observe separate _Thread_local values in main and one joined POSIX worker; record
the API that provides synchronization, not just thread storage. The type feature
dates to C11. For transfer hide the prior static with a no-linkage block identifier,
then add an inner extern. Derive external linkage and the resulting mixed-linkage
undefined case within that TU; compile only and record any diagnostics, without
requiring the standard to mandate one or executing the resulting program.

For .04.c, independently compile a double(double) provider and int(int) caller;
retain link observations but never execute their incompatible call. Include one
shared header to expose a conflict inside the provider, repair it, and run the
compatible positive pair. For transfer use actual GCC LTO with a declared
type-mismatch diagnostic policy and its valid control. An LTO refusal detects this
case; neither LTO nor successful linking proves program-wide C conformance.

## T02.05: calls, adjusted parameters and retained contexts

For .05.a, implement and document td_rows above before constructing consumers:
zero rows, null admission, callback result bounds, output ownership and order are
explicit. For transfer compare C17 `int f();` with the N3096 empty-list rule and an
actual compiler C23 capability probe. In C17 a nondefinition empty list does not
specify parameters; in the inspected later draft it means a void parameter list.
Keep invalid later-edition calls in negative translation only. Do not relabel a
compiler's __STDC_VERSION__ as evidence of every standard feature.

For .05.b, measure sizeof inside `f(int a[4])` versus `g(int (*a)[4])`; compare
sizeof the adjusted pointer with sizeof the pointed-to row, using actual argument
shapes. Array-looking parameter syntax does not carry runtime capacity. For
transfer use `int a[static 4]` with a genuine four-element object and derive the
minimum-access caller precondition. It supplies no required runtime length check;
a shorter object is analysis-only, not a valid success case.

For .05.c, implement the actual reordered wrapper and separately analyze the cast
alternative without calling it. For transfer implement TdJob as a deferred,
single-thread-owned callback slot. Initialize IDLE; submit nonnull callback once
to enter PENDING without invoking it. The caller keeps context and argument-related
objects live until DONE or acknowledged CANCELED. Run changes PENDING to RUNNING,
calls exactly once, publishes the callback value, clears borrowed pointers and
enters DONE. Run on any other terminal/idle state returns STATE preserving output.
Cancel only PENDING; clear pointers, enter CANCELED and guarantee no later callback.
Reset only IDLE/DONE/CANCELED; PENDING/RUNNING returns BUSY. Reentrant run/cancel/reset
while RUNNING returns BUSY. Failed submit never replaces a retained borrow.

This deferred invocation occurs in a later call on the same owner thread. It is
not an implementation of OS I/O cancellation or concurrent preemption. No field
is atomic. A running callback cannot be reclaimed by claiming cancellation success;
the BUSY result requires retaining context until the outer run returns. Context
lifetime, callback signature, nonlocal exits and cross-thread exclusion remain
caller obligations. ISO C diagnoses some type mismatches; this state machine adds
runtime lifecycle checks but no borrow checker or proof against forged pointers.

## Evidence, hints and assessment

Submit the header/provider, two consumers, scope/category/linkage tables, exact
commands and object evidence, callback lifetime diagram and criterion-indexed
predictions. Preserve negative translations separately from compile-only undefined
calls and from semantic mutants that compile. Tests must reject aliasing both
opaque owners, truncating nonintegral units, repeating the first row and falsely
acknowledging cancellation. Add an independently chosen boundary and defend it.

Hints: first derive the type of the expression using the identifier; next mark
where its object lives and who may retain it; finally ask which translation unit
can see enough information to diagnose the defect. The original0–3 rubric applies
per criterion: a matching example without the relevant type/lifetime argument is
superficial; independent changed-condition derivation is level3. The private
analysis and delayed administration package are excluded from learner export.
Current author evidence is recorded separately from non-author review, physical
observations and independent learner attempts after at least7 days.
